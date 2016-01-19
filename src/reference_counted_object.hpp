/*
   Copyright (C) 2008 - 2016 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef REFERENCE_COUNTED_OBJECT_HPP_INCLUDED
#define REFERENCE_COUNTED_OBJECT_HPP_INCLUDED

#include <boost/intrusive_ptr.hpp>

class reference_counted_object
{
public:
	reference_counted_object() : count_(0) {}
	reference_counted_object(const reference_counted_object& /*obj*/) : count_(0) {}
	reference_counted_object& operator=(const reference_counted_object& /*obj*/) {
		return *this;
	}
	virtual ~reference_counted_object() {}

	void add_ref() const { ++count_; }
	void dec_ref() const { if(--count_ == 0) { delete const_cast<reference_counted_object*>(this); } }

	int refcount() const { return count_; }

protected:
	void turn_reference_counting_off() const { count_ = 1000000; }
private:
	mutable int count_;
};

inline void intrusive_ptr_add_ref(const reference_counted_object* obj) {
	obj->add_ref();
}

inline void intrusive_ptr_release(const reference_counted_object* obj) {
	obj->dec_ref();
}

typedef boost::intrusive_ptr<reference_counted_object> object_ptr;
typedef boost::intrusive_ptr<const reference_counted_object> const_object_ptr;

#endif
