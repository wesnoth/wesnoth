//
// task_io_service_2lock.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_TASK_IO_SERVICE_2LOCK_HPP
#define BOOST_ASIO_DETAIL_TASK_IO_SERVICE_2LOCK_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/asio/detail/call_stack.hpp>
#include <boost/asio/detail/event.hpp>
#include <boost/asio/detail/handler_alloc_helpers.hpp>
#include <boost/asio/detail/handler_invoke_helpers.hpp>
#include <boost/asio/detail/indirect_handler_queue.hpp>
#include <boost/asio/detail/mutex.hpp>
#include <boost/asio/detail/service_base.hpp>
#include <boost/asio/detail/task_io_service_fwd.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <boost/detail/atomic_count.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio/detail/pop_options.hpp>

namespace boost {
namespace asio {
namespace detail {

// An alternative task_io_service implementation based on a two-lock queue.

template <typename Task>
class task_io_service
  : public boost::asio::detail::service_base<task_io_service<Task> >
{
public:
  typedef indirect_handler_queue handler_queue;

  // Constructor.
  task_io_service(boost::asio::io_service& io_service)
    : boost::asio::detail::service_base<task_io_service<Task> >(io_service),
      front_mutex_(),
      back_mutex_(),
      task_(&use_service<Task>(io_service)),
      outstanding_work_(0),
      front_stopped_(false),
      back_stopped_(false),
      back_shutdown_(false),
      back_first_idle_thread_(0),
      back_task_thread_(0)
  {
  }

  void init(size_t /*concurrency_hint*/)
  {
  }

  // Destroy all user-defined handler objects owned by the service.
  void shutdown_service()
  {
    boost::asio::detail::mutex::scoped_lock back_lock(back_mutex_);
    back_shutdown_ = true;
    back_lock.unlock();

    // Destroy handler objects.
    while (handler_queue::handler* h = handler_queue_.pop())
      if (h != &task_handler_)
        h->destroy();

    // Reset to initial state.
    task_ = 0;
  }

  // Initialise the task, if required.
  void init_task()
  {
    boost::asio::detail::mutex::scoped_lock back_lock(back_mutex_);
    if (!back_shutdown_ && !task_)
    {
      task_ = &use_service<Task>(this->get_io_service());
      handler_queue_.push(&task_handler_);
      interrupt_one_idle_thread(back_lock);
    }
  }

  // Run the event loop until interrupted or no more work.
  size_t run(boost::system::error_code& ec)
  {
    if (outstanding_work_ == 0)
    {
      stop();
      ec = boost::system::error_code();
      return 0;
    }

    typename call_stack<task_io_service>::context ctx(this);

    idle_thread_info this_idle_thread;
    this_idle_thread.next = 0;

    size_t n = 0;
    while (do_one(&this_idle_thread, ec))
      if (n != (std::numeric_limits<size_t>::max)())
        ++n;
    return n;
  }

  // Run until interrupted or one operation is performed.
  size_t run_one(boost::system::error_code& ec)
  {
    if (outstanding_work_ == 0)
    {
      stop();
      ec = boost::system::error_code();
      return 0;
    }

    typename call_stack<task_io_service>::context ctx(this);

    idle_thread_info this_idle_thread;
    this_idle_thread.next = 0;

    return do_one(&this_idle_thread, ec);
  }

  // Poll for operations without blocking.
  size_t poll(boost::system::error_code& ec)
  {
    if (outstanding_work_ == 0)
    {
      stop();
      ec = boost::system::error_code();
      return 0;
    }

    typename call_stack<task_io_service>::context ctx(this);

    size_t n = 0;
    while (do_one(0, ec))
      if (n != (std::numeric_limits<size_t>::max)())
        ++n;
    return n;
  }

  // Poll for one operation without blocking.
  size_t poll_one(boost::system::error_code& ec)
  {
    if (outstanding_work_ == 0)
    {
      stop();
      ec = boost::system::error_code();
      return 0;
    }

    typename call_stack<task_io_service>::context ctx(this);

    return do_one(0, ec);
  }

  // Interrupt the event processing loop.
  void stop()
  {
    boost::asio::detail::mutex::scoped_lock front_lock(front_mutex_);
    front_stopped_ = true;
    front_lock.unlock();

    boost::asio::detail::mutex::scoped_lock back_lock(back_mutex_);
    back_stopped_ = true;
    interrupt_all_idle_threads(back_lock);
  }

  // Reset in preparation for a subsequent run invocation.
  void reset()
  {
    boost::asio::detail::mutex::scoped_lock front_lock(front_mutex_);
    front_stopped_ = false;
    front_lock.unlock();

    boost::asio::detail::mutex::scoped_lock back_lock(back_mutex_);
    back_stopped_ = false;
  }

  // Notify that some work has started.
  void work_started()
  {
    ++outstanding_work_;
  }

  // Notify that some work has finished.
  void work_finished()
  {
    if (--outstanding_work_ == 0)
      stop();
  }

  // Request invocation of the given handler.
  template <typename Handler>
  void dispatch(Handler handler)
  {
    if (call_stack<task_io_service>::contains(this))
      boost_asio_handler_invoke_helpers::invoke(handler, &handler);
    else
      post(handler);
  }

  // Request invocation of the given handler and return immediately.
  template <typename Handler>
  void post(Handler handler)
  {
    // Allocate and construct an operation to wrap the handler.
    handler_queue::scoped_ptr ptr(handler_queue::wrap(handler));

    boost::asio::detail::mutex::scoped_lock back_lock(back_mutex_);

    // If the service has been shut down we silently discard the handler.
    if (back_shutdown_)
      return;

    // Add the handler to the end of the queue.
    handler_queue_.push(ptr.get());
    ptr.release();

    // An undelivered handler is treated as unfinished work.
    ++outstanding_work_;

    // Wake up a thread to execute the handler.
    interrupt_one_idle_thread(back_lock);
  }

private:
  struct idle_thread_info;

  size_t do_one(idle_thread_info* this_idle_thread,
      boost::system::error_code& ec)
  {
    bool task_has_run = false;
    for (;;)
    {
      // The front lock must be held before we can pop items from the queue.
      boost::asio::detail::mutex::scoped_lock front_lock(front_mutex_);
      if (front_stopped_)
      {
        ec = boost::system::error_code();
        return 0;
      }

      if (handler_queue::handler* h = handler_queue_.pop())
      {
        if (h == &task_handler_)
        {
          bool more_handlers = handler_queue_.poppable();
          unsigned long front_version = handler_queue_.front_version();
          front_lock.unlock();

          // The task is always added to the back of the queue when we exit
          // this block.
          task_cleanup c(*this);

          // If we're polling and the task has already run then we're done.
          bool polling = !this_idle_thread;
          if (task_has_run && polling)
          {
            ec = boost::system::error_code();
            return 0;
          }

          // If we're considering going idle we need to check whether the queue
          // is still empty. If it is, add the thread to the list of idle
          // threads.
          if (!more_handlers && !polling)
          {
            boost::asio::detail::mutex::scoped_lock back_lock(back_mutex_);
            if (back_stopped_)
            {
              ec = boost::system::error_code();
              return 0;
            }
            else if (front_version == handler_queue_.back_version())
            {
              back_task_thread_ = this_idle_thread;
            }
            else
            {
              more_handlers = true;
            }
          }

          // Run the task. May throw an exception. Only block if the handler
          // queue is empty and we're not polling, otherwise we want to return
          // as soon as possible.
          task_has_run = true;
          task_->run(!more_handlers && !polling);
        }
        else
        {
          front_lock.unlock();
          handler_cleanup c(*this);

          // Invoke the handler. May throw an exception.
          h->invoke(); // invoke() deletes the handler object

          ec = boost::system::error_code();
          return 1;
        }
      }
      else if (this_idle_thread)
      {
        unsigned long front_version = handler_queue_.front_version();
        front_lock.unlock();

        // If we're considering going idle we need to check whether the queue
        // is still empty. If it is, add the thread to the list of idle
        // threads.
        boost::asio::detail::mutex::scoped_lock back_lock(back_mutex_);
        if (back_stopped_)
        {
          ec = boost::system::error_code();
          return 0;
        }
        else if (front_version == handler_queue_.back_version())
        {
          this_idle_thread->next = back_first_idle_thread_;
          back_first_idle_thread_ = this_idle_thread;
          this_idle_thread->wakeup_event.clear(back_lock);
          this_idle_thread->wakeup_event.wait(back_lock);
        }
      }
      else
      {
        ec = boost::system::error_code();
        return 0;
      }
    }
  }

  // Interrupt a single idle thread.
  void interrupt_one_idle_thread(
      boost::asio::detail::mutex::scoped_lock& back_lock)
  {
    if (back_first_idle_thread_)
    {
      idle_thread_info* idle_thread = back_first_idle_thread_;
      back_first_idle_thread_ = idle_thread->next;
      idle_thread->next = 0;
      idle_thread->wakeup_event.signal(back_lock);
    }
    else if (back_task_thread_ && task_)
    {
      back_task_thread_ = 0;
      task_->interrupt();
    }
  }

  // Interrupt all idle threads.
  void interrupt_all_idle_threads(
      boost::asio::detail::mutex::scoped_lock& back_lock)
  {
    while (back_first_idle_thread_)
    {
      idle_thread_info* idle_thread = back_first_idle_thread_;
      back_first_idle_thread_ = idle_thread->next;
      idle_thread->next = 0;
      idle_thread->wakeup_event.signal(back_lock);
    }

    if (back_task_thread_ && task_)
    {
      back_task_thread_ = 0;
      task_->interrupt();
    }
  }

  // Helper class to perform task-related operations on block exit.
  class task_cleanup;
  friend class task_cleanup;
  class task_cleanup
  {
  public:
    task_cleanup(task_io_service& task_io_svc)
      : task_io_service_(task_io_svc)
    {
    }

    ~task_cleanup()
    {
      // Reinsert the task at the end of the handler queue.
      boost::asio::detail::mutex::scoped_lock back_lock(
          task_io_service_.back_mutex_);
      task_io_service_.back_task_thread_ = 0;
      task_io_service_.handler_queue_.push(&task_io_service_.task_handler_);
    }

  private:
    task_io_service& task_io_service_;
  };

  // Helper class to perform handler-related operations on block exit.
  class handler_cleanup
  {
  public:
    handler_cleanup(task_io_service& task_io_svc)
      : task_io_service_(task_io_svc)
    {
    }

    ~handler_cleanup()
    {
      task_io_service_.work_finished();
    }

  private:
    task_io_service& task_io_service_;
  };

  // Mutexes to protect access to internal data.
  boost::asio::detail::mutex front_mutex_;
  boost::asio::detail::mutex back_mutex_;

  // The task to be run by this service.
  Task* task_;

  // Handler object to represent the position of the task in the queue.
  class task_handler
    : public handler_queue::handler
  {
  public:
    task_handler()
      : handler_queue::handler(0, 0)
    {
    }
  } task_handler_;

  // The count of unfinished work.
  boost::detail::atomic_count outstanding_work_;

  // The queue of handlers that are ready to be delivered.
  handler_queue handler_queue_;

  // Flag to indicate that the dispatcher has been stopped.
  bool front_stopped_;
  bool back_stopped_;

  // Flag to indicate that the dispatcher has been shut down.
  bool back_shutdown_;

  // Structure containing information about an idle thread.
  struct idle_thread_info
  {
    event wakeup_event;
    idle_thread_info* next;
  };

  // The number of threads that are currently idle.
  idle_thread_info* back_first_idle_thread_;

  // The thread that is currently blocked on the task.
  idle_thread_info* back_task_thread_;
};

} // namespace detail
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_TASK_IO_SERVICE_2LOCK_HPP
