//
// read_until.ipp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_READ_UNTIL_IPP
#define BOOST_ASIO_READ_UNTIL_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <algorithm>
#include <limits>
#include <string>
#include <utility>
#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/buffer.hpp>
#include <boost/asio/buffers_iterator.hpp>
#include <boost/asio/detail/bind_handler.hpp>
#include <boost/asio/detail/handler_alloc_helpers.hpp>
#include <boost/asio/detail/handler_invoke_helpers.hpp>
#include <boost/asio/detail/throw_error.hpp>

namespace boost {
namespace asio {

template <typename SyncReadStream, typename Allocator>
inline std::size_t read_until(SyncReadStream& s,
    boost::asio::basic_streambuf<Allocator>& b, char delim)
{
  boost::system::error_code ec;
  std::size_t bytes_transferred = read_until(s, b, delim, ec);
  boost::asio::detail::throw_error(ec);
  return bytes_transferred;
}

template <typename SyncReadStream, typename Allocator>
std::size_t read_until(SyncReadStream& s,
    boost::asio::basic_streambuf<Allocator>& b, char delim,
    boost::system::error_code& ec)
{
  std::size_t next_search_start = 0;
  for (;;)
  {
    // Determine the range of the data to be searched.
    typedef typename boost::asio::basic_streambuf<
      Allocator>::const_buffers_type const_buffers_type;
    typedef boost::asio::buffers_iterator<const_buffers_type> iterator;
    const_buffers_type buffers = b.data();
    iterator begin = iterator::begin(buffers);
    iterator start = begin + next_search_start;
    iterator end = iterator::end(buffers);

    // Look for a match.
    iterator iter = std::find(start, end, delim);
    if (iter != end)
    {
      // Found a match. We're done.
      ec = boost::system::error_code();
      return iter - begin + 1;
    }
    else
    {
      // No match. Next search can start with the new data.
      next_search_start = end - begin;
    }

    // Check if buffer is full.
    if (b.size() == b.max_size())
    {
      ec = error::not_found;
      return 0;
    }

    // Need more data.
    std::size_t bytes_available =
      std::min<std::size_t>(512, b.max_size() - b.size());
    b.commit(s.read_some(b.prepare(bytes_available), ec));
    if (ec)
      return 0;
  }
}

template <typename SyncReadStream, typename Allocator>
inline std::size_t read_until(SyncReadStream& s,
    boost::asio::basic_streambuf<Allocator>& b, const std::string& delim)
{
  boost::system::error_code ec;
  std::size_t bytes_transferred = read_until(s, b, delim, ec);
  boost::asio::detail::throw_error(ec);
  return bytes_transferred;
}

namespace detail
{
  // Algorithm that finds a subsequence of equal values in a sequence. Returns
  // (iterator,true) if a full match was found, in which case the iterator
  // points to the beginning of the match. Returns (iterator,false) if a
  // partial match was found at the end of the first sequence, in which case
  // the iterator points to the beginning of the partial match. Returns
  // (last1,false) if no full or partial match was found.
  template <typename Iterator1, typename Iterator2>
  std::pair<Iterator1, bool> partial_search(
      Iterator1 first1, Iterator1 last1, Iterator2 first2, Iterator2 last2)
  {
    for (Iterator1 iter1 = first1; iter1 != last1; ++iter1)
    {
      Iterator1 test_iter1 = iter1;
      Iterator2 test_iter2 = first2;
      for (;; ++test_iter1, ++test_iter2)
      {
        if (test_iter2 == last2)
          return std::make_pair(iter1, true);
        if (test_iter1 == last1)
        {
          if (test_iter2 != first2)
            return std::make_pair(iter1, false);
          else
            break;
        }
        if (*test_iter1 != *test_iter2)
          break;
      }
    }
    return std::make_pair(last1, false);
  }
} // namespace detail

template <typename SyncReadStream, typename Allocator>
std::size_t read_until(SyncReadStream& s,
    boost::asio::basic_streambuf<Allocator>& b, const std::string& delim,
    boost::system::error_code& ec)
{
  std::size_t next_search_start = 0;
  for (;;)
  {
    // Determine the range of the data to be searched.
    typedef typename boost::asio::basic_streambuf<
      Allocator>::const_buffers_type const_buffers_type;
    typedef boost::asio::buffers_iterator<const_buffers_type> iterator;
    const_buffers_type buffers = b.data();
    iterator begin = iterator::begin(buffers);
    iterator start = begin + next_search_start;
    iterator end = iterator::end(buffers);

    // Look for a match.
    std::pair<iterator, bool> result = boost::asio::detail::partial_search(
        start, end, delim.begin(), delim.end());
    if (result.first != end)
    {
      if (result.second)
      {
        // Full match. We're done.
        ec = boost::system::error_code();
        return result.first - begin + delim.length();
      }
      else
      {
        // Partial match. Next search needs to start from beginning of match.
        next_search_start = result.first - begin;
      }
    }
    else
    {
      // No match. Next search can start with the new data.
      next_search_start = end - begin;
    }

    // Check if buffer is full.
    if (b.size() == b.max_size())
    {
      ec = error::not_found;
      return 0;
    }

    // Need more data.
    std::size_t bytes_available =
      std::min<std::size_t>(512, b.max_size() - b.size());
    b.commit(s.read_some(b.prepare(bytes_available), ec));
    if (ec)
      return 0;
  }
}

template <typename SyncReadStream, typename Allocator>
inline std::size_t read_until(SyncReadStream& s,
    boost::asio::basic_streambuf<Allocator>& b, const boost::regex& expr)
{
  boost::system::error_code ec;
  std::size_t bytes_transferred = read_until(s, b, expr, ec);
  boost::asio::detail::throw_error(ec);
  return bytes_transferred;
}

template <typename SyncReadStream, typename Allocator>
std::size_t read_until(SyncReadStream& s,
    boost::asio::basic_streambuf<Allocator>& b, const boost::regex& expr,
    boost::system::error_code& ec)
{
  std::size_t next_search_start = 0;
  for (;;)
  {
    // Determine the range of the data to be searched.
    typedef typename boost::asio::basic_streambuf<
      Allocator>::const_buffers_type const_buffers_type;
    typedef boost::asio::buffers_iterator<const_buffers_type> iterator;
    const_buffers_type buffers = b.data();
    iterator begin = iterator::begin(buffers);
    iterator start = begin + next_search_start;
    iterator end = iterator::end(buffers);

    // Look for a match.
    boost::match_results<iterator> match_results;
    if (boost::regex_search(start, end, match_results, expr,
          boost::match_default | boost::match_partial))
    {
      if (match_results[0].matched)
      {
        // Full match. We're done.
        ec = boost::system::error_code();
        return match_results[0].second - begin;
      }
      else
      {
        // Partial match. Next search needs to start from beginning of match.
        next_search_start = match_results[0].first - begin;
      }
    }
    else
    {
      // No match. Next search can start with the new data.
      next_search_start = end - begin;
    }

    // Check if buffer is full.
    if (b.size() == b.max_size())
    {
      ec = error::not_found;
      return 0;
    }

    // Need more data.
    std::size_t bytes_available =
      std::min<std::size_t>(512, b.max_size() - b.size());
    b.commit(s.read_some(b.prepare(bytes_available), ec));
    if (ec)
      return 0;
  }
}

template <typename SyncReadStream, typename Allocator, typename MatchCondition>
std::size_t read_until(SyncReadStream& s,
    boost::asio::basic_streambuf<Allocator>& b,
    MatchCondition match_condition, boost::system::error_code& ec,
    typename boost::enable_if<is_match_condition<MatchCondition> >::type*)
{
  std::size_t next_search_start = 0;
  for (;;)
  {
    // Determine the range of the data to be searched.
    typedef typename boost::asio::basic_streambuf<
      Allocator>::const_buffers_type const_buffers_type;
    typedef boost::asio::buffers_iterator<const_buffers_type> iterator;
    const_buffers_type buffers = b.data();
    iterator begin = iterator::begin(buffers);
    iterator start = begin + next_search_start;
    iterator end = iterator::end(buffers);

    // Look for a match.
    std::pair<iterator, bool> result = match_condition(start, end);
    if (result.second)
    {
      // Full match. We're done.
      ec = boost::system::error_code();
      return result.first - begin;
    }
    else if (result.first != end)
    {
      // Partial match. Next search needs to start from beginning of match.
      next_search_start = result.first - begin;
    }
    else
    {
      // No match. Next search can start with the new data.
      next_search_start = end - begin;
    }

    // Check if buffer is full.
    if (b.size() == b.max_size())
    {
      ec = error::not_found;
      return 0;
    }

    // Need more data.
    std::size_t bytes_available =
      std::min<std::size_t>(512, b.max_size() - b.size());
    b.commit(s.read_some(b.prepare(bytes_available), ec));
    if (ec)
      return 0;
  }
}

template <typename SyncReadStream, typename Allocator, typename MatchCondition>
inline std::size_t read_until(SyncReadStream& s,
    boost::asio::basic_streambuf<Allocator>& b, MatchCondition match_condition,
    typename boost::enable_if<is_match_condition<MatchCondition> >::type*)
{
  boost::system::error_code ec;
  std::size_t bytes_transferred = read_until(s, b, match_condition, ec);
  boost::asio::detail::throw_error(ec);
  return bytes_transferred;
}

namespace detail
{
  template <typename AsyncReadStream, typename Allocator, typename ReadHandler>
  class read_until_delim_handler
  {
  public:
    read_until_delim_handler(AsyncReadStream& stream,
        boost::asio::basic_streambuf<Allocator>& streambuf, char delim,
        std::size_t next_search_start, ReadHandler handler)
      : stream_(stream),
        streambuf_(streambuf),
        delim_(delim),
        next_search_start_(next_search_start),
        handler_(handler)
    {
    }

