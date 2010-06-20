//
// select_interrupter.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_SELECT_INTERRUPTER_HPP
#define BOOST_ASIO_DETAIL_SELECT_INTERRUPTER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <boost/config.hpp>
#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/detail/eventfd_select_interrupter.hpp>
#include <boost/asio/detail/pipe_select_interrupter.hpp>
#include <boost/asio/detail/socket_select_interrupter.hpp>

namespace boost {
namespace asio {
namespace detail {

#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
typedef socket_select_interrupter select_interrupter;
#elif defined(BOOST_ASIO_HAS_EVENTFD)
typedef eventfd_select_interrupter select_interrupter;
#else
typedef pipe_select_interrupter select_interrupter;
#endif

} // namespace detail
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_SELECT_INTERRUPTER_HPP
