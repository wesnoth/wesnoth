/*
   Copyright (C) 2003 - 2015 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Networking
 */

#include "global.hpp"

#include "gettext.hpp"
#include "log.hpp"
#include "network_worker.hpp"
#include "serialization/string_utils.hpp"
#include "thread.hpp"
#include "util.hpp"
#include "config.hpp"

#include "filesystem.hpp"


#include <cerrno>
#include <queue>
#include <iomanip>
#include <set>
#include <cstring>
#include <stdexcept>
#include <sstream>

#include <boost/exception/get_error_info.hpp>
#include <boost/exception/info.hpp>

#include <signal.h>
#if defined(_WIN32) || defined(__WIN32__) || defined (WIN32)
#undef INADDR_ANY
#undef INADDR_BROADCAST
#undef INADDR_NONE
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>  // for TCP_NODELAY
#include <fcntl.h>
#define SOCKET int
#endif

static lg::log_domain log_network("network");
#define DBG_NW LOG_STREAM(debug, log_network)
#define LOG_NW LOG_STREAM(info, log_network)
#define WRN_NW LOG_STREAM(warn, log_network)
#define ERR_NW LOG_STREAM(err, log_network)
// Only warnings and not errors to avoid DoS by log flooding

namespace {

// We store the details of a connection in a map that must be looked up by its handle.
// This allows a connection to be disconnected and then recovered,
// but the handle remains the same, so it's all seamless to the user.
struct connection_details {
	connection_details(TCPsocket sock, const std::string& host, int port)
		: sock(sock), host(host), port(port), remote_handle(0),
	      connected_at(SDL_GetTicks())
	{}

	TCPsocket sock;
	std::string host;
	int port;

	// The remote handle is the handle assigned to this connection by the remote host.
	// Is 0 before a handle has been assigned.
	int remote_handle;

	int connected_at;
};

typedef std::map<network::connection,connection_details> connection_map;
connection_map connections;

network::connection connection_id = 1;

/** Stores the time of the last server ping we received. */
time_t last_ping, last_ping_check = 0;

} // end anon namespace

static int create_connection(TCPsocket sock, const std::string& host, int port)
{
	connections.insert(std::pair<network::connection,connection_details>(connection_id,connection_details(sock,host,port)));
	return connection_id++;
}

static connection_details& get_connection_details(network::connection handle)
{
	const connection_map::iterator i = connections.find(handle);
	if(i == connections.end()) {
		throw network::error(_("invalid network handle"));
	}

	return i->second;
}

static TCPsocket get_socket(network::connection handle)
{
	return get_connection_details(handle).sock;
}

static void remove_connection(network::connection handle)
{
	connections.erase(handle);
}

static bool is_pending_remote_handle(network::connection handle)
{
	const connection_details& details = get_connection_details(handle);
	return details.host != "" && details.remote_handle == 0;
}

static void set_remote_handle(network::connection handle, int remote_handle)
{
	get_connection_details(handle).remote_handle = remote_handle;
}

static void check_error()
{
	const TCPsocket sock = network_worker_pool::detect_error();
	if(sock) {
		for(connection_map::const_iterator i = connections.begin(); i != connections.end(); ++i) {
			if(i->second.sock == sock) {
				throw network::error(_("Client disconnected"),i->first);
			}
		}
	}
}

/**
 * Check whether too much time since the last server ping has passed and we
 * timed out. If the last check is too long ago reset the last_ping to 'now'.
 * This happens when we "freeze" the client one way or another or we just
 * didn't try to receive data.
 */
