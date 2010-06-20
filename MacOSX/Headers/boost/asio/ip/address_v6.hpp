//
// address_v6.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_IP_ADDRESS_V6_HPP
#define BOOST_ASIO_IP_ADDRESS_V6_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <cstring>
#include <string>
#include <stdexcept>
#include <typeinfo>
#include <boost/array.hpp>
#include <boost/throw_exception.hpp>
#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/error.hpp>
#include <boost/asio/detail/socket_ops.hpp>
#include <boost/asio/detail/socket_types.hpp>
#include <boost/asio/detail/throw_error.hpp>
#include <boost/asio/ip/address_v4.hpp>

namespace boost {
namespace asio {
namespace ip {

/// Implements IP version 6 style addresses.
/**
 * The boost::asio::ip::address_v6 class provides the ability to use and
 * manipulate IP version 6 addresses.
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Unsafe.
 */
class address_v6
{
public:
  /// The type used to represent an address as an array of bytes.
  typedef boost::array<unsigned char, 16> bytes_type;

  /// Default constructor.
  address_v6()
    : scope_id_(0)
  {
    boost::asio::detail::in6_addr_type tmp_addr = IN6ADDR_ANY_INIT;
    addr_ = tmp_addr;
  }

  /// Construct an address from raw bytes and scope ID.
  explicit address_v6(const bytes_type& bytes, unsigned long scope_id = 0)
    : scope_id_(scope_id)
  {
#if UCHAR_MAX > 0xFF
    for (std::size_t i = 0; i < bytes.size(); ++i)
    {
      if (bytes[i] > 0xFF)
      {
        std::out_of_range ex("address_v6 from bytes_type");
        boost::throw_exception(ex);
      }
    }
#endif // UCHAR_MAX > 0xFF

    using namespace std; // For memcpy.
    memcpy(addr_.s6_addr, bytes.elems, 16);
  }

  /// Copy constructor.
  address_v6(const address_v6& other)
    : addr_(other.addr_),
      scope_id_(other.scope_id_)
  {
  }

  /// Assign from another address.
  address_v6& operator=(const address_v6& other)
  {
    addr_ = other.addr_;
    scope_id_ = other.scope_id_;
    return *this;
  }

  /// The scope ID of the address.
  /**
   * Returns the scope ID associated with the IPv6 address.
   */
  unsigned long scope_id() const
  {
    return scope_id_;
  }

  /// The scope ID of the address.
  /**
   * Modifies the scope ID associated with the IPv6 address.
   */
  void scope_id(unsigned long id)
  {
    scope_id_ = id;
  }

  /// Get the address in bytes.
  bytes_type to_bytes() const
  {
    using namespace std; // For memcpy.
    bytes_type bytes;
    memcpy(bytes.elems, addr_.s6_addr, 16);
    return bytes;
  }

  /// Get the address as a string.
  std::string to_string() const
  {
    boost::system::error_code ec;
    std::string addr = to_string(ec);
    boost::asio::detail::throw_error(ec);
    return addr;
  }

  /// Get the address as a string.
  std::string to_string(boost::system::error_code& ec) const
  {
    char addr_str[boost::asio::detail::max_addr_v6_str_len];
    const char* addr =
      boost::asio::detail::socket_ops::inet_ntop(AF_INET6, &addr_, addr_str,
          boost::asio::detail::max_addr_v6_str_len, scope_id_, ec);
    if (addr == 0)
      return std::string();
    return addr;
  }

  /// Create an address from an IP address string.
  static address_v6 from_string(const char* str)
  {
    boost::system::error_code ec;
    address_v6 addr = from_string(str, ec);
    boost::asio::detail::throw_error(ec);
    return addr;
  }

  /// Create an address from an IP address string.
  static address_v6 from_string(const char* str, boost::system::error_code& ec)
  {
    address_v6 tmp;
    if (boost::asio::detail::socket_ops::inet_pton(
          AF_INET6, str, &tmp.addr_, &tmp.scope_id_, ec) <= 0)
      return address_v6();
    return tmp;
  }

  /// Create an address from an IP address string.
  static address_v6 from_string(const std::string& str)
  {
    return from_string(str.c_str());
  }