    void operator()(const boost::system::error_code& ec,
        std::size_t bytes_transferred)
    {
      // Check for errors.
      if (ec)
      {
        std::size_t bytes = 0;
        handler_(ec, bytes);
        return;
      }

      // Commit received data to streambuf's get area.
      streambuf_.commit(bytes_transferred);

      // Determine the range of the data to be searched.
      typedef typename boost::asio::basic_streambuf<
        Allocator>::const_buffers_type const_buffers_type;
      typedef boost::asio::buffers_iterator<const_buffers_type> iterator;
      const_buffers_type buffers = streambuf_.data();
      iterator begin = iterator::begin(buffers);
      iterator start = begin + next_search_start_;
      iterator end = iterator::end(buffers);

      // Look for a match.
      iterator iter = std::find(start, end, delim_);
      if (iter != end)
      {
        // Found a match. We're done.
        std::size_t bytes = iter - begin + 1;
        handler_(ec, bytes);
        return;
      }

      // No match. Check if buffer is full.
      if (streambuf_.size() == streambuf_.max_size())
      {
        std::size_t bytes = 0;
        boost::system::error_code ec(error::not_found);
        handler_(ec, bytes);
        return;
      }

      // Next search can start with the new data.
      next_search_start_ = end - begin;

      // Start a new asynchronous read operation to obtain more data.
      std::size_t bytes_available =
        std::min<std::size_t>(512, streambuf_.max_size() - streambuf_.size());
      stream_.async_read_some(streambuf_.prepare(bytes_available), *this);
    }

