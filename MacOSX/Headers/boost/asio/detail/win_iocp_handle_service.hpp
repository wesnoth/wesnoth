//
// win_iocp_handle_service.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Copyright (c) 2008 Rep Invariant Systems, Inc. (info@repinvariant.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_WIN_IOCP_HANDLE_SERVICE_HPP
#define BOOST_ASIO_DETAIL_WIN_IOCP_HANDLE_SERVICE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/win_iocp_io_service_fwd.hpp>

#if defined(BOOST_ASIO_HAS_IOCP)

#include <boost/asio/detail/push_options.hpp>
#include <boost/cstdint.hpp>
#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/buffer.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/detail/bind_handler.hpp>
#include <boost/asio/detail/handler_alloc_helpers.hpp>
#include <boost/asio/detail/handler_invoke_helpers.hpp>
#include <boost/asio/detail/mutex.hpp>
#include <boost/asio/detail/win_iocp_io_service.hpp>

namespace boost {
namespace asio {
namespace detail {

class win_iocp_handle_service
  : public boost::asio::detail::service_base<win_iocp_handle_service>
{
public:
  // Base class for all operations.
  typedef win_iocp_io_service::operation operation;

  // The native type of a stream handle.
  typedef HANDLE native_type;

  // The implementation type of the stream handle.
  class implementation_type
  {
  public:
    // Default constructor.
    implementation_type()
      : handle_(INVALID_HANDLE_VALUE),
        safe_cancellation_thread_id_(0),
        next_(0),
        prev_(0)
    {
    }

  private:
    // Only this service will have access to the internal values.
    friend class win_iocp_handle_service;

    // The native stream handle representation.
    native_type handle_;

    // The ID of the thread from which it is safe to cancel asynchronous
    // operations. 0 means no asynchronous operations have been started yet.
    // ~0 means asynchronous operations have been started from more than one
    // thread, and cancellation is not supported for the handle.
    DWORD safe_cancellation_thread_id_;

    // Pointers to adjacent handle implementations in linked list.
    implementation_type* next_;
    implementation_type* prev_;
  };

  win_iocp_handle_service(boost::asio::io_service& io_service)
    : boost::asio::detail::service_base<win_iocp_handle_service>(io_service),
      iocp_service_(boost::asio::use_service<win_iocp_io_service>(io_service)),
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
      close_for_destruction(*impl);
      impl = impl->next_;
    }
  }

  // Construct a new handle implementation.
  void construct(implementation_type& impl)
  {
    impl.handle_ = INVALID_HANDLE_VALUE;
    impl.safe_cancellation_thread_id_ = 0;

    // Insert implementation into linked list of all implementations.
    boost::asio::detail::mutex::scoped_lock lock(mutex_);
    impl.next_ = impl_list_;
    impl.prev_ = 0;
    if (impl_list_)
      impl_list_->prev_ = &impl;
    impl_list_ = &impl;
  }

  // Destroy a handle implementation.
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

  // Assign a native handle to a handle implementation.
  boost::system::error_code assign(implementation_type& impl,
      const native_type& native_handle, boost::system::error_code& ec)
  {
    if (is_open(impl))
    {
      ec = boost::asio::error::already_open;
      return ec;
    }

    if (iocp_service_.register_handle(native_handle, ec))
      return ec;

    impl.handle_ = native_handle;
    ec = boost::system::error_code();
    return ec;
  }

  // Determine whether the handle is open.
  bool is_open(const implementation_type& impl) const
  {
    return impl.handle_ != INVALID_HANDLE_VALUE;
  }

  // Destroy a handle implementation.
  boost::system::error_code close(implementation_type& impl,
      boost::system::error_code& ec)
  {
    if (is_open(impl))
    {
      if (!::CloseHandle(impl.handle_))
      {
        DWORD last_error = ::GetLastError();
        ec = boost::system::error_code(last_error,
            boost::asio::error::get_system_category());
        return ec;
      }

      impl.handle_ = INVALID_HANDLE_VALUE;
      impl.safe_cancellation_thread_id_ = 0;
    }

    ec = boost::system::error_code();
    return ec;
  }

  // Get the native handle representation.
  native_type native(const implementation_type& impl) const
  {
    return impl.handle_;
  }

