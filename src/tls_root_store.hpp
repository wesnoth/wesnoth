#pragma once

#include <boost/asio/ssl/context.hpp>
#include <wincrypt.h>

namespace network_asio
{

void load_tls_root_certs(boost::asio::ssl::context &ctx);

}