  //private:
    AsyncReadStream& stream_;
    boost::asio::basic_streambuf<Allocator>& streambuf_;
    char delim_;
    std::size_t next_search_start_;
    ReadHandler handler_;
  };

  template <typename AsyncReadStream, typename Allocator, typename ReadHandler>
  inline void* asio_handler_allocate(std::size_t size,
      read_until_delim_handler<AsyncReadStream,
        Allocator, ReadHandler>* this_handler)
  {
    return boost_asio_handler_alloc_helpers::allocate(
        size, &this_handler->handler_);
  }

  template <typename AsyncReadStream, typename Allocator, typename ReadHandler>
  inline void asio_handler_deallocate(void* pointer, std::size_t size,
      read_until_delim_handler<AsyncReadStream,
        Allocator, ReadHandler>* this_handler)
  {
    boost_asio_handler_alloc_helpers::deallocate(
        pointer, size, &this_handler->handler_);
  }

  template <typename Function, typename AsyncReadStream, typename Allocator,
      typename ReadHandler>
  inline void asio_handler_invoke(const Function& function,
      read_until_delim_handler<AsyncReadStream,
        Allocator, ReadHandler>* this_handler)
  {
    boost_asio_handler_invoke_helpers::invoke(
        function, &this_handler->handler_);
  }
} // namespace detail

template <typename AsyncReadStream, typename Allocator, typename ReadHandler>
void async_read_until(AsyncReadStream& s,
    boost::asio::basic_streambuf<Allocator>& b, char delim, ReadHandler handler)
{
  // Determine the range of the data to be searched.
  typedef typename boost::asio::basic_streambuf<
    Allocator>::const_buffers_type const_buffers_type;
  typedef boost::asio::buffers_iterator<const_buffers_type> iterator;
  const_buffers_type buffers = b.data();
  iterator begin = iterator::begin(buffers);
  iterator end = iterator::end(buffers);

  // Look for a match.
  iterator iter = std::find(begin, end, delim);
  if (iter != end)
  {
    // Found a match. We're done.
    boost::system::error_code ec;
    std::size_t bytes = iter - begin + 1;
    s.get_io_service().post(detail::bind_handler(handler, ec, bytes));
    return;
  }

  // No match. Check if buffer is full.
  if (b.size() == b.max_size())
  {
    boost::system::error_code ec(error::not_found);
    s.get_io_service().post(detail::bind_handler(handler, ec, 0));
    return;
  }

  // Start a new asynchronous read operation to obtain more data.
  std::size_t bytes_available =
    std::min<std::size_t>(512, b.max_size() - b.size());
  s.async_read_some(b.prepare(bytes_available),
      detail::read_until_delim_handler<AsyncReadStream, Allocator, ReadHandler>(
        s, b, delim, end - begin, handler));
}

namespace detail
{
  template <typename AsyncReadStream, typename Allocator, typename ReadHandler>
  class read_until_delim_string_handler
  {
  public:
    read_until_delim_string_handler(AsyncReadStream& stream,
        boost::asio::basic_streambuf<Allocator>& streambuf,
        const std::string& delim, std::size_t next_search_start,
        ReadHandler handler)
      : stream_(stream),
        streambuf_(streambuf),
        delim_(delim),
        next_search_start_(next_search_start),
        handler_(handler)
    {
    }