static void check_timeout()
{
	if (network::nconnections() == 0) {
		LOG_NW << "No network connections but last_ping is: " << last_ping;
		last_ping = 0;
		return;
	}
	const time_t& now = time(NULL);
	DBG_NW << "Last ping: '" << last_ping << "' Current time: '" << now
			<< "' Time since last ping: " << now - last_ping << "s\n";
	// Reset last_ping if we didn't check for the last 10s.
	if (last_ping_check + 10 <= now) last_ping = now;
	// This static cast is required on some build systems.
	// (Reported in #wesnoth-dev 2012/11/24 21:10:21, log time.)
	if ( last_ping + static_cast<time_t>(network::ping_interval + network::ping_timeout)
	     <= now )
	{
		time_t timeout = now - last_ping;
		ERR_NW << "No server ping since " << timeout
				<< " seconds. Connection timed out.\n";
		utils::string_map symbols;
		symbols["timeout"] = lexical_cast<std::string>(timeout);
		throw network::error("No server ping since " + str_cast(timeout) + " second. "
				"Connection timed out.");
	}
	last_ping_check = now;
}


namespace {

SDLNet_SocketSet socket_set = 0;
std::set<network::connection> waiting_sockets;
typedef std::vector<network::connection> sockets_list;
sockets_list sockets;


struct partial_buffer {
	partial_buffer() :
		buf(),
		upto(0)
	{
	}

	std::vector<char> buf;
	size_t upto;
};

TCPsocket server_socket;

std::deque<network::connection> disconnection_queue;
std::set<network::connection> bad_sockets;

network_worker_pool::manager* worker_pool_man = NULL;

} // end anon namespace

namespace network {

/**
 * Amount of seconds after the last server ping when we assume to have timed out.
 * When set to '0' ping timeout isn't checked.
 * Gets set in preferences::manager according to the preferences file.
 */
unsigned int ping_timeout = 0;

connection_stats::connection_stats(int sent, int received, int connected_at)
       : bytes_sent(sent), bytes_received(received), time_connected(SDL_GetTicks() - connected_at)
{}

connection_stats get_connection_stats(connection connection_num)
{
	connection_details& details = get_connection_details(connection_num);
	return connection_stats(get_send_stats(connection_num).total,get_receive_stats(connection_num).total,details.connected_at);
}

error::error(const std::string& msg, connection sock) : game::error(msg), socket(sock)
{
	if(socket) {
		bad_sockets.insert(socket);
	}
}

void error::disconnect()
{
	if(socket) network::disconnect(socket);
}

pending_statistics get_pending_stats()
{
	return network_worker_pool::get_pending_stats();
}

manager::manager(size_t min_threads, size_t max_threads) : free_(true)
{
	DBG_NW << "NETWORK MANAGER CALLED!\n";
	// If the network is already being managed
	if(socket_set) {
		free_ = false;
		return;
	}

	if(SDLNet_Init() == -1) {
		ERR_NW << "could not initialize SDLNet; throwing error..." << std::endl;
		throw error(SDL_GetError());
	}

	socket_set = SDLNet_AllocSocketSet(512);

	worker_pool_man = new network_worker_pool::manager(min_threads, max_threads);
}

manager::~manager()
{
	if(free_) {
		disconnect();
		delete worker_pool_man;
		worker_pool_man = NULL;
		SDLNet_FreeSocketSet(socket_set);
		socket_set = 0;
		waiting_sockets.clear();
		SDLNet_Quit();
	}
}

void set_raw_data_only()
{
	network_worker_pool::set_raw_data_only();
}

// --- Proxy methods
void enable_connection_through_proxy()
{
    throw std::runtime_error("Proxy not available while using SDL_net. Use ANA instead.");
}
void set_proxy_address ( const std::string& )
{
    throw std::runtime_error("Proxy not available while using SDL_net. Use ANA instead.");
}

void set_proxy_port    ( const std::string& )
{
    throw std::runtime_error("Proxy not available while using SDL_net. Use ANA instead.");
}

void set_proxy_user    ( const std::string& )
{
    throw std::runtime_error("Proxy not available while using SDL_net. Use ANA instead.");
}

void set_proxy_password( const std::string& )
{
    throw std::runtime_error("Proxy not available while using SDL_net. Use ANA instead.");
}
// --- End Proxy methods

server_manager::server_manager(int port, CREATE_SERVER create_server) : free_(false), connection_(0)
{
	if(create_server != NO_SERVER && !server_socket) {
		try {
			connection_ = connect("",port);
			server_socket = get_socket(connection_);
		} catch(network::error&) {
			if(create_server == MUST_CREATE_SERVER) {
				throw;
			} else {
				return;
			}
		}

		DBG_NW << "server socket initialized: " << server_socket << "\n";
		free_ = true;
	}
}

server_manager::~server_manager()
{
	stop();
}

void server_manager::stop()
{
	if(free_) {
		SDLNet_TCP_Close(server_socket);
		remove_connection(connection_);
		server_socket = 0;
		free_ = false;
	}
}

bool server_manager::is_running() const
{
	return server_socket != NULL;
}

size_t nconnections()
{
	return sockets.size();
}

bool is_server()
{
	return server_socket != 0;
}

namespace {

class connect_operation : public threading::async_operation
{
public:
	connect_operation(const std::string& host, int port) : host_(host), port_(port), error_(), connect_(0)
	{}

