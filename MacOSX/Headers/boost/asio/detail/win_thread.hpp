//
// win_thread.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_WIN_THREAD_HPP
#define BOOST_ASIO_DETAIL_WIN_THREAD_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <boost/config.hpp>
#include <boost/system/system_error.hpp>
#include <boost/asio/detail/pop_options.hpp>

#if defined(BOOST_WINDOWS) && !defined(UNDER_CE)

#include <boost/asio/error.hpp>
#include <boost/asio/detail/noncopyable.hpp>
#include <boost/asio/detail/socket_types.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <boost/throw_exception.hpp>
#include <memory>
#include <process.h>
#include <boost/asio/detail/pop_options.hpp>

namespace boost {
namespace asio {
namespace detail {

unsigned int __stdcall win_thread_function(void* arg);

#if (WINVER < 0x0500)
void __stdcall apc_function(ULONG data);
#else
void __stdcall apc_function(ULONG_PTR data);
#endif

template <typename T>
class win_thread_base
{
public:
  static bool terminate_threads()
  {
    return ::InterlockedExchangeAdd(&terminate_threads_, 0) != 0;
  }

  static void set_terminate_threads(bool b)
  {
    ::InterlockedExchange(&terminate_threads_, b ? 1 : 0);
  }

private:
  static long terminate_threads_;
};

template <typename T>
long win_thread_base<T>::terminate_threads_ = 0;

class win_thread
  : private noncopyable,
    public win_thread_base<win_thread>
{
public:
  // Constructor.
  template <typename Function>
  win_thread(Function f)
    : exit_event_(0)
  {
    std::auto_ptr<func_base> arg(new func<Function>(f));

    ::HANDLE entry_event = 0;
    arg->entry_event_ = entry_event = ::CreateEvent(0, true, false, 0);
    if (!entry_event)
    {
      DWORD last_error = ::GetLastError();
      boost::system::system_error e(
          boost::system::error_code(last_error,
            boost::asio::error::get_system_category()),
          "thread.entry_event");
      boost::throw_exception(e);
    }

    arg->exit_event_ = exit_event_ = ::CreateEvent(0, true, false, 0);
    if (!exit_event_)
    {
      DWORD last_error = ::GetLastError();
      ::CloseHandle(entry_event);
      boost::system::system_error e(
          boost::system::error_code(last_error,
            boost::asio::error::get_system_category()),
          "thread.exit_event");
      boost::throw_exception(e);
    }

    unsigned int thread_id = 0;
    thread_ = reinterpret_cast<HANDLE>(::_beginthreadex(0, 0,
          win_thread_function, arg.get(), 0, &thread_id));
    if (!thread_)
    {
      DWORD last_error = ::GetLastError();
      if (entry_event)
        ::CloseHandle(entry_event);
      if (exit_event_)
        ::CloseHandle(exit_event_);
      boost::system::system_error e(
          boost::system::error_code(last_error,
            boost::asio::error::get_system_category()),
          "thread");
      boost::throw_exception(e);
    }
    arg.release();

    if (entry_event)
    {
      ::WaitForSingleObject(entry_event, INFINITE);
      ::CloseHandle(entry_event);
    }
  }

  // Destructor.
  ~win_thread()
  {
    ::CloseHandle(thread_);

    // The exit_event_ handle is deliberately allowed to leak here since it
    // is an error for the owner of an internal thread not to join() it.
  }

  // Wait for the thread to exit.
  void join()
  {
    ::WaitForSingleObject(exit_event_, INFINITE);
    ::CloseHandle(exit_event_);
    if (terminate_threads())
    {
      ::TerminateThread(thread_, 0);
    }
    else
    {
      ::QueueUserAPC(apc_function, thread_, 0);
      ::WaitForSingleObject(thread_, INFINITE);
    }
  }

private:
  friend unsigned int __stdcall win_thread_function(void* arg);

#if (WINVER < 0x0500)
  friend void __stdcall apc_function(ULONG);
#else
  friend void __stdcall apc_function(ULONG_PTR);
#endif

  class func_base
  {
  public:
    virtual ~func_base() {}
    virtual void run() = 0;
    ::HANDLE entry_event_;
    ::HANDLE exit_event_;
  };

  template <typename Function>
  class func
    : public func_base
  {
  public:
    func(Function f)
      : f_(f)
    {
    }

    virtual void run()
    {
      f_();
    }

  private:
    Function f_;
  };

  ::HANDLE thread_;
  ::HANDLE exit_event_;
};

inline unsigned int __stdcall win_thread_function(void* arg)
{
  std::auto_ptr<win_thread::func_base> func(
      static_cast<win_thread::func_base*>(arg));

  ::SetEvent(func->entry_event_);

  func->run();

  // Signal that the thread has finished its work, but rather than returning go
  // to sleep to put the thread into a well known state. If the thread is being
  // joined during global object destruction then it may be killed using
  // TerminateThread (to avoid a deadlock in DllMain). Otherwise, the SleepEx
  // call will be interrupted using QueueUserAPC and the thread will shut down
  // cleanly.
  HANDLE exit_event = func->exit_event_;
  func.reset();
  ::SetEvent(exit_event);
  ::SleepEx(INFINITE, TRUE);

  return 0;
}

#if (WINVER < 0x0500)
inline void __stdcall apc_function(ULONG) {}
#else
inline void __stdcall apc_function(ULONG_PTR) {}
#endif

} // namespace detail
} // namespace asio
} // namespace boost

#endif // defined(BOOST_WINDOWS) && !defined(UNDER_CE)

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_WIN_THREAD_HPP
