/*
	Copyright (C) 2011 - 2025
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

/**
 * @file
 * Emscripten WebSocket-based implementation of wesnothd_connection.
 *
 * Replaces the Boost.Asio TCP/TLS + worker-thread implementation
 * for the web port. Uses websocket_bridge.hpp declarations (EM_JS
 * definitions live in network_asio_emscripten.cpp) and
 * emscripten_sleep() for cooperative yielding (JSPI).
 *
 * Same Wesnoth wire protocol as native:
 *   1. Handshake: client sends 4 zero bytes, server responds with 4 bytes
 *   2. Messages:  [4-byte BE length][gzipped WML]
 */

#ifdef __EMSCRIPTEN__

#include "wesnothd_connection.hpp"

#include "websocket_bridge.hpp"

#include "gui/dialogs/loading_screen.hpp"
#include "log.hpp"
#include "serialization/parser.hpp"

#include <emscripten.h>

#include <arpa/inet.h>
#include <cstring>
#include <sstream>
#include <vector>

static lg::log_domain log_network("network");
#define DBG_NW LOG_STREAM(debug, log_network)
#define LOG_NW LOG_STREAM(info, log_network)
#define WRN_NW LOG_STREAM(warn, log_network)
#define ERR_NW LOG_STREAM(err, log_network)

// ── Implementation struct ───────────────────────────────────────────────

enum class ws_wd_state
{
	CONNECTING,
	HANDSHAKE_SENT,
	CONNECTED,
	CLOSED
};

struct ws_wesnothd_impl
{
	int ws_id = 0;
	ws_wd_state state = ws_wd_state::CONNECTING;

	// Receive buffer — accumulated from JS WebSocket callbacks
	std::vector<uint8_t> recv_buf;

	// Deserialized messages ready for the caller
	std::queue<config, std::list<config>> recv_queue;

	// Progress tracking
	std::size_t bytes_to_write = 0;
	std::size_t bytes_written = 0;
	std::size_t bytes_to_read = 0;
	std::size_t bytes_read = 0;
};

// ── Helpers ─────────────────────────────────────────────────────────────

/** Move all available data from the JS receive buffer into impl->recv_buf. */
static void drain_js_buffer(ws_wesnothd_impl& s)
{
	int avail = wa_ws_recv_available(s.ws_id);
	if(avail > 0) {
		std::size_t offset = s.recv_buf.size();
		s.recv_buf.resize(offset + static_cast<std::size_t>(avail));
		wa_ws_recv(s.ws_id, s.recv_buf.data() + offset, avail);
	}
}

/** Try to decode complete messages from recv_buf into recv_queue. */
static void try_decode_messages(ws_wesnothd_impl& s)
{
	while(s.recv_buf.size() >= 4) {
		uint32_t net_size;
		std::memcpy(&net_size, s.recv_buf.data(), 4);
		uint32_t payload_size = ntohl(net_size);

		if(s.recv_buf.size() < 4 + payload_size) {
			// Incomplete message — wait for more data
			s.bytes_to_read = 4 + payload_size;
			s.bytes_read = s.recv_buf.size();
			break;
		}

		// Full message available — deserialize
		std::string payload(
			reinterpret_cast<const char*>(s.recv_buf.data() + 4),
			payload_size);
		s.recv_buf.erase(
			s.recv_buf.begin(),
			s.recv_buf.begin() + 4 + payload_size);

		std::istringstream is(payload);
		config data = io::read_gz(is);
		if(!data.empty()) {
			DBG_NW << "WS received:\n" << data;
		}

		s.recv_queue.emplace(std::move(data));
		s.bytes_to_read = 0;
		s.bytes_read = 0;
	}
}

static void check_ws_error(ws_wesnothd_impl& s)
{
	int ws = wa_ws_state(s.ws_id);
	if(ws == 3) {
		throw wesnothd_error("WebSocket connection error");
	}
	if(ws == 2 && s.state != ws_wd_state::CLOSED) {
		throw wesnothd_error("WebSocket connection closed unexpectedly");
	}
}

// ── wesnothd_connection implementation ──────────────────────────────────

