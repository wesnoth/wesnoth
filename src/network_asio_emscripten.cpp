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
 * Emscripten WebSocket-based implementation of network_asio::connection.
 *
 * Replaces the Boost.Asio TCP/TLS implementation for the web port.
 * Connects to a WebSocket-to-TCP proxy (ws_proxy.py) which bridges
 * to the real campaignd/wesnothd TCP server.
 *
 * The Wesnoth wire protocol is preserved unchanged:
 *   1. Handshake: client sends 4 zero bytes, server responds with 4 bytes
 *   2. Transfer:  client sends [4-byte BE length][gzipped WML]
 *                 server responds with [4-byte BE length][gzipped WML]
 *
 * WebSocket carries these bytes transparently as binary frames.
 */

#ifdef __EMSCRIPTEN__

#include "network_asio.hpp"

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

// ── JavaScript WebSocket helpers (via EM_JS) ────────────────────────────
//
// State lives on Module._wsConns keyed by integer ID.
// Callbacks buffer received data in JS arrays; C++ drains via wa_ws_recv.
//
// NOTE: EM_JS functions cannot be shared across translation units in a
// static archive (the linker won't extract __em_js__ symbols on demand).
// These are duplicated in wesnothd_connection_emscripten.cpp.

EM_JS(char*, wa_get_ws_proxy_url, (), {
	var url = Module.wsProxyUrl || "ws://localhost:8041";
	var len = lengthBytesUTF8(url) + 1;
	var buf = _malloc(len);
	stringToUTF8(url, buf, len);
	return buf;
});

EM_JS(int, wa_ws_connect, (const char* url_ptr), {
	var url = UTF8ToString(url_ptr);
	if (!Module._wsConns) {
		Module._wsConns = {};
		Module._wsNextId = 0;
	}
	var id = ++Module._wsNextId;
	var conn = {
		ws: null,
		state: 0,       // 0=connecting, 1=open, 2=closed, 3=error
		recvChunks: [],
		recvLen: 0,
		error: ""
	};
	Module._wsConns[id] = conn;

	try {
		var ws = new WebSocket(url);
		ws.binaryType = "arraybuffer";
		conn.ws = ws;

		ws.onopen = function() {
			if (conn.state === 0) conn.state = 1;
		};
		ws.onmessage = function(e) {
			var data = new Uint8Array(e.data);
			conn.recvChunks.push(data);
			conn.recvLen += data.length;
		};
		ws.onerror = function(e) {
			conn.state = 3;
			conn.error = "WebSocket error";
		};
		ws.onclose = function(e) {
			if (conn.state < 2) conn.state = 2;
		};
	} catch (e) {
		conn.state = 3;
		conn.error = "WebSocket creation failed: " + e.message;
	}
	return id;
});

EM_JS(int, wa_ws_state, (int id), {
	var conn = Module._wsConns ? Module._wsConns[id] : null;
	return conn ? conn.state : -1;
});

EM_JS(int, wa_ws_send, (int id, const uint8_t* data, int len), {
	var conn = Module._wsConns ? Module._wsConns[id] : null;
	if (!conn || !conn.ws || conn.state !== 1) return -1;
	try {
		conn.ws.send(HEAPU8.slice(data, data + len));
		return 0;
	} catch (e) {
		conn.state = 3;
		conn.error = "Send failed: " + e.message;
		return -1;
	}
});

EM_JS(int, wa_ws_recv_available, (int id), {
	var conn = Module._wsConns ? Module._wsConns[id] : null;
	return conn ? conn.recvLen : 0;
});

EM_JS(int, wa_ws_recv, (int id, uint8_t* buf, int maxLen), {
	var conn = Module._wsConns ? Module._wsConns[id] : null;
	if (!conn) return 0;

	var copied = 0;
	while (copied < maxLen && conn.recvChunks.length > 0) {
		var chunk = conn.recvChunks[0];
		var needed = maxLen - copied;
		if (chunk.length <= needed) {
			HEAPU8.set(chunk, buf + copied);
			copied += chunk.length;
			conn.recvChunks.shift();
		} else {
			HEAPU8.set(chunk.subarray(0, needed), buf + copied);
			conn.recvChunks[0] = chunk.subarray(needed);
			copied += needed;
		}
	}
	conn.recvLen -= copied;
	return copied;
});

EM_JS(void, wa_ws_close, (int id), {
	var conn = Module._wsConns ? Module._wsConns[id] : null;
	if (!conn) return;
	try {
		if (conn.ws) conn.ws.close(1000);
	} catch (e) {}
	delete Module._wsConns[id];
});

// ── Implementation struct ───────────────────────────────────────────────

namespace network_asio
{

enum class ws_state
{
	CONNECTING,
	HANDSHAKE,
	IDLE,
	TRANSFERRING
};

struct ws_connection_impl
{
	int ws_id = 0;
	ws_state state = ws_state::CONNECTING;
	bool done = false;

	config* response_ptr = nullptr;

	// Receive buffer — accumulated from JS WebSocket callbacks
	std::vector<uint8_t> recv_buf;

	// Transfer state
	uint32_t expected_payload_size = 0;
	bool payload_size_known = false;