	void check_error();
	void run();

	network::connection result() const { return connect_; }

private:
	std::string host_;
	int port_;
	std::string error_;
	network::connection connect_;
};

void connect_operation::check_error()
{
	if(!error_.empty()) {
		throw error(error_);
	}
}

namespace {
	struct _TCPsocket {
		int ready;
		SOCKET channel;
		IPaddress remoteAddress;
		IPaddress localAddress;
		int sflag;
	};
} // end namespace

void connect_operation::run()
{
	char* const hostname = host_.empty() ? NULL : const_cast<char*>(host_.c_str());
	IPaddress ip;
	if(SDLNet_ResolveHost(&ip,hostname,port_) == -1) {
		error_ = N_("Could not connect to host.");
		return;
	}

	TCPsocket sock = SDLNet_TCP_Open(&ip);
	if(!sock) {
		error_ = hostname == NULL
				? "Could not bind to port"
				: N_("Could not connect to host.");
		return;
	}
	_TCPsocket* raw_sock = reinterpret_cast<_TCPsocket*>(sock);
#ifdef TCP_NODELAY
	//set TCP_NODELAY to 0 because SDL_Net turns it off, causing packet
	//flooding!
	{
		int no = 0;
		setsockopt(raw_sock->channel, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&no), sizeof(no));
	}
#endif

// Use non blocking IO
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
	{
		unsigned long mode = 1;
		ioctlsocket (raw_sock->channel, FIONBIO, &mode);
	}
#else
	int flags;
	flags = fcntl(raw_sock->channel, F_GETFL, 0);
#if defined(O_NONBLOCK)
	flags |= O_NONBLOCK;
#elif defined(O_NDELAY)
	flags |= O_NDELAY;
#elif defined(FNDELAY)
	flags |= FNDELAY;
#endif
	if (fcntl(raw_sock->channel, F_SETFL, flags) == -1) {
		error_ = "Could not make socket non-blocking: " + std::string(strerror(errno));
		SDLNet_TCP_Close(sock);
		return;
	}
#endif

	// If this is a server socket
	if(hostname == NULL) {
		const threading::lock l(get_mutex());
		connect_ = create_connection(sock,"",port_);
		return;
	}

	// Send data telling the remote host that this is a new connection
	union
	{
	char data[4] ALIGN_4;
	Uint32 num;
	} buf;
	SDLNet_Write32(0, &buf);
	const int nbytes = SDLNet_TCP_Send(sock,&buf,4);
	if(nbytes != 4) {
		SDLNet_TCP_Close(sock);
		error_ = "Could not send initial handshake";
		return;
	}

	// No blocking operations from here on
	const threading::lock l(get_mutex());
	DBG_NW << "sent handshake...\n";

	if(is_aborted()) {
		DBG_NW << "connect operation aborted by calling thread\n";
		SDLNet_TCP_Close(sock);
		return;
	}

	// Allocate this connection a connection handle
	connect_ = create_connection(sock,host_,port_);

	const int res = SDLNet_TCP_AddSocket(socket_set,sock);
	if(res == -1) {
		SDLNet_TCP_Close(sock);
		error_ = "Could not add socket to socket set";
		return;
	}

	waiting_sockets.insert(connect_);

	sockets.push_back(connect_);

