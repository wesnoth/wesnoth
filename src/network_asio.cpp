/*
   Copyright (C) 2011 - 2018 by Sergey Popov <loonycyborg@gmail.com>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define BOOST_ASIO_NO_DEPRECATED

#include "network_asio.hpp"

#include "log.hpp"
#include "serialization/parser.hpp"

#include <boost/asio/connect.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>

#include <algorithm>
#include <cstdint>
#include <deque>
#include <functional>

static lg::log_domain log_network("network");
#define DBG_NW LOG_STREAM(debug, log_network)
#define LOG_NW LOG_STREAM(info, log_network)
#define WRN_NW LOG_STREAM(warn, log_network)
#define ERR_NW LOG_STREAM(err, log_network)

namespace
{
std::deque<boost::asio::const_buffer> split_buffer(boost::asio::streambuf::const_buffers_type source_buffer)
{
	const unsigned int chunk_size = 4096;

	std::deque<boost::asio::const_buffer> buffers;
	unsigned int remaining_size = boost::asio::buffer_size(source_buffer);

#if BOOST_VERSION >= 106600
	const uint8_t* data = static_cast<const uint8_t*>(source_buffer.data());
#else
	const uint8_t* data = boost::asio::buffer_cast<const uint8_t*>(source_buffer);
#endif

	while(remaining_size > 0u) {
		unsigned int size = std::min(remaining_size, chunk_size);
		buffers.emplace_back(data, size);
		data += size;
		remaining_size -= size;
	}

	return buffers;
}
}

namespace network_asio
{
using boost::system::system_error;

connection::connection(const std::string& host, const std::string& service)
	: io_context_()
	, host_(host)
	, service_(service)
	, resolver_(io_context_)
	, socket_(raw_socket{io_context_})
	, done_(false)
	, write_buf_()
	, read_buf_()
	, handshake_response_()
	, payload_size_(0)
	, bytes_to_write_(0)
	, bytes_written_(0)
	, bytes_to_read_(0)
	, bytes_read_(0)
{
	boost::system::error_code ec;
	auto result = resolver_.resolve(host, service, boost::asio::ip::resolver_query_base::numeric_host, ec);
	if(!ec) { // if numeric resolve succeeds then we got raw ip address so TLS host name validation would never pass
		use_tls_ = false;
		boost::asio::post(io_context_, [this, ec, result](){ handle_resolve(ec, { result } ); } );
	} else {
		resolver_.async_resolve(host, service, 
			std::bind(&connection::handle_resolve, this, std::placeholders::_1, std::placeholders::_2));
	}

	LOG_NW << "Resolving hostname: " << host << '\n';
}

void connection::handle_resolve(const boost::system::error_code& ec, results_type results)
{
	if(ec) {
		throw system_error(ec);
	}

	boost::asio::async_connect(utils::get<raw_socket>(socket_), results,
		std::bind(&connection::handle_connect, this, std::placeholders::_1, std::placeholders::_2));
}

void connection::handle_connect(const boost::system::error_code& ec, endpoint endpoint)
{
	if(ec) {
		ERR_NW << "Tried all IPs. Giving up" << std::endl;
		throw system_error(ec);
	} else {
#if BOOST_VERSION >= 106600
		LOG_NW << "Connected to " << endpoint.address() << '\n';
#else
		LOG_NW << "Connected to " << endpoint->endpoint().address() << '\n';
#endif
		if(endpoint.address().is_loopback()) {
			use_tls_ = false;
		}
		handshake();
	}
}

void connection::handshake()
{
	static const uint32_t handshake = 0;
	static const uint32_t tls_handshake = htonl(uint32_t(1));

	boost::asio::async_write(
		utils::get<raw_socket>(socket_),
		boost::asio::buffer(use_tls_ ? reinterpret_cast<const char*>(&tls_handshake) : reinterpret_cast<const char*>(&handshake), 4),
		std::bind(&connection::handle_write, this, std::placeholders::_1, std::placeholders::_2)
	);

	boost::asio::async_read(utils::get<raw_socket>(socket_), boost::asio::buffer(&handshake_response_.binary, 4),
		std::bind(&connection::handle_handshake, this, std::placeholders::_1));
}

void connection::handle_handshake(const boost::system::error_code& ec)
{
	if(ec) {
		if(ec == boost::asio::error::eof && use_tls_) {
			// immediate disconnect likely means old server not supporting TLS handshake code
			fallback_to_unencrypted();
			return;
		}

		throw system_error(ec);
	}
	
	if(use_tls_) {
		if(handshake_response_.num == 0xFFFFFFFFU) {
			use_tls_ = false;
			handle_handshake(ec);
			return;
		}

		if(handshake_response_.num == 0x00000000) {
			tls_context_.set_default_verify_paths();
			raw_socket s { std::move(utils::get<raw_socket>(socket_)) };
			socket_.emplace<1>(std::move(s), tls_context_);
			
			auto& socket { utils::get<tls_socket>(socket_) };

			socket.set_verify_mode(
				boost::asio::ssl::verify_peer |
				boost::asio::ssl::verify_fail_if_no_peer_cert
			);

			socket.set_verify_callback(boost::asio::ssl::host_name_verification(host_));

			socket.async_handshake(boost::asio::ssl::stream_base::client, [this](const boost::system::error_code& ec) {
				if(ec) {
					throw system_error(ec);
				}

				done_ = true;
			});
			return;
		}

		fallback_to_unencrypted();
	} else {
		done_ = true;
	}
}

void connection::fallback_to_unencrypted()
{
	assert(use_tls_ == true);
	use_tls_ = false;

	boost::asio::ip::tcp::endpoint endpoint { utils::get<raw_socket>(socket_).remote_endpoint() };
	utils::get<raw_socket>(socket_).close();

	utils::get<raw_socket>(socket_).async_connect(endpoint,
		std::bind(&connection::handle_connect, this, std::placeholders::_1, endpoint));
}

void connection::transfer(const config& request, config& response)
{
#if BOOST_VERSION >= 106600
	io_context_.restart();
#else
	io_context_.reset();
#endif
	done_ = false;

	write_buf_.reset(new boost::asio::streambuf);
	read_buf_.reset(new boost::asio::streambuf);
	std::ostream os(write_buf_.get());
	write_gz(os, request);

	bytes_to_write_ = write_buf_->size() + 4;
	bytes_written_ = 0;
	payload_size_ = htonl(bytes_to_write_ - 4);

	auto bufs = split_buffer(write_buf_->data());
	bufs.push_front(boost::asio::buffer(reinterpret_cast<const char*>(&payload_size_), 4));

	utils::visit([this, &bufs, &response](auto&& socket) {
		boost::asio::async_write(socket, bufs,
			std::bind(&connection::is_write_complete, this, std::placeholders::_1, std::placeholders::_2),
			std::bind(&connection::handle_write, this, std::placeholders::_1, std::placeholders::_2));

		boost::asio::async_read(socket, *read_buf_,
			std::bind(&connection::is_read_complete, this, std::placeholders::_1, std::placeholders::_2),
			std::bind(&connection::handle_read, this, std::placeholders::_1, std::placeholders::_2, std::ref(response)));
	}, socket_);
}

void connection::cancel()
{
	utils::visit([](auto&& socket) {
		if(socket.lowest_layer().is_open()) {
			boost::system::error_code ec;

#ifdef _MSC_VER
// Silence warning about boost::asio::basic_socket<Protocol>::cancel always
// returning an error on XP, which we don't support anymore.
#pragma warning(push)
#pragma warning(disable:4996)
#endif
			socket.lowest_layer().cancel(ec);
#ifdef _MSC_VER
#pragma warning(pop)
#endif

			if(ec) {
				WRN_NW << "Failed to cancel network operations: " << ec.message() << std::endl;
			}
		}
	}, socket_);
	bytes_to_write_ = 0;
	bytes_written_ = 0;
	bytes_to_read_ = 0;
	bytes_read_ = 0;
}

std::size_t connection::is_write_complete(const boost::system::error_code& ec, std::size_t bytes_transferred)
{
	if(ec) {
		throw system_error(ec);
	}

	bytes_written_ = bytes_transferred;
	return bytes_to_write_ - bytes_transferred;
}

void connection::handle_write(const boost::system::error_code& ec, std::size_t bytes_transferred)
{
	DBG_NW << "Written " << bytes_transferred << " bytes.\n";
	if(write_buf_)
		write_buf_->consume(bytes_transferred);

	if(ec) {
		throw system_error(ec);
	}
}

std::size_t connection::is_read_complete(const boost::system::error_code& ec, std::size_t bytes_transferred)
{
	if(ec) {
		throw system_error(ec);
	}

	bytes_read_ = bytes_transferred;
	if(bytes_transferred < 4) {
		return 4;
	}

	if(!bytes_to_read_) {
		std::istream is(read_buf_.get());
		data_union data_size;

		is.read(data_size.binary, 4);
		bytes_to_read_ = ntohl(data_size.num) + 4;

		// Close immediately if we receive an invalid length
		if(bytes_to_read_ < 4) {
			bytes_to_read_ = bytes_transferred;
		}
	}

	return bytes_to_read_ - bytes_transferred;
}

void connection::handle_read(const boost::system::error_code& ec, std::size_t bytes_transferred, config& response)
{
	DBG_NW << "Read " << bytes_transferred << " bytes.\n";

	bytes_to_read_ = 0;
	bytes_to_write_ = 0;
	done_ = true;

	if(ec && ec != boost::asio::error::eof) {
		throw system_error(ec);
	}

	std::istream is(read_buf_.get());
	read_gz(response, is);
}
}
