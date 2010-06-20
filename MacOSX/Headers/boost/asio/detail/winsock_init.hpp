//
// winsock_init.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_WINSOCK_INIT_HPP
#define BOOST_ASIO_DETAIL_WINSOCK_INIT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <boost/config.hpp>
#include <boost/system/system_error.hpp>
#include <boost/asio/detail/pop_options.hpp>

#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)

#include <boost/asio/detail/push_options.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/throw_exception.hpp>
#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/error.hpp>
#include <boost/asio/detail/noncopyable.hpp>
#include <boost/asio/detail/socket_types.hpp>

namespace boost {
namespace asio {
namespace detail {

template <int Major = 2, int Minor = 0>
class winsock_init
  : private noncopyable
{
private:
  // Structure to perform the actual initialisation.
  struct do_init
  {
    do_init()
    {
      WSADATA wsa_data;
      result_ = ::WSAStartup(MAKEWORD(Major, Minor), &wsa_data);
    }

    ~do_init()
    {
      ::WSACleanup();
    }

    int result() const
    {
      return result_;
    }

    // Helper function to manage a do_init singleton. The static instance of the
    // winsock_init object ensures that this function is always called before
    // main, and therefore before any other threads can get started. The do_init
    // instance must be static in this function to ensure that it gets
    // initialised before any other global objects try to use it.
    static boost::shared_ptr<do_init> instance()
    {
      static boost::shared_ptr<do_init> init(new do_init);
      return init;
    }

  private:
    int result_;
  };

public:
  // Constructor.
  winsock_init()
    : ref_(do_init::instance())
  {
    // Check whether winsock was successfully initialised. This check is not
    // performed for the global instance since there will be nobody around to
    // catch the exception.
    if (this != &instance_ && ref_->result() != 0)
    {
      boost::system::system_error e(
          boost::system::error_code(ref_->result(),
            boost::asio::error::get_system_category()),
          "winsock");
      boost::throw_exception(e);
    }
  }

  // Destructor.
  ~winsock_init()
  {
  }

private:
  // Instance to force initialisation of winsock at global scope.
  static winsock_init instance_;

  // Reference to singleton do_init object to ensure that winsock does not get
  // cleaned up until the last user has finished with it.
  boost::shared_ptr<do_init> ref_;
};

template <int Major, int Minor>
winsock_init<Major, Minor> winsock_init<Major, Minor>::instance_;

} // namespace detail
} // namespace asio
} // namespace boost

#endif // defined(BOOST_WINDOWS) || defined(__CYGWIN__)

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_WINSOCK_INIT_HPP
