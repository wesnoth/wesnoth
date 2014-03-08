/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
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
 * Network worker handles data transfers in threads
 * Remember to use mutexs as little as possible
 * All global vars should be used in mutex
 * FIXME: @todo All code which holds a mutex should run O(1) time
 * for scalability. Implement read/write locks.
 *  (postponed for 1.5)
 */

#include "global.hpp"

#include "scoped_resource.hpp"
#include "log.hpp"
#include "network_worker.hpp"
#include "filesystem.hpp"
#include "thread.hpp"
#include "serialization/binary_or_text.hpp"
#include "serialization/parser.hpp"
#include "wesconfig.h"

#include <boost/iostreams/filter/gzip.hpp>
#include <boost/exception/info.hpp>

#include <cerrno>
#include <deque>
#include <sstream>

#ifdef HAVE_SENDFILE
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#endif


#if defined(_WIN32) || defined(__WIN32__) || defined (WIN32)
#  undef INADDR_ANY
#  undef INADDR_BROADCAST
#  undef INADDR_NONE
#  ifndef NOMINMAX
#   define NOMINMAX
#  endif
#  include <windows.h>
#  define USE_SELECT 1
typedef int socklen_t;
#else
#  include <sys/types.h>
#  include <sys/socket.h>
#  include <fcntl.h>
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

static lg::log_domain log_network("network");
#define DBG_NW LOG_STREAM(debug, log_network)
#define LOG_NW LOG_STREAM(info, log_network)
#define ERR_NW LOG_STREAM(err, log_network)

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

size_t get_shard(TCPsocket sock) { return reinterpret_cast<uintptr_t>(sock)%NUM_SHARDS; }

struct buffer {
	explicit buffer(TCPsocket sock) :
		sock(sock),
		config_buf(),
		config_error(""),
		stream(),
		raw_buffer()
		{}

	TCPsocket sock;
	mutable config config_buf;
	std::string config_error;
	std::ostringstream stream;

	/**
	 * This field is used if we're sending a raw buffer instead of through a
	 * config object. It will contain the entire contents of the buffer being
	 * sent.
	 */
	std::vector<char> raw_buffer;
};


bool managed = false, raw_data_only = false;
typedef std::vector< buffer* > buffer_set;
buffer_set outgoing_bufs[NUM_SHARDS];

/** a queue of sockets that we are waiting to receive on */
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
threading::mutex* received_mutex = NULL;
threading::condition* cond[NUM_SHARDS];

std::map<Uint32,threading::thread*> threads[NUM_SHARDS];
std::vector<Uint32> to_clear[NUM_SHARDS];
#if 0
int system_send_buffer_size = 0;
#endif
bool network_use_system_sendfile = false;

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

