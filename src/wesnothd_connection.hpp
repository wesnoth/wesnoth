/*
   Copyright (C) 2011 - 2016 by Sergey Popov <loonycyborg@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#ifdef _WIN32
# define BOOST_ASIO_DISABLE_IOCP
# ifdef INADDR_ANY
#  undef INADDR_ANY
# endif
# ifdef INADDR_BROADCAST
#  undef INADDR_BROADCAST
# endif
# ifdef INADDR_NONE
#  undef INADDR_NONE
# endif
#endif

#include <boost/asio.hpp>
#include <boost/optional.hpp>
#include <deque>
#include <list>
#include "exceptions.hpp"
#include "wesnothd_connection_error.hpp"
#include "configr_assign.hpp"
class config;

/** A class that represents a TCP/IP connection to the wesnothd server. */
class twesnothd_connection : boost::noncopyable
{
public:
	using error = wesnothd_connection_error;

	/**
	 * Constructor.
	 *
	 * @param host    Name of the host to connect to
	 * @param service Service identifier such as "80" or "http"
	 */
	twesnothd_connection(const std::string& host, const std::string& service);

	void send_data(const configr_of& request);

	bool receive_data(config& result);

	/** Handle all pending asynchonous events and return */
	std::size_t poll();
	/** Run asio's event loop
	 *
	 * Handle asynchronous events blocking until all asynchronous
	 * operations have finished
	 */

	void cancel();

	/** True if connected and no high-level operation is in progress */
	bool handshake_finished() const { return handshake_finished_; }

	std::size_t bytes_to_write() const
	{
		return bytes_to_write_;
	}
	std::size_t bytes_written() const
	{
		return bytes_written_;
	}
	std::size_t bytes_to_read() const
	{
		return bytes_to_read_;
	}
	std::size_t bytes_read() const
	{
		return bytes_read_;
	}
	bool has_data_received() const
	{
		return !recv_queue_.empty();
	}
	bool is_sending_data() const
	{
		return !send_queue_.empty();
	}
private:
	boost::asio::io_service io_service_;
	typedef boost::asio::ip::tcp::resolver resolver;
	resolver resolver_;

	typedef boost::asio::ip::tcp::socket socket;
	socket socket_;

	bool handshake_finished_;

	boost::asio::streambuf read_buf_;

	void handle_resolve(
		const boost::system::error_code& ec,
		resolver::iterator iterator
		);

	void connect(resolver::iterator iterator);
	void handle_connect(
		const boost::system::error_code& ec,
		resolver::iterator iterator
		);
	void handshake();
	void handle_handshake(
		const boost::system::error_code& ec
		);
	union {
		char binary[4];
		boost::uint32_t num;
	} handshake_response_;

	std::size_t is_write_complete(const boost::system::error_code& error, std::size_t bytes_transferred);
	void handle_write(const boost::system::error_code& ec, std::size_t bytes_transferred);
	std::size_t is_read_complete(const boost::system::error_code& error, std::size_t bytes_transferred);
	void handle_read(const boost::system::error_code& ec, std::size_t bytes_transferred);

	void send();
	void recv();

	std::list<boost::asio::streambuf> send_queue_;
	std::list<config> recv_queue_;

	boost::uint32_t payload_size_;
	std::size_t bytes_to_write_;
	std::size_t bytes_written_;
	std::size_t bytes_to_read_;
	std::size_t bytes_read_;

};
