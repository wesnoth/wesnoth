//
// timer_queue_base.hpp
// ~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_TIMER_QUEUE_BASE_HPP
#define BOOST_ASIO_DETAIL_TIMER_QUEUE_BASE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/socket_types.hpp> // Must come before posix_time.

#include <boost/asio/detail/push_options.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/detail/noncopyable.hpp>

namespace boost {
namespace asio {
namespace detail {

class timer_queue_base
  : private noncopyable
{
public:
  // Destructor.
  virtual ~timer_queue_base() {}

  // Whether there are no timers in the queue.
  virtual bool empty() const = 0;

  // Get the time to wait until the next timer.
  virtual boost::posix_time::time_duration wait_duration() const = 0;

  // Dispatch all ready timers.
  virtual void dispatch_timers() = 0;

  // Dispatch any pending cancels for timers.
  virtual void dispatch_cancellations() = 0;

  // Complete all timers that are waiting to be completed.
  virtual void complete_timers() = 0;

  // Destroy all timers.
  virtual void destroy_timers() = 0;
};

} // namespace detail
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_TIMER_QUEUE_BASE_HPP
