//
// win_tss_ptr.hpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_WIN_TSS_PTR_HPP
#define BOOST_ASIO_DETAIL_WIN_TSS_PTR_HPP

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
#include <boost/throw_exception.hpp>
#include <boost/asio/detail/pop_options.hpp>

namespace boost {
namespace asio {
namespace detail {

template <typename T>
class win_tss_ptr
  : private noncopyable
{
public:
#if defined(UNDER_CE)
  enum { out_of_indexes = 0xFFFFFFFF };
#else
  enum { out_of_indexes = TLS_OUT_OF_INDEXES };
#endif

  // Constructor.
  win_tss_ptr()
  {
    tss_key_ = ::TlsAlloc();
    if (tss_key_ == out_of_indexes)
    {
      DWORD last_error = ::GetLastError();
      boost::system::system_error e(
          boost::system::error_code(last_error,
            boost::asio::error::get_system_category()),
          "tss");
      boost::throw_exception(e);
    }
  }

  // Destructor.
  ~win_tss_ptr()
  {
    ::TlsFree(tss_key_);
  }

  // Get the value.
  operator T*() const
  {
    return static_cast<T*>(::TlsGetValue(tss_key_));
  }

  // Set the value.
  void operator=(T* value)
  {
    ::TlsSetValue(tss_key_, value);
  }

private:
  // Thread-specific storage to allow unlocked access to determine whether a
  // thread is a member of the pool.
  DWORD tss_key_;
};

} // namespace detail
} // namespace asio
} // namespace boost

#endif // defined(BOOST_WINDOWS)

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_WIN_TSS_PTR_HPP
