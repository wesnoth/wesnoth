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

#include <boost/asio.hpp>

#include <condition_variable>
#include <deque>
#include <future>
#include <list>
#include <thread>
#include <mutex>
#include <queue>

class config;
enum class loading_stage;

union data_union {
	char binary[4];
	uint32_t num;
};

/** A class that represents a TCP/IP connection to the wesnothd server. */
class wesnothd_connection
{
public:
	using error = wesnothd_connection_error;

	wesnothd_connection(const wesnothd_connection&) = delete;
	wesnothd_connection& operator=(const wesnothd_connection&) = delete;

	~wesnothd_connection();

public:
	/**
	 * Constructor.
	 *
	 * @param host    Name of the host to connect to
	 * @param service Service identifier such as "80" or "http"
	 */
	wesnothd_connection(const std::string& host, const std::string& service);

	bool fetch_data_with_loading_screen(config& cfg, loading_stage stage);

	void send_data(const configr_of& request);

	bool receive_data(config& result);

	/**
	 * Helper function that spins until data has been received.
	 * Should be used in tandem with the loading screen or other multi-threaded components.
	 */
	bool wait_and_receive_data(config& data);

	void wait_for_handshake();

	void cancel();

	// Destroys this object.
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

	boost::asio::io_service io_service_;

	typedef boost::asio::ip::tcp::resolver resolver;
	resolver resolver_;

	typedef boost::asio::ip::tcp::socket socket;
	socket socket_;

	boost::system::error_code last_error_;

	std::mutex last_error_mutex_;

	std::promise<void> handshake_finished_;

	boost::asio::streambuf read_buf_;

	void handle_resolve(const boost::system::error_code& ec, resolver::iterator iterator);

	void connect(resolver::iterator iterator);
	void handle_connect(const boost::system::error_code& ec, resolver::iterator iterator);

	void handshake();
	void handle_handshake(const boost::system::error_code& ec);

	data_union handshake_response_;

	std::size_t is_write_complete(const boost::system::error_code& error, std::size_t bytes_transferred);
	void handle_write(const boost::system::error_code& ec, std::size_t bytes_transferred);

	std::size_t is_read_complete(const boost::system::error_code& error, std::size_t bytes_transferred);
	void handle_read(const boost::system::error_code& ec, std::size_t bytes_transferred);

	void send();
	void recv();

	template<typename T>
	using data_queue = std::queue<T, std::list<T>>;

	data_queue<std::shared_ptr<boost::asio::streambuf>> send_queue_;
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

using wesnothd_connection_ptr = std::unique_ptr<wesnothd_connection>;