    void operator()(const boost::system::error_code& ec,
        std::size_t bytes_transferred)
    {
      // Check for errors.
      if (ec)
      {
        std::size_t bytes = 0;
        handler_(ec, bytes);
        return;
      }

      // Commit received data to streambuf's get area.
      streambuf_.commit(bytes_transferred);

      // Determine the range of the data to be searched.
      typedef typename boost::asio::basic_streambuf<
        Allocator>::const_buffers_type const_buffers_type;
      typedef boost::asio::buffers_iterator<const_buffers_type> iterator;
      const_buffers_type buffers = streambuf_.data();
      iterator begin = iterator::begin(buffers);
      iterator start = begin + next_search_start_;
      iterator end = iterator::end(buffers);

      // Look for a match.
      std::pair<iterator, bool> result = boost::asio::detail::partial_search(
          start, end, delim_.begin(), delim_.end());
      if (result.first != end)
      {
        if (result.second)
        {
          // Full match. We're done.
          std::size_t bytes = result.first - begin + delim_.length();
          handler_(ec, bytes);
          return;
        }
        else
        {
          // Partial match. Next search needs to start from beginning of match.
          next_search_start_ = result.first - begin;
        }
      }
      else
      {
        // No match. Next search can start with the new data.
        next_search_start_ = end - begin;
      }

      // Check if buffer is full.
      if (streambuf_.size() == streambuf_.max_size())
      {
        std::size_t bytes = 0;
        boost::system::error_code ec2(error::not_found);
        handler_(ec2, bytes);
        return;
      }

      // Start a new asynchronous read operation to obtain more data.
      std::size_t bytes_available =
        std::min<std::size_t>(512, streambuf_.max_size() - streambuf_.size());
      stream_.async_read_some(streambuf_.prepare(bytes_available), *this);
    }

