#include "log.hpp"
#include "network_worker.hpp"
#include "network.hpp"
#include "thread.hpp"

#include <cassert>
#include <cerrno>
#include <iostream>
#include <map>
#include <set>

#define LOG_NW lg::info(lg::network)

namespace {

unsigned int buf_id = 0;

struct buffer {
	explicit buffer(TCPsocket sock) : id(buf_id++), sock(sock), processing_started(false)
	{}

	unsigned int id;
	TCPsocket sock;
	mutable std::vector<char> buf;
	mutable bool processing_started;
};

bool operator<(const buffer& a, const buffer& b) {
	return a.id < b.id;
}

bool managed = false;
std::multiset<buffer> bufs;

enum SOCKET_STATE { SOCKET_READY, SOCKET_LOCKED, SOCKET_ERROR };
typedef std::map<TCPsocket,SOCKET_STATE> socket_state_map;
socket_state_map sockets_locked;
int socket_errors = 0;
threading::mutex* global_mutex = NULL;
threading::condition* cond = NULL;

std::vector<threading::thread*> threads;

int process_queue(void* data)
{
	threading::mutex m;
	const threading::lock mutex_lock(m);
	LOG_NW << "thread started...\n";
	for(;;) {

		std::multiset<buffer>::iterator itor;
		socket_state_map::iterator lock_it;

		{
			const threading::lock lock(*global_mutex);

			for(itor = bufs.begin(); itor != bufs.end(); ++itor) {
				if(itor->processing_started) {
					continue;
				}

				LOG_NW << "thread found a buffer...\n";

				lock_it = sockets_locked.find(itor->sock);
				assert(lock_it != sockets_locked.end());
				if(lock_it->second == SOCKET_READY) {
					//some implementations don't allow modification of items
					//in a set, so we fix this with a const_cast
					itor->processing_started = true;
					lock_it->second = SOCKET_LOCKED;
					break;
				}
			}

			if(itor == bufs.end()) {
				if(managed == false) {
					LOG_NW << "worker thread " << reinterpret_cast<long>(data) << " exiting...\n";
					return 0;
				}

				cond->wait(*global_mutex);
				LOG_NW << "thread couldn't find a buffer...\n";
				continue;
			}
		}

		SOCKET_STATE result = SOCKET_READY;

		std::vector<char>& v = itor->buf;
		for(size_t upto = 0; result != SOCKET_ERROR && upto < v.size(); ) {
			const int bytes_to_send = int(v.size() - upto);
			const int res = SDLNet_TCP_Send(itor->sock,&v[upto],bytes_to_send);
			if(res < 0 || res != bytes_to_send && errno != EAGAIN) {
				result = SOCKET_ERROR;
			} else {
				upto += res;
			}
		}

		LOG_NW << "thread sent " << v.size() << " bytes of data...\n";

		{
			const threading::lock lock(*global_mutex);
			bufs.erase(itor);
			lock_it->second = result;
			if(result == SOCKET_ERROR) {
				++socket_errors;
			}
		}
	}

	return 0;
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
			cond->notify_all();
		}

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
	const threading::lock lock(*global_mutex);

	LOG_NW << "queuing " << buf.size() << " bytes of data...\n";

	const std::multiset<buffer>::iterator i = bufs.insert(buffer(sock));
	i->buf.swap(buf);

	sockets_locked.insert(std::pair<TCPsocket,SOCKET_STATE>(sock,SOCKET_READY));

	cond->notify_one();
}

bool socket_locked(TCPsocket sock)
{
	const threading::lock lock(*global_mutex);
	const socket_state_map::const_iterator i = sockets_locked.find(sock);
	if(i != sockets_locked.end()) {
		return i->second == SOCKET_LOCKED;
	} else {
		return false;
	}
}

void close_socket(TCPsocket sock)
{
	while(socket_locked(sock)) {
		SDL_Delay(10);
	}

	const threading::lock lock(*global_mutex);
	sockets_locked.erase(sock);
	std::multiset<buffer>::iterator i = bufs.begin();
	while(i != bufs.end()) {
		if(i->sock == sock) {
			bufs.erase(i++);
		} else {
			++i;
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
				return res;
			}
		}
	}

	return 0;
}

}
