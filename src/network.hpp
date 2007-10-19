/* $Id$ */
/*
   Copyright (C) 2003-5 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef NETWORK_HPP_INCLUDED
#define NETWORK_HPP_INCLUDED

class config;

#include "SDL_net.h"

#include <string>

namespace threading
{
	class waiter;
}

//this module wraps the network interface.

namespace network {

//a network manager must be created before networking can be used.
//it must be destroyed only after all networking activity stops.
struct manager {
	explicit manager(size_t nthreads=1);
	~manager();

private:
	bool free_;

	manager(const manager&);
	void operator=(const manager&);
};

//a server manager causes listening on a given port to occur
//for the duration of its lifetime.
struct server_manager {

	//parameter to pass to the constructor.

	enum CREATE_SERVER { MUST_CREATE_SERVER, //will throw exception on failure
	                     TRY_CREATE_SERVER, //will swallow failure
	                     NO_SERVER }; //won't try to create a server at all

	//throws error.
	server_manager(int port, CREATE_SERVER create_server=MUST_CREATE_SERVER);
	~server_manager();

	bool is_running() const;

private:
	bool free_;
};

typedef int connection;

connection const null_connection = 0;

//the number of peers we are connected to
size_t nconnections();

//if we are currently accepting connections
bool is_server();

//function to attempt to connect to a remote host. Returns
//the new connection on success, or 0 on failure.
//throws error.
connection connect(const std::string& host, int port=15000);

connection connect(const std::string& host, int port, threading::waiter& waiter);

//function to accept a connection from a remote host. If no
//host is attempting to connect, it will return 0 immediately.
//otherwise returns the new connection.
//throws error.
connection accept_connection();

//function to disconnect from a certain host, or close all
//connections if connection_num is 0.
//returns true if the connection was disconnected.
//returns false on failure to disconnect, since the socket is
//in the middle of sending/receiving data. The socket will be closed when
//it has finished its send/receive
bool disconnect(connection connection_num=0);

//function to queue a disconnection. Next time receive_data is
//called, it will generate an error on the given connection.
//(and presumably then the handling of the error will include
// closing the connection)
void queue_disconnect(connection connection_num);

//function to receive data from either a certain connection, or
//all connections if connection_num is 0. Will store the data
//received in cfg. Times out after timeout milliseconds. Returns
//the connection that data was received from, or 0 if timeout
//occurred. Throws error if an error occurred.
connection receive_data(config& cfg, connection connection_num=0);
connection receive_data(config& cfg, connection connection_num, int timeout);

//function to send data down a given connection, or broadcast
//to all peers if connection_num is 0. throws error.
void send_data(const config& cfg, connection connection_num=0);

//! Function to queue data to be sent. (deprecated)
//! queue_data(cfg,sock) is equivalent to send_data(cfg,sock)
void queue_data(const config& cfg, connection connection_num=0);

//function to send any data that is in a connection's send_queue, up to a maximum
//of 'max_size' bytes -- or the entire send queue if 'max_size' bytes is 0
void process_send_queue(connection connection_num=0, size_t max_size=0);

//function to send data to all peers except 'connection_num'
void send_data_all_except(const config& cfg, connection connection_num);

//function to get the remote ip address of a socket
std::string ip_address(connection connection_num);

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

//function to see the number of bytes being processed on the current socket
statistics get_send_stats(connection handle);
statistics get_receive_stats(connection handle);

}


#endif
