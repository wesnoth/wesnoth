//
// address_v4.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_IP_ADDRESS_V4_HPP
#define BOOST_ASIO_IP_ADDRESS_V4_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <climits>
#include <string>
#include <stdexcept>
#include <boost/array.hpp>
#include <boost/throw_exception.hpp>
#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/error.hpp>
#include <boost/asio/detail/socket_ops.hpp>
#include <boost/asio/detail/socket_types.hpp>
#include <boost/asio/detail/throw_error.hpp>

namespace boost {
namespace asio {
namespace ip {

/// Implements IP version 4 style addresses.
/**
 * The boost::asio::ip::address_v4 class provides the ability to use and
 * manipulate IP version 4 addresses.
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Unsafe.
 */
class address_v4
{
public:
  /// The type used to represent an address as an array of bytes.
  typedef boost::array<unsigned char, 4> bytes_type;

  /// Default constructor.
  address_v4()
  {
    addr_.s_addr = 0;
  }

  /// Construct an address from raw bytes.
  explicit address_v4(const bytes_type& bytes)
  {
#if UCHAR_MAX > 0xFF
    if (bytes[0] > 0xFF || bytes[1] > 0xFF
        || bytes[2] > 0xFF || bytes[3] > 0xFF)
    {
      std::out_of_range ex("address_v4 from bytes_type");
      boost::throw_exception(ex);
    }
#endif // UCHAR_MAX > 0xFF

    using namespace std; // For memcpy.
    memcpy(&addr_.s_addr, bytes.elems, 4);
  }

  /// Construct an address from a unsigned long in host byte order.
  explicit address_v4(unsigned long addr)
  {
#if ULONG_MAX > 0xFFFFFFFF
    if (addr > 0xFFFFFFFF)
    {
      std::out_of_range ex("address_v4 from unsigned long");
      boost::throw_exception(ex);
    }
#endif // ULONG_MAX > 0xFFFFFFFF

    addr_.s_addr = boost::asio::detail::socket_ops::host_to_network_long(addr);
  }

  /// Copy constructor.
  address_v4(const address_v4& other)
    : addr_(other.addr_)
  {
  }

  /// Assign from another address.
  address_v4& operator=(const address_v4& other)
  {
    addr_ = other.addr_;
    return *this;
  }

  /// Get the address in bytes.
  bytes_type to_bytes() const
  {
    using namespace std; // For memcpy.
    bytes_type bytes;
    memcpy(bytes.elems, &addr_.s_addr, 4);
    return bytes;
  }

  /// Get the address as an unsigned long in host byte order
  unsigned long to_ulong() const
  {
    return boost::asio::detail::socket_ops::network_to_host_long(addr_.s_addr);
  }

  /// Get the address as a string in dotted decimal format.
  std::string to_string() const
  {
    boost::system::error_code ec;
    std::string addr = to_string(ec);
    boost::asio::detail::throw_error(ec);
    return addr;
  }

  /// Get the address as a string in dotted decimal format.
  std::string to_string(boost::system::error_code& ec) const
  {
    char addr_str[boost::asio::detail::max_addr_v4_str_len];
    const char* addr =
      boost::asio::detail::socket_ops::inet_ntop(AF_INET, &addr_, addr_str,
          boost::asio::detail::max_addr_v4_str_len, 0, ec);
    if (addr == 0)
      return std::string();
    return addr;
  }

  /// Create an address from an IP address string in dotted decimal form.
  static address_v4 from_string(const char* str)
  {
    boost::system::error_code ec;
    address_v4 addr = from_string(str, ec);
    boost::asio::detail::throw_error(ec);
    return addr;
  }

  /// Create an address from an IP address string in dotted decimal form.
  static address_v4 from_string(const char* str, boost::system::error_code& ec)
  {
    address_v4 tmp;
    if (boost::asio::detail::socket_ops::inet_pton(
          AF_INET, str, &tmp.addr_, 0, ec) <= 0)
      return address_v4();
    return tmp;
  }

