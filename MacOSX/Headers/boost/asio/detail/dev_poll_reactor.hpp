//
// dev_poll_reactor.hpp
// ~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_DEV_POLL_REACTOR_HPP
#define BOOST_ASIO_DETAIL_DEV_POLL_REACTOR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/dev_poll_reactor_fwd.hpp>

#if defined(BOOST_ASIO_HAS_DEV_POLL)

#include <boost/asio/detail/push_options.hpp>
#include <cstddef>
#include <vector>
#include <boost/config.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/throw_exception.hpp>
#include <sys/devpoll.h>
#include <boost/system/system_error.hpp>
#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/error.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/detail/bind_handler.hpp>
#include <boost/asio/detail/hash_map.hpp>
#include <boost/asio/detail/mutex.hpp>
#include <boost/asio/detail/task_io_service.hpp>
#include <boost/asio/detail/thread.hpp>
#include <boost/asio/detail/reactor_op_queue.hpp>
#include <boost/asio/detail/select_interrupter.hpp>
#include <boost/asio/detail/service_base.hpp>
#include <boost/asio/detail/signal_blocker.hpp>
#include <boost/asio/detail/socket_types.hpp>
#include <boost/asio/detail/timer_queue.hpp>

namespace boost {
namespace asio {
namespace detail {

template <bool Own_Thread>
class dev_poll_reactor
  : public boost::asio::detail::service_base<dev_poll_reactor<Own_Thread> >
{
public:
  // Per-descriptor data.
  struct per_descriptor_data
  {
  };

  // Constructor.
  dev_poll_reactor(boost::asio::io_service& io_service)
    : boost::asio::detail::service_base<
        dev_poll_reactor<Own_Thread> >(io_service),
      mutex_(),
      dev_poll_fd_(do_dev_poll_create()),
      wait_in_progress_(false),
      interrupter_(),
      read_op_queue_(),
      write_op_queue_(),
      except_op_queue_(),
      pending_cancellations_(),
      stop_thread_(false),
      thread_(0),
      shutdown_(false)
  {
    // Start the reactor's internal thread only if needed.
    if (Own_Thread)
    {
      boost::asio::detail::signal_blocker sb;
      thread_ = new boost::asio::detail::thread(
          bind_handler(&dev_poll_reactor::call_run_thread, this));
    }

    // Add the interrupter's descriptor to /dev/poll.
    ::pollfd ev = { 0 };
    ev.fd = interrupter_.read_descriptor();
    ev.events = POLLIN | POLLERR;
    ev.revents = 0;
    ::write(dev_poll_fd_, &ev, sizeof(ev));
  }

  // Destructor.
  ~dev_poll_reactor()
  {
    shutdown_service();
    ::close(dev_poll_fd_);
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
      typedef task_io_service<dev_poll_reactor<Own_Thread> >
        task_io_service_type;
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
      Handler handler, bool allow_speculative_read = true)
  {
    boost::asio::detail::mutex::scoped_lock lock(mutex_);

    if (shutdown_)
      return;

    if (allow_speculative_read)
    {
      if (!read_op_queue_.has_operation(descriptor))
      {
        boost::system::error_code ec;
        std::size_t bytes_transferred = 0;
        if (handler.perform(ec, bytes_transferred))
        {
          handler.complete(ec, bytes_transferred);
          return;
        }
      }
    }

    if (read_op_queue_.enqueue_operation(descriptor, handler))
    {
      ::pollfd& ev = add_pending_event_change(descriptor);
      ev.events = POLLIN | POLLERR | POLLHUP;
      if (write_op_queue_.has_operation(descriptor))
        ev.events |= POLLOUT;
      if (except_op_queue_.has_operation(descriptor))
        ev.events |= POLLPRI;
      interrupter_.interrupt();
    }
  }

  // Start a new write operation. The handler object will be invoked when the
  // given descriptor is ready to be written, or an error has occurred.
  template <typename Handler>
  void start_write_op(socket_type descriptor, per_descriptor_data&,
      Handler handler, bool allow_speculative_write = true)
  {
    boost::asio::detail::mutex::scoped_lock lock(mutex_);

    if (shutdown_)
      return;

    if (allow_speculative_write)
    {
      if (!write_op_queue_.has_operation(descriptor))
      {
        boost::system::error_code ec;
        std::size_t bytes_transferred = 0;
        if (handler.perform(ec, bytes_transferred))
        {
          handler.complete(ec, bytes_transferred);
          return;
        }
      }
    }

    if (write_op_queue_.enqueue_operation(descriptor, handler))
    {
      ::pollfd& ev = add_pending_event_change(descriptor);
      ev.events = POLLOUT | POLLERR | POLLHUP;
      if (read_op_queue_.has_operation(descriptor))
        ev.events |= POLLIN;
      if (except_op_queue_.has_operation(descriptor))
        ev.events |= POLLPRI;
      interrupter_.interrupt();
    }
  }

  // Start a new exception operation. The handler object will be invoked when
  // the given descriptor has exception information, or an error has occurred.
  template <typename Handler>
  void start_except_op(socket_type descriptor,
      per_descriptor_data&, Handler handler)
  {
    boost::asio::detail::mutex::scoped_lock lock(mutex_);

    if (shutdown_)
      return;

    if (except_op_queue_.enqueue_operation(descriptor, handler))
    {
      ::pollfd& ev = add_pending_event_change(descriptor);
      ev.events = POLLPRI | POLLERR | POLLHUP;
      if (read_op_queue_.has_operation(descriptor))
        ev.events |= POLLIN;
      if (write_op_queue_.has_operation(descriptor))
        ev.events |= POLLOUT;
      interrupter_.interrupt();
    }
  }

  // Start a new write operation. The handler object will be invoked when the
  // information available, or an error has occurred.
  template <typename Handler>
  void start_connect_op(socket_type descriptor,
      per_descriptor_data&, Handler handler)
  {
    boost::asio::detail::mutex::scoped_lock lock(mutex_);

    if (shutdown_)
      return;

    if (write_op_queue_.enqueue_operation(descriptor, handler))
    {
      ::pollfd& ev = add_pending_event_change(descriptor);
      ev.events = POLLOUT | POLLERR | POLLHUP;
      if (read_op_queue_.has_operation(descriptor))
        ev.events |= POLLIN;
      if (except_op_queue_.has_operation(descriptor))
        ev.events |= POLLPRI;
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

  // Cancel any operations that are running against the descriptor and remove
  // its registration from the reactor.
  void close_descriptor(socket_type descriptor, per_descriptor_data&)
  {
    boost::asio::detail::mutex::scoped_lock lock(mutex_);

    // Remove the descriptor from /dev/poll.
    ::pollfd& ev = add_pending_event_change(descriptor);
    ev.events = POLLREMOVE;
    interrupter_.interrupt();

    // Cancel any outstanding operations associated with the descriptor.
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
  friend class task_io_service<dev_poll_reactor<Own_Thread> >;

  // Run /dev/poll once until interrupted or events are ready to be dispatched.
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

    // Write the pending event registration changes to the /dev/poll descriptor.
    std::size_t events_size = sizeof(::pollfd) * pending_event_changes_.size();
    errno = 0;
    int result = ::write(dev_poll_fd_,
        &pending_event_changes_[0], events_size);
    if (result != static_cast<int>(events_size))
    {
      for (std::size_t i = 0; i < pending_event_changes_.size(); ++i)
      {
        int descriptor = pending_event_changes_[i].fd;
        boost::system::error_code ec = boost::system::error_code(
            errno, boost::asio::error::get_system_category());
        read_op_queue_.perform_all_operations(descriptor, ec);
        write_op_queue_.perform_all_operations(descriptor, ec);
        except_op_queue_.perform_all_operations(descriptor, ec);
      }
    }
    pending_event_changes_.clear();
    pending_event_change_index_.clear();

    int timeout = block ? get_timeout() : 0;
    wait_in_progress_ = true;
    lock.unlock();

    // Block on the /dev/poll descriptor.
    ::pollfd events[128] = { { 0 } };
    ::dvpoll dp = { 0 };
    dp.dp_fds = events;
    dp.dp_nfds = 128;
    dp.dp_timeout = timeout;
    int num_events = ::ioctl(dev_poll_fd_, DP_POLL, &dp);

    lock.lock();
    wait_in_progress_ = false;

    // Block signals while performing operations.
    boost::asio::detail::signal_blocker sb;

    // Dispatch the waiting events.
    for (int i = 0; i < num_events; ++i)
    {
      int descriptor = events[i].fd;
      if (descriptor == interrupter_.read_descriptor())
      {
        interrupter_.reset();
      }
      else
      {
        bool more_reads = false;
        bool more_writes = false;
        bool more_except = false;
        boost::system::error_code ec;

        // Exception operations must be processed first to ensure that any
        // out-of-band data is read before normal data.
        if (events[i].events & (POLLPRI | POLLERR | POLLHUP))
          more_except = except_op_queue_.perform_operation(descriptor, ec);
        else
          more_except = except_op_queue_.has_operation(descriptor);

        if (events[i].events & (POLLIN | POLLERR | POLLHUP))
          more_reads = read_op_queue_.perform_operation(descriptor, ec);
        else
          more_reads = read_op_queue_.has_operation(descriptor);

        if (events[i].events & (POLLOUT | POLLERR | POLLHUP))
          more_writes = write_op_queue_.perform_operation(descriptor, ec);
        else
          more_writes = write_op_queue_.has_operation(descriptor);

        if ((events[i].events & (POLLERR | POLLHUP)) != 0
              && (events[i].events & ~(POLLERR | POLLHUP)) == 0
              && !more_except && !more_reads && !more_writes)
        {
          // If we have an event and no operations associated with the
          // descriptor then we need to delete the descriptor from /dev/poll.
          // The poll operation can produce POLLHUP or POLLERR events when there
          // is no operation pending, so if we do not remove the descriptor we
          // can end up in a tight polling loop.
          ::pollfd ev = { 0 };
          ev.fd = descriptor;
          ev.events = POLLREMOVE;
          ev.revents = 0;
          ::write(dev_poll_fd_, &ev, sizeof(ev));
        }
        else
        {
          ::pollfd ev = { 0 };
          ev.fd = descriptor;
          ev.events = POLLERR | POLLHUP;
          if (more_reads)
            ev.events |= POLLIN;
          if (more_writes)
            ev.events |= POLLOUT;
          if (more_except)
            ev.events |= POLLPRI;
          ev.revents = 0;
          int result = ::write(dev_poll_fd_, &ev, sizeof(ev));
          if (result != sizeof(ev))
          {
            ec = boost::system::error_code(errno,
                boost::asio::error::get_system_category());
            read_op_queue_.perform_all_operations(descriptor, ec);
            write_op_queue_.perform_all_operations(descriptor, ec);
            except_op_queue_.perform_all_operations(descriptor, ec);
          }
        }
      }
    }
    read_op_queue_.perform_cancellations();
    write_op_queue_.perform_cancellations();
    except_op_queue_.perform_cancellations();
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
  static void call_run_thread(dev_poll_reactor* reactor)
  {
    reactor->run_thread();
  }

  // Interrupt the select loop.
  void interrupt()
  {
    interrupter_.interrupt();
  }

  // Create the /dev/poll file descriptor. Throws an exception if the descriptor
  // cannot be created.
  static int do_dev_poll_create()
  {
    int fd = ::open("/dev/poll", O_RDWR);
    if (fd == -1)
    {
      boost::throw_exception(
          boost::system::system_error(
            boost::system::error_code(errno,
              boost::asio::error::get_system_category()),
            "/dev/poll"));
    }
    return fd;
  }

  // Check if all timer queues are empty.
  bool all_timer_queues_are_empty() const
  {
    for (std::size_t i = 0; i < timer_queues_.size(); ++i)
      if (!timer_queues_[i]->empty())
        return false;
    return true;
  }

  // Get the timeout value for the /dev/poll DP_POLL operation. The timeout
  // value is returned as a number of milliseconds. A return value of -1
  // indicates that the poll should block indefinitely.
  int get_timeout()
  {
    if (all_timer_queues_are_empty())
      return -1;

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
      int milliseconds = minimum_wait_duration.total_milliseconds();
      return milliseconds > 0 ? milliseconds : 1;
    }
    else
    {
      return 0;
    }
  }

  // Cancel all operations associated with the given descriptor. The do_cancel
  // function of the handler objects will be invoked. This function does not
  // acquire the dev_poll_reactor's mutex.
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

  // Add a pending event entry for the given descriptor.
  ::pollfd& add_pending_event_change(int descriptor)
  {
    hash_map<int, std::size_t>::iterator iter
      = pending_event_change_index_.find(descriptor);
    if (iter == pending_event_change_index_.end())
    {
      std::size_t index = pending_event_changes_.size();
      pending_event_changes_.reserve(pending_event_changes_.size() + 1);
      pending_event_change_index_.insert(std::make_pair(descriptor, index));
      pending_event_changes_.push_back(::pollfd());
      pending_event_changes_[index].fd = descriptor;
      pending_event_changes_[index].revents = 0;
      return pending_event_changes_[index];
    }
    else
    {
      return pending_event_changes_[iter->second];
    }
  }

  // Mutex to protect access to internal data.
  boost::asio::detail::mutex mutex_;

  // The /dev/poll file descriptor.
  int dev_poll_fd_;

  // Vector of /dev/poll events waiting to be written to the descriptor.
  std::vector< ::pollfd> pending_event_changes_;

  // Hash map to associate a descriptor with a pending event change index.
  hash_map<int, std::size_t> pending_event_change_index_;

  // Whether the DP_POLL operation is currently in progress
  bool wait_in_progress_;

  // The interrupter is used to break a blocking DP_POLL operation.
  select_interrupter interrupter_;

  // The queue of read operations.
  reactor_op_queue<socket_type> read_op_queue_;

  // The queue of write operations.
  reactor_op_queue<socket_type> write_op_queue_;

  // The queue of except operations.
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

#endif // defined(BOOST_ASIO_HAS_DEV_POLL)

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_DEV_POLL_REACTOR_HPP