  // Cancel all operations associated with the handle.
  boost::system::error_code cancel(implementation_type& impl,
      boost::system::error_code& ec)
  {
    if (!is_open(impl))
    {
      ec = boost::asio::error::bad_descriptor;
    }
    else if (FARPROC cancel_io_ex_ptr = ::GetProcAddress(
          ::GetModuleHandleA("KERNEL32"), "CancelIoEx"))
    {
      // The version of Windows supports cancellation from any thread.
      typedef BOOL (WINAPI* cancel_io_ex_t)(HANDLE, LPOVERLAPPED);
      cancel_io_ex_t cancel_io_ex = (cancel_io_ex_t)cancel_io_ex_ptr;
      if (!cancel_io_ex(impl.handle_, 0))
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
    else if (impl.safe_cancellation_thread_id_ == 0)
    {
      // No operations have been started, so there's nothing to cancel.
      ec = boost::system::error_code();
    }
    else if (impl.safe_cancellation_thread_id_ == ::GetCurrentThreadId())
    {
      // Asynchronous operations have been started from the current thread only,
      // so it is safe to try to cancel them using CancelIo.
      if (!::CancelIo(impl.handle_))
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

    return ec;
  }

  class overlapped_wrapper
    : public OVERLAPPED
  {
  public:
    explicit overlapped_wrapper(boost::system::error_code& ec)
    {
      Internal = 0;
      InternalHigh = 0;
      Offset = 0;
      OffsetHigh = 0;

      // Create a non-signalled manual-reset event, for GetOverlappedResult.
      hEvent = ::CreateEvent(0, TRUE, FALSE, 0);
      if (hEvent)
      {
        // As documented in GetQueuedCompletionStatus, setting the low order
        // bit of this event prevents our synchronous writes from being treated
        // as completion port events.
        *reinterpret_cast<DWORD_PTR*>(&hEvent) |= 1;
      }
      else
      {
        DWORD last_error = ::GetLastError();
        ec = boost::system::error_code(last_error,
            boost::asio::error::get_system_category());
      }
    }

    ~overlapped_wrapper()
    {
      if (hEvent)
      {
        ::CloseHandle(hEvent);
      }
    }
  };

  // Write the given data. Returns the number of bytes written.
  template <typename ConstBufferSequence>
  size_t write_some(implementation_type& impl,
      const ConstBufferSequence& buffers, boost::system::error_code& ec)
  {
    return write_some_at(impl, 0, buffers, ec);
  }

  // Write the given data at the specified offset. Returns the number of bytes
  // written.
  template <typename ConstBufferSequence>
  size_t write_some_at(implementation_type& impl, boost::uint64_t offset,
      const ConstBufferSequence& buffers, boost::system::error_code& ec)
  {
    if (!is_open(impl))
    {
      ec = boost::asio::error::bad_descriptor;
      return 0;
    }

    // Find first buffer of non-zero length.
    boost::asio::const_buffer buffer;
    typename ConstBufferSequence::const_iterator iter = buffers.begin();
    typename ConstBufferSequence::const_iterator end = buffers.end();
    for (DWORD i = 0; iter != end; ++iter, ++i)
    {
      buffer = boost::asio::const_buffer(*iter);
      if (boost::asio::buffer_size(buffer) != 0)
        break;
    }

    // A request to write 0 bytes on a handle is a no-op.
    if (boost::asio::buffer_size(buffer) == 0)
    {
      ec = boost::system::error_code();
      return 0;
    }

    overlapped_wrapper overlapped(ec);
    if (ec)
    {
      return 0;
    }

    // Write the data. 
    overlapped.Offset = offset & 0xFFFFFFFF;
    overlapped.OffsetHigh = (offset >> 32) & 0xFFFFFFFF;
    BOOL ok = ::WriteFile(impl.handle_,
        boost::asio::buffer_cast<LPCVOID>(buffer),
        static_cast<DWORD>(boost::asio::buffer_size(buffer)), 0, &overlapped);
    if (!ok) 
    {
      DWORD last_error = ::GetLastError();
      if (last_error != ERROR_IO_PENDING)
      {
        ec = boost::system::error_code(last_error,
            boost::asio::error::get_system_category());
        return 0;
      }
    }

    // Wait for the operation to complete.
    DWORD bytes_transferred = 0;
    ok = ::GetOverlappedResult(impl.handle_,
        &overlapped, &bytes_transferred, TRUE);
    if (!ok)
    {
      DWORD last_error = ::GetLastError();
      ec = boost::system::error_code(last_error,
          boost::asio::error::get_system_category());
    }

    ec = boost::system::error_code();
    return bytes_transferred;
  }

  template <typename ConstBufferSequence, typename Handler>
  class write_operation
    : public operation
  {
  public:
    write_operation(win_iocp_io_service& io_service,
        const ConstBufferSequence& buffers, Handler handler)
      : operation(io_service,
          &write_operation<ConstBufferSequence, Handler>::do_completion_impl,
          &write_operation<ConstBufferSequence, Handler>::destroy_impl),
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
      typedef write_operation<ConstBufferSequence, Handler> op_type;
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

      // Make a copy of the handler so that the memory can be deallocated before
      // the upcall is made.
      Handler handler(handler_op->handler_);

      // Free the memory associated with the handler.
      ptr.reset();

      // Call the handler.
      boost::system::error_code ec(last_error,
          boost::asio::error::get_system_category());
      boost_asio_handler_invoke_helpers::invoke(
          bind_handler(handler, ec, bytes_transferred), &handler);
    }

    static void destroy_impl(operation* op)
    {
      // Take ownership of the operation object.
      typedef write_operation<ConstBufferSequence, Handler> op_type;
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

  // Start an asynchronous write. The data being written must be valid for the
  // lifetime of the asynchronous operation.
  template <typename ConstBufferSequence, typename Handler>
  void async_write_some(implementation_type& impl,
      const ConstBufferSequence& buffers, Handler handler)
  {
    async_write_some_at(impl, 0, buffers, handler);
  }

  // Start an asynchronous write at a specified offset. The data being written
  // must be valid for the lifetime of the asynchronous operation.
  template <typename ConstBufferSequence, typename Handler>
  void async_write_some_at(implementation_type& impl, boost::uint64_t offset,
      const ConstBufferSequence& buffers, Handler handler)
  {
    if (!is_open(impl))
    {
      this->get_io_service().post(bind_handler(handler,
            boost::asio::error::bad_descriptor, 0));
      return;
    }

    // Update the ID of the thread from which cancellation is safe.
    if (impl.safe_cancellation_thread_id_ == 0)
      impl.safe_cancellation_thread_id_ = ::GetCurrentThreadId();
    else if (impl.safe_cancellation_thread_id_ != ::GetCurrentThreadId())
      impl.safe_cancellation_thread_id_ = ~DWORD(0);

    // Allocate and construct an operation to wrap the handler.
    typedef write_operation<ConstBufferSequence, Handler> value_type;
    typedef handler_alloc_traits<Handler, value_type> alloc_traits;
    raw_handler_ptr<alloc_traits> raw_ptr(handler);
    handler_ptr<alloc_traits> ptr(raw_ptr, iocp_service_, buffers, handler);

    // Find first buffer of non-zero length.
    boost::asio::const_buffer buffer;
    typename ConstBufferSequence::const_iterator iter = buffers.begin();
    typename ConstBufferSequence::const_iterator end = buffers.end();
    for (DWORD i = 0; iter != end; ++iter, ++i)
    {
      buffer = boost::asio::const_buffer(*iter);
      if (boost::asio::buffer_size(buffer) != 0)
        break;
    }

    // A request to write 0 bytes on a handle is a no-op.
    if (boost::asio::buffer_size(buffer) == 0)
    {
      boost::asio::io_service::work work(this->get_io_service());
      ptr.reset();
      boost::system::error_code error;
      iocp_service_.post(bind_handler(handler, error, 0));
      return;
    }

    // Write the data.
    DWORD bytes_transferred = 0;
    ptr.get()->Offset = offset & 0xFFFFFFFF;
    ptr.get()->OffsetHigh = (offset >> 32) & 0xFFFFFFFF;
    BOOL ok = ::WriteFile(impl.handle_,
        boost::asio::buffer_cast<LPCVOID>(buffer),
        static_cast<DWORD>(boost::asio::buffer_size(buffer)),
        &bytes_transferred, ptr.get());
    DWORD last_error = ::GetLastError();

    // Check if the operation completed immediately.
    if (!ok && last_error != ERROR_IO_PENDING)
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

  // Read some data. Returns the number of bytes received.
  template <typename MutableBufferSequence>
  size_t read_some(implementation_type& impl,
      const MutableBufferSequence& buffers, boost::system::error_code& ec)
  {
    return read_some_at(impl, 0, buffers, ec);
  }

  // Read some data at a specified offset. Returns the number of bytes received.
  template <typename MutableBufferSequence>
  size_t read_some_at(implementation_type& impl, boost::uint64_t offset,
      const MutableBufferSequence& buffers, boost::system::error_code& ec)
  {
    if (!is_open(impl))
    {
      ec = boost::asio::error::bad_descriptor;
      return 0;
    }
    
    // Find first buffer of non-zero length.
    boost::asio::mutable_buffer buffer;
    typename MutableBufferSequence::const_iterator iter = buffers.begin();
    typename MutableBufferSequence::const_iterator end = buffers.end();
    for (DWORD i = 0; iter != end; ++iter, ++i)
    {
      buffer = boost::asio::mutable_buffer(*iter);
      if (boost::asio::buffer_size(buffer) != 0)
        break;
    }

    // A request to read 0 bytes on a stream handle is a no-op.
    if (boost::asio::buffer_size(buffer) == 0)
    {
      ec = boost::system::error_code();
      return 0;
    }

    overlapped_wrapper overlapped(ec);
    if (ec)
    {
      return 0;
    }

    // Read some data.
    overlapped.Offset = offset & 0xFFFFFFFF;
    overlapped.OffsetHigh = (offset >> 32) & 0xFFFFFFFF;
    BOOL ok = ::ReadFile(impl.handle_,
        boost::asio::buffer_cast<LPVOID>(buffer),
        static_cast<DWORD>(boost::asio::buffer_size(buffer)), 0, &overlapped);
    if (!ok) 
    {
      DWORD last_error = ::GetLastError();
      if (last_error != ERROR_IO_PENDING)
      {
        if (last_error == ERROR_HANDLE_EOF)
        {
          ec = boost::asio::error::eof;
        }
        else
        {
          ec = boost::system::error_code(last_error,
              boost::asio::error::get_system_category());
        }
        return 0;
      }
    }

    // Wait for the operation to complete.
    DWORD bytes_transferred = 0;
    ok = ::GetOverlappedResult(impl.handle_,
        &overlapped, &bytes_transferred, TRUE);
    if (!ok)
    {
      DWORD last_error = ::GetLastError();
      if (last_error == ERROR_HANDLE_EOF)
      {
        ec = boost::asio::error::eof;
      }
      else
      {
        ec = boost::system::error_code(last_error,
            boost::asio::error::get_system_category());
      }
    }

    ec = boost::system::error_code();
    return bytes_transferred;
  }

  template <typename MutableBufferSequence, typename Handler>
  class read_operation
    : public operation
  {
  public:
    read_operation(win_iocp_io_service& io_service,
        const MutableBufferSequence& buffers, Handler handler)
      : operation(io_service,
          &read_operation<
            MutableBufferSequence, Handler>::do_completion_impl,
          &read_operation<
            MutableBufferSequence, Handler>::destroy_impl),
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
      typedef read_operation<MutableBufferSequence, Handler> op_type;
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

      // Check for the end-of-file condition.
      boost::system::error_code ec(last_error,
          boost::asio::error::get_system_category());
      if (!ec && bytes_transferred == 0 || last_error == ERROR_HANDLE_EOF)
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
        bind_handler(handler, ec, bytes_transferred), &handler);
    }

    static void destroy_impl(operation* op)
    {
      // Take ownership of the operation object.
      typedef read_operation<MutableBufferSequence, Handler> op_type;
      op_type* handler_op(static_cast<op_type*>(op));
      typedef boost::asio::detail::handler_alloc_traits<
        Handler, op_type> alloc_traits;
      boost::asio::detail::handler_ptr<alloc_traits> ptr(
        handler_op->handler_, handler_op);

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
    MutableBufferSequence buffers_;
    Handler handler_;
  };

  // Start an asynchronous read. The buffer for the data being received must be
  // valid for the lifetime of the asynchronous operation.
  template <typename MutableBufferSequence, typename Handler>
  void async_read_some(implementation_type& impl,
      const MutableBufferSequence& buffers, Handler handler)
  {
    async_read_some_at(impl, 0, buffers, handler);
  }

  // Start an asynchronous read at a specified offset. The buffer for the data
  // being received must be valid for the lifetime of the asynchronous
  // operation.
  template <typename MutableBufferSequence, typename Handler>
  void async_read_some_at(implementation_type& impl, boost::uint64_t offset,
      const MutableBufferSequence& buffers, Handler handler)
  {
    if (!is_open(impl))
    {
      this->get_io_service().post(bind_handler(handler,
            boost::asio::error::bad_descriptor, 0));
      return;
    }

    // Update the ID of the thread from which cancellation is safe.
    if (impl.safe_cancellation_thread_id_ == 0)
      impl.safe_cancellation_thread_id_ = ::GetCurrentThreadId();
    else if (impl.safe_cancellation_thread_id_ != ::GetCurrentThreadId())
      impl.safe_cancellation_thread_id_ = ~DWORD(0);

    // Allocate and construct an operation to wrap the handler.
    typedef read_operation<MutableBufferSequence, Handler> value_type;
    typedef handler_alloc_traits<Handler, value_type> alloc_traits;
    raw_handler_ptr<alloc_traits> raw_ptr(handler);
    handler_ptr<alloc_traits> ptr(raw_ptr, iocp_service_, buffers, handler);

    // Find first buffer of non-zero length.
    boost::asio::mutable_buffer buffer;
    typename MutableBufferSequence::const_iterator iter = buffers.begin();
    typename MutableBufferSequence::const_iterator end = buffers.end();
    for (DWORD i = 0; iter != end; ++iter, ++i)
    {
      buffer = boost::asio::mutable_buffer(*iter);
      if (boost::asio::buffer_size(buffer) != 0)
        break;
    }

    // A request to receive 0 bytes on a stream handle is a no-op.
    if (boost::asio::buffer_size(buffer) == 0)
    {
      boost::asio::io_service::work work(this->get_io_service());
      ptr.reset();
      boost::system::error_code error;
      iocp_service_.post(bind_handler(handler, error, 0));
      return;
    }

    // Read some data.
    DWORD bytes_transferred = 0;
    ptr.get()->Offset = offset & 0xFFFFFFFF;
    ptr.get()->OffsetHigh = (offset >> 32) & 0xFFFFFFFF;
    BOOL ok = ::ReadFile(impl.handle_,
        boost::asio::buffer_cast<LPVOID>(buffer),
        static_cast<DWORD>(boost::asio::buffer_size(buffer)),
        &bytes_transferred, ptr.get());
    DWORD last_error = ::GetLastError();
    if (!ok && last_error != ERROR_IO_PENDING)
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

private:
  // Prevent the use of the null_buffers type with this service.
  size_t write_some(implementation_type& impl,
      const null_buffers& buffers, boost::system::error_code& ec);
  size_t write_some_at(implementation_type& impl, boost::uint64_t offset,
      const null_buffers& buffers, boost::system::error_code& ec);
  template <typename Handler>
  void async_write_some(implementation_type& impl,
      const null_buffers& buffers, Handler handler);
  template <typename Handler>
  void async_write_some_at(implementation_type& impl, boost::uint64_t offset,
      const null_buffers& buffers, Handler handler);
  size_t read_some(implementation_type& impl,
      const null_buffers& buffers, boost::system::error_code& ec);
  size_t read_some_at(implementation_type& impl, boost::uint64_t offset,
      const null_buffers& buffers, boost::system::error_code& ec);
  template <typename Handler>
  void async_read_some(implementation_type& impl,
      const null_buffers& buffers, Handler handler);
  template <typename Handler>
  void async_read_some_at(implementation_type& impl, boost::uint64_t offset,
      const null_buffers& buffers, Handler handler);

  // Helper function to close a handle when the associated object is being
  // destroyed.
  void close_for_destruction(implementation_type& impl)
  {
    if (is_open(impl))
    {
      ::CloseHandle(impl.handle_);
      impl.handle_ = INVALID_HANDLE_VALUE;
      impl.safe_cancellation_thread_id_ = 0;
    }
  }

  // The IOCP service used for running asynchronous operations and dispatching
  // handlers.
  win_iocp_io_service& iocp_service_;

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

#endif // BOOST_ASIO_DETAIL_WIN_IOCP_HANDLE_SERVICE_HPP