wesnothd_connection::wesnothd_connection(const std::string& host, const std::string& service)
	: impl_(new ws_wesnothd_impl())
{
	char* proxy_url_c = wa_get_ws_proxy_url();
	std::string ws_url = std::string(proxy_url_c) + "/" + host + "/" + service;
	free(proxy_url_c);

	LOG_NW << "wesnothd WebSocket connecting to proxy: " << ws_url;

	impl_->ws_id = wa_ws_connect(ws_url.c_str());
	if(impl_->ws_id <= 0) {
		throw wesnothd_error("Failed to create WebSocket connection to wesnothd");
	}
}

wesnothd_connection::~wesnothd_connection()
{
	if(impl_ && impl_->ws_id > 0) {
		wa_ws_close(impl_->ws_id);
	}
}

void wesnothd_connection::wait_for_handshake()
{
	LOG_NW << "wesnothd WS: waiting for handshake";
	auto& s = *impl_;

	// Wait for WebSocket to open
	while(wa_ws_state(s.ws_id) == 0) {
		gui2::dialogs::loading_screen::spin();
		emscripten_sleep(1);
	}

	check_ws_error(s);

	// Send non-TLS handshake (4 zero bytes)
	uint32_t handshake = 0;
	wa_ws_send(s.ws_id, reinterpret_cast<const uint8_t*>(&handshake), 4);
	s.state = ws_wd_state::HANDSHAKE_SENT;
	LOG_NW << "wesnothd WS: sent handshake";

	// Wait for 4-byte handshake response
	while(true) {
		gui2::dialogs::loading_screen::spin();
		check_ws_error(s);
		drain_js_buffer(s);

		if(s.recv_buf.size() >= 4) {
			// Consume handshake response (value irrelevant for non-TLS)
			s.recv_buf.erase(s.recv_buf.begin(), s.recv_buf.begin() + 4);
			s.state = ws_wd_state::CONNECTED;
			LOG_NW << "wesnothd WS: handshake complete";
			return;
		}

		emscripten_sleep(1);
	}
}

void wesnothd_connection::send_data(const configr_of& request)
{
	auto& s = *impl_;

	check_ws_error(s);

	// Serialize to gzipped WML
	std::ostringstream os;
	io::write_gz(os, request);
	std::string data = os.str();

	// Build wire message: [4-byte BE length][gzipped WML]
	uint32_t payload_size = htonl(static_cast<uint32_t>(data.size()));
	std::vector<uint8_t> send_buf(4 + data.size());
	std::memcpy(send_buf.data(), &payload_size, 4);
	std::memcpy(send_buf.data() + 4, data.data(), data.size());

	s.bytes_to_write = send_buf.size();
	s.bytes_written = 0;

	int rc = wa_ws_send(s.ws_id, send_buf.data(), static_cast<int>(send_buf.size()));
	if(rc < 0) {
		throw wesnothd_error("Failed to send data via WebSocket to wesnothd");
	}

	s.bytes_written = s.bytes_to_write;
	DBG_NW << "wesnothd WS: sent " << send_buf.size() << " bytes";
}

bool wesnothd_connection::receive_data(config& result)
{
	auto& s = *impl_;

	check_ws_error(s);
	drain_js_buffer(s);
	try_decode_messages(s);

	if(!s.recv_queue.empty()) {
		result.swap(s.recv_queue.front());
		s.recv_queue.pop();
		return true;
	}

	return false;
}

bool wesnothd_connection::wait_and_receive_data(config& data)
{
	while(!receive_data(data)) {
		gui2::dialogs::loading_screen::spin();
		emscripten_sleep(1);
	}
	return true;
}

void wesnothd_connection::cancel()
{
	if(impl_ && impl_->ws_id > 0) {
		wa_ws_close(impl_->ws_id);
		impl_->ws_id = 0;
		impl_->state = ws_wd_state::CLOSED;
	}
}

void wesnothd_connection::stop()
{
	cancel();
}

std::size_t wesnothd_connection::bytes_to_write() const
{
	return impl_->bytes_to_write;
}

std::size_t wesnothd_connection::bytes_written() const
{
	return impl_->bytes_written;
}

std::size_t wesnothd_connection::bytes_to_read() const
{
	return impl_->bytes_to_read;
}

std::size_t wesnothd_connection::bytes_read() const
{
	return impl_->bytes_read;
}

bool wesnothd_connection::has_data_received() const
{
	return !impl_->recv_queue.empty();
}

bool wesnothd_connection::is_sending_data() const
{
	return false; // WebSocket send is non-blocking
}

#endif // __EMSCRIPTEN__
