//
// basic_resolver_iterator.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_IP_BASIC_RESOLVER_ITERATOR_HPP
#define BOOST_ASIO_IP_BASIC_RESOLVER_ITERATOR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include <cstring>
#include <string>
#include <vector>
#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/detail/socket_ops.hpp>
#include <boost/asio/detail/socket_types.hpp>
#include <boost/asio/ip/basic_resolver_entry.hpp>

namespace boost {
namespace asio {
namespace ip {

/// An iterator over the entries produced by a resolver.
/**
 * The boost::asio::ip::basic_resolver_iterator class template is used to define
 * iterators over the results returned by a resolver.
 *
 * The iterator's value_type, obtained when the iterator is dereferenced, is:
 * @code const basic_resolver_entry<InternetProtocol> @endcode
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Unsafe.
 */
template <typename InternetProtocol>
class basic_resolver_iterator
  : public boost::iterator_facade<
        basic_resolver_iterator<InternetProtocol>,
        const basic_resolver_entry<InternetProtocol>,
        boost::forward_traversal_tag>
{
public:
  /// Default constructor creates an end iterator.
  basic_resolver_iterator()
  {
  }

  /// Create an iterator from an addrinfo list returned by getaddrinfo.
  static basic_resolver_iterator create(
      boost::asio::detail::addrinfo_type* address_info,
      const std::string& host_name, const std::string& service_name)
  {
    basic_resolver_iterator iter;
    if (!address_info)
      return iter;

    std::string actual_host_name = host_name;
    if (address_info->ai_canonname)
      actual_host_name = address_info->ai_canonname;

    iter.values_.reset(new values_type);

    while (address_info)
    {
      if (address_info->ai_family == PF_INET
          || address_info->ai_family == PF_INET6)
      {
        using namespace std; // For memcpy.
        typename InternetProtocol::endpoint endpoint;
        endpoint.resize(static_cast<std::size_t>(address_info->ai_addrlen));
        memcpy(endpoint.data(), address_info->ai_addr,
            address_info->ai_addrlen);
        iter.values_->push_back(
            basic_resolver_entry<InternetProtocol>(endpoint,
              actual_host_name, service_name));
      }
      address_info = address_info->ai_next;
    }

    if (iter.values_->size())
      iter.iter_ = iter.values_->begin();
    else
      iter.values_.reset();

    return iter;
  }

  /// Create an iterator from an endpoint, host name and service name.
  static basic_resolver_iterator create(
      const typename InternetProtocol::endpoint& endpoint,
      const std::string& host_name, const std::string& service_name)
  {
    basic_resolver_iterator iter;
    iter.values_.reset(new values_type);
    iter.values_->push_back(
        basic_resolver_entry<InternetProtocol>(
          endpoint, host_name, service_name));
    iter.iter_ = iter.values_->begin();
    return iter;
  }

private:
  friend class boost::iterator_core_access;

  void increment()
  {
    if (++*iter_ == values_->end())
    {
      // Reset state to match a default constructed end iterator.
      values_.reset();
      typedef typename values_type::const_iterator values_iterator_type;
      iter_.reset();
    }
  }

  bool equal(const basic_resolver_iterator& other) const
  {
    if (!values_ && !other.values_)
      return true;
    if (values_ != other.values_)
      return false;
    return *iter_ == *other.iter_;
  }

  const basic_resolver_entry<InternetProtocol>& dereference() const
  {
    return **iter_;
  }

  typedef std::vector<basic_resolver_entry<InternetProtocol> > values_type;
  typedef typename values_type::const_iterator values_iter_type;
  boost::shared_ptr<values_type> values_;
  boost::optional<values_iter_type> iter_;
};

} // namespace ip
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_IP_BASIC_RESOLVER_ITERATOR_HPP
