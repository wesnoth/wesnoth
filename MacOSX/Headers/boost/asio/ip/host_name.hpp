//
// host_name.hpp
// ~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_IP_HOST_NAME_HPP
#define BOOST_ASIO_IP_HOST_NAME_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <string>
#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/error.hpp>
#include <boost/asio/detail/socket_ops.hpp>
#include <boost/asio/detail/throw_error.hpp>

namespace boost {
namespace asio {
namespace ip {

/// Get the current host name.
std::string host_name();

/// Get the current host name.
std::string host_name(boost::system::error_code& ec);

inline std::string host_name()
{
  char name[1024];
  boost::system::error_code ec;
  if (boost::asio::detail::socket_ops::gethostname(name, sizeof(name), ec) != 0)
  {
    boost::asio::detail::throw_error(ec);
    return std::string();
  }
  return std::string(name);
}

inline std::string host_name(boost::system::error_code& ec)
{
  char name[1024];
  if (boost::asio::detail::socket_ops::gethostname(name, sizeof(name), ec) != 0)
    return std::string();
  return std::string(name);
}

} // namespace ip
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_IP_HOST_NAME_HPP