  //private:
    AsyncReadStream& stream_;
    boost::asio::basic_streambuf<Allocator>& streambuf_;
    std::string delim_;
    std::size_t next_search_start_;
    ReadHandler handler_;
  };

  template <typename AsyncReadStream, typename Allocator, typename ReadHandler>
  inline void* asio_handler_allocate(std::size_t size,
      read_until_delim_string_handler<AsyncReadStream,
        Allocator, ReadHandler>* this_handler)
  {
    return boost_asio_handler_alloc_helpers::allocate(
        size, &this_handler->handler_);
  }

  template <typename AsyncReadStream, typename Allocator, typename ReadHandler>
  inline void asio_handler_deallocate(void* pointer, std::size_t size,
      read_until_delim_string_handler<AsyncReadStream,
        Allocator, ReadHandler>* this_handler)
  {
    boost_asio_handler_alloc_helpers::deallocate(
        pointer, size, &this_handler->handler_);
  }

  template <typename Function, typename AsyncReadStream,
      typename Allocator, typename ReadHandler>
  inline void asio_handler_invoke(const Function& function,
      read_until_delim_string_handler<AsyncReadStream,
        Allocator, ReadHandler>* this_handler)
  {
    boost_asio_handler_invoke_helpers::invoke(
        function, &this_handler->handler_);
  }
} // namespace detail

template <typename AsyncReadStream, typename Allocator, typename ReadHandler>
void async_read_until(AsyncReadStream& s,
    boost::asio::basic_streambuf<Allocator>& b, const std::string& delim,
    ReadHandler handler)
{
  // Determine the range of the data to be searched.
  typedef typename boost::asio::basic_streambuf<
    Allocator>::const_buffers_type const_buffers_type;
  typedef boost::asio::buffers_iterator<const_buffers_type> iterator;
  const_buffers_type buffers = b.data();
  iterator begin = iterator::begin(buffers);
  iterator end = iterator::end(buffers);

  // Look for a match.
  std::size_t next_search_start;
  std::pair<iterator, bool> result = boost::asio::detail::partial_search(
      begin, end, delim.begin(), delim.end());
  if (result.first != end)
  {
    if (result.second)
    {
      // Full match. We're done.
      boost::system::error_code ec;
      std::size_t bytes = result.first - begin + delim.length();
      s.get_io_service().post(detail::bind_handler(handler, ec, bytes));
      return;
    }
    else
    {
      // Partial match. Next search needs to start from beginning of match.
      next_search_start = result.first - begin;
    }
  }
  else
  {
    // No match. Next search can start with the new data.
    next_search_start = end - begin;
  }

  // Check if buffer is full.
  if (b.size() == b.max_size())
  {
    boost::system::error_code ec(error::not_found);
    s.get_io_service().post(detail::bind_handler(handler, ec, 0));
    return;
  }

  // Start a new asynchronous read operation to obtain more data.
  std::size_t bytes_available =
    std::min<std::size_t>(512, b.max_size() - b.size());
  s.async_read_some(b.prepare(bytes_available),
      detail::read_until_delim_string_handler<
        AsyncReadStream, Allocator, ReadHandler>(
          s, b, delim, next_search_start, handler));
}

namespace detail
{
  template <typename AsyncReadStream, typename Allocator, typename ReadHandler>
  class read_until_expr_handler
  {
  public:
    read_until_expr_handler(AsyncReadStream& stream,
        boost::asio::basic_streambuf<Allocator>& streambuf,
        const boost::regex& expr, std::size_t next_search_start,
        ReadHandler handler)
      : stream_(stream),
        streambuf_(streambuf),
        expr_(expr),
        next_search_start_(next_search_start),
        handler_(handler)
    {
    }