  /// Create an address from an IP address string.
  static address_v6 from_string(const std::string& str,
      boost::system::error_code& ec)
  {
    return from_string(str.c_str(), ec);
  }

  /// Converts an IPv4-mapped or IPv4-compatible address to an IPv4 address.
  address_v4 to_v4() const
  {
    if (!is_v4_mapped() && !is_v4_compatible())
    {
      std::bad_cast ex;
      boost::throw_exception(ex);
    }

    address_v4::bytes_type v4_bytes = { { addr_.s6_addr[12],
      addr_.s6_addr[13], addr_.s6_addr[14], addr_.s6_addr[15] } };
    return address_v4(v4_bytes);
  }

  /// Determine whether the address is a loopback address.
  bool is_loopback() const
  {
#if defined(__BORLANDC__)
    return ((addr_.s6_addr[0] == 0) && (addr_.s6_addr[1] == 0)
        && (addr_.s6_addr[2] == 0) && (addr_.s6_addr[3] == 0)
        && (addr_.s6_addr[4] == 0) && (addr_.s6_addr[5] == 0)
        && (addr_.s6_addr[6] == 0) && (addr_.s6_addr[7] == 0)
        && (addr_.s6_addr[8] == 0) && (addr_.s6_addr[9] == 0)
        && (addr_.s6_addr[10] == 0) && (addr_.s6_addr[11] == 0)
        && (addr_.s6_addr[12] == 0) && (addr_.s6_addr[13] == 0)
        && (addr_.s6_addr[14] == 0) && (addr_.s6_addr[15] == 1));
#else
    using namespace boost::asio::detail;
    return IN6_IS_ADDR_LOOPBACK(&addr_) != 0;
#endif
  }

  /// Determine whether the address is unspecified.
  bool is_unspecified() const
  {
#if defined(__BORLANDC__)
    return ((addr_.s6_addr[0] == 0) && (addr_.s6_addr[1] == 0)
        && (addr_.s6_addr[2] == 0) && (addr_.s6_addr[3] == 0)
        && (addr_.s6_addr[4] == 0) && (addr_.s6_addr[5] == 0)
        && (addr_.s6_addr[6] == 0) && (addr_.s6_addr[7] == 0)
        && (addr_.s6_addr[8] == 0) && (addr_.s6_addr[9] == 0)
        && (addr_.s6_addr[10] == 0) && (addr_.s6_addr[11] == 0)
        && (addr_.s6_addr[12] == 0) && (addr_.s6_addr[13] == 0)
        && (addr_.s6_addr[14] == 0) && (addr_.s6_addr[15] == 0));
#else
    using namespace boost::asio::detail;
    return IN6_IS_ADDR_UNSPECIFIED(&addr_) != 0;
#endif
  }

  /// Determine whether the address is link local.
  bool is_link_local() const
  {
    using namespace boost::asio::detail;
    return IN6_IS_ADDR_LINKLOCAL(&addr_) != 0;
  }

  /// Determine whether the address is site local.
  bool is_site_local() const
  {
    using namespace boost::asio::detail;
    return IN6_IS_ADDR_SITELOCAL(&addr_) != 0;
  }

  /// Determine whether the address is a mapped IPv4 address.
  bool is_v4_mapped() const
  {
    using namespace boost::asio::detail;
    return IN6_IS_ADDR_V4MAPPED(&addr_) != 0;
  }

  /// Determine whether the address is an IPv4-compatible address.
  bool is_v4_compatible() const
  {
    using namespace boost::asio::detail;
    return IN6_IS_ADDR_V4COMPAT(&addr_) != 0;
  }

  /// Determine whether the address is a multicast address.
  bool is_multicast() const
  {
    using namespace boost::asio::detail;
    return IN6_IS_ADDR_MULTICAST(&addr_) != 0;
  }

  /// Determine whether the address is a global multicast address.
  bool is_multicast_global() const
  {
    using namespace boost::asio::detail;
    return IN6_IS_ADDR_MC_GLOBAL(&addr_) != 0;
  }

