/*
   Copyright (C) 2009 - 2016 by Chris Hopman <cjhopman@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef SHARED_OBJECT_HPP_INCLUDED
#define SHARED_OBJECT_HPP_INCLUDED

#include <algorithm>
#include <cassert>
#include <climits>
#include <cstddef>
#include <ostream>

template <typename T>
struct shared_node {
	T val;
	mutable unsigned long count;
	shared_node() : val(), count(0) { }
	shared_node(const T& o) : val(o), count(0) { }
	static const unsigned long max_count = ULONG_MAX;
};

template <typename T, typename node = shared_node<T> >
class shared_object {
public:
	typedef T type;

	static const node* insert_into_index(const node &);
	static void erase_from_index(const node *);

	shared_object() : val_(NULL) { set(T()); }

	explicit shared_object(const T &o) : val_(NULL) { set(o); }

	shared_object(const shared_object& o) : val_(o.val_) {
		assert(valid());
		val_->count++;
	}

	operator const T &() const {
		assert(valid());
		return val_->val;
	}

	shared_object& operator=(const shared_object& o) {
		if (val_ == o.val_) return *this;
		shared_object tmp(o);
		swap(tmp);
		return *this;
	}

	/// Assignment from the template type is supported.
	shared_object& operator=(const T& o) {
		shared_object tmp(o);
		swap(tmp);
		return *this;
	}

	~shared_object() { clear(); }

	void set(const T& o) {
		if (valid() && o == get()) return;
		clear();

		val_ = insert_into_index(node(o)); //&*index().insert(node(o)).first;
		val_->count++;

		assert((val_->count) < (node::max_count));
	}

	const T& get() const {
		assert(valid());
		return val_->val;
	}

	void swap(shared_object& o) {
		std::swap(val_, o.val_);
	}

	const node* ptr() const {
		return val_;
	}

protected:

	const node* val_;

	bool valid() const {
		return val_ != NULL;
	}

	void clear() {
		if (!valid()) return;
		val_->count--;

		if (val_->count == 0) erase_from_index(val_); //index().erase(index().find(val_->val));
		val_ = NULL;
	}

};

template <typename T>
bool operator==(const shared_object<T>& a, const shared_object<T>& b) {
	return a.ptr() == b.ptr() ? true : a.get() == b.get();
}

template <typename T>
bool operator<(const shared_object<T>& a, const shared_object<T>& b) {
	assert(a.valid());
	assert(b.valid());
	return a.get() < b.get();
}

template <typename T>
std::ostream& operator<<(std::ostream& stream, const shared_object<T>& o) {
	assert(o.valid());
	return stream << o.get();
}

template <typename T>
std::istream& operator>>(std::istream& stream, shared_object<T>& o) {
	T t;
	std::istream& ret = stream >> t;
	o.set(t);
	return ret;
}

#endif
