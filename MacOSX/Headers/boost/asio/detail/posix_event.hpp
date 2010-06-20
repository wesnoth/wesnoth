//
// posix_event.hpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_POSIX_EVENT_HPP
#define BOOST_ASIO_DETAIL_POSIX_EVENT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <boost/config.hpp>
#include <boost/system/system_error.hpp>
#include <boost/asio/detail/pop_options.hpp>

#if defined(BOOST_HAS_PTHREADS)

#include <boost/asio/detail/push_options.hpp>
#include <boost/assert.hpp>
#include <boost/throw_exception.hpp>
#include <pthread.h>
#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/error.hpp>
#include <boost/asio/detail/noncopyable.hpp>

namespace boost {
namespace asio {
namespace detail {

class posix_event
  : private noncopyable
{
public:
  // Constructor.
  posix_event()
    : signalled_(false)
  {
    int error = ::pthread_cond_init(&cond_, 0);
    if (error != 0)
    {
      boost::system::system_error e(
          boost::system::error_code(error,
            boost::asio::error::get_system_category()),
          "event");
      boost::throw_exception(e);
    }
  }

  // Destructor.
  ~posix_event()
  {
    ::pthread_cond_destroy(&cond_);
  }

  // Signal the event.
  template <typename Lock>
  void signal(Lock& lock)
  {
    BOOST_ASSERT(lock.locked());
    (void)lock;
    signalled_ = true;
    ::pthread_cond_signal(&cond_); // Ignore EINVAL.
  }

  // Reset the event.
  template <typename Lock>
  void clear(Lock& lock)
  {
    BOOST_ASSERT(lock.locked());
    (void)lock;
    signalled_ = false;
  }

  // Wait for the event to become signalled.
  template <typename Lock>
  void wait(Lock& lock)
  {
    BOOST_ASSERT(lock.locked());
    while (!signalled_)
      ::pthread_cond_wait(&cond_, &lock.mutex().mutex_); // Ignore EINVAL.
  }

private:
  ::pthread_cond_t cond_;
  bool signalled_;
};

} // namespace detail
} // namespace asio
} // namespace boost

#endif // defined(BOOST_HAS_PTHREADS)

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_POSIX_EVENT_HPP
