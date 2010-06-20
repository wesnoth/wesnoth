//
// win_iocp_io_service.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_WIN_IOCP_IO_SERVICE_HPP
#define BOOST_ASIO_DETAIL_WIN_IOCP_IO_SERVICE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/win_iocp_io_service_fwd.hpp>

#if defined(BOOST_ASIO_HAS_IOCP)

#include <boost/asio/detail/push_options.hpp>
#include <limits>
#include <boost/throw_exception.hpp>
#include <boost/system/system_error.hpp>
#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/asio/detail/call_stack.hpp>
#include <boost/asio/detail/handler_alloc_helpers.hpp>
#include <boost/asio/detail/handler_invoke_helpers.hpp>
#include <boost/asio/detail/service_base.hpp>
#include <boost/asio/detail/socket_types.hpp>
#include <boost/asio/detail/timer_queue.hpp>
#include <boost/asio/detail/mutex.hpp>

namespace boost {
namespace asio {
namespace detail {

class win_iocp_io_service
  : public boost::asio::detail::service_base<win_iocp_io_service>
{
public:
  // Base class for all operations. A function pointer is used instead of
  // virtual functions to avoid the associated overhead.
  //
  // This class inherits from OVERLAPPED so that we can downcast to get back to
  // the operation pointer from the LPOVERLAPPED out parameter of
  // GetQueuedCompletionStatus.
  class operation
    : public OVERLAPPED
  {
  public:
    typedef void (*invoke_func_type)(operation*, DWORD, size_t);
    typedef void (*destroy_func_type)(operation*);

    operation(win_iocp_io_service& iocp_service,
        invoke_func_type invoke_func, destroy_func_type destroy_func)
      : outstanding_operations_(&iocp_service.outstanding_operations_),
        invoke_func_(invoke_func),
        destroy_func_(destroy_func)
    {
      Internal = 0;
      InternalHigh = 0;
      Offset = 0;
      OffsetHigh = 0;
      hEvent = 0;

      ::InterlockedIncrement(outstanding_operations_);
    }

    void do_completion(DWORD last_error, size_t bytes_transferred)
    {
      invoke_func_(this, last_error, bytes_transferred);
    }

    void destroy()
    {
      destroy_func_(this);
    }

  protected:
    // Prevent deletion through this type.
    ~operation()
    {
      ::InterlockedDecrement(outstanding_operations_);
    }

  private:
    long* outstanding_operations_;
    invoke_func_type invoke_func_;
    destroy_func_type destroy_func_;
  };


  // Constructor.
  win_iocp_io_service(boost::asio::io_service& io_service)
    : boost::asio::detail::service_base<win_iocp_io_service>(io_service),
      iocp_(),
      outstanding_work_(0),
      outstanding_operations_(0),
      stopped_(0),
      shutdown_(0),
      timer_thread_(0),
      timer_interrupt_issued_(false)
  {
  }

  void init(size_t concurrency_hint)
  {
    iocp_.handle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0,
        static_cast<DWORD>((std::min<size_t>)(concurrency_hint, DWORD(~0))));
    if (!iocp_.handle)
    {
      DWORD last_error = ::GetLastError();
      boost::system::system_error e(
          boost::system::error_code(last_error,
            boost::asio::error::get_system_category()),
          "iocp");
      boost::throw_exception(e);
    }
  }

  // Destroy all user-defined handler objects owned by the service.
  void shutdown_service()
  {
    ::InterlockedExchange(&shutdown_, 1);

    while (::InterlockedExchangeAdd(&outstanding_operations_, 0) > 0)
    {
      DWORD bytes_transferred = 0;
#if (WINVER < 0x0500)
      DWORD completion_key = 0;
#else
      DWORD_PTR completion_key = 0;
#endif
      LPOVERLAPPED overlapped = 0;
      ::GetQueuedCompletionStatus(iocp_.handle, &bytes_transferred,
          &completion_key, &overlapped, INFINITE);
      if (overlapped)
        static_cast<operation*>(overlapped)->destroy();
    }

    for (std::size_t i = 0; i < timer_queues_.size(); ++i)
      timer_queues_[i]->destroy_timers();
    timer_queues_.clear();
  }

