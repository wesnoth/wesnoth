/* $Id$ */
/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! Network worker handles data transfers in threads
//! Remember to use mutexs as little as possible
//! All global vars should be used in mutex
//! FIXME: @todo: All code which holds a mutex should run O(1) time
//! for scalability. Implement read/write locks.
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

#ifndef NUM_SHARDS
#define NUM_SHARDS 1
#endif

unsigned int waiting_threads[NUM_SHARDS];
size_t min_threads = 0;
size_t max_threads = 0;


int get_shard(TCPsocket sock) { return intptr_t(sock)%NUM_SHARDS; }

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


bool managed = false;
typedef std::vector< buffer* > buffer_set;
buffer_set bufs[NUM_SHARDS];

struct schema_pair
{
	compression_schema incoming, outgoing;
};

typedef std::map<TCPsocket,schema_pair> schema_map;

schema_map schemas; //schemas_mutex

//a queue of sockets that we are waiting to receive on
typedef std::vector<TCPsocket> receive_list;
receive_list pending_receives[NUM_SHARDS];

typedef std::deque<buffer*> received_queue;
received_queue received_data_queue;  // receive_mutex

enum SOCKET_STATE { SOCKET_READY, SOCKET_LOCKED, SOCKET_ERRORED, SOCKET_INTERRUPT };
typedef std::map<TCPsocket,SOCKET_STATE> socket_state_map;
typedef std::map<TCPsocket, std::pair<network::statistics,network::statistics> > socket_stats_map;

socket_state_map sockets_locked[NUM_SHARDS];
socket_stats_map transfer_stats; // stats_mutex

int socket_errors[NUM_SHARDS];
threading::mutex* shard_mutexes[NUM_SHARDS];
threading::mutex* stats_mutex = NULL;
threading::mutex* schemas_mutex = NULL;
threading::mutex* received_mutex = NULL;
threading::condition* cond[NUM_SHARDS];

std::map<Uint32,threading::thread*> threads[NUM_SHARDS];
std::vector<Uint32> to_clear[NUM_SHARDS];

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
		bool update_stats=false, int timeout_ms=10000)
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
					poll_res = poll(&fd, 1, timeout_ms - time_used);
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
					time_left = timeout_ms - time_used;
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
			// We got some data from server so reset start time so slow conenction won't timeout.
			startTicks = SDL_GetTicks();
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
			const int shard = get_shard(sock);
			// check if the socket is still locked
			const threading::lock lock(*shard_mutexes[shard]);
			if(sockets_locked[shard][sock] != SOCKET_LOCKED)
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

inline void check_socket_result(TCPsocket& sock, SOCKET_STATE& result)
{
	const int shard = get_shard(sock);
	const threading::lock lock(*shard_mutexes[shard]);
	socket_state_map::iterator lock_it = sockets_locked[shard].find(sock);
	assert(lock_it != sockets_locked[shard].end());
	lock_it->second = result;
	if(result == SOCKET_ERRORED) {
		++socket_errors[shard];
	}
}