    void operator()(const boost::system::error_code& ec,
        std::size_t bytes_transferred)
    {
      // Check for errors.
      if (ec)
      {
        std::size_t bytes = 0;
        handler_(ec, bytes);
        return;
      }

      // Commit received data to streambuf's get area.
      streambuf_.commit(bytes_transferred);

      // Determine the range of the data to be searched.
      typedef typename boost::asio::basic_streambuf<
        Allocator>::const_buffers_type const_buffers_type;
      typedef boost::asio::buffers_iterator<const_buffers_type> iterator;
      const_buffers_type buffers = streambuf_.data();
      iterator begin = iterator::begin(buffers);
      iterator start = begin + next_search_start_;
      iterator end = iterator::end(buffers);

      // Look for a match.
      boost::match_results<iterator> match_results;
      if (boost::regex_search(start, end, match_results, expr_,
            boost::match_default | boost::match_partial))
      {
        if (match_results[0].matched)
        {
          // Full match. We're done.
          std::size_t bytes = match_results[0].second - begin;
          handler_(ec, bytes);
          return;
        }
        else
        {
          // Partial match. Next search needs to start from beginning of match.
          next_search_start_ = match_results[0].first - begin;
        }
      }
      else
      {
        // No match. Next search can start with the new data.
        next_search_start_ = end - begin;
      }

      // Check if buffer is full.
      if (streambuf_.size() == streambuf_.max_size())
      {
        std::size_t bytes = 0;
        boost::system::error_code ec(error::not_found);
        handler_(ec, bytes);
        return;
      }

      // Start a new asynchronous read operation to obtain more data.
      std::size_t bytes_available =
        std::min<std::size_t>(512, streambuf_.max_size() - streambuf_.size());
      stream_.async_read_some(streambuf_.prepare(bytes_available), *this);
    }

  //private:
    AsyncReadStream& stream_;
    boost::asio::basic_streambuf<Allocator>& streambuf_;
    boost::regex expr_;
    std::size_t next_search_start_;
    ReadHandler handler_;
  };

  template <typename AsyncReadStream, typename Allocator, typename ReadHandler>
  inline void* asio_handler_allocate(std::size_t size,
      read_until_expr_handler<AsyncReadStream,
        Allocator, ReadHandler>* this_handler)
  {
    return boost_asio_handler_alloc_helpers::allocate(
        size, &this_handler->handler_);
  }

  template <typename AsyncReadStream, typename Allocator, typename ReadHandler>
  inline void asio_handler_deallocate(void* pointer, std::size_t size,
      read_until_expr_handler<AsyncReadStream,
        Allocator, ReadHandler>* this_handler)
  {
    boost_asio_handler_alloc_helpers::deallocate(
        pointer, size, &this_handler->handler_);
  }

