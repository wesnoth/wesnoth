//
// socket_select_interrupter.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_SOCKET_SELECT_INTERRUPTER_HPP
#define BOOST_ASIO_DETAIL_SOCKET_SELECT_INTERRUPTER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <cstdlib>
#include <boost/throw_exception.hpp>
#include <boost/system/system_error.hpp>
#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/error.hpp>
#include <boost/asio/detail/socket_holder.hpp>
#include <boost/asio/detail/socket_ops.hpp>
#include <boost/asio/detail/socket_types.hpp>

namespace boost {
namespace asio {
namespace detail {

class socket_select_interrupter
{
public:
  // Constructor.
  socket_select_interrupter()
  {
    boost::system::error_code ec;
    socket_holder acceptor(socket_ops::socket(
          AF_INET, SOCK_STREAM, IPPROTO_TCP, ec));
    if (acceptor.get() == invalid_socket)
    {
      boost::system::system_error e(ec, "socket_select_interrupter");
      boost::throw_exception(e);
    }

    int opt = 1;
    socket_ops::setsockopt(acceptor.get(),
        SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt), ec);

    using namespace std; // For memset.
    sockaddr_in4_type addr;
    std::size_t addr_len = sizeof(addr);
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = 0;
    if (socket_ops::bind(acceptor.get(), (const socket_addr_type*)&addr,
          addr_len, ec) == socket_error_retval)
    {
      boost::system::system_error e(ec, "socket_select_interrupter");
      boost::throw_exception(e);
    }

    if (socket_ops::getsockname(acceptor.get(), (socket_addr_type*)&addr,
          &addr_len, ec) == socket_error_retval)
    {
      boost::system::system_error e(ec, "socket_select_interrupter");
      boost::throw_exception(e);
    }

    if (socket_ops::listen(acceptor.get(),
          SOMAXCONN, ec) == socket_error_retval)
    {
      boost::system::system_error e(ec, "socket_select_interrupter");
      boost::throw_exception(e);
    }

    socket_holder client(socket_ops::socket(
          AF_INET, SOCK_STREAM, IPPROTO_TCP, ec));
    if (client.get() == invalid_socket)
    {
      boost::system::system_error e(ec, "socket_select_interrupter");
      boost::throw_exception(e);
    }

    if (socket_ops::connect(client.get(), (const socket_addr_type*)&addr,
          addr_len, ec) == socket_error_retval)
    {
      boost::system::system_error e(ec, "socket_select_interrupter");
      boost::throw_exception(e);
    }

    socket_holder server(socket_ops::accept(acceptor.get(), 0, 0, ec));
    if (server.get() == invalid_socket)
    {
      boost::system::system_error e(ec, "socket_select_interrupter");
      boost::throw_exception(e);
    }
    
    ioctl_arg_type non_blocking = 1;
    if (socket_ops::ioctl(client.get(), FIONBIO, &non_blocking, ec))
    {
      boost::system::system_error e(ec, "socket_select_interrupter");
      boost::throw_exception(e);
    }

    opt = 1;
    socket_ops::setsockopt(client.get(),
        IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt), ec);

    non_blocking = 1;
    if (socket_ops::ioctl(server.get(), FIONBIO, &non_blocking, ec))
    {
      boost::system::system_error e(ec, "socket_select_interrupter");
      boost::throw_exception(e);
    }

    opt = 1;
    socket_ops::setsockopt(server.get(),
        IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt), ec);

    read_descriptor_ = server.release();
    write_descriptor_ = client.release();
  }

  // Destructor.
  ~socket_select_interrupter()
  {
    boost::system::error_code ec;
    if (read_descriptor_ != invalid_socket)
      socket_ops::close(read_descriptor_, ec);
    if (write_descriptor_ != invalid_socket)
      socket_ops::close(write_descriptor_, ec);
  }

  // Interrupt the select call.
  void interrupt()
  {
    char byte = 0;
    socket_ops::buf b;
    socket_ops::init_buf(b, &byte, 1);
    boost::system::error_code ec;
    socket_ops::send(write_descriptor_, &b, 1, 0, ec);
  }

  // Reset the select interrupt. Returns true if the call was interrupted.
  bool reset()
  {
    char data[1024];
    socket_ops::buf b;
    socket_ops::init_buf(b, data, sizeof(data));
    boost::system::error_code ec;
    int bytes_read = socket_ops::recv(read_descriptor_, &b, 1, 0, ec);
    bool was_interrupted = (bytes_read > 0);
    while (bytes_read == sizeof(data))
      bytes_read = socket_ops::recv(read_descriptor_, &b, 1, 0, ec);
    return was_interrupted;
  }

  // Get the read descriptor to be passed to select.
  socket_type read_descriptor() const
  {
    return read_descriptor_;
  }

private:
  // The read end of a connection used to interrupt the select call. This file
  // descriptor is passed to select such that when it is time to stop, a single
  // byte will be written on the other end of the connection and this
  // descriptor will become readable.
  socket_type read_descriptor_;

  // The write end of a connection used to interrupt the select call. A single
  // byte may be written to this to wake up the select which is waiting for the
  // other end to become readable.
  socket_type write_descriptor_;
};

} // namespace detail
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_SOCKET_SELECT_INTERRUPTER_HPP
