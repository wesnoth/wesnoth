//
// address.hpp
// ~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_IP_ADDRESS_HPP
#define BOOST_ASIO_IP_ADDRESS_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <iosfwd>
#include <string>
#include <boost/throw_exception.hpp>
#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/error.hpp>
#include <boost/asio/ip/address_v4.hpp>
#include <boost/asio/ip/address_v6.hpp>
#include <boost/asio/detail/throw_error.hpp>

namespace boost {
namespace asio {
namespace ip {

/// Implements version-independent IP addresses.
/**
 * The boost::asio::ip::address class provides the ability to use either IP
 * version 4 or version 6 addresses.
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Unsafe.
 */
class address
{
public:
  /// Default constructor.
  address()
    : type_(ipv4),
      ipv4_address_(),
      ipv6_address_()
  {
  }

  /// Construct an address from an IPv4 address.
  address(const boost::asio::ip::address_v4& ipv4_address)
    : type_(ipv4),
      ipv4_address_(ipv4_address),
      ipv6_address_()
  {
  }

  /// Construct an address from an IPv6 address.
  address(const boost::asio::ip::address_v6& ipv6_address)
    : type_(ipv6),
      ipv4_address_(),
      ipv6_address_(ipv6_address)
  {
  }

  /// Copy constructor.
  address(const address& other)
    : type_(other.type_),
      ipv4_address_(other.ipv4_address_),
      ipv6_address_(other.ipv6_address_)
  {
  }

  /// Assign from another address.
  address& operator=(const address& other)
  {
    type_ = other.type_;
    ipv4_address_ = other.ipv4_address_;
    ipv6_address_ = other.ipv6_address_;
    return *this;
  }

  /// Assign from an IPv4 address.
  address& operator=(const boost::asio::ip::address_v4& ipv4_address)
  {
    type_ = ipv4;
    ipv4_address_ = ipv4_address;
    ipv6_address_ = boost::asio::ip::address_v6();
    return *this;
  }

  /// Assign from an IPv6 address.
  address& operator=(const boost::asio::ip::address_v6& ipv6_address)
  {
    type_ = ipv6;
    ipv4_address_ = boost::asio::ip::address_v4();
    ipv6_address_ = ipv6_address;
    return *this;
  }

  /// Get whether the address is an IP version 4 address.
  bool is_v4() const
  {
    return type_ == ipv4;
  }

  /// Get whether the address is an IP version 6 address.
  bool is_v6() const
  {
    return type_ == ipv6;
  }

  /// Get the address as an IP version 4 address.
  boost::asio::ip::address_v4 to_v4() const
  {
    if (type_ != ipv4)
    {
      boost::system::system_error e(
          boost::asio::error::address_family_not_supported);
      boost::throw_exception(e);
    }
    return ipv4_address_;
  }

  /// Get the address as an IP version 6 address.
  boost::asio::ip::address_v6 to_v6() const
  {
    if (type_ != ipv6)
    {
      boost::system::system_error e(
          boost::asio::error::address_family_not_supported);
      boost::throw_exception(e);
    }
    return ipv6_address_;
  }

  /// Get the address as a string in dotted decimal format.
  std::string to_string() const
  {
    if (type_ == ipv6)
      return ipv6_address_.to_string();
    return ipv4_address_.to_string();
  }

  /// Get the address as a string in dotted decimal format.
  std::string to_string(boost::system::error_code& ec) const
  {
    if (type_ == ipv6)
      return ipv6_address_.to_string(ec);
    return ipv4_address_.to_string(ec);
  }

  /// Create an address from an IPv4 address string in dotted decimal form,
  /// or from an IPv6 address in hexadecimal notation.
  static address from_string(const char* str)
  {
    boost::system::error_code ec;
    address addr = from_string(str, ec);
    boost::asio::detail::throw_error(ec);
    return addr;
  }

  /// Create an address from an IPv4 address string in dotted decimal form,
  /// or from an IPv6 address in hexadecimal notation.
  static address from_string(const char* str, boost::system::error_code& ec)
  {
    boost::asio::ip::address_v6 ipv6_address =
      boost::asio::ip::address_v6::from_string(str, ec);
    if (!ec)
    {
      address tmp;
      tmp.type_ = ipv6;
      tmp.ipv6_address_ = ipv6_address;
      return tmp;
    }

    boost::asio::ip::address_v4 ipv4_address =
      boost::asio::ip::address_v4::from_string(str, ec);
    if (!ec)
    {
      address tmp;
      tmp.type_ = ipv4;
      tmp.ipv4_address_ = ipv4_address;
      return tmp;
    }

    return address();
  }

  /// Create an address from an IPv4 address string in dotted decimal form,
  /// or from an IPv6 address in hexadecimal notation.
  static address from_string(const std::string& str)
  {
    return from_string(str.c_str());
  }

  /// Create an address from an IPv4 address string in dotted decimal form,
  /// or from an IPv6 address in hexadecimal notation.
  static address from_string(const std::string& str,
      boost::system::error_code& ec)
  {
    return from_string(str.c_str(), ec);
  }

  /// Compare two addresses for equality.
  friend bool operator==(const address& a1, const address& a2)
  {
    if (a1.type_ != a2.type_)
      return false;
    if (a1.type_ == ipv6)
      return a1.ipv6_address_ == a2.ipv6_address_;
    return a1.ipv4_address_ == a2.ipv4_address_;
  }

  /// Compare two addresses for inequality.
  friend bool operator!=(const address& a1, const address& a2)
  {
    if (a1.type_ != a2.type_)
      return true;
    if (a1.type_ == ipv6)
      return a1.ipv6_address_ != a2.ipv6_address_;
    return a1.ipv4_address_ != a2.ipv4_address_;
  }

  /// Compare addresses for ordering.
  friend bool operator<(const address& a1, const address& a2)
  {
    if (a1.type_ < a2.type_)
      return true;
    if (a1.type_ > a2.type_)
      return false;
    if (a1.type_ == ipv6)
      return a1.ipv6_address_ < a2.ipv6_address_;
    return a1.ipv4_address_ < a2.ipv4_address_;
  }

private:
  // The type of the address.
  enum { ipv4, ipv6 } type_;

  // The underlying IPv4 address.
  boost::asio::ip::address_v4 ipv4_address_;

  // The underlying IPv6 address.
  boost::asio::ip::address_v6 ipv6_address_;
};

/// Output an address as a string.
/**
 * Used to output a human-readable string for a specified address.
 *
 * @param os The output stream to which the string will be written.
 *
 * @param addr The address to be written.
 *
 * @return The output stream.
 *
 * @relates boost::asio::ip::address
 */
template <typename Elem, typename Traits>
std::basic_ostream<Elem, Traits>& operator<<(
    std::basic_ostream<Elem, Traits>& os, const address& addr)
{
  os << addr.to_string();
  return os;
}

} // namespace ip
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_IP_ADDRESS_HPP
