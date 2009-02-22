/* $Id$ */
/*
   Copyright (C) 2003 - 2009 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef NETWORK_WORKER_HPP_INCLUDED
#define NETWORK_WORKER_HPP_INCLUDED

// Check if sendfile function is implemented by glibc
// It is linux only extension
#if defined(__linux__) && defined(__GNUC_PREREQ)
#if __GNUC_PREREQ(2,1)
#define USE_SENDFILE
#endif
#endif


#include <map>
#include <vector>
#include "config.hpp"
#include "network.hpp"
#include "SDL_net.h"

namespace network_worker_pool
{

struct manager
{
	explicit manager(size_t min_threads,size_t max_threads);
	~manager();

private:
	manager(const manager&);
	void operator=(const manager&);

	bool active_;
};

network::pending_statistics get_pending_stats();

void set_raw_data_only();
void set_use_system_sendfile(bool);

/** Function to asynchronously received data to the given socket. */
void receive_data(TCPsocket sock);

TCPsocket get_received_data(TCPsocket sock, config& cfg, bool* gzipped, network::bandwidth_in_ptr&);

TCPsocket get_received_data(std::vector<char>& buf);

void queue_file(TCPsocket sock, const std::string&);

void queue_raw_data(TCPsocket sock, const char* buf, int len);
size_t queue_data(TCPsocket sock, const config& buf, const bool gzipped, const std::string& packet_type);
bool is_locked(const TCPsocket sock);
bool close_socket(TCPsocket sock);
TCPsocket detect_error();

std::pair<network::statistics,network::statistics> get_current_transfer_stats(TCPsocket sock);

}

#endif
