/* $Id$ */
/*
   Copyright (C) 2007
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "proxy.hpp"

#include <map>

namespace
{

typedef std::map<network::connection,network::connection> connection_map;
connection_map clients_to_servers, servers_to_clients;

network::connection find_peer(network::connection sock)
{
	const connection_map::const_iterator i = clients_to_servers.find(sock);
	if(i == clients_to_servers.end()) {
		const connection_map::const_iterator i = servers_to_clients.find(sock);
		if(i == servers_to_clients.end()) {
			return 0;
		} else {
			return i->second;
		}
	} else {
		return i->second;
	}
}

}

namespace proxy
{

void create_proxy(network::connection sock, const std::string& host, int port)
{
	const network::connection peer = network::connect(host,port);
	if(!peer) {
		network::disconnect(sock);
	} else {
		clients_to_servers.insert(std::pair<network::connection,network::connection>(sock,peer));
		servers_to_clients.insert(std::pair<network::connection,network::connection>(peer,sock));
	}
}

bool is_proxy(network::connection sock)
{
	return clients_to_servers.count(sock) || servers_to_clients.count(sock);
}

void disconnect(network::connection sock)
{
	const network::connection peer = find_peer(sock);
	if(!peer) {
		return;
	}

	servers_to_clients.erase(sock);
	servers_to_clients.erase(peer);
	clients_to_servers.erase(sock);
	clients_to_servers.erase(peer);

	network::disconnect(peer);
}

//! @todo remove gzip param after 1.3.12 is no longer allowed on the server.
void received_data(network::connection sock, const config& data, const bool send_gzipped)
{
	const network::connection peer = find_peer(sock);
	if(!peer) {
		return;
	}

	network::send_data(data, peer, send_gzipped);
}

}
