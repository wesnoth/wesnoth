/* $Id$ */
/*
   Copyright (C) 2003 - 2007 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file network.cpp
//! Networking

#include "global.hpp"

#include "serialization/binary_wml.hpp"
#include "config.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "network.hpp"
#include "network_worker.hpp"
#include "thread.hpp"

#include "SDL_net.h"

#include <algorithm>
#include <cassert>
#include <cerrno>
#include <queue>
#include <iostream>
#include <set>
#include <vector>
#include <ctime>

#include <signal.h>
#if defined(_WIN32) || defined(__WIN32__) || defined (WIN32)
#undef INADDR_ANY
#undef INADDR_BROADCAST
#undef INADDR_NONE
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#ifdef __BEOS__
#include <socket.h>
#else
#include <fcntl.h>
#endif
#define SOCKET int
#endif

#define DBG_NW LOG_STREAM(debug, network)
#define LOG_NW LOG_STREAM(info, network)
#define WRN_NW LOG_STREAM(warn, network)
#define ERR_NW LOG_STREAM(err, network)
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

//! Stores the time of the last server ping we received.
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

//! Check whether too much time since the last server ping has passed and we
//! timed out. If the last check is too long ago reset the last_ping to 'now'.
//! This happens when we "freeze" the client one way or another or we just
//! didn't try to receive data.
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
	if (last_ping + network::ping_timeout <= now) {
		int timeout = now - last_ping;
		ERR_NW << "No server ping since " << timeout
				<< " seconds. Connection timed out.\n";
		utils::string_map symbols;
		symbols["timeout"] = lexical_cast<std::string>(timeout);
		throw network::error(vngettext("No server ping since $timeout second. "
				"Connection timed out.", "No server ping since $timeout seconds. "
				"Connection timed out.", timeout, symbols));
	}
	last_ping_check = now;
}


namespace {

SDLNet_SocketSet socket_set = 0;
std::set<network::connection> waiting_sockets;
typedef std::vector<network::connection> sockets_list;
sockets_list sockets;


struct partial_buffer {
	partial_buffer() : upto(0) {}
	std::vector<char> buf;
	size_t upto;
};

TCPsocket server_socket;

std::deque<network::connection> disconnection_queue;
std::set<network::connection> bad_sockets;

network_worker_pool::manager* worker_pool_man = NULL;

} // end anon namespace

namespace network {

//! Amount of seconds after the last server ping when we assume to have timed out.
//! When set to '0' ping timeout isn't checked.
//! Gets set in preferences::manager according to the preferences file.
unsigned int ping_timeout = 0;

connection_stats::connection_stats(int sent, int received, int connected_at)
       : bytes_sent(sent), bytes_received(received), time_connected(SDL_GetTicks() - connected_at)
{}

connection_stats get_connection_stats(connection connection_num)
{
	connection_details& details = get_connection_details(connection_num);
	return connection_stats(get_send_stats(connection_num).total,get_receive_stats(connection_num).total,details.connected_at);
}

error::error(const std::string& msg, connection sock) : message(msg), socket(sock)
{
	if(socket) {
		bad_sockets.insert(socket);
	}
}

void error::disconnect()
{
	if(socket) network::disconnect(socket);
}

manager::manager(size_t min_threads, size_t max_threads) : free_(true)
{
	// If the network is already being managed
	if(socket_set) {
		free_ = false;
		return;
	}

	// On Unix-based systems, set sigpipe to be ignored
#if !(defined(_WIN32) || defined(__APPLE__) || defined(__AMIGAOS4__))
	WRN_NW << "ignoring SIGPIPE\n";
	signal(SIGPIPE,SIG_IGN);
#endif

	if(SDLNet_Init() == -1) {
		ERR_NW << "could not initialize SDLNet; throwing error...\n";
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

		DBG_NW << "server socket initialized: " << server_socket << "\n";
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
	connect_operation(const std::string& host, int port) : host_(host), port_(port), error_(NULL), connect_(0)
	{}

	void check_error();
	void run();

	network::connection result() const { return connect_; }

private:
	std::string host_;
	int port_;
	const char* error_;
	network::connection connect_;
};

void connect_operation::check_error()
{
	if(error_ != NULL) {
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
		error_ = "Could not connect to host";
		return;
	}

	TCPsocket sock = SDLNet_TCP_Open(&ip);
	if(!sock) {
		error_ = hostname == NULL ? "Could not bind to port" :
		                            "Could not connect to host";
		return;
	}

// Use non blocking IO
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
	{
		unsigned long mode = 1;
		ioctlsocket (((_TCPsocket*)sock)->channel, FIONBIO, &mode);
	}
#elif !defined(__BEOS__)
	int flags;
	flags = fcntl((reinterpret_cast<_TCPsocket*>(sock))->channel, F_GETFL, 0);
#if defined(O_NONBLOCK)
	flags |= O_NONBLOCK;
#elif defined(O_NDELAY)
	flags |= O_NDELAY;
#elif defined(FNDELAY)
	flags |= FNDELAY;
#endif
	if(fcntl((reinterpret_cast<_TCPsocket*>(sock))->channel, F_SETFL, flags) == -1) {
		error_ = ("Could not make socket non-blocking: " + std::string(strerror(errno))).c_str();
		return;
	}
#else
	int on = 1;
	if(setsockopt(((_TCPsocket*)sock)->channel, SOL_SOCKET, SO_NONBLOCK, &on, sizeof(int)) < 0) {
		error_ = ("Could not make socket non-blocking: " + std::string(strerror(errno))).c_str();
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
	char buf[4];
	SDLNet_Write32(0,buf);
	const int nbytes = SDLNet_TCP_Send(sock,buf,4);
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

	while(!notify_finished());
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
	const threading::async_operation_holder<connect_operation> op(new connect_operation(host,port));
	const connect_operation::RESULT res = op.operation().execute(waiter);
	if(res == connect_operation::ABORTED) {
		return 0;
	}

	op.operation().check_error();
	return op.operation().result();
}

connection accept_connection()
{
	if(!server_socket) {
		return 0;
	}

	// A connection isn't considered 'accepted' until it has sent its initial handshake.
	// The initial handshake is a 4 byte value, which is 0 for a new connection,
	// or the handle of the connection if it's trying to recover a lost connection.

	//! A list of all the sockets which have connected,
	//! but haven't had their initial handshake received.
	static std::vector<TCPsocket> pending_sockets;
	static SDLNet_SocketSet pending_socket_set = 0;

	const TCPsocket sock = SDLNet_TCP_Accept(server_socket);
	if(sock) {
		DBG_NW << "received connection. Pending handshake...\n";
		pending_sockets.push_back(sock);
		if(pending_socket_set == 0) {
			pending_socket_set = SDLNet_AllocSocketSet(512);
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

	DBG_NW << "pending socket activity...\n";

	for(std::vector<TCPsocket>::iterator i = pending_sockets.begin(); i != pending_sockets.end(); ++i) {
		if(!SDLNet_SocketReady(*i)) {
			continue;
		}

		// Receive the 4 bytes telling us if they're a new connection
		// or trying to recover a connection
		char buf[4];

		const TCPsocket sock = *i;
		SDLNet_TCP_DelSocket(pending_socket_set,sock);
		pending_sockets.erase(i);

		DBG_NW << "receiving data from pending socket...\n";

		const int len = SDLNet_TCP_Recv(sock,buf,4);
		if(len != 4) {
			WRN_NW << "pending socket disconnected\n";
			SDLNet_TCP_Close(sock);
			return 0;
		}

		const int handle = SDLNet_Read32(buf);

		DBG_NW << "received handshake from client: '" << handle << "'\n";

		const int res = SDLNet_TCP_AddSocket(socket_set,sock);
		if(res == -1) {
			SDLNet_TCP_Close(sock);

			throw network::error(_("Could not add socket to socket set"));
		}


		const connection connect = create_connection(sock,"",0);

		// Send back their connection number
		SDLNet_Write32(connect,buf);
		const int nbytes = SDLNet_TCP_Send(sock,buf,4);
		if(nbytes != 4) {
			SDLNet_TCP_Close(sock);
			throw network::error(_("Could not send initial handshake"));
		}

		waiting_sockets.insert(connect);
		sockets.push_back(connect);
		return connect;
	}

	return 0;
}

bool disconnect(connection s, bool force)
{
	if(s == 0) {
		while(sockets.empty() == false) {
			assert(sockets.back() != 0);
//			disconnect(sockets.back(), true);
			while(disconnect(sockets.back()) == false) {
				SDL_Delay(1);
			}
		}
		return true;
	}
	if (!is_server()) last_ping = 0;

	const connection_map::iterator info = connections.find(s);
	if(info != connections.end()) {
		if (!network_worker_pool::close_socket(info->second.sock, force)) {
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

connection receive_data(config& cfg, connection connection_num, int timeout)
{
	int cur_ticks = SDL_GetTicks();
	while(timeout >= 0) {
		const connection res = receive_data(cfg,connection_num);
		if(res != 0) {
			return res;
		}

		if(timeout > 0) {
			SDL_Delay(1);
		}

		const int ticks = SDL_GetTicks();
		timeout -= maximum<int>(1,(ticks - cur_ticks));
		cur_ticks = ticks;
	}

	return 0;
}

connection receive_data(config& cfg, connection connection_num)
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
				char buf[4];
				int len = SDLNet_TCP_Recv(sock,buf,4);
				if(len != 4) {
					throw error("Remote host disconnected",*i);
				}

				const int remote_handle = SDLNet_Read32(buf);
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
	sock = network_worker_pool::get_received_data(sock,cfg);
	if(!cfg.empty()) {
		LOG_NW << "RECEIVED from: " << connection_num << ": " << cfg.debug();
	}
	if(sock == NULL) {
		if(last_ping != 0 && ping_timeout != 0 && !is_server() ) {
			check_timeout();
		}
		return 0;
	}

	SDLNet_TCP_AddSocket(socket_set,sock);

	connection result = 0;
	for(connection_map::const_iterator j = connections.begin(); j != connections.end(); ++j) {
		if(j->second.sock == sock) {
			result = j->first;
			break;
		}
	}

	assert(result != 0);
	waiting_sockets.insert(result);
	if(!is_server()) {
		const time_t& now = time(NULL);
		const string_map::const_iterator ping = cfg.values.find("ping");
		if (ping != cfg.values.end()) {
			LOG_NW << "Lag: " << (now - lexical_cast<time_t>(cfg["ping"])) << "\n";
			last_ping = now;
		} else if (last_ping != 0) {
			last_ping = now;
		}			
	}
	return result;
}

//! @todo Note the gzipped parameter should be removed later, we want to send
//! all data gzipped. This can be done once the campaign server is also updated
//! to work with gzipped data.
void send_data(const config& cfg, connection connection_num, const bool gzipped)
{
	DBG_NW << "in send_data()...\n";
	
	if(cfg.empty()) {
		return;
	}

	if(bad_sockets.count(connection_num) || bad_sockets.count(0)) {
		return;
	}

//	log_scope2(network, "sending data");
	if(!connection_num) {
		DBG_NW << "sockets: " << sockets.size() << "\n";
		for(sockets_list::const_iterator i = sockets.begin();
		    i != sockets.end(); ++i) {
			DBG_NW << "server socket: " << server_socket << "\ncurrent socket: " << *i << "\n";
			send_data(cfg,*i, gzipped);
		}
		return;
	}

	const connection_map::iterator info = connections.find(connection_num);
	if (info == connections.end()) {
		ERR_NW << "Error: socket: " << connection_num
			<< "\tnot found in connection_map. Not sending...\n";
		return;
	}

	LOG_NW << "SENDING to: " << connection_num << ": " << cfg.debug();
	network_worker_pool::queue_data(info->second.sock, cfg, gzipped);
}

void process_send_queue(connection, size_t)
{
	check_error();
}

//! @todo Note the gzipped parameter should be removed later.
void send_data_all_except(const config& cfg, connection connection_num, const bool gzipped)
{
	for(sockets_list::const_iterator i = sockets.begin(); i != sockets.end(); ++i) {
		if(*i == connection_num) {
			continue;
		}

		send_data(cfg,*i, gzipped);
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


std::pair<unsigned int,size_t> get_thread_state()
{
	return network_worker_pool::thread_state();
}

statistics get_send_stats(connection handle)
{
	return network_worker_pool::get_current_transfer_stats(handle == 0 ? get_socket(sockets.back()) : get_socket(handle)).first;
}
statistics get_receive_stats(connection handle)
{
	return network_worker_pool::get_current_transfer_stats(handle == 0 ? get_socket(sockets.back()) : get_socket(handle)).second;
}

} // end namespace network
