#ifndef NETWORK_HPP_INCLUDED
#define NETWORK_HPP_INCLUDED

#include "config.hpp"

#include "SDL_net.h"

#include <string>

//this module wraps the network interface.

namespace network {

//a network manager must be created before networking can be used.
//it must be destroyed only after all networking activity stops.
struct manager {
	manager();
	~manager();

private:
	bool free_;
};

//a server manager causes listening on a given port to occur
//for the duration of its lifetime.
struct server_manager {
	//if create_server is false, then the object has no effect.
	//throws error.
	server_manager(int port, bool create_server=true);
	~server_manager();

private:
	bool free_;
};

typedef TCPsocket connection;

//the number of peers we are connected to
size_t nconnections();

//if we are currently accepting connections
bool is_server();

//function to attempt to connect to a remote host. Returns
//the new connection on success, or 0 on failure.
//throws error.
connection connect(const std::string& host, int port=15000);

//function to accept a connection from a remote host. If no
//host is attempting to connect, it will return 0 immediately.
//otherwise returns the new connection.
//throws error.
connection accept_connection();

//function to disconnect from a certain host, or close all
//connections if connection_num is 0
void disconnect(connection connection_num=0);

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
connection receive_data(config& cfg, connection connection_num=0,
						int timeout=0);

//function to send data down a given connection, or broadcast
//to all peers if connection_num is 0. throws error.
void send_data(const config& cfg, connection connection_num=0);

//function to send data to all peers except 'connection_num'
void send_data_all_except(const config& cfg, connection connection_num);

//function to see the number of bytes being processed on the current socket
std::pair<int,int> current_transfer_stats();

struct error
{
	error(const std::string& msg, connection sock=0);
	std::string message;
	connection socket;

	void disconnect();
};

}

#endif
