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

#ifndef SKIPLIST_MAP_HPP_INCLUDED
#define SKIPLIST_MAP_HPP_INCLUDED

#include "skiplist.hpp"

template <typename Key, typename Value, typename Compare = std::less<Key>, typename Alloc = std::allocator<Value> >
class skiplist_map : private skiplist<std::pair<const Key, Value>, Key, key_extractor<std::pair<const Key, Value> >, Compare, Alloc> {
private:
	typedef skiplist<std::pair<const Key, Value>, Key, key_extractor<std::pair<const Key, Value> >, Compare, Alloc> super;
public:
	typedef Key key_type;
	typedef typename super::value_type value_type;
	typedef typename super::reference reference;
	typedef typename super::const_reference const_reference;
	typedef typename super::iterator iterator;
	typedef typename super::const_iterator const_iterator;
	typedef typename super::reverse_iterator reverse_iterator;
	typedef typename super::const_reverse_iterator const_reverse_iterator;
	typedef Compare key_compare;
	typedef map_value_compare<value_type, key_compare> value_compare;
	typedef typename super::allocator_type allocator_type;

	explicit skiplist_map(const Compare& oc = Compare(), const Alloc& oa = Alloc()) : super(oc, oa) { }
	skiplist_map(const skiplist_map& o) : super(o) { }
	template <typename InputIterator>
	skiplist_map(InputIterator first, InputIterator last, const Compare& oc = Compare(), const Alloc& oa = Alloc()) : super(oc, oa) {
		super::insert_unique(first, last);
	}

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

	std::pair<iterator, bool> insert(const value_type& v) { return super::insert_unique(v); }
	std::pair<iterator, bool> insert(const iterator& iter, const value_type& v) { return super::insert_unique(iter, v); }
	template <typename InputIterator>
	void insert(InputIterator first, InputIterator last) { super::insert_unique(first, last); }

	Value& operator[](const Key& k) {
		return insert_unique(std::make_pair(k, Value())).first->second;
	}

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
	const_iterator find(const Key& k) const { return super::find(k); }

	size_t count(const key_type& k) const {
		std::pair<const_iterator, const_iterator> range = equal_range(k);
		return std::distance(range.first, range.second);
	}

	key_compare key_comp() const { return super::key_comp(); }
	value_compare value_comp() const { return value_compare(super::key_comp()); }

	allocator_type get_allocator() const { return super::get_allocator(); }

	bool operator==(const skiplist_map& o) const {
		return size() == o.size() && std::equal(begin(), end(), o.begin());
	}

	bool operator!=(const skiplist_map& o) const {
		return !operator==(o);
	}

	void swap(skiplist_map& o) {
		super::swap(o);
	}

	void clear() {
		super::clear();
	}
};

template <typename Key, typename Value, typename Compare, typename Alloc>
bool operator==(const skiplist_map<Key, Value, Compare, Alloc>& a, const skiplist_map<Key, Value, Compare, Alloc>& b) {
	return a.operator==(b);
}

template <typename Key, typename Value, typename Compare, typename Alloc>
bool operator!=(const skiplist_map<Key, Value, Compare, Alloc>& a, const skiplist_map<Key, Value, Compare, Alloc>& b) {
	return !operator==(a, b);
}
#endif
