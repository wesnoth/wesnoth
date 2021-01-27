/*
	Copyright (C) 2021 by the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include <functional>
#include <optional>

namespace utils
{
/**
 * A simple wrapper class for optional reference types.
 *
 * Since std::optional (as of C++17 at least) does not support reference types (see [1]),
 * the only way to use those is std::optional<std::reference_wrapper>. However, this makes
 * the interace messy, as to access the referenced object you need an extra get() call to
 * access the value stored in the reference wrapper.
 *
 * This does not rebind operator=() as boost::optional does. Instead, assigning a value
 * to this object will simply change the object to which it points. To change the value
 * of the referred to object, use value() or one of the other operators.
 *
 * [1] https://www.fluentcpp.com/2018/10/05/pros-cons-optional-references/
 */
template<typename T>
class optional_reference
{
public:
	optional_reference() = default;

	optional_reference(T& val)
		: value_(val)
	{
	}

	optional_reference(std::nullopt_t val)
		: value_(val)
	{
	}

	T& value() const
	{
#ifndef __APPLE__
		return value_.value().get();
#else
		if(value_) {
			return value_->get();
		} else {
			throw std::bad_optional_access{};
		}
#endif
	}

	optional_reference<T>& operator=(T& new_val)
	{
		value_ = new_val;
		return *this;
	}

	operator bool() const
	{
		return value_.has_value();
	}

	T* operator->() const
	{
		return &value();
	}

	T& operator*() const
	{
		return value();
	}

private:
	std::optional<std::reference_wrapper<T>> value_;
};

} // namespace utils
