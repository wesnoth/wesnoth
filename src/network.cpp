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

struct schema_pair
{
	compression_schema incoming, outgoing;
};

typedef std::map<network::connection,schema_pair> schema_map;

schema_map schemas;

struct partial_buffer {
	partial_buffer() : upto(0) {}
	std::vector<char> buf;
	size_t upto;
};

typedef std::map<network::connection,partial_buffer> partial_map;
partial_map received_data;
partial_map::const_iterator current_connection = received_data.end();

typedef std::multimap<network::connection,partial_buffer> send_queue_map;
send_queue_map send_queue;

TCPsocket server_socket;

std::deque<network::connection> disconnection_queue;
std::set<network::connection> bad_sockets;

}

namespace network {

error::error(const std::string& msg, connection sock) : message(msg), socket(sock)
{
	if(socket) {
		bad_sockets.insert(socket);
	}
}

void error::disconnect()
{
	network::disconnect(socket);
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
		throw error("Could not connect to host");
	}

	TCPsocket sock = SDLNet_TCP_Open(&ip);
	if(!sock) {
		throw error("Could not connect to host");
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
		schemas.insert(std::pair<network::connection,schema_pair>(sock,schema_pair()));
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
		schemas.insert(std::pair<network::connection,schema_pair>(sock,schema_pair()));
	}

	return sock;
}

void disconnect(connection s)
{
	if(s == 0) {
		while(sockets.empty() == false) {
			assert(sockets.back() != 0);
			disconnect(sockets.back());
		}		

		return;
	}

	schemas.erase(s);
	bad_sockets.erase(s);
	received_data.erase(s);
	current_connection = received_data.end();

	std::deque<network::connection>::iterator dqi = std::find(disconnection_queue.begin(),disconnection_queue.end(),s);
	if(dqi != disconnection_queue.end()) {
		disconnection_queue.erase(dqi);
	}

	const sockets_list::iterator i = std::find(sockets.begin(),sockets.end(),s);
	if(i != sockets.end()) {
		sockets.erase(i);
		SDLNet_TCP_DelSocket(socket_set,s);
		SDLNet_TCP_Close(s);
	} else {
		if(sockets.size() == 1) {
			std::cerr << "valid socket: " << (int)*sockets.begin() << "\n";
		}
	}
}

void queue_disconnect(network::connection sock)
{
	disconnection_queue.push_back(sock);
}

connection receive_data(config& cfg, connection connection_num, int timeout)
{
	if(disconnection_queue.empty() == false) {
		const network::connection sock = disconnection_queue.front();
		disconnection_queue.pop_front();
		throw error("",sock);
	}

	if(bad_sockets.count(connection_num) || bad_sockets.count(0)) {
		return 0;
	}

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
					std::cerr << "bad length in network packet. Throwing error\n";
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

				const schema_map::iterator schema = schemas.find(*i);
				assert(schema != schemas.end());

				cfg.read_compressed(buffer,schema->second.incoming);

				std::cerr << "--- RECEIVED DATA from " << ((int)*i) << ": '"
				          << cfg.write() << "'\n--- END RECEIVED DATA\n";

				
				return *i;
			}
		}
	}

	const int time_taken = SDL_GetTicks() - starting_ticks;
	const int time_left = maximum<int>(0,timeout - time_taken);

	return receive_data(cfg,connection_num,time_left);
}

namespace {
	size_t default_max_send_size = 0;
}

void set_default_send_size(size_t max_size)
{
	default_max_send_size = max_size;
}