#if 0
void check_send_buffer_size(TCPsocket& s)
{
	if (system_send_buffer_size)
		return;
	_TCPsocket* sock = reinterpret_cast<_TCPsocket*>(s);
	socklen_t len = sizeof(system_send_buffer_size);
#ifdef _WIN32
	getsockopt(sock->channel, SOL_SOCKET, SO_RCVBUF,reinterpret_cast<char*>(&system_send_buffer_size), &len);
#else
	getsockopt(sock->channel, SOL_SOCKET, SO_RCVBUF,&system_send_buffer_size, &len);
#endif
	--system_send_buffer_size;
	DBG_NW << "send buffer size: " << system_send_buffer_size << "\n";
}
#endif
bool receive_with_timeout(TCPsocket s, char* buf, size_t nbytes,
		bool update_stats=false, int idle_timeout_ms=30000,
		int total_timeout_ms=300000)
{
#if !defined(USE_POLL) && !defined(USE_SELECT)
	int startTicks = SDL_GetTicks();
	int time_used = 0;
#endif
	int timeout_ms = idle_timeout_ms;
	while(nbytes > 0) {
		const int bytes_read = receive_bytes(s, buf, nbytes);
		if(bytes_read == 0) {
			return false;
		} else if(bytes_read < 0) {
#if defined(EAGAIN) && !defined(_WIN32)
			if(errno == EAGAIN)
#elif defined(_WIN32) && defined(WSAEWOULDBLOCK)
			//it seems like 'errno == EWOULDBLOCK' compiles on msvc2010, but doesnt work properly at rumtime.
			if(WSAGetLastError() == WSAEWOULDBLOCK)
#elif defined(EWOULDBLOCK)
			if(errno == EWOULDBLOCK)
#else
			// assume non-recoverable error.
			if(false)
#endif
			{
#ifdef USE_POLL
				struct pollfd fd = { reinterpret_cast<_TCPsocket*>(s)->channel, POLLIN, 0 };
				int poll_res;

				//we timeout of the poll every 100ms. This lets us check to
				//see if we have been disconnected, in which case we should
				//abort the receive.
				const int poll_timeout = std::min(timeout_ms, 100);
				do {
					poll_res = poll(&fd, 1, poll_timeout);

					if(poll_res == 0) {
						timeout_ms -= poll_timeout;
						total_timeout_ms -= poll_timeout;
						if(timeout_ms <= 0 || total_timeout_ms <= 0) {
							//we've been waiting too long; abort the receive
							//as having failed due to timeout.
							return false;
						}

						//check to see if we've been interrupted
						const size_t shard = get_shard(s);
						const threading::lock lock(*shard_mutexes[shard]);
						socket_state_map::iterator lock_it = sockets_locked[shard].find(s);
						assert(lock_it != sockets_locked[shard].end());
						if(lock_it->second == SOCKET_INTERRUPT) {
							return false;
						}
					}

				} while(poll_res == 0 || (poll_res == -1 && errno == EINTR));

				if (poll_res < 1)
					return false;
#elif defined(USE_SELECT)
				int retval;
				const int select_timeout = std::min(timeout_ms, 100);
				do {
					fd_set readfds;
					FD_ZERO(&readfds);
					FD_SET(((_TCPsocket*)s)->channel, &readfds);
					struct timeval tv;
					tv.tv_sec = select_timeout/1000;
					tv.tv_usec = select_timeout % 1000 * 1000;
					retval = select(((_TCPsocket*)s)->channel + 1, &readfds, NULL, NULL, &tv);
					DBG_NW << "select retval: " << retval << ", timeout idle " << timeout_ms
						<< " total " << total_timeout_ms << " (ms)\n";
					if(retval == 0) {
						timeout_ms -= select_timeout;
						total_timeout_ms -= select_timeout;
						if(timeout_ms <= 0 || total_timeout_ms <= 0) {
							//we've been waiting too long; abort the receive
							//as having failed due to timeout.
							return false;
						}

						//check to see if we've been interrupted
						const size_t shard = get_shard(s);
						const threading::lock lock(*shard_mutexes[shard]);
						socket_state_map::iterator lock_it = sockets_locked[shard].find(s);
						assert(lock_it != sockets_locked[shard].end());
						if(lock_it->second == SOCKET_INTERRUPT) {
							return false;
						}
					}
				} while(retval == 0 || (retval == -1 && errno == EINTR));

				if (retval < 1) {
					return false;
				}
#else
				//TODO: consider replacing this with a select call
				time_used = SDL_GetTicks() - startTicks;
				if(time_used >= timeout_ms) {
					return false;
				}
				SDL_Delay(20);
#endif
			} else {
				return false;
			}
		} else {
			timeout_ms = idle_timeout_ms;
			buf += bytes_read;
			if(update_stats && !raw_data_only) {
				const threading::lock lock(*stats_mutex);
				transfer_stats[s].second.transfer(static_cast<size_t>(bytes_read));
			}

			if(bytes_read > static_cast<int>(nbytes)) {
				return false;
			}
			nbytes -= bytes_read;
			// We got some data from server so reset start time so slow conenction won't timeout.
#if !defined(USE_POLL) && !defined(USE_SELECT)
			startTicks = SDL_GetTicks();
#endif
		}
		{
			const size_t shard = get_shard(s);
			const threading::lock lock(*shard_mutexes[shard]);
			socket_state_map::iterator lock_it = sockets_locked[shard].find(s);
			assert(lock_it != sockets_locked[shard].end());
			if(lock_it->second == SOCKET_INTERRUPT) {
				return false;
			}
		}
	}

	return true;
}

