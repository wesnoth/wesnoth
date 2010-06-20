//
// basic_endpoint.hpp
// ~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_IP_BASIC_ENDPOINT_HPP
#define BOOST_ASIO_IP_BASIC_ENDPOINT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <boost/throw_exception.hpp>
#include <boost/detail/workaround.hpp>
#include <cstring>
#if BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x564))
# include <ostream>
#endif // BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x564))
#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/error.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/detail/socket_ops.hpp>
#include <boost/asio/detail/socket_types.hpp>

namespace boost {
namespace asio {
namespace ip {

/// Describes an endpoint for a version-independent IP socket.
/**
 * The boost::asio::ip::basic_endpoint class template describes an endpoint that
 * may be associated with a particular socket.
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Unsafe.
 *
 * @par Concepts:
 * Endpoint.
 */
template <typename InternetProtocol>
class basic_endpoint
{
public:
  /// The protocol type associated with the endpoint.
  typedef InternetProtocol protocol_type;

  /// The type of the endpoint structure. This type is dependent on the
  /// underlying implementation of the socket layer.
#if defined(GENERATING_DOCUMENTATION)
  typedef implementation_defined data_type;
#else
  typedef boost::asio::detail::socket_addr_type data_type;
#endif

  /// Default constructor.
  basic_endpoint()
    : data_()
  {
    data_.v4.sin_family = AF_INET;
    data_.v4.sin_port = 0;
    data_.v4.sin_addr.s_addr = INADDR_ANY;
  }

  /// Construct an endpoint using a port number, specified in the host's byte
  /// order. The IP address will be the any address (i.e. INADDR_ANY or
  /// in6addr_any). This constructor would typically be used for accepting new
  /// connections.
  /**
   * @par Examples
   * To initialise an IPv4 TCP endpoint for port 1234, use:
   * @code
   * boost::asio::ip::tcp::endpoint ep(boost::asio::ip::tcp::v4(), 1234);
   * @endcode
   *
   * To specify an IPv6 UDP endpoint for port 9876, use:
   * @code
   * boost::asio::ip::udp::endpoint ep(boost::asio::ip::udp::v6(), 9876);
   * @endcode
   */
  basic_endpoint(const InternetProtocol& protocol, unsigned short port_num)
    : data_()
  {
    using namespace std; // For memcpy.
    if (protocol.family() == PF_INET)
    {
      data_.v4.sin_family = AF_INET;
      data_.v4.sin_port =
        boost::asio::detail::socket_ops::host_to_network_short(port_num);
      data_.v4.sin_addr.s_addr = INADDR_ANY;
    }
    else
    {
      data_.v6.sin6_family = AF_INET6;
      data_.v6.sin6_port =
        boost::asio::detail::socket_ops::host_to_network_short(port_num);
      data_.v6.sin6_flowinfo = 0;
      boost::asio::detail::in6_addr_type tmp_addr = IN6ADDR_ANY_INIT;
      data_.v6.sin6_addr = tmp_addr;
      data_.v6.sin6_scope_id = 0;
    }
  }

  /// Construct an endpoint using a port number and an IP address. This
  /// constructor may be used for accepting connections on a specific interface
  /// or for making a connection to a remote endpoint.
  basic_endpoint(const boost::asio::ip::address& addr, unsigned short port_num)
    : data_()
  {
    using namespace std; // For memcpy.
    if (addr.is_v4())
    {
      data_.v4.sin_family = AF_INET;
      data_.v4.sin_port =
        boost::asio::detail::socket_ops::host_to_network_short(port_num);
      data_.v4.sin_addr.s_addr =
        boost::asio::detail::socket_ops::host_to_network_long(
            addr.to_v4().to_ulong());
    }
    else
    {
      data_.v6.sin6_family = AF_INET6;
      data_.v6.sin6_port =
        boost::asio::detail::socket_ops::host_to_network_short(port_num);
      data_.v6.sin6_flowinfo = 0;
      boost::asio::ip::address_v6 v6_addr = addr.to_v6();
      boost::asio::ip::address_v6::bytes_type bytes = v6_addr.to_bytes();
      memcpy(data_.v6.sin6_addr.s6_addr, bytes.elems, 16);
      data_.v6.sin6_scope_id = v6_addr.scope_id();
    }
  }

  /// Copy constructor.
  basic_endpoint(const basic_endpoint& other)
    : data_(other.data_)
  {
  }

  /// Assign from another endpoint.
  basic_endpoint& operator=(const basic_endpoint& other)
  {
    data_ = other.data_;
    return *this;
  }

  /// The protocol associated with the endpoint.
  protocol_type protocol() const
  {
    if (is_v4())
      return InternetProtocol::v4();
    return InternetProtocol::v6();
  }

  /// Get the underlying endpoint in the native type.
  data_type* data()
  {
    return &data_.base;
  }

  /// Get the underlying endpoint in the native type.
  const data_type* data() const
  {
    return &data_.base;
  }

  /// Get the underlying size of the endpoint in the native type.
  std::size_t size() const
  {
    if (is_v4())
      return sizeof(boost::asio::detail::sockaddr_in4_type);
    else
      return sizeof(boost::asio::detail::sockaddr_in6_type);
  }

