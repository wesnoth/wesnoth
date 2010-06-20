//
// null_signal_blocker.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_NULL_SIGNAL_BLOCKER_HPP
#define BOOST_ASIO_DETAIL_NULL_SIGNAL_BLOCKER_HPP

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

class null_signal_blocker
  : private noncopyable
{
public:
  // Constructor blocks all signals for the calling thread.
  null_signal_blocker()
  {
  }

  // Destructor restores the previous signal mask.
  ~null_signal_blocker()
  {
  }

  // Block all signals for the calling thread.
  void block()
  {
  }

  // Restore the previous signal mask.
  void unblock()
  {
  }
};

} // namespace detail
} // namespace asio
} // namespace boost

#endif // !defined(BOOST_HAS_THREADS)

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_NULL_SIGNAL_BLOCKER_HPP