  /// Create an address from an IP address string in dotted decimal form.
  static address_v4 from_string(const std::string& str)
  {
    return from_string(str.c_str());
  }

  /// Create an address from an IP address string in dotted decimal form.
  static address_v4 from_string(const std::string& str,
      boost::system::error_code& ec)
  {
    return from_string(str.c_str(), ec);
  }

  /// Determine whether the address is a class A address.
  bool is_class_a() const
  {
    return IN_CLASSA(to_ulong());
  }

  /// Determine whether the address is a class B address.
  bool is_class_b() const
  {
    return IN_CLASSB(to_ulong());
  }

  /// Determine whether the address is a class C address.
  bool is_class_c() const
  {
    return IN_CLASSC(to_ulong());
  }

  /// Determine whether the address is a multicast address.
  bool is_multicast() const
  {
    return IN_MULTICAST(to_ulong());
  }

  /// Compare two addresses for equality.
  friend bool operator==(const address_v4& a1, const address_v4& a2)
  {
    return a1.addr_.s_addr == a2.addr_.s_addr;
  }

  /// Compare two addresses for inequality.
  friend bool operator!=(const address_v4& a1, const address_v4& a2)
  {
    return a1.addr_.s_addr != a2.addr_.s_addr;
  }

  /// Compare addresses for ordering.
  friend bool operator<(const address_v4& a1, const address_v4& a2)
  {
    return a1.to_ulong() < a2.to_ulong();
  }

  /// Compare addresses for ordering.
  friend bool operator>(const address_v4& a1, const address_v4& a2)
  {
    return a1.to_ulong() > a2.to_ulong();
  }

  /// Compare addresses for ordering.
  friend bool operator<=(const address_v4& a1, const address_v4& a2)
  {
    return a1.to_ulong() <= a2.to_ulong();
  }

  /// Compare addresses for ordering.
  friend bool operator>=(const address_v4& a1, const address_v4& a2)
  {
    return a1.to_ulong() >= a2.to_ulong();
  }

  /// Obtain an address object that represents any address.
  static address_v4 any()
  {
    return address_v4(static_cast<unsigned long>(INADDR_ANY));
  }

  /// Obtain an address object that represents the loopback address.
  static address_v4 loopback()
  {
    return address_v4(static_cast<unsigned long>(INADDR_LOOPBACK));
  }

  /// Obtain an address object that represents the broadcast address.
  static address_v4 broadcast()
  {
    return address_v4(static_cast<unsigned long>(INADDR_BROADCAST));
  }

  /// Obtain an address object that represents the broadcast address that
  /// corresponds to the specified address and netmask.
  static address_v4 broadcast(const address_v4& addr, const address_v4& mask)
  {
    return address_v4(addr.to_ulong() | ~mask.to_ulong());
  }

  /// Obtain the netmask that corresponds to the address, based on its address
  /// class.
  static address_v4 netmask(const address_v4& addr)
  {
    if (addr.is_class_a())
      return address_v4(0xFF000000);
    if (addr.is_class_b())
      return address_v4(0xFFFF0000);
    if (addr.is_class_c())
      return address_v4(0xFFFFFF00);
    return address_v4(0xFFFFFFFF);
  }

private:
  // The underlying IPv4 address.
  boost::asio::detail::in4_addr_type addr_;
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
 * @relates boost::asio::ip::address_v4
 */
template <typename Elem, typename Traits>
std::basic_ostream<Elem, Traits>& operator<<(
    std::basic_ostream<Elem, Traits>& os, const address_v4& addr)
{
  boost::system::error_code ec;
  std::string s = addr.to_string(ec);
  if (ec)
  {
    if (os.exceptions() & std::ios::failbit)
      boost::asio::detail::throw_error(ec);
    else
      os.setstate(std::ios_base::failbit);
  }
  else
    for (std::string::iterator i = s.begin(); i != s.end(); ++i)
      os << os.widen(*i);
  return os;
}

} // namespace ip
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_IP_ADDRESS_V4_HPP
