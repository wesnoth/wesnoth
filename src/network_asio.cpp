/*
   Copyright (C) 2011 - 2018 by Sergey Popov <loonycyborg@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "network_asio.hpp"

#include "log.hpp"
#include "serialization/parser.hpp"
#include "utils/functional.hpp"

#include <cstdint>
#include <deque>

static lg::log_domain log_network("network");
#define DBG_NW LOG_STREAM(debug, log_network)
#define LOG_NW LOG_STREAM(info, log_network)
#define WRN_NW LOG_STREAM(warn, log_network)
#define ERR_NW LOG_STREAM(err, log_network)

namespace network_asio
{
using boost::system::system_error;

connection::connection(const std::string& host, const std::string& service)
	: io_service_()
	, resolver_(io_service_)
	, socket_(io_service_)
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
	resolver_.async_resolve(
		boost::asio::ip::tcp::resolver::query(host, service), std::bind(&connection::handle_resolve, this, _1, _2));

	LOG_NW << "Resolving hostname: " << host << '\n';
}

void connection::handle_resolve(const boost::system::error_code& ec, resolver::iterator iterator)
{
	if(ec) {
		throw system_error(ec);
	}

	connect(iterator);
}

void connection::connect(resolver::iterator iterator)
{
	socket_.async_connect(*iterator, std::bind(&connection::handle_connect, this, _1, iterator));
	LOG_NW << "Connecting to " << iterator->endpoint().address() << '\n';
}

void connection::handle_connect(const boost::system::error_code& ec, resolver::iterator iterator)
{
	if(ec) {
		WRN_NW << "Failed to connect to " << iterator->endpoint().address() << ": " << ec.message() << '\n';
		socket_.close();

		if(++iterator == resolver::iterator()) {
			ERR_NW << "Tried all IPs. Giving up" << std::endl;
			throw system_error(ec);
		} else {
			connect(iterator);
		}
	} else {
		LOG_NW << "Connected to " << iterator->endpoint().address() << '\n';
		handshake();
	}
}

void connection::handshake()
{
	static const uint32_t handshake = 0;

	boost::asio::async_write(socket_, boost::asio::buffer(reinterpret_cast<const char*>(&handshake), 4),
		std::bind(&connection::handle_write, this, _1, _2));

	boost::asio::async_read(socket_, boost::asio::buffer(&handshake_response_.binary, 4),
		std::bind(&connection::handle_handshake, this, _1));
}

void connection::handle_handshake(const boost::system::error_code& ec)
{
	if(ec) {
		throw system_error(ec);
	}

	done_ = true;
}

void connection::transfer(const config& request, config& response)
{
	io_service_.reset();
	done_ = false;

	write_buf_.reset(new boost::asio::streambuf);
	read_buf_.reset(new boost::asio::streambuf);
	std::ostream os(write_buf_.get());
	write_gz(os, request);

	bytes_to_write_ = write_buf_->size() + 4;
	bytes_written_ = 0;
	payload_size_ = htonl(bytes_to_write_ - 4);

	boost::asio::streambuf::const_buffers_type gzipped_data = write_buf_->data();
	std::deque<boost::asio::const_buffer> bufs(gzipped_data.begin(), gzipped_data.end());

	bufs.push_front(boost::asio::buffer(reinterpret_cast<const char*>(&payload_size_), 4));

	boost::asio::async_write(socket_, bufs, std::bind(&connection::is_write_complete, this, _1, _2),
		std::bind(&connection::handle_write, this, _1, _2));

	boost::asio::async_read(socket_, *read_buf_, std::bind(&connection::is_read_complete, this, _1, _2),
		std::bind(&connection::handle_read, this, _1, _2, std::ref(response)));
}

void connection::cancel()
{
	if(socket_.is_open()) {
		boost::system::error_code ec;

#ifdef _MSC_VER
// Silence warning about boost::asio::basic_socket<Protocol>::cancel always
// returning an error on XP, which we don't support anymore.
#pragma warning(push)
#pragma warning(disable:4996)
#endif
		socket_.cancel(ec);
#ifdef _MSC_VER
#pragma warning(pop)
#endif

		if(ec) {
			WRN_NW << "Failed to cancel network operations: " << ec.message() << std::endl;
		}
	}
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