  /// Set the underlying size of the endpoint in the native type.
  void resize(std::size_t size)
  {
    if (size > sizeof(boost::asio::detail::sockaddr_storage_type))
    {
      boost::system::system_error e(boost::asio::error::invalid_argument);
      boost::throw_exception(e);
    }
  }

  /// Get the capacity of the endpoint in the native type.
  std::size_t capacity() const
  {
    return sizeof(boost::asio::detail::sockaddr_storage_type);
  }

  /// Get the port associated with the endpoint. The port number is always in
  /// the host's byte order.
  unsigned short port() const
  {
    if (is_v4())
    {
      return boost::asio::detail::socket_ops::network_to_host_short(
          data_.v4.sin_port);
    }
    else
    {
      return boost::asio::detail::socket_ops::network_to_host_short(
          data_.v6.sin6_port);
    }
  }

  /// Set the port associated with the endpoint. The port number is always in
  /// the host's byte order.
  void port(unsigned short port_num)
  {
    if (is_v4())
    {
      data_.v4.sin_port
        = boost::asio::detail::socket_ops::host_to_network_short(port_num);
    }
    else
    {
      data_.v6.sin6_port
        = boost::asio::detail::socket_ops::host_to_network_short(port_num);
    }
  }

  /// Get the IP address associated with the endpoint.
  boost::asio::ip::address address() const
  {
    using namespace std; // For memcpy.
    if (is_v4())
    {
      return boost::asio::ip::address_v4(
          boost::asio::detail::socket_ops::network_to_host_long(
            data_.v4.sin_addr.s_addr));
    }
    else
    {
      boost::asio::ip::address_v6::bytes_type bytes;
      memcpy(bytes.elems, data_.v6.sin6_addr.s6_addr, 16);
      return boost::asio::ip::address_v6(bytes, data_.v6.sin6_scope_id);
    }
  }

  /// Set the IP address associated with the endpoint.
  void address(const boost::asio::ip::address& addr)
  {
    basic_endpoint<InternetProtocol> tmp_endpoint(addr, port());
    data_ = tmp_endpoint.data_;
  }

  /// Compare two endpoints for equality.
  friend bool operator==(const basic_endpoint<InternetProtocol>& e1,
      const basic_endpoint<InternetProtocol>& e2)
  {
    return e1.address() == e2.address() && e1.port() == e2.port();
  }

  /// Compare two endpoints for inequality.
  friend bool operator!=(const basic_endpoint<InternetProtocol>& e1,
      const basic_endpoint<InternetProtocol>& e2)
  {
    return e1.address() != e2.address() || e1.port() != e2.port();
  }

  /// Compare endpoints for ordering.
  friend bool operator<(const basic_endpoint<InternetProtocol>& e1,
      const basic_endpoint<InternetProtocol>& e2)
  {
    if (e1.address() < e2.address())
      return true;
    if (e1.address() != e2.address())
      return false;
    return e1.port() < e2.port();
  }

private:
  // Helper function to determine whether the endpoint is IPv4.
  bool is_v4() const
  {
    return data_.base.sa_family == AF_INET;
  }

  // The underlying IP socket address.
  union data_union
  {
    boost::asio::detail::socket_addr_type base;
    boost::asio::detail::sockaddr_storage_type storage;
    boost::asio::detail::sockaddr_in4_type v4;
    boost::asio::detail::sockaddr_in6_type v6;
  } data_;
};

/// Output an endpoint as a string.
/**
 * Used to output a human-readable string for a specified endpoint.
 *
 * @param os The output stream to which the string will be written.
 *
 * @param endpoint The endpoint to be written.
 *
 * @return The output stream.
 *
 * @relates boost::asio::ip::basic_endpoint
 */
#if BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x564))
template <typename InternetProtocol>
std::ostream& operator<<(std::ostream& os,
    const basic_endpoint<InternetProtocol>& endpoint)
{
  const address& addr = endpoint.address();
  boost::system::error_code ec;
  std::string a = addr.to_string(ec);
  if (ec)
  {
    if (os.exceptions() & std::ios::failbit)
      boost::asio::detail::throw_error(ec);
    else
      os.setstate(std::ios_base::failbit);
  }
  else
  {
    if (addr.is_v4())
      os << a;
    else
      os << '[' << a << ']';
    os << ':' << endpoint.port();
  }
  return os;
}
#else // BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x564))
template <typename Elem, typename Traits, typename InternetProtocol>
std::basic_ostream<Elem, Traits>& operator<<(
    std::basic_ostream<Elem, Traits>& os,
    const basic_endpoint<InternetProtocol>& endpoint)
{
  const address& addr = endpoint.address();
  boost::system::error_code ec;
  std::string a = addr.to_string(ec);
  if (ec)
  {
    if (os.exceptions() & std::ios::failbit)
      boost::asio::detail::throw_error(ec);
    else
      os.setstate(std::ios_base::failbit);
  }
  else
  {
    if (addr.is_v4())
      os << a;
    else
      os << '[' << a << ']';
    os << ':' << endpoint.port();
  }
  return os;
}
#endif // BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x564))

} // namespace ip
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_IP_BASIC_ENDPOINT_HPP
