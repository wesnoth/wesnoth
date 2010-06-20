//
// win_iocp_overlapped_ptr.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_WIN_IOCP_OVERLAPPED_PTR_HPP
#define BOOST_ASIO_DETAIL_WIN_IOCP_OVERLAPPED_PTR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/win_iocp_io_service_fwd.hpp>

#if defined(BOOST_ASIO_HAS_IOCP)

#include <boost/asio/detail/noncopyable.hpp>
#include <boost/asio/detail/win_iocp_io_service.hpp>

namespace boost {
namespace asio {
namespace detail {

// Wraps a handler to create an OVERLAPPED object for use with overlapped I/O.
class win_iocp_overlapped_ptr
  : private noncopyable
{
public:
  // Construct an empty win_iocp_overlapped_ptr.
  win_iocp_overlapped_ptr()
    : ptr_(0)
  {
  }

  // Construct an win_iocp_overlapped_ptr to contain the specified handler.
  template <typename Handler>
  explicit win_iocp_overlapped_ptr(
      boost::asio::io_service& io_service, Handler handler)
    : ptr_(0)
  {
    this->reset(io_service, handler);
  }

  // Destructor automatically frees the OVERLAPPED object unless released.
  ~win_iocp_overlapped_ptr()
  {
    reset();
  }

  // Reset to empty.
  void reset()
  {
    if (ptr_)
    {
      ptr_->destroy();
      ptr_ = 0;
    }
  }

  // Reset to contain the specified handler, freeing any current OVERLAPPED
  // object.
  template <typename Handler>
  void reset(boost::asio::io_service& io_service, Handler handler)
  {
    typedef overlapped_operation<Handler> value_type;
    typedef handler_alloc_traits<Handler, value_type> alloc_traits;
    raw_handler_ptr<alloc_traits> raw_ptr(handler);
    handler_ptr<alloc_traits> ptr(raw_ptr, io_service.impl_, handler);
    reset();
    ptr_ = ptr.release();
  }

  // Get the contained OVERLAPPED object.
  OVERLAPPED* get()
  {
    return ptr_;
  }

  // Get the contained OVERLAPPED object.
  const OVERLAPPED* get() const
  {
    return ptr_;
  }

  // Release ownership of the OVERLAPPED object.
  OVERLAPPED* release()
  {
    OVERLAPPED* tmp = ptr_;
    ptr_ = 0;
    return tmp;
  }

  // Post completion notification for overlapped operation. Releases ownership.
  void complete(const boost::system::error_code& ec,
      std::size_t bytes_transferred)
  {
    if (ptr_)
    {
      ptr_->io_service_.post_completion(ptr_, 0, 0);
      ptr_ = 0;
    }
  }

private:
  struct overlapped_operation_base
    : public win_iocp_io_service::operation
  {
    overlapped_operation_base(win_iocp_io_service& io_service,
        invoke_func_type invoke_func, destroy_func_type destroy_func)
      : win_iocp_io_service::operation(io_service, invoke_func, destroy_func),
        io_service_(io_service)
    {
      io_service_.work_started();
    }

    ~overlapped_operation_base()
    {
      io_service_.work_finished();
    }

    win_iocp_io_service& io_service_;
    boost::system::error_code ec_;
  };

  template <typename Handler>
  struct overlapped_operation
    : public overlapped_operation_base
  {
    overlapped_operation(win_iocp_io_service& io_service,
        Handler handler)
      : overlapped_operation_base(io_service,
          &overlapped_operation<Handler>::do_completion_impl,
          &overlapped_operation<Handler>::destroy_impl),
        handler_(handler)
    {
    }

  private:
    // Prevent copying and assignment.
    overlapped_operation(const overlapped_operation&);
    void operator=(const overlapped_operation&);
    
    static void do_completion_impl(win_iocp_io_service::operation* op,
        DWORD last_error, size_t bytes_transferred)
    {
      // Take ownership of the operation object.
      typedef overlapped_operation<Handler> op_type;
      op_type* handler_op(static_cast<op_type*>(op));
      typedef handler_alloc_traits<Handler, op_type> alloc_traits;
      handler_ptr<alloc_traits> ptr(handler_op->handler_, handler_op);

      // Make a copy of the handler and error_code so that the memory can be
      // deallocated before the upcall is made.
      Handler handler(handler_op->handler_);
      boost::system::error_code ec(handler_op->ec_);
      if (last_error)
        ec = boost::system::error_code(last_error,
            boost::asio::error::get_system_category());

      // Free the memory associated with the handler.
      ptr.reset();

      // Make the upcall.
      boost_asio_handler_invoke_helpers::invoke(
          bind_handler(handler, ec, bytes_transferred), &handler);
    }

    static void destroy_impl(win_iocp_io_service::operation* op)
    {
      // Take ownership of the operation object.
      typedef overlapped_operation<Handler> op_type;
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

    Handler handler_;
  };

  overlapped_operation_base* ptr_;
};

} // namespace detail
} // namespace asio
} // namespace boost

#endif // defined(BOOST_ASIO_HAS_IOCP)

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_WIN_IOCP_OVERLAPPED_PTR_HPP
