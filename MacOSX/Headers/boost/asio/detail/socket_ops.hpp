//
// socket_ops.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_SOCKET_OPS_HPP
#define BOOST_ASIO_DETAIL_SOCKET_OPS_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <boost/config.hpp>
#include <boost/assert.hpp>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <boost/detail/workaround.hpp>
#include <new>
#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/error.hpp>
#include <boost/asio/detail/socket_types.hpp>

namespace boost {
namespace asio {
namespace detail {
namespace socket_ops {

#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
struct msghdr { int msg_namelen; };
#endif // defined(BOOST_WINDOWS) || defined(__CYGWIN__)

#if defined(__hpux)
// HP-UX doesn't declare these functions extern "C", so they are declared again
// here to avoid linker errors about undefined symbols.
extern "C" char* if_indextoname(unsigned int, char*);
extern "C" unsigned int if_nametoindex(const char*);
#endif // defined(__hpux)

inline void clear_error(boost::system::error_code& ec)
{
#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
  WSASetLastError(0);
#else
  errno = 0;
#endif
  ec = boost::system::error_code();
}

template <typename ReturnType>
inline ReturnType error_wrapper(ReturnType return_value,
    boost::system::error_code& ec)
{
#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
  ec = boost::system::error_code(WSAGetLastError(),
      boost::asio::error::get_system_category());
#else
  ec = boost::system::error_code(errno,
      boost::asio::error::get_system_category());
#endif
  return return_value;
}

template <typename SockLenType>
inline socket_type call_accept(SockLenType msghdr::*,
    socket_type s, socket_addr_type* addr, std::size_t* addrlen)
{
  SockLenType tmp_addrlen = addrlen ? (SockLenType)*addrlen : 0;
  socket_type result = ::accept(s, addr, addrlen ? &tmp_addrlen : 0);
  if (addrlen)
    *addrlen = (std::size_t)tmp_addrlen;
  return result;
}

inline socket_type accept(socket_type s, socket_addr_type* addr,
    std::size_t* addrlen, boost::system::error_code& ec)
{
  clear_error(ec);

  socket_type new_s = error_wrapper(call_accept(
        &msghdr::msg_namelen, s, addr, addrlen), ec);
  if (new_s == invalid_socket)
    return new_s;

#if defined(__MACH__) && defined(__APPLE__) || defined(__FreeBSD__)
  int optval = 1;
  int result = error_wrapper(::setsockopt(new_s,
        SOL_SOCKET, SO_NOSIGPIPE, &optval, sizeof(optval)), ec);
  if (result != 0)
  {
    ::close(new_s);
    return invalid_socket;
  }
#endif

#if defined(BOOST_WINDOWS) && defined(UNDER_CE)
  clear_error(ec);
#endif

  return new_s;
}

template <typename SockLenType>
inline int call_bind(SockLenType msghdr::*,
    socket_type s, const socket_addr_type* addr, std::size_t addrlen)
{
  return ::bind(s, addr, (SockLenType)addrlen);
}

inline int bind(socket_type s, const socket_addr_type* addr,
    std::size_t addrlen, boost::system::error_code& ec)
{
  clear_error(ec);
  int result = error_wrapper(call_bind(
        &msghdr::msg_namelen, s, addr, addrlen), ec);
#if defined(BOOST_WINDOWS) && defined(UNDER_CE)
  if (result == 0)
    clear_error(ec);
#endif
  return result;
}

inline int close(socket_type s, boost::system::error_code& ec)
{
  clear_error(ec);
#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
  int result = error_wrapper(::closesocket(s), ec);
# if defined(UNDER_CE)
  if (result == 0)
    clear_error(ec);
# endif
  return result;
#else // defined(BOOST_WINDOWS) || defined(__CYGWIN__)
  return error_wrapper(::close(s), ec);
#endif // defined(BOOST_WINDOWS) || defined(__CYGWIN__)
}

inline int shutdown(socket_type s, int what, boost::system::error_code& ec)
{
  clear_error(ec);
  int result = error_wrapper(::shutdown(s, what), ec);
#if defined(BOOST_WINDOWS) && defined(UNDER_CE)
  if (result == 0)
    clear_error(ec);
#endif
  return result;
}

template <typename SockLenType>
inline int call_connect(SockLenType msghdr::*,
    socket_type s, const socket_addr_type* addr, std::size_t addrlen)
{
  return ::connect(s, addr, (SockLenType)addrlen);
}

inline int connect(socket_type s, const socket_addr_type* addr,
    std::size_t addrlen, boost::system::error_code& ec)
{
  clear_error(ec);
  int result = error_wrapper(call_connect(
        &msghdr::msg_namelen, s, addr, addrlen), ec);
#if defined(BOOST_WINDOWS) && defined(UNDER_CE)
  if (result == 0)
    clear_error(ec);
#endif
  return result;
}

inline int socketpair(int af, int type, int protocol,
    socket_type sv[2], boost::system::error_code& ec)
{
#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
  (void)(af);
  (void)(type);
  (void)(protocol);
  (void)(sv);
  ec = boost::asio::error::operation_not_supported;
  return -1;
#else
  clear_error(ec);
  return error_wrapper(::socketpair(af, type, protocol, sv), ec);
#endif
}

inline int listen(socket_type s, int backlog, boost::system::error_code& ec)
{
  clear_error(ec);
  int result = error_wrapper(::listen(s, backlog), ec);
#if defined(BOOST_WINDOWS) && defined(UNDER_CE)
  if (result == 0)
    clear_error(ec);
#endif
  return result;
}

inline void init_buf_iov_base(void*& base, void* addr)
{
  base = addr;
}

template <typename T>
inline void init_buf_iov_base(T& base, void* addr)
{
  base = static_cast<T>(addr);
}

#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
typedef WSABUF buf;
#else // defined(BOOST_WINDOWS) || defined(__CYGWIN__)
typedef iovec buf;
#endif // defined(BOOST_WINDOWS) || defined(__CYGWIN__)

inline void init_buf(buf& b, void* data, size_t size)
{
#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
  b.buf = static_cast<char*>(data);
  b.len = static_cast<u_long>(size);
#else // defined(BOOST_WINDOWS) || defined(__CYGWIN__)
  init_buf_iov_base(b.iov_base, data);
  b.iov_len = size;
#endif // defined(BOOST_WINDOWS) || defined(__CYGWIN__)
}

inline void init_buf(buf& b, const void* data, size_t size)
{
#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
  b.buf = static_cast<char*>(const_cast<void*>(data));
  b.len = static_cast<u_long>(size);
#else // defined(BOOST_WINDOWS) || defined(__CYGWIN__)
  init_buf_iov_base(b.iov_base, const_cast<void*>(data));
  b.iov_len = size;
#endif // defined(BOOST_WINDOWS) || defined(__CYGWIN__)
}

inline void init_msghdr_msg_name(void*& name, socket_addr_type* addr)
{
  name = addr;
}

inline void init_msghdr_msg_name(void*& name, const socket_addr_type* addr)
{
  name = const_cast<socket_addr_type*>(addr);
}

template <typename T>
inline void init_msghdr_msg_name(T& name, socket_addr_type* addr)
{
  name = reinterpret_cast<T>(addr);
}

template <typename T>
inline void init_msghdr_msg_name(T& name, const socket_addr_type* addr)
{
  name = reinterpret_cast<T>(const_cast<socket_addr_type*>(addr));
}

inline int recv(socket_type s, buf* bufs, size_t count, int flags,
    boost::system::error_code& ec)
{
  clear_error(ec);
#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
  // Receive some data.
  DWORD recv_buf_count = static_cast<DWORD>(count);
  DWORD bytes_transferred = 0;
  DWORD recv_flags = flags;
  int result = error_wrapper(::WSARecv(s, bufs,
        recv_buf_count, &bytes_transferred, &recv_flags, 0, 0), ec);
  if (result != 0)
    return -1;
# if defined(UNDER_CE)
  clear_error(ec);
# endif
  return bytes_transferred;
#else // defined(BOOST_WINDOWS) || defined(__CYGWIN__)
  msghdr msg = msghdr();
  msg.msg_iov = bufs;
  msg.msg_iovlen = count;
  return error_wrapper(::recvmsg(s, &msg, flags), ec);
#endif // defined(BOOST_WINDOWS) || defined(__CYGWIN__)
}

inline int recvfrom(socket_type s, buf* bufs, size_t count, int flags,
    socket_addr_type* addr, std::size_t* addrlen,
    boost::system::error_code& ec)
{
  clear_error(ec);
#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
  // Receive some data.
  DWORD recv_buf_count = static_cast<DWORD>(count);
  DWORD bytes_transferred = 0;
  DWORD recv_flags = flags;
  int tmp_addrlen = (int)*addrlen;
  int result = error_wrapper(::WSARecvFrom(s, bufs, recv_buf_count,
        &bytes_transferred, &recv_flags, addr, &tmp_addrlen, 0, 0), ec);
  *addrlen = (std::size_t)tmp_addrlen;
  if (result != 0)
    return -1;
# if defined(UNDER_CE)
  clear_error(ec);
# endif
  return bytes_transferred;
#else // defined(BOOST_WINDOWS) || defined(__CYGWIN__)
  msghdr msg = msghdr();
  init_msghdr_msg_name(msg.msg_name, addr);
  msg.msg_namelen = *addrlen;
  msg.msg_iov = bufs;
  msg.msg_iovlen = count;
  int result = error_wrapper(::recvmsg(s, &msg, flags), ec);
  *addrlen = msg.msg_namelen;
  return result;
#endif // defined(BOOST_WINDOWS) || defined(__CYGWIN__)
}

inline int send(socket_type s, const buf* bufs, size_t count, int flags,
    boost::system::error_code& ec)
{
  clear_error(ec);
#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
  // Send the data.
  DWORD send_buf_count = static_cast<DWORD>(count);
  DWORD bytes_transferred = 0;
  DWORD send_flags = flags;
  int result = error_wrapper(::WSASend(s, const_cast<buf*>(bufs),
        send_buf_count, &bytes_transferred, send_flags, 0, 0), ec);
  if (result != 0)
    return -1;
# if defined(UNDER_CE)
  clear_error(ec);
# endif
  return bytes_transferred;
#else // defined(BOOST_WINDOWS) || defined(__CYGWIN__)
  msghdr msg = msghdr();
  msg.msg_iov = const_cast<buf*>(bufs);
  msg.msg_iovlen = count;
#if defined(__linux__)
  flags |= MSG_NOSIGNAL;
#endif // defined(__linux__)
  return error_wrapper(::sendmsg(s, &msg, flags), ec);
#endif // defined(BOOST_WINDOWS) || defined(__CYGWIN__)
}

inline int sendto(socket_type s, const buf* bufs, size_t count, int flags,
    const socket_addr_type* addr, std::size_t addrlen,
    boost::system::error_code& ec)
{
  clear_error(ec);
#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
  // Send the data.
  DWORD send_buf_count = static_cast<DWORD>(count);
  DWORD bytes_transferred = 0;
  int result = error_wrapper(::WSASendTo(s, const_cast<buf*>(bufs),
        send_buf_count, &bytes_transferred, flags, addr,
        static_cast<int>(addrlen), 0, 0), ec);
  if (result != 0)
    return -1;
# if defined(UNDER_CE)
  clear_error(ec);
# endif
  return bytes_transferred;
#else // defined(BOOST_WINDOWS) || defined(__CYGWIN__)
  msghdr msg = msghdr();
  init_msghdr_msg_name(msg.msg_name, addr);
  msg.msg_namelen = addrlen;
  msg.msg_iov = const_cast<buf*>(bufs);
  msg.msg_iovlen = count;
#if defined(__linux__)
  flags |= MSG_NOSIGNAL;
#endif // defined(__linux__)
  return error_wrapper(::sendmsg(s, &msg, flags), ec);
#endif // defined(BOOST_WINDOWS) || defined(__CYGWIN__)
}

inline socket_type socket(int af, int type, int protocol,
    boost::system::error_code& ec)
{
  clear_error(ec);
#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
  socket_type s = error_wrapper(::WSASocket(af, type, protocol, 0, 0,
        WSA_FLAG_OVERLAPPED), ec);
  if (s == invalid_socket)
    return s;

  if (af == AF_INET6)
  {
    // Try to enable the POSIX default behaviour of having IPV6_V6ONLY set to
    // false. This will only succeed on Windows Vista and later versions of
    // Windows, where a dual-stack IPv4/v6 implementation is available.
    DWORD optval = 0;
    ::setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY,
        reinterpret_cast<const char*>(&optval), sizeof(optval));
  }

# if defined(UNDER_CE)
  clear_error(ec);
# endif

