//
// handler_alloc_helpers.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_HANDLER_ALLOC_HELPERS_HPP
#define BOOST_ASIO_DETAIL_HANDLER_ALLOC_HELPERS_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <boost/detail/workaround.hpp>
#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/handler_alloc_hook.hpp>
#include <boost/asio/detail/noncopyable.hpp>

// Calls to asio_handler_allocate and asio_handler_deallocate must be made from
// a namespace that does not contain any overloads of these functions. The
// boost_asio_handler_alloc_helpers namespace is defined here for that purpose.
namespace boost_asio_handler_alloc_helpers {

template <typename Handler>
inline void* allocate(std::size_t s, Handler* h)
{
#if BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x564))
  return ::operator new(s);
#else
  using namespace boost::asio;
  return asio_handler_allocate(s, h);
#endif
}

template <typename Handler>
inline void deallocate(void* p, std::size_t s, Handler* h)
{
#if BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x564))
  ::operator delete(p);
#else
  using namespace boost::asio;
  asio_handler_deallocate(p, s, h);
#endif
}

} // namespace boost_asio_handler_alloc_helpers

namespace boost {
namespace asio {
namespace detail {

// Traits for handler allocation.
template <typename Handler, typename Object>
struct handler_alloc_traits
{
  typedef Handler handler_type;
  typedef Object value_type;
  typedef Object* pointer_type;
  BOOST_STATIC_CONSTANT(std::size_t, value_size = sizeof(Object));
};

template <typename Alloc_Traits>
class handler_ptr;

// Helper class to provide RAII on uninitialised handler memory.
template <typename Alloc_Traits>
class raw_handler_ptr
  : private noncopyable
{
public:
  typedef typename Alloc_Traits::handler_type handler_type;
  typedef typename Alloc_Traits::value_type value_type;
  typedef typename Alloc_Traits::pointer_type pointer_type;
  BOOST_STATIC_CONSTANT(std::size_t, value_size = Alloc_Traits::value_size);

  // Constructor allocates the memory.
  raw_handler_ptr(handler_type& handler)
    : handler_(handler),
      pointer_(static_cast<pointer_type>(
            boost_asio_handler_alloc_helpers::allocate(value_size, &handler_)))
  {
  }

  // Destructor automatically deallocates memory, unless it has been stolen by
  // a handler_ptr object.
  ~raw_handler_ptr()
  {
    if (pointer_)
      boost_asio_handler_alloc_helpers::deallocate(
          pointer_, value_size, &handler_);
  }

private:
  friend class handler_ptr<Alloc_Traits>;
  handler_type& handler_;
  pointer_type pointer_;
};

// Helper class to provide RAII on uninitialised handler memory.
template <typename Alloc_Traits>
class handler_ptr
  : private noncopyable
{
public:
  typedef typename Alloc_Traits::handler_type handler_type;
  typedef typename Alloc_Traits::value_type value_type;
  typedef typename Alloc_Traits::pointer_type pointer_type;
  BOOST_STATIC_CONSTANT(std::size_t, value_size = Alloc_Traits::value_size);
  typedef raw_handler_ptr<Alloc_Traits> raw_ptr_type;

  // Take ownership of existing memory.
  handler_ptr(handler_type& handler, pointer_type pointer)
    : handler_(handler),
      pointer_(pointer)
  {
  }

  // Construct object in raw memory and take ownership if construction succeeds.
  handler_ptr(raw_ptr_type& raw_ptr)
    : handler_(raw_ptr.handler_),
      pointer_(new (raw_ptr.pointer_) value_type)
  {
    raw_ptr.pointer_ = 0;
  }

  // Construct object in raw memory and take ownership if construction succeeds.
  template <typename Arg1>
  handler_ptr(raw_ptr_type& raw_ptr, Arg1& a1)
    : handler_(raw_ptr.handler_),
      pointer_(new (raw_ptr.pointer_) value_type(a1))
  {
    raw_ptr.pointer_ = 0;
  }

  // Construct object in raw memory and take ownership if construction succeeds.
  template <typename Arg1, typename Arg2>
  handler_ptr(raw_ptr_type& raw_ptr, Arg1& a1, Arg2& a2)
    : handler_(raw_ptr.handler_),
      pointer_(new (raw_ptr.pointer_) value_type(a1, a2))
  {
    raw_ptr.pointer_ = 0;
  }

  // Construct object in raw memory and take ownership if construction succeeds.
  template <typename Arg1, typename Arg2, typename Arg3>
  handler_ptr(raw_ptr_type& raw_ptr, Arg1& a1, Arg2& a2, Arg3& a3)
    : handler_(raw_ptr.handler_),
      pointer_(new (raw_ptr.pointer_) value_type(a1, a2, a3))
  {
    raw_ptr.pointer_ = 0;
  }

  // Construct object in raw memory and take ownership if construction succeeds.
  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4>
  handler_ptr(raw_ptr_type& raw_ptr, Arg1& a1, Arg2& a2, Arg3& a3, Arg4& a4)
    : handler_(raw_ptr.handler_),
      pointer_(new (raw_ptr.pointer_) value_type(a1, a2, a3, a4))
  {
    raw_ptr.pointer_ = 0;
  }

  // Construct object in raw memory and take ownership if construction succeeds.
  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4,
      typename Arg5>
  handler_ptr(raw_ptr_type& raw_ptr, Arg1& a1, Arg2& a2, Arg3& a3, Arg4& a4,
      Arg5& a5)
    : handler_(raw_ptr.handler_),
      pointer_(new (raw_ptr.pointer_) value_type(a1, a2, a3, a4, a5))
  {
    raw_ptr.pointer_ = 0;
  }

  // Construct object in raw memory and take ownership if construction succeeds.
  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4,
      typename Arg5, typename Arg6>
  handler_ptr(raw_ptr_type& raw_ptr, Arg1& a1, Arg2& a2, Arg3& a3, Arg4& a4,
      Arg5& a5, Arg6& a6)
    : handler_(raw_ptr.handler_),
      pointer_(new (raw_ptr.pointer_) value_type(a1, a2, a3, a4, a5, a6))
  {
    raw_ptr.pointer_ = 0;
  }

  // Construct object in raw memory and take ownership if construction succeeds.
  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4,
      typename Arg5, typename Arg6, typename Arg7>
  handler_ptr(raw_ptr_type& raw_ptr, Arg1& a1, Arg2& a2, Arg3& a3, Arg4& a4,
      Arg5& a5, Arg6& a6, Arg7& a7)
    : handler_(raw_ptr.handler_),
      pointer_(new (raw_ptr.pointer_) value_type(a1, a2, a3, a4, a5, a6, a7))
  {
    raw_ptr.pointer_ = 0;
  }

  // Construct object in raw memory and take ownership if construction succeeds.
  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4,
      typename Arg5, typename Arg6, typename Arg7, typename Arg8>
  handler_ptr(raw_ptr_type& raw_ptr, Arg1& a1, Arg2& a2, Arg3& a3, Arg4& a4,
      Arg5& a5, Arg6& a6, Arg7& a7, Arg8& a8)
    : handler_(raw_ptr.handler_),
      pointer_(new (raw_ptr.pointer_) value_type(
            a1, a2, a3, a4, a5, a6, a7, a8))
  {
    raw_ptr.pointer_ = 0;
  }

  // Destructor automatically deallocates memory, unless it has been released.
  ~handler_ptr()
  {
    reset();
  }

  // Get the memory.
  pointer_type get() const
  {
    return pointer_;
  }

  // Release ownership of the memory.
  pointer_type release()
  {
    pointer_type tmp = pointer_;
    pointer_ = 0;
    return tmp;
  }

  // Explicitly destroy and deallocate the memory.
  void reset()
  {
    if (pointer_)
    {
      pointer_->value_type::~value_type();
      boost_asio_handler_alloc_helpers::deallocate(
          pointer_, value_size, &handler_);
      pointer_ = 0;
    }
  }

private:
  handler_type& handler_;
  pointer_type pointer_;
};

} // namespace detail
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_HANDLER_ALLOC_HELPERS_HPP
