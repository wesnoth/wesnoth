//
// indirect_handler_queue.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_INDIRECT_HANDLER_QUEUE_HPP
#define BOOST_ASIO_DETAIL_INDIRECT_HANDLER_QUEUE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/handler_alloc_helpers.hpp>
#include <boost/asio/detail/handler_invoke_helpers.hpp>
#include <boost/asio/detail/noncopyable.hpp>

#if defined(_MSC_VER) && (_MSC_VER >= 1310)
extern "C" void _ReadWriteBarrier();
# pragma intrinsic(_ReadWriteBarrier)
#endif // defined(_MSC_VER) && (_MSC_VER >= 1310)

namespace boost {
namespace asio {
namespace detail {

class indirect_handler_queue
  : private noncopyable
{
public:
  class handler;

  // Element for a node in the queue.
  class node
  {
  public:
    node()
      : version_(0),
        handler_(0),
        next_(0)
    {
    }

  private:
    friend class indirect_handler_queue;
    unsigned long version_;
    handler* handler_;
    node* next_;
  };

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
      : node_(new node),
        invoke_func_(invoke_func),
        destroy_func_(destroy_func)
    {
    }

    ~handler()
    {
      if (node_)
        delete node_;
    }

  private:
    friend class indirect_handler_queue;
    node* node_;
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
  indirect_handler_queue()
    : front_(new node),
      back_(front_),
      next_version_(1)
  {
  }

  // Destructor.
  ~indirect_handler_queue()
  {
    while (front_)
    {
      node* tmp = front_;
      front_ = front_->next_;
      delete tmp;
    }
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

  // Determine whether the queue has something ready to pop.
  bool poppable()
  {
    return front_->next_ != 0;
  }

  // The version number at the front of the queue.
  unsigned long front_version()
  {
    return front_->version_;
  }

  // The version number at the back of the queue.
  unsigned long back_version()
  {
    return back_->version_;
  }

  // Pop a handler from the front of the queue.
  handler* pop()
  {
    node* n = front_;
    node* new_front = n->next_;
    if (new_front)
    {
      handler* h = new_front->handler_;
      h->node_ = n;
      new_front->handler_ = 0;
      front_ = new_front;
      return h;
    }
    return 0;
  }

  // Push a handler on to the back of the queue.
  void push(handler* h)
  {
    node* n = h->node_;
    h->node_ = 0;
    n->version_ = next_version_;
    next_version_ += 2;
    n->handler_ = h;
    n->next_ = 0;
    memory_barrier();
    back_->next_ = n;
    back_ = n;
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

  // Helper function to create a memory barrier.
  static void memory_barrier()
  {
#if defined(_GLIBCXX_WRITE_MEM_BARRIER)
    _GLIBCXX_WRITE_MEM_BARRIER;
#elif defined(_MSC_VER) && (_MSC_VER >= 1310)
    _ReadWriteBarrier();
#else
# error memory barrier required
#endif
  }

  // The front of the queue.
  node* front_;

  // The back of the queue.
  node* back_;

  // The next version counter to be assigned to a node.
  unsigned long next_version_;
};

} // namespace detail
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_INDIRECT_HANDLER_QUEUE_HPP
