//
// handler_queue.hpp
// ~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_HANDLER_QUEUE_HPP
#define BOOST_ASIO_DETAIL_HANDLER_QUEUE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/handler_alloc_helpers.hpp>
#include <boost/asio/detail/handler_invoke_helpers.hpp>
#include <boost/asio/detail/noncopyable.hpp>

namespace boost {
namespace asio {
namespace detail {

class handler_queue
  : private noncopyable
{
public:
  // Base class for handlers in the queue.
  class handler
    : private noncopyable
  {
  public:
    void invoke()
    {
      invoke_func_(this);
    }

    void destroy()
    {
      destroy_func_(this);
    }

  protected:
    typedef void (*invoke_func_type)(handler*);
    typedef void (*destroy_func_type)(handler*);

    handler(invoke_func_type invoke_func,
        destroy_func_type destroy_func)
      : next_(0),
        invoke_func_(invoke_func),
        destroy_func_(destroy_func)
    {
    }

    ~handler()
    {
    }

  private:
    friend class handler_queue;
    handler* next_;
    invoke_func_type invoke_func_;
    destroy_func_type destroy_func_;
  };

  // Smart point to manager handler lifetimes.
  class scoped_ptr
    : private noncopyable
  {
  public:
    explicit scoped_ptr(handler* h)
      : handler_(h)
    {
    }

    ~scoped_ptr()
    {
      if (handler_)
        handler_->destroy();
    }

    handler* get() const
    {
      return handler_;
    }

    handler* release()
    {
      handler* tmp = handler_;
      handler_ = 0;
      return tmp;
    }

  private:
    handler* handler_;
  };

  // Constructor.
  handler_queue()
    : front_(0),
      back_(0)
  {
  }

  // Wrap a handler to be pushed into the queue.
  template <typename Handler>
  static handler* wrap(Handler h)
  {
    // Allocate and construct an object to wrap the handler.
    typedef handler_wrapper<Handler> value_type;
    typedef handler_alloc_traits<Handler, value_type> alloc_traits;
    raw_handler_ptr<alloc_traits> raw_ptr(h);
    handler_ptr<alloc_traits> ptr(raw_ptr, h);
    return ptr.release();
  }

  // Get the handler at the front of the queue.
  handler* front()
  {
    return front_;
  }

  // Pop a handler from the front of the queue.
  void pop()
  {
    if (front_)
    {
      handler* tmp = front_;
      front_ = front_->next_;
      if (front_ == 0)
        back_ = 0;
      tmp->next_= 0;
    }
  }

  // Push a handler on to the back of the queue.
  void push(handler* h)
  {
    h->next_ = 0;
    if (back_)
    {
      back_->next_ = h;
      back_ = h;
    }
    else
    {
      front_ = back_ = h;
    }
  }

  // Whether the queue is empty.
  bool empty() const
  {
    return front_ == 0;
  }

private:
  // Template wrapper for handlers.
  template <typename Handler>
  class handler_wrapper
    : public handler
  {
  public:
    handler_wrapper(Handler h)
      : handler(
          &handler_wrapper<Handler>::do_call,
          &handler_wrapper<Handler>::do_destroy),
        handler_(h)
    {
    }

    static void do_call(handler* base)
    {
      // Take ownership of the handler object.
      typedef handler_wrapper<Handler> this_type;
      this_type* h(static_cast<this_type*>(base));
      typedef handler_alloc_traits<Handler, this_type> alloc_traits;
      handler_ptr<alloc_traits> ptr(h->handler_, h);

      // Make a copy of the handler so that the memory can be deallocated before
      // the upcall is made.
      Handler handler(h->handler_);

      // Free the memory associated with the handler.
      ptr.reset();

      // Make the upcall.
      boost_asio_handler_invoke_helpers::invoke(handler, &handler);
    }

    static void do_destroy(handler* base)
    {
      // Take ownership of the handler object.
      typedef handler_wrapper<Handler> this_type;
      this_type* h(static_cast<this_type*>(base));
      typedef handler_alloc_traits<Handler, this_type> alloc_traits;
      handler_ptr<alloc_traits> ptr(h->handler_, h);

      // A sub-object of the handler may be the true owner of the memory
      // associated with the handler. Consequently, a local copy of the handler
      // is required to ensure that any owning sub-object remains valid until
      // after we have deallocated the memory here.
      Handler handler(h->handler_);
      (void)handler;

      // Free the memory associated with the handler.
      ptr.reset();
    }

  private:
    Handler handler_;
  };

  // The front of the queue.
  handler* front_;

  // The back of the queue.
  handler* back_;
};

} // namespace detail
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_HANDLER_QUEUE_HPP
