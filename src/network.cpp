#include "log.hpp"
#include "network.hpp"
#include "network_worker.hpp"
#include "util.hpp"

#include "SDL_net.h"

#include <algorithm>
#include <cassert>
#include <cerrno>
#include <queue>
#include <iostream>
#include <set>
#include <vector>

#include <signal.h>

#define LOG_NW lg::info(lg::network)
#define WRN_NW lg::warn(lg::network)
// only warnings and not errors to avoid DoS by log flooding

namespace {

//We store the details of a connection in a map that must be looked up by its handle.
//This allows a connection to be disconnected and then recovered, but the handle remains
//the same, so it's all seamless to the user
struct connection_details {
	connection_details(TCPsocket sock, const std::string& host, int port)
		: sock(sock), host(host), port(port), remote_handle(0),
	      connected_at(SDL_GetTicks()), sent(0), received(0)
	{}

	TCPsocket sock;
	std::string host;
	int port;

	//the remote handle is the handle assigned to this connection by the remote host.
	//is 0 before a handle has been assigned.
	int remote_handle;

	int connected_at;
	int sent, received;
};

typedef std::map<network::connection,connection_details> connection_map;
connection_map connections;

network::connection connection_id = 1;

int create_connection(TCPsocket sock, const std::string& host, int port)
{
	connections.insert(std::pair<network::connection,connection_details>(connection_id,connection_details(sock,host,port)));
	return connection_id++;
}

connection_details& get_connection_details(network::connection handle)
{
	const connection_map::iterator i = connections.find(handle);
	if(i == connections.end()) {
		throw network::error("invalid network handle");
	}

	return i->second;
}

TCPsocket get_socket(network::connection handle)
{
	return get_connection_details(handle).sock;
}

void remove_connection(network::connection handle)
{
	connections.erase(handle);
}

bool is_pending_remote_handle(network::connection handle)
{
	const connection_details& details = get_connection_details(handle);
	return details.host != "" && details.remote_handle == 0;
}

void set_remote_handle(network::connection handle, int remote_handle)
{
	get_connection_details(handle).remote_handle = remote_handle;
}

void check_error()
{
	const TCPsocket sock = network_worker_pool::detect_error();
	if(sock) {
		for(connection_map::const_iterator i = connections.begin(); i != connections.end(); ++i) {
			if(i->second.sock == sock) {
				throw network::error("Error sending data",i->first);
			}
		}
	}
}

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

TCPsocket server_socket;

std::deque<network::connection> disconnection_queue;
std::set<network::connection> bad_sockets;

network_worker_pool::manager* worker_pool_man = NULL;

}

