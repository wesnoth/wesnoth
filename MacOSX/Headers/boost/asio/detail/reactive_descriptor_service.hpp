//
// reactive_descriptor_service.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_REACTIVE_DESCRIPTOR_SERVICE_HPP
#define BOOST_ASIO_DETAIL_REACTIVE_DESCRIPTOR_SERVICE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/buffer.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/detail/bind_handler.hpp>
#include <boost/asio/detail/handler_base_from_member.hpp>
#include <boost/asio/detail/noncopyable.hpp>
#include <boost/asio/detail/service_base.hpp>
#include <boost/asio/detail/descriptor_ops.hpp>

#if !defined(BOOST_WINDOWS) && !defined(__CYGWIN__)

namespace boost {
namespace asio {
namespace detail {

template <typename Reactor>
class reactive_descriptor_service
  : public boost::asio::detail::service_base<
      reactive_descriptor_service<Reactor> >
{
public:
  // The native type of a descriptor.
  typedef int native_type;

  // The implementation type of the descriptor.
  class implementation_type
    : private boost::asio::detail::noncopyable
  {
  public:
    // Default constructor.
    implementation_type()
      : descriptor_(-1),
        flags_(0)
    {
    }

  private:
    // Only this service will have access to the internal values.
    friend class reactive_descriptor_service<Reactor>;

    // The native descriptor representation.
    int descriptor_;

    enum
    {
      user_set_non_blocking = 1, // The user wants a non-blocking descriptor.
      internal_non_blocking = 2  // The descriptor has been set non-blocking.
    };

    // Flags indicating the current state of the descriptor.
    unsigned char flags_;

    // Per-descriptor data used by the reactor.
    typename Reactor::per_descriptor_data reactor_data_;
  };

  // The maximum number of buffers to support in a single operation.
  enum { max_buffers = 64 < max_iov_len ? 64 : max_iov_len };

  // Constructor.
  reactive_descriptor_service(boost::asio::io_service& io_service)
    : boost::asio::detail::service_base<
        reactive_descriptor_service<Reactor> >(io_service),
      reactor_(boost::asio::use_service<Reactor>(io_service))
  {
    reactor_.init_task();
  }

  // Destroy all user-defined handler objects owned by the service.
  void shutdown_service()
  {
  }

  // Construct a new descriptor implementation.
  void construct(implementation_type& impl)
  {
    impl.descriptor_ = -1;
    impl.flags_ = 0;
  }

  // Destroy a descriptor implementation.
  void destroy(implementation_type& impl)
  {
    if (impl.descriptor_ != -1)
    {
      reactor_.close_descriptor(impl.descriptor_, impl.reactor_data_);

      if (impl.flags_ & implementation_type::internal_non_blocking)
      {
        ioctl_arg_type non_blocking = 0;
        boost::system::error_code ignored_ec;
        descriptor_ops::ioctl(impl.descriptor_,
            FIONBIO, &non_blocking, ignored_ec);
        impl.flags_ &= ~implementation_type::internal_non_blocking;
      }

      boost::system::error_code ignored_ec;
      descriptor_ops::close(impl.descriptor_, ignored_ec);

      impl.descriptor_ = -1;
    }
  }

  // Assign a native descriptor to a descriptor implementation.
  boost::system::error_code assign(implementation_type& impl,
      const native_type& native_descriptor, boost::system::error_code& ec)
  {
    if (is_open(impl))
    {
      ec = boost::asio::error::already_open;
      return ec;
    }

    if (int err = reactor_.register_descriptor(
          native_descriptor, impl.reactor_data_))
    {
      ec = boost::system::error_code(err,
          boost::asio::error::get_system_category());
      return ec;
    }

    impl.descriptor_ = native_descriptor;
    impl.flags_ = 0;
    ec = boost::system::error_code();
    return ec;
  }

  // Determine whether the descriptor is open.
  bool is_open(const implementation_type& impl) const
  {
    return impl.descriptor_ != -1;
  }

  // Destroy a descriptor implementation.
  boost::system::error_code close(implementation_type& impl,
      boost::system::error_code& ec)
  {
    if (is_open(impl))
    {
      reactor_.close_descriptor(impl.descriptor_, impl.reactor_data_);

      if (impl.flags_ & implementation_type::internal_non_blocking)
      {
        ioctl_arg_type non_blocking = 0;
        boost::system::error_code ignored_ec;
        descriptor_ops::ioctl(impl.descriptor_,
            FIONBIO, &non_blocking, ignored_ec);
        impl.flags_ &= ~implementation_type::internal_non_blocking;
      }

      if (descriptor_ops::close(impl.descriptor_, ec) == -1)
        return ec;

      impl.descriptor_ = -1;
    }

    ec = boost::system::error_code();
    return ec;
  }

  // Get the native descriptor representation.
  native_type native(const implementation_type& impl) const
  {
    return impl.descriptor_;
  }

  // Cancel all operations associated with the descriptor.
  boost::system::error_code cancel(implementation_type& impl,
      boost::system::error_code& ec)
  {
    if (!is_open(impl))
    {
      ec = boost::asio::error::bad_descriptor;
      return ec;
    }

    reactor_.cancel_ops(impl.descriptor_, impl.reactor_data_);
    ec = boost::system::error_code();
    return ec;
  }

  // Perform an IO control command on the descriptor.
  template <typename IO_Control_Command>
  boost::system::error_code io_control(implementation_type& impl,
      IO_Control_Command& command, boost::system::error_code& ec)
  {
    if (!is_open(impl))
    {
      ec = boost::asio::error::bad_descriptor;
      return ec;
    }

    if (command.name() == static_cast<int>(FIONBIO))
    {
      if (command.get())
        impl.flags_ |= implementation_type::user_set_non_blocking;
      else
        impl.flags_ &= ~implementation_type::user_set_non_blocking;
      ec = boost::system::error_code();
    }
    else
    {
      descriptor_ops::ioctl(impl.descriptor_, command.name(),
          static_cast<ioctl_arg_type*>(command.data()), ec);
    }
    return ec;
  }

  // Write some data to the descriptor.
  template <typename ConstBufferSequence>
  size_t write_some(implementation_type& impl,
      const ConstBufferSequence& buffers, boost::system::error_code& ec)
  {
    if (!is_open(impl))
    {
      ec = boost::asio::error::bad_descriptor;
      return 0;
    }

    // Copy buffers into array.
    descriptor_ops::buf bufs[max_buffers];
    typename ConstBufferSequence::const_iterator iter = buffers.begin();
    typename ConstBufferSequence::const_iterator end = buffers.end();
    size_t i = 0;
    size_t total_buffer_size = 0;
    for (; iter != end && i < max_buffers; ++iter, ++i)
    {
      boost::asio::const_buffer buffer(*iter);
      descriptor_ops::init_buf(bufs[i],
          boost::asio::buffer_cast<const void*>(buffer),
          boost::asio::buffer_size(buffer));
      total_buffer_size += boost::asio::buffer_size(buffer);
    }

    // A request to read_some 0 bytes on a stream is a no-op.
    if (total_buffer_size == 0)
    {
      ec = boost::system::error_code();
      return 0;
    }

    // Make descriptor non-blocking if user wants non-blocking.
    if (impl.flags_ & implementation_type::user_set_non_blocking)
    {
      if (!(impl.flags_ & implementation_type::internal_non_blocking))
      {
        ioctl_arg_type non_blocking = 1;
        if (descriptor_ops::ioctl(impl.descriptor_,
              FIONBIO, &non_blocking, ec))
          return 0;
        impl.flags_ |= implementation_type::internal_non_blocking;
      }
    }

    // Send the data.
    for (;;)
    {
      // Try to complete the operation without blocking.
      int bytes_sent = descriptor_ops::gather_write(
          impl.descriptor_, bufs, i, ec);

      // Check if operation succeeded.
      if (bytes_sent >= 0)
        return bytes_sent;

      // Operation failed.
      if ((impl.flags_ & implementation_type::user_set_non_blocking)
          || (ec != boost::asio::error::would_block
            && ec != boost::asio::error::try_again))
        return 0;

      // Wait for descriptor to become ready.
      if (descriptor_ops::poll_write(impl.descriptor_, ec) < 0)
        return 0;
    }
  }

  // Wait until data can be written without blocking.
  size_t write_some(implementation_type& impl,
      const null_buffers&, boost::system::error_code& ec)
  {
    if (!is_open(impl))
    {
      ec = boost::asio::error::bad_descriptor;
      return 0;
    }

    // Wait for descriptor to become ready.
    descriptor_ops::poll_write(impl.descriptor_, ec);

    return 0;
  }

  template <typename ConstBufferSequence, typename Handler>
  class write_operation :
    public handler_base_from_member<Handler>
  {
  public:
    write_operation(int descriptor, boost::asio::io_service& io_service,
        const ConstBufferSequence& buffers, Handler handler)
      : handler_base_from_member<Handler>(handler),
        descriptor_(descriptor),
        io_service_(io_service),
        work_(io_service),
        buffers_(buffers)
    {
    }

    bool perform(boost::system::error_code& ec,
        std::size_t& bytes_transferred)
    {
      // Check whether the operation was successful.
      if (ec)
      {
        bytes_transferred = 0;
        return true;
      }

      // Copy buffers into array.
      descriptor_ops::buf bufs[max_buffers];
      typename ConstBufferSequence::const_iterator iter = buffers_.begin();
      typename ConstBufferSequence::const_iterator end = buffers_.end();
      size_t i = 0;
      for (; iter != end && i < max_buffers; ++iter, ++i)
      {
        boost::asio::const_buffer buffer(*iter);
        descriptor_ops::init_buf(bufs[i],
            boost::asio::buffer_cast<const void*>(buffer),
            boost::asio::buffer_size(buffer));
      }

      // Write the data.
      int bytes = descriptor_ops::gather_write(descriptor_, bufs, i, ec);

      // Check if we need to run the operation again.
      if (ec == boost::asio::error::would_block
          || ec == boost::asio::error::try_again)
        return false;

      bytes_transferred = (bytes < 0 ? 0 : bytes);
      return true;
    }

    void complete(const boost::system::error_code& ec,
        std::size_t bytes_transferred)
    {
      io_service_.post(bind_handler(this->handler_, ec, bytes_transferred));
    }

  private:
    int descriptor_;
    boost::asio::io_service& io_service_;
    boost::asio::io_service::work work_;
    ConstBufferSequence buffers_;
  };

  // Start an asynchronous write. The data being sent must be valid for the
  // lifetime of the asynchronous operation.
  template <typename ConstBufferSequence, typename Handler>
  void async_write_some(implementation_type& impl,
      const ConstBufferSequence& buffers, Handler handler)
  {
    if (!is_open(impl))
    {
      this->get_io_service().post(bind_handler(handler,
            boost::asio::error::bad_descriptor, 0));
    }
    else
    {
      // Determine total size of buffers.
      typename ConstBufferSequence::const_iterator iter = buffers.begin();
      typename ConstBufferSequence::const_iterator end = buffers.end();
      size_t i = 0;
      size_t total_buffer_size = 0;
      for (; iter != end && i < max_buffers; ++iter, ++i)
      {
        boost::asio::const_buffer buffer(*iter);
        total_buffer_size += boost::asio::buffer_size(buffer);
      }

      // A request to read_some 0 bytes on a stream is a no-op.
      if (total_buffer_size == 0)
      {
        this->get_io_service().post(bind_handler(handler,
              boost::system::error_code(), 0));
        return;
      }

      // Make descriptor non-blocking.
      if (!(impl.flags_ & implementation_type::internal_non_blocking))
      {
        ioctl_arg_type non_blocking = 1;
        boost::system::error_code ec;
        if (descriptor_ops::ioctl(impl.descriptor_, FIONBIO, &non_blocking, ec))
        {
          this->get_io_service().post(bind_handler(handler, ec, 0));
          return;
        }
        impl.flags_ |= implementation_type::internal_non_blocking;
      }

      reactor_.start_write_op(impl.descriptor_, impl.reactor_data_,
          write_operation<ConstBufferSequence, Handler>(
            impl.descriptor_, this->get_io_service(), buffers, handler));
    }
  }

  template <typename Handler>
  class null_buffers_operation :
    public handler_base_from_member<Handler>
  {
  public:
    null_buffers_operation(boost::asio::io_service& io_service, Handler handler)
      : handler_base_from_member<Handler>(handler),
        work_(io_service)
    {
    }

    bool perform(boost::system::error_code&,
        std::size_t& bytes_transferred)
    {
      bytes_transferred = 0;
      return true;
    }

    void complete(const boost::system::error_code& ec,
        std::size_t bytes_transferred)
    {
      work_.get_io_service().post(bind_handler(
            this->handler_, ec, bytes_transferred));
    }

  private:
    boost::asio::io_service::work work_;
  };

  // Start an asynchronous wait until data can be written without blocking.
  template <typename Handler>
  void async_write_some(implementation_type& impl,
      const null_buffers&, Handler handler)
  {
    if (!is_open(impl))
    {
      this->get_io_service().post(bind_handler(handler,
            boost::asio::error::bad_descriptor, 0));
    }
    else
    {
      reactor_.start_write_op(impl.descriptor_, impl.reactor_data_,
          null_buffers_operation<Handler>(this->get_io_service(), handler),
          false);
    }
  }

  // Read some data from the stream. Returns the number of bytes read.
  template <typename MutableBufferSequence>
  size_t read_some(implementation_type& impl,
      const MutableBufferSequence& buffers, boost::system::error_code& ec)
  {
    if (!is_open(impl))
    {
      ec = boost::asio::error::bad_descriptor;
      return 0;
    }

    // Copy buffers into array.
    descriptor_ops::buf bufs[max_buffers];
    typename MutableBufferSequence::const_iterator iter = buffers.begin();
    typename MutableBufferSequence::const_iterator end = buffers.end();
    size_t i = 0;
    size_t total_buffer_size = 0;
    for (; iter != end && i < max_buffers; ++iter, ++i)
    {
      boost::asio::mutable_buffer buffer(*iter);
      descriptor_ops::init_buf(bufs[i],
          boost::asio::buffer_cast<void*>(buffer),
          boost::asio::buffer_size(buffer));
      total_buffer_size += boost::asio::buffer_size(buffer);
    }

    // A request to read_some 0 bytes on a stream is a no-op.
    if (total_buffer_size == 0)
    {
      ec = boost::system::error_code();
      return 0;
    }

    // Make descriptor non-blocking if user wants non-blocking.
    if (impl.flags_ & implementation_type::user_set_non_blocking)
    {
      if (!(impl.flags_ & implementation_type::internal_non_blocking))
      {
        ioctl_arg_type non_blocking = 1;
        if (descriptor_ops::ioctl(impl.descriptor_, FIONBIO, &non_blocking, ec))
          return 0;
        impl.flags_ |= implementation_type::internal_non_blocking;
      }
    }

    // Read some data.
    for (;;)
    {
      // Try to complete the operation without blocking.
      int bytes_read = descriptor_ops::scatter_read(
          impl.descriptor_, bufs, i, ec);

      // Check if operation succeeded.
      if (bytes_read > 0)
        return bytes_read;

      // Check for EOF.
      if (bytes_read == 0)
      {
        ec = boost::asio::error::eof;
        return 0;
      }

      // Operation failed.
      if ((impl.flags_ & implementation_type::user_set_non_blocking)
          || (ec != boost::asio::error::would_block
            && ec != boost::asio::error::try_again))
        return 0;

      // Wait for descriptor to become ready.
      if (descriptor_ops::poll_read(impl.descriptor_, ec) < 0)
        return 0;
    }
  }

  // Wait until data can be read without blocking.
  size_t read_some(implementation_type& impl,
      const null_buffers&, boost::system::error_code& ec)
  {
    if (!is_open(impl))
    {
      ec = boost::asio::error::bad_descriptor;
      return 0;
    }

    // Wait for descriptor to become ready.
    descriptor_ops::poll_read(impl.descriptor_, ec);

    return 0;
  }

  template <typename MutableBufferSequence, typename Handler>
  class read_operation :
    public handler_base_from_member<Handler>
  {
  public:
    read_operation(int descriptor, boost::asio::io_service& io_service,
        const MutableBufferSequence& buffers, Handler handler)
      : handler_base_from_member<Handler>(handler),
        descriptor_(descriptor),
        io_service_(io_service),
        work_(io_service),
        buffers_(buffers)
    {
    }

    bool perform(boost::system::error_code& ec,
        std::size_t& bytes_transferred)
    {
      // Check whether the operation was successful.
      if (ec)
      {
        bytes_transferred = 0;
        return true;
      }

      // Copy buffers into array.
      descriptor_ops::buf bufs[max_buffers];
      typename MutableBufferSequence::const_iterator iter = buffers_.begin();
      typename MutableBufferSequence::const_iterator end = buffers_.end();
      size_t i = 0;
      for (; iter != end && i < max_buffers; ++iter, ++i)
      {
        boost::asio::mutable_buffer buffer(*iter);
        descriptor_ops::init_buf(bufs[i],
            boost::asio::buffer_cast<void*>(buffer),
            boost::asio::buffer_size(buffer));
      }

      // Read some data.
      int bytes = descriptor_ops::scatter_read(descriptor_, bufs, i, ec);
      if (bytes == 0)
        ec = boost::asio::error::eof;

      // Check if we need to run the operation again.
      if (ec == boost::asio::error::would_block
          || ec == boost::asio::error::try_again)
        return false;

      bytes_transferred = (bytes < 0 ? 0 : bytes);
      return true;
    }

    void complete(const boost::system::error_code& ec,
        std::size_t bytes_transferred)
    {
      io_service_.post(bind_handler(this->handler_, ec, bytes_transferred));
    }

  private:
    int descriptor_;
    boost::asio::io_service& io_service_;
    boost::asio::io_service::work work_;
    MutableBufferSequence buffers_;
  };

  // Start an asynchronous read. The buffer for the data being read must be
  // valid for the lifetime of the asynchronous operation.
  template <typename MutableBufferSequence, typename Handler>
  void async_read_some(implementation_type& impl,
      const MutableBufferSequence& buffers, Handler handler)
  {
    if (!is_open(impl))
    {
      this->get_io_service().post(bind_handler(handler,
            boost::asio::error::bad_descriptor, 0));
    }
    else
    {
      // Determine total size of buffers.
      typename MutableBufferSequence::const_iterator iter = buffers.begin();
      typename MutableBufferSequence::const_iterator end = buffers.end();
      size_t i = 0;
      size_t total_buffer_size = 0;
      for (; iter != end && i < max_buffers; ++iter, ++i)
      {
        boost::asio::mutable_buffer buffer(*iter);
        total_buffer_size += boost::asio::buffer_size(buffer);
      }

      // A request to read_some 0 bytes on a stream is a no-op.
      if (total_buffer_size == 0)
      {
        this->get_io_service().post(bind_handler(handler,
              boost::system::error_code(), 0));
        return;
      }

      // Make descriptor non-blocking.
      if (!(impl.flags_ & implementation_type::internal_non_blocking))
      {
        ioctl_arg_type non_blocking = 1;
        boost::system::error_code ec;
        if (descriptor_ops::ioctl(impl.descriptor_, FIONBIO, &non_blocking, ec))
        {
          this->get_io_service().post(bind_handler(handler, ec, 0));
          return;
        }
        impl.flags_ |= implementation_type::internal_non_blocking;
      }

      reactor_.start_read_op(impl.descriptor_, impl.reactor_data_,
          read_operation<MutableBufferSequence, Handler>(
            impl.descriptor_, this->get_io_service(), buffers, handler));
    }
  }

  // Wait until data can be read without blocking.
  template <typename Handler>
  void async_read_some(implementation_type& impl,
      const null_buffers&, Handler handler)
  {
    if (!is_open(impl))
    {
      this->get_io_service().post(bind_handler(handler,
            boost::asio::error::bad_descriptor, 0));
    }
    else
    {
      reactor_.start_read_op(impl.descriptor_, impl.reactor_data_,
          null_buffers_operation<Handler>(this->get_io_service(), handler),
          false);
    }
  }

private:
  // The selector that performs event demultiplexing for the service.
  Reactor& reactor_;
};

} // namespace detail
} // namespace asio
} // namespace boost

#endif // !defined(BOOST_WINDOWS) && !defined(__CYGWIN__)

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_REACTIVE_DESCRIPTOR_SERVICE_HPP
