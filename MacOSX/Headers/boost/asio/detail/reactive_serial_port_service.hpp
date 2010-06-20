//
// reactive_serial_port_service.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Copyright (c) 2008 Rep Invariant Systems, Inc. (info@repinvariant.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_REACTIVE_SERIAL_PORT_SERVICE_HPP
#define BOOST_ASIO_DETAIL_REACTIVE_SERIAL_PORT_SERVICE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <cstring>
#include <string>
#include <boost/asio/detail/pop_options.hpp>

#if !defined(BOOST_WINDOWS) && !defined(__CYGWIN__)

#include <boost/asio/detail/push_options.hpp>
#include <termios.h>
#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/error.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/detail/descriptor_ops.hpp>
#include <boost/asio/detail/reactive_descriptor_service.hpp>

namespace boost {
namespace asio {
namespace detail {

// Extend reactive_descriptor_service to provide serial port support.
template <typename Reactor>
class reactive_serial_port_service
  : public boost::asio::detail::service_base<
      reactive_serial_port_service<Reactor> >
{
public:
  // The native type of a stream handle.
  typedef typename reactive_descriptor_service<Reactor>::native_type
    native_type;

  // The implementation type of the stream handle.
  typedef typename reactive_descriptor_service<Reactor>::implementation_type
    implementation_type;

  reactive_serial_port_service(boost::asio::io_service& io_service)
    : boost::asio::detail::service_base<
        reactive_serial_port_service>(io_service),
      descriptor_service_(boost::asio::use_service<
          reactive_descriptor_service<Reactor> >(io_service))
  {
  }

  // Destroy all user-defined handler objects owned by the service.
  void shutdown_service()
  {
  }

  // Construct a new handle implementation.
  void construct(implementation_type& impl)
  {
    descriptor_service_.construct(impl);
  }

  // Destroy a handle implementation.
  void destroy(implementation_type& impl)
  {
    descriptor_service_.destroy(impl);
  }

  // Open the serial port using the specified device name.
  boost::system::error_code open(implementation_type& impl,
      const std::string& device, boost::system::error_code& ec)
  {
    if (is_open(impl))
    {
      ec = boost::asio::error::already_open;
      return ec;
    }

    int fd = descriptor_ops::open(device.c_str(),
        O_RDWR | O_NONBLOCK | O_NOCTTY, ec);
    if (fd < 0)
      return ec;

    int s = descriptor_ops::fcntl(fd, F_GETFL, ec);
    if (s >= 0)
      s = descriptor_ops::fcntl(fd, F_SETFL, s | O_NONBLOCK, ec);
    if (s < 0)
    {
      boost::system::error_code ignored_ec;
      descriptor_ops::close(fd, ignored_ec);
      return ec;
    }
  
    // Set up default serial port options.
    termios ios;
    descriptor_ops::clear_error(ec);
    s = descriptor_ops::error_wrapper(::tcgetattr(fd, &ios), ec);
    if (s >= 0)
    {
#if defined(_BSD_SOURCE)
      ::cfmakeraw(&ios);
#else
      ios.c_iflag &= ~(IGNBRK | BRKINT | PARMRK
          | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
      ios.c_oflag &= ~OPOST;
      ios.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
      ios.c_cflag &= ~(CSIZE | PARENB);
      ios.c_cflag |= CS8;
#endif
      ios.c_iflag |= IGNPAR;
      ios.c_cflag |= CREAD | CLOCAL;
      descriptor_ops::clear_error(ec);
      s = descriptor_ops::error_wrapper(::tcsetattr(fd, TCSANOW, &ios), ec);
    }
    if (s < 0)
    {
      boost::system::error_code ignored_ec;
      descriptor_ops::close(fd, ignored_ec);
      return ec;
    }
  
    // We're done. Take ownership of the serial port descriptor.
    if (descriptor_service_.assign(impl, fd, ec))
    {
      boost::system::error_code ignored_ec;
      descriptor_ops::close(fd, ignored_ec);
    }

    return ec;
  }

  // Assign a native handle to a handle implementation.
  boost::system::error_code assign(implementation_type& impl,
      const native_type& native_descriptor, boost::system::error_code& ec)
  {
    return descriptor_service_.assign(impl, native_descriptor, ec);
  }

  // Determine whether the handle is open.
  bool is_open(const implementation_type& impl) const
  {
    return descriptor_service_.is_open(impl);
  }

  // Destroy a handle implementation.
  boost::system::error_code close(implementation_type& impl,
      boost::system::error_code& ec)
  {
    return descriptor_service_.close(impl, ec);
  }

  // Get the native handle representation.
  native_type native(implementation_type& impl)
  {
    return descriptor_service_.native(impl);
  }

  // Cancel all operations associated with the handle.
  boost::system::error_code cancel(implementation_type& impl,
      boost::system::error_code& ec)
  {
    return descriptor_service_.cancel(impl, ec);
  }

  // Set an option on the serial port.
  template <typename SettableSerialPortOption>
  boost::system::error_code set_option(implementation_type& impl,
      const SettableSerialPortOption& option, boost::system::error_code& ec)
  {
    termios ios;
    descriptor_ops::clear_error(ec);
    descriptor_ops::error_wrapper(::tcgetattr(
          descriptor_service_.native(impl), &ios), ec);
    if (ec)
      return ec;

    if (option.store(ios, ec))
      return ec;

    descriptor_ops::clear_error(ec);
    descriptor_ops::error_wrapper(::tcsetattr(
          descriptor_service_.native(impl), TCSANOW, &ios), ec);
    return ec;
  }

  // Get an option from the serial port.
  template <typename GettableSerialPortOption>
  boost::system::error_code get_option(const implementation_type& impl,
      GettableSerialPortOption& option, boost::system::error_code& ec) const
  {
    termios ios;
    descriptor_ops::clear_error(ec);
    descriptor_ops::error_wrapper(::tcgetattr(
          descriptor_service_.native(impl), &ios), ec);
    if (ec)
      return ec;

    return option.load(ios, ec);
  }

  // Send a break sequence to the serial port.
  boost::system::error_code send_break(implementation_type& impl,
      boost::system::error_code& ec)
  {
    descriptor_ops::clear_error(ec);
    descriptor_ops::error_wrapper(::tcsendbreak(
          descriptor_service_.native(impl), 0), ec);
    return ec;
  }

  // Write the given data. Returns the number of bytes sent.
  template <typename ConstBufferSequence>
  size_t write_some(implementation_type& impl,
      const ConstBufferSequence& buffers, boost::system::error_code& ec)
  {
    return descriptor_service_.write_some(impl, buffers, ec);
  }

  // Start an asynchronous write. The data being written must be valid for the
  // lifetime of the asynchronous operation.
  template <typename ConstBufferSequence, typename Handler>
  void async_write_some(implementation_type& impl,
      const ConstBufferSequence& buffers, Handler handler)
  {
    descriptor_service_.async_write_some(impl, buffers, handler);
  }

  // Read some data. Returns the number of bytes received.
  template <typename MutableBufferSequence>
  size_t read_some(implementation_type& impl,
      const MutableBufferSequence& buffers, boost::system::error_code& ec)
  {
    return descriptor_service_.read_some(impl, buffers, ec);
  }

  // Start an asynchronous read. The buffer for the data being received must be
  // valid for the lifetime of the asynchronous operation.
  template <typename MutableBufferSequence, typename Handler>
  void async_read_some(implementation_type& impl,
      const MutableBufferSequence& buffers, Handler handler)
  {
    descriptor_service_.async_read_some(impl, buffers, handler);
  }

private:
  // The handle service used for initiating asynchronous operations.
  reactive_descriptor_service<Reactor>& descriptor_service_;
};

} // namespace detail
} // namespace asio
} // namespace boost

#endif // !defined(BOOST_WINDOWS) && !defined(__CYGWIN__)

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_REACTIVE_SERIAL_PORT_SERVICE_HPP
