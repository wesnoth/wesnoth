#include "log.hpp"
#include "network.hpp"

#include "SDL_net.h"

#include <algorithm>
#include <cassert>
#include <vector>

namespace {

SDLNet_SocketSet socket_set = 0;
typedef std::vector<network::connection> sockets_list;
sockets_list sockets;

std::map<network::connection,std::string> received_data;

TCPsocket server_socket;

}

namespace network {

manager::manager()
{
	if(SDLNet_Init() == -1) {
		throw error(SDL_GetError());
	}

	socket_set = SDLNet_AllocSocketSet(64);
}

manager::~manager()
{
	disconnect();
	SDLNet_FreeSocketSet(socket_set);
	SDLNet_Quit();
}

server_manager::server_manager(int port, bool create_server)
{
	if(create_server) {
		server_socket = connect("",port);
		std::cerr << "server socket initialized: " << server_socket << "\n";
	}
}

server_manager::~server_manager()
{
	if(server_socket) {
		SDLNet_TCP_Close(server_socket);
		server_socket = 0;
	}
}

size_t nconnections()
{
	return sockets.size();
}

connection connect(const std::string& host, int port)
{
	char* const hostname = host.empty() ? NULL:const_cast<char*>(host.c_str());
	IPaddress ip;
	if(SDLNet_ResolveHost(&ip,hostname,port) == -1) {
		throw error(SDLNet_GetError());
	}

	TCPsocket sock = SDLNet_TCP_Open(&ip);
	if(!sock) {
		throw error(SDLNet_GetError());
	}

	//if this is not a server socket, then add it to the list
	//of sockets we listen to
	if(hostname != NULL) {
		const int res = SDLNet_TCP_AddSocket(socket_set,sock);
		if(res == -1) {
			SDLNet_TCP_Close(sock);
			throw network::error("Could not add socket to socket set");
		}

		assert(sock != server_socket);
		sockets.push_back(sock);
	}

	return sock;
}

connection accept_connection()
{
	assert(server_socket);
	const connection sock = SDLNet_TCP_Accept(server_socket);
	if(sock) {
		const int res = SDLNet_TCP_AddSocket(socket_set,sock);
		if(res == -1) {
			SDLNet_TCP_Close(sock);
			throw network::error("Could not add socket to socket set");
		}

		assert(sock != server_socket);
		sockets.push_back(sock);
		std::cerr << "new socket: " << sock << "\n";
		std::cerr << "server socket: " << server_socket << "\n";
	}

	return sock;
}

void disconnect(connection s)
{
	if(!s) {
		while(sockets.empty() == false) {
			disconnect(sockets.back());
		}

		return;
	}

	received_data.erase(s);

	const sockets_list::iterator i = std::find(sockets.begin(),sockets.end(),s);
	if(i != sockets.end()) {
		sockets.erase(i);
		SDLNet_TCP_DelSocket(socket_set,s);
		SDLNet_TCP_Close(s);
	}
}

connection receive_data(config& cfg, connection connection_num, int timeout)
{
	if(sockets.empty()) {
		return 0;
	}

	const int starting_ticks = SDL_GetTicks();

	const int res = SDLNet_CheckSockets(socket_set,timeout);
	if(res <= 0) {
		return 0;
	}

	for(sockets_list::const_iterator i = sockets.begin();
	    i != sockets.end(); ++i) {
		if(SDLNet_SocketReady(*i)) {
			std::string buffer;

			for(;;) {
				char c;
				const int len = SDLNet_TCP_Recv(*i,&c,1);
				if(len == 0) {
					break;
				} else if(len < 0) {
					throw error(std::string("error receiving data: ") +
					            SDLNet_GetError(),*i);
				}

				buffer.resize(buffer.size()+1);
				buffer[buffer.size()-1] = c;

				if(c == 0) {
					break;
				}
			}

			if(buffer == "") {
				throw error("error receiving data",*i);
			}

			std::cerr << "received: " << buffer << "\n";

			if(buffer[buffer.size()-1] != 0) {
				received_data[*i] += buffer;
				const int ticks_taken = SDL_GetTicks() - starting_ticks;
				if(ticks_taken < timeout) {
					return receive_data(cfg,connection_num,timeout-ticks_taken);
				} else {
					return 0;
				}
			} else {
				const std::map<connection,std::string>::iterator it =
				                                   received_data.find(*i);
				if(it != received_data.end()) {
					buffer = it->second + buffer;
					received_data.erase(it);
				}

				cfg.read(buffer);
				return *i;
			}
		}
	}

	assert(false);
	return 0;
}

void send_data(config& cfg, connection connection_num)
{
	log_scope("sending data");
	if(!connection_num) {
		std::cerr << "sockets: " << sockets.size() << "\n";
		for(sockets_list::const_iterator i = sockets.begin();
		    i != sockets.end(); ++i) {
			std::cerr << "server socket: " << server_socket << "\n";
			std::cerr << "current socket: " << *i << "\n";
			assert(*i && *i != server_socket);
			send_data(cfg,*i);
		}

		return;
	}

	std::cerr << "a\n";

	assert(connection_num != server_socket);

	std::string value = cfg.write();
	std::cerr << "sending " << (value.size()+1) << " bytes\n";
	const int res = SDLNet_TCP_Send(connection_num,
	                                const_cast<char*>(value.c_str()),
	                                value.size()+1);

	if(res < int(value.size()+1)) {
		std::cerr << "sending data failed: " << res << "/" << value.size() << ": " << SDL_GetError() << "\n";
		throw error("Could not send data over socket",connection_num);
	}
}

}
