//
// strand_service.hpp
// ~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_STRAND_SERVICE_HPP
#define BOOST_ASIO_DETAIL_STRAND_SERVICE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <boost/aligned_storage.hpp>
#include <boost/assert.hpp>
#include <boost/detail/atomic_count.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/asio/detail/bind_handler.hpp>
#include <boost/asio/detail/call_stack.hpp>
#include <boost/asio/detail/handler_alloc_helpers.hpp>
#include <boost/asio/detail/handler_invoke_helpers.hpp>
#include <boost/asio/detail/mutex.hpp>
#include <boost/asio/detail/noncopyable.hpp>
#include <boost/asio/detail/service_base.hpp>

namespace boost {
namespace asio {
namespace detail {

// Default service implementation for a strand.
class strand_service
  : public boost::asio::detail::service_base<strand_service>
{
public:
  class handler_base;
  class invoke_current_handler;
  class post_next_waiter_on_exit;

  // The underlying implementation of a strand.
  class strand_impl
  {
#if defined (__BORLANDC__)
  public:
#else
  private:
#endif
    void add_ref()
    {
      ++ref_count_;
    }

    void release()
    {
      if (--ref_count_ == 0)
        delete this;
    }

  private:
    // Only this service will have access to the internal values.
    friend class strand_service;
    friend class post_next_waiter_on_exit;
    friend class invoke_current_handler;

    strand_impl(strand_service& owner)
      : owner_(owner),
        current_handler_(0),
        first_waiter_(0),
        last_waiter_(0),
        ref_count_(0)
    {
      // Insert implementation into linked list of all implementations.
      boost::asio::detail::mutex::scoped_lock lock(owner_.mutex_);
      next_ = owner_.impl_list_;
      prev_ = 0;
      if (owner_.impl_list_)
        owner_.impl_list_->prev_ = this;
      owner_.impl_list_ = this;
    }

    ~strand_impl()
    {
      // Remove implementation from linked list of all implementations.
      boost::asio::detail::mutex::scoped_lock lock(owner_.mutex_);
      if (owner_.impl_list_ == this)
        owner_.impl_list_ = next_;
      if (prev_)
        prev_->next_ = next_;
      if (next_)
        next_->prev_= prev_;
      next_ = 0;
      prev_ = 0;
      lock.unlock();

      if (current_handler_)
      {
        current_handler_->destroy();
      }

      while (first_waiter_)
      {
        handler_base* next = first_waiter_->next_;
        first_waiter_->destroy();
        first_waiter_ = next;
      }
    }

    // Mutex to protect access to internal data.
    boost::asio::detail::mutex mutex_;

    // The service that owns this implementation.
    strand_service& owner_;

    // The handler that is ready to execute. If this pointer is non-null then it
    // indicates that a handler holds the lock.
    handler_base* current_handler_;

    // The start of the list of waiting handlers for the strand.
    handler_base* first_waiter_;
    
    // The end of the list of waiting handlers for the strand.
    handler_base* last_waiter_;

    // Storage for posted handlers.
    typedef boost::aligned_storage<128> handler_storage_type;
#if defined(__BORLANDC__)
    boost::aligned_storage<128> handler_storage_;
#else
    handler_storage_type handler_storage_;
#endif

    // Pointers to adjacent socket implementations in linked list.
    strand_impl* next_;
    strand_impl* prev_;

    // The reference count on the strand implementation.
    boost::detail::atomic_count ref_count_;

#if !defined(__BORLANDC__)
    friend void intrusive_ptr_add_ref(strand_impl* p)
    {
      p->add_ref();
    }

    friend void intrusive_ptr_release(strand_impl* p)
    {
      p->release();
    }
#endif
  };

  friend class strand_impl;

  typedef boost::intrusive_ptr<strand_impl> implementation_type;

  // Base class for all handler types.
  class handler_base
  {
  public:
    typedef void (*invoke_func_type)(handler_base*,
        strand_service&, implementation_type&);
    typedef void (*destroy_func_type)(handler_base*);

    handler_base(invoke_func_type invoke_func, destroy_func_type destroy_func)
      : next_(0),
        invoke_func_(invoke_func),
        destroy_func_(destroy_func)
    {
    }

    void invoke(strand_service& service_impl, implementation_type& impl)
    {
      invoke_func_(this, service_impl, impl);
    }

    void destroy()
    {
      destroy_func_(this);
    }

  protected:
    ~handler_base()
    {
    }

  private:
    friend class strand_service;
    friend class strand_impl;
    friend class post_next_waiter_on_exit;
    handler_base* next_;
    invoke_func_type invoke_func_;
    destroy_func_type destroy_func_;
  };

  // Helper class to allow handlers to be dispatched.
  class invoke_current_handler
  {
  public:
    invoke_current_handler(strand_service& service_impl,
        const implementation_type& impl)
      : service_impl_(service_impl),
        impl_(impl)
    {
    }

    void operator()()
    {
      impl_->current_handler_->invoke(service_impl_, impl_);
    }

    friend void* asio_handler_allocate(std::size_t size,
        invoke_current_handler* this_handler)
    {
      return this_handler->do_handler_allocate(size);
    }

    friend void asio_handler_deallocate(void*, std::size_t,
        invoke_current_handler*)
    {
    }

    void* do_handler_allocate(std::size_t size)
    {
#if defined(__BORLANDC__)
      BOOST_ASSERT(size <= boost::aligned_storage<128>::size);
#else
      BOOST_ASSERT(size <= strand_impl::handler_storage_type::size);
#endif
      (void)size;
      return impl_->handler_storage_.address();
    }

    // The asio_handler_invoke hook is not defined here since the default one
    // provides the correct behaviour, and including it here breaks MSVC 7.1
    // in some situations.

  private:
    strand_service& service_impl_;
    implementation_type impl_;
  };

  // Helper class to automatically enqueue next waiter on block exit.
  class post_next_waiter_on_exit
  {
  public:
    post_next_waiter_on_exit(strand_service& service_impl,
        implementation_type& impl)
      : service_impl_(service_impl),
        impl_(impl),
        cancelled_(false)
    {
    }

    ~post_next_waiter_on_exit()
    {
      if (!cancelled_)
      {
        boost::asio::detail::mutex::scoped_lock lock(impl_->mutex_);
        impl_->current_handler_ = impl_->first_waiter_;
        if (impl_->current_handler_)
        {
          impl_->first_waiter_ = impl_->first_waiter_->next_;
          if (impl_->first_waiter_ == 0)
            impl_->last_waiter_ = 0;
          lock.unlock();
          service_impl_.get_io_service().post(
              invoke_current_handler(service_impl_, impl_));
        }
      }
    }

    void cancel()
    {
      cancelled_ = true;
    }

  private:
    strand_service& service_impl_;
    implementation_type& impl_;
    bool cancelled_;
  };

  // Class template for a waiter.
  template <typename Handler>
  class handler_wrapper
    : public handler_base
  {
  public:
    handler_wrapper(Handler handler)
      : handler_base(&handler_wrapper<Handler>::do_invoke,
          &handler_wrapper<Handler>::do_destroy),
        handler_(handler)
    {
    }

    static void do_invoke(handler_base* base,
        strand_service& service_impl, implementation_type& impl)
    {
      // Take ownership of the handler object.
      typedef handler_wrapper<Handler> this_type;
      this_type* h(static_cast<this_type*>(base));
      typedef handler_alloc_traits<Handler, this_type> alloc_traits;
      handler_ptr<alloc_traits> ptr(h->handler_, h);

      post_next_waiter_on_exit p1(service_impl, impl);

      // Make a copy of the handler so that the memory can be deallocated before
      // the upcall is made.
      Handler handler(h->handler_);

      // A handler object must still be valid when the next waiter is posted
      // since destroying the last handler might cause the strand object to be
      // destroyed. Therefore we create a second post_next_waiter_on_exit object
      // that will be destroyed before the handler object.
      p1.cancel();
      post_next_waiter_on_exit p2(service_impl, impl);

      // Free the memory associated with the handler.
      ptr.reset();

      // Indicate that this strand is executing on the current thread.
      call_stack<strand_impl>::context ctx(impl.get());

      // Make the upcall.
      boost_asio_handler_invoke_helpers::invoke(handler, &handler);
    }

    static void do_destroy(handler_base* base)
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

  // Construct a new strand service for the specified io_service.
  explicit strand_service(boost::asio::io_service& io_service)
    : boost::asio::detail::service_base<strand_service>(io_service),
      mutex_(),
      impl_list_(0)
  {
  }

  // Destroy all user-defined handler objects owned by the service.
  void shutdown_service()
  {
    // Construct a list of all handlers to be destroyed.
    boost::asio::detail::mutex::scoped_lock lock(mutex_);
    strand_impl* impl = impl_list_;
    handler_base* first_handler = 0;
    while (impl)
    {
      if (impl->current_handler_)
      {
        impl->current_handler_->next_ = first_handler;
        first_handler = impl->current_handler_;
        impl->current_handler_ = 0;
      }
      if (impl->first_waiter_)
      {
        impl->last_waiter_->next_ = first_handler;
        first_handler = impl->first_waiter_;
        impl->first_waiter_ = 0;
        impl->last_waiter_ = 0;
      }
      impl = impl->next_;
    }

    // Destroy all handlers without holding the lock.
    lock.unlock();
    while (first_handler)
    {
      handler_base* next = first_handler->next_;
      first_handler->destroy();
      first_handler = next;
    }
  }

  // Construct a new strand implementation.
  void construct(implementation_type& impl)
  {
    impl = implementation_type(new strand_impl(*this));
  }

  // Destroy a strand implementation.
  void destroy(implementation_type& impl)
  {
    implementation_type().swap(impl);
  }

  // Request the io_service to invoke the given handler.
  template <typename Handler>
  void dispatch(implementation_type& impl, Handler handler)
  {
    if (call_stack<strand_impl>::contains(impl.get()))
    {
      boost_asio_handler_invoke_helpers::invoke(handler, &handler);
    }
    else
    {
      // Allocate and construct an object to wrap the handler.
      typedef handler_wrapper<Handler> value_type;
      typedef handler_alloc_traits<Handler, value_type> alloc_traits;
      raw_handler_ptr<alloc_traits> raw_ptr(handler);
      handler_ptr<alloc_traits> ptr(raw_ptr, handler);

      boost::asio::detail::mutex::scoped_lock lock(impl->mutex_);

      if (impl->current_handler_ == 0)
      {
        // This handler now has the lock, so can be dispatched immediately.
        impl->current_handler_ = ptr.release();
        lock.unlock();
        this->get_io_service().dispatch(invoke_current_handler(*this, impl));
      }
      else
      {
        // Another handler already holds the lock, so this handler must join
        // the list of waiters. The handler will be posted automatically when
        // its turn comes.
        if (impl->last_waiter_)
        {
          impl->last_waiter_->next_ = ptr.get();
          impl->last_waiter_ = impl->last_waiter_->next_;
        }
        else
        {
          impl->first_waiter_ = ptr.get();
          impl->last_waiter_ = ptr.get();
        }
        ptr.release();
      }
    }
  }

  // Request the io_service to invoke the given handler and return immediately.
  template <typename Handler>
  void post(implementation_type& impl, Handler handler)
  {
    // Allocate and construct an object to wrap the handler.
    typedef handler_wrapper<Handler> value_type;
    typedef handler_alloc_traits<Handler, value_type> alloc_traits;
    raw_handler_ptr<alloc_traits> raw_ptr(handler);
    handler_ptr<alloc_traits> ptr(raw_ptr, handler);

    boost::asio::detail::mutex::scoped_lock lock(impl->mutex_);

    if (impl->current_handler_ == 0)
    {
      // This handler now has the lock, so can be dispatched immediately.
      impl->current_handler_ = ptr.release();
      lock.unlock();
      this->get_io_service().post(invoke_current_handler(*this, impl));
    }
    else
    {
      // Another handler already holds the lock, so this handler must join the
      // list of waiters. The handler will be posted automatically when its turn
      // comes.
      if (impl->last_waiter_)
      {
        impl->last_waiter_->next_ = ptr.get();
        impl->last_waiter_ = impl->last_waiter_->next_;
      }
      else
      {
        impl->first_waiter_ = ptr.get();
        impl->last_waiter_ = ptr.get();
      }
      ptr.release();
    }
  }

private:
  // Mutex to protect access to the linked list of implementations. 
  boost::asio::detail::mutex mutex_;

  // The head of a linked list of all implementations.
  strand_impl* impl_list_;
};

} // namespace detail
} // namespace asio
} // namespace boost

#if defined(__BORLANDC__)

namespace boost {

inline void intrusive_ptr_add_ref(
    boost::asio::detail::strand_service::strand_impl* p)
{
  p->add_ref();
}

inline void intrusive_ptr_release(
    boost::asio::detail::strand_service::strand_impl* p)
{
  p->release();
}

} // namespace boost

#endif // defined(__BORLANDC__)

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_STRAND_SERVICE_HPP
