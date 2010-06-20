//
// handler_base_from_member.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_HANDLER_BASE_FROM_MEMBER_HPP
#define BOOST_ASIO_DETAIL_HANDLER_BASE_FROM_MEMBER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/handler_alloc_helpers.hpp>
#include <boost/asio/detail/handler_invoke_helpers.hpp>

namespace boost {
namespace asio {
namespace detail {

// Base class for classes that need a handler data member. Forwards the custom
// allocation and invocation hooks to the contained handler.
template <typename Handler>
class handler_base_from_member
{
public:
  handler_base_from_member(Handler handler)
    : handler_(handler)
  {
  }

//protected:
  Handler handler_;

protected:
  // Protected destructor to prevent deletion through this type.
  ~handler_base_from_member()
  {
  }
};

template <typename Handler>
inline void* asio_handler_allocate(std::size_t size,
    handler_base_from_member<Handler>* this_handler)
{
  return boost_asio_handler_alloc_helpers::allocate(
      size, &this_handler->handler_);
}

template <typename Handler>
inline void asio_handler_deallocate(void* pointer, std::size_t size,
    handler_base_from_member<Handler>* this_handler)
{
  boost_asio_handler_alloc_helpers::deallocate(
      pointer, size, &this_handler->handler_);
}

template <typename Function, typename Handler>
inline void asio_handler_invoke(const Function& function,
    handler_base_from_member<Handler>* this_handler)
{
  boost_asio_handler_invoke_helpers::invoke(
      function, &this_handler->handler_);
}

} // namespace detail
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_HANDLER_BASE_FROM_MEMBER_HPP
