/*
	Copyright (C) 2011 - 2024
	by Sergey Popov <loonycyborg@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

#ifdef INADDR_ANY
#undef INADDR_ANY
#endif

#ifdef INADDR_BROADCAST
#undef INADDR_BROADCAST
#endif

#ifdef INADDR_NONE
#undef INADDR_NONE
#endif

#endif

#include "exceptions.hpp"
#include "utils/variant.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/ssl.hpp>

class config;

namespace network_asio
{
struct error : public game::error
{
	error(const boost::system::error_code& error)
		: game::error(error.message())
	{
	}
};

/** A class that represents a TCP/IP connection. */
class connection
{
public:
	/**
	 * Constructor.
	 *
	 * @param host    Name of the host to connect to
	 * @param service Service identifier such as "80" or "http"
	 */
	connection(const std::string& host, const std::string& service);
	~connection();

	void transfer(const config& request, config& response);

	/** Handle all pending asynchronous events and return */
	std::size_t poll()
	{
		try {
			return io_context_.poll();
		} catch(const boost::system::system_error& err) {
			if(err.code() == boost::asio::error::operation_aborted) {
				return 1;
			}

			throw error(err.code());
		}
	}

	/**
	 * Run asio's event loop
	 *
	 * Handle asynchronous events blocking until all asynchronous operations have finished.
	 */
	void run()
	{
		io_context_.run();
	}

	void cancel();

	/** True if connected and no high-level operation is in progress */
	bool done() const
	{
		return done_;
	}

	/** True if connection is currently using TLS and thus is allowed to send cleartext passwords or auth tokens */
	bool using_tls() const
	{
		// Calling this function before connection is ready may return wrong result
		assert(done_);
		return utils::holds_alternative<tls_socket>(socket_);
	}

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

private:
	boost::asio::io_context io_context_;

	std::string host_;
	const std::string service_;
	typedef boost::asio::ip::tcp::resolver resolver;
	resolver resolver_;

	boost::asio::ssl::context tls_context_ { boost::asio::ssl::context::sslv23 };

	typedef std::unique_ptr<boost::asio::ip::tcp::socket> raw_socket;
	typedef std::unique_ptr<boost::asio::ssl::stream<raw_socket::element_type>> tls_socket;
	typedef utils::variant<raw_socket, tls_socket> any_socket;
	bool use_tls_;
	any_socket socket_;

	bool done_;

	std::unique_ptr<boost::asio::streambuf> write_buf_;
	std::unique_ptr<boost::asio::streambuf> read_buf_;

	using results_type = resolver::results_type;
	using endpoint = const boost::asio::ip::tcp::endpoint&;

	void handle_resolve(const boost::system::error_code& ec, const results_type& results);
	void handle_connect(const boost::system::error_code& ec, endpoint endpoint);

	void handshake();
	void handle_handshake(const boost::system::error_code& ec);

	uint32_t handshake_response_;

	void fallback_to_unencrypted();

	std::size_t is_write_complete(const boost::system::error_code& error, std::size_t bytes_transferred);
	void handle_write(const boost::system::error_code& ec, std::size_t bytes_transferred);

	std::size_t is_read_complete(const boost::system::error_code& error, std::size_t bytes_transferred);
	void handle_read(const boost::system::error_code& ec, std::size_t bytes_transferred, config& response);

	uint32_t payload_size_;

	std::size_t bytes_to_write_;
	std::size_t bytes_written_;
	std::size_t bytes_to_read_;
	std::size_t bytes_read_;
};
}