  // Initialise the task. Nothing to do here.
  void init_task()
  {
  }

  // Register a handle with the IO completion port.
  boost::system::error_code register_handle(
      HANDLE handle, boost::system::error_code& ec)
  {
    if (::CreateIoCompletionPort(handle, iocp_.handle, 0, 0) == 0)
    {
      DWORD last_error = ::GetLastError();
      ec = boost::system::error_code(last_error,
          boost::asio::error::get_system_category());
    }
    else
    {
      ec = boost::system::error_code();
    }
    return ec;
  }

  // Run the event loop until stopped or no more work.
  size_t run(boost::system::error_code& ec)
  {
    if (::InterlockedExchangeAdd(&outstanding_work_, 0) == 0)
    {
      ec = boost::system::error_code();
      return 0;
    }

    call_stack<win_iocp_io_service>::context ctx(this);

    size_t n = 0;
    while (do_one(true, ec))
      if (n != (std::numeric_limits<size_t>::max)())
        ++n;
    return n;
  }

  // Run until stopped or one operation is performed.
  size_t run_one(boost::system::error_code& ec)
  {
    if (::InterlockedExchangeAdd(&outstanding_work_, 0) == 0)
    {
      ec = boost::system::error_code();
      return 0;
    }

    call_stack<win_iocp_io_service>::context ctx(this);

    return do_one(true, ec);
  }

  // Poll for operations without blocking.
  size_t poll(boost::system::error_code& ec)
  {
    if (::InterlockedExchangeAdd(&outstanding_work_, 0) == 0)
    {
      ec = boost::system::error_code();
      return 0;
    }

    call_stack<win_iocp_io_service>::context ctx(this);

    size_t n = 0;
    while (do_one(false, ec))
      if (n != (std::numeric_limits<size_t>::max)())
        ++n;
    return n;
  }

  // Poll for one operation without blocking.
  size_t poll_one(boost::system::error_code& ec)
  {
    if (::InterlockedExchangeAdd(&outstanding_work_, 0) == 0)
    {
      ec = boost::system::error_code();
      return 0;
    }

    call_stack<win_iocp_io_service>::context ctx(this);

    return do_one(false, ec);
  }

  // Stop the event processing loop.
  void stop()
  {
    if (::InterlockedExchange(&stopped_, 1) == 0)
    {
      if (!::PostQueuedCompletionStatus(iocp_.handle, 0, 0, 0))
      {
        DWORD last_error = ::GetLastError();
        boost::system::system_error e(
            boost::system::error_code(last_error,
              boost::asio::error::get_system_category()),
            "pqcs");
        boost::throw_exception(e);
      }
    }
  }

  // Reset in preparation for a subsequent run invocation.
  void reset()
  {
    ::InterlockedExchange(&stopped_, 0);
  }

  // Notify that some work has started.
  void work_started()
  {
    ::InterlockedIncrement(&outstanding_work_);
  }

  // Notify that some work has finished.
  void work_finished()
  {
    if (::InterlockedDecrement(&outstanding_work_) == 0)
      stop();
  }

  // Request invocation of the given handler.
  template <typename Handler>
  void dispatch(Handler handler)
  {
    if (call_stack<win_iocp_io_service>::contains(this))
      boost_asio_handler_invoke_helpers::invoke(handler, &handler);
    else
      post(handler);
  }

  // Request invocation of the given handler and return immediately.
  template <typename Handler>
  void post(Handler handler)
  {
    // If the service has been shut down we silently discard the handler.
    if (::InterlockedExchangeAdd(&shutdown_, 0) != 0)
      return;

    // Allocate and construct an operation to wrap the handler.
    typedef handler_operation<Handler> value_type;
    typedef handler_alloc_traits<Handler, value_type> alloc_traits;
    raw_handler_ptr<alloc_traits> raw_ptr(handler);
    handler_ptr<alloc_traits> ptr(raw_ptr, *this, handler);

    // Enqueue the operation on the I/O completion port.
    if (!::PostQueuedCompletionStatus(iocp_.handle, 0, 0, ptr.get()))
    {
      DWORD last_error = ::GetLastError();
      boost::system::system_error e(
          boost::system::error_code(last_error,
            boost::asio::error::get_system_category()),
          "pqcs");
      boost::throw_exception(e);
    }

    // Operation has been successfully posted.
    ptr.release();
  }

