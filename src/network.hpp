/* $Id$ */
/*
   Copyright (C) 2003 - 2007 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file network.hpp
//!

#ifndef NETWORK_HPP_INCLUDED
#define NETWORK_HPP_INCLUDED

class config;

#include "SDL_net.h"

#include <string>

namespace threading
{
	class waiter;
}

// This module wraps the network interface.

namespace network {

// A network manager must be created before networking can be used.
// It must be destroyed only after all networking activity stops.

// min_threads is the maximum number we allow to wait,
// if more threads attempt to wait, they will die.
// If min_threads == 0 no thread will ever be destroyed,
// and we will stay at the max number of threads ever needed.

// max_threads is the overall max number of helper threads.
// If we have that many threads already running, we will never create more.
// If max_threads == 0 we will always create a thread if we need it.
struct manager {
	explicit manager(size_t min_threads = 1,size_t max_threads = 0);
	~manager();

private:
	bool free_;

	manager(const manager&);
	void operator=(const manager&);
};

//! A server manager causes listening on a given port
//! to occur for the duration of its lifetime.
struct server_manager {

	//! Parameter to pass to the constructor.
	enum CREATE_SERVER { MUST_CREATE_SERVER,	//!< Will throw exception on failure
	                     TRY_CREATE_SERVER,	//!< Will swallow failure
	                     NO_SERVER };			//!< Won't try to create a server at all

	// Throws error.
	server_manager(int port, CREATE_SERVER create_server=MUST_CREATE_SERVER);
	~server_manager();

	bool is_running() const;

private:
	bool free_;
};

typedef int connection;

connection const null_connection = 0;

//! The number of peers we are connected to.
size_t nconnections();

//! If we are currently accepting connections.
bool is_server();

//! Function to attempt to connect to a remote host.
//! Returns the new connection on success, or 0 on failure.
//! Throws error.
connection connect(const std::string& host, int port=15000);

connection connect(const std::string& host, int port, threading::waiter& waiter);

//! Function to accept a connection from a remote host.
//! If no host is attempting to connect, it will return 0 immediately.
//! Otherwise returns the new connection.
//! Throws error.
connection accept_connection();

//! Function to disconnect from a certain host,
//! or close all connections if connection_num is 0.
//! Returns true if the connection was disconnected.
//! Returns false on failure to disconnect, since the socket is
//! in the middle of sending/receiving data.
//! The socket will be closed when it has finished its send/receive.
bool disconnect(connection connection_num=0, bool force=false);

//! Function to queue a disconnection.
//! Next time receive_data is called, it will generate an error
//! on the given connection (and presumably then the handling of the error
//! will include closing the connection).
void queue_disconnect(connection connection_num);

//! Function to receive data from either a certain connection,
//! or all connections if connection_num is 0.
//! Will store the data received in cfg.
//! Times out after timeout milliseconds.
//! Returns the connection that data was received from,
//! or 0 if timeout occurred.
//! Throws error if an error occurred.
connection receive_data(config& cfg, connection connection_num=0);
connection receive_data(config& cfg, connection connection_num, int timeout);

//! Function to send data down a given connection,
//! or broadcast to all peers if connection_num is 0.
//! Throws error.
void send_data(const config& cfg, connection connection_num /*= 0*/, const bool gzipped);

//! Function to send any data that is in a connection's send_queue,
//! up to a maximum of 'max_size' bytes --
//! or the entire send queue if 'max_size' bytes is 0.
void process_send_queue(connection connection_num=0, size_t max_size=0);

//! Function to send data to all peers except 'connection_num'.
void send_data_all_except(const config& cfg, connection connection_num, const bool gzipped);

//! Function to get the remote ip address of a socket.
std::string ip_address(connection connection_num);


//! Function to know the total number of threads and the number of idle threads.
std::pair<unsigned int,size_t> get_thread_state();

struct connection_stats
{
	connection_stats(int sent, int received, int connected_at);

	int bytes_sent, bytes_received;
	int time_connected;
};

connection_stats get_connection_stats(connection connection_num);

struct error
{
	error(const std::string& msg="", connection sock=0);
	std::string message;
	connection socket;

	void disconnect();
};

struct statistics
{
	statistics() : total(0), current(0), current_max(0) {}
	void fresh_current(size_t len)
	{
		current = 0;
		current_max = len;
	}
	void transfer(size_t size)
	{
		total += size;
		current += size;
	}
	bool operator==(const statistics& stats) const
	{
		return total == stats.total && current == stats.current && current_max == stats.current_max;
	}
	bool operator!=(const statistics& stats) const
	{
		return !operator==(stats);
	}
	size_t total;
	size_t current;
	size_t current_max;
};

//! Function to see the number of bytes being processed on the current socket.
statistics get_send_stats(connection handle);
statistics get_receive_stats(connection handle);

//! Amount of seconds after the last server ping when we assume to have timed out.
extern unsigned int ping_timeout;
} // network namespace


#endif
