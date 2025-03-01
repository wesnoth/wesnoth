/*
	Copyright (C) 2003 - 2025
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

namespace utils {

/**
 * Data-based RAII scope guard.
 * Assigns a value to a specific location, then restores the old value once it goes out of scope.
 */
template<typename T>
class guard_value {
	T* ref_;
	T old_val_;
public:
	/**
	 * @param ref The memory location being guarded
	 * @param new_val The new value to temporarily assign to that location
	 */
	guard_value(T& ref, T new_val)
		: ref_(&ref)
		, old_val_(ref)
	{
		ref = new_val;
	}
	~guard_value()
	{
		*ref_ = old_val_;
	}
};

}