  // Request invocation of the given OVERLAPPED-derived operation.
  void post_completion(operation* op, DWORD op_last_error,
      DWORD bytes_transferred)
  {
    // Enqueue the operation on the I/O completion port.
    if (!::PostQueuedCompletionStatus(iocp_.handle,
          bytes_transferred, op_last_error, op))
    {
      DWORD last_error = ::GetLastError();
      boost::system::system_error e(
          boost::system::error_code(last_error,
            boost::asio::error::get_system_category()),
          "pqcs");
      boost::throw_exception(e);
    }
  }

  // Add a new timer queue to the service.
  template <typename Time_Traits>
  void add_timer_queue(timer_queue<Time_Traits>& timer_queue)
  {
    boost::asio::detail::mutex::scoped_lock lock(timer_mutex_);
    timer_queues_.push_back(&timer_queue);
  }

  // Remove a timer queue from the service.
  template <typename Time_Traits>
  void remove_timer_queue(timer_queue<Time_Traits>& timer_queue)
  {
    boost::asio::detail::mutex::scoped_lock lock(timer_mutex_);
    for (std::size_t i = 0; i < timer_queues_.size(); ++i)
    {
      if (timer_queues_[i] == &timer_queue)
      {
        timer_queues_.erase(timer_queues_.begin() + i);
        return;
      }
    }
  }

  // Schedule a timer in the given timer queue to expire at the specified
  // absolute time. The handler object will be invoked when the timer expires.
  template <typename Time_Traits, typename Handler>
  void schedule_timer(timer_queue<Time_Traits>& timer_queue,
      const typename Time_Traits::time_type& time, Handler handler, void* token)
  {
    // If the service has been shut down we silently discard the timer.
    if (::InterlockedExchangeAdd(&shutdown_, 0) != 0)
      return;

    boost::asio::detail::mutex::scoped_lock lock(timer_mutex_);
    if (timer_queue.enqueue_timer(time, handler, token))
    {
      if (!timer_interrupt_issued_)
      {
        timer_interrupt_issued_ = true;
        lock.unlock();
        ::PostQueuedCompletionStatus(iocp_.handle,
            0, steal_timer_dispatching, 0);
      }
    }
  }

