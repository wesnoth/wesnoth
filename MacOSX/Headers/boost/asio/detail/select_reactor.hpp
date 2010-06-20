//
// select_reactor.hpp
// ~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_SELECT_REACTOR_HPP
#define BOOST_ASIO_DETAIL_SELECT_REACTOR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/socket_types.hpp> // Must come before posix_time.

#include <boost/asio/detail/push_options.hpp>
#include <cstddef>
#include <boost/config.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/asio/detail/bind_handler.hpp>
#include <boost/asio/detail/fd_set_adapter.hpp>
#include <boost/asio/detail/mutex.hpp>
#include <boost/asio/detail/noncopyable.hpp>
#include <boost/asio/detail/reactor_op_queue.hpp>
#include <boost/asio/detail/select_interrupter.hpp>
#include <boost/asio/detail/select_reactor_fwd.hpp>
#include <boost/asio/detail/service_base.hpp>
#include <boost/asio/detail/signal_blocker.hpp>
#include <boost/asio/detail/socket_ops.hpp>
#include <boost/asio/detail/socket_types.hpp>
#include <boost/asio/detail/task_io_service.hpp>
#include <boost/asio/detail/thread.hpp>
#include <boost/asio/detail/timer_queue.hpp>

namespace boost {
namespace asio {
namespace detail {

template <bool Own_Thread>
class select_reactor
  : public boost::asio::detail::service_base<select_reactor<Own_Thread> >
{
public:
  // Per-descriptor data.
  struct per_descriptor_data
  {
  };

  // Constructor.
  select_reactor(boost::asio::io_service& io_service)
    : boost::asio::detail::service_base<
        select_reactor<Own_Thread> >(io_service),
      mutex_(),
      select_in_progress_(false),
      interrupter_(),
      read_op_queue_(),
      write_op_queue_(),
      except_op_queue_(),
      pending_cancellations_(),
      stop_thread_(false),
      thread_(0),
      shutdown_(false)
  {
    if (Own_Thread)
    {
      boost::asio::detail::signal_blocker sb;
      thread_ = new boost::asio::detail::thread(
          bind_handler(&select_reactor::call_run_thread, this));
    }
  }

  // Destructor.
  ~select_reactor()
  {
    shutdown_service();
  }

  // Destroy all user-defined handler objects owned by the service.
  void shutdown_service()
  {
    boost::asio::detail::mutex::scoped_lock lock(mutex_);
    shutdown_ = true;
    stop_thread_ = true;
    lock.unlock();

    if (thread_)
    {
      interrupter_.interrupt();
      thread_->join();
      delete thread_;
      thread_ = 0;
    }

    read_op_queue_.destroy_operations();
    write_op_queue_.destroy_operations();
    except_op_queue_.destroy_operations();

    for (std::size_t i = 0; i < timer_queues_.size(); ++i)
      timer_queues_[i]->destroy_timers();
    timer_queues_.clear();
  }

  // Initialise the task, but only if the reactor is not in its own thread.
  void init_task()
  {
    if (!Own_Thread)
    {
      typedef task_io_service<select_reactor<Own_Thread> > task_io_service_type;
      use_service<task_io_service_type>(this->get_io_service()).init_task();
    }
  }

  // Register a socket with the reactor. Returns 0 on success, system error
  // code on failure.
  int register_descriptor(socket_type, per_descriptor_data&)
  {
    return 0;
  }

  // Start a new read operation. The handler object will be invoked when the
  // given descriptor is ready to be read, or an error has occurred.
  template <typename Handler>
  void start_read_op(socket_type descriptor, per_descriptor_data&,
      Handler handler, bool /*allow_speculative_read*/ = true)
  {
    boost::asio::detail::mutex::scoped_lock lock(mutex_);
    if (!shutdown_)
      if (read_op_queue_.enqueue_operation(descriptor, handler))
        interrupter_.interrupt();
  }

  // Start a new write operation. The handler object will be invoked when the
  // given descriptor is ready to be written, or an error has occurred.
  template <typename Handler>
  void start_write_op(socket_type descriptor, per_descriptor_data&,
      Handler handler, bool /*allow_speculative_write*/ = true)
  {
    boost::asio::detail::mutex::scoped_lock lock(mutex_);
    if (!shutdown_)
      if (write_op_queue_.enqueue_operation(descriptor, handler))
        interrupter_.interrupt();
  }

  // Start a new exception operation. The handler object will be invoked when
  // the given descriptor has exception information, or an error has occurred.
  template <typename Handler>
  void start_except_op(socket_type descriptor,
      per_descriptor_data&, Handler handler)
  {
    boost::asio::detail::mutex::scoped_lock lock(mutex_);
    if (!shutdown_)
      if (except_op_queue_.enqueue_operation(descriptor, handler))
        interrupter_.interrupt();
  }

  // Wrapper for connect handlers to enable the handler object to be placed
  // in both the write and the except operation queues, but ensure that only
  // one of the handlers is called.
  template <typename Handler>
  class connect_handler_wrapper
  {
  public:
    connect_handler_wrapper(socket_type descriptor,
        boost::shared_ptr<bool> completed,
        select_reactor<Own_Thread>& reactor, Handler handler)
      : descriptor_(descriptor),
        completed_(completed),
        reactor_(reactor),
        handler_(handler)
    {
    }

    bool perform(boost::system::error_code& ec,
        std::size_t& bytes_transferred)
    {
      // Check whether one of the handlers has already been called. If it has,
      // then we don't want to do anything in this handler.
      if (*completed_)
      {
        completed_.reset(); // Indicate that this handler should not complete.
        return true;
      }

      // Cancel the other reactor operation for the connection.
      *completed_ = true;
      reactor_.enqueue_cancel_ops_unlocked(descriptor_);

      // Call the contained handler.
      return handler_.perform(ec, bytes_transferred);
    }

    void complete(const boost::system::error_code& ec,
        std::size_t bytes_transferred)
    {
      if (completed_.get())
        handler_.complete(ec, bytes_transferred);
    }

  private:
    socket_type descriptor_;
    boost::shared_ptr<bool> completed_;
    select_reactor<Own_Thread>& reactor_;
    Handler handler_;
  };

  // Start new write and exception operations. The handler object will be
  // invoked when the given descriptor is ready for writing or has exception
  // information available, or an error has occurred. The handler will be called
  // only once.
  template <typename Handler>
  void start_connect_op(socket_type descriptor,
      per_descriptor_data&, Handler handler)
  {
    boost::asio::detail::mutex::scoped_lock lock(mutex_);
    if (!shutdown_)
    {
      boost::shared_ptr<bool> completed(new bool(false));
      connect_handler_wrapper<Handler> wrapped_handler(
          descriptor, completed, *this, handler);
      bool interrupt = write_op_queue_.enqueue_operation(
          descriptor, wrapped_handler);
      interrupt = except_op_queue_.enqueue_operation(
          descriptor, wrapped_handler) || interrupt;
      if (interrupt)
        interrupter_.interrupt();
    }
  }

  // Cancel all operations associated with the given descriptor. The
  // handlers associated with the descriptor will be invoked with the
  // operation_aborted error.
  void cancel_ops(socket_type descriptor, per_descriptor_data&)
  {
    boost::asio::detail::mutex::scoped_lock lock(mutex_);
    cancel_ops_unlocked(descriptor);
  }

  // Enqueue cancellation of all operations associated with the given
  // descriptor. The handlers associated with the descriptor will be invoked
  // with the operation_aborted error. This function does not acquire the
  // select_reactor's mutex, and so should only be used when the reactor lock is
  // already held.
  void enqueue_cancel_ops_unlocked(socket_type descriptor)
  {
    pending_cancellations_.push_back(descriptor);
  }

  // Cancel any operations that are running against the descriptor and remove
  // its registration from the reactor.
  void close_descriptor(socket_type descriptor, per_descriptor_data&)
  {
    boost::asio::detail::mutex::scoped_lock lock(mutex_);
    cancel_ops_unlocked(descriptor);
  }

  // Add a new timer queue to the reactor.
  template <typename Time_Traits>
  void add_timer_queue(timer_queue<Time_Traits>& timer_queue)
  {
    boost::asio::detail::mutex::scoped_lock lock(mutex_);
    timer_queues_.push_back(&timer_queue);
  }

  // Remove a timer queue from the reactor.
  template <typename Time_Traits>
  void remove_timer_queue(timer_queue<Time_Traits>& timer_queue)
  {
    boost::asio::detail::mutex::scoped_lock lock(mutex_);
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
    boost::asio::detail::mutex::scoped_lock lock(mutex_);
    if (!shutdown_)
      if (timer_queue.enqueue_timer(time, handler, token))
        interrupter_.interrupt();
  }

  // Cancel the timer associated with the given token. Returns the number of
  // handlers that have been posted or dispatched.
  template <typename Time_Traits>
  std::size_t cancel_timer(timer_queue<Time_Traits>& timer_queue, void* token)
  {
    boost::asio::detail::mutex::scoped_lock lock(mutex_);
    std::size_t n = timer_queue.cancel_timer(token);
    if (n > 0)
      interrupter_.interrupt();
    return n;
  }

private:
  friend class task_io_service<select_reactor<Own_Thread> >;

  // Run select once until interrupted or events are ready to be dispatched.
  void run(bool block)
  {
    boost::asio::detail::mutex::scoped_lock lock(mutex_);

    // Dispatch any operation cancellations that were made while the select
    // loop was not running.
    read_op_queue_.perform_cancellations();
    write_op_queue_.perform_cancellations();
    except_op_queue_.perform_cancellations();
    for (std::size_t i = 0; i < timer_queues_.size(); ++i)
      timer_queues_[i]->dispatch_cancellations();

    // Check if the thread is supposed to stop.
    if (stop_thread_)
    {
      complete_operations_and_timers(lock);
      return;
    }

    // We can return immediately if there's no work to do and the reactor is
    // not supposed to block.
    if (!block && read_op_queue_.empty() && write_op_queue_.empty()
        && except_op_queue_.empty() && all_timer_queues_are_empty())
    {
      complete_operations_and_timers(lock);
      return;
    }

    // Set up the descriptor sets.
    fd_set_adapter read_fds;
    read_fds.set(interrupter_.read_descriptor());
    read_op_queue_.get_descriptors(read_fds);
    fd_set_adapter write_fds;
    write_op_queue_.get_descriptors(write_fds);
    fd_set_adapter except_fds;
    except_op_queue_.get_descriptors(except_fds);
    socket_type max_fd = read_fds.max_descriptor();
    if (write_fds.max_descriptor() > max_fd)
      max_fd = write_fds.max_descriptor();
    if (except_fds.max_descriptor() > max_fd)
      max_fd = except_fds.max_descriptor();

    // Block on the select call without holding the lock so that new
    // operations can be started while the call is executing.
    timeval tv_buf = { 0, 0 };
    timeval* tv = block ? get_timeout(tv_buf) : &tv_buf;
    select_in_progress_ = true;
    lock.unlock();
    boost::system::error_code ec;
    int retval = socket_ops::select(static_cast<int>(max_fd + 1),
        read_fds, write_fds, except_fds, tv, ec);
    lock.lock();
    select_in_progress_ = false;

    // Block signals while dispatching operations.
    boost::asio::detail::signal_blocker sb;

    // Reset the interrupter.
    if (retval > 0 && read_fds.is_set(interrupter_.read_descriptor()))
      interrupter_.reset();

    // Dispatch all ready operations.
    if (retval > 0)
    {
      // Exception operations must be processed first to ensure that any
      // out-of-band data is read before normal data.
      except_op_queue_.perform_operations_for_descriptors(
          except_fds, boost::system::error_code());
      read_op_queue_.perform_operations_for_descriptors(
          read_fds, boost::system::error_code());
      write_op_queue_.perform_operations_for_descriptors(
          write_fds, boost::system::error_code());
      except_op_queue_.perform_cancellations();
      read_op_queue_.perform_cancellations();
      write_op_queue_.perform_cancellations();
    }
    for (std::size_t i = 0; i < timer_queues_.size(); ++i)
    {
      timer_queues_[i]->dispatch_timers();
      timer_queues_[i]->dispatch_cancellations();
    }

    // Issue any pending cancellations.
    for (size_t i = 0; i < pending_cancellations_.size(); ++i)
      cancel_ops_unlocked(pending_cancellations_[i]);
    pending_cancellations_.clear();

    complete_operations_and_timers(lock);
  }

  // Run the select loop in the thread.
  void run_thread()
  {
    boost::asio::detail::mutex::scoped_lock lock(mutex_);
    while (!stop_thread_)
    {
      lock.unlock();
      run(true);
      lock.lock();
    }
  }

  // Entry point for the select loop thread.
  static void call_run_thread(select_reactor* reactor)
  {
    reactor->run_thread();
  }

  // Interrupt the select loop.
  void interrupt()
  {
    interrupter_.interrupt();
  }

  // Check if all timer queues are empty.
  bool all_timer_queues_are_empty() const
  {
    for (std::size_t i = 0; i < timer_queues_.size(); ++i)
      if (!timer_queues_[i]->empty())
        return false;
    return true;
  }

  // Get the timeout value for the select call.
  timeval* get_timeout(timeval& tv)
  {
    if (all_timer_queues_are_empty())
      return 0;

    // By default we will wait no longer than 5 minutes. This will ensure that
    // any changes to the system clock are detected after no longer than this.
    boost::posix_time::time_duration minimum_wait_duration
      = boost::posix_time::minutes(5);

    for (std::size_t i = 0; i < timer_queues_.size(); ++i)
    {
      boost::posix_time::time_duration wait_duration
        = timer_queues_[i]->wait_duration();
      if (wait_duration < minimum_wait_duration)
        minimum_wait_duration = wait_duration;
    }

    if (minimum_wait_duration > boost::posix_time::time_duration())
    {
      tv.tv_sec = minimum_wait_duration.total_seconds();
      tv.tv_usec = minimum_wait_duration.total_microseconds() % 1000000;
    }
    else
    {
      tv.tv_sec = 0;
      tv.tv_usec = 0;
    }

    return &tv;
  }

  // Cancel all operations associated with the given descriptor. The do_cancel
  // function of the handler objects will be invoked. This function does not
  // acquire the select_reactor's mutex.
  void cancel_ops_unlocked(socket_type descriptor)
  {
    bool interrupt = read_op_queue_.cancel_operations(descriptor);
    interrupt = write_op_queue_.cancel_operations(descriptor) || interrupt;
    interrupt = except_op_queue_.cancel_operations(descriptor) || interrupt;
    if (interrupt)
      interrupter_.interrupt();
  }

  // Clean up operations and timers. We must not hold the lock since the
  // destructors may make calls back into this reactor. We make a copy of the
  // vector of timer queues since the original may be modified while the lock
  // is not held.
  void complete_operations_and_timers(
      boost::asio::detail::mutex::scoped_lock& lock)
  {
    timer_queues_for_cleanup_ = timer_queues_;
    lock.unlock();
    read_op_queue_.complete_operations();
    write_op_queue_.complete_operations();
    except_op_queue_.complete_operations();
    for (std::size_t i = 0; i < timer_queues_for_cleanup_.size(); ++i)
      timer_queues_for_cleanup_[i]->complete_timers();
  }

  // Mutex to protect access to internal data.
  boost::asio::detail::mutex mutex_;

  // Whether the select loop is currently running or not.
  bool select_in_progress_;

  // The interrupter is used to break a blocking select call.
  select_interrupter interrupter_;

  // The queue of read operations.
  reactor_op_queue<socket_type> read_op_queue_;

  // The queue of write operations.
  reactor_op_queue<socket_type> write_op_queue_;

  // The queue of exception operations.
  reactor_op_queue<socket_type> except_op_queue_;

  // The timer queues.
  std::vector<timer_queue_base*> timer_queues_;

  // A copy of the timer queues, used when cleaning up timers. The copy is
  // stored as a class data member to avoid unnecessary memory allocation.
  std::vector<timer_queue_base*> timer_queues_for_cleanup_;

  // The descriptors that are pending cancellation.
  std::vector<socket_type> pending_cancellations_;

  // Does the reactor loop thread need to stop.
  bool stop_thread_;

  // The thread that is running the reactor loop.
  boost::asio::detail::thread* thread_;

  // Whether the service has been shut down.
  bool shutdown_;
};

} // namespace detail
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_SELECT_REACTOR_HPP
