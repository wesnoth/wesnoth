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

//! Network worker handles data transfers in threads
//! Remmber to use mutexs as little as possible
//! All global vars should be used in mutex
//! FIXME: TODO: All code in mutex shoudl run O(1) time
//! for scaleablity. Implement read/write locks.
//!  (postponed for 1.5)

#include "global.hpp"

#include "log.hpp"
#include "network_worker.hpp"
#include "network.hpp"
#include "thread.hpp"
//#include "wesconfig.h"
#include "serialization/binary_or_text.hpp"
#include "serialization/binary_wml.hpp"
#include "serialization/parser.hpp"

#include <algorithm>
#include <cassert>
#include <cerrno>
#include <cstring>
#include <deque>
#include <iostream>
#include <map>
#include <vector>

#include <boost/iostreams/filter/gzip.hpp>

#ifdef __AMIGAOS4__
#include <unistd.h>
//#include <sys/clib2_net.h>
#endif

#if defined(_WIN32) || defined(__WIN32__) || defined (WIN32)
#  undef INADDR_ANY
#  undef INADDR_BROADCAST
#  undef INADDR_NONE
#  include <windows.h>
#  define USE_SELECT 1
#else
#  include <sys/types.h>
#  include <sys/socket.h>
#  ifdef __BEOS__
#    include <socket.h>
#  else
#    include <fcntl.h>
#  endif
#  define SOCKET int
#  ifdef HAVE_POLL_H
#    define USE_POLL 1
#    include <poll.h>
#  elif defined(HAVE_SYS_POLL_H)
#    define USE_POLL 1
#    include <sys/poll.h>
#  endif
#  ifndef USE_POLL
#    define USE_SELECT 1
#    ifdef HAVE_SYS_SELECT_H
#      include <sys/select.h>
#    else
#      include <sys/time.h>
#      include <sys/types.h>
#      include <unistd.h>
#    endif
#  endif
#endif

#define DBG_NW LOG_STREAM(debug, network)
#define LOG_NW LOG_STREAM(info, network)
#define ERR_NW LOG_STREAM(err, network)

namespace {
struct _TCPsocket {
	int ready;
	SOCKET channel;
	IPaddress remoteAddress;
	IPaddress localAddress;
	int sflag;
};
unsigned int waiting_threads = 0; // management_mutex
size_t min_threads = 0; // management_mutex
size_t max_threads = 0; // management_mutex

struct buffer {
	explicit buffer(TCPsocket sock) : 
		sock(sock),
		config_buf(),
		config_error(""),
		gzipped(false)
		{}

