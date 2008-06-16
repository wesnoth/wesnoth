/* $Id$ */
/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file network.cpp
//! Networking

#include "global.hpp"

#include "SDL.h"

#include "serialization/binary_wml.hpp"
#include "config.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "network_new.hpp"
#include "network_boost.hpp"


#include <algorithm>
#include <cassert>
#include <cerrno>
#include <queue>
#include <iostream>
#include <set>
#include <vector>
#include <ctime>

#include <signal.h>
#include <string.h>
#if defined(_WIN32) || defined(__WIN32__) || defined (WIN32)
#undef INADDR_ANY
#undef INADDR_BROADCAST
#undef INADDR_NONE
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>  // for TCP_NODELAY
#ifdef __BEOS__
#include <socket.h>
#else
#include <fcntl.h>
#endif
#define SOCKET int
#endif

#define DBG_NW LOG_STREAM(debug, network)
#define LOG_NW LOG_STREAM(info, network)
#define WRN_NW LOG_STREAM(warn, network)
#define ERR_NW LOG_STREAM(err, network)
// Only warnings and not errors to avoid DoS by log flooding



namespace {

} // end anon namespace

namespace network {

network_boost::connection::statistics get_connection_stats(connection connection_num)
{
	return network_boost::manager::get_manager()->get_connection(connection_num)->get_statistics();
}

network_boost::manager* manager::boost_manager_ = 0;

manager::manager(size_t /*min_threads*/, size_t /*max_threads*/) : free_(true)
{
	// If the network is already being managed
	if(!boost_manager_) {
		free_ = false;
		return;
	}

	boost_manager_ = new network_boost::manager();

}

manager::~manager()
{
	if(free_) {
		delete boost_manager_;
		boost_manager_ = 0;
	}
}

void set_raw_data_only()
{
}

network_boost::connection_id server_connection;

server_manager::server_manager(int port, CREATE_SERVER create_server) : free_(false)
{
	if(create_server != NO_SERVER) {
		server_connection = network_boost::manager::get_manager()->listen(port)->get_id();
		free_ = true;
	}
}

server_manager::~server_manager()
{
	stop();
}

void server_manager::stop()
{
	if(free_) {
		network_boost::manager::get_manager()->get_connection(server_connection)->disconnect();
		free_ = false;
	}
}

bool server_manager::is_running() const
{
	return !free_;
}

size_t nconnections()
{
	return network_boost::manager::get_manager()->nconnections();
}


connection connect(const std::string& host, int port)
{
	return network_boost::manager::get_manager()->connect(host,port, network_boost::connection::GZIP)->get_id();
}

connection connect(const std::string& host, int port, threading::waiter& /*waiter*/)
{
	return connect(host, port);
}

connection accept_connection()
{
	network_boost::connection_ptr new_con;
	if (network_boost::manager::get_manager()->accept(new_con))
	{
		return new_con->get_id();
	}
	return 0;
}

bool disconnect(connection s, bool force)
{
	if(s == 0) {
		network_boost::manager::get_manager()->disconnect_all();
	}
	else
	{
		network_boost::manager::get_manager()->get_connection(s)->disconnect(force);
	}

	return true;
}

void queue_disconnect(network::connection sock)
{
	network_boost::manager::get_manager()->get_connection(sock)->disconnect();
}


connection receive_data(config& cfg, connection /*connection_num*/, bool*)
{
	network_boost::connection_ptr connection;
	network_boost::buffer_ptr buffer;
	if (network_boost::manager::get_manager()->receive_data(connection, buffer))
	{
		buffer->get_config(cfg);
		return connection->get_id();
	}
	return 0;
}

connection receive_data(std::vector<char>& buf)
{
	network_boost::connection_ptr connection;
	network_boost::buffer_ptr buffer;
	if (network_boost::manager::get_manager()->receive_data(connection, buffer))
	{
		buffer->get_raw_buffer(buf);
		return connection->get_id();
	}
	return 0;
}

void send_file(const std::string& filename, connection connection_num)
{
	network_boost::buffer_ptr buffer(new network_boost::file_buffer(filename));
	network_boost::manager::get_manager()->get_connection(connection_num)->send_buffer(buffer);
}

//! @todo Note the gzipped parameter should be removed later, we want to send
//! all data gzipped. This can be done once the campaign server is also updated
//! to work with gzipped data.
void send_data(const config& cfg, connection connection_num, const bool /*gzipped*/)
{
	network_boost::buffer_ptr buffer(new network_boost::config_buffer(cfg));
	network_boost::manager::get_manager()->get_connection(connection_num)->send_buffer(buffer);
}

void process_send_queue(connection, size_t)
{
	network_boost::manager::get_manager()->handle_network();
}

std::string ip_address(connection connection_num)
{
	return network_boost::manager::get_manager()->get_connection(connection_num)->get_ip_address();
}

} // end namespace network
