//
// reactor_op_queue.hpp
// ~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_REACTOR_OP_QUEUE_HPP
#define BOOST_ASIO_DETAIL_REACTOR_OP_QUEUE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <memory>
#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/error.hpp>
#include <boost/asio/detail/handler_alloc_helpers.hpp>
#include <boost/asio/detail/hash_map.hpp>
#include <boost/asio/detail/noncopyable.hpp>

namespace boost {
namespace asio {
namespace detail {

template <typename Descriptor>
class reactor_op_queue
  : private noncopyable
{
public:
  // Constructor.
  reactor_op_queue()
    : operations_(),
      cancelled_operations_(0),
      complete_operations_(0)
  {
  }

  // Add a new operation to the queue. Returns true if this is the only
  // operation for the given descriptor, in which case the reactor's event
  // demultiplexing function call may need to be interrupted and restarted.
  template <typename Operation>
  bool enqueue_operation(Descriptor descriptor, Operation operation)
  {
    // Allocate and construct an object to wrap the handler.
    typedef handler_alloc_traits<Operation, op<Operation> > alloc_traits;
    raw_handler_ptr<alloc_traits> raw_ptr(operation);
    handler_ptr<alloc_traits> ptr(raw_ptr, descriptor, operation);

    typedef typename operation_map::iterator iterator;
    typedef typename operation_map::value_type value_type;
    std::pair<iterator, bool> entry =
      operations_.insert(value_type(descriptor, ptr.get()));
    if (entry.second)
    {
      ptr.release();
      return true;
    }

    op_base* current_op = entry.first->second;
    while (current_op->next_)
      current_op = current_op->next_;
    current_op->next_ = ptr.release();

    return false;
  }

  // Cancel all operations associated with the descriptor. Any operations
  // pending for the descriptor will be notified that they have been cancelled
  // next time perform_cancellations is called. Returns true if any operations
  // were cancelled, in which case the reactor's event demultiplexing function
  // may need to be interrupted and restarted.
  bool cancel_operations(Descriptor descriptor)
  {
    typename operation_map::iterator i = operations_.find(descriptor);
    if (i != operations_.end())
    {
      op_base* last_op = i->second;
      while (last_op->next_)
        last_op = last_op->next_;
      last_op->next_ = cancelled_operations_;
      cancelled_operations_ = i->second;
      operations_.erase(i);
      return true;
    }

    return false;
  }

  // Whether there are no operations in the queue.
  bool empty() const
  {
    return operations_.empty();
  }

  // Determine whether there are any operations associated with the descriptor.
  bool has_operation(Descriptor descriptor) const
  {
    return operations_.find(descriptor) != operations_.end();
  }

  // Perform the first operation corresponding to the descriptor. Returns true
  // if there are more operations queued for the descriptor.
  bool perform_operation(Descriptor descriptor,
      const boost::system::error_code& result)
  {
    typename operation_map::iterator i = operations_.find(descriptor);
    if (i != operations_.end())
    {
      op_base* this_op = i->second;
      i->second = this_op->next_;
      this_op->next_ = complete_operations_;
      complete_operations_ = this_op;
      bool done = this_op->perform(result);
      if (done)
      {
        // Operation has finished.
        if (i->second)
        {
          return true;
        }
        else
        {
          operations_.erase(i);
          return false;
        }
      }
      else
      {
        // Operation wants to be called again. Leave it at the front of the
        // queue for this descriptor, and remove from the completed list.
        complete_operations_ = this_op->next_;
        this_op->next_ = i->second;
        i->second = this_op;
        return true;
      }
    }
    return false;
  }

  // Perform all operations corresponding to the descriptor.
  void perform_all_operations(Descriptor descriptor,
      const boost::system::error_code& result)
  {
    typename operation_map::iterator i = operations_.find(descriptor);
    if (i != operations_.end())
    {
      while (i->second)
      {
        op_base* this_op = i->second;
        i->second = this_op->next_;
        this_op->next_ = complete_operations_;
        complete_operations_ = this_op;
        bool done = this_op->perform(result);
        if (!done)
        {
          // Operation has not finished yet, so leave at front of queue, and
          // remove from the completed list.
          complete_operations_ = this_op->next_;
          this_op->next_ = i->second;
          i->second = this_op;
          return;
        }
      }
      operations_.erase(i);
    }
  }

  // Fill a descriptor set with the descriptors corresponding to each active
  // operation.
  template <typename Descriptor_Set>
  void get_descriptors(Descriptor_Set& descriptors)
  {
    typename operation_map::iterator i = operations_.begin();
    while (i != operations_.end())
    {
      Descriptor descriptor = i->first;
      ++i;
      if (!descriptors.set(descriptor))
      {
        boost::system::error_code ec(error::fd_set_failure);
        perform_all_operations(descriptor, ec);
      }
    }
  }

  // Perform the operations corresponding to the ready file descriptors
  // contained in the given descriptor set.
  template <typename Descriptor_Set>
  void perform_operations_for_descriptors(const Descriptor_Set& descriptors,
      const boost::system::error_code& result)
  {
    typename operation_map::iterator i = operations_.begin();
    while (i != operations_.end())
    {
      typename operation_map::iterator op_iter = i++;
      if (descriptors.is_set(op_iter->first))
      {
        op_base* this_op = op_iter->second;
        op_iter->second = this_op->next_;
        this_op->next_ = complete_operations_;
        complete_operations_ = this_op;
        bool done = this_op->perform(result);
        if (done)
        {
          if (!op_iter->second)
            operations_.erase(op_iter);
        }
        else
        {
          // Operation has not finished yet, so leave at front of queue, and
          // remove from the completed list.
          complete_operations_ = this_op->next_;
          this_op->next_ = op_iter->second;
          op_iter->second = this_op;
        }
      }
    }
  }

  // Perform any pending cancels for operations.
  void perform_cancellations()
  {
    while (cancelled_operations_)
    {
      op_base* this_op = cancelled_operations_;
      cancelled_operations_ = this_op->next_;
      this_op->next_ = complete_operations_;
      complete_operations_ = this_op;
      this_op->perform(boost::asio::error::operation_aborted);
    }
  }

  // Complete all operations that are waiting to be completed.
  void complete_operations()
  {
    while (complete_operations_)
    {
      op_base* next_op = complete_operations_->next_;
      complete_operations_->next_ = 0;
      complete_operations_->complete();
      complete_operations_ = next_op;
    }
  }

  // Destroy all operations owned by the queue.
  void destroy_operations()
  {
    while (cancelled_operations_)
    {
      op_base* next_op = cancelled_operations_->next_;
      cancelled_operations_->next_ = 0;
      cancelled_operations_->destroy();
      cancelled_operations_ = next_op;
    }

    while (complete_operations_)
    {
      op_base* next_op = complete_operations_->next_;
      complete_operations_->next_ = 0;
      complete_operations_->destroy();
      complete_operations_ = next_op;
    }

    typename operation_map::iterator i = operations_.begin();
    while (i != operations_.end())
    {
      typename operation_map::iterator op_iter = i++;
      op_base* curr_op = op_iter->second;
      operations_.erase(op_iter);
      while (curr_op)
      {
        op_base* next_op = curr_op->next_;
        curr_op->next_ = 0;
        curr_op->destroy();
        curr_op = next_op;
      }
    }
  }

private:
  // Base class for reactor operations. A function pointer is used instead of
  // virtual functions to avoid the associated overhead.
  class op_base
  {
  public:
    // Get the descriptor associated with the operation.
    Descriptor descriptor() const
    {
      return descriptor_;
    }

    // Perform the operation.
    bool perform(const boost::system::error_code& result)
    {
      result_ = result;
      return perform_func_(this, result_, bytes_transferred_);
    }

    // Destroy the operation and post the handler.
    void complete()
    {
      complete_func_(this, result_, bytes_transferred_);
    }

    // Destroy the operation.
    void destroy()
    {
      destroy_func_(this);
    }

  protected:
    typedef bool (*perform_func_type)(op_base*,
        boost::system::error_code&, std::size_t&);
    typedef void (*complete_func_type)(op_base*,
        const boost::system::error_code&, std::size_t);
    typedef void (*destroy_func_type)(op_base*);

    // Construct an operation for the given descriptor.
    op_base(perform_func_type perform_func, complete_func_type complete_func,
        destroy_func_type destroy_func, Descriptor descriptor)
      : perform_func_(perform_func),
        complete_func_(complete_func),
        destroy_func_(destroy_func),
        descriptor_(descriptor),
        result_(),
        bytes_transferred_(0),
        next_(0)
    {
    }

    // Prevent deletion through this type.
    ~op_base()
    {
    }

  private:
    friend class reactor_op_queue<Descriptor>;

    // The function to be called to perform the operation.
    perform_func_type perform_func_;

    // The function to be called to delete the operation and post the handler.
    complete_func_type complete_func_;

    // The function to be called to delete the operation.
    destroy_func_type destroy_func_;

    // The descriptor associated with the operation.
    Descriptor descriptor_;

    // The result of the operation.
    boost::system::error_code result_;

    // The number of bytes transferred in the operation.
    std::size_t bytes_transferred_;

    // The next operation for the same file descriptor.
    op_base* next_;
  };

  // Adaptor class template for operations.
  template <typename Operation>
  class op
    : public op_base
  {
  public:
    // Constructor.
    op(Descriptor descriptor, Operation operation)
      : op_base(&op<Operation>::do_perform, &op<Operation>::do_complete,
          &op<Operation>::do_destroy, descriptor),
        operation_(operation)
    {
    }

    // Perform the operation.
    static bool do_perform(op_base* base,
        boost::system::error_code& result, std::size_t& bytes_transferred)
    {
      return static_cast<op<Operation>*>(base)->operation_.perform(
          result, bytes_transferred);
    }

    // Destroy the operation and post the handler.
    static void do_complete(op_base* base,
        const boost::system::error_code& result, std::size_t bytes_transferred)
    {
      // Take ownership of the operation object.
      typedef op<Operation> this_type;
      this_type* this_op(static_cast<this_type*>(base));
      typedef handler_alloc_traits<Operation, this_type> alloc_traits;
      handler_ptr<alloc_traits> ptr(this_op->operation_, this_op);

      // Make a copy of the error_code and the operation so that the memory can
      // be deallocated before the upcall is made.
      boost::system::error_code ec(result);
      Operation operation(this_op->operation_);

      // Free the memory associated with the operation.
      ptr.reset();

      // Make the upcall.
      operation.complete(ec, bytes_transferred);
    }

    // Destroy the operation.
    static void do_destroy(op_base* base)
    {
      // Take ownership of the operation object.
      typedef op<Operation> this_type;
      this_type* this_op(static_cast<this_type*>(base));
      typedef handler_alloc_traits<Operation, this_type> alloc_traits;
      handler_ptr<alloc_traits> ptr(this_op->operation_, this_op);

      // A sub-object of the operation may be the true owner of the memory
      // associated with the operation. Consequently, a local copy of the
      // operation is required to ensure that any owning sub-object remains
      // valid until after we have deallocated the memory here.
      Operation operation(this_op->operation_);
      (void)operation;

      // Free the memory associated with the operation.
      ptr.reset();
    }

  private:
    Operation operation_;
  };

  // The type for a map of operations.
  typedef hash_map<Descriptor, op_base*> operation_map;

  // The operations that are currently executing asynchronously.
  operation_map operations_;

  // The list of operations that have been cancelled.
  op_base* cancelled_operations_;

  // The list of operations waiting to be completed.
  op_base* complete_operations_;
};

} // namespace detail
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_REACTOR_OP_QUEUE_HPP
