#include "log.hpp"
#include "network.hpp"
#include "util.hpp"

#include "SDL_net.h"

#include <algorithm>
#include <cassert>
#include <queue>
#include <set>
#include <vector>

namespace {

SDLNet_SocketSet socket_set = 0;
typedef std::vector<network::connection> sockets_list;
sockets_list sockets;

struct partial_buffer {
	partial_buffer() : upto(0) {}
	std::vector<char> buf;
	size_t upto;
};

typedef std::map<network::connection,partial_buffer> partial_map;
partial_map received_data;
partial_map::const_iterator current_connection = received_data.end();

TCPsocket server_socket;

std::queue<network::connection> disconnection_queue;
std::set<network::connection> bad_sockets;

}

namespace network {

error::error(const std::string& msg, connection sock) : message(msg), socket(sock)
{
	bad_sockets.insert(socket);
}

void error::disconnect()
{
	if(socket) {
		network::disconnect(socket);
	}
}

manager::manager() : free_(true)
{
	//if the network is already being managed
	if(socket_set) {
		free_ = false;
		return;
	}

	if(SDLNet_Init() == -1) {
		throw error(SDL_GetError());
	}

	socket_set = SDLNet_AllocSocketSet(64);
}

manager::~manager()
{
	if(free_) {
		disconnect();
		SDLNet_FreeSocketSet(socket_set);
		socket_set = 0;
		SDLNet_Quit();
	}
}

server_manager::server_manager(int port, bool create_server)
                                    : free_(false)
{
	if(create_server && !server_socket) {
		server_socket = connect("",port);
		std::cerr << "server socket initialized: " << server_socket << "\n";
		free_ = true;
	}
}

server_manager::~server_manager()
{
	if(free_) {
		SDLNet_TCP_Close(server_socket);
		server_socket = 0;
	}
}

size_t nconnections()
{
	return sockets.size();
}

bool is_server()
{
	return server_socket != 0;
}

connection connect(const std::string& host, int port)
{
	char* const hostname = host.empty() ? NULL:const_cast<char*>(host.c_str());
	IPaddress ip;
	if(SDLNet_ResolveHost(&ip,hostname,port) == -1) {
		throw error(SDLNet_GetError());
	}

	std::cerr << "opening connection\n";
	TCPsocket sock = SDLNet_TCP_Open(&ip);
	std::cerr << "opened connection okay\n";
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
	if(!server_socket)
		return 0;

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

	bad_sockets.erase(s);
	received_data.erase(s);
	current_connection = received_data.end();

	const sockets_list::iterator i = std::find(sockets.begin(),sockets.end(),s);
	if(i != sockets.end()) {
		sockets.erase(i);
		SDLNet_TCP_DelSocket(socket_set,s);
		SDLNet_TCP_Close(s);
	} else {
		std::cerr << "Could not find socket to close: " << (int)s << "\n";
		if(sockets.size() == 1) {
			std::cerr << "valid socket: " << (int)*sockets.begin() << "\n";
		}
	}
}

void queue_disconnect(network::connection sock)
{
	disconnection_queue.push(sock);
}

connection receive_data(config& cfg, connection connection_num, int timeout)
{
	if(disconnection_queue.empty() == false) {
		const network::connection sock = disconnection_queue.front();
		disconnection_queue.pop();
		throw error("",sock);
	}

	if(bad_sockets.count(connection_num) || bad_sockets.count(0))
		return 0;

	if(sockets.empty()) {
		return 0;
	}

	const int starting_ticks = SDL_GetTicks();

	const int res = SDLNet_CheckSockets(socket_set,timeout);
	if(res <= 0) {
		return 0;
	}

	for(sockets_list::const_iterator i = sockets.begin(); i != sockets.end(); ++i) {
		if(SDLNet_SocketReady(*i)) {
			std::map<connection,partial_buffer>::iterator part_received = received_data.find(*i);
			if(part_received == received_data.end()) {
				char num_buf[4];
				size_t len = SDLNet_TCP_Recv(*i,num_buf,4);

				if(len != 4) {
					throw error("Remote host disconnected",*i);
				}

				len = SDLNet_Read32(num_buf);

				std::cerr << "received packet length: " << len << "\n";

				if(len > 10000000) {
					throw error(std::string("network error: bad length data"),*i);
				}

				part_received = received_data.insert(std::pair<connection,partial_buffer>(*i,partial_buffer())).first;
				part_received->second.buf.resize(len);
			}

			current_connection = part_received;
			partial_buffer& buf = part_received->second;

			const size_t expected = buf.buf.size() - buf.upto;
			const size_t nbytes = SDLNet_TCP_Recv(*i,&buf.buf[buf.upto],expected);
			if(nbytes > expected) {
				std::cerr << "received " << nbytes << "/" << expected << "\n";
				throw error(std::string("network error: received wrong number of bytes: ") + SDLNet_GetError(),*i);
			}

			buf.upto += nbytes;
			std::cerr << "received " << nbytes << "=" << buf.upto << "/" << buf.buf.size() << "\n";

			if(buf.upto == buf.buf.size()) {
				current_connection = received_data.end();
				const std::string buffer(buf.buf.begin(),buf.buf.end());
				received_data.erase(part_received); //invalidates buf. don't use again
				if(buffer == "") {
					std::cerr << "buffer from remote host is empty\n";
					throw error("remote host closed connection",*i);
				}

				if(buffer[buffer.size()-1] != 0) {
					std::cerr << "buf not nul-delimited. Network error\n";
					throw network::error("sanity check on incoming data failed",*i);
				}

				cfg.read(buffer);
				return *i;
			}
		}
	}

	const int time_taken = SDL_GetTicks() - starting_ticks;
	const int time_left = maximum<int>(0,timeout - time_taken);

	return receive_data(cfg,connection_num,time_left);
}

void send_data(const config& cfg, connection connection_num)
{
	if(bad_sockets.count(connection_num) || bad_sockets.count(0))
		return;

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

	assert(connection_num != server_socket);

	std::string value(4,'x');
	value += cfg.write();

	char buf[4];
	SDLNet_Write32(value.size()+1-4,buf);
	std::copy(buf,buf+4,value.begin());

	std::cerr << "sending " << (value.size()+1) << " bytes\n";
	const int res = SDLNet_TCP_Send(connection_num,
	                                const_cast<char*>(value.c_str()),
	                                value.size()+1);

	if(res < int(value.size()+1)) {
		std::cerr << "sending data failed: " << res << "/" << value.size() << "\n";
		throw error("Could not send data over socket",connection_num);
	}
}

void send_data_all_except(const config& cfg, connection connection_num)
{
	for(sockets_list::const_iterator i = sockets.begin(); i != sockets.end(); ++i) {
		if(*i == connection_num)
			continue;

		assert(*i && *i != server_socket);
		send_data(cfg,*i);
	}
}

std::pair<int,int> current_transfer_stats()
{
	if(current_connection == received_data.end())
		return std::pair<int,int>(-1,-1);
	else
		return std::pair<int,int>(current_connection->second.upto,current_connection->second.buf.size());
}

} //end namespace network