/**
 * @todo See if the TCPsocket argument should be removed.
 */
static void output_to_buffer(TCPsocket /*sock*/, const config& cfg, std::ostringstream& compressor)
{
	config_writer writer(compressor, true);
	writer.write(cfg);
}

static void make_network_buffer(const char* input, int len, std::vector<char>& buf)
{
	buf.resize(4 + len);
	SDLNet_Write32(len, &buf[0]);
	memcpy(&buf[4], input, len);
}

static SOCKET_STATE send_buffer(TCPsocket sock, std::vector<char>& buf, int in_size = -1)
{
//	check_send_buffer_size(sock);
	size_t upto = 0;
	size_t size = buf.size();
	if (in_size != -1)
		size = in_size;
	int send_len = 0;

	if (!raw_data_only)
	{
		const threading::lock lock(*stats_mutex);
		transfer_stats[sock].first.fresh_current(size);
	}

	while(true) {
		{
			const size_t shard = get_shard(sock);
			// check if the socket is still locked
			const threading::lock lock(*shard_mutexes[shard]);
			if(sockets_locked[shard][sock] != SOCKET_LOCKED)
			{
				return SOCKET_ERRORED;
			}
		}
		send_len = static_cast<int>(size - upto);
		const int res = SDLNet_TCP_Send(sock, &buf[upto],send_len);


		if( res == send_len) {
			if (!raw_data_only)
			{
				const threading::lock lock(*stats_mutex);
				transfer_stats[sock].first.transfer(static_cast<size_t>(res));
			}
			return SOCKET_READY;
		}
#if defined(_WIN32)
		if(WSAGetLastError() == WSAEWOULDBLOCK)
#elif defined(EAGAIN)
		if(errno == EAGAIN)
#elif defined(EWOULDBLOCK)
		if(errno == EWOULDBLOCK)
#endif
		{
			// update how far we are
			upto += static_cast<size_t>(res);
			if (!raw_data_only)
			{
				const threading::lock lock(*stats_mutex);
				transfer_stats[sock].first.transfer(static_cast<size_t>(res));
			}

#ifdef USE_POLL
			struct pollfd fd = { ((_TCPsocket*)sock)->channel, POLLOUT, 0 };
			int poll_res;
			do {
				poll_res = poll(&fd, 1, 60000);
			} while(poll_res == -1 && errno == EINTR);


			if(poll_res > 0)
				continue;
#elif defined(USE_SELECT)
			fd_set writefds;
			FD_ZERO(&writefds);
			FD_SET(((_TCPsocket*)sock)->channel, &writefds);
			int retval;
			struct timeval tv;
			tv.tv_sec = 60;
			tv.tv_usec = 0;

			do {
				retval = select(((_TCPsocket*)sock)->channel + 1, NULL, &writefds, NULL, &tv);
			} while(retval == -1 && errno == EINTR);

			if(retval > 0)
				continue;
#endif
		}

		return SOCKET_ERRORED;
	}
}

#ifdef HAVE_SENDFILE

#ifdef TCP_CORK
	struct cork_setter {
		cork_setter(int socket) : cork_(1), socket_(socket)
		{
			setsockopt(socket_, IPPROTO_TCP, TCP_CORK, &cork_, sizeof(cork_));;
		}
		~cork_setter()
		{
			cork_ = 0;
			setsockopt(socket_, IPPROTO_TCP, TCP_CORK, &cork_, sizeof(cork_));
		}
		private:
		int cork_;
		int socket_;
	};
#else
	struct cork_setter
	{
		cork_setter(int) {}
	};
#endif

struct close_fd {
	    void operator()(int fd) const { close(fd); }
};
typedef util::scoped_resource<int, close_fd> scoped_fd;
#endif

