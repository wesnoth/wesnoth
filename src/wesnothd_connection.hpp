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

#pragma once

#ifdef _WIN32

// MSVC compilation throws deprecation warnings on boost's use of gethostbyaddr and
// gethostbyname in socket_ops.ipp. This define silences that.
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#if _WIN32_WINNT < _WIN32_WINNT_VISTA
#define BOOST_ASIO_DISABLE_IOCP
#endif

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
#include "exceptions.hpp"
#include "wesnothd_connection_error.hpp"

#include <boost/asio.hpp>

#include <deque>
#include <list>
#include <mutex>

namespace boost
{
class thread;
}

class config;
class wesnothd_connection_ptr;
enum class loading_stage;

union data_union {
	char binary[4];
	uint32_t num;
};

/** A class that represents a TCP/IP connection to the wesnothd server. */
class wesnothd_connection : public std::enable_shared_from_this<wesnothd_connection>
{
public:
	using error = wesnothd_connection_error;

	wesnothd_connection(const wesnothd_connection&) = delete;
	wesnothd_connection& operator=(const wesnothd_connection&) = delete;

	~wesnothd_connection();

private:
	/**
	 * Constructor.
	 *
	 * May only be called via wesnothd_connection_ptr
	 *
	 * @param host    Name of the host to connect to
	 * @param service Service identifier such as "80" or "http"
	 */
	wesnothd_connection(const std::string& host, const std::string& service);

public:
	static wesnothd_connection_ptr create(const std::string& host, const std::string& service);

	bool fetch_data_with_loading_screen(config& cfg, loading_stage stage);

	void send_data(const configr_of& request);

	bool receive_data(config& result);

	/**
	 * Helper function that spins until data has been received.
	 * Should be used in tandem with the loading screen or other multi-threaded components.
	 */
	bool wait_and_receive_data(config& data);

	/** Handles all pending asynchornous events and returns. */
	std::size_t poll();

	void cancel();

	// Destroys this object.
	void stop();

	/** True if connected and no high-level operation is in progress */
	bool handshake_finished() const
	{
		return handshake_finished_;
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

	bool has_data_received() const
	{
		return !recv_queue_.empty();
	}

	bool is_sending_data() const
	{
		return !send_queue_.empty();
	}

private:
	std::unique_ptr<boost::thread> worker_thread_;

	boost::asio::io_service io_service_;

	typedef boost::asio::ip::tcp::resolver resolver;
	resolver resolver_;

	typedef boost::asio::ip::tcp::socket socket;
	socket socket_;

	boost::system::error_code last_error_;

	std::mutex last_error_mutex_;

	bool handshake_finished_;

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

	std::list<std::shared_ptr<boost::asio::streambuf>> send_queue_;
	std::list<config> recv_queue_;

	std::mutex recv_queue_mutex_;

	uint32_t payload_size_;

	// TODO: do i need to guard the following 4 values with a mutex?
	std::size_t bytes_to_write_;
	std::size_t bytes_written_;
	std::size_t bytes_to_read_;
	std::size_t bytes_read_;
};

/**
 * This class acts like a unique_ptr<wesnothd_connection>, wesnothd_connection objects may only be owned though this
 * pointer. The reason why we need this is that wesnothd_connection runs a workerthread so we use a shared_ptr to make
 * sure the wesnothd_connection isn't destroyed before the worker thread has finished. When this object is destroyed, it
 * calls wesnothd_connection::stop() which stops the worker thread which will then destroy the other
 * shared_ptr<wesnothd_connection> which destroys the wesnothd_connection object.
 */
class wesnothd_connection_ptr
{
private:
	friend class wesnothd_connection;

	wesnothd_connection_ptr(std::shared_ptr<wesnothd_connection>&& ptr)
		: ptr_(std::move(ptr))
	{
	}

public:
	wesnothd_connection_ptr() = default;

	wesnothd_connection_ptr(const wesnothd_connection_ptr&) = delete;
	wesnothd_connection_ptr& operator=(const wesnothd_connection_ptr&) = delete;

#if defined(_MSC_VER) && _MSC_VER == 1800
	wesnothd_connection_ptr(wesnothd_connection_ptr&& other)
		: ptr_(std::move(other.ptr_))
	{
	}

#else
	wesnothd_connection_ptr(wesnothd_connection_ptr&&) = default;
#endif
	wesnothd_connection_ptr& operator=(wesnothd_connection_ptr&&);

	~wesnothd_connection_ptr();

	explicit operator bool() const
	{
		return !!ptr_;
	}

	wesnothd_connection& operator*()
	{
		return *ptr_;
	}

	const wesnothd_connection& operator*() const
	{
		return *ptr_;
	}

	wesnothd_connection* operator->()
	{
		return ptr_.get();
	}

	const wesnothd_connection* operator->() const
	{
		return ptr_.get();
	}

	wesnothd_connection* get() const
	{
		return ptr_.get();
	}

private:
	std::shared_ptr<wesnothd_connection> ptr_;
};
