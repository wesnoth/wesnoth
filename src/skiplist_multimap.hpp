/*
   Copyright (C) 2009 - 2010 by Chris Hopman <cjhopman@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef SKIPLIST_MULTIMAP_HPP_INCLUDED
#define SKIPLIST_MULTIMAP_HPP_INCLUDED

#include "skiplist.hpp"

template <typename Key, typename Value, typename Compare = std::less<Key>, typename Alloc = std::allocator<Value> >
class skiplist_multimap : private skiplist<std::pair<const Key, Value>, Key, key_extractor<std::pair<const Key, Value> >, Compare, Alloc> {
private:
	typedef skiplist<std::pair<const Key, Value>, Key, key_extractor<std::pair<const Key, Value> >, Compare, Alloc> super;
public:
	typedef Key key_type;
	typedef std::pair<const Key, Value> value_type;
	typedef typename super::iterator iterator;
	typedef typename super::const_iterator const_iterator;
	typedef typename super::reverse_iterator reverse_iterator;
	typedef typename super::const_reverse_iterator const_reverse_iterator;
	typedef Compare key_compare;
	typedef map_value_compare<value_type, key_compare> value_compare;
	typedef typename super::allocator_type allocator_type;

	explicit skiplist_multimap(const Compare& oc = Compare(), const Alloc& oa = Alloc()) : super(oc, oa) { }
	skiplist_multimap(const skiplist_multimap& o) : super(o) { }
	template <typename InputIterator>
	skiplist_multimap(InputIterator first, InputIterator last, const Compare& oc = Compare(), const Alloc& oa = Alloc()) :
		super(first, last, oc, oa) { }

	iterator begin() { return super::begin(); }
	const_iterator begin() const { return super::begin(); }

	iterator end() { return super::end(); }
	const_iterator end() const { return super::end(); }

	reverse_iterator rbegin() { return super::rbegin(); }
	const_reverse_iterator rbegin() const { return super::rbegin(); }

	reverse_iterator rend() { return super::rend(); }
	const_reverse_iterator rend() const { return super::rend(); }

	bool empty() const { return super::empty(); }
	size_t size() const { return super::size(); }
	size_t max_size() const { return super::max_size(); }

	iterator insert(const value_type& v) { return super::insert(v); }
	iterator insert(const iterator& iter, const value_type& v) { return super::insert(iter, v); }
	template <typename InputIterator>
	void insert(InputIterator first, InputIterator last) { super::insert(first, last); }

	size_t erase(const Key& k) { return super::erase(k); }
	void erase(iterator first, iterator last) { super::erase(first, last); }
	void erase(iterator pos) { super::erase(pos); }

	iterator lower_bound(const Key& k) { return super::lower_bound(k); }
	const_iterator lower_bound(const Key& k) const { return super::lower_bound(k); }

	iterator upper_bound(const Key& k) { return super::upper_bound(k); }
	const_iterator upper_bound(const Key& k) const { return super::upper_bound(k); }

	std::pair<iterator, iterator> equal_range(const Key& k) { return super::equal_range(k); }
	std::pair<const_iterator, const_iterator> equal_range(const Key& k) const { return super::equal_range(k); }

	iterator find(const Key& k) { return super::find(k); }
	size_t count(const key_type& k) const {
		std::pair<const_iterator, const_iterator> range = equal_range(k);
		return std::distance(range.first, range.second);
	}

	key_compare key_comp() const { return super::key_comp(); }
	value_compare value_comp() const { return value_compare(super::key_comp()); }

	allocator_type get_allocator() const { return super::get_allocator(); }

	void swap(skiplist_multimap& o) {
		super::swap(o);
	}

	void clear() {
		super::clear();
	}
};

#endif
