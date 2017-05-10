/*
   Copyright (C) 2011 - 2017 by Sergey Popov <loonycyborg@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include <deque>
#include "utils/functional.hpp"
#include <cstdint>
#include "log.hpp"
#include "wesnothd_connection.hpp"
#include "serialization/parser.hpp"

static lg::log_domain log_network("network");
#define DBG_NW LOG_STREAM(debug, log_network)
#define LOG_NW LOG_STREAM(info, log_network)
#define WRN_NW LOG_STREAM(warn, log_network)
#define ERR_NW LOG_STREAM(err, log_network)

using boost::system::system_error;
using boost::system::error_code;

wesnothd_connection::wesnothd_connection(const std::string& host, const std::string& service)
	: io_service_()
	, resolver_(io_service_)
	, socket_(io_service_)
	, handshake_finished_(false)
	, read_buf_()
	, handshake_response_()
	, payload_size_(0)
	, bytes_to_write_(0)
	, bytes_written_(0)
	, bytes_to_read_(0)
	, bytes_read_(0)
{
	resolver_.async_resolve(
		boost::asio::ip::tcp::resolver::query(host, service),
		std::bind(&wesnothd_connection::handle_resolve, this, _1, _2)
	);
	LOG_NW << "Resolving hostname: " << host << '\n';
}

void wesnothd_connection::handle_resolve(const error_code& ec, resolver::iterator iterator)
{
	if (ec) {
		throw system_error(ec);
	}
	connect(iterator);
}

void wesnothd_connection::connect(resolver::iterator iterator)
{
	socket_.async_connect(*iterator, std::bind(
		&wesnothd_connection::handle_connect, this, _1, iterator)
	);
	LOG_NW << "Connecting to " << iterator->endpoint().address() << '\n';
}

void wesnothd_connection::handle_connect(
		const boost::system::error_code& ec,
		resolver::iterator iterator
		)
{
	if(ec) {
		WRN_NW << "Failed to connect to " <<
			iterator->endpoint().address() << ": " <<
			ec.message() << '\n';
		socket_.close();
		if(++iterator == resolver::iterator()) {
			ERR_NW << "Tried all IPs. Giving up" << std::endl;
			throw system_error(ec);
		}
		else {
			connect(iterator);
		}
	} else {
		LOG_NW << "Connected to " << iterator->endpoint().address() << '\n';
		handshake();
	}
}

void wesnothd_connection::handshake()
{
	static const uint32_t handshake = 0;
	boost::asio::async_write(socket_,
		boost::asio::buffer(reinterpret_cast<const char*>(&handshake), 4),
		[](const error_code& ec, std::size_t) { if (ec) throw system_error(ec); }
	);
	boost::asio::async_read(socket_,
		boost::asio::buffer(&handshake_response_.binary, 4),
		std::bind(&wesnothd_connection::handle_handshake, this, _1)
	);
}

void wesnothd_connection::handle_handshake(const error_code& ec)
{
	if (ec) {
		throw system_error(ec);
	}
	handshake_finished_ = true;
	recv();
}

void wesnothd_connection::send_data(const configr_of& request)
{
	poll();
	send_queue_.emplace_back();

	std::ostream os(&send_queue_.back());
	write_gz(os, request);
	if (send_queue_.size() == 1) {
		send();
	}
}

void wesnothd_connection::cancel()
{
	if(socket_.is_open()) {
		boost::system::error_code ec;
		socket_.cancel(ec);
		if(ec) {
			WRN_NW << "Failed to cancel network operations: " << ec.message() << std::endl;
		}
	}
}

std::size_t wesnothd_connection::is_write_complete(const boost::system::error_code& ec, size_t bytes_transferred)
{
	if(ec)
		throw system_error(ec);
	bytes_written_ = bytes_transferred;
	return bytes_to_write_ - bytes_transferred;
}

void wesnothd_connection::handle_write(
	const boost::system::error_code& ec,
	std::size_t bytes_transferred
	)
{
	DBG_NW << "Written " << bytes_transferred << " bytes.\n";
	send_queue_.pop_front();
	if (ec) {
		throw system_error(ec);
	}
	if (!send_queue_.empty()) {
		send();
	}
}

std::size_t wesnothd_connection::is_read_complete(
		const boost::system::error_code& ec,
		std::size_t bytes_transferred
		)
{
	if(ec) {
		throw system_error(ec);
	}
	bytes_read_ = bytes_transferred;
	if(bytes_transferred < 4) {
		return 4;
	} else {
		if(!bytes_to_read_) {
			std::istream is(&read_buf_);
			union { char binary[4]; uint32_t num; } data_size;
			is.read(data_size.binary, 4);
			bytes_to_read_ = ntohl(data_size.num) + 4;
			//Close immediately if we receive an invalid length
			if (bytes_to_read_ < 4)
				bytes_to_read_ = bytes_transferred;
		}
		return bytes_to_read_ - bytes_transferred;
	}
}

void wesnothd_connection::handle_read(
	const boost::system::error_code& ec,
	std::size_t bytes_transferred
	)
{
	DBG_NW << "Read " << bytes_transferred << " bytes.\n";
	bytes_to_read_ = 0;
	if(ec && ec != boost::asio::error::eof)
		throw system_error(ec);
	std::istream is(&read_buf_);
	recv_queue_.emplace_back();
	read_gz(recv_queue_.back(), is);
	DBG_NW << "Received " << recv_queue_.back() << " bytes.\n";

	recv();
}

void wesnothd_connection::send()
{
	auto& buf = send_queue_.front();
	size_t buf_size = buf.size();
	bytes_to_write_ = buf_size + 4;
	bytes_written_ = 0;
	payload_size_ = htonl(buf_size);

	boost::asio::streambuf::const_buffers_type gzipped_data = buf.data();
	std::deque<boost::asio::const_buffer> bufs(gzipped_data.begin(), gzipped_data.end());
	bufs.push_front(boost::asio::buffer(reinterpret_cast<const char*>(&payload_size_), 4));
	boost::asio::async_write(socket_, bufs,
		std::bind(&wesnothd_connection::is_write_complete, this, _1, _2),
		std::bind(&wesnothd_connection::handle_write, this, _1, _2)
	);
}

void wesnothd_connection::recv()
{
	boost::asio::async_read(socket_, read_buf_,
		std::bind(&wesnothd_connection::is_read_complete, this, _1, _2),
		std::bind(&wesnothd_connection::handle_read, this, _1, _2)
	);
}

std::size_t wesnothd_connection::poll()
{
	try {
		return io_service_.poll();
	}
	catch (const boost::system::system_error& err) {
		if(
			err.code() == boost::asio::error::operation_aborted ||
			err.code() == boost::asio::error::eof
		)
			return 1;
		throw error(err.code());
	}
}
bool wesnothd_connection::receive_data(config& result)
{
	poll();
	if (recv_queue_.empty()) {
		return false;
	}
	else {
		result.swap(recv_queue_.front());
		recv_queue_.pop_front();
		return true;
	}
}
