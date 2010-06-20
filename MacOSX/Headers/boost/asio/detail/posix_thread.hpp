//
// posix_thread.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_POSIX_THREAD_HPP
#define BOOST_ASIO_DETAIL_POSIX_THREAD_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <boost/config.hpp>
#include <boost/system/system_error.hpp>
#include <boost/asio/detail/pop_options.hpp>

#if defined(BOOST_HAS_PTHREADS)

#include <boost/asio/detail/push_options.hpp>
#include <memory>
#include <boost/throw_exception.hpp>
#include <pthread.h>
#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/error.hpp>
#include <boost/asio/detail/noncopyable.hpp>

namespace boost {
namespace asio {
namespace detail {

extern "C" void* asio_detail_posix_thread_function(void* arg);

class posix_thread
  : private noncopyable
{
public:
  // Constructor.
  template <typename Function>
  posix_thread(Function f)
    : joined_(false)
  {
    std::auto_ptr<func_base> arg(new func<Function>(f));
    int error = ::pthread_create(&thread_, 0,
          asio_detail_posix_thread_function, arg.get());
    if (error != 0)
    {
      boost::system::system_error e(
          boost::system::error_code(error,
            boost::asio::error::get_system_category()),
          "thread");
      boost::throw_exception(e);
    }
    arg.release();
  }

  // Destructor.
  ~posix_thread()
  {
    if (!joined_)
      ::pthread_detach(thread_);
  }

  // Wait for the thread to exit.
  void join()
  {
    if (!joined_)
    {
      ::pthread_join(thread_, 0);
      joined_ = true;
    }
  }

private:
  friend void* asio_detail_posix_thread_function(void* arg);

  class func_base
  {
  public:
    virtual ~func_base() {}
    virtual void run() = 0;
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

  ::pthread_t thread_;
  bool joined_;
};

inline void* asio_detail_posix_thread_function(void* arg)
{
  std::auto_ptr<posix_thread::func_base> f(
      static_cast<posix_thread::func_base*>(arg));
  f->run();
  return 0;
}

} // namespace detail
} // namespace asio
} // namespace boost

#endif // defined(BOOST_HAS_PTHREADS)

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_POSIX_THREAD_HPP
