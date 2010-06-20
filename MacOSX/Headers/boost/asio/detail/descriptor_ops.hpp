//
// descriptor_ops.hpp
// ~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_DESCRIPTOR_OPS_HPP
#define BOOST_ASIO_DETAIL_DESCRIPTOR_OPS_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <boost/config.hpp>
#include <cerrno>
#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/error.hpp>
#include <boost/asio/detail/socket_types.hpp>

#if !defined(BOOST_WINDOWS) && !defined(__CYGWIN__)

namespace boost {
namespace asio {
namespace detail {
namespace descriptor_ops {

inline void clear_error(boost::system::error_code& ec)
{
  errno = 0;
  ec = boost::system::error_code();
}

template <typename ReturnType>
inline ReturnType error_wrapper(ReturnType return_value,
    boost::system::error_code& ec)
{
  ec = boost::system::error_code(errno,
      boost::asio::error::get_system_category());
  return return_value;
}

inline int open(const char* path, int flags, boost::system::error_code& ec)
{
  clear_error(ec);
  return error_wrapper(::open(path, flags), ec);
}

inline int close(int d, boost::system::error_code& ec)
{
  clear_error(ec);
  return error_wrapper(::close(d), ec);
}

inline void init_buf_iov_base(void*& base, void* addr)
{
  base = addr;
}

template <typename T>
inline void init_buf_iov_base(T& base, void* addr)
{
  base = static_cast<T>(addr);
}

typedef iovec buf;

inline void init_buf(buf& b, void* data, size_t size)
{
  init_buf_iov_base(b.iov_base, data);
  b.iov_len = size;
}

inline void init_buf(buf& b, const void* data, size_t size)
{
  init_buf_iov_base(b.iov_base, const_cast<void*>(data));
  b.iov_len = size;
}

inline int scatter_read(int d, buf* bufs, size_t count,
    boost::system::error_code& ec)
{
  clear_error(ec);
  return error_wrapper(::readv(d, bufs, static_cast<int>(count)), ec);
}

inline int gather_write(int d, const buf* bufs, size_t count,
    boost::system::error_code& ec)
{
  clear_error(ec);
  return error_wrapper(::writev(d, bufs, static_cast<int>(count)), ec);
}

inline int ioctl(int d, long cmd, ioctl_arg_type* arg,
    boost::system::error_code& ec)
{
  clear_error(ec);
  return error_wrapper(::ioctl(d, cmd, arg), ec);
}

inline int fcntl(int d, long cmd, boost::system::error_code& ec)
{
  clear_error(ec);
  return error_wrapper(::fcntl(d, cmd), ec);
}

inline int fcntl(int d, long cmd, long arg, boost::system::error_code& ec)
{
  clear_error(ec);
  return error_wrapper(::fcntl(d, cmd, arg), ec);
}

inline int poll_read(int d, boost::system::error_code& ec)
{
  clear_error(ec);
  pollfd fds;
  fds.fd = d;
  fds.events = POLLIN;
  fds.revents = 0;
  clear_error(ec);
  return error_wrapper(::poll(&fds, 1, -1), ec);
}

inline int poll_write(int d, boost::system::error_code& ec)
{
  clear_error(ec);
  pollfd fds;
  fds.fd = d;
  fds.events = POLLOUT;
  fds.revents = 0;
  clear_error(ec);
  return error_wrapper(::poll(&fds, 1, -1), ec);
}

} // namespace descriptor_ops
} // namespace detail
} // namespace asio
} // namespace boost

#endif // !defined(BOOST_WINDOWS) && !defined(__CYGWIN__)

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_DESCRIPTOR_OPS_HPP
