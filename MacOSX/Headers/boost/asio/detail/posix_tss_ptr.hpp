//
// posix_tss_ptr.hpp
// ~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_POSIX_TSS_PTR_HPP
#define BOOST_ASIO_DETAIL_POSIX_TSS_PTR_HPP

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
#include <boost/throw_exception.hpp>
#include <pthread.h>
#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/error.hpp>
#include <boost/asio/detail/noncopyable.hpp>

namespace boost {
namespace asio {
namespace detail {

template <typename T>
class posix_tss_ptr
  : private noncopyable
{
public:
  // Constructor.
  posix_tss_ptr()
  {
    int error = ::pthread_key_create(&tss_key_, 0);
    if (error != 0)
    {
      boost::system::system_error e(
          boost::system::error_code(error,
            boost::asio::error::get_system_category()),
          "tss");
      boost::throw_exception(e);
    }
  }

  // Destructor.
  ~posix_tss_ptr()
  {
    ::pthread_key_delete(tss_key_);
  }

  // Get the value.
  operator T*() const
  {
    return static_cast<T*>(::pthread_getspecific(tss_key_));
  }

  // Set the value.
  void operator=(T* value)
  {
    ::pthread_setspecific(tss_key_, value);
  }

private:
  // Thread-specific storage to allow unlocked access to determine whether a
  // thread is a member of the pool.
  pthread_key_t tss_key_;
};

} // namespace detail
} // namespace asio
} // namespace boost

#endif // defined(BOOST_HAS_PTHREADS)

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_POSIX_TSS_PTR_HPP
