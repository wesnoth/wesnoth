//
// win_iocp_socket_service.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_WIN_IOCP_SOCKET_SERVICE_HPP
#define BOOST_ASIO_DETAIL_WIN_IOCP_SOCKET_SERVICE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/win_iocp_io_service_fwd.hpp>

#if defined(BOOST_ASIO_HAS_IOCP)

#include <boost/asio/detail/push_options.hpp>
#include <cstring>
#include <boost/shared_ptr.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/buffer.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/socket_base.hpp>
#include <boost/asio/detail/bind_handler.hpp>
#include <boost/asio/detail/handler_alloc_helpers.hpp>
#include <boost/asio/detail/handler_invoke_helpers.hpp>
#include <boost/asio/detail/mutex.hpp>
#include <boost/asio/detail/select_reactor.hpp>
#include <boost/asio/detail/socket_holder.hpp>
#include <boost/asio/detail/socket_ops.hpp>
#include <boost/asio/detail/socket_types.hpp>
#include <boost/asio/detail/win_iocp_io_service.hpp>

namespace boost {
namespace asio {
namespace detail {

template <typename Protocol>
class win_iocp_socket_service
  : public boost::asio::detail::service_base<win_iocp_socket_service<Protocol> >
{
public:
  // The protocol type.
  typedef Protocol protocol_type;

  // The endpoint type.
  typedef typename Protocol::endpoint endpoint_type;

  // Base class for all operations.
  typedef win_iocp_io_service::operation operation;

  struct noop_deleter { void operator()(void*) {} };
  typedef boost::shared_ptr<void> shared_cancel_token_type;
  typedef boost::weak_ptr<void> weak_cancel_token_type;

  // The native type of a socket.
  class native_type
  {
  public:
    native_type(socket_type s)
      : socket_(s),
        have_remote_endpoint_(false)
    {
    }

    native_type(socket_type s, const endpoint_type& ep)
      : socket_(s),
        have_remote_endpoint_(true),
        remote_endpoint_(ep)
    {
    }

    void operator=(socket_type s)
    {
      socket_ = s;
      have_remote_endpoint_ = false;
      remote_endpoint_ = endpoint_type();
    }

    operator socket_type() const
    {
      return socket_;
    }

    HANDLE as_handle() const
    {
      return reinterpret_cast<HANDLE>(socket_);
    }

    bool have_remote_endpoint() const
    {
      return have_remote_endpoint_;
    }

    endpoint_type remote_endpoint() const
    {
      return remote_endpoint_;
    }

  private:
    socket_type socket_;
    bool have_remote_endpoint_;
    endpoint_type remote_endpoint_;
  };

  // The type of the reactor used for connect operations.
  typedef detail::select_reactor<true> reactor_type;

  // The implementation type of the socket.
  class implementation_type
  {
  public:
    // Default constructor.
    implementation_type()
      : socket_(invalid_socket),
        flags_(0),
        cancel_token_(),
        protocol_(endpoint_type().protocol()),
        next_(0),
        prev_(0)
    {
    }

  private:
    // Only this service will have access to the internal values.
    friend class win_iocp_socket_service;

    // The native socket representation.
    native_type socket_;

    enum
    {
      enable_connection_aborted = 1, // User wants connection_aborted errors.
      close_might_block = 2, // User set linger option for blocking close.
      user_set_non_blocking = 4 // The user wants a non-blocking socket.
    };

    // Flags indicating the current state of the socket.
    unsigned char flags_;

    // We use a shared pointer as a cancellation token here to work around the
    // broken Windows support for cancellation. MSDN says that when you call
    // closesocket any outstanding WSARecv or WSASend operations will complete
    // with the error ERROR_OPERATION_ABORTED. In practice they complete with
    // ERROR_NETNAME_DELETED, which means you can't tell the difference between
    // a local cancellation and the socket being hard-closed by the peer.
    shared_cancel_token_type cancel_token_;

    // The protocol associated with the socket.
    protocol_type protocol_;

    // Per-descriptor data used by the reactor.
    reactor_type::per_descriptor_data reactor_data_;

#if defined(BOOST_ASIO_ENABLE_CANCELIO)
    // The ID of the thread from which it is safe to cancel asynchronous
    // operations. 0 means no asynchronous operations have been started yet.
    // ~0 means asynchronous operations have been started from more than one
    // thread, and cancellation is not supported for the socket.
    DWORD safe_cancellation_thread_id_;
#endif // defined(BOOST_ASIO_ENABLE_CANCELIO)

    // Pointers to adjacent socket implementations in linked list.
    implementation_type* next_;
    implementation_type* prev_;
  };

  // The maximum number of buffers to support in a single operation.
  enum { max_buffers = 64 < max_iov_len ? 64 : max_iov_len };

  // Constructor.
  win_iocp_socket_service(boost::asio::io_service& io_service)
    : boost::asio::detail::service_base<
        win_iocp_socket_service<Protocol> >(io_service),
      iocp_service_(boost::asio::use_service<win_iocp_io_service>(io_service)),
      reactor_(0),
      mutex_(),
      impl_list_(0)
  {
  }

  // Destroy all user-defined handler objects owned by the service.
  void shutdown_service()
  {
    // Close all implementations, causing all operations to complete.
    boost::asio::detail::mutex::scoped_lock lock(mutex_);
    implementation_type* impl = impl_list_;
    while (impl)
    {
      boost::system::error_code ignored_ec;
      close_for_destruction(*impl);
      impl = impl->next_;
    }
  }

  // Construct a new socket implementation.
  void construct(implementation_type& impl)
  {
    impl.socket_ = invalid_socket;
    impl.flags_ = 0;
    impl.cancel_token_.reset();
#if defined(BOOST_ASIO_ENABLE_CANCELIO)
    impl.safe_cancellation_thread_id_ = 0;
#endif // defined(BOOST_ASIO_ENABLE_CANCELIO)

    // Insert implementation into linked list of all implementations.
    boost::asio::detail::mutex::scoped_lock lock(mutex_);
    impl.next_ = impl_list_;
    impl.prev_ = 0;
    if (impl_list_)
      impl_list_->prev_ = &impl;
    impl_list_ = &impl;
  }

  // Destroy a socket implementation.
  void destroy(implementation_type& impl)
  {
    close_for_destruction(impl);

    // Remove implementation from linked list of all implementations.
    boost::asio::detail::mutex::scoped_lock lock(mutex_);
    if (impl_list_ == &impl)
      impl_list_ = impl.next_;
    if (impl.prev_)
      impl.prev_->next_ = impl.next_;
    if (impl.next_)
      impl.next_->prev_= impl.prev_;
    impl.next_ = 0;
    impl.prev_ = 0;
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

    socket_holder sock(socket_ops::socket(protocol.family(), protocol.type(),
          protocol.protocol(), ec));
    if (sock.get() == invalid_socket)
      return ec;

    HANDLE sock_as_handle = reinterpret_cast<HANDLE>(sock.get());
    if (iocp_service_.register_handle(sock_as_handle, ec))
      return ec;

    impl.socket_ = sock.release();
    impl.flags_ = 0;
    impl.cancel_token_.reset(static_cast<void*>(0), noop_deleter());
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

    if (iocp_service_.register_handle(native_socket.as_handle(), ec))
      return ec;

    impl.socket_ = native_socket;
    impl.flags_ = 0;
    impl.cancel_token_.reset(static_cast<void*>(0), noop_deleter());
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
      // Check if the reactor was created, in which case we need to close the
      // socket on the reactor as well to cancel any operations that might be
      // running there.
      reactor_type* reactor = static_cast<reactor_type*>(
            interlocked_compare_exchange_pointer(
              reinterpret_cast<void**>(&reactor_), 0, 0));
      if (reactor)
        reactor->close_descriptor(impl.socket_, impl.reactor_data_);

      if (socket_ops::close(impl.socket_, ec) == socket_error_retval)
        return ec;

      impl.socket_ = invalid_socket;
      impl.flags_ = 0;
      impl.cancel_token_.reset();
#if defined(BOOST_ASIO_ENABLE_CANCELIO)
      impl.safe_cancellation_thread_id_ = 0;
#endif // defined(BOOST_ASIO_ENABLE_CANCELIO)
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
    else if (FARPROC cancel_io_ex_ptr = ::GetProcAddress(
          ::GetModuleHandleA("KERNEL32"), "CancelIoEx"))
    {
      // The version of Windows supports cancellation from any thread.
      typedef BOOL (WINAPI* cancel_io_ex_t)(HANDLE, LPOVERLAPPED);
      cancel_io_ex_t cancel_io_ex = (cancel_io_ex_t)cancel_io_ex_ptr;
      socket_type sock = impl.socket_;
      HANDLE sock_as_handle = reinterpret_cast<HANDLE>(sock);
      if (!cancel_io_ex(sock_as_handle, 0))
      {
        DWORD last_error = ::GetLastError();
        if (last_error == ERROR_NOT_FOUND)
        {
          // ERROR_NOT_FOUND means that there were no operations to be
          // cancelled. We swallow this error to match the behaviour on other
          // platforms.
          ec = boost::system::error_code();
        }
        else
        {
          ec = boost::system::error_code(last_error,
              boost::asio::error::get_system_category());
        }
      }
      else
      {
        ec = boost::system::error_code();
      }
    }
#if defined(BOOST_ASIO_ENABLE_CANCELIO)
    else if (impl.safe_cancellation_thread_id_ == 0)
    {
      // No operations have been started, so there's nothing to cancel.
      ec = boost::system::error_code();
    }
    else if (impl.safe_cancellation_thread_id_ == ::GetCurrentThreadId())
    {
      // Asynchronous operations have been started from the current thread only,
      // so it is safe to try to cancel them using CancelIo.
      socket_type sock = impl.socket_;
      HANDLE sock_as_handle = reinterpret_cast<HANDLE>(sock);
      if (!::CancelIo(sock_as_handle))
      {
        DWORD last_error = ::GetLastError();
        ec = boost::system::error_code(last_error,
            boost::asio::error::get_system_category());
      }
      else
      {
        ec = boost::system::error_code();
      }
    }
    else
    {
      // Asynchronous operations have been started from more than one thread,
      // so cancellation is not safe.
      ec = boost::asio::error::operation_not_supported;
    }
#else // defined(BOOST_ASIO_ENABLE_CANCELIO)
    else
    {
      // Cancellation is not supported as CancelIo may not be used.
      ec = boost::asio::error::operation_not_supported;
    }
#endif // defined(BOOST_ASIO_ENABLE_CANCELIO)

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
        const ::linger* linger_option =
          reinterpret_cast<const ::linger*>(option.data(impl.protocol_));
        if (linger_option->l_onoff != 0 && linger_option->l_linger != 0)
          impl.flags_ |= implementation_type::close_might_block;
        else
          impl.flags_ &= ~implementation_type::close_might_block;
      }

      socket_ops::setsockopt(impl.socket_,
          option.level(impl.protocol_), option.name(impl.protocol_),
          option.data(impl.protocol_), option.size(impl.protocol_), ec);
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

    socket_ops::ioctl(impl.socket_, command.name(),
        static_cast<ioctl_arg_type*>(command.data()), ec);

    if (!ec && command.name() == static_cast<int>(FIONBIO))
    {
      if (command.get())
        impl.flags_ |= implementation_type::user_set_non_blocking;
      else
        impl.flags_ &= ~implementation_type::user_set_non_blocking;
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

    if (impl.socket_.have_remote_endpoint())
    {
      // Check if socket is still connected.
      DWORD connect_time = 0;
      size_t connect_time_len = sizeof(connect_time);
      if (socket_ops::getsockopt(impl.socket_, SOL_SOCKET, SO_CONNECT_TIME,
            &connect_time, &connect_time_len, ec) == socket_error_retval)
      {
        return endpoint_type();
      }
      if (connect_time == 0xFFFFFFFF)
      {
        ec = boost::asio::error::not_connected;
        return endpoint_type();
      }

      ec = boost::system::error_code();
      return impl.socket_.remote_endpoint();
    }
    else
    {
      endpoint_type endpoint;
      std::size_t addr_len = endpoint.capacity();
      if (socket_ops::getpeername(impl.socket_, endpoint.data(), &addr_len, ec))
        return endpoint_type();
      endpoint.resize(addr_len);
      return endpoint;
    }
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

  // Send the given data to the peer. Returns the number of bytes sent.
  template <typename ConstBufferSequence>
  size_t send(implementation_type& impl, const ConstBufferSequence& buffers,
      socket_base::message_flags flags, boost::system::error_code& ec)
  {
    if (!is_open(impl))
    {
      ec = boost::asio::error::bad_descriptor;
      return 0;
    }

    // Copy buffers into WSABUF array.
    ::WSABUF bufs[max_buffers];
    typename ConstBufferSequence::const_iterator iter = buffers.begin();
    typename ConstBufferSequence::const_iterator end = buffers.end();
    DWORD i = 0;
    size_t total_buffer_size = 0;
    for (; iter != end && i < max_buffers; ++iter, ++i)
    {
      boost::asio::const_buffer buffer(*iter);
      bufs[i].len = static_cast<u_long>(boost::asio::buffer_size(buffer));
      bufs[i].buf = const_cast<char*>(
          boost::asio::buffer_cast<const char*>(buffer));
      total_buffer_size += boost::asio::buffer_size(buffer);
    }

    // A request to receive 0 bytes on a stream socket is a no-op.
    if (impl.protocol_.type() == SOCK_STREAM && total_buffer_size == 0)
    {
      ec = boost::system::error_code();
      return 0;
    }

    // Send the data.
    DWORD bytes_transferred = 0;
    int result = ::WSASend(impl.socket_, bufs,
        i, &bytes_transferred, flags, 0, 0);
    if (result != 0)
    {
      DWORD last_error = ::WSAGetLastError();
      if (last_error == ERROR_NETNAME_DELETED)
        last_error = WSAECONNRESET;
      else if (last_error == ERROR_PORT_UNREACHABLE)
        last_error = WSAECONNREFUSED;
      ec = boost::system::error_code(last_error,
          boost::asio::error::get_system_category());
      return 0;
    }

    ec = boost::system::error_code();
    return bytes_transferred;
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
  class send_operation
    : public operation
  {
  public:
    send_operation(win_iocp_io_service& io_service,
        weak_cancel_token_type cancel_token,
        const ConstBufferSequence& buffers, Handler handler)
      : operation(io_service,
          &send_operation<ConstBufferSequence, Handler>::do_completion_impl,
          &send_operation<ConstBufferSequence, Handler>::destroy_impl),
        work_(io_service.get_io_service()),
        cancel_token_(cancel_token),
        buffers_(buffers),
        handler_(handler)
    {
    }

  private:
    static void do_completion_impl(operation* op,
        DWORD last_error, size_t bytes_transferred)
    {
      // Take ownership of the operation object.
      typedef send_operation<ConstBufferSequence, Handler> op_type;
      op_type* handler_op(static_cast<op_type*>(op));
      typedef handler_alloc_traits<Handler, op_type> alloc_traits;
      handler_ptr<alloc_traits> ptr(handler_op->handler_, handler_op);

#if defined(BOOST_ASIO_ENABLE_BUFFER_DEBUGGING)
      // Check whether buffers are still valid.
      typename ConstBufferSequence::const_iterator iter
        = handler_op->buffers_.begin();
      typename ConstBufferSequence::const_iterator end
        = handler_op->buffers_.end();
      while (iter != end)
      {
        boost::asio::const_buffer buffer(*iter);
        boost::asio::buffer_cast<const char*>(buffer);
        ++iter;
      }
#endif // defined(BOOST_ASIO_ENABLE_BUFFER_DEBUGGING)

      // Map non-portable errors to their portable counterparts.
      boost::system::error_code ec(last_error,
          boost::asio::error::get_system_category());
      if (ec.value() == ERROR_NETNAME_DELETED)
      {
        if (handler_op->cancel_token_.expired())
          ec = boost::asio::error::operation_aborted;
        else
          ec = boost::asio::error::connection_reset;
      }
      else if (ec.value() == ERROR_PORT_UNREACHABLE)
      {
        ec = boost::asio::error::connection_refused;
      }

      // Make a copy of the handler so that the memory can be deallocated before
      // the upcall is made.
      Handler handler(handler_op->handler_);

      // Free the memory associated with the handler.
      ptr.reset();

      // Call the handler.
      boost_asio_handler_invoke_helpers::invoke(
          detail::bind_handler(handler, ec, bytes_transferred), &handler);
    }

    static void destroy_impl(operation* op)
    {
      // Take ownership of the operation object.
      typedef send_operation<ConstBufferSequence, Handler> op_type;
      op_type* handler_op(static_cast<op_type*>(op));
      typedef handler_alloc_traits<Handler, op_type> alloc_traits;
      handler_ptr<alloc_traits> ptr(handler_op->handler_, handler_op);

      // A sub-object of the handler may be the true owner of the memory
      // associated with the handler. Consequently, a local copy of the handler
      // is required to ensure that any owning sub-object remains valid until
      // after we have deallocated the memory here.
      Handler handler(handler_op->handler_);
      (void)handler;

      // Free the memory associated with the handler.
      ptr.reset();
    }

    boost::asio::io_service::work work_;
    weak_cancel_token_type cancel_token_;
    ConstBufferSequence buffers_;
    Handler handler_;
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
      return;
    }

#if defined(BOOST_ASIO_ENABLE_CANCELIO)
    // Update the ID of the thread from which cancellation is safe.
    if (impl.safe_cancellation_thread_id_ == 0)
      impl.safe_cancellation_thread_id_ = ::GetCurrentThreadId();
    else if (impl.safe_cancellation_thread_id_ != ::GetCurrentThreadId())
      impl.safe_cancellation_thread_id_ = ~DWORD(0);
#endif // defined(BOOST_ASIO_ENABLE_CANCELIO)

    // Allocate and construct an operation to wrap the handler.
    typedef send_operation<ConstBufferSequence, Handler> value_type;
    typedef handler_alloc_traits<Handler, value_type> alloc_traits;
    raw_handler_ptr<alloc_traits> raw_ptr(handler);
    handler_ptr<alloc_traits> ptr(raw_ptr, iocp_service_,
        impl.cancel_token_, buffers, handler);

    // Copy buffers into WSABUF array.
    ::WSABUF bufs[max_buffers];
    typename ConstBufferSequence::const_iterator iter = buffers.begin();
    typename ConstBufferSequence::const_iterator end = buffers.end();
    DWORD i = 0;
    size_t total_buffer_size = 0;
    for (; iter != end && i < max_buffers; ++iter, ++i)
    {
      boost::asio::const_buffer buffer(*iter);
      bufs[i].len = static_cast<u_long>(boost::asio::buffer_size(buffer));
      bufs[i].buf = const_cast<char*>(
          boost::asio::buffer_cast<const char*>(buffer));
      total_buffer_size += boost::asio::buffer_size(buffer);
    }

    // A request to receive 0 bytes on a stream socket is a no-op.
    if (impl.protocol_.type() == SOCK_STREAM && total_buffer_size == 0)
    {
      boost::asio::io_service::work work(this->get_io_service());
      ptr.reset();
      boost::system::error_code error;
      iocp_service_.post(bind_handler(handler, error, 0));
      return;
    }

    // Send the data.
    DWORD bytes_transferred = 0;
    int result = ::WSASend(impl.socket_, bufs, i,
        &bytes_transferred, flags, ptr.get(), 0);
    DWORD last_error = ::WSAGetLastError();

    // Check if the operation completed immediately.
    if (result != 0 && last_error != WSA_IO_PENDING)
    {
      boost::asio::io_service::work work(this->get_io_service());
      ptr.reset();
      boost::system::error_code ec(last_error,
          boost::asio::error::get_system_category());
      iocp_service_.post(bind_handler(handler, ec, bytes_transferred));
    }
    else
    {
      ptr.release();
    }
  }

  template <typename Handler>
  class null_buffers_operation
  {
  public:
    null_buffers_operation(boost::asio::io_service& io_service, Handler handler)
      : work_(io_service),
        handler_(handler)
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
            handler_, ec, bytes_transferred));
    }

