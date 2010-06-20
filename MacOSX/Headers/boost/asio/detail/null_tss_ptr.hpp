//
// null_tss_ptr.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_NULL_TSS_PTR_HPP
#define BOOST_ASIO_DETAIL_NULL_TSS_PTR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <boost/config.hpp>
#include <boost/asio/detail/pop_options.hpp>

#if !defined(BOOST_HAS_THREADS)

#include <boost/asio/detail/noncopyable.hpp>

namespace boost {
namespace asio {
namespace detail {

template <typename T>
class null_tss_ptr
  : private noncopyable
{
public:
  // Constructor.
  null_tss_ptr()
    : value_(0)
  {
  }

  // Destructor.
  ~null_tss_ptr()
  {
  }

  // Get the value.
  operator T*() const
  {
    return value_;
  }

  // Set the value.
  void operator=(T* value)
  {
    value_ = value;
  }

private:
  T* value_;
};

} // namespace detail
} // namespace asio
} // namespace boost

#endif // !defined(BOOST_HAS_THREADS)

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_NULL_TSS_PTR_HPP
