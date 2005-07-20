#ifndef PROXY_HPP_INCLUDED
#define PROXY_HPP_INCLUDED

#include "config.hpp"
#include "network.hpp"

#include <string>

namespace proxy
{

void create_proxy(network::connection sock, const std::string& host, int port);
bool is_proxy(network::connection sock);
void disconnect(network::connection sock);

void received_data(network::connection sock, const config& data);

}

#endif