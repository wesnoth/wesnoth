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

#endif // endif _WIN32

#include "configr_assign.hpp"
#include "wesnothd_connection_error.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/ssl.hpp>

#include <condition_variable>
#include <future>
#include <list>
#include <mutex>
#include <queue>
#include <thread>


/** A class that represents a TCP/IP connection to the wesnothd server. */
class wesnothd_connection
{
public:
	using error = wesnothd_connection_error;

	wesnothd_connection(const wesnothd_connection&) = delete;
	wesnothd_connection& operator=(const wesnothd_connection&) = delete;

	~wesnothd_connection();

	/**
	 * Constructor.
	 *
	 * @param host        Name of the host to connect to
	 * @param service     Service identifier such as "80" or "http"
	 */
	wesnothd_connection(const std::string& host, const std::string& service);

	/**
	 * Queues the given data to be sent to the server.
	 *
	 * @param request     The data to send
	 */
	void send_data(const configr_of& request);

	/**
	 * Receives the next pending data pack from the server, if available.
	 *
	 * @param result      The object to which the received data will be written.
	 * @returns           True if any data was available, false otherwise.
	 */
	bool receive_data(config& result);

	/**
	 * Unlike @ref receive_data, waits until data is available instead of returning immediately.
	 *
	 * @param data        Config object passed to @ref receive_data
	 * @returns           True, since data will always be available.
	 */
	bool wait_and_receive_data(config& data);

	/** Waits until the server handshake is complete. */
	void wait_for_handshake();

	/** True if connection is currently using TLS and thus is allowed to send cleartext passwords or auth tokens */
	bool using_tls() const
	{
		return utils::holds_alternative<tls_socket>(socket_);
	}

	void cancel();

	void stop();

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
	std::thread worker_thread_;

	boost::asio::io_context io_context_;

	typedef boost::asio::ip::tcp::resolver resolver;
	resolver resolver_;

	boost::asio::ssl::context tls_context_;

	std::string host_;
	std::string service_;
	typedef std::unique_ptr<boost::asio::ip::tcp::socket> raw_socket;
	typedef std::unique_ptr<boost::asio::ssl::stream<raw_socket::element_type>> tls_socket;
	typedef utils::variant<raw_socket, tls_socket> any_socket;
	bool use_tls_;
	any_socket socket_;

	boost::system::error_code last_error_;

	std::mutex last_error_mutex_;

	std::promise<void> handshake_finished_;

	boost::asio::streambuf read_buf_;

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
	void handle_read(const boost::system::error_code& ec, std::size_t bytes_transferred);

	void send();
	void recv();

	void set_keepalive(int seconds);

	template<typename T>
	using data_queue = std::queue<T, std::list<T>>;

	data_queue<std::unique_ptr<boost::asio::streambuf>> send_queue_;
	data_queue<config> recv_queue_;

	std::mutex recv_queue_mutex_;

	std::condition_variable recv_queue_lock_;

	uint32_t payload_size_;

	// TODO: do i need to guard the following 4 values with a mutex?
	std::size_t bytes_to_write_;
	std::size_t bytes_written_;
	std::size_t bytes_to_read_;
	std::size_t bytes_read_;
};