	while(!notify_finished()) {};
}


} // end namespace


connection connect(const std::string& host, int port)
{
	connect_operation op(host,port);
	op.run();
	op.check_error();
	return op.result();
}

connection connect(const std::string& host, int port, threading::waiter& waiter)
{
	const threading::async_operation_ptr op(new connect_operation(host,port));
	const connect_operation::RESULT res = op->execute(op, waiter);
	if(res == connect_operation::ABORTED) {
		return 0;
	}

	static_cast<connect_operation*>(op.get())->check_error();
	return static_cast<connect_operation*>(op.get())->result();
}

namespace {

connection accept_connection_pending(std::vector<TCPsocket>& pending_sockets,
	SDLNet_SocketSet& pending_socket_set)
{
	DBG_NW << "pending socket activity...\n";

	std::vector<TCPsocket>::iterator i = pending_sockets.begin();
	while (i != pending_sockets.end() && !SDLNet_SocketReady(*i)) ++i;

	if (i == pending_sockets.end()) return 0;

	// Receive the 4 bytes telling us if they're a new connection
	// or trying to recover a connection
	union
	{
	char data[4] ALIGN_4;
	Uint32 num;
	} buf;

	const TCPsocket psock = *i;
	SDLNet_TCP_DelSocket(pending_socket_set,psock);
	pending_sockets.erase(i);

	DBG_NW << "receiving data from pending socket...\n";

	const int len = SDLNet_TCP_Recv(psock,&buf,4);
	if(len != 4) {
		WRN_NW << "pending socket disconnected" << std::endl;
		SDLNet_TCP_Close(psock);
		return 0;
	}

	const int handle = SDLNet_Read32(&buf);

	DBG_NW << "received handshake from client: '" << handle << "'\n";

	const int res = SDLNet_TCP_AddSocket(socket_set,psock);
	if(res == -1) {
		ERR_NW << "SDLNet_GetError(): " << SDLNet_GetError() << std::endl;
		SDLNet_TCP_Close(psock);

		throw network::error(_("Could not add socket to socket set"));
	}

	const connection connect = create_connection(psock,"",0);

	// Send back their connection number
	SDLNet_Write32(connect, &buf);
	const int nbytes = SDLNet_TCP_Send(psock,&buf,4);
	if(nbytes != 4) {
		SDLNet_TCP_DelSocket(socket_set,psock);
		SDLNet_TCP_Close(psock);
		remove_connection(connect);
		throw network::error(_("Could not send initial handshake"));
	}

	waiting_sockets.insert(connect);
	sockets.push_back(connect);
	return connect;
}

} //anon namespace

connection accept_connection()
{
	if(!server_socket) {
		return 0;
	}

	// A connection isn't considered 'accepted' until it has sent its initial handshake.
	// The initial handshake is a 4 byte value, which is 0 for a new connection,
	// or the handle of the connection if it's trying to recover a lost connection.

	/**
	 * A list of all the sockets which have connected,
	 * but haven't had their initial handshake received.
	 */
	static std::vector<TCPsocket> pending_sockets;
	static SDLNet_SocketSet pending_socket_set = 0;

	const TCPsocket sock = SDLNet_TCP_Accept(server_socket);
	if(sock) {
#if !defined(_WIN32) && !defined(__WIN32__) && !defined (WIN32)
		_TCPsocket* raw_sock = reinterpret_cast<_TCPsocket*>(sock);
		int fd_flags = fcntl(raw_sock->channel, F_GETFD, 0);
		fd_flags |= FD_CLOEXEC;
		if (fcntl(raw_sock->channel, F_SETFD, fd_flags) == -1) {
			WRN_NW << "could not make socket " << sock << " close-on-exec: " << strerror(errno);
		} else {
			DBG_NW << "made socket " << sock << " close-on-exec\n";
		}
#endif

		DBG_NW << "received connection. Pending handshake...\n";

		if(pending_socket_set == 0) {
			pending_socket_set = SDLNet_AllocSocketSet(32);
		}

		if(pending_socket_set != 0) {
			int res = SDLNet_TCP_AddSocket(pending_socket_set,sock);

			if (res != -1) {
				pending_sockets.push_back(sock);
			} else {
				ERR_NW << "Pending socket set is full! Disconnecting " << sock << " connection" << std::endl;
				ERR_NW << "SDLNet_GetError(): " << SDLNet_GetError() << std::endl;

				SDLNet_TCP_Close(sock);
			}
		} else {
			ERR_NW << "Error in SDLNet_AllocSocketSet" << std::endl;
		}
	}

	if(pending_socket_set == 0) {
		return 0;
	}

	const int set_res = SDLNet_CheckSockets(pending_socket_set,0);
	if(set_res <= 0) {
		return 0;
	}

	return accept_connection_pending(pending_sockets, pending_socket_set);
}