	// Progress tracking
	std::size_t bytes_to_write = 0;
	std::size_t bytes_written = 0;
	std::size_t bytes_to_read = 0;
	std::size_t bytes_read = 0;
};

// ── Helpers ─────────────────────────────────────────────────────────────

/** Move all available data from the JS receive buffer into impl->recv_buf. */
static void drain_js_buffer(ws_connection_impl& s)
{
	int avail = wa_ws_recv_available(s.ws_id);
	if(avail > 0) {
		std::size_t offset = s.recv_buf.size();
		s.recv_buf.resize(offset + static_cast<std::size_t>(avail));
		wa_ws_recv(s.ws_id, s.recv_buf.data() + offset, avail);
	}
}

// ── connection implementation ───────────────────────────────────────────

connection::connection(const std::string& host, const std::string& service)
	: impl_(new ws_connection_impl())
{
	char* proxy_url_c = wa_get_ws_proxy_url();
	std::string ws_url = std::string(proxy_url_c) + "/" + host + "/" + service;
	free(proxy_url_c);

	LOG_NW << "WebSocket connecting to proxy: " << ws_url;

	impl_->ws_id = wa_ws_connect(ws_url.c_str());
	if(impl_->ws_id <= 0) {
		throw error("Failed to create WebSocket connection");
	}
}

connection::~connection()
{
	if(impl_ && impl_->ws_id > 0) {
		wa_ws_close(impl_->ws_id);
	}
}

std::size_t connection::poll()
{
	auto& s = *impl_;

	// Check WebSocket state
	int ws = wa_ws_state(s.ws_id);
	if(ws == 3) { // error
		throw error("WebSocket connection error");
	}
	if(ws == 2 && !s.done) { // closed unexpectedly
		throw error("WebSocket connection closed unexpectedly");
	}

	// Drain any data from JS into our C++ buffer
	drain_js_buffer(s);

	switch(s.state) {
	case ws_state::CONNECTING:
		if(ws == 1) { // WebSocket open
			// Send Wesnoth handshake: 4 zero bytes (no TLS request)
			uint32_t handshake = 0;
			wa_ws_send(s.ws_id, reinterpret_cast<const uint8_t*>(&handshake), 4);
			s.state = ws_state::HANDSHAKE;
			LOG_NW << "WebSocket open, sent handshake";
			return 1;
		}
		break;

	case ws_state::HANDSHAKE:
		if(s.recv_buf.size() >= 4) {
			// Consume 4-byte handshake response (value irrelevant for non-TLS)
			s.recv_buf.erase(s.recv_buf.begin(), s.recv_buf.begin() + 4);
			s.state = ws_state::IDLE;
			s.done = true;
			LOG_NW << "WebSocket handshake complete";
			return 1;
		}
		break;

	case ws_state::TRANSFERRING: {
		// Step 1: Read the 4-byte payload size header
		if(!s.payload_size_known && s.recv_buf.size() >= 4) {
			uint32_t net_size;
			std::memcpy(&net_size, s.recv_buf.data(), 4);
			s.expected_payload_size = ntohl(net_size);
			s.bytes_to_read = s.expected_payload_size + 4;
			s.recv_buf.erase(s.recv_buf.begin(), s.recv_buf.begin() + 4);
			s.payload_size_known = true;
			DBG_NW << "Transfer: expecting " << s.expected_payload_size << " bytes payload";
		}

		// Update progress
		if(s.payload_size_known) {
			s.bytes_read = 4 + s.recv_buf.size();
		}

		// Step 2: Check if full payload has arrived
		if(s.payload_size_known && s.recv_buf.size() >= s.expected_payload_size) {
			// Deserialize the gzipped WML response
			std::string payload(
				reinterpret_cast<const char*>(s.recv_buf.data()),
				s.expected_payload_size);
			s.recv_buf.erase(
				s.recv_buf.begin(),
				s.recv_buf.begin() + s.expected_payload_size);

			std::istringstream is(payload);
			*s.response_ptr = io::read_gz(is);

			// Reset transfer state
			s.done = true;
			s.state = ws_state::IDLE;
			s.payload_size_known = false;
			s.expected_payload_size = 0;
			s.bytes_to_read = 0;
			s.bytes_to_write = 0;
			LOG_NW << "WebSocket transfer complete";
			return 1;
		}
		break;
	}

	case ws_state::IDLE:
		break;
	}

	return 0;
}

void connection::run()
{
	while(!impl_->done) {
		poll();
		emscripten_sleep(1);
	}
}

void connection::transfer(const config& request, config& response)
{
	auto& s = *impl_;

	s.done = false;
	s.state = ws_state::TRANSFERRING;
	s.response_ptr = &response;
	s.recv_buf.clear();
	s.payload_size_known = false;
	s.expected_payload_size = 0;

	// Serialize request to gzipped WML
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
	s.bytes_to_read = 0;
	s.bytes_read = 0;

	int rc = wa_ws_send(s.ws_id, send_buf.data(), static_cast<int>(send_buf.size()));
	if(rc < 0) {
		throw error("Failed to send data via WebSocket");
	}

	// WebSocket send is non-blocking (data is in browser's send buffer)
	s.bytes_written = s.bytes_to_write;
	DBG_NW << "Transfer: sent " << send_buf.size() << " bytes";
}

void connection::cancel()
{
	if(impl_ && impl_->ws_id > 0) {
		wa_ws_close(impl_->ws_id);
		impl_->ws_id = 0;
	}

	impl_->bytes_to_write = 0;
	impl_->bytes_written = 0;
	impl_->bytes_to_read = 0;
	impl_->bytes_read = 0;
}

bool connection::done() const
{
	return impl_->done;
}

bool connection::using_tls() const
{
	return false;
}

std::size_t connection::bytes_to_write() const
{
	return impl_->bytes_to_write;
}

std::size_t connection::bytes_written() const
{
	return impl_->bytes_written;
}

std::size_t connection::bytes_to_read() const
{
	return impl_->bytes_to_read;
}

std::size_t connection::bytes_read() const
{
	return impl_->bytes_read;
}

} // namespace network_asio

#endif // __EMSCRIPTEN__