  template <typename Function, typename AsyncReadStream, typename Allocator,
      typename ReadHandler>
  inline void asio_handler_invoke(const Function& function,
      read_until_expr_handler<AsyncReadStream,
        Allocator, ReadHandler>* this_handler)
  {
    boost_asio_handler_invoke_helpers::invoke(
        function, &this_handler->handler_);
  }
} // namespace detail

template <typename AsyncReadStream, typename Allocator, typename ReadHandler>
void async_read_until(AsyncReadStream& s,
    boost::asio::basic_streambuf<Allocator>& b, const boost::regex& expr,
    ReadHandler handler)
{
  // Determine the range of the data to be searched.
  typedef typename boost::asio::basic_streambuf<
    Allocator>::const_buffers_type const_buffers_type;
  typedef boost::asio::buffers_iterator<const_buffers_type> iterator;
  const_buffers_type buffers = b.data();
  iterator begin = iterator::begin(buffers);
  iterator end = iterator::end(buffers);

  // Look for a match.
  std::size_t next_search_start;
  boost::match_results<iterator> match_results;
  if (boost::regex_search(begin, end, match_results, expr,
        boost::match_default | boost::match_partial))
  {
    if (match_results[0].matched)
    {
      // Full match. We're done.
      boost::system::error_code ec;
      std::size_t bytes = match_results[0].second - begin;
      s.get_io_service().post(detail::bind_handler(handler, ec, bytes));
      return;
    }
    else
    {
      // Partial match. Next search needs to start from beginning of match.
      next_search_start = match_results[0].first - begin;
    }
  }
  else
  {
    // No match. Next search can start with the new data.
    next_search_start = end - begin;
  }

  // Check if buffer is full.
  if (b.size() == b.max_size())
  {
    boost::system::error_code ec(error::not_found);
    s.get_io_service().post(detail::bind_handler(handler, ec, 0));
    return;
  }

  // Start a new asynchronous read operation to obtain more data.
  std::size_t bytes_available =
    std::min<std::size_t>(512, b.max_size() - b.size());
  s.async_read_some(b.prepare(bytes_available),
      detail::read_until_expr_handler<AsyncReadStream, Allocator, ReadHandler>(
        s, b, expr, next_search_start, handler));
}

namespace detail
{
  template <typename AsyncReadStream, typename Allocator,
      typename MatchCondition, typename ReadHandler>
  class read_until_match_handler
  {
  public:
    read_until_match_handler(AsyncReadStream& stream,
        boost::asio::basic_streambuf<Allocator>& streambuf,
        MatchCondition match_condition, std::size_t next_search_start,
        ReadHandler handler)
      : stream_(stream),
        streambuf_(streambuf),
        match_condition_(match_condition),
        next_search_start_(next_search_start),
        handler_(handler)
    {
    }

    void operator()(const boost::system::error_code& ec,
        std::size_t bytes_transferred)
    {
      // Check for errors.
      if (ec)
      {
        std::size_t bytes = 0;
        handler_(ec, bytes);
        return;
      }

      // Commit received data to streambuf's get area.
      streambuf_.commit(bytes_transferred);

      // Determine the range of the data to be searched.
      typedef typename boost::asio::basic_streambuf<
        Allocator>::const_buffers_type const_buffers_type;
      typedef boost::asio::buffers_iterator<const_buffers_type> iterator;
      const_buffers_type buffers = streambuf_.data();
      iterator begin = iterator::begin(buffers);
      iterator start = begin + next_search_start_;
      iterator end = iterator::end(buffers);

      // Look for a match.
      std::pair<iterator, bool> result = match_condition_(start, end);
      if (result.second)
      {
        // Full match. We're done.
        std::size_t bytes = result.first - begin;
        handler_(ec, bytes);
        return;
      }
      else if (result.first != end)
      {
        // Partial match. Next search needs to start from beginning of match.
        next_search_start_ = result.first - begin;
      }
      else
      {
        // No match. Next search can start with the new data.
        next_search_start_ = end - begin;
      }

      // Check if buffer is full.
      if (streambuf_.size() == streambuf_.max_size())
      {
        std::size_t bytes = 0;
        boost::system::error_code ec(error::not_found);
        handler_(ec, bytes);
        return;
      }

      // Start a new asynchronous read operation to obtain more data.
      std::size_t bytes_available =
        std::min<std::size_t>(512, streambuf_.max_size() - streambuf_.size());
      stream_.async_read_some(streambuf_.prepare(bytes_available), *this);
    }

