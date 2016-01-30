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

#ifndef NETWORK_ASIO_HPP_INCLUDED
#define NETWORK_ASIO_HPP_INCLUDED

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
#include "exceptions.hpp"
#include "config.hpp"

namespace network_asio {

struct error : public game::error
{
	error(const boost::system::error_code& error) : game::error(error.message()) {}
};

/** A class that represents a TCP/IP connection. */
class connection
{
	boost::asio::io_service io_service_;
	typedef boost::asio::ip::tcp::resolver resolver;
	resolver resolver_;

	typedef boost::asio::ip::tcp::socket socket;
	socket socket_;

	bool done_;

	boost::asio::streambuf write_buf_;
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

	std::size_t is_write_complete(
		const boost::system::error_code& error,
		std::size_t bytes_transferred
		);
	void handle_write(
		const boost::system::error_code& ec,
		std::size_t bytes_transferred
		);
	std::size_t is_read_complete(
		const boost::system::error_code& error,
		std::size_t bytes_transferred
		);
	void handle_read(
		const boost::system::error_code& ec,
		std::size_t bytes_transferred,
		config& response
		);
	boost::uint32_t payload_size_;
	std::size_t bytes_to_write_;
	std::size_t bytes_written_;
	std::size_t bytes_to_read_;
	std::size_t bytes_read_;

	public:
	/**
	 * Constructor.
	 *
	 * @param host    Name of the host to connect to
	 * @param service Service identifier such as "80" or "http"
	 */
	connection(const std::string& host, const std::string& service);

	void transfer(const config& request, config& response);

	/** Handle all pending asynchonous events and return */
	std::size_t poll()
	{
		try {
			return io_service_.poll();
		} catch(const boost::system::system_error& err) {
			if(err.code() == boost::asio::error::operation_aborted)
				return 1;
			throw error(err.code());
		}
	}
	/** Run asio's event loop
	 *
	 * Handle asynchronous events blocking until all asynchronous
	 * operations have finished
	 */
	void run() { io_service_.run(); }

	void cancel();

	/** True if connected and no high-level operation is in progress */
	bool done() const { return done_; }

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
};

}

#endif
