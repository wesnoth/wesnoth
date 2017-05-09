/*
   Copyright (C) 2016 - 2017 by Sergey Popov <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

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

// MSVC compilation throws deprecation warnings on boost's use of gethostbyaddr and gethostbyname in socket_ops.ipp
#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#include <boost/asio.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/shared_array.hpp>

typedef std::shared_ptr<boost::asio::ip::tcp::socket> socket_ptr;

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

protected:
	unsigned short port_;
	bool keep_alive_;
	boost::asio::io_service io_service_;
	boost::asio::ip::tcp::acceptor acceptor_;
	void start_server();
	void serve();
	void accept_connection(const boost::system::error_code& error, socket_ptr socket);

	union {
		boost::uint32_t connection_num;
		char buf[4];
	} handshake_response_;
	void serverside_handshake(socket_ptr socket);
	void handle_handshake(const boost::system::error_code& error, socket_ptr socket, boost::shared_array<char> buf);

	virtual void handle_new_client(socket_ptr socket) = 0;

	virtual bool accepting_connections() const { return true; }
	virtual std::string is_ip_banned(const std::string&) const { return std::string(); }

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

std::string client_address(socket_ptr socket);
bool check_error(const boost::system::error_code& error, socket_ptr socket);

void async_send_error(socket_ptr socket, const std::string& msg, const char* error_code = "");
void async_send_warning(socket_ptr socket, const std::string& msg, const char* warning_code = "");
void async_send_message(socket_ptr socket, const std::string& msg);