  //private:
    AsyncReadStream& stream_;
    boost::asio::basic_streambuf<Allocator>& streambuf_;
    MatchCondition match_condition_;
    std::size_t next_search_start_;
    ReadHandler handler_;
  };

  template <typename AsyncReadStream, typename Allocator,
      typename MatchCondition, typename ReadHandler>
  inline void* asio_handler_allocate(std::size_t size,
      read_until_match_handler<AsyncReadStream,
        Allocator, MatchCondition, ReadHandler>* this_handler)
  {
    return boost_asio_handler_alloc_helpers::allocate(
        size, &this_handler->handler_);
  }

  template <typename AsyncReadStream, typename Allocator,
      typename MatchCondition, typename ReadHandler>
  inline void asio_handler_deallocate(void* pointer, std::size_t size,
      read_until_match_handler<AsyncReadStream,
        Allocator, MatchCondition, ReadHandler>* this_handler)
  {
    boost_asio_handler_alloc_helpers::deallocate(
        pointer, size, &this_handler->handler_);
  }

  template <typename Function, typename AsyncReadStream, typename Allocator,
      typename MatchCondition, typename ReadHandler>
  inline void asio_handler_invoke(const Function& function,
      read_until_match_handler<AsyncReadStream,
        Allocator, MatchCondition, ReadHandler>* this_handler)
  {
    boost_asio_handler_invoke_helpers::invoke(
        function, &this_handler->handler_);
  }
} // namespace detail

template <typename AsyncReadStream, typename Allocator,
    typename MatchCondition, typename ReadHandler>
void async_read_until(AsyncReadStream& s,
    boost::asio::basic_streambuf<Allocator>& b,
    MatchCondition match_condition, ReadHandler handler,
    typename boost::enable_if<is_match_condition<MatchCondition> >::type*)
{
  // Determine the range of the data to be searched.
  typedef typename boost::asio::basic_streambuf<
    Allocator>::const_buffers_type const_buffers_type;
  typedef boost::asio::buffers_iterator<const_buffers_type> iterator;
  const_buffers_type buffers = b.data();
  iterator begin = iterator::begin(buffers);
  iterator end = iterator::end(buffers);

  // Look for a match.
  std::size_t next_search_start;
  std::pair<iterator, bool> result = match_condition(begin, end);
  if (result.second)
  {
    // Full match. We're done.
    boost::system::error_code ec;
    std::size_t bytes = result.first - begin;
    s.get_io_service().post(detail::bind_handler(handler, ec, bytes));
    return;
  }
  else if (result.first != end)
  {
    // Partial match. Next search needs to start from beginning of match.
    next_search_start = result.first - begin;
  }
  else
  {
    // No match. Next search can start with the new data.
    next_search_start = end - begin;
  }

  // Check if buffer is full.
  if (b.size() == b.max_size())
  {
    boost::system::error_code ec(error::not_found);
    s.get_io_service().post(detail::bind_handler(handler, ec, 0));
    return;
  }

  // Start a new asynchronous read operation to obtain more data.
  std::size_t bytes_available =
    std::min<std::size_t>(512, b.max_size() - b.size());
  s.async_read_some(b.prepare(bytes_available),
      detail::read_until_match_handler<
        AsyncReadStream, Allocator, MatchCondition, ReadHandler>(
          s, b, match_condition, next_search_start, handler));
}

} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_READ_UNTIL_IPP
