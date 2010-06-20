//
// epoll_reactor_fwd.hpp
// ~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_EPOLL_REACTOR_FWD_HPP
#define BOOST_ASIO_DETAIL_EPOLL_REACTOR_FWD_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#if !defined(BOOST_ASIO_DISABLE_EPOLL)
#if defined(__linux__) // This service is only supported on Linux.

#include <boost/asio/detail/push_options.hpp>
#include <linux/version.h>
#include <boost/asio/detail/pop_options.hpp>

#if LINUX_VERSION_CODE >= KERNEL_VERSION (2,5,45) // Only kernels >= 2.5.45.

// Define this to indicate that epoll is supported on the target platform.
#define BOOST_ASIO_HAS_EPOLL 1

namespace boost {
namespace asio {
namespace detail {

template <bool Own_Thread>
class epoll_reactor;

} // namespace detail
} // namespace asio
} // namespace boost

#endif // LINUX_VERSION_CODE >= KERNEL_VERSION (2,5,45)
#endif // defined(__linux__)
#endif // !defined(BOOST_ASIO_DISABLE_EPOLL)

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_EPOLL_REACTOR_FWD_HPP