  // Cancel the timer associated with the given token. Returns the number of
  // handlers that have been posted or dispatched.
  template <typename Time_Traits>
  std::size_t cancel_timer(timer_queue<Time_Traits>& timer_queue, void* token)
  {
    // If the service has been shut down we silently ignore the cancellation.
    if (::InterlockedExchangeAdd(&shutdown_, 0) != 0)
      return 0;

    boost::asio::detail::mutex::scoped_lock lock(timer_mutex_);
    std::size_t n = timer_queue.cancel_timer(token);
    if (n > 0 && !timer_interrupt_issued_)
    {
      timer_interrupt_issued_ = true;
      lock.unlock();
      ::PostQueuedCompletionStatus(iocp_.handle,
          0, steal_timer_dispatching, 0);
    }
    return n;
  }

private:
  // Dequeues at most one operation from the I/O completion port, and then
  // executes it. Returns the number of operations that were dequeued (i.e.
  // either 0 or 1).
  size_t do_one(bool block, boost::system::error_code& ec)
  {
    long this_thread_id = static_cast<long>(::GetCurrentThreadId());

    for (;;)
    {
      // Try to acquire responsibility for dispatching timers.
      bool dispatching_timers = (::InterlockedCompareExchange(
            &timer_thread_, this_thread_id, 0) == 0);

      // Calculate timeout for GetQueuedCompletionStatus call.
      DWORD timeout = max_timeout;
      if (dispatching_timers)
      {
        boost::asio::detail::mutex::scoped_lock lock(timer_mutex_);
        timer_interrupt_issued_ = false;
        timeout = get_timeout();
      }

      // Get the next operation from the queue.
      DWORD bytes_transferred = 0;
#if (WINVER < 0x0500)
      DWORD completion_key = 0;
#else
      DWORD_PTR completion_key = 0;
#endif
      LPOVERLAPPED overlapped = 0;
      ::SetLastError(0);
      BOOL ok = ::GetQueuedCompletionStatus(iocp_.handle, &bytes_transferred,
          &completion_key, &overlapped, block ? timeout : 0);
      DWORD last_error = ::GetLastError();

      // Dispatch any pending timers.
      if (dispatching_timers)
      {
        try
        {
          boost::asio::detail::mutex::scoped_lock lock(timer_mutex_);
          timer_queues_copy_ = timer_queues_;
          for (std::size_t i = 0; i < timer_queues_copy_.size(); ++i)
          {
            timer_queues_copy_[i]->dispatch_timers();
            timer_queues_copy_[i]->dispatch_cancellations();
            timer_queues_copy_[i]->complete_timers();
          }
        }
        catch (...)
        {
          // Transfer responsibility for dispatching timers to another thread.
          if (::InterlockedCompareExchange(&timer_thread_,
                0, this_thread_id) == this_thread_id)
          {
            ::PostQueuedCompletionStatus(iocp_.handle,
                0, transfer_timer_dispatching, 0);
          }

          throw;
        }
      }

      if (!ok && overlapped == 0)
      {
        if (block && last_error == WAIT_TIMEOUT)
        {
          // Relinquish responsibility for dispatching timers.
          if (dispatching_timers)
          {
            ::InterlockedCompareExchange(&timer_thread_, 0, this_thread_id);
          }

          continue;
        }

        // Transfer responsibility for dispatching timers to another thread.
        if (dispatching_timers && ::InterlockedCompareExchange(
              &timer_thread_, 0, this_thread_id) == this_thread_id)
        {
          ::PostQueuedCompletionStatus(iocp_.handle,
              0, transfer_timer_dispatching, 0);
        }

        ec = boost::system::error_code();
        return 0;
      }
      else if (overlapped)
      {
        // We may have been passed a last_error value in the completion_key.
        if (last_error == 0)
        {
          last_error = completion_key;
        }

        // Transfer responsibility for dispatching timers to another thread.
        if (dispatching_timers && ::InterlockedCompareExchange(
              &timer_thread_, 0, this_thread_id) == this_thread_id)
        {
          ::PostQueuedCompletionStatus(iocp_.handle,
              0, transfer_timer_dispatching, 0);
        }

        // Ensure that the io_service does not exit due to running out of work
        // while we make the upcall.
        auto_work work(*this);

        // Dispatch the operation.
        operation* op = static_cast<operation*>(overlapped);
        op->do_completion(last_error, bytes_transferred);

        ec = boost::system::error_code();
        return 1;
      }
      else if (completion_key == transfer_timer_dispatching)
      {
        // Woken up to try to acquire responsibility for dispatching timers.
        ::InterlockedCompareExchange(&timer_thread_, 0, this_thread_id);
      }
      else if (completion_key == steal_timer_dispatching)
      {
        // Woken up to steal responsibility for dispatching timers.
        ::InterlockedExchange(&timer_thread_, 0);
      }
      else
      {
        // Relinquish responsibility for dispatching timers. If the io_service
        // is not being stopped then the thread will get an opportunity to
        // reacquire timer responsibility on the next loop iteration.
        if (dispatching_timers)
        {
          ::InterlockedCompareExchange(&timer_thread_, 0, this_thread_id);
        }

        // The stopped_ flag is always checked to ensure that any leftover
        // interrupts from a previous run invocation are ignored.
        if (::InterlockedExchangeAdd(&stopped_, 0) != 0)
        {
          // Wake up next thread that is blocked on GetQueuedCompletionStatus.
          if (!::PostQueuedCompletionStatus(iocp_.handle, 0, 0, 0))
          {
            last_error = ::GetLastError();
            ec = boost::system::error_code(last_error,
                boost::asio::error::get_system_category());
            return 0;
          }

          ec = boost::system::error_code();
          return 0;
        }
      }
    }
  }

  // Check if all timer queues are empty.
  bool all_timer_queues_are_empty() const
  {
    for (std::size_t i = 0; i < timer_queues_.size(); ++i)
      if (!timer_queues_[i]->empty())
        return false;
    return true;
  }

