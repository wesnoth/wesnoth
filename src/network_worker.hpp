#ifndef NETWORK_WORKER_HPP_INCLUDED
#define NETWORK_WORKER_HPP_INCLUDED

#include <vector>

#include "SDL_net.h"

namespace network_worker_pool
{

struct manager
{
	explicit manager(size_t nthreads=1);
	~manager();

private:
	manager(const manager&);
	void operator=(const manager&);

	bool active_;
};

void queue_data(TCPsocket sock, std::vector<char>& buf);
bool socket_locked(TCPsocket sock);
void close_socket(TCPsocket sock);
TCPsocket detect_error();

}

#endif