static SOCKET_STATE send_file(buffer* buf)
{
	size_t upto = 0;
	size_t filesize = file_size(buf->config_error);
#ifdef HAVE_SENDFILE
	// implements linux sendfile support
	LOG_NW << "send_file use system sendfile: " << (network_use_system_sendfile?"yes":"no") << "\n";
	if (network_use_system_sendfile)
	{
		std::vector<char> buffer;
		buffer.resize(4);
		SDLNet_Write32(filesize,&buffer[0]);
		int socket = reinterpret_cast<_TCPsocket*>(buf->sock)->channel;
		const scoped_fd in_file(open(buf->config_error.c_str(), O_RDONLY));
		cork_setter set_socket_cork(socket);
		int poll_res;
		struct pollfd fd = {socket, POLLOUT, 0 };
		do {
			poll_res = poll(&fd, 1, 600000);
		} while(poll_res == -1 && errno == EINTR);

		SOCKET_STATE result;
		if (poll_res > 0)
			result = send_buffer(buf->sock, buffer, 4);
		else
			result = SOCKET_ERRORED;


		if (result != SOCKET_READY)
		{
			return result;
		}
		result = SOCKET_READY;

		while (true)
		{

			do {
				poll_res = poll(&fd, 1, 600000);
			} while(poll_res == -1 && errno == EINTR);

			if (poll_res <= 0 )
			{
				result = SOCKET_ERRORED;
				break;
			}


			int bytes = ::sendfile(socket, in_file, 0, filesize);

			if (bytes == -1)
			{
				if (errno == EAGAIN)
					continue;
				result = SOCKET_ERRORED;
				break;
			}
			upto += bytes;


			if (upto == filesize)
			{
				break;
			}
		}

		return result;
	}
#endif
	// default sendfile implementation
	// if no system implementation is enabled
	int send_size = 0;
	// reserve 1024*8 bytes buffer
	buf->raw_buffer.resize(std::min<size_t>(1024*8, filesize));
	SDLNet_Write32(filesize,&buf->raw_buffer[0]);
	scoped_istream file_stream = istream_file(buf->config_error);
	SOCKET_STATE result = send_buffer(buf->sock, buf->raw_buffer, 4);

	if (!file_stream->good()) {
		ERR_NW << "send_file: Couldn't open file " << buf->config_error << "\n";
	}
	if (result != SOCKET_READY)
	{
		return result;
	}
	while (file_stream->good())
	{
		// read data
		file_stream->read(&buf->raw_buffer[0], buf->raw_buffer.size());
		send_size = file_stream->gcount();
		upto += send_size;
		// send data to socket
		result = send_buffer(buf->sock, buf->raw_buffer, send_size);
		if (result != SOCKET_READY)
		{
			break;
		}
		if (upto == filesize)
		{
			break;
		}

	}
	if (upto != filesize && !file_stream->good()) {
		ERR_NW << "send_file failed because the stream from file '"
			<< buf->config_error << "' is not good. Sent up to: " << upto
			<< " of file size: " << filesize << "\n";
	}
	return result;
}

