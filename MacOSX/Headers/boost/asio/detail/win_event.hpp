//
// win_event.hpp
// ~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_WIN_EVENT_HPP
#define BOOST_ASIO_DETAIL_WIN_EVENT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <boost/config.hpp>
#include <boost/system/system_error.hpp>
#include <boost/asio/detail/pop_options.hpp>

#if defined(BOOST_WINDOWS)

#include <boost/asio/error.hpp>
#include <boost/asio/detail/noncopyable.hpp>
#include <boost/asio/detail/socket_types.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <boost/assert.hpp>
#include <boost/throw_exception.hpp>
#include <boost/asio/detail/pop_options.hpp>

namespace boost {
namespace asio {
namespace detail {

class win_event
  : private noncopyable
{
public:
  // Constructor.
  win_event()
    : event_(::CreateEvent(0, true, false, 0))
  {
    if (!event_)
    {
      DWORD last_error = ::GetLastError();
      boost::system::system_error e(
          boost::system::error_code(last_error,
            boost::asio::error::get_system_category()),
          "event");
      boost::throw_exception(e);
    }
  }

  // Destructor.
  ~win_event()
  {
    ::CloseHandle(event_);
  }

  // Signal the event.
  template <typename Lock>
  void signal(Lock& lock)
  {
    BOOST_ASSERT(lock.locked());
    (void)lock;
    ::SetEvent(event_);
  }

  // Reset the event.
  template <typename Lock>
  void clear(Lock& lock)
  {
    BOOST_ASSERT(lock.locked());
    (void)lock;
    ::ResetEvent(event_);
  }

  // Wait for the event to become signalled.
  template <typename Lock>
  void wait(Lock& lock)
  {
    BOOST_ASSERT(lock.locked());
    lock.unlock();
    ::WaitForSingleObject(event_, INFINITE);
    lock.lock();
  }

private:
  HANDLE event_;
};

} // namespace detail
} // namespace asio
} // namespace boost

#endif // defined(BOOST_WINDOWS)

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_WIN_EVENT_HPP