namespace network {

connection_stats::connection_stats(int sent, int received, int connected_at)
       : bytes_sent(sent), bytes_received(received), time_connected(SDL_GetTicks() - connected_at)
{}

connection_stats get_connection_stats(connection connection_num)
{
	connection_details& details = get_connection_details(connection_num);
	return connection_stats(details.sent,details.received,details.connected_at);
}

const connection null_connection = 0;

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

manager::manager(size_t nthreads) : free_(true)
{
	//if the network is already being managed
	if(socket_set) {
		free_ = false;
		return;
	}

	//on Unix-based systems, set sigpipe to be ignored
#if !(defined(_WIN32) || defined(__APPLE__))
	WRN_NW << "ignoring SIGPIPE\n";
	signal(SIGPIPE,SIG_IGN);
#endif

	if(SDLNet_Init() == -1) {
		lg::err(lg::network) << "could not initialize SDLNet; throwing error...\n";
		throw error(SDL_GetError());
	}

	socket_set = SDLNet_AllocSocketSet(64);

	worker_pool_man = new network_worker_pool::manager(nthreads);
}

manager::~manager()
{
	if(free_) {
		disconnect();
		delete worker_pool_man;
		worker_pool_man = NULL;
		SDLNet_FreeSocketSet(socket_set);
		socket_set = 0;
		SDLNet_Quit();
	}
}

server_manager::server_manager(int port, CREATE_SERVER create_server) : free_(false)
{
	if(create_server != NO_SERVER && !server_socket) {
		try {
			server_socket = get_socket(connect("",port));
		} catch(network::error& e) {
			if(create_server == MUST_CREATE_SERVER) {
				throw e;
			} else {
				return;
			}
		}

		LOG_NW << "server socket initialized: " << server_socket << "\n";
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

bool server_manager::is_running() const
{
	return static_cast<bool>(server_socket);
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
		throw error(hostname == NULL ? "Could not bind to port" :
		                               "Could not connect to host");
	} else {
		//TODO: add code in here which sets the socket to non-blocking
	}

	//if this is a server socket
	if(hostname == NULL) {
		return create_connection(sock,"",port);
	}

	LOG_NW << "sending handshake...\n";
	//send data telling the remote host that this is a new connection
	char buf[4];
	SDLNet_Write32(0,buf);
	const int nbytes = SDLNet_TCP_Send(sock,buf,4);
	if(nbytes != 4) {
		SDLNet_TCP_Close(sock);
		throw network::error("Could not send initial handshake");
	}
	LOG_NW << "sent handshake...\n";

	//allocate this connection a connection handle
	const network::connection connect = create_connection(sock,host,port);

	const int res = SDLNet_TCP_AddSocket(socket_set,sock);
	if(res == -1) {
		SDLNet_TCP_Close(sock);
		throw network::error("Could not add socket to socket set");
	}

	sockets.push_back(connect);
	schemas.insert(std::pair<network::connection,schema_pair>(connect,schema_pair()));

	return connect;
}

connection accept_connection()
{
	if(!server_socket) {
		return 0;
	}

	//a connection isn't considered 'accepted' until it has sent its initial handshake.
	//The initial handshake is a 4 byte value, which is 0 for a new connection, or the
	//handle of the connection if it's trying to recover a lost connection.

	//a list of all the sockets which have connected, but haven't had their initial
	//handshake received
	static std::vector<TCPsocket> pending_sockets;
	static SDLNet_SocketSet pending_socket_set = 0;

	const TCPsocket sock = SDLNet_TCP_Accept(server_socket);
	if(sock) {
		LOG_NW << "received connection. Pending handshake...\n";
		pending_sockets.push_back(sock);
		if(pending_socket_set == 0) {
			pending_socket_set = SDLNet_AllocSocketSet(64);
		}

		if(pending_socket_set != 0) {
			SDLNet_TCP_AddSocket(pending_socket_set,sock);
		}
	}

	if(pending_socket_set == 0) {
		return 0;
	}

	const int set_res = SDLNet_CheckSockets(pending_socket_set,0);
	if(set_res <= 0) {
		return 0;
	}

	LOG_NW << "pending socket activity...\n";

	for(std::vector<TCPsocket>::iterator i = pending_sockets.begin(); i != pending_sockets.end(); ++i) {
		if(!SDLNet_SocketReady(*i)) {
			continue;
		}

		//receive the 4 bytes telling us if they're a new connection or trying to
		//recover a connection
		char buf[4];

		const TCPsocket sock = *i;
		SDLNet_TCP_DelSocket(pending_socket_set,sock);
		pending_sockets.erase(i);

		LOG_NW << "receiving data from pending socket...\n";

		const int len = SDLNet_TCP_Recv(sock,buf,4);
		if(len != 4) {
			WRN_NW << "pending socket disconnected\n";
			SDLNet_TCP_Close(sock);
			return 0;
		}

		const int handle = SDLNet_Read32(buf);

		LOG_NW << "received handshake from client: '" << handle << "'\n";

		const int res = SDLNet_TCP_AddSocket(socket_set,sock);
		if(res == -1) {
			SDLNet_TCP_Close(sock);

			throw network::error("Could not add socket to socket set");
		}

		const connection connect = create_connection(sock,"",0);

		//send back their connection number
		SDLNet_Write32(connect,buf);
		const int nbytes = SDLNet_TCP_Send(sock,buf,4);
		if(nbytes != 4) {
			SDLNet_TCP_Close(sock);
			throw network::error("Could not send initial handshake");
		}

		sockets.push_back(connect);
		schemas.insert(std::pair<network::connection,schema_pair>(connect,schema_pair()));
		return connect;
	}

	return 0;
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

	const connection_map::iterator info = connections.find(s);
	if(info != connections.end()) {
		network_worker_pool::close_socket(info->second.sock);
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

		const TCPsocket sock = get_socket(s);

		SDLNet_TCP_DelSocket(socket_set,sock);
		SDLNet_TCP_Close(sock);

		remove_connection(s);
	} else {
		if(sockets.size() == 1) {
			LOG_NW << "valid socket: " << (int)*sockets.begin() << "\n";
		}
	}
}

void queue_disconnect(network::connection sock)
{
	disconnection_queue.push_back(sock);
}

connection receive_data(config& cfg, connection connection_num, int timeout)
{
	if(!socket_set) {
		return 0;
	}

	check_error();

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
		connection_details& details = get_connection_details(*i);
		const TCPsocket sock = details.sock;
		if(SDLNet_SocketReady(sock)) {

			//see if this socket is still waiting for it to be assigned its remote handle
			//if it is, then the first 4 bytes must be the remote handle.
			if(is_pending_remote_handle(*i)) {
				char buf[4];
				int len = SDLNet_TCP_Recv(sock,buf,4);
				if(len != 4) {
					throw error("Remote host disconnected",*i);
				}

				const int remote_handle = SDLNet_Read32(buf);
				set_remote_handle(*i,remote_handle);

				break;
			}


			std::map<connection,partial_buffer>::iterator part_received = received_data.find(*i);
			if(part_received == received_data.end()) {
				char num_buf[4];
				int len = SDLNet_TCP_Recv(sock,num_buf,4);

				if(len != 4) {
					throw error("Remote host disconnected",*i);
				}

				details.received += len;

				len = SDLNet_Read32(num_buf);

				LOG_NW << "received packet length: " << len << "\n";

				if((len < 1) || (len > 10000000)) {
					WRN_NW << "bad length in network packet. Throwing error\n";
					throw error("network error: bad length data",*i);
				}

				part_received = received_data.insert(std::pair<connection,partial_buffer>(*i,partial_buffer())).first;
				part_received->second.buf.resize(len);

				//make sure that this connection still has data
				const int res = SDLNet_CheckSockets(socket_set,0);
				if(res <= 0 || !SDLNet_SocketReady(sock)) {
					WRN_NW << "packet has no data after length. Throwing error\n";
					throw error("network error: received wrong number of bytes: 0",*i);
				}
			}

			current_connection = part_received;
			partial_buffer& buf = part_received->second;

			const size_t expected = buf.buf.size() - buf.upto;
			const int nbytes = SDLNet_TCP_Recv(sock,&buf.buf[buf.upto],expected);
			if(nbytes <= 0) {
				WRN_NW << "SDLNet_TCP_Recv returned " << nbytes << " error in socket\n";
				throw error("remote host disconnected",*i);
			}

			details.received += nbytes;

			buf.upto += nbytes;
			LOG_NW << "received " << nbytes << "=" << buf.upto << "/" << buf.buf.size() << "\n";

			if(buf.upto == buf.buf.size()) {
				current_connection = received_data.end();
				const std::string buffer(buf.buf.begin(),buf.buf.end());
				received_data.erase(part_received); //invalidates buf. don't use again
				if(buffer == "") {
					WRN_NW << "buffer from remote host is empty\n";
					throw error("remote host closed connection",*i);
				}

				if(buffer[buffer.size()-1] != 0) {
					WRN_NW << "buf not nul-delimited. Network error\n";
					throw error("sanity check on incoming data failed",*i);
				}

				const schema_map::iterator schema = schemas.find(*i);
				assert(schema != schemas.end());

				cfg.read_compressed(buffer,schema->second.incoming);

//				std::cerr << "--- RECEIVED DATA from " << ((int)*i) << ": '"
//				          << cfg.write() << "'\n--- END RECEIVED DATA\n";

				
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

void send_data(const config& cfg, connection connection_num, size_t max_size, SEND_TYPE mode)
{
	LOG_NW << "in send_data()...\n";
	if(cfg.empty()) {
		return;
	}

	if(bad_sockets.count(connection_num) || bad_sockets.count(0)) {
		return;
	}

	if(max_size == 0) {
		max_size = default_max_send_size;
	}

	if(max_size > 0 && max_size < 8) {
		max_size = 8;
	}

	log_scope2(network, "sending data");
	if(!connection_num) {
		LOG_NW << "sockets: " << sockets.size() << "\n";
		for(sockets_list::const_iterator i = sockets.begin();
		    i != sockets.end(); ++i) {
			LOG_NW << "server socket: " << server_socket << "\ncurrent socket: " << *i << "\n";
			send_data(cfg,*i);
		}

		return;
	}

	const schema_map::iterator schema = schemas.find(connection_num);
	assert(schema != schemas.end());

	const std::string& value = cfg.write_compressed(schema->second.outgoing);

//	std::cerr << "--- SEND DATA to " << ((int)connection_num) << ": '"
//	          << cfg.write() << "'\n--- END SEND DATA\n";

	std::vector<char> buf(4 + value.size() + 1);
	SDLNet_Write32(value.size()+1,&buf[0]);
	std::copy(value.begin(),value.end(),buf.begin()+4);
	buf.back() = 0;

	const connection_map::iterator info = connections.find(connection_num);
	assert(info != connections.end());

	network_worker_pool::queue_data(info->second.sock,buf);
}

void queue_data(const config& cfg, connection connection_num)
{
	send_data(cfg,connection_num,0,QUEUE_ONLY);
}

void process_send_queue(connection connection_num, size_t max_size)
{
	check_error();
}

void send_data_all_except(const config& cfg, connection connection_num, size_t max_size)
{
	for(sockets_list::const_iterator i = sockets.begin(); i != sockets.end(); ++i) {
		if(*i == connection_num) {
			continue;
		}

		send_data(cfg,*i,max_size);
	}
}

std::string ip_address(connection connection_num)
{
	std::stringstream str;
	const IPaddress* const ip = SDLNet_TCP_GetPeerAddress(get_socket(connection_num));
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
