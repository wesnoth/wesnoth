//
// basic_resolver_query.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_IP_BASIC_RESOLVER_QUERY_HPP
#define BOOST_ASIO_IP_BASIC_RESOLVER_QUERY_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <boost/config.hpp>
#include <string>
#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/detail/socket_ops.hpp>
#include <boost/asio/ip/resolver_query_base.hpp>

namespace boost {
namespace asio {
namespace ip {

/// An query to be passed to a resolver.
/**
 * The boost::asio::ip::basic_resolver_query class template describes a query
 * that can be passed to a resolver.
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Unsafe.
 */
template <typename InternetProtocol>
class basic_resolver_query
  : public resolver_query_base
{
public:
  /// The protocol type associated with the endpoint query.
  typedef InternetProtocol protocol_type;

  /// Construct with specified service name for any protocol.
  basic_resolver_query(const std::string& service_name,
      int flags = passive | address_configured)
    : hints_(),
      host_name_(),
      service_name_(service_name)
  {
    typename InternetProtocol::endpoint endpoint;
    hints_.ai_flags = flags;
    hints_.ai_family = PF_UNSPEC;
    hints_.ai_socktype = endpoint.protocol().type();
    hints_.ai_protocol = endpoint.protocol().protocol();
    hints_.ai_addrlen = 0;
    hints_.ai_canonname = 0;
    hints_.ai_addr = 0;
    hints_.ai_next = 0;
  }

  /// Construct with specified service name for a given protocol.
  basic_resolver_query(const protocol_type& protocol,
      const std::string& service_name,
      int flags = passive | address_configured)
    : hints_(),
      host_name_(),
      service_name_(service_name)
  {
    hints_.ai_flags = flags;
    hints_.ai_family = protocol.family();
    hints_.ai_socktype = protocol.type();
    hints_.ai_protocol = protocol.protocol();
    hints_.ai_addrlen = 0;
    hints_.ai_canonname = 0;
    hints_.ai_addr = 0;
    hints_.ai_next = 0;
  }

  /// Construct with specified host name and service name for any protocol.
  basic_resolver_query(const std::string& host_name,
      const std::string& service_name, int flags = address_configured)
    : hints_(),
      host_name_(host_name),
      service_name_(service_name)
  {
    typename InternetProtocol::endpoint endpoint;
    hints_.ai_flags = flags;
    hints_.ai_family = PF_UNSPEC;
    hints_.ai_socktype = endpoint.protocol().type();
    hints_.ai_protocol = endpoint.protocol().protocol();
    hints_.ai_addrlen = 0;
    hints_.ai_canonname = 0;
    hints_.ai_addr = 0;
    hints_.ai_next = 0;
  }

  /// Construct with specified host name and service name for a given protocol.
  basic_resolver_query(const protocol_type& protocol,
      const std::string& host_name, const std::string& service_name,
      int flags = address_configured)
    : hints_(),
      host_name_(host_name),
      service_name_(service_name)
  {
    hints_.ai_flags = flags;
    hints_.ai_family = protocol.family();
    hints_.ai_socktype = protocol.type();
    hints_.ai_protocol = protocol.protocol();
    hints_.ai_addrlen = 0;
    hints_.ai_canonname = 0;
    hints_.ai_addr = 0;
    hints_.ai_next = 0;
  }

  /// Get the hints associated with the query.
  const boost::asio::detail::addrinfo_type& hints() const
  {
    return hints_;
  }

  /// Get the host name associated with the query.
  std::string host_name() const
  {
    return host_name_;
  }

  /// Get the service name associated with the query.
  std::string service_name() const
  {
    return service_name_;
  }

private:
  boost::asio::detail::addrinfo_type hints_;
  std::string host_name_;
  std::string service_name_;
};

} // namespace ip
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_IP_BASIC_RESOLVER_QUERY_HPP
