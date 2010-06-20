//
// eventfd_select_interrupter.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Copyright (c) 2008 Roelof Naude (roelof.naude at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_EVENTFD_SELECT_INTERRUPTER_HPP
#define BOOST_ASIO_DETAIL_EVENTFD_SELECT_INTERRUPTER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <boost/config.hpp>
#include <boost/throw_exception.hpp>
#include <boost/system/system_error.hpp>
#include <boost/asio/detail/pop_options.hpp>

#if defined(linux)
# if !defined(BOOST_ASIO_DISABLE_EVENTFD)
#  include <linux/version.h>
#  if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22)
#   define BOOST_ASIO_HAS_EVENTFD
#  endif // LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22)
# endif // !defined(BOOST_ASIO_DISABLE_EVENTFD)
#endif // defined(linux)

#if defined(BOOST_ASIO_HAS_EVENTFD)

#include <boost/asio/detail/push_options.hpp>
#include <fcntl.h>
#if __GLIBC__ == 2 && __GLIBC_MINOR__ < 8
# include <asm/unistd.h>
#else // __GLIBC__ == 2 && __GLIBC_MINOR__ < 8
# include <sys/eventfd.h>
#endif // __GLIBC__ == 2 && __GLIBC_MINOR__ < 8
#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/error.hpp>
#include <boost/asio/detail/socket_types.hpp>

namespace boost {
namespace asio {
namespace detail {

class eventfd_select_interrupter
{
public:
  // Constructor.
  eventfd_select_interrupter()
  {
#if __GLIBC__ == 2 && __GLIBC_MINOR__ < 8
    read_descriptor_ = syscall(__NR_eventfd, 0);
#else // __GLIBC__ == 2 && __GLIBC_MINOR__ < 8
    read_descriptor_ = ::eventfd(0, 0);
#endif // __GLIBC__ == 2 && __GLIBC_MINOR__ < 8
    if (read_descriptor_ != -1)
    {
      ::fcntl(read_descriptor_, F_SETFL, O_NONBLOCK);
    }
    else
    {
      boost::system::error_code ec(errno,
          boost::asio::error::get_system_category());
      boost::system::system_error e(ec, "eventfd_select_interrupter");
      boost::throw_exception(e);
    }
  }

  // Destructor.
  ~eventfd_select_interrupter()
  {
    if (read_descriptor_ != -1)
      ::close(read_descriptor_);
  }

  // Interrupt the select call.
  void interrupt()
  {
    uint64_t counter(1UL);
    ::write(read_descriptor_, &counter, sizeof(uint64_t));
  }

  // Reset the select interrupt. Returns true if the call was interrupted.
  bool reset()
  {
    // Only perform one read. The kernel maintains an atomic counter.
    uint64_t counter(0);
    int bytes_read = ::read(read_descriptor_, &counter, sizeof(uint64_t));
    bool was_interrupted = (bytes_read > 0);
    return was_interrupted;
  }

  // Get the read descriptor to be passed to select.
  int read_descriptor() const
  {
    return read_descriptor_;
  }

private:
  // The read end of a connection used to interrupt the select call. This file
  // descriptor is passed to select such that when it is time to stop, a single
  // 64bit value will be written on the other end of the connection and this
  // descriptor will become readable.
  int read_descriptor_;
};

} // namespace detail
} // namespace asio
} // namespace boost

#endif // defined(BOOST_ASIO_HAS_EVENTFD)

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_EVENTFD_SELECT_INTERRUPTER_HPP
