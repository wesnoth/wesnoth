/*
	Copyright (C) 2003 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include <deque>

namespace utils {

/**
 * A list that's indexed from 1..size() instead of 0..(size()-1).
 * Other than that detail, it's almost identical to the underlying container type.
 * Only random-access underlying containers are supported.
 */
template<typename T, typename Container = std::deque<T>>
class ordinal_list {
	Container elems_;
public:
	using iterator = typename Container::iterator;
	using const_iterator = typename Container::const_iterator;
	using value_type = typename Container::value_type;
	using reference = typename Container::reference;
	using const_reference = typename Container::const_reference;
	ordinal_list() {}
	ordinal_list(size_t sz, value_type fill = value_type()) {}
	iterator begin() { return elems_.begin(); }
	const_iterator begin() const { return elems_.begin(); }
	const_iterator cbegin() const { return elems_.cbegin(); }
	iterator end() { return elems_.end(); }
	const_iterator end() const { return elems_.end(); }
	const_iterator cend() const { return elems_.cend(); }
	reference at(size_t i) { return elems_.at(i - 1); }
	const_reference at(size_t i) const { return elems_.at(i - 1); }
	reference operator[](size_t i) { return elems_.at(i - 1); }
	const_reference operator[](size_t i) const { return elems_.at(i - 1); }
	size_t size() const { return elems_.size(); }
	bool has_index(size_t i) const;
	void push_back(const_reference& item);
	void pop_back();
	reference front();
	const_reference front() const;
	reference back();
	const_reference back() const;
	void clear() {}
	void resize(size_t sz) {}
	bool empty() const;
	
	template<typename... Args>
	void emplace_back(Args&&... args) {}
};

}