static int process_queue(void* shard_num)
{
	int shard = static_cast<int>(reinterpret_cast<intptr_t>(shard_num));
	DBG_NW << "thread started...\n";
	for(;;) {

		//if we find a socket to send data to, sent_buf will be non-NULL. If we find a socket
		//to receive data from, sent_buf will be NULL. 'sock' will always refer to the socket
		//that data is being sent to/received from
		TCPsocket sock = NULL;
		buffer* sent_buf = 0;

		{
			const threading::lock lock(*shard_mutexes[shard]);
			while(managed && !to_clear[shard].empty()) {
				Uint32 tmp = to_clear[shard].back();
				to_clear[shard].pop_back();
				threading::thread *zombie = threads[shard][tmp];
				threads[shard].erase(tmp);
				delete zombie;

			}
			if(min_threads && waiting_threads[shard] >= min_threads) {
					DBG_NW << "worker thread exiting... not enough jobs\n";
					to_clear[shard].push_back(threading::get_current_thread_id());
					return 0;
			}
			waiting_threads[shard]++;
			for(;;) {

				buffer_set::iterator itor = bufs[shard].begin(), itor_end = bufs[shard].end();
				for(; itor != itor_end; ++itor) {
					socket_state_map::iterator lock_it = sockets_locked[shard].find((*itor)->sock);
					assert(lock_it != sockets_locked[shard].end());
					if(lock_it->second == SOCKET_READY) {
						lock_it->second = SOCKET_LOCKED;
						sent_buf = *itor;
						sock = sent_buf->sock;
						bufs[shard].erase(itor);
						break;
					}
				}

				if(sock == NULL) {
					receive_list::iterator itor = pending_receives[shard].begin(), itor_end = pending_receives[shard].end();
					for(; itor != itor_end; ++itor) {
						socket_state_map::iterator lock_it = sockets_locked[shard].find(*itor);
						assert(lock_it != sockets_locked[shard].end());
						if(lock_it->second == SOCKET_READY) {
							lock_it->second = SOCKET_LOCKED;
							sock = *itor;
							pending_receives[shard].erase(itor);
							break;
						}
					}
				}

				if(sock != NULL) {
					break;
				}

				if(managed == false) {
					DBG_NW << "worker thread exiting...\n";
					waiting_threads[shard]--;
					to_clear[shard].push_back(threading::get_current_thread_id());
					return 0;
				}

				cond[shard]->wait(*shard_mutexes[shard]); // temporarily release the mutex and wait for a buffer
			}
			waiting_threads[shard]--;
			// if we are the last thread in the pool, create a new one
			if(!waiting_threads[shard] && managed == true) {
				// max_threads of 0 is unlimited
				if(!max_threads || max_threads >threads[shard].size()) {
					threading::thread * tmp = new threading::thread(process_queue,shard_num);
					threads[shard][tmp->get_id()] =tmp;
				}
			}
		}

		assert(sock);

		DBG_NW << "thread found a buffer...\n";

		SOCKET_STATE result = SOCKET_READY;
		std::vector<char> buf;

		if(sent_buf) {
			result = send_buffer(sent_buf->sock, sent_buf->config_buf, sent_buf->gzipped);
			delete sent_buf;
		} else {
			result = receive_buf(sock,buf);
		}


		if(result != SOCKET_READY || buf.empty()) 
		{
			check_socket_result(sock,result);
		       	continue;
		}
		//if we received data, add it to the queue
		buffer* received_data = new buffer(sock);
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
		check_socket_result(sock,result);
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
		for(int i = 0; i != NUM_SHARDS; ++i) {
			shard_mutexes[i] = new threading::mutex();
			cond[i] = new threading::condition();
		}
		stats_mutex = new threading::mutex();
		schemas_mutex = new threading::mutex();
		received_mutex = new threading::mutex();

		min_threads = p_min_threads;
		max_threads = p_max_threads;

		for(int shard = 0; shard != NUM_SHARDS; ++shard) {
			const threading::lock lock(*shard_mutexes[shard]);
			for(size_t n = 0; n != min_threads; ++n) {
				threading::thread * tmp = new threading::thread(process_queue,(void*)intptr_t(shard));
				threads[shard][tmp->get_id()] = tmp;
			}
		}
	}
}

manager::~manager()
{
	if(active_) {
		
		managed = false;

		for(int shard = 0; shard != NUM_SHARDS; ++shard) {
			{
				const threading::lock lock(*shard_mutexes[shard]);
				socket_errors[shard] = 0;
			}

			cond[shard]->notify_all();

			for(std::map<Uint32,threading::thread*>::const_iterator i = threads[shard].begin(); i != threads[shard].end(); ++i) {
				DBG_NW << "waiting for thread " << i->first << " to exit...\n";
				delete i->second;
				DBG_NW << "thread exited...\n";
			}

			threads[shard].clear();
			delete shard_mutexes[shard];
			shard_mutexes[shard] = NULL;
			delete cond[shard];
			cond[shard] = NULL;
		}

		delete  stats_mutex;
		delete schemas_mutex;
		delete received_mutex;
		stats_mutex = 0;
		schemas_mutex = 0;
		received_mutex = 0;

		for(int i = 0; i != NUM_SHARDS; ++i) {
			sockets_locked[i].clear();
		}
		transfer_stats.clear();

		DBG_NW << "exiting manager::~manager()\n";
	}
}