  /// Determine whether the address is a link-local multicast address.
  bool is_multicast_link_local() const
  {
    using namespace boost::asio::detail;
    return IN6_IS_ADDR_MC_LINKLOCAL(&addr_) != 0;
  }

  /// Determine whether the address is a node-local multicast address.
  bool is_multicast_node_local() const
  {
    using namespace boost::asio::detail;
    return IN6_IS_ADDR_MC_NODELOCAL(&addr_) != 0;
  }

  /// Determine whether the address is a org-local multicast address.
  bool is_multicast_org_local() const
  {
    using namespace boost::asio::detail;
    return IN6_IS_ADDR_MC_ORGLOCAL(&addr_) != 0;
  }

  /// Determine whether the address is a site-local multicast address.
  bool is_multicast_site_local() const
  {
    using namespace boost::asio::detail;
    return IN6_IS_ADDR_MC_SITELOCAL(&addr_) != 0;
  }

  /// Compare two addresses for equality.
  friend bool operator==(const address_v6& a1, const address_v6& a2)
  {
    using namespace std; // For memcmp.
    return memcmp(&a1.addr_, &a2.addr_,
        sizeof(boost::asio::detail::in6_addr_type)) == 0
      && a1.scope_id_ == a2.scope_id_;
  }

  /// Compare two addresses for inequality.
  friend bool operator!=(const address_v6& a1, const address_v6& a2)
  {
    using namespace std; // For memcmp.
    return memcmp(&a1.addr_, &a2.addr_,
        sizeof(boost::asio::detail::in6_addr_type)) != 0
      || a1.scope_id_ != a2.scope_id_;
  }

  /// Compare addresses for ordering.
  friend bool operator<(const address_v6& a1, const address_v6& a2)
  {
    using namespace std; // For memcmp.
    int memcmp_result = memcmp(&a1.addr_, &a2.addr_,
        sizeof(boost::asio::detail::in6_addr_type));
    if (memcmp_result < 0)
      return true;
    if (memcmp_result > 0)
      return false;
    return a1.scope_id_ < a2.scope_id_;
  }

  /// Compare addresses for ordering.
  friend bool operator>(const address_v6& a1, const address_v6& a2)
  {
    return a2 < a1;
  }

  /// Compare addresses for ordering.
  friend bool operator<=(const address_v6& a1, const address_v6& a2)
  {
    return !(a2 < a1);
  }

  /// Compare addresses for ordering.
  friend bool operator>=(const address_v6& a1, const address_v6& a2)
  {
    return !(a1 < a2);
  }

  /// Obtain an address object that represents any address.
  static address_v6 any()
  {
    return address_v6();
  }

  /// Obtain an address object that represents the loopback address.
  static address_v6 loopback()
  {
    address_v6 tmp;
    boost::asio::detail::in6_addr_type tmp_addr = IN6ADDR_LOOPBACK_INIT;
    tmp.addr_ = tmp_addr;
    return tmp;
  }

  /// Create an IPv4-mapped IPv6 address.
  static address_v6 v4_mapped(const address_v4& addr)
  {
    address_v4::bytes_type v4_bytes = addr.to_bytes();
    bytes_type v6_bytes = { { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xFF, 0xFF,
      v4_bytes[0], v4_bytes[1], v4_bytes[2], v4_bytes[3] } };
    return address_v6(v6_bytes);
  }

  /// Create an IPv4-compatible IPv6 address.
  static address_v6 v4_compatible(const address_v4& addr)
  {
    address_v4::bytes_type v4_bytes = addr.to_bytes();
    bytes_type v6_bytes = { { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      v4_bytes[0], v4_bytes[1], v4_bytes[2], v4_bytes[3] } };
    return address_v6(v6_bytes);
  }

private:
  // The underlying IPv6 address.
  boost::asio::detail::in6_addr_type addr_;

  // The scope ID associated with the address.
  unsigned long scope_id_;
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
 * @relates boost::asio::ip::address_v6
 */
template <typename Elem, typename Traits>
std::basic_ostream<Elem, Traits>& operator<<(
    std::basic_ostream<Elem, Traits>& os, const address_v6& addr)
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

#endif // BOOST_ASIO_IP_ADDRESS_V6_HPP