	TCPsocket sock;
	mutable config config_buf;
	std::string config_error;
	//! Do we wish to send the data gzipped, if not use binary wml.
	//! This needs to stay until the last user of binary_wml has
	//! been removed.
	bool gzipped;

};

bool managed = false; // management_mutex
typedef std::vector< buffer * > buffer_set;
buffer_set bufs; // management_mutex

struct schema_pair
{
	compression_schema incoming, outgoing;
};

typedef std::map<TCPsocket,schema_pair> schema_map;

schema_map schemas; //schemas_mutex

//a queue of sockets that we are waiting to receive on
typedef std::vector<TCPsocket> receive_list;
receive_list pending_receives; // management_mutex

typedef std::deque<buffer *> received_queue;
received_queue received_data_queue;  // receive_mutex

enum SOCKET_STATE { SOCKET_READY, SOCKET_LOCKED, SOCKET_ERRORED, SOCKET_INTERRUPT };
typedef std::map<TCPsocket,SOCKET_STATE> socket_state_map;
typedef std::map<TCPsocket, std::pair<network::statistics,network::statistics> > socket_stats_map;

socket_state_map sockets_locked;  // management_mutex
socket_stats_map transfer_stats; // stats_mutex

int socket_errors = 0; // management_mutex
threading::mutex* management_mutex = NULL;
threading::mutex* stats_mutex = NULL;
threading::mutex* schemas_mutex = NULL;
threading::mutex* received_mutex = NULL;
threading::condition* cond = NULL;

std::map<Uint32,threading::thread*> threads; // management_mutex
std::vector<Uint32> to_clear; // management_mutex 

int receive_bytes(TCPsocket s, char* buf, size_t nbytes)
{
#ifdef NETWORK_USE_RAW_SOCKETS
	_TCPsocket* sock = reinterpret_cast<_TCPsocket*>(s);
	int res = 0;
	do {
		errno = 0;
		res = recv(sock->channel, buf, nbytes, MSG_DONTWAIT);
	} while(errno == EINTR);
	sock->ready = 0;
	return res;
#else
	return SDLNet_TCP_Recv(s, buf, nbytes);
#endif
}

bool receive_with_timeout(TCPsocket s, char* buf, size_t nbytes,
		bool update_stats=false, int timeout_ms=60000)
{
	int startTicks = SDL_GetTicks();
	int time_used = 0;
	while(nbytes > 0) {
		const int bytes_read = receive_bytes(s, buf, nbytes);
		if(bytes_read == 0) {
			return false;
		} else if(bytes_read < 0) {
#if defined(EAGAIN) && !defined(__BEOS__) && !defined(_WIN32)
			if(errno == EAGAIN)
#elif defined(EWOULDBLOCK)
			if(errno == EWOULDBLOCK)
#else
			// Ignore the error.
			if(true)
#endif
			{
				//TODO: consider replacing this with a select call
				time_used = SDL_GetTicks() - startTicks;
				if(time_used >= timeout_ms) {
					return false;
				}
#ifdef USE_POLL
				struct pollfd fd = { ((_TCPsocket*)s)->channel, POLLIN, 0 };
				int poll_res;
				do {
					time_used = SDL_GetTicks() - startTicks;
					poll_res = poll(&fd, 1, minimum<int>(15000,timeout_ms - time_used));
				} while(poll_res == -1 && errno == EINTR);

#elif defined(USE_SELECT)
				fd_set readfds;
				FD_ZERO(&readfds);
				FD_SET(((_TCPsocket*)s)->channel, &readfds);
				int retval;
				int time_left;
				struct timeval tv;

				do {
					time_used = SDL_GetTicks() - startTicks;
					time_left = minimum<int>(15000, timeout_ms - time_used);
					tv.tv_sec = time_left/1000;
					tv.tv_usec = time_left % 1000;
					retval = select(((_TCPsocket*)s)->channel + 1, &readfds, NULL, NULL, &tv);
				} while(retval == -1 && errno == EINTR);

#elif
				SDL_Delay(5);
#endif
			} else {
				return false;
			}
		} else {

			buf += bytes_read;
			if(update_stats) {
				const threading::lock lock(*stats_mutex);
				transfer_stats[s].second.transfer(static_cast<size_t>(bytes_read));
			}
			
			if(bytes_read > static_cast<int>(nbytes)) {
				return false;
			}
			nbytes -= bytes_read;
		}
	}

	return true;
}

static SOCKET_STATE send_buffer(TCPsocket sock, config& config_in, const bool gzipped) {
#ifdef __BEOS__
	int timeout = 15000;
#endif
	size_t upto = 0;
	std::ostringstream compressor;
	if(gzipped) {
//		std::cerr << "send gzipped\n.";
		config_writer writer(compressor, true, "");
		writer.write(config_in);
	} else {
		compression_schema *compress;
//		std::cerr << "send binary wml\n.";
		{
			const threading::lock lock(*schemas_mutex);
			compress = &schemas.insert(std::pair<TCPsocket,schema_pair>(sock,schema_pair())).first->second.outgoing;
		}
		write_compressed(compressor, config_in, *compress);
	}
	std::string const &value = compressor.str();
	std::vector<char> buf(4 + value.size() + 1);
	SDLNet_Write32(value.size()+1,&buf[0]);
	std::copy(value.begin(),value.end(),buf.begin()+4);
	buf.back() = 0;
	size_t size = buf.size();
	{
		const threading::lock lock(*stats_mutex);
		transfer_stats[sock].first.fresh_current(size);
	}
#ifdef __BEOS__
	while(upto < size && timeout > 0) {
#else
	while(true) {
#endif
		{
			// check if the socket is still locked
			const threading::lock lock(*management_mutex);
			if(sockets_locked[sock] != SOCKET_LOCKED)
				return SOCKET_ERRORED;
		}
		const int res = SDLNet_TCP_Send(sock, &buf[upto], static_cast<int>(size - upto));

		if(res == static_cast<int>(size - upto)) {
			{
				const threading::lock lock(*stats_mutex);
				transfer_stats[sock].first.transfer(static_cast<size_t>(res));
			}
			return SOCKET_READY;
		}
#if defined(EAGAIN) && !defined(__BEOS__) && !defined(_WIN32)
		if(errno == EAGAIN)
#elif defined(EWOULDBLOCK)
		if(errno == EWOULDBLOCK)
#endif
		{
			// update how far we are
			upto += static_cast<size_t>(res);
			{
				const threading::lock lock(*stats_mutex);
				transfer_stats[sock].first.transfer(static_cast<size_t>(res));
			}

#ifdef USE_POLL
			struct pollfd fd = { ((_TCPsocket*)sock)->channel, POLLOUT, 0 };
			int poll_res;
			do {
				poll_res = poll(&fd, 1, 15000);
			} while(poll_res == -1 && errno == EINTR);

			if(poll_res > 0)
				continue;
#elif defined(USE_SELECT) && !defined(__BEOS__)
			fd_set writefds;
			FD_ZERO(&writefds);
			FD_SET(((_TCPsocket*)sock)->channel, &writefds);
			int retval;
			struct timeval tv;
			tv.tv_sec = 15;
			tv.tv_usec = 0;

			do {
				retval = select(((_TCPsocket*)sock)->channel + 1, NULL, &writefds, NULL, &tv);
			} while(retval == -1 && errno == EINTR);

			if(retval > 0)
				continue;
#elif defined(__BEOS__)
			if(res > 0) {
				// some data was sent, reset timeout
				timeout = 15000;
			} else {
				// sleep for 100 milliseconds
				SDL_Delay(100);
				timeout -= 100;
			}
			continue;
#endif
		}

		return SOCKET_ERRORED;
	}
}

static SOCKET_STATE receive_buf(TCPsocket sock, std::vector<char>& buf)
{
	char num_buf[4];
	bool res = receive_with_timeout(sock,num_buf,4,false);

	if(!res) {
		return SOCKET_ERRORED;
	}

	const int len = SDLNet_Read32(num_buf);

	if(len < 1 || len > 100000000) {
		return SOCKET_ERRORED;
	}

	buf.resize(len);
	char* beg = &buf[0];
	const char* const end = beg + len;

	{
		const threading::lock lock(*stats_mutex);
		transfer_stats[sock].second.fresh_current(len);
	}

	res = receive_with_timeout(sock, beg, end - beg, true);
	if(!res) {
		return SOCKET_ERRORED;
	}

	return SOCKET_READY;
}

static int process_queue(void*)
{
	DBG_NW << "thread started...\n";
	for(;;) {

		//if we find a socket to send data to, sent_buf will be non-NULL. If we find a socket
		//to receive data from, sent_buf will be NULL. 'sock' will always refer to the socket
		//that data is being sent to/received from
		TCPsocket sock = NULL;
		buffer *sent_buf = NULL;

		{
			const threading::lock lock(*management_mutex);
			while(managed && !to_clear.empty()) {
				Uint32 tmp = to_clear.back();
				to_clear.pop_back();
				threading::thread *zombie = threads[tmp];
				threads.erase(tmp);
				delete zombie;

			}
			if(min_threads && waiting_threads >= min_threads) {
					DBG_NW << "worker thread exiting... not enough jobs\n";
					to_clear.push_back(threading::get_current_thread_id());
					return 0;
			}
			waiting_threads++;

			for(;;) {

				buffer_set::iterator itor = bufs.begin(), itor_end = bufs.end();
				for(; itor != itor_end; ++itor) {
					socket_state_map::iterator lock_it = sockets_locked.find((*itor)->sock);
					assert(lock_it != sockets_locked.end());
					if(lock_it->second == SOCKET_READY) {
						lock_it->second = SOCKET_LOCKED;
						sent_buf = *itor;
						sock = sent_buf->sock;
						bufs.erase(itor);
						break;
					}
				}

				if(sock == NULL) {
					receive_list::iterator itor = pending_receives.begin(), itor_end = pending_receives.end();
					for(; itor != itor_end; ++itor) {
						socket_state_map::iterator lock_it = sockets_locked.find(*itor);
						assert(lock_it != sockets_locked.end());
						if(lock_it->second == SOCKET_READY) {
							lock_it->second = SOCKET_LOCKED;
							sock = *itor;
							pending_receives.erase(itor);
							break;
						}
					}
				}

				if(sock != NULL) {
					break;
				}

				if(managed == false) {
					DBG_NW << "worker thread exiting...\n";
					waiting_threads--;
					to_clear.push_back(threading::get_current_thread_id());
					return 0;
				}

				cond->wait(*management_mutex); // temporarily release the mutex and wait for a buffer
			}
			waiting_threads--;
			// if we are the last thread in the pool, create a new one
			if(!waiting_threads && managed == true) {
				// max_threads of 0 is unlimited
				if(!max_threads || max_threads >threads.size()) {
					threading::thread * tmp = new threading::thread(process_queue,NULL);
					threads[tmp->get_id()] =tmp;
				}
			}
		}

		assert(sock);

		DBG_NW << "thread found a buffer...\n";

		SOCKET_STATE result = SOCKET_READY;
		std::vector<char> buf;

		if(sent_buf != NULL) {
			result = send_buffer(sent_buf->sock, sent_buf->config_buf, sent_buf->gzipped);
			delete sent_buf;
			sent_buf = NULL;
		} else {
			result = receive_buf(sock,buf);
		}

		{
			const threading::lock lock(*management_mutex);
			socket_state_map::iterator lock_it = sockets_locked.find(sock);
			assert(lock_it != sockets_locked.end());
			lock_it->second = result;
			if(result == SOCKET_ERRORED) {
				++socket_errors;
			}

			if(result != SOCKET_READY || buf.empty()) continue;
		}
		//if we received data, add it to the queue
		buffer *received_data = new buffer(sock);
		std::string buffer(buf.begin(), buf.end());
		std::istringstream stream(buffer);
		// Binary wml starts with a char < 4, the first char of a gzip header is 31
		// so test that here and use the proper reader.
		try {
			if(stream.peek() == 31) {
				read_gz(received_data->config_buf, stream);
			} else {
				compression_schema *compress;
				{
					const threading::lock lock_schemas(*schemas_mutex);
					compress = &schemas.insert(std::pair<TCPsocket,schema_pair>(sock,schema_pair())).first->second.incoming;
				}
				read_compressed(received_data->config_buf, stream, *compress);
			}
		} catch(config::error &e)
		{
			received_data->config_error = e.message;
		}

		{
			// Now add data
			const threading::lock lock_received(*received_mutex);
			received_data_queue.push_back(received_data);
		}
	}
	// unreachable
}

} //anonymous namespace

namespace network_worker_pool
{

manager::manager(size_t p_min_threads,size_t p_max_threads) : active_(!managed)
{
	if(active_) {
		managed = true;
		management_mutex = new threading::mutex();
		stats_mutex = new threading::mutex();
		schemas_mutex = new threading::mutex();
		received_mutex = new threading::mutex();

		cond = new threading::condition();
		const threading::lock lock(*management_mutex);
		min_threads = p_min_threads;
		max_threads = p_max_threads;

		for(size_t n = 0; n != min_threads; ++n) {
			threading::thread * tmp = new threading::thread(process_queue,NULL);
			threads[tmp->get_id()] =tmp;
		}
	}
}

manager::~manager()
{
	if(active_) {
		{
			const threading::lock lock(*management_mutex);
			managed = false;
			socket_errors = 0;
		}
		cond->notify_all();

		for(std::map<Uint32,threading::thread*>::const_iterator i = threads.begin(); i != threads.end(); ++i) {
			DBG_NW << "waiting for thread " << i->first << " to exit...\n";
			delete i->second;
			DBG_NW << "thread exited...\n";
		}

		threads.clear();

		delete management_mutex;
		delete  stats_mutex;
		delete schemas_mutex;
		delete received_mutex;
		delete cond;
		management_mutex = NULL;
		stats_mutex = 0;
		schemas_mutex = 0;
		received_mutex = 0;
		cond = NULL;

		sockets_locked.clear();
		transfer_stats.clear();

		DBG_NW << "exiting manager::~manager()\n";
	}
}

std::pair<unsigned int,size_t> thread_state()
{
	const threading::lock lock(*management_mutex);
	return std::pair<unsigned int,size_t>(waiting_threads,threads.size());

}

void receive_data(TCPsocket sock)
{
	{
		const threading::lock lock(*management_mutex);
		pending_receives.push_back(sock);

		sockets_locked.insert(std::pair<TCPsocket,SOCKET_STATE>(sock,SOCKET_READY));
	}

	cond->notify_one();
}

TCPsocket get_received_data(TCPsocket sock, config& cfg)
{
	const threading::lock lock_received(*received_mutex);
	received_queue::iterator itor = received_data_queue.begin();
	if(sock != NULL) {
		for(; itor != received_data_queue.end(); ++itor) {
			if((*itor)->sock == sock) {
				break;
			}
		}
	}

	if(itor == received_data_queue.end()) {
		return NULL;
	} else if (!(*itor)->config_error.empty()){
		// throw the error in parent thread
		std::string error = (*itor)->config_error;
		buffer *buf = *itor;
		received_data_queue.erase(itor);
		delete buf;
		throw config::error(error);
	} else {
		cfg = (*itor)->config_buf;
		const TCPsocket res = (*itor)->sock;
		buffer *buf = *itor;
		received_data_queue.erase(itor);
		delete buf;
		return res;
	}
}

void queue_data(TCPsocket sock,const  config& buf, const bool gzipped)
{
	DBG_NW << "queuing data...\n";

	buffer *queued_buf = new buffer(sock);
	queued_buf->config_buf = buf;
	queued_buf->gzipped = gzipped;
	{
		const threading::lock lock(*management_mutex);

		bufs.push_back(queued_buf);

		sockets_locked.insert(std::pair<TCPsocket,SOCKET_STATE>(sock,SOCKET_READY));
	}

	cond->notify_one();
}

namespace
{

//! Caller has to make sure to own management_mutex
static void remove_buffers(TCPsocket sock)
{
	{
		buffer_set new_bufs;
		new_bufs.reserve(bufs.size());
		for(buffer_set::iterator i = bufs.begin(), i_end = bufs.end(); i != i_end; ++i) {
			if ((*i)->sock == sock)
				delete *i;
			else
				new_bufs.push_back(*i);
		}
		bufs.swap(new_bufs);
	}

	{
		const threading::lock lock_receive(*received_mutex);

		for(received_queue::iterator j = received_data_queue.begin(); j != received_data_queue.end(); ) {
			if((*j)->sock == sock) {
				j = received_data_queue.erase(j);
			} else {
				++j;
			}
		}
	}
}

} // anonymous namespace

bool is_locked(const TCPsocket sock) {
	const threading::lock lock(*management_mutex);
	const socket_state_map::iterator lock_it = sockets_locked.find(sock);
	if (lock_it == sockets_locked.end()) return false;
	return (lock_it->second == SOCKET_LOCKED);
}

bool close_socket(TCPsocket sock, bool force)
{
	{
		const threading::lock lock(*management_mutex);

		pending_receives.erase(std::remove(pending_receives.begin(),pending_receives.end(),sock),pending_receives.end());
		const socket_state_map::iterator lock_it = sockets_locked.find(sock);
		if(lock_it == sockets_locked.end()) {
			remove_buffers(sock);
			return true;
		}
		{	
			const threading::lock lock_schemas(*schemas_mutex);
			schemas.erase(sock);
		}

		if (!(lock_it->second == SOCKET_LOCKED || lock_it->second == SOCKET_INTERRUPT) || force) {
			sockets_locked.erase(lock_it);
			remove_buffers(sock);
			return true;
		} else {
			lock_it->second = SOCKET_INTERRUPT;
			return false;
		}
	
	}


}

TCPsocket detect_error()
{
	const threading::lock lock(*management_mutex);
	if(socket_errors > 0) {
		for(socket_state_map::iterator i = sockets_locked.begin(); i != sockets_locked.end(); ++i) {
			if(i->second == SOCKET_ERRORED) {
				--socket_errors;
				const TCPsocket sock = i->first;
				sockets_locked.erase(i);
				pending_receives.erase(std::remove(pending_receives.begin(),pending_receives.end(),sock),pending_receives.end());
				remove_buffers(sock);
				const threading::lock lock_schema(*schemas_mutex);
				schemas.erase(sock);
				return sock;
			}
		}
	}

	socket_errors = 0;

	return 0;
}

std::pair<network::statistics,network::statistics> get_current_transfer_stats(TCPsocket sock)
{
	const threading::lock lock(*stats_mutex);
	return transfer_stats[sock];
}

} // network_worker_pool namespace


