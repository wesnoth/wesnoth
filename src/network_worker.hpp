#ifndef NETWORK_WORKER_HPP_INCLUDED
#define NETWORK_WORKER_HPP_INCLUDED

#include <map>
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

//function to asynchronously received data to the given socket
void receive_data(TCPsocket sock);

TCPsocket get_received_data(TCPsocket sock, std::vector<char>& buf);

void queue_data(TCPsocket sock, std::vector<char>& buf);
bool socket_locked(TCPsocket sock);
void close_socket(TCPsocket sock);
TCPsocket detect_error();

std::pair<int,int> get_current_transfer_stats();

}

#endif