void receive_data(TCPsocket sock)
{
	{
		const int shard = get_shard(sock);
		const threading::lock lock(*shard_mutexes[shard]);
		pending_receives[shard].push_back(sock);

		socket_state_map::const_iterator i = sockets_locked[shard].insert(std::pair<TCPsocket,SOCKET_STATE>(sock,SOCKET_READY)).first;
		if(i->second == SOCKET_READY || i->second == SOCKET_ERRORED) {
			cond[shard]->notify_one();
		}
	}
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
		buffer* buf = *itor;
		received_data_queue.erase(itor);
		delete buf;
		throw config::error(error);
	} else {
		cfg.swap((*itor)->config_buf);
		const TCPsocket res = (*itor)->sock;
		buffer* buf = *itor;
		received_data_queue.erase(itor);
		delete buf;
		return res;
	}
}

void queue_data(TCPsocket sock,const  config& buf, const bool gzipped)
{
	DBG_NW << "queuing data...\n";

	buffer* queued_buf = new buffer(sock);
	queued_buf->config_buf = buf;
	queued_buf->gzipped = gzipped;
	{
		const int shard = get_shard(sock);
		const threading::lock lock(*shard_mutexes[shard]);

		bufs[shard].push_back(queued_buf);

		socket_state_map::const_iterator i = sockets_locked[shard].insert(std::pair<TCPsocket,SOCKET_STATE>(sock,SOCKET_READY)).first;
		if(i->second == SOCKET_READY || i->second == SOCKET_ERRORED) {
			cond[shard]->notify_one();
		}
	}

}

namespace
{

//! Caller has to make sure to own the mutex for this shard
void remove_buffers(TCPsocket sock)
{
	{
		const int shard = get_shard(sock);
		for(buffer_set::iterator i = bufs[shard].begin(); i != bufs[shard].end();) {
			if ((*i)->sock == sock)
			{
				buffer* buf = *i;
				i = bufs[shard].erase(i);
				delete buf;
			}
			else
			{
				++i;
			}
		}
	}

	{
		const threading::lock lock_receive(*received_mutex);

		for(received_queue::iterator j = received_data_queue.begin(); j != received_data_queue.end(); ) {
			if((*j)->sock == sock) {
				buffer *buf = *j;
				j = received_data_queue.erase(j);
				delete buf;
			} else {
				++j;
			}
		}
	}
}

} // anonymous namespace

bool is_locked(const TCPsocket sock) {
	const int shard = get_shard(sock);
	const threading::lock lock(*shard_mutexes[shard]);
	const socket_state_map::iterator lock_it = sockets_locked[shard].find(sock);
	if (lock_it == sockets_locked[shard].end()) return false;
	return (lock_it->second == SOCKET_LOCKED);
}

bool close_socket(TCPsocket sock, bool force)
{
	{
		const int shard = get_shard(sock);
		const threading::lock lock(*shard_mutexes[shard]);

		pending_receives[shard].erase(std::remove(pending_receives[shard].begin(),pending_receives[shard].end(),sock),pending_receives[shard].end());
		const socket_state_map::iterator lock_it = sockets_locked[shard].find(sock);
		if(lock_it == sockets_locked[shard].end()) {
			remove_buffers(sock);
			return true;
		}
		{	
			const threading::lock lock_schemas(*schemas_mutex);
			schemas.erase(sock);
		}

		if (!(lock_it->second == SOCKET_LOCKED || lock_it->second == SOCKET_INTERRUPT) || force) {
			sockets_locked[shard].erase(lock_it);
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
	for(int shard = 0; shard != NUM_SHARDS; ++shard) {
		const threading::lock lock(*shard_mutexes[shard]);
		if(socket_errors[shard] > 0) {
			for(socket_state_map::iterator i = sockets_locked[shard].begin(); i != sockets_locked[shard].end();) {
				if(i->second == SOCKET_ERRORED) {
					--socket_errors[shard];
					const TCPsocket sock = i->first;
					sockets_locked[shard].erase(i++);
					pending_receives[shard].erase(std::remove(pending_receives[shard].begin(),pending_receives[shard].end(),sock),pending_receives[shard].end());
					remove_buffers(sock);
					const threading::lock lock_schema(*schemas_mutex);
					schemas.erase(sock);
					return sock;
				}
				else
				{
					++i;
				}
			}
		}

		socket_errors[shard] = 0;
	}

	return 0;
}

std::pair<network::statistics,network::statistics> get_current_transfer_stats(TCPsocket sock)
{
	const threading::lock lock(*stats_mutex);
	return transfer_stats[sock];
}

} // network_worker_pool namespace