static SOCKET_STATE receive_buf(TCPsocket sock, std::vector<char>& buf)
{
	union {
	char buf[4] ALIGN_4;
	Uint32 num;
	} num_buf;
	bool res = receive_with_timeout(sock,num_buf.buf,4,false);

	if(!res) {
		return SOCKET_ERRORED;
	}

	const int len = SDLNet_Read32(&num_buf);

	if(len < 1 || len > 100000000) {
		return SOCKET_ERRORED;
	}

	buf.resize(len);
	char* beg = &buf[0];
	const char* const end = beg + len;

	if (!raw_data_only)
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
	const size_t shard = get_shard(sock);
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
	size_t shard = reinterpret_cast<uintptr_t>(shard_num);
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

				buffer_set::iterator itor = outgoing_bufs[shard].begin(), itor_end = outgoing_bufs[shard].end();
				for(; itor != itor_end; ++itor) {
					socket_state_map::iterator lock_it = sockets_locked[shard].find((*itor)->sock);
					assert(lock_it != sockets_locked[shard].end());
					if(lock_it->second == SOCKET_READY) {
						lock_it->second = SOCKET_LOCKED;
						sent_buf = *itor;
						sock = sent_buf->sock;
						outgoing_bufs[shard].erase(itor);
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

 			if(!sent_buf->config_error.empty())
 			{
 				// We have file to send over net
 				result = send_file(sent_buf);
			} else {
				if(sent_buf->raw_buffer.empty()) {
					const std::string &value = sent_buf->stream.str();
					make_network_buffer(value.c_str(), value.size(), sent_buf->raw_buffer);
				}

				result = send_buffer(sent_buf->sock, sent_buf->raw_buffer);
			}
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

		if(raw_data_only) {
			received_data->raw_buffer.swap(buf);
		} else {
			std::string buffer(buf.begin(), buf.end());
			std::istringstream stream(buffer);
			try {
				read_gz(received_data->config_buf, stream);
			} catch(boost::iostreams::gzip_error&) {
				received_data->config_error = "Malformed compressed data";
			} catch(config::error &e) {
				received_data->config_error = e.message;
			}
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
		received_mutex = new threading::mutex();

		min_threads = p_min_threads;
		max_threads = p_max_threads;

		for(size_t shard = 0; shard != NUM_SHARDS; ++shard) {
			const threading::lock lock(*shard_mutexes[shard]);
			for(size_t n = 0; n != p_min_threads; ++n) {
				threading::thread * tmp = new threading::thread(process_queue,(void*)(shard));
				threads[shard][tmp->get_id()] = tmp;
			}
		}
	}
}

manager::~manager()
{
	if(active_) {
		managed = false;

		for(size_t shard = 0; shard != NUM_SHARDS; ++shard) {
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

			// Condition variables must be deleted first as
			// they make reference to mutexs. If the mutexs
			// are destroyed first, the condition variables
			// will access memory already freed by way of
			// stale mutex. Bad things will follow. ;)
 			threads[shard].clear();
			// Have to clean up to_clear so no bogus clearing of threads
			to_clear[shard].clear();
 			delete cond[shard];
 			cond[shard] = NULL;
			delete shard_mutexes[shard];
 			shard_mutexes[shard] = NULL;
 		}

		delete stats_mutex;
		delete received_mutex;
		stats_mutex = 0;
		received_mutex = 0;

		for(int i = 0; i != NUM_SHARDS; ++i) {
			sockets_locked[i].clear();
		}
		transfer_stats.clear();

		DBG_NW << "exiting manager::~manager()\n";
	}
}

network::pending_statistics get_pending_stats()
{
	network::pending_statistics stats;
	stats.npending_sends = 0;
	stats.nbytes_pending_sends = 0;
	for(size_t shard = 0; shard != NUM_SHARDS; ++shard) {
		const threading::lock lock(*shard_mutexes[shard]);
		stats.npending_sends += outgoing_bufs[shard].size();
		for(buffer_set::const_iterator i = outgoing_bufs[shard].begin(); i != outgoing_bufs[shard].end(); ++i) {
			stats.nbytes_pending_sends += (*i)->raw_buffer.size();
		}
	}

	return stats;
}

void set_raw_data_only()
{
	raw_data_only = true;
}

void set_use_system_sendfile(bool use)
{
	network_use_system_sendfile = use;
}

void receive_data(TCPsocket sock)
{
	{
		const size_t shard = get_shard(sock);
		const threading::lock lock(*shard_mutexes[shard]);
		pending_receives[shard].push_back(sock);

		socket_state_map::const_iterator i = sockets_locked[shard].insert(std::pair<TCPsocket,SOCKET_STATE>(sock,SOCKET_READY)).first;
		if(i->second == SOCKET_READY || i->second == SOCKET_ERRORED) {
			cond[shard]->notify_one();
		}
	}
}

TCPsocket get_received_data(TCPsocket sock, config& cfg, network::bandwidth_in_ptr& bandwidth_in)
{
	assert(!raw_data_only);
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
		TCPsocket err_sock = (*itor)->sock;
		received_data_queue.erase(itor);
		delete buf;
		throw config::error(error) << network::tcpsocket_info(err_sock);
	} else {
		cfg.swap((*itor)->config_buf);
		const TCPsocket res = (*itor)->sock;
		buffer* buf = *itor;
		bandwidth_in.reset(new network::bandwidth_in((*itor)->raw_buffer.size()));
		received_data_queue.erase(itor);
		delete buf;
		return res;
	}
}

TCPsocket get_received_data(std::vector<char>& out)
{
	assert(raw_data_only);
	const threading::lock lock_received(*received_mutex);
	if(received_data_queue.empty()) {
		return NULL;
	}

	buffer* buf = received_data_queue.front();
	received_data_queue.pop_front();
	out.swap(buf->raw_buffer);
	const TCPsocket res = buf->sock;
	delete buf;
	return res;
}

static void queue_buffer(TCPsocket sock, buffer* queued_buf)
{
	const size_t shard = get_shard(sock);
	const threading::lock lock(*shard_mutexes[shard]);
	outgoing_bufs[shard].push_back(queued_buf);
	socket_state_map::const_iterator i = sockets_locked[shard].insert(std::pair<TCPsocket,SOCKET_STATE>(sock,SOCKET_READY)).first;
	if(i->second == SOCKET_READY || i->second == SOCKET_ERRORED) {
		cond[shard]->notify_one();
	}

}

void queue_raw_data(TCPsocket sock, const char* buf, int len)
{
	buffer* queued_buf = new buffer(sock);
	assert(*buf == 31);
	make_network_buffer(buf, len, queued_buf->raw_buffer);
	queue_buffer(sock, queued_buf);
}


void queue_file(TCPsocket sock, const std::string& filename)
{
 	buffer* queued_buf = new buffer(sock);
 	queued_buf->config_error = filename;
 	queue_buffer(sock, queued_buf);
}

size_t queue_data(TCPsocket sock,const config& buf, const std::string& packet_type)
{
	DBG_NW << "queuing data...\n";

	buffer* queued_buf = new buffer(sock);
	output_to_buffer(sock, buf, queued_buf->stream);
	const size_t size = queued_buf->stream.str().size();

	network::add_bandwidth_out(packet_type, size);
	queue_buffer(sock, queued_buf);
	return size;
}

namespace
{

/** Caller has to make sure to own the mutex for this shard */
void remove_buffers(TCPsocket sock)
{
	{
		const size_t shard = get_shard(sock);
		for(buffer_set::iterator i = outgoing_bufs[shard].begin(); i != outgoing_bufs[shard].end();) {
			if ((*i)->sock == sock)
			{
				buffer* buf = *i;
				i = outgoing_bufs[shard].erase(i);
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
	const size_t shard = get_shard(sock);
	const threading::lock lock(*shard_mutexes[shard]);
	const socket_state_map::iterator lock_it = sockets_locked[shard].find(sock);
	if (lock_it == sockets_locked[shard].end()) return false;
	return (lock_it->second == SOCKET_LOCKED);
}

bool close_socket(TCPsocket sock)
{
	{
		const size_t shard = get_shard(sock);
		const threading::lock lock(*shard_mutexes[shard]);

		pending_receives[shard].erase(std::remove(pending_receives[shard].begin(),pending_receives[shard].end(),sock),pending_receives[shard].end());

		const socket_state_map::iterator lock_it = sockets_locked[shard].find(sock);
		if(lock_it == sockets_locked[shard].end()) {
			remove_buffers(sock);
			return true;
		}
		if (!(lock_it->second == SOCKET_LOCKED || lock_it->second == SOCKET_INTERRUPT)) {
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
	for(size_t shard = 0; shard != NUM_SHARDS; ++shard) {
		const threading::lock lock(*shard_mutexes[shard]);
		if(socket_errors[shard] > 0) {
			for(socket_state_map::iterator i = sockets_locked[shard].begin(); i != sockets_locked[shard].end();) {
				if(i->second == SOCKET_ERRORED) {
					--socket_errors[shard];
					const TCPsocket sock = i->first;
					sockets_locked[shard].erase(i++);
					pending_receives[shard].erase(std::remove(pending_receives[shard].begin(),pending_receives[shard].end(),sock),pending_receives[shard].end());
					remove_buffers(sock);
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


