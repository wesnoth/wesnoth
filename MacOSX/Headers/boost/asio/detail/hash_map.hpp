//
// hash_map.hpp
// ~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_HASH_MAP_HPP
#define BOOST_ASIO_DETAIL_HASH_MAP_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <cassert>
#include <list>
#include <utility>
#include <boost/functional/hash.hpp>
#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/detail/noncopyable.hpp>
#include <boost/asio/detail/socket_types.hpp>

namespace boost {
namespace asio {
namespace detail {

template <typename T>
inline std::size_t calculate_hash_value(const T& t)
{
  return boost::hash_value(t);
}

#if defined(_WIN64)
inline std::size_t calculate_hash_value(SOCKET s)
{
  return static_cast<std::size_t>(s);
}
#endif // defined(_WIN64)

// Note: assumes K and V are POD types.
template <typename K, typename V>
class hash_map
  : private noncopyable
{
public:
  // The type of a value in the map.
  typedef std::pair<K, V> value_type;

  // The type of a non-const iterator over the hash map.
  typedef typename std::list<value_type>::iterator iterator;

  // The type of a const iterator over the hash map.
  typedef typename std::list<value_type>::const_iterator const_iterator;

  // Constructor.
  hash_map()
  {
    // Initialise all buckets to empty.
    for (size_t i = 0; i < num_buckets; ++i)
      buckets_[i].first = buckets_[i].last = values_.end();
  }

  // Get an iterator for the beginning of the map.
  iterator begin()
  {
    return values_.begin();
  }

  // Get an iterator for the beginning of the map.
  const_iterator begin() const
  {
    return values_.begin();
  }

  // Get an iterator for the end of the map.
  iterator end()
  {
    return values_.end();
  }

  // Get an iterator for the end of the map.
  const_iterator end() const
  {
    return values_.end();
  }

  // Check whether the map is empty.
  bool empty() const
  {
    return values_.empty();
  }

  // Find an entry in the map.
  iterator find(const K& k)
  {
    size_t bucket = calculate_hash_value(k) % num_buckets;
    iterator it = buckets_[bucket].first;
    if (it == values_.end())
      return values_.end();
    iterator end = buckets_[bucket].last;
    ++end;
    while (it != end)
    {
      if (it->first == k)
        return it;
      ++it;
    }
    return values_.end();
  }

  // Find an entry in the map.
  const_iterator find(const K& k) const
  {
    size_t bucket = calculate_hash_value(k) % num_buckets;
    const_iterator it = buckets_[bucket].first;
    if (it == values_.end())
      return it;
    const_iterator end = buckets_[bucket].last;
    ++end;
    while (it != end)
    {
      if (it->first == k)
        return it;
      ++it;
    }
    return values_.end();
  }

  // Insert a new entry into the map.
  std::pair<iterator, bool> insert(const value_type& v)
  {
    size_t bucket = calculate_hash_value(v.first) % num_buckets;
    iterator it = buckets_[bucket].first;
    if (it == values_.end())
    {
      buckets_[bucket].first = buckets_[bucket].last =
        values_insert(values_.end(), v);
      return std::pair<iterator, bool>(buckets_[bucket].last, true);
    }
    iterator end = buckets_[bucket].last;
    ++end;
    while (it != end)
    {
      if (it->first == v.first)
        return std::pair<iterator, bool>(it, false);
      ++it;
    }
    buckets_[bucket].last = values_insert(end, v);
    return std::pair<iterator, bool>(buckets_[bucket].last, true);
  }

  // Erase an entry from the map.
  void erase(iterator it)
  {
    assert(it != values_.end());

    size_t bucket = calculate_hash_value(it->first) % num_buckets;
    bool is_first = (it == buckets_[bucket].first);
    bool is_last = (it == buckets_[bucket].last);
    if (is_first && is_last)
      buckets_[bucket].first = buckets_[bucket].last = values_.end();
    else if (is_first)
      ++buckets_[bucket].first;
    else if (is_last)
      --buckets_[bucket].last;

    values_erase(it);
  }

  // Remove all entries from the map.
  void clear()
  {
    // Clear the values.
    values_.clear();

    // Initialise all buckets to empty.
    for (size_t i = 0; i < num_buckets; ++i)
      buckets_[i].first = buckets_[i].last = values_.end();
  }

private:
  // Insert an element into the values list by splicing from the spares list,
  // if a spare is available, and otherwise by inserting a new element.
  iterator values_insert(iterator it, const value_type& v)
  {
    if (spares_.empty())
    {
      return values_.insert(it, v);
    }
    else
    {
      spares_.front() = v;
      values_.splice(it, spares_, spares_.begin());
      return --it;
    }
  }

  // Erase an element from the values list by splicing it to the spares list.
  void values_erase(iterator it)
  {
    *it = value_type();
    spares_.splice(spares_.begin(), values_, it);
  }

  // The list of all values in the hash map.
  std::list<value_type> values_;

  // The list of spare nodes waiting to be recycled. Assumes that POD types only
  // are stored in the hash map.
  std::list<value_type> spares_;

  // The type for a bucket in the hash table.
  struct bucket_type
  {
    iterator first;
    iterator last;
  };

  // The number of buckets in the hash.
  enum { num_buckets = 1021 };

  // The buckets in the hash.
  bucket_type buckets_[num_buckets];
};

} // namespace detail
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_HASH_MAP_HPP
