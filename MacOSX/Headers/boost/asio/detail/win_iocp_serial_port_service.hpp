//
// win_iocp_serial_port_service.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Copyright (c) 2008 Rep Invariant Systems, Inc. (info@repinvariant.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_WIN_IOCP_SERIAL_PORT_SERVICE_HPP
#define BOOST_ASIO_DETAIL_WIN_IOCP_SERIAL_PORT_SERVICE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <cstring>
#include <string>
#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/detail/win_iocp_io_service_fwd.hpp>

#if defined(BOOST_ASIO_HAS_IOCP)

#include <boost/asio/error.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/detail/win_iocp_handle_service.hpp>

namespace boost {
namespace asio {
namespace detail {

// Extend win_iocp_handle_service to provide serial port support.
class win_iocp_serial_port_service
  : public boost::asio::detail::service_base<win_iocp_serial_port_service>
{
public:
  // The native type of a stream handle.
  typedef win_iocp_handle_service::native_type native_type;

  // The implementation type of the stream handle.
  typedef win_iocp_handle_service::implementation_type implementation_type;

  win_iocp_serial_port_service(boost::asio::io_service& io_service)
    : boost::asio::detail::service_base<
        win_iocp_serial_port_service>(io_service),
      handle_service_(
          boost::asio::use_service<win_iocp_handle_service>(io_service))
  {
  }

  // Destroy all user-defined handler objects owned by the service.
  void shutdown_service()
  {
  }

  // Construct a new handle implementation.
  void construct(implementation_type& impl)
  {
    handle_service_.construct(impl);
  }

  // Destroy a handle implementation.
  void destroy(implementation_type& impl)
  {
    handle_service_.destroy(impl);
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

    // For convenience, add a leading \\.\ sequence if not already present.
    std::string name = (device[0] == '\\') ? device : "\\\\.\\" + device;

    // Open a handle to the serial port.
    ::HANDLE handle = ::CreateFileA(name.c_str(),
        GENERIC_READ | GENERIC_WRITE, 0, 0,
        OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
    if (handle == INVALID_HANDLE_VALUE)
    {
      DWORD last_error = ::GetLastError();
      ec = boost::system::error_code(last_error,
          boost::asio::error::get_system_category());
      return ec;
    }

    // Determine the initial serial port parameters.
    using namespace std; // For memcpy.
    ::DCB dcb;
    memset(&dcb, 0, sizeof(DCB));
    dcb.DCBlength = sizeof(DCB);
    if (!::GetCommState(handle, &dcb))
    {
      DWORD last_error = ::GetLastError();
      ::CloseHandle(handle);
      ec = boost::system::error_code(last_error,
          boost::asio::error::get_system_category());
      return ec;
    }

    // Set some default serial port parameters. This implementation does not
    // support changing these, so they might as well be in a known state.
    dcb.fBinary = TRUE; // Win32 only supports binary mode.
    dcb.fDsrSensitivity = FALSE;
    dcb.fNull = FALSE; // Do not ignore NULL characters.
    dcb.fAbortOnError = FALSE; // Ignore serial framing errors.
    if (!::SetCommState(handle, &dcb))
    {
      DWORD last_error = ::GetLastError();
      ::CloseHandle(handle);
      ec = boost::system::error_code(last_error,
          boost::asio::error::get_system_category());
      return ec;
    }

    // Set up timeouts so that the serial port will behave similarly to a
    // network socket. Reads wait for at least one byte, then return with
    // whatever they have. Writes return once everything is out the door.
    ::COMMTIMEOUTS timeouts;
    timeouts.ReadIntervalTimeout = 1;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.ReadTotalTimeoutConstant = 0;
    timeouts.WriteTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 0;
    if (!::SetCommTimeouts(handle, &timeouts))
    {
      DWORD last_error = ::GetLastError();
      ::CloseHandle(handle);
      ec = boost::system::error_code(last_error,
          boost::asio::error::get_system_category());
      return ec;
    }

    // We're done. Take ownership of the serial port handle.
    if (handle_service_.assign(impl, handle, ec))
      ::CloseHandle(handle);
    return ec;
  }

  // Assign a native handle to a handle implementation.
  boost::system::error_code assign(implementation_type& impl,
      const native_type& native_handle, boost::system::error_code& ec)
  {
    return handle_service_.assign(impl, native_handle, ec);
  }

  // Determine whether the handle is open.
  bool is_open(const implementation_type& impl) const
  {
    return handle_service_.is_open(impl);
  }

  // Destroy a handle implementation.
  boost::system::error_code close(implementation_type& impl,
      boost::system::error_code& ec)
  {
    return handle_service_.close(impl, ec);
  }

  // Get the native handle representation.
  native_type native(implementation_type& impl)
  {
    return handle_service_.native(impl);
  }

  // Cancel all operations associated with the handle.
  boost::system::error_code cancel(implementation_type& impl,
      boost::system::error_code& ec)
  {
    return handle_service_.cancel(impl, ec);
  }

  // Set an option on the serial port.
  template <typename SettableSerialPortOption>
  boost::system::error_code set_option(implementation_type& impl,
      const SettableSerialPortOption& option, boost::system::error_code& ec)
  {
    using namespace std; // For memcpy.

    ::DCB dcb;
    memset(&dcb, 0, sizeof(DCB));
    dcb.DCBlength = sizeof(DCB);
    if (!::GetCommState(handle_service_.native(impl), &dcb))
    {
      DWORD last_error = ::GetLastError();
      ec = boost::system::error_code(last_error,
          boost::asio::error::get_system_category());
      return ec;
    }

    if (option.store(dcb, ec))
      return ec;

    if (!::SetCommState(handle_service_.native(impl), &dcb))
    {
      DWORD last_error = ::GetLastError();
      ec = boost::system::error_code(last_error,
          boost::asio::error::get_system_category());
      return ec;
    }

    ec = boost::system::error_code();
    return ec;
  }

  // Get an option from the serial port.
  template <typename GettableSerialPortOption>
  boost::system::error_code get_option(const implementation_type& impl,
      GettableSerialPortOption& option, boost::system::error_code& ec) const
  {
    using namespace std; // For memcpy.

    ::DCB dcb;
    memset(&dcb, 0, sizeof(DCB));
    dcb.DCBlength = sizeof(DCB);
    if (!::GetCommState(handle_service_.native(impl), &dcb))
    {
      DWORD last_error = ::GetLastError();
      ec = boost::system::error_code(last_error,
          boost::asio::error::get_system_category());
      return ec;
    }

    return option.load(dcb, ec);
  }

  // Send a break sequence to the serial port.
  boost::system::error_code send_break(implementation_type& impl,
      boost::system::error_code& ec)
  {
    ec = boost::asio::error::operation_not_supported;
    return ec;
  }

  // Write the given data. Returns the number of bytes sent.
  template <typename ConstBufferSequence>
  size_t write_some(implementation_type& impl,
      const ConstBufferSequence& buffers, boost::system::error_code& ec)
  {
    return handle_service_.write_some(impl, buffers, ec);
  }

  // Start an asynchronous write. The data being written must be valid for the
  // lifetime of the asynchronous operation.
  template <typename ConstBufferSequence, typename Handler>
  void async_write_some(implementation_type& impl,
      const ConstBufferSequence& buffers, Handler handler)
  {
    handle_service_.async_write_some(impl, buffers, handler);
  }

  // Read some data. Returns the number of bytes received.
  template <typename MutableBufferSequence>
  size_t read_some(implementation_type& impl,
      const MutableBufferSequence& buffers, boost::system::error_code& ec)
  {
    return handle_service_.read_some(impl, buffers, ec);
  }

  // Start an asynchronous read. The buffer for the data being received must be
  // valid for the lifetime of the asynchronous operation.
  template <typename MutableBufferSequence, typename Handler>
  void async_read_some(implementation_type& impl,
      const MutableBufferSequence& buffers, Handler handler)
  {
    handle_service_.async_read_some(impl, buffers, handler);
  }

private:
  // The handle service used for initiating asynchronous operations.
  win_iocp_handle_service& handle_service_;
};

} // namespace detail
} // namespace asio
} // namespace boost

#endif // defined(BOOST_ASIO_HAS_IOCP)

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_WIN_IOCP_SERIAL_PORT_SERVICE_HPP