  return s;
#elif defined(__MACH__) && defined(__APPLE__) || defined(__FreeBSD__)
  socket_type s = error_wrapper(::socket(af, type, protocol), ec);
  if (s == invalid_socket)
    return s;

  int optval = 1;
  int result = error_wrapper(::setsockopt(s,
        SOL_SOCKET, SO_NOSIGPIPE, &optval, sizeof(optval)), ec);
  if (result != 0)
  {
    ::close(s);
    return invalid_socket;
  }

  return s;
#else
  return error_wrapper(::socket(af, type, protocol), ec);
#endif
}

template <typename SockLenType>
inline int call_setsockopt(SockLenType msghdr::*,
    socket_type s, int level, int optname,
    const void* optval, std::size_t optlen)
{
  return ::setsockopt(s, level, optname,
      (const char*)optval, (SockLenType)optlen);
}

inline int setsockopt(socket_type s, int level, int optname,
    const void* optval, std::size_t optlen, boost::system::error_code& ec)
{
  if (level == custom_socket_option_level && optname == always_fail_option)
  {
    ec = boost::asio::error::invalid_argument;
    return -1;
  }

#if defined(__BORLANDC__)
  // Mysteriously, using the getsockopt and setsockopt functions directly with
  // Borland C++ results in incorrect values being set and read. The bug can be
  // worked around by using function addresses resolved with GetProcAddress.
  if (HMODULE winsock_module = ::GetModuleHandleA("ws2_32"))
  {
    typedef int (WSAAPI *sso_t)(SOCKET, int, int, const char*, int);
    if (sso_t sso = (sso_t)::GetProcAddress(winsock_module, "setsockopt"))
    {
      clear_error(ec);
      return error_wrapper(sso(s, level, optname,
            reinterpret_cast<const char*>(optval),
            static_cast<int>(optlen)), ec);
    }
  }
  ec = boost::asio::error::fault;
  return -1;
#else // defined(__BORLANDC__)
  clear_error(ec);
  int result = error_wrapper(call_setsockopt(&msghdr::msg_namelen,
        s, level, optname, optval, optlen), ec);
# if defined(BOOST_WINDOWS) && defined(UNDER_CE)
  if (result == 0)
    clear_error(ec);
# endif
  return result;
#endif // defined(__BORLANDC__)
}

template <typename SockLenType>
inline int call_getsockopt(SockLenType msghdr::*,
    socket_type s, int level, int optname,
    void* optval, std::size_t* optlen)
{
  SockLenType tmp_optlen = (SockLenType)*optlen;
  int result = ::getsockopt(s, level, optname, (char*)optval, &tmp_optlen);
  *optlen = (std::size_t)tmp_optlen;
  return result;
}

inline int getsockopt(socket_type s, int level, int optname, void* optval,
    size_t* optlen, boost::system::error_code& ec)
{
  if (level == custom_socket_option_level && optname == always_fail_option)
  {
    ec = boost::asio::error::invalid_argument;
    return -1;
  }

#if defined(__BORLANDC__)
  // Mysteriously, using the getsockopt and setsockopt functions directly with
  // Borland C++ results in incorrect values being set and read. The bug can be
  // worked around by using function addresses resolved with GetProcAddress.
  if (HMODULE winsock_module = ::GetModuleHandleA("ws2_32"))
  {
    typedef int (WSAAPI *gso_t)(SOCKET, int, int, char*, int*);
    if (gso_t gso = (gso_t)::GetProcAddress(winsock_module, "getsockopt"))
    {
      clear_error(ec);
      int tmp_optlen = static_cast<int>(*optlen);
      int result = error_wrapper(gso(s, level, optname,
            reinterpret_cast<char*>(optval), &tmp_optlen), ec);
      *optlen = static_cast<size_t>(tmp_optlen);
      if (result != 0 && level == IPPROTO_IPV6 && optname == IPV6_V6ONLY
          && ec.value() == WSAENOPROTOOPT && *optlen == sizeof(DWORD))
      {
        // Dual-stack IPv4/v6 sockets, and the IPV6_V6ONLY socket option, are
        // only supported on Windows Vista and later. To simplify program logic
        // we will fake success of getting this option and specify that the
        // value is non-zero (i.e. true). This corresponds to the behavior of
        // IPv6 sockets on Windows platforms pre-Vista.
        *static_cast<DWORD*>(optval) = 1;
        clear_error(ec);
      }
      return result;
    }
  }
  ec = boost::asio::error::fault;
  return -1;
#elif defined(BOOST_WINDOWS) || defined(__CYGWIN__)
  clear_error(ec);
  int result = error_wrapper(call_getsockopt(&msghdr::msg_namelen,
        s, level, optname, optval, optlen), ec);
  if (result != 0 && level == IPPROTO_IPV6 && optname == IPV6_V6ONLY
      && ec.value() == WSAENOPROTOOPT && *optlen == sizeof(DWORD))
  {
    // Dual-stack IPv4/v6 sockets, and the IPV6_V6ONLY socket option, are only
    // supported on Windows Vista and later. To simplify program logic we will
    // fake success of getting this option and specify that the value is
    // non-zero (i.e. true). This corresponds to the behavior of IPv6 sockets
    // on Windows platforms pre-Vista.
    *static_cast<DWORD*>(optval) = 1;
    clear_error(ec);
  }
# if defined(UNDER_CE)
  if (result == 0)
    clear_error(ec);
# endif
  return result;
#else // defined(BOOST_WINDOWS) || defined(__CYGWIN__)
  clear_error(ec);
  int result = error_wrapper(call_getsockopt(&msghdr::msg_namelen,
        s, level, optname, optval, optlen), ec);
#if defined(__linux__)
  if (result == 0 && level == SOL_SOCKET && *optlen == sizeof(int)
      && (optname == SO_SNDBUF || optname == SO_RCVBUF))
  {
    // On Linux, setting SO_SNDBUF or SO_RCVBUF to N actually causes the kernel
    // to set the buffer size to N*2. Linux puts additional stuff into the
    // buffers so that only about half is actually available to the application.
    // The retrieved value is divided by 2 here to make it appear as though the
    // correct value has been set.
    *static_cast<int*>(optval) /= 2;
  }
#endif // defined(__linux__)
  return result;
#endif // defined(BOOST_WINDOWS) || defined(__CYGWIN__)
}

template <typename SockLenType>
inline int call_getpeername(SockLenType msghdr::*,
    socket_type s, socket_addr_type* addr, std::size_t* addrlen)
{
  SockLenType tmp_addrlen = (SockLenType)*addrlen;
  int result = ::getpeername(s, addr, &tmp_addrlen);
  *addrlen = (std::size_t)tmp_addrlen;
  return result;
}

inline int getpeername(socket_type s, socket_addr_type* addr,
    std::size_t* addrlen, boost::system::error_code& ec)
{
  clear_error(ec);
  int result = error_wrapper(call_getpeername(
        &msghdr::msg_namelen, s, addr, addrlen), ec);
#if defined(BOOST_WINDOWS) && defined(UNDER_CE)
  if (result == 0)
    clear_error(ec);
#endif
  return result;
}

template <typename SockLenType>
inline int call_getsockname(SockLenType msghdr::*,
    socket_type s, socket_addr_type* addr, std::size_t* addrlen)
{
  SockLenType tmp_addrlen = (SockLenType)*addrlen;
  int result = ::getsockname(s, addr, &tmp_addrlen);
  *addrlen = (std::size_t)tmp_addrlen;
  return result;
}

inline int getsockname(socket_type s, socket_addr_type* addr,
    std::size_t* addrlen, boost::system::error_code& ec)
{
  clear_error(ec);
  int result = error_wrapper(call_getsockname(
        &msghdr::msg_namelen, s, addr, addrlen), ec);
#if defined(BOOST_WINDOWS) && defined(UNDER_CE)
  if (result == 0)
    clear_error(ec);
#endif
  return result;
}

inline int ioctl(socket_type s, long cmd, ioctl_arg_type* arg,
    boost::system::error_code& ec)
{
  clear_error(ec);
#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
  int result = error_wrapper(::ioctlsocket(s, cmd, arg), ec);
# if defined(UNDER_CE)
  if (result == 0)
    clear_error(ec);
# endif
  return result;
#else // defined(BOOST_WINDOWS) || defined(__CYGWIN__)
  return error_wrapper(::ioctl(s, cmd, arg), ec);
#endif // defined(BOOST_WINDOWS) || defined(__CYGWIN__)
}

inline int select(int nfds, fd_set* readfds, fd_set* writefds,
    fd_set* exceptfds, timeval* timeout, boost::system::error_code& ec)
{
  clear_error(ec);
#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
  if (!readfds && !writefds && !exceptfds && timeout)
  {
    DWORD milliseconds = timeout->tv_sec * 1000 + timeout->tv_usec / 1000;
    if (milliseconds == 0)
      milliseconds = 1; // Force context switch.
    ::Sleep(milliseconds);
    ec = boost::system::error_code();
    return 0;
  }

  // The select() call allows timeout values measured in microseconds, but the
  // system clock (as wrapped by boost::posix_time::microsec_clock) typically
  // has a resolution of 10 milliseconds. This can lead to a spinning select
  // reactor, meaning increased CPU usage, when waiting for the earliest
  // scheduled timeout if it's less than 10 milliseconds away. To avoid a tight
  // spin we'll use a minimum timeout of 1 millisecond.
  if (timeout && timeout->tv_sec == 0
      && timeout->tv_usec > 0 && timeout->tv_usec < 1000)
    timeout->tv_usec = 1000;
#endif // defined(BOOST_WINDOWS) || defined(__CYGWIN__)

#if defined(__hpux) && defined(__HP_aCC)
  timespec ts;
  ts.tv_sec = timeout ? timeout->tv_sec : 0;
  ts.tv_nsec = timeout ? timeout->tv_usec * 1000 : 0;
  return error_wrapper(::pselect(nfds, readfds,
        writefds, exceptfds, timeout ? &ts : 0, 0), ec);
#else
  int result = error_wrapper(::select(nfds, readfds,
        writefds, exceptfds, timeout), ec);
# if defined(BOOST_WINDOWS) && defined(UNDER_CE)
  if (result >= 0)
    clear_error(ec);
# endif
  return result;
#endif
}

inline int poll_read(socket_type s, boost::system::error_code& ec)
{
#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
  FD_SET fds;
  FD_ZERO(&fds);
  FD_SET(s, &fds);
  clear_error(ec);
  int result = error_wrapper(::select(s, &fds, 0, 0, 0), ec);
# if defined(UNDER_CE)
  if (result >= 0)
    clear_error(ec);
# endif
  return result;
#else // defined(BOOST_WINDOWS) || defined(__CYGWIN__)
  pollfd fds;
  fds.fd = s;
  fds.events = POLLIN;
  fds.revents = 0;
  clear_error(ec);
  return error_wrapper(::poll(&fds, 1, -1), ec);
#endif // defined(BOOST_WINDOWS) || defined(__CYGWIN__)
}

inline int poll_write(socket_type s, boost::system::error_code& ec)
{
#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
  FD_SET fds;
  FD_ZERO(&fds);
  FD_SET(s, &fds);
  clear_error(ec);
  int result = error_wrapper(::select(s, 0, &fds, 0, 0), ec);
# if defined(UNDER_CE)
  if (result >= 0)
    clear_error(ec);
# endif
  return result;
#else // defined(BOOST_WINDOWS) || defined(__CYGWIN__)
  pollfd fds;
  fds.fd = s;
  fds.events = POLLOUT;
  fds.revents = 0;
  clear_error(ec);
  return error_wrapper(::poll(&fds, 1, -1), ec);
#endif // defined(BOOST_WINDOWS) || defined(__CYGWIN__)
}

inline int poll_connect(socket_type s, boost::system::error_code& ec)
{
#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
  FD_SET write_fds;
  FD_ZERO(&write_fds);
  FD_SET(s, &write_fds);
  FD_SET except_fds;
  FD_ZERO(&except_fds);
  FD_SET(s, &except_fds);
  clear_error(ec);
  int result = error_wrapper(::select(s, 0, &write_fds, &except_fds, 0), ec);
# if defined(UNDER_CE)
  if (result >= 0)
    clear_error(ec);
# endif
  return result;
#else // defined(BOOST_WINDOWS) || defined(__CYGWIN__)
  pollfd fds;
  fds.fd = s;
  fds.events = POLLOUT;
  fds.revents = 0;
  clear_error(ec);
  return error_wrapper(::poll(&fds, 1, -1), ec);
#endif // defined(BOOST_WINDOWS) || defined(__CYGWIN__)
}

inline const char* inet_ntop(int af, const void* src, char* dest, size_t length,
    unsigned long scope_id, boost::system::error_code& ec)
{
  clear_error(ec);
#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
  using namespace std; // For memcpy.

  if (af != AF_INET && af != AF_INET6)
  {
    ec = boost::asio::error::address_family_not_supported;
    return 0;
  }

  union
  {
    socket_addr_type base;
    sockaddr_storage_type storage;
    sockaddr_in4_type v4;
    sockaddr_in6_type v6;
  } address;
  DWORD address_length;
  if (af == AF_INET)
  {
    address_length = sizeof(sockaddr_in4_type);
    address.v4.sin_family = AF_INET;
    address.v4.sin_port = 0;
    memcpy(&address.v4.sin_addr, src, sizeof(in4_addr_type));
  }
  else // AF_INET6
  {
    address_length = sizeof(sockaddr_in6_type);
    address.v6.sin6_family = AF_INET6;
    address.v6.sin6_port = 0;
    address.v6.sin6_flowinfo = 0;
    address.v6.sin6_scope_id = scope_id;
    memcpy(&address.v6.sin6_addr, src, sizeof(in6_addr_type));
  }

  DWORD string_length = static_cast<DWORD>(length);
#if defined(BOOST_NO_ANSI_APIS)
  LPWSTR string_buffer = (LPWSTR)_alloca(length * sizeof(WCHAR));
  int result = error_wrapper(::WSAAddressToStringW(&address.base,
        address_length, 0, string_buffer, &string_length), ec);
  ::WideCharToMultiByte(CP_ACP, 0, string_buffer, -1, dest, length, 0, 0);
#else
  int result = error_wrapper(::WSAAddressToStringA(
        &address.base, address_length, 0, dest, &string_length), ec);
#endif

  // Windows may set error code on success.
  if (result != socket_error_retval)
    clear_error(ec);

  // Windows may not set an error code on failure.
  else if (result == socket_error_retval && !ec)
    ec = boost::asio::error::invalid_argument;

  return result == socket_error_retval ? 0 : dest;
#else // defined(BOOST_WINDOWS) || defined(__CYGWIN__)
  const char* result = error_wrapper(::inet_ntop(af, src, dest, length), ec);
  if (result == 0 && !ec)
    ec = boost::asio::error::invalid_argument;
  if (result != 0 && af == AF_INET6 && scope_id != 0)
  {
    using namespace std; // For strcat and sprintf.
    char if_name[IF_NAMESIZE + 1] = "%";
    const in6_addr_type* ipv6_address = static_cast<const in6_addr_type*>(src);
    bool is_link_local = IN6_IS_ADDR_LINKLOCAL(ipv6_address);
    if (!is_link_local || if_indextoname(scope_id, if_name + 1) == 0)
      sprintf(if_name + 1, "%lu", scope_id);
    strcat(dest, if_name);
  }
  return result;
#endif // defined(BOOST_WINDOWS) || defined(__CYGWIN__)
}

inline int inet_pton(int af, const char* src, void* dest,
    unsigned long* scope_id, boost::system::error_code& ec)
{
  clear_error(ec);
#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
  using namespace std; // For memcpy and strcmp.

  if (af != AF_INET && af != AF_INET6)
  {
    ec = boost::asio::error::address_family_not_supported;
    return -1;
  }

  union
  {
    socket_addr_type base;
    sockaddr_storage_type storage;
    sockaddr_in4_type v4;
    sockaddr_in6_type v6;
  } address;
  int address_length = sizeof(sockaddr_storage_type);
#if defined(BOOST_NO_ANSI_APIS)
  int num_wide_chars = strlen(src) + 1;
  LPWSTR wide_buffer = (LPWSTR)_alloca(num_wide_chars * sizeof(WCHAR));
  ::MultiByteToWideChar(CP_ACP, 0, src, -1, wide_buffer, num_wide_chars);
  int result = error_wrapper(::WSAStringToAddressW(
        wide_buffer, af, 0, &address.base, &address_length), ec);
#else
  int result = error_wrapper(::WSAStringToAddressA(
        const_cast<char*>(src), af, 0, &address.base, &address_length), ec);
#endif

  if (af == AF_INET)
  {
    if (result != socket_error_retval)
    {
      memcpy(dest, &address.v4.sin_addr, sizeof(in4_addr_type));
      clear_error(ec);
    }
    else if (strcmp(src, "255.255.255.255") == 0)
    {
      static_cast<in4_addr_type*>(dest)->s_addr = INADDR_NONE;
      clear_error(ec);
    }
  }
  else // AF_INET6
  {
    if (result != socket_error_retval)
    {
      memcpy(dest, &address.v6.sin6_addr, sizeof(in6_addr_type));
      if (scope_id)
        *scope_id = address.v6.sin6_scope_id;
      clear_error(ec);
    }
  }

  // Windows may not set an error code on failure.
  if (result == socket_error_retval && !ec)
    ec = boost::asio::error::invalid_argument;

#if defined(UNDER_CE)
  if (result != socket_error_retval)
    clear_error(ec);
#endif

  return result == socket_error_retval ? -1 : 1;
#else // defined(BOOST_WINDOWS) || defined(__CYGWIN__)
  int result = error_wrapper(::inet_pton(af, src, dest), ec);
  if (result <= 0 && !ec)
    ec = boost::asio::error::invalid_argument;
  if (result > 0 && af == AF_INET6 && scope_id)
  {
    using namespace std; // For strchr and atoi.
    *scope_id = 0;
    if (const char* if_name = strchr(src, '%'))
    {
      in6_addr_type* ipv6_address = static_cast<in6_addr_type*>(dest);
      bool is_link_local = IN6_IS_ADDR_LINKLOCAL(ipv6_address);
      if (is_link_local)
        *scope_id = if_nametoindex(if_name + 1);
      if (*scope_id == 0)
        *scope_id = atoi(if_name + 1);
    }
  }
  return result;
#endif // defined(BOOST_WINDOWS) || defined(__CYGWIN__)
}

inline int gethostname(char* name, int namelen, boost::system::error_code& ec)
{
  clear_error(ec);
  int result = error_wrapper(::gethostname(name, namelen), ec);
#if defined(BOOST_WINDOWS) && defined(UNDER_CE)
  if (result == 0)
    clear_error(ec);
#endif
  return result;
}

#if defined(BOOST_WINDOWS) || defined(__CYGWIN__) \
  || defined(__MACH__) && defined(__APPLE__)

// The following functions are only needed for emulation of getaddrinfo and
// getnameinfo.

inline boost::system::error_code translate_netdb_error(int error)
{
  switch (error)
  {
  case 0:
    return boost::system::error_code();
  case HOST_NOT_FOUND:
    return boost::asio::error::host_not_found;
  case TRY_AGAIN:
    return boost::asio::error::host_not_found_try_again;
  case NO_RECOVERY:
    return boost::asio::error::no_recovery;
  case NO_DATA:
    return boost::asio::error::no_data;
  default:
    BOOST_ASSERT(false);
    return boost::asio::error::invalid_argument;
  }
}

inline hostent* gethostbyaddr(const char* addr, int length, int af,
    hostent* result, char* buffer, int buflength, boost::system::error_code& ec)
{
  clear_error(ec);
#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
  (void)(buffer);
  (void)(buflength);
  hostent* retval = error_wrapper(::gethostbyaddr(addr, length, af), ec);
  if (!retval)
    return 0;
# if defined(UNDER_CE)
  clear_error(ec);
# endif
  *result = *retval;
  return retval;
#elif defined(__sun) || defined(__QNX__)
  int error = 0;
  hostent* retval = error_wrapper(::gethostbyaddr_r(addr, length, af, result,
        buffer, buflength, &error), ec);
  if (error)
    ec = translate_netdb_error(error);
  return retval;
#elif defined(__MACH__) && defined(__APPLE__)
  (void)(buffer);
  (void)(buflength);
  int error = 0;
  hostent* retval = error_wrapper(::getipnodebyaddr(
        addr, length, af, &error), ec);
  if (error)
    ec = translate_netdb_error(error);
  if (!retval)
    return 0;
  *result = *retval;
  return retval;
#else
  hostent* retval = 0;
  int error = 0;
  error_wrapper(::gethostbyaddr_r(addr, length, af, result, buffer,
        buflength, &retval, &error), ec);
  if (error)
    ec = translate_netdb_error(error);
  return retval;
#endif
}

inline hostent* gethostbyname(const char* name, int af, struct hostent* result,
    char* buffer, int buflength, int ai_flags, boost::system::error_code& ec)
{
  clear_error(ec);
#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
  (void)(buffer);
  (void)(buflength);
  (void)(ai_flags);
  if (af != AF_INET)
  {
    ec = boost::asio::error::address_family_not_supported;
    return 0;
  }
  hostent* retval = error_wrapper(::gethostbyname(name), ec);
  if (!retval)
    return 0;
# if defined(UNDER_CE)
  clear_error(ec);
# endif
  *result = *retval;
  return result;
#elif defined(__sun) || defined(__QNX__)
  (void)(ai_flags);
  if (af != AF_INET)
  {
    ec = boost::asio::error::address_family_not_supported;
    return 0;
  }
  int error = 0;
  hostent* retval = error_wrapper(::gethostbyname_r(name, result, buffer,
        buflength, &error), ec);
  if (error)
    ec = translate_netdb_error(error);
  return retval;
#elif defined(__MACH__) && defined(__APPLE__)
  (void)(buffer);
  (void)(buflength);
  int error = 0;
  hostent* retval = error_wrapper(::getipnodebyname(
        name, af, ai_flags, &error), ec);
  if (error)
    ec = translate_netdb_error(error);
  if (!retval)
    return 0;
  *result = *retval;
  return retval;
#else
  (void)(ai_flags);
  if (af != AF_INET)
  {
    ec = boost::asio::error::address_family_not_supported;
    return 0;
  }
  hostent* retval = 0;
  int error = 0;
  error_wrapper(::gethostbyname_r(name, result,
        buffer, buflength, &retval, &error), ec);
  if (error)
    ec = translate_netdb_error(error);
  return retval;
#endif
}

inline void freehostent(hostent* h)
{
#if defined(__MACH__) && defined(__APPLE__)
  if (h)
    ::freehostent(h);
#else
  (void)(h);
#endif
}

// Emulation of getaddrinfo based on implementation in:
// Stevens, W. R., UNIX Network Programming Vol. 1, 2nd Ed., Prentice-Hall 1998.

struct gai_search
{
  const char* host;
  int family;
};

inline int gai_nsearch(const char* host,
    const addrinfo_type* hints, gai_search (&search)[2])
{
  int search_count = 0;
  if (host == 0 || host[0] == '\0')
  {
    if (hints->ai_flags & AI_PASSIVE)
    {
      // No host and AI_PASSIVE implies wildcard bind.
      switch (hints->ai_family)
      {
      case AF_INET:
        search[search_count].host = "0.0.0.0";
        search[search_count].family = AF_INET;
        ++search_count;
        break;
      case AF_INET6:
        search[search_count].host = "0::0";
        search[search_count].family = AF_INET6;
        ++search_count;
        break;
      case AF_UNSPEC:
        search[search_count].host = "0::0";
        search[search_count].family = AF_INET6;
        ++search_count;
        search[search_count].host = "0.0.0.0";
        search[search_count].family = AF_INET;
        ++search_count;
        break;
      default:
        break;
      }
    }
    else
    {
      // No host and not AI_PASSIVE means connect to local host.
      switch (hints->ai_family)
      {
      case AF_INET:
        search[search_count].host = "localhost";
        search[search_count].family = AF_INET;
        ++search_count;
        break;
      case AF_INET6:
        search[search_count].host = "localhost";
        search[search_count].family = AF_INET6;
        ++search_count;
        break;
      case AF_UNSPEC:
        search[search_count].host = "localhost";
        search[search_count].family = AF_INET6;
        ++search_count;
        search[search_count].host = "localhost";
        search[search_count].family = AF_INET;
        ++search_count;
        break;
      default:
        break;
      }
    }
  }
  else
  {
    // Host is specified.
    switch (hints->ai_family)
    {
    case AF_INET:
      search[search_count].host = host;
      search[search_count].family = AF_INET;
      ++search_count;
      break;
    case AF_INET6:
      search[search_count].host = host;
      search[search_count].family = AF_INET6;
      ++search_count;
      break;
    case AF_UNSPEC:
      search[search_count].host = host;
      search[search_count].family = AF_INET6;
      ++search_count;
      search[search_count].host = host;
      search[search_count].family = AF_INET;
      ++search_count;
      break;
    default:
      break;
    }
  }
  return search_count;
}

template <typename T>
inline T* gai_alloc(std::size_t size = sizeof(T))
{
  using namespace std;
  T* p = static_cast<T*>(::operator new(size, std::nothrow));
  if (p)
    memset(p, 0, size);
  return p;
}

inline void gai_free(void* p)
{
  ::operator delete(p);
}

inline void gai_strcpy(char* target, const char* source, std::size_t max_size)
{
  using namespace std;
#if BOOST_WORKAROUND(BOOST_MSVC, >= 1400) && !defined(UNDER_CE)
  strcpy_s(target, max_size, source);
#else
  *target = 0;
  strncat(target, source, max_size);
#endif
}

enum { gai_clone_flag = 1 << 30 };

inline int gai_aistruct(addrinfo_type*** next, const addrinfo_type* hints,
    const void* addr, int family)
{
  using namespace std;

  addrinfo_type* ai = gai_alloc<addrinfo_type>();
  if (ai == 0)
    return EAI_MEMORY;

  ai->ai_next = 0;
  **next = ai;
  *next = &ai->ai_next;

  ai->ai_canonname = 0;
  ai->ai_socktype = hints->ai_socktype;
  if (ai->ai_socktype == 0)
    ai->ai_flags |= gai_clone_flag;
  ai->ai_protocol = hints->ai_protocol;
  ai->ai_family = family;

  switch (ai->ai_family)
  {
  case AF_INET:
    {
      sockaddr_in4_type* sinptr = gai_alloc<sockaddr_in4_type>();
      if (sinptr == 0)
        return EAI_MEMORY;
      sinptr->sin_family = AF_INET;
      memcpy(&sinptr->sin_addr, addr, sizeof(in4_addr_type));
      ai->ai_addr = reinterpret_cast<sockaddr*>(sinptr);
      ai->ai_addrlen = sizeof(sockaddr_in4_type);
      break;
    }
  case AF_INET6:
    {
      sockaddr_in6_type* sin6ptr = gai_alloc<sockaddr_in6_type>();
      if (sin6ptr == 0)
        return EAI_MEMORY;
      sin6ptr->sin6_family = AF_INET6;
      memcpy(&sin6ptr->sin6_addr, addr, sizeof(in6_addr_type));
      ai->ai_addr = reinterpret_cast<sockaddr*>(sin6ptr);
      ai->ai_addrlen = sizeof(sockaddr_in6_type);
      break;
    }
  default:
    break;
  }

  return 0;
}

inline addrinfo_type* gai_clone(addrinfo_type* ai)
{
  using namespace std;

  addrinfo_type* new_ai = gai_alloc<addrinfo_type>();
  if (new_ai == 0)
    return new_ai;

  new_ai->ai_next = ai->ai_next;
  ai->ai_next = new_ai;

  new_ai->ai_flags = 0;
  new_ai->ai_family = ai->ai_family;
  new_ai->ai_socktype = ai->ai_socktype;
  new_ai->ai_protocol = ai->ai_protocol;
  new_ai->ai_canonname = 0;
  new_ai->ai_addrlen = ai->ai_addrlen;
  new_ai->ai_addr = gai_alloc<sockaddr>(ai->ai_addrlen);
  memcpy(new_ai->ai_addr, ai->ai_addr, ai->ai_addrlen);

  return new_ai;
}

inline int gai_port(addrinfo_type* aihead, int port, int socktype)
{
  int num_found = 0;

  for (addrinfo_type* ai = aihead; ai; ai = ai->ai_next)
  {
    if (ai->ai_flags & gai_clone_flag)
    {
      if (ai->ai_socktype != 0)
      {
        ai = gai_clone(ai);
        if (ai == 0)
          return -1;
        // ai now points to newly cloned entry.
      }
    }
    else if (ai->ai_socktype != socktype)
    {
      // Ignore if mismatch on socket type.
      continue;
    }

    ai->ai_socktype = socktype;

    switch (ai->ai_family)
    {
    case AF_INET:
      {
        sockaddr_in4_type* sinptr =
          reinterpret_cast<sockaddr_in4_type*>(ai->ai_addr);
        sinptr->sin_port = port;
        ++num_found;
        break;
      }
    case AF_INET6:
      {
        sockaddr_in6_type* sin6ptr =
          reinterpret_cast<sockaddr_in6_type*>(ai->ai_addr);
        sin6ptr->sin6_port = port;
        ++num_found;
        break;
      }
    default:
      break;
    }
  }

  return num_found;
}

inline int gai_serv(addrinfo_type* aihead,
    const addrinfo_type* hints, const char* serv)
{
  using namespace std;

  int num_found = 0;

  if (
#if defined(AI_NUMERICSERV)
      (hints->ai_flags & AI_NUMERICSERV) ||
#endif
      isdigit(serv[0]))
  {
    int port = htons(atoi(serv));
    if (hints->ai_socktype)
    {
      // Caller specifies socket type.
      int rc = gai_port(aihead, port, hints->ai_socktype);
      if (rc < 0)
        return EAI_MEMORY;
      num_found += rc;
    }
    else
    {
      // Caller does not specify socket type.
      int rc = gai_port(aihead, port, SOCK_STREAM);
      if (rc < 0)
        return EAI_MEMORY;
      num_found += rc;
      rc = gai_port(aihead, port, SOCK_DGRAM);
      if (rc < 0)
        return EAI_MEMORY;
      num_found += rc;
    }
  }
  else
  {
    // Try service name with TCP first, then UDP.
    if (hints->ai_socktype == 0 || hints->ai_socktype == SOCK_STREAM)
    {
      servent* sptr = getservbyname(serv, "tcp");
      if (sptr != 0)
      {
        int rc = gai_port(aihead, sptr->s_port, SOCK_STREAM);
        if (rc < 0)
          return EAI_MEMORY;
        num_found += rc;
      }
    }
    if (hints->ai_socktype == 0 || hints->ai_socktype == SOCK_DGRAM)
    {
      servent* sptr = getservbyname(serv, "udp");
      if (sptr != 0)
      {
        int rc = gai_port(aihead, sptr->s_port, SOCK_DGRAM);
        if (rc < 0)
          return EAI_MEMORY;
        num_found += rc;
      }
    }
  }

  if (num_found == 0)
  {
    if (hints->ai_socktype == 0)
    {
      // All calls to getservbyname() failed.
      return EAI_NONAME;
    }
    else
    {
      // Service not supported for socket type.
      return EAI_SERVICE;
    }
  }

  return 0;
}

inline int gai_echeck(const char* host, const char* service,
    int flags, int family, int socktype, int protocol)
{
  (void)(flags);
  (void)(protocol);

  // Host or service must be specified.
  if (host == 0 || host[0] == '\0')
    if (service == 0 || service[0] == '\0')
      return EAI_NONAME;

  // Check combination of family and socket type.
  switch (family)
  {
  case AF_UNSPEC:
    break;
  case AF_INET:
  case AF_INET6:
    if (socktype != 0 && socktype != SOCK_STREAM && socktype != SOCK_DGRAM)
      return EAI_SOCKTYPE;
    break;
  default:
    return EAI_FAMILY;
  }

  return 0;
}

inline void freeaddrinfo_emulation(addrinfo_type* aihead)
{
  addrinfo_type* ai = aihead;
  while (ai)
  {
    gai_free(ai->ai_addr);
    gai_free(ai->ai_canonname);
    addrinfo_type* ainext = ai->ai_next;
    gai_free(ai);
    ai = ainext;
  }
}

inline int getaddrinfo_emulation(const char* host, const char* service,
    const addrinfo_type* hintsp, addrinfo_type** result)
{
  // Set up linked list of addrinfo structures.
  addrinfo_type* aihead = 0;
  addrinfo_type** ainext = &aihead;
  char* canon = 0;

  // Supply default hints if not specified by caller.
  addrinfo_type hints = addrinfo_type();
  hints.ai_family = AF_UNSPEC;
  if (hintsp)
    hints = *hintsp;

  // If the resolution is not specifically for AF_INET6, remove the AI_V4MAPPED
  // and AI_ALL flags.
#if defined(AI_V4MAPPED)
  if (hints.ai_family != AF_INET6)
    hints.ai_flags &= ~AI_V4MAPPED;
#endif
#if defined(AI_ALL)
  if (hints.ai_family != AF_INET6)
    hints.ai_flags &= ~AI_ALL;
#endif

  // Basic error checking.
  int rc = gai_echeck(host, service, hints.ai_flags, hints.ai_family,
      hints.ai_socktype, hints.ai_protocol);
  if (rc != 0)
  {
    freeaddrinfo_emulation(aihead);
    return rc;
  }

  gai_search search[2];
  int search_count = gai_nsearch(host, &hints, search);
  for (gai_search* sptr = search; sptr < search + search_count; ++sptr)
  {
    // Check for IPv4 dotted decimal string.
    in4_addr_type inaddr;
    boost::system::error_code ec;
    if (socket_ops::inet_pton(AF_INET, sptr->host, &inaddr, 0, ec) == 1)
    {
      if (hints.ai_family != AF_UNSPEC && hints.ai_family != AF_INET)
      {
        freeaddrinfo_emulation(aihead);
        gai_free(canon);
        return EAI_FAMILY;
      }
      if (sptr->family == AF_INET)
      {
        rc = gai_aistruct(&ainext, &hints, &inaddr, AF_INET);
        if (rc != 0)
        {
          freeaddrinfo_emulation(aihead);
          gai_free(canon);
          return rc;
        }
      }
      continue;
    }

    // Check for IPv6 hex string.
    in6_addr_type in6addr;
    if (socket_ops::inet_pton(AF_INET6, sptr->host, &in6addr, 0, ec) == 1)
    {
      if (hints.ai_family != AF_UNSPEC && hints.ai_family != AF_INET6)
      {
        freeaddrinfo_emulation(aihead);
        gai_free(canon);
        return EAI_FAMILY;
      }
      if (sptr->family == AF_INET6)
      {
        rc = gai_aistruct(&ainext, &hints, &in6addr, AF_INET6);
        if (rc != 0)
        {
          freeaddrinfo_emulation(aihead);
          gai_free(canon);
          return rc;
        }
      }
      continue;
    }

    // Look up hostname.
    hostent hent;
    char hbuf[8192] = "";
    hostent* hptr = socket_ops::gethostbyname(sptr->host,
        sptr->family, &hent, hbuf, sizeof(hbuf), hints.ai_flags, ec);
    if (hptr == 0)
    {
      if (search_count == 2)
      {
        // Failure is OK if there are multiple searches.
        continue;
      }
      freeaddrinfo_emulation(aihead);
      gai_free(canon);
      if (ec == boost::asio::error::host_not_found)
        return EAI_NONAME;
      if (ec == boost::asio::error::host_not_found_try_again)
        return EAI_AGAIN;
      if (ec == boost::asio::error::no_recovery)
        return EAI_FAIL;
      if (ec == boost::asio::error::no_data)
        return EAI_NONAME;
      return EAI_NONAME;
    }

    // Check for address family mismatch if one was specified.
    if (hints.ai_family != AF_UNSPEC && hints.ai_family != hptr->h_addrtype)
    {
      freeaddrinfo_emulation(aihead);
      gai_free(canon);
      socket_ops::freehostent(hptr);
      return EAI_FAMILY;
    }

    // Save canonical name first time.
    if (host != 0 && host[0] != '\0' && hptr->h_name && hptr->h_name[0]
        && (hints.ai_flags & AI_CANONNAME) && canon == 0)
    {
      std::size_t canon_len = strlen(hptr->h_name) + 1;
      canon = gai_alloc<char>(canon_len);
      if (canon == 0)
      {
        freeaddrinfo_emulation(aihead);
        socket_ops::freehostent(hptr);
        return EAI_MEMORY;
      }
      gai_strcpy(canon, hptr->h_name, canon_len);
    }

    // Create an addrinfo structure for each returned address.
    for (char** ap = hptr->h_addr_list; *ap; ++ap)
    {
      rc = gai_aistruct(&ainext, &hints, *ap, hptr->h_addrtype);
      if (rc != 0)
      {
        freeaddrinfo_emulation(aihead);
        gai_free(canon);
        socket_ops::freehostent(hptr);
        return EAI_FAMILY;
      }
    }

    socket_ops::freehostent(hptr);
  }

  // Check if we found anything.
  if (aihead == 0)
  {
    gai_free(canon);
    return EAI_NONAME;
  }

  // Return canonical name in first entry.
  if (host != 0 && host[0] != '\0' && (hints.ai_flags & AI_CANONNAME))
  {
    if (canon)
    {
      aihead->ai_canonname = canon;
      canon = 0;
    }
    else
    {
      std::size_t canonname_len = strlen(search[0].host) + 1;
      aihead->ai_canonname = gai_alloc<char>(canonname_len);
      if (aihead->ai_canonname == 0)
      {
        freeaddrinfo_emulation(aihead);
        return EAI_MEMORY;
      }
      gai_strcpy(aihead->ai_canonname, search[0].host, canonname_len);
    }
  }
  gai_free(canon);

  // Process the service name.
  if (service != 0 && service[0] != '\0')
  {
    rc = gai_serv(aihead, &hints, service);
    if (rc != 0)
    {
      freeaddrinfo_emulation(aihead);
      return rc;
    }
  }

  // Return result to caller.
  *result = aihead;
  return 0;
}

inline boost::system::error_code getnameinfo_emulation(
    const socket_addr_type* sa, std::size_t salen, char* host,
    std::size_t hostlen, char* serv, std::size_t servlen, int flags,
    boost::system::error_code& ec)
{
  using namespace std;

  const char* addr;
  size_t addr_len;
  unsigned short port;
  switch (sa->sa_family)
  {
  case AF_INET:
    if (salen != sizeof(sockaddr_in4_type))
    {
      return ec = boost::asio::error::invalid_argument;
    }
    addr = reinterpret_cast<const char*>(
        &reinterpret_cast<const sockaddr_in4_type*>(sa)->sin_addr);
    addr_len = sizeof(in4_addr_type);
    port = reinterpret_cast<const sockaddr_in4_type*>(sa)->sin_port;
    break;
  case AF_INET6:
    if (salen != sizeof(sockaddr_in6_type))
    {
      return ec = boost::asio::error::invalid_argument;
    }
    addr = reinterpret_cast<const char*>(
        &reinterpret_cast<const sockaddr_in6_type*>(sa)->sin6_addr);
    addr_len = sizeof(in6_addr_type);
    port = reinterpret_cast<const sockaddr_in6_type*>(sa)->sin6_port;
    break;
  default:
    return ec = boost::asio::error::address_family_not_supported;
  }

  if (host && hostlen > 0)
  {
    if (flags & NI_NUMERICHOST)
    {
      if (socket_ops::inet_ntop(sa->sa_family, addr, host, hostlen, 0, ec) == 0)
      {
        return ec;
      }
    }
    else
    {
      hostent hent;
      char hbuf[8192] = "";
      hostent* hptr = socket_ops::gethostbyaddr(addr,
          static_cast<int>(addr_len), sa->sa_family,
          &hent, hbuf, sizeof(hbuf), ec);
      if (hptr && hptr->h_name && hptr->h_name[0] != '\0')
      {
        if (flags & NI_NOFQDN)
        {
          char* dot = strchr(hptr->h_name, '.');
          if (dot)
          {
            *dot = 0;
          }
        }
        gai_strcpy(host, hptr->h_name, hostlen);
        socket_ops::freehostent(hptr);
      }
      else
      {
        socket_ops::freehostent(hptr);
        if (flags & NI_NAMEREQD)
        {
          return ec = boost::asio::error::host_not_found;
        }
        if (socket_ops::inet_ntop(sa->sa_family,
              addr, host, hostlen, 0, ec) == 0)
        {
          return ec;
        }
      }
    }
  }

  if (serv && servlen > 0)
  {
    if (flags & NI_NUMERICSERV)
    {
      if (servlen < 6)
      {
        return ec = boost::asio::error::no_buffer_space;
      }
#if BOOST_WORKAROUND(BOOST_MSVC, >= 1400) && !defined(UNDER_CE)
      sprintf_s(serv, servlen, "%u", ntohs(port));
#else
      sprintf(serv, "%u", ntohs(port));
#endif
    }
    else
    {
#if defined(BOOST_HAS_THREADS) && defined(BOOST_HAS_PTHREADS)
      static ::pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
      ::pthread_mutex_lock(&mutex);
#endif // defined(BOOST_HAS_THREADS) && defined(BOOST_HAS_PTHREADS)
      servent* sptr = ::getservbyport(port, (flags & NI_DGRAM) ? "udp" : 0);
      if (sptr && sptr->s_name && sptr->s_name[0] != '\0')
      {
        gai_strcpy(serv, sptr->s_name, servlen);
      }
      else
      {
        if (servlen < 6)
        {
          return ec = boost::asio::error::no_buffer_space;
        }
#if BOOST_WORKAROUND(BOOST_MSVC, >= 1400) && !defined(UNDER_CE)
        sprintf_s(serv, servlen, "%u", ntohs(port));
#else
        sprintf(serv, "%u", ntohs(port));
#endif
      }
#if defined(BOOST_HAS_THREADS) && defined(BOOST_HAS_PTHREADS)
      ::pthread_mutex_unlock(&mutex);
#endif // defined(BOOST_HAS_THREADS) && defined(BOOST_HAS_PTHREADS)
    }
  }

  clear_error(ec);
  return ec;
}

#endif // defined(BOOST_WINDOWS) || defined(__CYGWIN__)
       //   || defined(__MACH__) && defined(__APPLE__)

inline boost::system::error_code translate_addrinfo_error(int error)
{
  switch (error)
  {
  case 0:
    return boost::system::error_code();
  case EAI_AGAIN:
    return boost::asio::error::host_not_found_try_again;
  case EAI_BADFLAGS:
    return boost::asio::error::invalid_argument;
  case EAI_FAIL:
    return boost::asio::error::no_recovery;
  case EAI_FAMILY:
    return boost::asio::error::address_family_not_supported;
  case EAI_MEMORY:
    return boost::asio::error::no_memory;
  case EAI_NONAME:
#if defined(EAI_ADDRFAMILY)
  case EAI_ADDRFAMILY:
#endif
#if defined(EAI_NODATA) && (EAI_NODATA != EAI_NONAME)
  case EAI_NODATA:
#endif
    return boost::asio::error::host_not_found;
  case EAI_SERVICE:
    return boost::asio::error::service_not_found;
  case EAI_SOCKTYPE:
    return boost::asio::error::socket_type_not_supported;
  default: // Possibly the non-portable EAI_SYSTEM.
#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
    return boost::system::error_code(
        WSAGetLastError(), boost::asio::error::get_system_category());
#else
    return boost::system::error_code(
        errno, boost::asio::error::get_system_category());
#endif
  }
}

inline boost::system::error_code getaddrinfo(const char* host,
    const char* service, const addrinfo_type* hints, addrinfo_type** result,
    boost::system::error_code& ec)
{
  clear_error(ec);
#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
# if defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x0501) || defined(UNDER_CE)
  // Building for Windows XP, Windows Server 2003, or later.
  int error = ::getaddrinfo(host, service, hints, result);
  return ec = translate_addrinfo_error(error);
# else
  // Building for Windows 2000 or earlier.
  typedef int (WSAAPI *gai_t)(const char*,
      const char*, const addrinfo_type*, addrinfo_type**);
  if (HMODULE winsock_module = ::GetModuleHandleA("ws2_32"))
  {
    if (gai_t gai = (gai_t)::GetProcAddress(winsock_module, "getaddrinfo"))
    {
      int error = gai(host, service, hints, result);
      return ec = translate_addrinfo_error(error);
    }
  }
  int error = getaddrinfo_emulation(host, service, hints, result);
  return ec = translate_addrinfo_error(error);
# endif
#elif defined(__MACH__) && defined(__APPLE__)
  int error = getaddrinfo_emulation(host, service, hints, result);
  return ec = translate_addrinfo_error(error);
#else
  int error = ::getaddrinfo(host, service, hints, result);
  return ec = translate_addrinfo_error(error);
#endif
}

inline void freeaddrinfo(addrinfo_type* ai)
{
#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
# if defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x0501) || defined(UNDER_CE)
  // Building for Windows XP, Windows Server 2003, or later.
  ::freeaddrinfo(ai);
# else
  // Building for Windows 2000 or earlier.
  typedef int (WSAAPI *fai_t)(addrinfo_type*);
  if (HMODULE winsock_module = ::GetModuleHandleA("ws2_32"))
  {
    if (fai_t fai = (fai_t)::GetProcAddress(winsock_module, "freeaddrinfo"))
    {
      fai(ai);
      return;
    }
  }
  freeaddrinfo_emulation(ai);
# endif
#elif defined(__MACH__) && defined(__APPLE__)
  freeaddrinfo_emulation(ai);
#else
  ::freeaddrinfo(ai);
#endif
}

inline boost::system::error_code getnameinfo(const socket_addr_type* addr,
    std::size_t addrlen, char* host, std::size_t hostlen,
    char* serv, std::size_t servlen, int flags, boost::system::error_code& ec)
{
#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
# if defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x0501) || defined(UNDER_CE)
  // Building for Windows XP, Windows Server 2003, or later.
  clear_error(ec);
  int error = ::getnameinfo(addr, static_cast<socklen_t>(addrlen),
      host, static_cast<DWORD>(hostlen),
      serv, static_cast<DWORD>(servlen), flags);
  return ec = translate_addrinfo_error(error);
# else
  // Building for Windows 2000 or earlier.
  typedef int (WSAAPI *gni_t)(const socket_addr_type*,
      int, char*, DWORD, char*, DWORD, int);
  if (HMODULE winsock_module = ::GetModuleHandleA("ws2_32"))
  {
    if (gni_t gni = (gni_t)::GetProcAddress(winsock_module, "getnameinfo"))
    {
      clear_error(ec);
      int error = gni(addr, static_cast<int>(addrlen),
          host, static_cast<DWORD>(hostlen),
          serv, static_cast<DWORD>(servlen), flags);
      return ec = translate_addrinfo_error(error);
    }
  }
  clear_error(ec);
  return getnameinfo_emulation(addr, addrlen,
      host, hostlen, serv, servlen, flags, ec);
# endif
#elif defined(__MACH__) && defined(__APPLE__)
  using namespace std; // For memcpy.
  sockaddr_storage_type tmp_addr;
  memcpy(&tmp_addr, addr, addrlen);
  tmp_addr.ss_len = addrlen;
  addr = reinterpret_cast<socket_addr_type*>(&tmp_addr);
  clear_error(ec);
  return getnameinfo_emulation(addr, addrlen,
      host, hostlen, serv, servlen, flags, ec);
#else
  clear_error(ec);
  int error = ::getnameinfo(addr, addrlen, host, hostlen, serv, servlen, flags);
  return ec = translate_addrinfo_error(error);
#endif
}

inline u_long_type network_to_host_long(u_long_type value)
{
  return ntohl(value);
}

inline u_long_type host_to_network_long(u_long_type value)
{
  return htonl(value);
}

inline u_short_type network_to_host_short(u_short_type value)
{
  return ntohs(value);
}

inline u_short_type host_to_network_short(u_short_type value)
{
  return htons(value);
}

} // namespace socket_ops
} // namespace detail
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_SOCKET_OPS_HPP