bool disconnect(connection s)
{
	if(s == 0) {
		while(sockets.empty() == false) {
			assert(sockets.back() != 0);
			while(disconnect(sockets.back()) == false) {
				SDL_Delay(1);
			}
		}
		return true;
	}
	if (!is_server()) last_ping = 0;

	const connection_map::iterator info = connections.find(s);
	if(info != connections.end()) {
		if (info->second.sock == server_socket)
		{
			return true;
		}
		if (!network_worker_pool::close_socket(info->second.sock)) {
			return false;
		}
	}

	bad_sockets.erase(s);

	std::deque<network::connection>::iterator dqi = std::find(disconnection_queue.begin(),disconnection_queue.end(),s);
	if(dqi != disconnection_queue.end()) {
		disconnection_queue.erase(dqi);
	}

	const sockets_list::iterator i = std::find(sockets.begin(),sockets.end(),s);
	if(i != sockets.end()) {
		sockets.erase(i);

		const TCPsocket sock = get_socket(s);

		waiting_sockets.erase(s);
		SDLNet_TCP_DelSocket(socket_set,sock);
		SDLNet_TCP_Close(sock);

		remove_connection(s);
	} else {
		if(sockets.size() == 1) {
			DBG_NW << "valid socket: " << static_cast<int>(*sockets.begin()) << "\n";
		}
	}

	return true;
}

void queue_disconnect(network::connection sock)
{
	disconnection_queue.push_back(sock);
}

connection receive_data(config& cfg, connection connection_num, unsigned int timeout, bandwidth_in_ptr* bandwidth_in)
{
	unsigned int start_ticks = SDL_GetTicks();
	while(true) {
		const connection res = receive_data(
				cfg,connection_num, bandwidth_in);
		if(res != 0) {
			return res;
		}

		if(timeout > SDL_GetTicks() - start_ticks) {
			SDL_Delay(1);
		}
		else
		{
			break;
		}

	}

	return 0;
}

