#ifndef NETWORK_HPP_INCLUDED
#define NETWORK_HPP_INCLUDED

#include "config.hpp"

#include "SDL_net.h"

#include <string>

namespace network {

struct manager {
	manager();
	~manager();
};

struct server_manager {
	server_manager(int port=15000, bool create_server=true);
	~server_manager();
};

typedef TCPsocket connection;

size_t nconnections();

connection connect(const std::string& host, int port=15000);
connection accept_connection();
void disconnect(connection connection_num=0);

connection receive_data(config& cfg, connection connection_num=0, int tout=0);

void send_data(config& cfg, connection connection_num=0);

struct error
{
	error(const std::string& msg) : message(msg) {}
	std::string message;
};

}

#endif