  private:
    boost::asio::io_service::work work_;
    Handler handler_;
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
      // Check if the reactor was already obtained from the io_service.
      reactor_type* reactor = static_cast<reactor_type*>(
            interlocked_compare_exchange_pointer(
              reinterpret_cast<void**>(&reactor_), 0, 0));
      if (!reactor)
      {
        reactor = &(boost::asio::use_service<reactor_type>(
              this->get_io_service()));
        interlocked_exchange_pointer(
            reinterpret_cast<void**>(&reactor_), reactor);
      }

      reactor->start_write_op(impl.socket_, impl.reactor_data_,
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

    // Copy buffers into WSABUF array.
    ::WSABUF bufs[max_buffers];
    typename ConstBufferSequence::const_iterator iter = buffers.begin();
    typename ConstBufferSequence::const_iterator end = buffers.end();
    DWORD i = 0;
    for (; iter != end && i < max_buffers; ++iter, ++i)
    {
      boost::asio::const_buffer buffer(*iter);
      bufs[i].len = static_cast<u_long>(boost::asio::buffer_size(buffer));
      bufs[i].buf = const_cast<char*>(
          boost::asio::buffer_cast<const char*>(buffer));
    }

    // Send the data.
    DWORD bytes_transferred = 0;
    int result = ::WSASendTo(impl.socket_, bufs, i, &bytes_transferred,
        flags, destination.data(), static_cast<int>(destination.size()), 0, 0);
    if (result != 0)
    {
      DWORD last_error = ::WSAGetLastError();
      if (last_error == ERROR_PORT_UNREACHABLE)
        last_error = WSAECONNREFUSED;
      ec = boost::system::error_code(last_error,
          boost::asio::error::get_system_category());
      return 0;
    }

    ec = boost::system::error_code();
    return bytes_transferred;
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
  class send_to_operation
    : public operation
  {
  public:
    send_to_operation(win_iocp_io_service& io_service,
        const ConstBufferSequence& buffers, Handler handler)
      : operation(io_service,
          &send_to_operation<ConstBufferSequence, Handler>::do_completion_impl,
          &send_to_operation<ConstBufferSequence, Handler>::destroy_impl),
        work_(io_service.get_io_service()),
        buffers_(buffers),
        handler_(handler)
    {
    }

  private:
    static void do_completion_impl(operation* op,
        DWORD last_error, size_t bytes_transferred)
    {
      // Take ownership of the operation object.
      typedef send_to_operation<ConstBufferSequence, Handler> op_type;
      op_type* handler_op(static_cast<op_type*>(op));
      typedef handler_alloc_traits<Handler, op_type> alloc_traits;
      handler_ptr<alloc_traits> ptr(handler_op->handler_, handler_op);

#if defined(BOOST_ASIO_ENABLE_BUFFER_DEBUGGING)
      // Check whether buffers are still valid.
      typename ConstBufferSequence::const_iterator iter
        = handler_op->buffers_.begin();
      typename ConstBufferSequence::const_iterator end
        = handler_op->buffers_.end();
      while (iter != end)
      {
        boost::asio::const_buffer buffer(*iter);
        boost::asio::buffer_cast<const char*>(buffer);
        ++iter;
      }
#endif // defined(BOOST_ASIO_ENABLE_BUFFER_DEBUGGING)

      // Map non-portable errors to their portable counterparts.
      boost::system::error_code ec(last_error,
          boost::asio::error::get_system_category());
      if (ec.value() == ERROR_PORT_UNREACHABLE)
      {
        ec = boost::asio::error::connection_refused;
      }

      // Make a copy of the handler so that the memory can be deallocated before
      // the upcall is made.
      Handler handler(handler_op->handler_);

      // Free the memory associated with the handler.
      ptr.reset();

      // Call the handler.
      boost_asio_handler_invoke_helpers::invoke(
          detail::bind_handler(handler, ec, bytes_transferred), &handler);
    }

    static void destroy_impl(operation* op)
    {
      // Take ownership of the operation object.
      typedef send_to_operation<ConstBufferSequence, Handler> op_type;
      op_type* handler_op(static_cast<op_type*>(op));
      typedef handler_alloc_traits<Handler, op_type> alloc_traits;
      handler_ptr<alloc_traits> ptr(handler_op->handler_, handler_op);

      // A sub-object of the handler may be the true owner of the memory
      // associated with the handler. Consequently, a local copy of the handler
      // is required to ensure that any owning sub-object remains valid until
      // after we have deallocated the memory here.
      Handler handler(handler_op->handler_);
      (void)handler;

      // Free the memory associated with the handler.
      ptr.reset();
    }

    boost::asio::io_service::work work_;
    ConstBufferSequence buffers_;
    Handler handler_;
  };

  // Start an asynchronous send. The data being sent must be valid for the
  // lifetime of the asynchronous operation.
  template <typename ConstBufferSequence, typename Handler>
  void async_send_to(implementation_type& impl,
      const ConstBufferSequence& buffers, const endpoint_type& destination,
      socket_base::message_flags flags, Handler handler)
  {
    if (!is_open(impl))
    {
      this->get_io_service().post(bind_handler(handler,
            boost::asio::error::bad_descriptor, 0));
      return;
    }

#if defined(BOOST_ASIO_ENABLE_CANCELIO)
    // Update the ID of the thread from which cancellation is safe.
    if (impl.safe_cancellation_thread_id_ == 0)
      impl.safe_cancellation_thread_id_ = ::GetCurrentThreadId();
    else if (impl.safe_cancellation_thread_id_ != ::GetCurrentThreadId())
      impl.safe_cancellation_thread_id_ = ~DWORD(0);
#endif // defined(BOOST_ASIO_ENABLE_CANCELIO)

    // Allocate and construct an operation to wrap the handler.
    typedef send_to_operation<ConstBufferSequence, Handler> value_type;
    typedef handler_alloc_traits<Handler, value_type> alloc_traits;
    raw_handler_ptr<alloc_traits> raw_ptr(handler);
    handler_ptr<alloc_traits> ptr(raw_ptr, iocp_service_, buffers, handler);

    // Copy buffers into WSABUF array.
    ::WSABUF bufs[max_buffers];
    typename ConstBufferSequence::const_iterator iter = buffers.begin();
    typename ConstBufferSequence::const_iterator end = buffers.end();
    DWORD i = 0;
    for (; iter != end && i < max_buffers; ++iter, ++i)
    {
      boost::asio::const_buffer buffer(*iter);
      bufs[i].len = static_cast<u_long>(boost::asio::buffer_size(buffer));
      bufs[i].buf = const_cast<char*>(
          boost::asio::buffer_cast<const char*>(buffer));
    }

    // Send the data.
    DWORD bytes_transferred = 0;
    int result = ::WSASendTo(impl.socket_, bufs, i, &bytes_transferred, flags,
        destination.data(), static_cast<int>(destination.size()), ptr.get(), 0);
    DWORD last_error = ::WSAGetLastError();

    // Check if the operation completed immediately.
    if (result != 0 && last_error != WSA_IO_PENDING)
    {
      boost::asio::io_service::work work(this->get_io_service());
      ptr.reset();
      boost::system::error_code ec(last_error,
          boost::asio::error::get_system_category());
      iocp_service_.post(bind_handler(handler, ec, bytes_transferred));
    }
    else
    {
      ptr.release();
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
      // Check if the reactor was already obtained from the io_service.
      reactor_type* reactor = static_cast<reactor_type*>(
            interlocked_compare_exchange_pointer(
              reinterpret_cast<void**>(&reactor_), 0, 0));
      if (!reactor)
      {
        reactor = &(boost::asio::use_service<reactor_type>(
              this->get_io_service()));
        interlocked_exchange_pointer(
            reinterpret_cast<void**>(&reactor_), reactor);
      }

      reactor->start_write_op(impl.socket_, impl.reactor_data_,
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

    // Copy buffers into WSABUF array.
    ::WSABUF bufs[max_buffers];
    typename MutableBufferSequence::const_iterator iter = buffers.begin();
    typename MutableBufferSequence::const_iterator end = buffers.end();
    DWORD i = 0;
    size_t total_buffer_size = 0;
    for (; iter != end && i < max_buffers; ++iter, ++i)
    {
      boost::asio::mutable_buffer buffer(*iter);
      bufs[i].len = static_cast<u_long>(boost::asio::buffer_size(buffer));
      bufs[i].buf = boost::asio::buffer_cast<char*>(buffer);
      total_buffer_size += boost::asio::buffer_size(buffer);
    }

    // A request to receive 0 bytes on a stream socket is a no-op.
    if (impl.protocol_.type() == SOCK_STREAM && total_buffer_size == 0)
    {
      ec = boost::system::error_code();
      return 0;
    }

    // Receive some data.
    DWORD bytes_transferred = 0;
    DWORD recv_flags = flags;
    int result = ::WSARecv(impl.socket_, bufs, i,
        &bytes_transferred, &recv_flags, 0, 0);
    if (result != 0)
    {
      DWORD last_error = ::WSAGetLastError();
      if (last_error == ERROR_NETNAME_DELETED)
        last_error = WSAECONNRESET;
      else if (last_error == ERROR_PORT_UNREACHABLE)
        last_error = WSAECONNREFUSED;
      ec = boost::system::error_code(last_error,
          boost::asio::error::get_system_category());
      return 0;
    }
    if (bytes_transferred == 0 && impl.protocol_.type() == SOCK_STREAM)
    {
      ec = boost::asio::error::eof;
      return 0;
    }

    ec = boost::system::error_code();
    return bytes_transferred;
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
  class receive_operation
    : public operation
  {
  public:
    receive_operation(int protocol_type, win_iocp_io_service& io_service,
        weak_cancel_token_type cancel_token,
        const MutableBufferSequence& buffers, Handler handler)
      : operation(io_service,
          &receive_operation<
            MutableBufferSequence, Handler>::do_completion_impl,
          &receive_operation<
            MutableBufferSequence, Handler>::destroy_impl),
        protocol_type_(protocol_type),
        work_(io_service.get_io_service()),
        cancel_token_(cancel_token),
        buffers_(buffers),
        handler_(handler)
    {
    }

  private:
    static void do_completion_impl(operation* op,
        DWORD last_error, size_t bytes_transferred)
    {
      // Take ownership of the operation object.
      typedef receive_operation<MutableBufferSequence, Handler> op_type;
      op_type* handler_op(static_cast<op_type*>(op));
      typedef handler_alloc_traits<Handler, op_type> alloc_traits;
      handler_ptr<alloc_traits> ptr(handler_op->handler_, handler_op);

#if defined(BOOST_ASIO_ENABLE_BUFFER_DEBUGGING)
      // Check whether buffers are still valid.
      typename MutableBufferSequence::const_iterator iter
        = handler_op->buffers_.begin();
      typename MutableBufferSequence::const_iterator end
        = handler_op->buffers_.end();
      while (iter != end)
      {
        boost::asio::mutable_buffer buffer(*iter);
        boost::asio::buffer_cast<char*>(buffer);
        ++iter;
      }
#endif // defined(BOOST_ASIO_ENABLE_BUFFER_DEBUGGING)

      // Map non-portable errors to their portable counterparts.
      boost::system::error_code ec(last_error,
          boost::asio::error::get_system_category());
      if (ec.value() == ERROR_NETNAME_DELETED)
      {
        if (handler_op->cancel_token_.expired())
          ec = boost::asio::error::operation_aborted;
        else
          ec = boost::asio::error::connection_reset;
      }
      else if (ec.value() == ERROR_PORT_UNREACHABLE)
      {
        ec = boost::asio::error::connection_refused;
      }

      // Check for connection closed.
      else if (!ec && bytes_transferred == 0
          && handler_op->protocol_type_ == SOCK_STREAM
          && !boost::is_same<MutableBufferSequence, null_buffers>::value)
      {
        ec = boost::asio::error::eof;
      }

      // Make a copy of the handler so that the memory can be deallocated before
      // the upcall is made.
      Handler handler(handler_op->handler_);

      // Free the memory associated with the handler.
      ptr.reset();

      // Call the handler.
      boost_asio_handler_invoke_helpers::invoke(
          detail::bind_handler(handler, ec, bytes_transferred), &handler);
    }

    static void destroy_impl(operation* op)
    {
      // Take ownership of the operation object.
      typedef receive_operation<MutableBufferSequence, Handler> op_type;
      op_type* handler_op(static_cast<op_type*>(op));
      typedef handler_alloc_traits<Handler, op_type> alloc_traits;
      handler_ptr<alloc_traits> ptr(handler_op->handler_, handler_op);

      // A sub-object of the handler may be the true owner of the memory
      // associated with the handler. Consequently, a local copy of the handler
      // is required to ensure that any owning sub-object remains valid until
      // after we have deallocated the memory here.
      Handler handler(handler_op->handler_);
      (void)handler;

      // Free the memory associated with the handler.
      ptr.reset();
    }

    int protocol_type_;
    boost::asio::io_service::work work_;
    weak_cancel_token_type cancel_token_;
    MutableBufferSequence buffers_;
    Handler handler_;
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
      return;
    }

#if defined(BOOST_ASIO_ENABLE_CANCELIO)
    // Update the ID of the thread from which cancellation is safe.
    if (impl.safe_cancellation_thread_id_ == 0)
      impl.safe_cancellation_thread_id_ = ::GetCurrentThreadId();
    else if (impl.safe_cancellation_thread_id_ != ::GetCurrentThreadId())
      impl.safe_cancellation_thread_id_ = ~DWORD(0);
#endif // defined(BOOST_ASIO_ENABLE_CANCELIO)

    // Allocate and construct an operation to wrap the handler.
    typedef receive_operation<MutableBufferSequence, Handler> value_type;
    typedef handler_alloc_traits<Handler, value_type> alloc_traits;
    raw_handler_ptr<alloc_traits> raw_ptr(handler);
    int protocol_type = impl.protocol_.type();
    handler_ptr<alloc_traits> ptr(raw_ptr, protocol_type,
        iocp_service_, impl.cancel_token_, buffers, handler);

    // Copy buffers into WSABUF array.
    ::WSABUF bufs[max_buffers];
    typename MutableBufferSequence::const_iterator iter = buffers.begin();
    typename MutableBufferSequence::const_iterator end = buffers.end();
    DWORD i = 0;
    size_t total_buffer_size = 0;
    for (; iter != end && i < max_buffers; ++iter, ++i)
    {
      boost::asio::mutable_buffer buffer(*iter);
      bufs[i].len = static_cast<u_long>(boost::asio::buffer_size(buffer));
      bufs[i].buf = boost::asio::buffer_cast<char*>(buffer);
      total_buffer_size += boost::asio::buffer_size(buffer);
    }

    // A request to receive 0 bytes on a stream socket is a no-op.
    if (impl.protocol_.type() == SOCK_STREAM && total_buffer_size == 0)
    {
      boost::asio::io_service::work work(this->get_io_service());
      ptr.reset();
      boost::system::error_code error;
      iocp_service_.post(bind_handler(handler, error, 0));
      return;
    }

    // Receive some data.
    DWORD bytes_transferred = 0;
    DWORD recv_flags = flags;
    int result = ::WSARecv(impl.socket_, bufs, i,
        &bytes_transferred, &recv_flags, ptr.get(), 0);
    DWORD last_error = ::WSAGetLastError();
    if (result != 0 && last_error != WSA_IO_PENDING)
    {
      boost::asio::io_service::work work(this->get_io_service());
      ptr.reset();
      boost::system::error_code ec(last_error,
          boost::asio::error::get_system_category());
      iocp_service_.post(bind_handler(handler, ec, bytes_transferred));
    }
    else
    {
      ptr.release();
    }
  }

  // Wait until data can be received without blocking.
  template <typename Handler>
  void async_receive(implementation_type& impl, const null_buffers& buffers,
      socket_base::message_flags flags, Handler handler)
  {
    if (!is_open(impl))
    {
      this->get_io_service().post(bind_handler(handler,
            boost::asio::error::bad_descriptor, 0));
    }
    else if (impl.protocol_.type() == SOCK_STREAM)
    {
      // For stream sockets on Windows, we may issue a 0-byte overlapped
      // WSARecv to wait until there is data available on the socket.

#if defined(BOOST_ASIO_ENABLE_CANCELIO)
      // Update the ID of the thread from which cancellation is safe.
      if (impl.safe_cancellation_thread_id_ == 0)
        impl.safe_cancellation_thread_id_ = ::GetCurrentThreadId();
      else if (impl.safe_cancellation_thread_id_ != ::GetCurrentThreadId())
        impl.safe_cancellation_thread_id_ = ~DWORD(0);
#endif // defined(BOOST_ASIO_ENABLE_CANCELIO)

      // Allocate and construct an operation to wrap the handler.
      typedef receive_operation<null_buffers, Handler> value_type;
      typedef handler_alloc_traits<Handler, value_type> alloc_traits;
      raw_handler_ptr<alloc_traits> raw_ptr(handler);
      int protocol_type = impl.protocol_.type();
      handler_ptr<alloc_traits> ptr(raw_ptr, protocol_type,
          iocp_service_, impl.cancel_token_, buffers, handler);

      // Issue a receive operation with an empty buffer.
      ::WSABUF buf = { 0, 0 };
      DWORD bytes_transferred = 0;
      DWORD recv_flags = flags;
      int result = ::WSARecv(impl.socket_, &buf, 1,
          &bytes_transferred, &recv_flags, ptr.get(), 0);
      DWORD last_error = ::WSAGetLastError();
      if (result != 0 && last_error != WSA_IO_PENDING)
      {
        boost::asio::io_service::work work(this->get_io_service());
        ptr.reset();
        boost::system::error_code ec(last_error,
            boost::asio::error::get_system_category());
        iocp_service_.post(bind_handler(handler, ec, bytes_transferred));
      }
      else
      {
        ptr.release();
      }
    }
    else
    {
      // Check if the reactor was already obtained from the io_service.
      reactor_type* reactor = static_cast<reactor_type*>(
            interlocked_compare_exchange_pointer(
              reinterpret_cast<void**>(&reactor_), 0, 0));
      if (!reactor)
      {
        reactor = &(boost::asio::use_service<reactor_type>(
              this->get_io_service()));
        interlocked_exchange_pointer(
            reinterpret_cast<void**>(&reactor_), reactor);
      }

      if (flags & socket_base::message_out_of_band)
      {
        reactor->start_except_op(impl.socket_, impl.reactor_data_,
            null_buffers_operation<Handler>(this->get_io_service(), handler));
      }
      else
      {
        reactor->start_read_op(impl.socket_, impl.reactor_data_,
            null_buffers_operation<Handler>(this->get_io_service(), handler),
            false);
      }
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

    // Copy buffers into WSABUF array.
    ::WSABUF bufs[max_buffers];
    typename MutableBufferSequence::const_iterator iter = buffers.begin();
    typename MutableBufferSequence::const_iterator end = buffers.end();
    DWORD i = 0;
    for (; iter != end && i < max_buffers; ++iter, ++i)
    {
      boost::asio::mutable_buffer buffer(*iter);
      bufs[i].len = static_cast<u_long>(boost::asio::buffer_size(buffer));
      bufs[i].buf = boost::asio::buffer_cast<char*>(buffer);
    }

    // Receive some data.
    DWORD bytes_transferred = 0;
    DWORD recv_flags = flags;
    int endpoint_size = static_cast<int>(sender_endpoint.capacity());
    int result = ::WSARecvFrom(impl.socket_, bufs, i, &bytes_transferred,
        &recv_flags, sender_endpoint.data(), &endpoint_size, 0, 0);
    if (result != 0)
    {
      DWORD last_error = ::WSAGetLastError();
      if (last_error == ERROR_PORT_UNREACHABLE)
        last_error = WSAECONNREFUSED;
      ec = boost::system::error_code(last_error,
          boost::asio::error::get_system_category());
      return 0;
    }
    if (bytes_transferred == 0 && impl.protocol_.type() == SOCK_STREAM)
    {
      ec = boost::asio::error::eof;
      return 0;
    }

    sender_endpoint.resize(static_cast<std::size_t>(endpoint_size));

    ec = boost::system::error_code();
    return bytes_transferred;
  }

  // Wait until data can be received without blocking.
  size_t receive_from(implementation_type& impl,
      const null_buffers&, endpoint_type& sender_endpoint,
      socket_base::message_flags, boost::system::error_code& ec)
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
  class receive_from_operation
    : public operation
  {
  public:
    receive_from_operation(int protocol_type, win_iocp_io_service& io_service,
        endpoint_type& endpoint, const MutableBufferSequence& buffers,
        Handler handler)
      : operation(io_service,
          &receive_from_operation<
            MutableBufferSequence, Handler>::do_completion_impl,
          &receive_from_operation<
            MutableBufferSequence, Handler>::destroy_impl),
        protocol_type_(protocol_type),
        endpoint_(endpoint),
        endpoint_size_(static_cast<int>(endpoint.capacity())),
        work_(io_service.get_io_service()),
        buffers_(buffers),
        handler_(handler)
    {
    }

    int& endpoint_size()
    {
      return endpoint_size_;
    }

  private:
    static void do_completion_impl(operation* op,
        DWORD last_error, size_t bytes_transferred)
    {
      // Take ownership of the operation object.
      typedef receive_from_operation<MutableBufferSequence, Handler> op_type;
      op_type* handler_op(static_cast<op_type*>(op));
      typedef handler_alloc_traits<Handler, op_type> alloc_traits;
      handler_ptr<alloc_traits> ptr(handler_op->handler_, handler_op);

#if defined(BOOST_ASIO_ENABLE_BUFFER_DEBUGGING)
      // Check whether buffers are still valid.
      typename MutableBufferSequence::const_iterator iter
        = handler_op->buffers_.begin();
      typename MutableBufferSequence::const_iterator end
        = handler_op->buffers_.end();
      while (iter != end)
      {
        boost::asio::mutable_buffer buffer(*iter);
        boost::asio::buffer_cast<char*>(buffer);
        ++iter;
      }
#endif // defined(BOOST_ASIO_ENABLE_BUFFER_DEBUGGING)

      // Map non-portable errors to their portable counterparts.
      boost::system::error_code ec(last_error,
          boost::asio::error::get_system_category());
      if (ec.value() == ERROR_PORT_UNREACHABLE)
      {
        ec = boost::asio::error::connection_refused;
      }

      // Check for connection closed.
      if (!ec && bytes_transferred == 0
          && handler_op->protocol_type_ == SOCK_STREAM)
      {
        ec = boost::asio::error::eof;
      }

      // Record the size of the endpoint returned by the operation.
      handler_op->endpoint_.resize(handler_op->endpoint_size_);

      // Make a copy of the handler so that the memory can be deallocated before
      // the upcall is made.
      Handler handler(handler_op->handler_);

      // Free the memory associated with the handler.
      ptr.reset();

      // Call the handler.
      boost_asio_handler_invoke_helpers::invoke(
          detail::bind_handler(handler, ec, bytes_transferred), &handler);
    }

    static void destroy_impl(operation* op)
    {
      // Take ownership of the operation object.
      typedef receive_from_operation<MutableBufferSequence, Handler> op_type;
      op_type* handler_op(static_cast<op_type*>(op));
      typedef handler_alloc_traits<Handler, op_type> alloc_traits;
      handler_ptr<alloc_traits> ptr(handler_op->handler_, handler_op);

      // A sub-object of the handler may be the true owner of the memory
      // associated with the handler. Consequently, a local copy of the handler
      // is required to ensure that any owning sub-object remains valid until
      // after we have deallocated the memory here.
      Handler handler(handler_op->handler_);
      (void)handler;

      // Free the memory associated with the handler.
      ptr.reset();
    }

    int protocol_type_;
    endpoint_type& endpoint_;
    int endpoint_size_;
    boost::asio::io_service::work work_;
    MutableBufferSequence buffers_;
    Handler handler_;
  };

  // Start an asynchronous receive. The buffer for the data being received and
  // the sender_endpoint object must both be valid for the lifetime of the
  // asynchronous operation.
  template <typename MutableBufferSequence, typename Handler>
  void async_receive_from(implementation_type& impl,
      const MutableBufferSequence& buffers, endpoint_type& sender_endp,
      socket_base::message_flags flags, Handler handler)
  {
    if (!is_open(impl))
    {
      this->get_io_service().post(bind_handler(handler,
            boost::asio::error::bad_descriptor, 0));
      return;
    }

#if defined(BOOST_ASIO_ENABLE_CANCELIO)
    // Update the ID of the thread from which cancellation is safe.
    if (impl.safe_cancellation_thread_id_ == 0)
      impl.safe_cancellation_thread_id_ = ::GetCurrentThreadId();
    else if (impl.safe_cancellation_thread_id_ != ::GetCurrentThreadId())
      impl.safe_cancellation_thread_id_ = ~DWORD(0);
#endif // defined(BOOST_ASIO_ENABLE_CANCELIO)

    // Allocate and construct an operation to wrap the handler.
    typedef receive_from_operation<MutableBufferSequence, Handler> value_type;
    typedef handler_alloc_traits<Handler, value_type> alloc_traits;
    raw_handler_ptr<alloc_traits> raw_ptr(handler);
    int protocol_type = impl.protocol_.type();
    handler_ptr<alloc_traits> ptr(raw_ptr, protocol_type,
        iocp_service_, sender_endp, buffers, handler);

    // Copy buffers into WSABUF array.
    ::WSABUF bufs[max_buffers];
    typename MutableBufferSequence::const_iterator iter = buffers.begin();
    typename MutableBufferSequence::const_iterator end = buffers.end();
    DWORD i = 0;
    for (; iter != end && i < max_buffers; ++iter, ++i)
    {
      boost::asio::mutable_buffer buffer(*iter);
      bufs[i].len = static_cast<u_long>(boost::asio::buffer_size(buffer));
      bufs[i].buf = boost::asio::buffer_cast<char*>(buffer);
    }

    // Receive some data.
    DWORD bytes_transferred = 0;
    DWORD recv_flags = flags;
    int result = ::WSARecvFrom(impl.socket_, bufs, i, &bytes_transferred,
        &recv_flags, sender_endp.data(), &ptr.get()->endpoint_size(),
        ptr.get(), 0);
    DWORD last_error = ::WSAGetLastError();
    if (result != 0 && last_error != WSA_IO_PENDING)
    {
      boost::asio::io_service::work work(this->get_io_service());
      ptr.reset();
      boost::system::error_code ec(last_error,
          boost::asio::error::get_system_category());
      iocp_service_.post(bind_handler(handler, ec, bytes_transferred));
    }
    else
    {
      ptr.release();
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
      // Check if the reactor was already obtained from the io_service.
      reactor_type* reactor = static_cast<reactor_type*>(
            interlocked_compare_exchange_pointer(
              reinterpret_cast<void**>(&reactor_), 0, 0));
      if (!reactor)
      {
        reactor = &(boost::asio::use_service<reactor_type>(
              this->get_io_service()));
        interlocked_exchange_pointer(
            reinterpret_cast<void**>(&reactor_), reactor);
      }

      // Reset endpoint since it can be given no sensible value at this time.
      sender_endpoint = endpoint_type();

      if (flags & socket_base::message_out_of_band)
      {
        reactor->start_except_op(impl.socket_, impl.reactor_data_,
            null_buffers_operation<Handler>(this->get_io_service(), handler));
      }
      else
      {
        reactor->start_read_op(impl.socket_, impl.reactor_data_,
            null_buffers_operation<Handler>(this->get_io_service(), handler),
            false);
      }
    }
  }

  // Accept a new connection.
  template <typename Socket>
  boost::system::error_code accept(implementation_type& impl, Socket& peer,
      endpoint_type* peer_endpoint, boost::system::error_code& ec)
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

    for (;;)
    {
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

      if (ec)
      {
        if (ec == boost::asio::error::connection_aborted
            && !(impl.flags_ & implementation_type::enable_connection_aborted))
        {
          // Retry accept operation.
          continue;
        }
        else
        {
          return ec;
        }
      }

      if (peer_endpoint)
        peer_endpoint->resize(addr_len);

      peer.assign(impl.protocol_, new_socket.get(), ec);
      if (!ec)
        new_socket.release();
      return ec;
    }
  }

  template <typename Socket, typename Handler>
  class accept_operation
    : public operation
  {
  public:
    accept_operation(win_iocp_io_service& io_service,
        socket_type socket, socket_type new_socket, Socket& peer,
        const protocol_type& protocol, endpoint_type* peer_endpoint,
        bool enable_connection_aborted, Handler handler)
      : operation(io_service,
          &accept_operation<Socket, Handler>::do_completion_impl,
          &accept_operation<Socket, Handler>::destroy_impl),
        io_service_(io_service),
        socket_(socket),
        new_socket_(new_socket),
        peer_(peer),
        protocol_(protocol),
        peer_endpoint_(peer_endpoint),
        work_(io_service.get_io_service()),
        enable_connection_aborted_(enable_connection_aborted),
        handler_(handler)
    {
    }

    socket_type new_socket()
    {
      return new_socket_.get();
    }

    void* output_buffer()
    {
      return output_buffer_;
    }

    DWORD address_length()
    {
      return sizeof(sockaddr_storage_type) + 16;
    }

  private:
    static void do_completion_impl(operation* op, DWORD last_error, size_t)
    {
      // Take ownership of the operation object.
      typedef accept_operation<Socket, Handler> op_type;
      op_type* handler_op(static_cast<op_type*>(op));
      typedef handler_alloc_traits<Handler, op_type> alloc_traits;
      handler_ptr<alloc_traits> ptr(handler_op->handler_, handler_op);

      // Map Windows error ERROR_NETNAME_DELETED to connection_aborted.
      if (last_error == ERROR_NETNAME_DELETED)
      {
        last_error = WSAECONNABORTED;
      }

      // Restart the accept operation if we got the connection_aborted error
      // and the enable_connection_aborted socket option is not set.
      if (last_error == WSAECONNABORTED
          && !ptr.get()->enable_connection_aborted_)
      {
        // Reset OVERLAPPED structure.
        ptr.get()->Internal = 0;
        ptr.get()->InternalHigh = 0;
        ptr.get()->Offset = 0;
        ptr.get()->OffsetHigh = 0;
        ptr.get()->hEvent = 0;

        // Create a new socket for the next connection, since the AcceptEx call
        // fails with WSAEINVAL if we try to reuse the same socket.
        boost::system::error_code ec;
        ptr.get()->new_socket_.reset();
        ptr.get()->new_socket_.reset(socket_ops::socket(
              ptr.get()->protocol_.family(), ptr.get()->protocol_.type(),
              ptr.get()->protocol_.protocol(), ec));
        if (ptr.get()->new_socket() != invalid_socket)
        {
          // Accept a connection.
          DWORD bytes_read = 0;
          BOOL result = ::AcceptEx(ptr.get()->socket_, ptr.get()->new_socket(),
              ptr.get()->output_buffer(), 0, ptr.get()->address_length(),
              ptr.get()->address_length(), &bytes_read, ptr.get());
          last_error = ::WSAGetLastError();

          // Check if the operation completed immediately.
          if (!result && last_error != WSA_IO_PENDING)
          {
            if (last_error == ERROR_NETNAME_DELETED
                || last_error == WSAECONNABORTED)
            {
              // Post this handler so that operation will be restarted again.
              ptr.get()->io_service_.post_completion(ptr.get(), last_error, 0);
              ptr.release();
              return;
            }
            else
            {
              // Operation already complete. Continue with rest of this handler.
            }
          }
          else
          {
            // Asynchronous operation has been successfully restarted.
            ptr.release();
            return;
          }
        }
      }

      // Get the address of the peer.
      endpoint_type peer_endpoint;
      if (last_error == 0)
      {
        LPSOCKADDR local_addr = 0;
        int local_addr_length = 0;
        LPSOCKADDR remote_addr = 0;
        int remote_addr_length = 0;
        GetAcceptExSockaddrs(handler_op->output_buffer(), 0,
            handler_op->address_length(), handler_op->address_length(),
            &local_addr, &local_addr_length, &remote_addr, &remote_addr_length);
        if (static_cast<std::size_t>(remote_addr_length)
            > peer_endpoint.capacity())
        {
          last_error = WSAEINVAL;
        }
        else
        {
          using namespace std; // For memcpy.
          memcpy(peer_endpoint.data(), remote_addr, remote_addr_length);
          peer_endpoint.resize(static_cast<std::size_t>(remote_addr_length));
        }
      }

      // Need to set the SO_UPDATE_ACCEPT_CONTEXT option so that getsockname
      // and getpeername will work on the accepted socket.
      if (last_error == 0)
      {
        SOCKET update_ctx_param = handler_op->socket_;
        boost::system::error_code ec;
        if (socket_ops::setsockopt(handler_op->new_socket_.get(),
              SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
              &update_ctx_param, sizeof(SOCKET), ec) != 0)
        {
          last_error = ec.value();
        }
      }

      // If the socket was successfully accepted, transfer ownership of the
      // socket to the peer object.
      if (last_error == 0)
      {
        boost::system::error_code ec;
        handler_op->peer_.assign(handler_op->protocol_,
            native_type(handler_op->new_socket_.get(), peer_endpoint), ec);
        if (ec)
          last_error = ec.value();
        else
          handler_op->new_socket_.release();
      }

      // Pass endpoint back to caller.
      if (handler_op->peer_endpoint_)
        *handler_op->peer_endpoint_ = peer_endpoint;

      // Make a copy of the handler so that the memory can be deallocated before
      // the upcall is made.
      Handler handler(handler_op->handler_);

      // Free the memory associated with the handler.
      ptr.reset();

      // Call the handler.
      boost::system::error_code ec(last_error,
          boost::asio::error::get_system_category());
      boost_asio_handler_invoke_helpers::invoke(
          detail::bind_handler(handler, ec), &handler);
    }

    static void destroy_impl(operation* op)
    {
      // Take ownership of the operation object.
      typedef accept_operation<Socket, Handler> op_type;
      op_type* handler_op(static_cast<op_type*>(op));
      typedef handler_alloc_traits<Handler, op_type> alloc_traits;
      handler_ptr<alloc_traits> ptr(handler_op->handler_, handler_op);

      // A sub-object of the handler may be the true owner of the memory
      // associated with the handler. Consequently, a local copy of the handler
      // is required to ensure that any owning sub-object remains valid until
      // after we have deallocated the memory here.
      Handler handler(handler_op->handler_);
      (void)handler;

      // Free the memory associated with the handler.
      ptr.reset();
    }

    win_iocp_io_service& io_service_;
    socket_type socket_;
    socket_holder new_socket_;
    Socket& peer_;
    protocol_type protocol_;
    endpoint_type* peer_endpoint_;
    boost::asio::io_service::work work_;
    unsigned char output_buffer_[(sizeof(sockaddr_storage_type) + 16) * 2];
    bool enable_connection_aborted_;
    Handler handler_;
  };

  // Start an asynchronous accept. The peer and peer_endpoint objects
  // must be valid until the accept's handler is invoked.
  template <typename Socket, typename Handler>
  void async_accept(implementation_type& impl, Socket& peer,
      endpoint_type* peer_endpoint, Handler handler)
  {
    // Check whether acceptor has been initialised.
    if (!is_open(impl))
    {
      this->get_io_service().post(bind_handler(handler,
            boost::asio::error::bad_descriptor));
      return;
    }

    // Check that peer socket has not already been opened.
    if (peer.is_open())
    {
      this->get_io_service().post(bind_handler(handler,
            boost::asio::error::already_open));
      return;
    }

#if defined(BOOST_ASIO_ENABLE_CANCELIO)
    // Update the ID of the thread from which cancellation is safe.
    if (impl.safe_cancellation_thread_id_ == 0)
      impl.safe_cancellation_thread_id_ = ::GetCurrentThreadId();
    else if (impl.safe_cancellation_thread_id_ != ::GetCurrentThreadId())
      impl.safe_cancellation_thread_id_ = ~DWORD(0);
#endif // defined(BOOST_ASIO_ENABLE_CANCELIO)

    // Create a new socket for the connection.
    boost::system::error_code ec;
    socket_holder sock(socket_ops::socket(impl.protocol_.family(),
          impl.protocol_.type(), impl.protocol_.protocol(), ec));
    if (sock.get() == invalid_socket)
    {
      this->get_io_service().post(bind_handler(handler, ec));
      return;
    }

    // Allocate and construct an operation to wrap the handler.
    typedef accept_operation<Socket, Handler> value_type;
    typedef handler_alloc_traits<Handler, value_type> alloc_traits;
    raw_handler_ptr<alloc_traits> raw_ptr(handler);
    socket_type new_socket = sock.get();
    bool enable_connection_aborted =
      (impl.flags_ & implementation_type::enable_connection_aborted);
    handler_ptr<alloc_traits> ptr(raw_ptr,
        iocp_service_, impl.socket_, new_socket, peer, impl.protocol_,
        peer_endpoint, enable_connection_aborted, handler);
    sock.release();

    // Accept a connection.
    DWORD bytes_read = 0;
    BOOL result = ::AcceptEx(impl.socket_, ptr.get()->new_socket(),
        ptr.get()->output_buffer(), 0, ptr.get()->address_length(),
        ptr.get()->address_length(), &bytes_read, ptr.get());
    DWORD last_error = ::WSAGetLastError();

    // Check if the operation completed immediately.
    if (!result && last_error != WSA_IO_PENDING)
    {
      if (!enable_connection_aborted
          && (last_error == ERROR_NETNAME_DELETED
            || last_error == WSAECONNABORTED))
      {
        // Post handler so that operation will be restarted again. We do not
        // perform the AcceptEx again here to avoid the possibility of starving
        // other handlers.
        iocp_service_.post_completion(ptr.get(), last_error, 0);
        ptr.release();
      }
      else
      {
        boost::asio::io_service::work work(this->get_io_service());
        ptr.reset();
        boost::system::error_code ec(last_error,
            boost::asio::error::get_system_category());
        iocp_service_.post(bind_handler(handler, ec));
      }
    }
    else
    {
      ptr.release();
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
    return ec;
  }

  template <typename Handler>
  class connect_operation
  {
  public:
    connect_operation(socket_type socket, bool user_set_non_blocking,
        boost::asio::io_service& io_service, Handler handler)
      : socket_(socket),
        user_set_non_blocking_(user_set_non_blocking),
        io_service_(io_service),
        work_(io_service),
        handler_(handler)
    {
    }

    bool perform(boost::system::error_code& ec,
        std::size_t& bytes_transferred)
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

      // If connection failed then post the handler with the error code.
      if (connect_error)
      {
        ec = boost::system::error_code(connect_error,
            boost::asio::error::get_system_category());
        return true;
      }

      // Revert socket to blocking mode unless the user requested otherwise.
      if (!user_set_non_blocking_)
      {
        ioctl_arg_type non_blocking = 0;
        if (socket_ops::ioctl(socket_, FIONBIO, &non_blocking, ec))
          return true;
      }

      // Post the result of the successful connection operation.
      ec = boost::system::error_code();
      return true;
    }

    void complete(const boost::system::error_code& ec, std::size_t)
    {
      io_service_.post(bind_handler(handler_, ec));
    }

  private:
    socket_type socket_;
    bool user_set_non_blocking_;
    boost::asio::io_service& io_service_;
    boost::asio::io_service::work work_;
    Handler handler_;
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

#if defined(BOOST_ASIO_ENABLE_CANCELIO)
    // Update the ID of the thread from which cancellation is safe.
    if (impl.safe_cancellation_thread_id_ == 0)
      impl.safe_cancellation_thread_id_ = ::GetCurrentThreadId();
    else if (impl.safe_cancellation_thread_id_ != ::GetCurrentThreadId())
      impl.safe_cancellation_thread_id_ = ~DWORD(0);
#endif // defined(BOOST_ASIO_ENABLE_CANCELIO)

    // Check if the reactor was already obtained from the io_service.
    reactor_type* reactor = static_cast<reactor_type*>(
          interlocked_compare_exchange_pointer(
            reinterpret_cast<void**>(&reactor_), 0, 0));
    if (!reactor)
    {
      reactor = &(boost::asio::use_service<reactor_type>(
            this->get_io_service()));
      interlocked_exchange_pointer(
          reinterpret_cast<void**>(&reactor_), reactor);
    }

    // Mark the socket as non-blocking so that the connection will take place
    // asynchronously.
    ioctl_arg_type non_blocking = 1;
    boost::system::error_code ec;
    if (socket_ops::ioctl(impl.socket_, FIONBIO, &non_blocking, ec))
    {
      this->get_io_service().post(bind_handler(handler, ec));
      return;
    }

    // Start the connect operation.
    if (socket_ops::connect(impl.socket_, peer_endpoint.data(),
          peer_endpoint.size(), ec) == 0)
    {
      // Revert socket to blocking mode unless the user requested otherwise.
      if (!(impl.flags_ & implementation_type::user_set_non_blocking))
      {
        non_blocking = 0;
        socket_ops::ioctl(impl.socket_, FIONBIO, &non_blocking, ec);
      }

      // The connect operation has finished successfully so we need to post the
      // handler immediately.
      this->get_io_service().post(bind_handler(handler, ec));
    }
    else if (ec == boost::asio::error::in_progress
        || ec == boost::asio::error::would_block)
    {
      // The connection is happening in the background, and we need to wait
      // until the socket becomes writeable.
      boost::shared_ptr<bool> completed(new bool(false));
      reactor->start_connect_op(impl.socket_, impl.reactor_data_,
          connect_operation<Handler>(
            impl.socket_,
            (impl.flags_ & implementation_type::user_set_non_blocking) != 0,
            this->get_io_service(), handler));
    }
    else
    {
      // Revert socket to blocking mode unless the user requested otherwise.
      if (!(impl.flags_ & implementation_type::user_set_non_blocking))
      {
        non_blocking = 0;
        boost::system::error_code ignored_ec;
        socket_ops::ioctl(impl.socket_, FIONBIO, &non_blocking, ignored_ec);
      }

      // The connect operation has failed, so post the handler immediately.
      this->get_io_service().post(bind_handler(handler, ec));
    }
  }

private:
  // Helper function to close a socket when the associated object is being
  // destroyed.
  void close_for_destruction(implementation_type& impl)
  {
    if (is_open(impl))
    {
      // Check if the reactor was created, in which case we need to close the
      // socket on the reactor as well to cancel any operations that might be
      // running there.
      reactor_type* reactor = static_cast<reactor_type*>(
            interlocked_compare_exchange_pointer(
              reinterpret_cast<void**>(&reactor_), 0, 0));
      if (reactor)
        reactor->close_descriptor(impl.socket_, impl.reactor_data_);

      // The socket destructor must not block. If the user has changed the
      // linger option to block in the foreground, we will change it back to the
      // default so that the closure is performed in the background.
      if (impl.flags_ & implementation_type::close_might_block)
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
      impl.flags_ = 0;
      impl.cancel_token_.reset();
#if defined(BOOST_ASIO_ENABLE_CANCELIO)
      impl.safe_cancellation_thread_id_ = 0;
#endif // defined(BOOST_ASIO_ENABLE_CANCELIO)
    }
  }

  // Helper function to emulate InterlockedCompareExchangePointer functionality
  // for:
  // - very old Platform SDKs; and
  // - platform SDKs where MSVC's /Wp64 option causes spurious warnings.
  void* interlocked_compare_exchange_pointer(void** dest, void* exch, void* cmp)
  {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(InterlockedCompareExchange(
          reinterpret_cast<PLONG>(dest), reinterpret_cast<LONG>(exch),
          reinterpret_cast<LONG>(cmp)));
#else
    return InterlockedCompareExchangePointer(dest, exch, cmp);
#endif
  }

  // Helper function to emulate InterlockedExchangePointer functionality for:
  // - very old Platform SDKs; and
  // - platform SDKs where MSVC's /Wp64 option causes spurious warnings.
  void* interlocked_exchange_pointer(void** dest, void* val)
  {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(InterlockedExchange(
          reinterpret_cast<PLONG>(dest), reinterpret_cast<LONG>(val)));
#else
    return InterlockedExchangePointer(dest, val);
#endif
  }

  // The IOCP service used for running asynchronous operations and dispatching
  // handlers.
  win_iocp_io_service& iocp_service_;

  // The reactor used for performing connect operations. This object is created
  // only if needed.
  reactor_type* reactor_;

  // Mutex to protect access to the linked list of implementations. 
  boost::asio::detail::mutex mutex_;

  // The head of a linked list of all implementations.
  implementation_type* impl_list_;
};

} // namespace detail
} // namespace asio
} // namespace boost

#endif // defined(BOOST_ASIO_HAS_IOCP)

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_WIN_IOCP_SOCKET_SERVICE_HPP