connection receive_data(config& cfg, connection connection_num, bandwidth_in_ptr* bandwidth_in)
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

	const int res = SDLNet_CheckSockets(socket_set,0);

	for(std::set<network::connection>::iterator i = waiting_sockets.begin(); res != 0 && i != waiting_sockets.end(); ) {
		connection_details& details = get_connection_details(*i);
		const TCPsocket sock = details.sock;
		if(SDLNet_SocketReady(sock)) {

			// See if this socket is still waiting for it to be assigned its remote handle.
			// If it is, then the first 4 bytes must be the remote handle.
			if(is_pending_remote_handle(*i)) {
				union {
				char data[4] ALIGN_4;
				} buf;
				int len = SDLNet_TCP_Recv(sock,&buf,4);
				if(len != 4) {
					throw error("Remote host disconnected",*i);
				}

				const int remote_handle = SDLNet_Read32(&buf);
				set_remote_handle(*i,remote_handle);

				continue;
			}

			waiting_sockets.erase(i++);
			SDLNet_TCP_DelSocket(socket_set,sock);
			network_worker_pool::receive_data(sock);
		} else {
			++i;
		}
	}


	TCPsocket sock = connection_num == 0 ? 0 : get_socket(connection_num);
	TCPsocket s = sock;
	bandwidth_in_ptr temp;
	if (!bandwidth_in)
	{
		bandwidth_in = &temp;
	}
	try {
		sock = network_worker_pool::get_received_data(sock,cfg, *bandwidth_in);
	} catch(const config::error& e) {
		TCPsocket const * err_sock = boost::get_error_info<tcpsocket_info>(e);
		if(err_sock == NULL)
			throw;
		connection err_connection = 0;
		for(connection_map::const_iterator i = connections.begin(); i != connections.end(); ++i) {
			if(i->second.sock == *err_sock) {
				err_connection = i->first;
			}
		}
		if(err_connection) {
			throw e << connection_info(err_connection);
		}
		throw;
	}
	if (sock == NULL) {
		if (!is_server() && last_ping != 0 && ping_timeout != 0)
		{
			if (connection_num == 0)
			{
				s = get_socket(sockets.back());
			}
			if (!network_worker_pool::is_locked(s))
			{
				check_timeout();
			}
		}
		return 0;
	}

	int set_res = SDLNet_TCP_AddSocket(socket_set,sock);
	if (set_res == -1)
	{
		ERR_NW << "Socket set is full! Disconnecting " << sock << " connection" << std::endl;
		SDLNet_TCP_Close(sock);
		return 0;
	}

	connection result = 0;
	for(connection_map::const_iterator j = connections.begin(); j != connections.end(); ++j) {
		if(j->second.sock == sock) {
			result = j->first;
			break;
		}
	}
	if(!cfg.empty()) {
		DBG_NW << "RECEIVED from: " << result << ": " << cfg;
	}

	assert(result != 0);
	waiting_sockets.insert(result);
	if(!is_server()) {
		const time_t& now = time(NULL);
		if (cfg.has_attribute("ping")) {
			LOG_NW << "Lag: " << (now - lexical_cast<time_t>(cfg["ping"])) << "\n";
			last_ping = now;
		} else if (last_ping != 0) {
			last_ping = now;
		}
	}
	return result;
}

connection receive_data(std::vector<char>& buf, bandwidth_in_ptr* bandwidth_in)
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

	if(bad_sockets.count(0)) {
		return 0;
	}

	if(sockets.empty()) {
		return 0;
	}

	const int res = SDLNet_CheckSockets(socket_set,0);

	for(std::set<network::connection>::iterator i = waiting_sockets.begin(); res != 0 && i != waiting_sockets.end(); ) {
		connection_details& details = get_connection_details(*i);
		const TCPsocket sock = details.sock;
		if(SDLNet_SocketReady(sock)) {

			// See if this socket is still waiting for it to be assigned its remote handle.
			// If it is, then the first 4 bytes must be the remote handle.
			if(is_pending_remote_handle(*i)) {
				union {
				char data[4] ALIGN_4;
				} buf;
				int len = SDLNet_TCP_Recv(sock,&buf,4);
				if(len != 4) {
					throw error("Remote host disconnected",*i);
				}

				const int remote_handle = SDLNet_Read32(&buf);
				set_remote_handle(*i,remote_handle);

				continue;
			}

			waiting_sockets.erase(i++);
			SDLNet_TCP_DelSocket(socket_set,sock);
			network_worker_pool::receive_data(sock);
		} else {
			++i;
		}
	}


	TCPsocket sock = network_worker_pool::get_received_data(buf);
	if (sock == NULL) {
		return 0;
	}

	{
		bandwidth_in_ptr temp;
		if (!bandwidth_in)
		{
			bandwidth_in = &temp;
		}
		const int headers = 4;
		bandwidth_in->reset(new network::bandwidth_in(buf.size() + headers));
	}

	int set_res = SDLNet_TCP_AddSocket(socket_set,sock);

	if (set_res == -1)
	{
		ERR_NW << "Socket set is full! Disconnecting " << sock << " connection" << std::endl;
		SDLNet_TCP_Close(sock);
		return 0;
	}
	connection result = 0;
	for(connection_map::const_iterator j = connections.begin(); j != connections.end(); ++j) {
		if(j->second.sock == sock) {
			result = j->first;
			break;
		}
	}

	assert(result != 0);
	waiting_sockets.insert(result);
	return result;
}
struct bandwidth_stats {
	int out_packets;
	int out_bytes;
	int in_packets;
	int in_bytes;
	int day;
	const static size_t type_width = 16;
	const static size_t packet_width = 7;
	const static size_t bytes_width = 10;
	bandwidth_stats& operator+=(const bandwidth_stats& a)
	{
		out_packets += a.out_packets;
		out_bytes += a.out_bytes;
		in_packets += a.in_packets;
		in_bytes += a.in_bytes;

		return *this;
	}
};
typedef std::map<const std::string, bandwidth_stats> bandwidth_map;
typedef std::vector<bandwidth_map> hour_stats_vector;

