#ifndef NETWORK_HPP_INCLUDED
#define NETWORK_HPP_INCLUDED

#include "config.hpp"

#include "SDL_net.h"

#include <string>

namespace network {

struct manager {
	manager();
	~manager();

private:
	bool free_;
};

struct server_manager {
	server_manager(int port=15000, bool create_server=true);
	~server_manager();

private:
	bool free_;
};

typedef TCPsocket connection;

size_t nconnections();
bool is_server();

connection connect(const std::string& host, int port=15000);
connection accept_connection();
void disconnect(connection connection_num=0);
void queue_disconnect(connection connection_num);

connection receive_data(config& cfg, connection connection_num=0, int tout=0);

void send_data(const config& cfg, connection connection_num=0);

struct error
{
	error(const std::string& msg, connection sock=0)
	                                 : message(msg), socket(sock) {}
	std::string message;
	connection socket;

	void disconnect() { if(socket) { network::disconnect(socket); } }
};

}

#endif
