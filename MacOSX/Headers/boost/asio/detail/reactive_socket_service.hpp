//
// reactive_socket_service.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_REACTIVE_SOCKET_SERVICE_HPP
#define BOOST_ASIO_DETAIL_REACTIVE_SOCKET_SERVICE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/buffer.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/socket_base.hpp>
#include <boost/asio/detail/bind_handler.hpp>
#include <boost/asio/detail/handler_base_from_member.hpp>
#include <boost/asio/detail/noncopyable.hpp>
#include <boost/asio/detail/service_base.hpp>
#include <boost/asio/detail/socket_holder.hpp>
#include <boost/asio/detail/socket_ops.hpp>
#include <boost/asio/detail/socket_types.hpp>

namespace boost {
namespace asio {
namespace detail {

template <typename Protocol, typename Reactor>
class reactive_socket_service
  : public boost::asio::detail::service_base<
      reactive_socket_service<Protocol, Reactor> >
{
public:
  // The protocol type.
  typedef Protocol protocol_type;

  // The endpoint type.
  typedef typename Protocol::endpoint endpoint_type;

  // The native type of a socket.
  typedef socket_type native_type;

  // The implementation type of the socket.
  class implementation_type
    : private boost::asio::detail::noncopyable
  {
  public:
    // Default constructor.
    implementation_type()
      : socket_(invalid_socket),
        flags_(0),
        protocol_(endpoint_type().protocol())
    {
    }

  private:
    // Only this service will have access to the internal values.
    friend class reactive_socket_service<Protocol, Reactor>;

    // The native socket representation.
    socket_type socket_;

    enum
    {
      // The user wants a non-blocking socket.
      user_set_non_blocking = 1,

      // The implementation wants a non-blocking socket (in order to be able to
      // perform asynchronous read and write operations).
      internal_non_blocking = 2,

      // Helper "flag" used to determine whether the socket is non-blocking.
      non_blocking = user_set_non_blocking | internal_non_blocking,

      // User wants connection_aborted errors, which are disabled by default.
      enable_connection_aborted = 4,

      // The user set the linger option. Needs to be checked when closing. 
      user_set_linger = 8
    };

    // Flags indicating the current state of the socket.
    unsigned char flags_;

    // The protocol associated with the socket.
    protocol_type protocol_;

    // Per-descriptor data used by the reactor.
    typename Reactor::per_descriptor_data reactor_data_;
  };

  // The maximum number of buffers to support in a single operation.
  enum { max_buffers = 64 < max_iov_len ? 64 : max_iov_len };

  // Constructor.
  reactive_socket_service(boost::asio::io_service& io_service)
    : boost::asio::detail::service_base<
        reactive_socket_service<Protocol, Reactor> >(io_service),
      reactor_(boost::asio::use_service<Reactor>(io_service))
  {
    reactor_.init_task();
  }

  // Destroy all user-defined handler objects owned by the service.
  void shutdown_service()
  {
  }

  // Construct a new socket implementation.
  void construct(implementation_type& impl)
  {
    impl.socket_ = invalid_socket;
    impl.flags_ = 0;
  }

  // Destroy a socket implementation.
  void destroy(implementation_type& impl)
  {
    if (impl.socket_ != invalid_socket)
    {
      reactor_.close_descriptor(impl.socket_, impl.reactor_data_);

      if (impl.flags_ & implementation_type::non_blocking)
      {
        ioctl_arg_type non_blocking = 0;
        boost::system::error_code ignored_ec;
        socket_ops::ioctl(impl.socket_, FIONBIO, &non_blocking, ignored_ec);
        impl.flags_ &= ~implementation_type::non_blocking;
      }

      if (impl.flags_ & implementation_type::user_set_linger)
      {
        ::linger opt;
        opt.l_onoff = 0;
        opt.l_linger = 0;
        boost::system::error_code ignored_ec;
        socket_ops::setsockopt(impl.socket_,
            SOL_SOCKET, SO_LINGER, &opt, sizeof(opt), ignored_ec);
      }

      boost::system::error_code ignored_ec;
      socket_ops::close(impl.socket_, ignored_ec);

      impl.socket_ = invalid_socket;
    }
  }

  // Open a new socket implementation.
  boost::system::error_code open(implementation_type& impl,
      const protocol_type& protocol, boost::system::error_code& ec)
  {
    if (is_open(impl))
    {
      ec = boost::asio::error::already_open;
      return ec;
    }

    socket_holder sock(socket_ops::socket(protocol.family(),
          protocol.type(), protocol.protocol(), ec));
    if (sock.get() == invalid_socket)
      return ec;

    if (int err = reactor_.register_descriptor(sock.get(), impl.reactor_data_))
    {
      ec = boost::system::error_code(err,
          boost::asio::error::get_system_category());
      return ec;
    }

    impl.socket_ = sock.release();
    impl.flags_ = 0;
    impl.protocol_ = protocol;
    ec = boost::system::error_code();
    return ec;
  }

  // Assign a native socket to a socket implementation.
  boost::system::error_code assign(implementation_type& impl,
      const protocol_type& protocol, const native_type& native_socket,
      boost::system::error_code& ec)
  {
    if (is_open(impl))
    {
      ec = boost::asio::error::already_open;
      return ec;
    }

    if (int err = reactor_.register_descriptor(
          native_socket, impl.reactor_data_))
    {
      ec = boost::system::error_code(err,
          boost::asio::error::get_system_category());
      return ec;
    }

    impl.socket_ = native_socket;
    impl.flags_ = 0;
    impl.protocol_ = protocol;
    ec = boost::system::error_code();
    return ec;
  }

  // Determine whether the socket is open.
  bool is_open(const implementation_type& impl) const
  {
    return impl.socket_ != invalid_socket;
  }

  // Destroy a socket implementation.
  boost::system::error_code close(implementation_type& impl,
      boost::system::error_code& ec)
  {
    if (is_open(impl))
    {
      reactor_.close_descriptor(impl.socket_, impl.reactor_data_);

      if (impl.flags_ & implementation_type::non_blocking)
      {
        ioctl_arg_type non_blocking = 0;
        boost::system::error_code ignored_ec;
        socket_ops::ioctl(impl.socket_, FIONBIO, &non_blocking, ignored_ec);
        impl.flags_ &= ~implementation_type::non_blocking;
      }

      if (socket_ops::close(impl.socket_, ec) == socket_error_retval)
        return ec;

      impl.socket_ = invalid_socket;
    }

    ec = boost::system::error_code();
    return ec;
  }

  // Get the native socket representation.
  native_type native(implementation_type& impl)
  {
    return impl.socket_;
  }

  // Cancel all operations associated with the socket.
  boost::system::error_code cancel(implementation_type& impl,
      boost::system::error_code& ec)
  {
    if (!is_open(impl))
    {
      ec = boost::asio::error::bad_descriptor;
      return ec;
    }

    reactor_.cancel_ops(impl.socket_, impl.reactor_data_);
    ec = boost::system::error_code();
    return ec;
  }

  // Determine whether the socket is at the out-of-band data mark.
  bool at_mark(const implementation_type& impl,
      boost::system::error_code& ec) const
  {
    if (!is_open(impl))
    {
      ec = boost::asio::error::bad_descriptor;
      return false;
    }

    boost::asio::detail::ioctl_arg_type value = 0;
    socket_ops::ioctl(impl.socket_, SIOCATMARK, &value, ec);
#if defined(ENOTTY)
    if (ec.value() == ENOTTY)
      ec = boost::asio::error::not_socket;
#endif // defined(ENOTTY)
    return ec ? false : value != 0;
  }

  // Determine the number of bytes available for reading.
  std::size_t available(const implementation_type& impl,
      boost::system::error_code& ec) const
  {
    if (!is_open(impl))
    {
      ec = boost::asio::error::bad_descriptor;
      return 0;
    }

    boost::asio::detail::ioctl_arg_type value = 0;
    socket_ops::ioctl(impl.socket_, FIONREAD, &value, ec);
#if defined(ENOTTY)
    if (ec.value() == ENOTTY)
      ec = boost::asio::error::not_socket;
#endif // defined(ENOTTY)
    return ec ? static_cast<std::size_t>(0) : static_cast<std::size_t>(value);
  }

  // Bind the socket to the specified local endpoint.
  boost::system::error_code bind(implementation_type& impl,
      const endpoint_type& endpoint, boost::system::error_code& ec)
  {
    if (!is_open(impl))
    {
      ec = boost::asio::error::bad_descriptor;
      return ec;
    }

    socket_ops::bind(impl.socket_, endpoint.data(), endpoint.size(), ec);
    return ec;
  }

  // Place the socket into the state where it will listen for new connections.
  boost::system::error_code listen(implementation_type& impl, int backlog,
      boost::system::error_code& ec)
  {
    if (!is_open(impl))
    {
      ec = boost::asio::error::bad_descriptor;
      return ec;
    }

    socket_ops::listen(impl.socket_, backlog, ec);
    return ec;
  }

  // Set a socket option.
  template <typename Option>
  boost::system::error_code set_option(implementation_type& impl,
      const Option& option, boost::system::error_code& ec)
  {
    if (!is_open(impl))
    {
      ec = boost::asio::error::bad_descriptor;
      return ec;
    }

    if (option.level(impl.protocol_) == custom_socket_option_level
        && option.name(impl.protocol_) == enable_connection_aborted_option)
    {
      if (option.size(impl.protocol_) != sizeof(int))
      {
        ec = boost::asio::error::invalid_argument;
      }
      else
      {
        if (*reinterpret_cast<const int*>(option.data(impl.protocol_)))
          impl.flags_ |= implementation_type::enable_connection_aborted;
        else
          impl.flags_ &= ~implementation_type::enable_connection_aborted;
        ec = boost::system::error_code();
      }
      return ec;
    }
    else
    {
      if (option.level(impl.protocol_) == SOL_SOCKET
          && option.name(impl.protocol_) == SO_LINGER)
      {
        impl.flags_ |= implementation_type::user_set_linger;
      }

      socket_ops::setsockopt(impl.socket_,
          option.level(impl.protocol_), option.name(impl.protocol_),
          option.data(impl.protocol_), option.size(impl.protocol_), ec);

#if defined(__MACH__) && defined(__APPLE__) \
|| defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
      // To implement portable behaviour for SO_REUSEADDR with UDP sockets we
      // need to also set SO_REUSEPORT on BSD-based platforms.
      if (!ec && impl.protocol_.type() == SOCK_DGRAM
          && option.level(impl.protocol_) == SOL_SOCKET
          && option.name(impl.protocol_) == SO_REUSEADDR)
      {
        boost::system::error_code ignored_ec;
        socket_ops::setsockopt(impl.socket_, SOL_SOCKET, SO_REUSEPORT,
            option.data(impl.protocol_), option.size(impl.protocol_),
            ignored_ec);
      }
#endif

      return ec;
    }
  }

  // Set a socket option.
  template <typename Option>
  boost::system::error_code get_option(const implementation_type& impl,
      Option& option, boost::system::error_code& ec) const
  {
    if (!is_open(impl))
    {
      ec = boost::asio::error::bad_descriptor;
      return ec;
    }

    if (option.level(impl.protocol_) == custom_socket_option_level
        && option.name(impl.protocol_) == enable_connection_aborted_option)
    {
      if (option.size(impl.protocol_) != sizeof(int))
      {
        ec = boost::asio::error::invalid_argument;
      }
      else
      {
        int* target = reinterpret_cast<int*>(option.data(impl.protocol_));
        if (impl.flags_ & implementation_type::enable_connection_aborted)
          *target = 1;
        else
          *target = 0;
        option.resize(impl.protocol_, sizeof(int));
        ec = boost::system::error_code();
      }
      return ec;
    }
    else
    {
      size_t size = option.size(impl.protocol_);
      socket_ops::getsockopt(impl.socket_,
          option.level(impl.protocol_), option.name(impl.protocol_),
          option.data(impl.protocol_), &size, ec);
      if (!ec)
        option.resize(impl.protocol_, size);
      return ec;
    }
  }

  // Perform an IO control command on the socket.
  template <typename IO_Control_Command>
  boost::system::error_code io_control(implementation_type& impl,
      IO_Control_Command& command, boost::system::error_code& ec)
  {
    if (!is_open(impl))
    {
      ec = boost::asio::error::bad_descriptor;
      return ec;
    }

    if (command.name() == static_cast<int>(FIONBIO))
    {
      // Flags are manipulated in a temporary variable so that the socket
      // implementation is not updated unless the ioctl operation succeeds.
      unsigned char new_flags = impl.flags_;
      if (command.get())
        new_flags |= implementation_type::user_set_non_blocking;
      else
        new_flags &= ~implementation_type::user_set_non_blocking;

      // Perform ioctl on socket if the non-blocking state has changed.
      if (!(impl.flags_ & implementation_type::non_blocking)
          && (new_flags & implementation_type::non_blocking))
      {
        ioctl_arg_type non_blocking = 1;
        socket_ops::ioctl(impl.socket_, FIONBIO, &non_blocking, ec);
      }
      else if ((impl.flags_ & implementation_type::non_blocking)
          && !(new_flags & implementation_type::non_blocking))
      {
        ioctl_arg_type non_blocking = 0;
        socket_ops::ioctl(impl.socket_, FIONBIO, &non_blocking, ec);
      }
      else
      {
        ec = boost::system::error_code();
      }

      // Update socket implementation's flags only if successful.
      if (!ec)
        impl.flags_ = new_flags;
    }
    else
    {
      socket_ops::ioctl(impl.socket_, command.name(),
          static_cast<ioctl_arg_type*>(command.data()), ec);
    }
    return ec;
  }

  // Get the local endpoint.
  endpoint_type local_endpoint(const implementation_type& impl,
      boost::system::error_code& ec) const
  {
    if (!is_open(impl))
    {
      ec = boost::asio::error::bad_descriptor;
      return endpoint_type();
    }

    endpoint_type endpoint;
    std::size_t addr_len = endpoint.capacity();
    if (socket_ops::getsockname(impl.socket_, endpoint.data(), &addr_len, ec))
      return endpoint_type();
    endpoint.resize(addr_len);
    return endpoint;
  }

  // Get the remote endpoint.
  endpoint_type remote_endpoint(const implementation_type& impl,
      boost::system::error_code& ec) const
  {
    if (!is_open(impl))
    {
      ec = boost::asio::error::bad_descriptor;
      return endpoint_type();
    }

    endpoint_type endpoint;
    std::size_t addr_len = endpoint.capacity();
    if (socket_ops::getpeername(impl.socket_, endpoint.data(), &addr_len, ec))
      return endpoint_type();
    endpoint.resize(addr_len);
    return endpoint;
  }

  /// Disable sends or receives on the socket.
  boost::system::error_code shutdown(implementation_type& impl,
      socket_base::shutdown_type what, boost::system::error_code& ec)
  {
    if (!is_open(impl))
    {
      ec = boost::asio::error::bad_descriptor;
      return ec;
    }

    socket_ops::shutdown(impl.socket_, what, ec);
    return ec;
  }

  // Send the given data to the peer.
  template <typename ConstBufferSequence>
  size_t send(implementation_type& impl, const ConstBufferSequence& buffers,
      socket_base::message_flags flags, boost::system::error_code& ec)
  {
    if (!is_open(impl))
    {
      ec = boost::asio::error::bad_descriptor;
      return 0;
    }

    // Copy buffers into array.
    socket_ops::buf bufs[max_buffers];
    typename ConstBufferSequence::const_iterator iter = buffers.begin();
    typename ConstBufferSequence::const_iterator end = buffers.end();
    size_t i = 0;
    size_t total_buffer_size = 0;
    for (; iter != end && i < max_buffers; ++iter, ++i)
    {
      boost::asio::const_buffer buffer(*iter);
      socket_ops::init_buf(bufs[i],
          boost::asio::buffer_cast<const void*>(buffer),
          boost::asio::buffer_size(buffer));
      total_buffer_size += boost::asio::buffer_size(buffer);
    }

    // A request to receive 0 bytes on a stream socket is a no-op.
    if (impl.protocol_.type() == SOCK_STREAM && total_buffer_size == 0)
    {
      ec = boost::system::error_code();
      return 0;
    }

    // Send the data.
    for (;;)
    {
      // Try to complete the operation without blocking.
      int bytes_sent = socket_ops::send(impl.socket_, bufs, i, flags, ec);

      // Check if operation succeeded.
      if (bytes_sent >= 0)
        return bytes_sent;

      // Operation failed.
      if ((impl.flags_ & implementation_type::user_set_non_blocking)
          || (ec != boost::asio::error::would_block
            && ec != boost::asio::error::try_again))
        return 0;

      // Wait for socket to become ready.
      if (socket_ops::poll_write(impl.socket_, ec) < 0)
        return 0;
    }
  }

  // Wait until data can be sent without blocking.
  size_t send(implementation_type& impl, const null_buffers&,
      socket_base::message_flags, boost::system::error_code& ec)
  {
    if (!is_open(impl))
    {
      ec = boost::asio::error::bad_descriptor;
      return 0;
    }

    // Wait for socket to become ready.
    socket_ops::poll_write(impl.socket_, ec);

    return 0;
  }

  template <typename ConstBufferSequence, typename Handler>
  class send_operation :
    public handler_base_from_member<Handler>
  {
  public:
    send_operation(socket_type socket, boost::asio::io_service& io_service,
        const ConstBufferSequence& buffers, socket_base::message_flags flags,
        Handler handler)
      : handler_base_from_member<Handler>(handler),
        socket_(socket),
        io_service_(io_service),
        work_(io_service),
        buffers_(buffers),
        flags_(flags)
    {
    }

    bool perform(boost::system::error_code& ec,
        std::size_t& bytes_transferred)
    {
      // Check whether the operation was successful.
      if (ec)
      {
        bytes_transferred = 0;
        return true;
      }

      // Copy buffers into array.
      socket_ops::buf bufs[max_buffers];
      typename ConstBufferSequence::const_iterator iter = buffers_.begin();
      typename ConstBufferSequence::const_iterator end = buffers_.end();
      size_t i = 0;
      for (; iter != end && i < max_buffers; ++iter, ++i)
      {
        boost::asio::const_buffer buffer(*iter);
        socket_ops::init_buf(bufs[i],
            boost::asio::buffer_cast<const void*>(buffer),
            boost::asio::buffer_size(buffer));
      }

      // Send the data.
      int bytes = socket_ops::send(socket_, bufs, i, flags_, ec);

      // Check if we need to run the operation again.
      if (ec == boost::asio::error::would_block
          || ec == boost::asio::error::try_again)
        return false;

      bytes_transferred = (bytes < 0 ? 0 : bytes);
      return true;
    }

    void complete(const boost::system::error_code& ec,
        std::size_t bytes_transferred)
    {
      io_service_.post(bind_handler(this->handler_, ec, bytes_transferred));
    }

  private:
    socket_type socket_;
    boost::asio::io_service& io_service_;
    boost::asio::io_service::work work_;
    ConstBufferSequence buffers_;
    socket_base::message_flags flags_;
  };

  // Start an asynchronous send. The data being sent must be valid for the
  // lifetime of the asynchronous operation.
  template <typename ConstBufferSequence, typename Handler>
  void async_send(implementation_type& impl, const ConstBufferSequence& buffers,
      socket_base::message_flags flags, Handler handler)
  {
    if (!is_open(impl))
    {
      this->get_io_service().post(bind_handler(handler,
            boost::asio::error::bad_descriptor, 0));
    }
    else
    {
      if (impl.protocol_.type() == SOCK_STREAM)
      {
        // Determine total size of buffers.
        typename ConstBufferSequence::const_iterator iter = buffers.begin();
        typename ConstBufferSequence::const_iterator end = buffers.end();
        size_t i = 0;
        size_t total_buffer_size = 0;
        for (; iter != end && i < max_buffers; ++iter, ++i)
        {
          boost::asio::const_buffer buffer(*iter);
          total_buffer_size += boost::asio::buffer_size(buffer);
        }

        // A request to receive 0 bytes on a stream socket is a no-op.
        if (total_buffer_size == 0)
        {
          this->get_io_service().post(bind_handler(handler,
                boost::system::error_code(), 0));
          return;
        }
      }

      // Make socket non-blocking.
      if (!(impl.flags_ & implementation_type::internal_non_blocking))
      {
        if (!(impl.flags_ & implementation_type::non_blocking))
        {
          ioctl_arg_type non_blocking = 1;
          boost::system::error_code ec;
          if (socket_ops::ioctl(impl.socket_, FIONBIO, &non_blocking, ec))
          {
            this->get_io_service().post(bind_handler(handler, ec, 0));
            return;
          }
        }
        impl.flags_ |= implementation_type::internal_non_blocking;
      }

      reactor_.start_write_op(impl.socket_, impl.reactor_data_,
          send_operation<ConstBufferSequence, Handler>(
            impl.socket_, this->get_io_service(), buffers, flags, handler));
    }
  }

  template <typename Handler>
  class null_buffers_operation :
    public handler_base_from_member<Handler>
  {
  public:
    null_buffers_operation(boost::asio::io_service& io_service, Handler handler)
      : handler_base_from_member<Handler>(handler),
        work_(io_service)
    {
    }

    bool perform(boost::system::error_code&,
        std::size_t& bytes_transferred)
    {
      bytes_transferred = 0;
      return true;
    }

    void complete(const boost::system::error_code& ec,
        std::size_t bytes_transferred)
    {
      work_.get_io_service().post(bind_handler(
            this->handler_, ec, bytes_transferred));
    }

  private:
    boost::asio::io_service::work work_;
  };

  // Start an asynchronous wait until data can be sent without blocking.
  template <typename Handler>
  void async_send(implementation_type& impl, const null_buffers&,
      socket_base::message_flags, Handler handler)
  {
    if (!is_open(impl))
    {
      this->get_io_service().post(bind_handler(handler,
            boost::asio::error::bad_descriptor, 0));
    }
    else
    {
      reactor_.start_write_op(impl.socket_, impl.reactor_data_,
          null_buffers_operation<Handler>(this->get_io_service(), handler),
          false);
    }
  }

  // Send a datagram to the specified endpoint. Returns the number of bytes
  // sent.
  template <typename ConstBufferSequence>
  size_t send_to(implementation_type& impl, const ConstBufferSequence& buffers,
      const endpoint_type& destination, socket_base::message_flags flags,
      boost::system::error_code& ec)
  {
    if (!is_open(impl))
    {
      ec = boost::asio::error::bad_descriptor;
      return 0;
    }

    // Copy buffers into array.
    socket_ops::buf bufs[max_buffers];
    typename ConstBufferSequence::const_iterator iter = buffers.begin();
    typename ConstBufferSequence::const_iterator end = buffers.end();
    size_t i = 0;
    for (; iter != end && i < max_buffers; ++iter, ++i)
    {
      boost::asio::const_buffer buffer(*iter);
      socket_ops::init_buf(bufs[i],
          boost::asio::buffer_cast<const void*>(buffer),
          boost::asio::buffer_size(buffer));
    }

    // Send the data.
    for (;;)
    {
      // Try to complete the operation without blocking.
      int bytes_sent = socket_ops::sendto(impl.socket_, bufs, i, flags,
          destination.data(), destination.size(), ec);

      // Check if operation succeeded.
      if (bytes_sent >= 0)
        return bytes_sent;

      // Operation failed.
      if ((impl.flags_ & implementation_type::user_set_non_blocking)
          || (ec != boost::asio::error::would_block
            && ec != boost::asio::error::try_again))
        return 0;

      // Wait for socket to become ready.
      if (socket_ops::poll_write(impl.socket_, ec) < 0)
        return 0;
    }
  }

  // Wait until data can be sent without blocking.
  size_t send_to(implementation_type& impl, const null_buffers&,
      socket_base::message_flags, const endpoint_type&,
      boost::system::error_code& ec)
  {
    if (!is_open(impl))
    {
      ec = boost::asio::error::bad_descriptor;
      return 0;
    }

    // Wait for socket to become ready.
    socket_ops::poll_write(impl.socket_, ec);

    return 0;
  }

  template <typename ConstBufferSequence, typename Handler>
  class send_to_operation :
    public handler_base_from_member<Handler>
  {
  public:
    send_to_operation(socket_type socket, boost::asio::io_service& io_service,
        const ConstBufferSequence& buffers, const endpoint_type& endpoint,
        socket_base::message_flags flags, Handler handler)
      : handler_base_from_member<Handler>(handler),
        socket_(socket),
        io_service_(io_service),
        work_(io_service),
        buffers_(buffers),
        destination_(endpoint),
        flags_(flags)
    {
    }

    bool perform(boost::system::error_code& ec,
        std::size_t& bytes_transferred)
    {
      // Check whether the operation was successful.
      if (ec)
      {
        bytes_transferred = 0;
        return true;
      }

      // Copy buffers into array.
      socket_ops::buf bufs[max_buffers];
      typename ConstBufferSequence::const_iterator iter = buffers_.begin();
      typename ConstBufferSequence::const_iterator end = buffers_.end();
      size_t i = 0;
      for (; iter != end && i < max_buffers; ++iter, ++i)
      {
        boost::asio::const_buffer buffer(*iter);
        socket_ops::init_buf(bufs[i],
            boost::asio::buffer_cast<const void*>(buffer),
            boost::asio::buffer_size(buffer));
      }

      // Send the data.
      int bytes = socket_ops::sendto(socket_, bufs, i, flags_,
          destination_.data(), destination_.size(), ec);

      // Check if we need to run the operation again.
      if (ec == boost::asio::error::would_block
          || ec == boost::asio::error::try_again)
        return false;

      bytes_transferred = (bytes < 0 ? 0 : bytes);
      return true;
    }

    void complete(const boost::system::error_code& ec,
        std::size_t bytes_transferred)
    {
      io_service_.post(bind_handler(this->handler_, ec, bytes_transferred));
    }

  private:
    socket_type socket_;
    boost::asio::io_service& io_service_;
    boost::asio::io_service::work work_;
    ConstBufferSequence buffers_;
    endpoint_type destination_;
    socket_base::message_flags flags_;
  };

  // Start an asynchronous send. The data being sent must be valid for the
  // lifetime of the asynchronous operation.
  template <typename ConstBufferSequence, typename Handler>
  void async_send_to(implementation_type& impl,
      const ConstBufferSequence& buffers,
      const endpoint_type& destination, socket_base::message_flags flags,
      Handler handler)
  {
    if (!is_open(impl))
    {
      this->get_io_service().post(bind_handler(handler,
            boost::asio::error::bad_descriptor, 0));
    }
    else
    {
      // Make socket non-blocking.
      if (!(impl.flags_ & implementation_type::internal_non_blocking))
      {
        if (!(impl.flags_ & implementation_type::non_blocking))
        {
          ioctl_arg_type non_blocking = 1;
          boost::system::error_code ec;
          if (socket_ops::ioctl(impl.socket_, FIONBIO, &non_blocking, ec))
          {
            this->get_io_service().post(bind_handler(handler, ec, 0));
            return;
          }
        }
        impl.flags_ |= implementation_type::internal_non_blocking;
      }

      reactor_.start_write_op(impl.socket_, impl.reactor_data_,
          send_to_operation<ConstBufferSequence, Handler>(
            impl.socket_, this->get_io_service(), buffers,
            destination, flags, handler));
    }
  }

  // Start an asynchronous wait until data can be sent without blocking.
  template <typename Handler>
  void async_send_to(implementation_type& impl, const null_buffers&,
      socket_base::message_flags, const endpoint_type&, Handler handler)
  {
    if (!is_open(impl))
    {
      this->get_io_service().post(bind_handler(handler,
            boost::asio::error::bad_descriptor, 0));
    }
    else
    {
      reactor_.start_write_op(impl.socket_, impl.reactor_data_,
          null_buffers_operation<Handler>(this->get_io_service(), handler),
          false);
    }
  }

  // Receive some data from the peer. Returns the number of bytes received.
  template <typename MutableBufferSequence>
  size_t receive(implementation_type& impl,
      const MutableBufferSequence& buffers,
      socket_base::message_flags flags, boost::system::error_code& ec)
  {
    if (!is_open(impl))
    {
      ec = boost::asio::error::bad_descriptor;
      return 0;
    }

    // Copy buffers into array.
    socket_ops::buf bufs[max_buffers];
    typename MutableBufferSequence::const_iterator iter = buffers.begin();
    typename MutableBufferSequence::const_iterator end = buffers.end();
    size_t i = 0;
    size_t total_buffer_size = 0;
    for (; iter != end && i < max_buffers; ++iter, ++i)
    {
      boost::asio::mutable_buffer buffer(*iter);
      socket_ops::init_buf(bufs[i],
          boost::asio::buffer_cast<void*>(buffer),
          boost::asio::buffer_size(buffer));
      total_buffer_size += boost::asio::buffer_size(buffer);
    }

    // A request to receive 0 bytes on a stream socket is a no-op.
    if (impl.protocol_.type() == SOCK_STREAM && total_buffer_size == 0)
    {
      ec = boost::system::error_code();
      return 0;
    }

    // Receive some data.
    for (;;)
    {
      // Try to complete the operation without blocking.
      int bytes_recvd = socket_ops::recv(impl.socket_, bufs, i, flags, ec);

      // Check if operation succeeded.
      if (bytes_recvd > 0)
        return bytes_recvd;

      // Check for EOF.
      if (bytes_recvd == 0 && impl.protocol_.type() == SOCK_STREAM)
      {
        ec = boost::asio::error::eof;
        return 0;
      }

      // Operation failed.
      if ((impl.flags_ & implementation_type::user_set_non_blocking)
          || (ec != boost::asio::error::would_block
            && ec != boost::asio::error::try_again))
        return 0;

      // Wait for socket to become ready.
      if (socket_ops::poll_read(impl.socket_, ec) < 0)
        return 0;
    }
  }

  // Wait until data can be received without blocking.
  size_t receive(implementation_type& impl, const null_buffers&,
      socket_base::message_flags, boost::system::error_code& ec)
  {
    if (!is_open(impl))
    {
      ec = boost::asio::error::bad_descriptor;
      return 0;
    }

    // Wait for socket to become ready.
    socket_ops::poll_read(impl.socket_, ec);

    return 0;
  }

  template <typename MutableBufferSequence, typename Handler>
  class receive_operation :
    public handler_base_from_member<Handler>
  {
  public:
    receive_operation(socket_type socket, int protocol_type,
        boost::asio::io_service& io_service,
        const MutableBufferSequence& buffers,
        socket_base::message_flags flags, Handler handler)
      : handler_base_from_member<Handler>(handler),
        socket_(socket),
        protocol_type_(protocol_type),
        io_service_(io_service),
        work_(io_service),
        buffers_(buffers),
        flags_(flags)
    {
    }

    bool perform(boost::system::error_code& ec,
        std::size_t& bytes_transferred)
    {
      // Check whether the operation was successful.
      if (ec)
      {
        bytes_transferred = 0;
        return true;
      }

      // Copy buffers into array.
      socket_ops::buf bufs[max_buffers];
      typename MutableBufferSequence::const_iterator iter = buffers_.begin();
      typename MutableBufferSequence::const_iterator end = buffers_.end();
      size_t i = 0;
      for (; iter != end && i < max_buffers; ++iter, ++i)
      {
        boost::asio::mutable_buffer buffer(*iter);
        socket_ops::init_buf(bufs[i],
            boost::asio::buffer_cast<void*>(buffer),
            boost::asio::buffer_size(buffer));
      }

      // Receive some data.
      int bytes = socket_ops::recv(socket_, bufs, i, flags_, ec);
      if (bytes == 0 && protocol_type_ == SOCK_STREAM)
        ec = boost::asio::error::eof;

      // Check if we need to run the operation again.
      if (ec == boost::asio::error::would_block
          || ec == boost::asio::error::try_again)
        return false;

      bytes_transferred = (bytes < 0 ? 0 : bytes);
      return true;
    }

    void complete(const boost::system::error_code& ec,
        std::size_t bytes_transferred)
    {
      io_service_.post(bind_handler(this->handler_, ec, bytes_transferred));
    }

  private:
    socket_type socket_;
    int protocol_type_;
    boost::asio::io_service& io_service_;
    boost::asio::io_service::work work_;
    MutableBufferSequence buffers_;
    socket_base::message_flags flags_;
  };

  // Start an asynchronous receive. The buffer for the data being received
  // must be valid for the lifetime of the asynchronous operation.
  template <typename MutableBufferSequence, typename Handler>
  void async_receive(implementation_type& impl,
      const MutableBufferSequence& buffers,
      socket_base::message_flags flags, Handler handler)
  {
    if (!is_open(impl))
    {
      this->get_io_service().post(bind_handler(handler,
            boost::asio::error::bad_descriptor, 0));
    }
    else
    {
      if (impl.protocol_.type() == SOCK_STREAM)
      {
        // Determine total size of buffers.
        typename MutableBufferSequence::const_iterator iter = buffers.begin();
        typename MutableBufferSequence::const_iterator end = buffers.end();
        size_t i = 0;
        size_t total_buffer_size = 0;
        for (; iter != end && i < max_buffers; ++iter, ++i)
        {
          boost::asio::mutable_buffer buffer(*iter);
          total_buffer_size += boost::asio::buffer_size(buffer);
        }

        // A request to receive 0 bytes on a stream socket is a no-op.
        if (total_buffer_size == 0)
        {
          this->get_io_service().post(bind_handler(handler,
                boost::system::error_code(), 0));
          return;
        }
      }

      // Make socket non-blocking.
      if (!(impl.flags_ & implementation_type::internal_non_blocking))
      {
        if (!(impl.flags_ & implementation_type::non_blocking))
        {
          ioctl_arg_type non_blocking = 1;
          boost::system::error_code ec;
          if (socket_ops::ioctl(impl.socket_, FIONBIO, &non_blocking, ec))
          {
            this->get_io_service().post(bind_handler(handler, ec, 0));
            return;
          }
        }
        impl.flags_ |= implementation_type::internal_non_blocking;
      }

      if (flags & socket_base::message_out_of_band)
      {
        reactor_.start_except_op(impl.socket_, impl.reactor_data_,
            receive_operation<MutableBufferSequence, Handler>(
              impl.socket_, impl.protocol_.type(),
              this->get_io_service(), buffers, flags, handler));
      }
      else
      {
        reactor_.start_read_op(impl.socket_, impl.reactor_data_,
            receive_operation<MutableBufferSequence, Handler>(
              impl.socket_, impl.protocol_.type(),
              this->get_io_service(), buffers, flags, handler));
      }
    }
  }

  // Wait until data can be received without blocking.
  template <typename Handler>
  void async_receive(implementation_type& impl, const null_buffers&,
      socket_base::message_flags flags, Handler handler)
  {
    if (!is_open(impl))
    {
      this->get_io_service().post(bind_handler(handler,
            boost::asio::error::bad_descriptor, 0));
    }
    else if (flags & socket_base::message_out_of_band)
    {
      reactor_.start_except_op(impl.socket_, impl.reactor_data_,
          null_buffers_operation<Handler>(this->get_io_service(), handler));
    }
    else
    {
      reactor_.start_read_op(impl.socket_, impl.reactor_data_,
          null_buffers_operation<Handler>(this->get_io_service(), handler),
          false);
    }
  }

  // Receive a datagram with the endpoint of the sender. Returns the number of
  // bytes received.
  template <typename MutableBufferSequence>
  size_t receive_from(implementation_type& impl,
      const MutableBufferSequence& buffers,
      endpoint_type& sender_endpoint, socket_base::message_flags flags,
      boost::system::error_code& ec)
  {
    if (!is_open(impl))
    {
      ec = boost::asio::error::bad_descriptor;
      return 0;
    }

    // Copy buffers into array.
    socket_ops::buf bufs[max_buffers];
    typename MutableBufferSequence::const_iterator iter = buffers.begin();
    typename MutableBufferSequence::const_iterator end = buffers.end();
    size_t i = 0;
    for (; iter != end && i < max_buffers; ++iter, ++i)
    {
      boost::asio::mutable_buffer buffer(*iter);
      socket_ops::init_buf(bufs[i],
          boost::asio::buffer_cast<void*>(buffer),
          boost::asio::buffer_size(buffer));
    }

    // Receive some data.
    for (;;)
    {
      // Try to complete the operation without blocking.
      std::size_t addr_len = sender_endpoint.capacity();
      int bytes_recvd = socket_ops::recvfrom(impl.socket_, bufs, i, flags,
          sender_endpoint.data(), &addr_len, ec);

      // Check if operation succeeded.
      if (bytes_recvd > 0)
      {
        sender_endpoint.resize(addr_len);
        return bytes_recvd;
      }

      // Check for EOF.
      if (bytes_recvd == 0 && impl.protocol_.type() == SOCK_STREAM)
      {
        ec = boost::asio::error::eof;
        return 0;
      }

      // Operation failed.
      if ((impl.flags_ & implementation_type::user_set_non_blocking)
          || (ec != boost::asio::error::would_block
            && ec != boost::asio::error::try_again))
        return 0;

      // Wait for socket to become ready.
      if (socket_ops::poll_read(impl.socket_, ec) < 0)
        return 0;
    }
  }

  // Wait until data can be received without blocking.
  size_t receive_from(implementation_type& impl, const null_buffers&,
      endpoint_type& sender_endpoint, socket_base::message_flags,
      boost::system::error_code& ec)
  {
    if (!is_open(impl))
    {
      ec = boost::asio::error::bad_descriptor;
      return 0;
    }

    // Wait for socket to become ready.
    socket_ops::poll_read(impl.socket_, ec);

    // Reset endpoint since it can be given no sensible value at this time.
    sender_endpoint = endpoint_type();

    return 0;
  }

  template <typename MutableBufferSequence, typename Handler>
  class receive_from_operation :
    public handler_base_from_member<Handler>
  {
  public:
    receive_from_operation(socket_type socket, int protocol_type,
        boost::asio::io_service& io_service,
        const MutableBufferSequence& buffers, endpoint_type& endpoint,
        socket_base::message_flags flags, Handler handler)
      : handler_base_from_member<Handler>(handler),
        socket_(socket),
        protocol_type_(protocol_type),
        io_service_(io_service),
        work_(io_service),
        buffers_(buffers),
        sender_endpoint_(endpoint),
        flags_(flags)
    {
    }

    bool perform(boost::system::error_code& ec,
        std::size_t& bytes_transferred)
    {
      // Check whether the operation was successful.
      if (ec)
      {
        bytes_transferred = 0;
        return true;
      }

      // Copy buffers into array.
      socket_ops::buf bufs[max_buffers];
      typename MutableBufferSequence::const_iterator iter = buffers_.begin();
      typename MutableBufferSequence::const_iterator end = buffers_.end();
      size_t i = 0;
      for (; iter != end && i < max_buffers; ++iter, ++i)
      {
        boost::asio::mutable_buffer buffer(*iter);
        socket_ops::init_buf(bufs[i],
            boost::asio::buffer_cast<void*>(buffer),
            boost::asio::buffer_size(buffer));
      }

      // Receive some data.
      std::size_t addr_len = sender_endpoint_.capacity();
      int bytes = socket_ops::recvfrom(socket_, bufs, i, flags_,
          sender_endpoint_.data(), &addr_len, ec);
      if (bytes == 0 && protocol_type_ == SOCK_STREAM)
        ec = boost::asio::error::eof;

      // Check if we need to run the operation again.
      if (ec == boost::asio::error::would_block
          || ec == boost::asio::error::try_again)
        return false;

      sender_endpoint_.resize(addr_len);
      bytes_transferred = (bytes < 0 ? 0 : bytes);
      return true;
    }

    void complete(const boost::system::error_code& ec,
        std::size_t bytes_transferred)
    {
      io_service_.post(bind_handler(this->handler_, ec, bytes_transferred));
    }

  private:
    socket_type socket_;
    int protocol_type_;
    boost::asio::io_service& io_service_;
    boost::asio::io_service::work work_;
    MutableBufferSequence buffers_;
    endpoint_type& sender_endpoint_;
    socket_base::message_flags flags_;
  };

  // Start an asynchronous receive. The buffer for the data being received and
  // the sender_endpoint object must both be valid for the lifetime of the
  // asynchronous operation.
  template <typename MutableBufferSequence, typename Handler>
  void async_receive_from(implementation_type& impl,
      const MutableBufferSequence& buffers, endpoint_type& sender_endpoint,
      socket_base::message_flags flags, Handler handler)
  {
    if (!is_open(impl))
    {
      this->get_io_service().post(bind_handler(handler,
            boost::asio::error::bad_descriptor, 0));
    }
    else
    {
      // Make socket non-blocking.
      if (!(impl.flags_ & implementation_type::internal_non_blocking))
      {
        if (!(impl.flags_ & implementation_type::non_blocking))
        {
          ioctl_arg_type non_blocking = 1;
          boost::system::error_code ec;
          if (socket_ops::ioctl(impl.socket_, FIONBIO, &non_blocking, ec))
          {
            this->get_io_service().post(bind_handler(handler, ec, 0));
            return;
          }
        }
        impl.flags_ |= implementation_type::internal_non_blocking;
      }

      reactor_.start_read_op(impl.socket_, impl.reactor_data_,
          receive_from_operation<MutableBufferSequence, Handler>(
            impl.socket_, impl.protocol_.type(), this->get_io_service(),
            buffers, sender_endpoint, flags, handler));
    }
  }

  // Wait until data can be received without blocking.
  template <typename Handler>
  void async_receive_from(implementation_type& impl,
      const null_buffers&, endpoint_type& sender_endpoint,
      socket_base::message_flags flags, Handler handler)
  {
    if (!is_open(impl))
    {
      this->get_io_service().post(bind_handler(handler,
            boost::asio::error::bad_descriptor, 0));
    }
    else
    {
      // Reset endpoint since it can be given no sensible value at this time.
      sender_endpoint = endpoint_type();

      if (flags & socket_base::message_out_of_band)
      {
        reactor_.start_except_op(impl.socket_, impl.reactor_data_,
            null_buffers_operation<Handler>(this->get_io_service(), handler));
      }
      else
      {
        reactor_.start_read_op(impl.socket_, impl.reactor_data_,
            null_buffers_operation<Handler>(this->get_io_service(), handler),
            false);
      }
    }
  }

  // Accept a new connection.
  template <typename Socket>
  boost::system::error_code accept(implementation_type& impl,
      Socket& peer, endpoint_type* peer_endpoint, boost::system::error_code& ec)
  {
    if (!is_open(impl))
    {
      ec = boost::asio::error::bad_descriptor;
      return ec;
    }

    // We cannot accept a socket that is already open.
    if (peer.is_open())
    {
      ec = boost::asio::error::already_open;
      return ec;
    }

    // Accept a socket.
    for (;;)
    {
      // Try to complete the operation without blocking.
      boost::system::error_code ec;
      socket_holder new_socket;
      std::size_t addr_len = 0;
      if (peer_endpoint)
      {
        addr_len = peer_endpoint->capacity();
        new_socket.reset(socket_ops::accept(impl.socket_,
              peer_endpoint->data(), &addr_len, ec));
      }
      else
      {
        new_socket.reset(socket_ops::accept(impl.socket_, 0, 0, ec));
      }

      // Check if operation succeeded.
      if (new_socket.get() >= 0)
      {
        if (peer_endpoint)
          peer_endpoint->resize(addr_len);
        peer.assign(impl.protocol_, new_socket.get(), ec);
        if (!ec)
          new_socket.release();
        return ec;
      }

      // Operation failed.
      if (ec == boost::asio::error::would_block
          || ec == boost::asio::error::try_again)
      {
        if (impl.flags_ & implementation_type::user_set_non_blocking)
          return ec;
        // Fall through to retry operation.
      }
      else if (ec == boost::asio::error::connection_aborted)
      {
        if (impl.flags_ & implementation_type::enable_connection_aborted)
          return ec;
        // Fall through to retry operation.
      }
#if defined(EPROTO)
      else if (ec.value() == EPROTO)
      {
        if (impl.flags_ & implementation_type::enable_connection_aborted)
          return ec;
        // Fall through to retry operation.
      }
#endif // defined(EPROTO)
      else
        return ec;

      // Wait for socket to become ready.
      if (socket_ops::poll_read(impl.socket_, ec) < 0)
        return ec;
    }
  }

  template <typename Socket, typename Handler>
  class accept_operation :
    public handler_base_from_member<Handler>
  {
  public:
    accept_operation(socket_type socket, boost::asio::io_service& io_service,
        Socket& peer, const protocol_type& protocol,
        endpoint_type* peer_endpoint, bool enable_connection_aborted,
        Handler handler)
      : handler_base_from_member<Handler>(handler),
        socket_(socket),
        io_service_(io_service),
        work_(io_service),
        peer_(peer),
        protocol_(protocol),
        peer_endpoint_(peer_endpoint),
        enable_connection_aborted_(enable_connection_aborted)
    {
    }

    bool perform(boost::system::error_code& ec, std::size_t&)
    {
      // Check whether the operation was successful.
      if (ec)
        return true;

      // Accept the waiting connection.
      socket_holder new_socket;
      std::size_t addr_len = 0;
      if (peer_endpoint_)
      {
        addr_len = peer_endpoint_->capacity();
        new_socket.reset(socket_ops::accept(socket_,
              peer_endpoint_->data(), &addr_len, ec));
      }
      else
      {
        new_socket.reset(socket_ops::accept(socket_, 0, 0, ec));
      }

      // Check if we need to run the operation again.
      if (ec == boost::asio::error::would_block
          || ec == boost::asio::error::try_again)
        return false;
      if (ec == boost::asio::error::connection_aborted
          && !enable_connection_aborted_)
        return false;
#if defined(EPROTO)
      if (ec.value() == EPROTO && !enable_connection_aborted_)
        return false;
#endif // defined(EPROTO)

      // Transfer ownership of the new socket to the peer object.
      if (!ec)
      {
        if (peer_endpoint_)
          peer_endpoint_->resize(addr_len);
        peer_.assign(protocol_, new_socket.get(), ec);
        if (!ec)
          new_socket.release();
      }

      return true;
    }

    void complete(const boost::system::error_code& ec, std::size_t)
    {
      io_service_.post(bind_handler(this->handler_, ec));
    }

  private:
    socket_type socket_;
    boost::asio::io_service& io_service_;
    boost::asio::io_service::work work_;
    Socket& peer_;
    protocol_type protocol_;
    endpoint_type* peer_endpoint_;
    bool enable_connection_aborted_;
  };

  // Start an asynchronous accept. The peer and peer_endpoint objects
  // must be valid until the accept's handler is invoked.
  template <typename Socket, typename Handler>
  void async_accept(implementation_type& impl, Socket& peer,
      endpoint_type* peer_endpoint, Handler handler)
  {
    if (!is_open(impl))
    {
      this->get_io_service().post(bind_handler(handler,
            boost::asio::error::bad_descriptor));
    }
    else if (peer.is_open())
    {
      this->get_io_service().post(bind_handler(handler,
            boost::asio::error::already_open));
    }
    else
    {
      // Make socket non-blocking.
      if (!(impl.flags_ & implementation_type::internal_non_blocking))
      {
        if (!(impl.flags_ & implementation_type::non_blocking))
        {
          ioctl_arg_type non_blocking = 1;
          boost::system::error_code ec;
          if (socket_ops::ioctl(impl.socket_, FIONBIO, &non_blocking, ec))
          {
            this->get_io_service().post(bind_handler(handler, ec));
            return;
          }
        }
        impl.flags_ |= implementation_type::internal_non_blocking;
      }

      reactor_.start_read_op(impl.socket_, impl.reactor_data_,
          accept_operation<Socket, Handler>(
            impl.socket_, this->get_io_service(),
            peer, impl.protocol_, peer_endpoint,
            (impl.flags_ & implementation_type::enable_connection_aborted) != 0,
            handler));
    }
  }

  // Connect the socket to the specified endpoint.
  boost::system::error_code connect(implementation_type& impl,
      const endpoint_type& peer_endpoint, boost::system::error_code& ec)
  {
    if (!is_open(impl))
    {
      ec = boost::asio::error::bad_descriptor;
      return ec;
    }

    // Perform the connect operation.
    socket_ops::connect(impl.socket_,
        peer_endpoint.data(), peer_endpoint.size(), ec);
    if (ec != boost::asio::error::in_progress
        && ec != boost::asio::error::would_block)
    {
      // The connect operation finished immediately.
      return ec;
    }

    // Wait for socket to become ready.
    if (socket_ops::poll_connect(impl.socket_, ec) < 0)
      return ec;

    // Get the error code from the connect operation.
    int connect_error = 0;
    size_t connect_error_len = sizeof(connect_error);
    if (socket_ops::getsockopt(impl.socket_, SOL_SOCKET, SO_ERROR,
          &connect_error, &connect_error_len, ec) == socket_error_retval)
      return ec;

    // Return the result of the connect operation.
    ec = boost::system::error_code(connect_error,
        boost::asio::error::get_system_category());
    return ec;
  }

  template <typename Handler>
  class connect_operation :
    public handler_base_from_member<Handler>
  {
  public:
    connect_operation(socket_type socket,
        boost::asio::io_service& io_service, Handler handler)
      : handler_base_from_member<Handler>(handler),
        socket_(socket),
        io_service_(io_service),
        work_(io_service)
    {
    }

    bool perform(boost::system::error_code& ec, std::size_t&)
    {
      // Check whether the operation was successful.
      if (ec)
        return true;

      // Get the error code from the connect operation.
      int connect_error = 0;
      size_t connect_error_len = sizeof(connect_error);
      if (socket_ops::getsockopt(socket_, SOL_SOCKET, SO_ERROR,
            &connect_error, &connect_error_len, ec) == socket_error_retval)
        return true;

      // The connection failed so the handler will be posted with an error code.
      if (connect_error)
      {
        ec = boost::system::error_code(connect_error,
            boost::asio::error::get_system_category());
        return true;
      }

      return true;
    }

    void complete(const boost::system::error_code& ec, std::size_t)
    {
      io_service_.post(bind_handler(this->handler_, ec));
    }

  private:
    socket_type socket_;
    boost::asio::io_service& io_service_;
    boost::asio::io_service::work work_;
  };

  // Start an asynchronous connect.
  template <typename Handler>
  void async_connect(implementation_type& impl,
      const endpoint_type& peer_endpoint, Handler handler)
  {
    if (!is_open(impl))
    {
      this->get_io_service().post(bind_handler(handler,
            boost::asio::error::bad_descriptor));
      return;
    }

    // Make socket non-blocking.
    if (!(impl.flags_ & implementation_type::internal_non_blocking))
    {
      if (!(impl.flags_ & implementation_type::non_blocking))
      {
        ioctl_arg_type non_blocking = 1;
        boost::system::error_code ec;
        if (socket_ops::ioctl(impl.socket_, FIONBIO, &non_blocking, ec))
        {
          this->get_io_service().post(bind_handler(handler, ec));
          return;
        }
      }
      impl.flags_ |= implementation_type::internal_non_blocking;
    }

    // Start the connect operation. The socket is already marked as non-blocking
    // so the connection will take place asynchronously.
    boost::system::error_code ec;
    if (socket_ops::connect(impl.socket_, peer_endpoint.data(),
          peer_endpoint.size(), ec) == 0)
    {
      // The connect operation has finished successfully so we need to post the
      // handler immediately.
      this->get_io_service().post(bind_handler(handler,
            boost::system::error_code()));
    }
    else if (ec == boost::asio::error::in_progress
        || ec == boost::asio::error::would_block)
    {
      // The connection is happening in the background, and we need to wait
      // until the socket becomes writeable.
      reactor_.start_connect_op(impl.socket_, impl.reactor_data_,
          connect_operation<Handler>(impl.socket_,
            this->get_io_service(), handler));
    }
    else
    {
      // The connect operation has failed, so post the handler immediately.
      this->get_io_service().post(bind_handler(handler, ec));
    }
  }

private:
  // The selector that performs event demultiplexing for the service.
  Reactor& reactor_;
};

} // namespace detail
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_REACTIVE_SOCKET_SERVICE_HPP
