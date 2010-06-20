//
// win_iocp_io_service_fwd.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_WIN_IOCP_IO_SERVICE_FWD_HPP
#define BOOST_ASIO_DETAIL_WIN_IOCP_IO_SERVICE_FWD_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <boost/config.hpp>
#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/detail/socket_types.hpp>

// This service is only supported on Win32 (NT4 and later).
#if !defined(BOOST_ASIO_DISABLE_IOCP)
#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
#if defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x0400)
#if !defined(UNDER_CE)

// Define this to indicate that IOCP is supported on the target platform.
#define BOOST_ASIO_HAS_IOCP 1

namespace boost {
namespace asio {
namespace detail {

class win_iocp_io_service;
class win_iocp_overlapped_ptr;

} // namespace detail
} // namespace asio
} // namespace boost

#endif // !defined(UNDER_CE)
#endif // defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x0400)
#endif // defined(BOOST_WINDOWS) || defined(__CYGWIN__)
#endif // !defined(BOOST_ASIO_DISABLE_IOCP)

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_WIN_IOCP_IO_SERVICE_FWD_HPP
