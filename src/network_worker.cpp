#include "global.hpp"

#include "log.hpp"
#include "network_worker.hpp"
#include "network.hpp"
#include "thread.hpp"
#include "wassert.hpp"

#include <cerrno>
#include <iostream>
#include <map>
#include <vector>

#define LOG_NW lg::info(lg::network)

namespace {

unsigned int buf_id = 0;

struct buffer {
	explicit buffer(TCPsocket sock) : sock(sock) {}

	TCPsocket sock;
	mutable std::vector<char> buf;
};

bool managed = false;
typedef std::vector< buffer * > buffer_set;
buffer_set bufs;

enum SOCKET_STATE { SOCKET_READY, SOCKET_LOCKED, SOCKET_ERROR };
typedef std::map<TCPsocket,SOCKET_STATE> socket_state_map;
socket_state_map sockets_locked;
int socket_errors = 0;
threading::mutex* global_mutex = NULL;
threading::condition* cond = NULL;

std::vector<threading::thread*> threads;

int process_queue(void* data)
{
	LOG_NW << "thread started...\n";
	for(;;) {

		buffer *sent_buf = NULL;

		{
			const threading::lock lock(*global_mutex);

			for(;;) {

				buffer_set::iterator itor = bufs.begin(), itor_end = bufs.end();
				for(; itor != itor_end; ++itor) {
					socket_state_map::iterator lock_it = sockets_locked.find((*itor)->sock);
					wassert(lock_it != sockets_locked.end());
					if(lock_it->second == SOCKET_READY) {
						lock_it->second = SOCKET_LOCKED;
						break;
					}
				}

				if(itor == itor_end) {
					if(managed == false) {
						LOG_NW << "worker thread exiting...\n";
						return 0;
					}

					cond->wait(*global_mutex); // temporarily release the mutex and wait for a buffer
					continue;
				} else {
					sent_buf = *itor;
					bufs.erase(itor);
					break; // a buffer has been found
				}
			}
		}

		LOG_NW << "thread found a buffer...\n";

		SOCKET_STATE result = SOCKET_READY;

		std::vector<char> &v = sent_buf->buf;
		for(size_t upto = 0, size = v.size(); result != SOCKET_ERROR && upto < size; ) {
			const int bytes_to_send = int(size - upto);
			const int res = SDLNet_TCP_Send(sent_buf->sock, &v[upto], bytes_to_send);
			if(res < 0 || res != bytes_to_send && errno != EAGAIN) {
				result = SOCKET_ERROR;
			} else {
				upto += res;
			}
		}

		LOG_NW << "thread sent " << v.size() << " bytes of data...\n";

		{
			const threading::lock lock(*global_mutex);
			socket_state_map::iterator lock_it = sockets_locked.find(sent_buf->sock);
			wassert(lock_it != sockets_locked.end());
			lock_it->second = result;
			if(result == SOCKET_ERROR) {
				++socket_errors;
			}
		}
		delete sent_buf;
	}
	// unreachable
}

}

namespace network_worker_pool
{

manager::manager(size_t nthreads) : active_(!managed)
{
	if(active_) {
		managed = true;
		global_mutex = new threading::mutex();
		cond = new threading::condition();

		for(size_t n = 0; n != nthreads; ++n) {
			threads.push_back(new threading::thread(process_queue,NULL));
		}
	}
}

manager::~manager()
{
	if(active_) {
		{
			const threading::lock lock(*global_mutex);
			managed = false;
			sockets_locked.clear();
			socket_errors = 0;
		}
		cond->notify_all();

		for(std::vector<threading::thread*>::const_iterator i = threads.begin(); i != threads.end(); ++i) {
			LOG_NW << "waiting for thread " << int(i - threads.begin()) << " to exit...\n";
			delete *i;
			LOG_NW << "thread exited...\n";
		}

		threads.clear();

		delete global_mutex;
		delete cond;
		global_mutex = NULL;
		cond = NULL;

		LOG_NW << "exiting manager::~manager()\n";
	}
}

void queue_data(TCPsocket sock, std::vector<char>& buf)
{
	LOG_NW << "queuing " << buf.size() << " bytes of data...\n";

	{
		const threading::lock lock(*global_mutex);

		buffer *queued_buf = new buffer(sock);
		queued_buf->buf.swap(buf);
		bufs.push_back(queued_buf);

		sockets_locked.insert(std::pair<TCPsocket,SOCKET_STATE>(sock,SOCKET_READY));
	}

	cond->notify_one();
}

namespace
{

void remove_buffers(TCPsocket sock)
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

}

void close_socket(TCPsocket sock)
{
	for(bool first_time = true; ; first_time = false) {
		if(!first_time) {
			SDL_Delay(10);
		}

		const threading::lock lock(*global_mutex);

		const socket_state_map::iterator lock_it = sockets_locked.find(sock);
		
		if(lock_it == sockets_locked.end() || lock_it->second != SOCKET_LOCKED) {
			if(lock_it != sockets_locked.end()) {
				sockets_locked.erase(lock_it);
			}

			remove_buffers(sock);

			break;
		}
	}
}

TCPsocket detect_error()
{
	const threading::lock lock(*global_mutex);
	if(socket_errors > 0) {
		for(socket_state_map::iterator i = sockets_locked.begin(); i != sockets_locked.end(); ++i) {
			if(i->second == SOCKET_ERROR) {
				--socket_errors;
				const TCPsocket res = i->first;
				sockets_locked.erase(i);
				remove_buffers(res);
				return res;
			}
		}
	}

	return 0;
}

}