  // Get the timeout value for the GetQueuedCompletionStatus call. The timeout
  // value is returned as a number of milliseconds. We will wait no longer than
  // 1000 milliseconds.
  DWORD get_timeout()
  {
    if (all_timer_queues_are_empty())
      return max_timeout;

    boost::posix_time::time_duration minimum_wait_duration
      = boost::posix_time::milliseconds(max_timeout);

    for (std::size_t i = 0; i < timer_queues_.size(); ++i)
    {
      boost::posix_time::time_duration wait_duration
        = timer_queues_[i]->wait_duration();
      if (wait_duration < minimum_wait_duration)
        minimum_wait_duration = wait_duration;
    }

    if (minimum_wait_duration > boost::posix_time::time_duration())
    {
      int milliseconds = minimum_wait_duration.total_milliseconds();
      return static_cast<DWORD>(milliseconds > 0 ? milliseconds : 1);
    }
    else
    {
      return 0;
    }
  }

  struct auto_work
  {
    auto_work(win_iocp_io_service& io_service)
      : io_service_(io_service)
    {
      io_service_.work_started();
    }

    ~auto_work()
    {
      io_service_.work_finished();
    }

  private:
    win_iocp_io_service& io_service_;
  };

  template <typename Handler>
  struct handler_operation
    : public operation
  {
    handler_operation(win_iocp_io_service& io_service,
        Handler handler)
      : operation(io_service, &handler_operation<Handler>::do_completion_impl,
          &handler_operation<Handler>::destroy_impl),
        io_service_(io_service),
        handler_(handler)
    {
      io_service_.work_started();
    }

    ~handler_operation()
    {
      io_service_.work_finished();
    }

  private:
    // Prevent copying and assignment.
    handler_operation(const handler_operation&);
    void operator=(const handler_operation&);
    
    static void do_completion_impl(operation* op, DWORD, size_t)
    {
      // Take ownership of the operation object.
      typedef handler_operation<Handler> op_type;
      op_type* handler_op(static_cast<op_type*>(op));
      typedef handler_alloc_traits<Handler, op_type> alloc_traits;
      handler_ptr<alloc_traits> ptr(handler_op->handler_, handler_op);

      // Make a copy of the handler so that the memory can be deallocated before
      // the upcall is made.
      Handler handler(handler_op->handler_);

      // Free the memory associated with the handler.
      ptr.reset();

      // Make the upcall.
      boost_asio_handler_invoke_helpers::invoke(handler, &handler);
    }

    static void destroy_impl(operation* op)
    {
      // Take ownership of the operation object.
      typedef handler_operation<Handler> op_type;
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
    Handler handler_;
  };

  // The IO completion port used for queueing operations.
  struct iocp_holder
  {
    HANDLE handle;
    iocp_holder() : handle(0) {}
    ~iocp_holder() { if (handle) ::CloseHandle(handle); }
  } iocp_;

  // The count of unfinished work.
  long outstanding_work_;

  // The count of unfinished operations.
  long outstanding_operations_;
  friend class operation;

  // Flag to indicate whether the event loop has been stopped.
  long stopped_;

  // Flag to indicate whether the service has been shut down.
  long shutdown_;

  enum
  {
    // Maximum GetQueuedCompletionStatus timeout, in milliseconds.
    max_timeout = 500,

    // Completion key value to indicate that responsibility for dispatching
    // timers is being cooperatively transferred from one thread to another.
    transfer_timer_dispatching = 1,

    // Completion key value to indicate that responsibility for dispatching
    // timers should be stolen from another thread.
    steal_timer_dispatching = 2
  };

  // The thread that's currently in charge of dispatching timers.
  long timer_thread_;

  // Mutex for protecting access to the timer queues.
  mutex timer_mutex_;

  // Whether a thread has been interrupted to process a new timeout.
  bool timer_interrupt_issued_;

  // The timer queues.
  std::vector<timer_queue_base*> timer_queues_;

  // A copy of the timer queues, used when dispatching, cancelling and cleaning
  // up timers. The copy is stored as a class data member to avoid unnecessary
  // memory allocation.
  std::vector<timer_queue_base*> timer_queues_copy_;
};

} // namespace detail
} // namespace asio
} // namespace boost

#endif // defined(BOOST_ASIO_HAS_IOCP)

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_WIN_IOCP_IO_SERVICE_HPP