void send_data(const config& cfg, connection connection_num, size_t max_size)
{
	if(bad_sockets.count(connection_num) || bad_sockets.count(0))
		return;

	if(max_size == 0) {
		max_size = default_max_send_size;
	}

	if(max_size > 0 && max_size < 8) {
		max_size = 8;
	}

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

	const schema_map::iterator schema = schemas.find(connection_num);
	assert(schema != schemas.end());

	std::string value(4,'x');
	value += cfg.write_compressed(schema->second.outgoing);

	std::cerr << "--- SEND DATA to " << ((int)connection_num) << ": '"
	          << cfg.write() << "'\n--- END SEND DATA\n";

	char buf[4];
	SDLNet_Write32(value.size()+1-4,buf);
	std::copy(buf,buf+4,value.begin());

	//if the data is less than our maximum chunk, and there is no data queued to send
	//to this host, then send all data now
	if((max_size == 0 || value.size()+1 <= max_size) && send_queue.count(connection_num) == 0) {
		std::cerr << "sending " << (value.size()+1) << " bytes\n";
		const int res = SDLNet_TCP_Send(connection_num,
		                                const_cast<char*>(value.c_str()),
		                                value.size()+1);

		if(res < int(value.size()+1)) {
			std::cerr << "sending data failed: " << res << "/" << value.size() << "\n";
			throw error("Could not send data over socket",connection_num);
		}
	} else {
		std::cerr << "cannot send all " << (value.size()+1) << " bytes at once. Placing in send queue.\n";
		//place the data in the send queue
		const send_queue_map::iterator itor =
		     send_queue.insert(std::pair<network::connection,partial_buffer>(connection_num,partial_buffer()));

		itor->second.buf.resize(value.size()+1);
		std::copy(value.begin(),value.end(),itor->second.buf.begin());
		itor->second.buf.back() = 0;

		process_send_queue(connection_num,max_size);
	}
}

void process_send_queue(connection connection_num, size_t max_size)
{
	if(connection_num == 0) {
		for(sockets_list::iterator i = sockets.begin(); i != sockets.end(); ++i) {
			process_send_queue(*i,max_size);
		}

		return;
	}

	if(max_size == 0) {
		max_size = default_max_send_size;
	}
	
	if(max_size != 0 && max_size < 8) {
		max_size = 8;
	}

	std::pair<send_queue_map::iterator,send_queue_map::iterator> itor = send_queue.equal_range(connection_num);
	if(itor.first != itor.second) {
		std::vector<char>& buf = itor.first->second.buf;
		size_t& upto = itor.first->second.upto;

		size_t bytes_to_send = buf.size() - upto;
		if(max_size != 0 && bytes_to_send > max_size) {
			bytes_to_send = max_size;
		}

		std::cerr << "sending " << bytes_to_send << " from send queue\n";

		const int res = SDLNet_TCP_Send(connection_num,&buf[upto],bytes_to_send);
		if(res < int(bytes_to_send)) {
			std::cerr << "sending data failed: " << res << "/" << bytes_to_send << "\n";
			throw error("Sending queued data failed",connection_num);
		}

		upto += bytes_to_send;

		//if we've now sent the entire item, erase it from the send queue
		if(upto == buf.size()) {
			std::cerr << "erasing item from the send queue\n";
			send_queue.erase(itor.first);
		}

		//if we haven't sent 'max_size' bytes yet, try to go onto the next item in
		//the queue by recursing
		if(bytes_to_send < max_size || max_size == 0) {
			process_send_queue(connection_num,max_size-bytes_to_send);
		}
	}
}

void send_data_all_except(const config& cfg, connection connection_num, size_t max_size)
{
	for(sockets_list::const_iterator i = sockets.begin(); i != sockets.end(); ++i) {
		if(*i == connection_num)
			continue;

		assert(*i && *i != server_socket);
		send_data(cfg,*i,max_size);
	}
}

std::string ip_address(connection connection_num)
{
	std::stringstream str;
	const IPaddress* const ip = SDLNet_TCP_GetPeerAddress(connection_num);
	if(ip != NULL) {
		const unsigned char* buf = reinterpret_cast<const unsigned char*>(&ip->host);
		for(int i = 0; i != sizeof(ip->host); ++i) {
			str << int(buf[i]);
			if(i+1 != sizeof(ip->host)) {
				str << '.';
			}
		}

	}

	return str.str();
}

std::pair<int,int> current_transfer_stats()
{
	if(current_connection == received_data.end())
		return std::pair<int,int>(-1,-1);
	else
		return std::pair<int,int>(current_connection->second.upto,current_connection->second.buf.size());
}

} //end namespace network
