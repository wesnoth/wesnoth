/*
   Copyright (C) 2016 - 2018 by Sergey Popov <dave@whitevine.net>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License 2
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Base class for servers using Wesnoth's WML over TCP protocol.
 */

#pragma once

#include "exceptions.hpp"
#include "server/common/simple_wml.hpp"

#ifdef _WIN32
#include "serialization/unicode_cast.hpp"
#endif

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#ifndef _WIN32
#include <boost/asio/posix/stream_descriptor.hpp>
#endif
#include <boost/asio/signal_set.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/shared_array.hpp>
#include <utils/variant.hpp>

#include <map>

extern bool dump_wml;

class config;

typedef std::shared_ptr<boost::asio::ip::tcp::socket> socket_ptr;
typedef std::shared_ptr<boost::asio::ssl::stream<socket_ptr::element_type>> tls_socket_ptr;
typedef utils::variant<socket_ptr, tls_socket_ptr> any_socket_ptr;

struct server_shutdown : public game::error
{
	server_shutdown(const std::string& msg) : game::error(msg) {}
};

class server_base
{
public:
	server_base(unsigned short port, bool keep_alive);
	virtual ~server_base() {}
	void run();

	/**
	 * Send a WML document from within a coroutine
	 * @param socket
	 * @param doc
	 * @param yield The function will suspend on write operation using this yield context
	 */
	template<class SocketPtr> void coro_send_doc(SocketPtr socket, simple_wml::document& doc, boost::asio::yield_context yield);
	/**
	 * Send contents of entire file directly to socket from within a coroutine
	 * @param socket
	 * @param filename
	 * @param yield The function will suspend on write operations using this yield context
	 */
	template<class SocketPtr> void coro_send_file(SocketPtr socket, const std::string& filename, boost::asio::yield_context yield);
	/**
	 * Receive WML document from a coroutine
	 * @param socket
	 * @param yield The function will suspend on read operation using this yield context
	 */
	template<class SocketPtr> std::unique_ptr<simple_wml::document> coro_receive_doc(SocketPtr socket, boost::asio::yield_context yield);

	/**
	 * High level wrapper for sending a WML document
	 *
	 * This function returns before send is finished. This function can be called again on same socket before previous send was finished.
	 * WML documents are kept in internal queue and sent in FIFO order.
	 * @param socket
	 * @param doc Document to send. A copy of it will be made so there is no need to keep the reference live after the function returns.
	 */
	template<class SocketPtr> void async_send_doc_queued(SocketPtr socket, simple_wml::document& doc);

	typedef std::map<std::string, std::string> info_table;
	template<class SocketPtr> void async_send_error(SocketPtr socket, const std::string& msg, const char* error_code = "", const info_table& info = {});
	template<class SocketPtr> void async_send_warning(SocketPtr socket, const std::string& msg, const char* warning_code = "", const info_table& info = {});

protected:
	unsigned short port_;
	bool keep_alive_;
	boost::asio::io_service io_service_;
	boost::asio::ssl::context tls_context_ { boost::asio::ssl::context::sslv23 };
	bool tls_enabled_ { false };
	boost::asio::ip::tcp::acceptor acceptor_v6_;
	boost::asio::ip::tcp::acceptor acceptor_v4_;

	void load_tls_config(const config& cfg);

	void start_server();
	void serve(boost::asio::yield_context yield, boost::asio::ip::tcp::acceptor& acceptor, boost::asio::ip::tcp::endpoint endpoint);

	union {
		uint32_t connection_num;
		char buf[4];
	} handshake_response_;

	virtual void handle_new_client(socket_ptr socket) = 0;
	virtual void handle_new_client(tls_socket_ptr socket) = 0;

	virtual bool accepting_connections() const { return true; }
	virtual std::string is_ip_banned(const std::string&) { return std::string(); }
	virtual bool ip_exceeds_connection_limit(const std::string&) const { return false; }

#ifndef _WIN32
	boost::asio::posix::stream_descriptor input_;
	std::string fifo_path_;
	void read_from_fifo();
	virtual void handle_read_from_fifo(const boost::system::error_code& error, std::size_t bytes_transferred) = 0;
	boost::asio::streambuf admin_cmd_;

	boost::asio::signal_set sighup_;
	virtual void handle_sighup(const boost::system::error_code& error, int signal_number) = 0;
#endif
	boost::asio::signal_set sigs_;
	void handle_termination(const boost::system::error_code& error, int signal_number);
};

template<class SocketPtr> std::string client_address(SocketPtr socket);
template<class SocketPtr> bool check_error(const boost::system::error_code& error, SocketPtr socket);