static hour_stats_vector hour_stats(24);


static bandwidth_map::iterator add_bandwidth_entry(const std::string& packet_type)
{
	time_t now = time(0);
	struct tm * timeinfo = localtime(&now);
	int hour = timeinfo->tm_hour;
	int day = timeinfo->tm_mday;
	assert(hour < 24 && hour >= 0);
	std::pair<bandwidth_map::iterator,bool> insertion = hour_stats[hour].insert(std::make_pair(packet_type, bandwidth_stats()));
	bandwidth_map::iterator inserted = insertion.first;
	if (!insertion.second && day != inserted->second.day)
	{
		// clear previuos day stats
		hour_stats[hour].clear();
		//insert again to cleared map
		insertion = hour_stats[hour].insert(std::make_pair(packet_type, bandwidth_stats()));
		inserted = insertion.first;
	}

	inserted->second.day = day;
	return inserted;
}

typedef boost::shared_ptr<bandwidth_stats> bandwidth_stats_ptr;


struct bandwidth_stats_output {
	bandwidth_stats_output(std::stringstream& ss) : ss_(ss), totals_(new bandwidth_stats())
	{}
	void operator()(const bandwidth_map::value_type& stats)
	{
		// name
		ss_	<< " " << std::setw(bandwidth_stats::type_width) <<  stats.first << "| "
			<< std::setw(bandwidth_stats::packet_width)<< stats.second.out_packets << "| "
			<< std::setw(bandwidth_stats::bytes_width) << stats.second.out_bytes/1024 << "| "
			<< std::setw(bandwidth_stats::packet_width)<< stats.second.in_packets << "| "
			<< std::setw(bandwidth_stats::bytes_width) << stats.second.in_bytes/1024 << "\n";
		*totals_ += stats.second;
	}
	void output_totals()
	{
		(*this)(std::make_pair(std::string("total"), *totals_));
	}
	private:
	std::stringstream& ss_;
	bandwidth_stats_ptr totals_;
};

std::string get_bandwidth_stats_all()
{
	std::string result;
	for (int hour = 0; hour < 24; ++hour)
	{
		result += get_bandwidth_stats(hour);
	}
	return result;
}

std::string get_bandwidth_stats()
{
	time_t now = time(0);
	struct tm * timeinfo = localtime(&now);
	int hour = timeinfo->tm_hour - 1;
	if (hour < 0)
		hour = 23;
	return get_bandwidth_stats(hour);
}

std::string get_bandwidth_stats(int hour)
{
	assert(hour < 24 && hour >= 0);
	std::stringstream ss;

	ss << "Hour stat starting from " << hour << "\n " << std::left << std::setw(bandwidth_stats::type_width) <<  "Type of packet" << "| "
		<< std::setw(bandwidth_stats::packet_width)<< "out #"  << "| "
		<< std::setw(bandwidth_stats::bytes_width) << "out KiB" << "| "
		<< std::setw(bandwidth_stats::packet_width)<< "in #"  << "| "
		<< std::setw(bandwidth_stats::bytes_width) << "in KiB" << "\n";

	bandwidth_stats_output outputer(ss);
	std::for_each(hour_stats[hour].begin(), hour_stats[hour].end(), outputer);

	outputer.output_totals();
	return ss.str();
}

void add_bandwidth_out(const std::string& packet_type, size_t len)
{
	bandwidth_map::iterator itor = add_bandwidth_entry(packet_type);
	itor->second.out_bytes += len;
	++(itor->second.out_packets);
}

void add_bandwidth_in(const std::string& packet_type, size_t len)
{
	bandwidth_map::iterator itor = add_bandwidth_entry(packet_type);
	itor->second.in_bytes += len;
	++(itor->second.in_packets);
}

	bandwidth_in::~bandwidth_in()
	{
		add_bandwidth_in(type_, len_);
	}

void send_file(const std::string& filename, connection connection_num, const std::string& packet_type)
{
	assert(connection_num > 0);
	if(bad_sockets.count(connection_num) || bad_sockets.count(0)) {
		return;
	}

	const connection_map::iterator info = connections.find(connection_num);
	if (info == connections.end()) {
		ERR_NW << "Error: socket: " << connection_num
			<< "\tnot found in connection_map. Not sending...\n";
		return;
	}

	const int size = filesystem::file_size(filename);

	if(size < 0) {
		ERR_NW << "Could not determine size of file " << filename << ", not sending." << std::endl;
		return;
	}

	const int packet_headers = 4;
	add_bandwidth_out(packet_type, size + packet_headers);
	network_worker_pool::queue_file(info->second.sock, filename);

}

size_t send_data(const config& cfg, connection connection_num, const std::string& packet_type)
{
	DBG_NW << "in send_data()...\n";

	if(cfg.empty()) {
		return 0;
	}

	if(bad_sockets.count(connection_num) || bad_sockets.count(0)) {
		return 0;
	}

//	log_scope2(network, "sending data");
	if(!connection_num) {
		DBG_NW << "sockets: " << sockets.size() << "\n";
		size_t size = 0;
		for(sockets_list::const_iterator i = sockets.begin();
		    i != sockets.end(); ++i) {
			DBG_NW << "server socket: " << server_socket << "\ncurrent socket: " << *i << "\n";
			size = send_data(cfg,*i, packet_type);
		}
		return size;
	}

	const connection_map::iterator info = connections.find(connection_num);
	if (info == connections.end()) {
		ERR_NW << "Error: socket: " << connection_num
			<< "\tnot found in connection_map. Not sending...\n";
		return 0;
	}

	LOG_NW << "SENDING to: " << connection_num << ": " << cfg;
	return network_worker_pool::queue_data(info->second.sock, cfg, packet_type);
}

void send_raw_data(const char* buf, int len, connection connection_num, const std::string& packet_type)
{
	if(len == 0) {
		return;
	}

	if(bad_sockets.count(connection_num) || bad_sockets.count(0)) {
		return;
	}

	if(!connection_num) {
		for(sockets_list::const_iterator i = sockets.begin();
		    i != sockets.end(); ++i) {
			send_raw_data(buf, len, *i, packet_type);
		}
		return;
	}

	const connection_map::iterator info = connections.find(connection_num);
	if (info == connections.end()) {
		ERR_NW << "Error: socket: " << connection_num
			<< "\tnot found in connection_map. Not sending...\n";
		return;
	}
	const int packet_headers = 4;
	add_bandwidth_out(packet_type, len + packet_headers);

	network_worker_pool::queue_raw_data(info->second.sock, buf, len);
}

void process_send_queue(connection, size_t)
{
	check_error();
}

void send_data_all_except(const config& cfg, connection connection_num, const std::string& packet_type)
{
	for(sockets_list::const_iterator i = sockets.begin(); i != sockets.end(); ++i) {
		if(*i == connection_num) {
			continue;
		}

		send_data(cfg,*i, packet_type);
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

statistics get_send_stats(connection handle)
{
	return network_worker_pool::get_current_transfer_stats(handle == 0 ? get_socket(sockets.back()) : get_socket(handle)).first;
}
statistics get_receive_stats(connection handle)
{
    const statistics result = network_worker_pool::get_current_transfer_stats(handle == 0 ? get_socket(sockets.back()) : get_socket(handle)).second;

	return result;
}

} // end namespace network
