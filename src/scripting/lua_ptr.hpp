/*
	Copyright (C) 2020 - 2024
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

#include <memory>
template<typename T>
class lua_ptr;

template<typename T>
class enable_lua_ptr
{
public:
	enable_lua_ptr(T* tp) : self_(std::make_shared<T*>(tp)) {}
private:
	friend class lua_ptr<T>;
	std::shared_ptr<T*> self_;
};

/** Tmust inherit enable_lua_ptr<T> */
template<typename T>
class lua_ptr
{
public:
	lua_ptr(enable_lua_ptr<T>& o) : self_(o.self_) { }
	T* get_ptr()
	{
		if(auto pp = self_.lock()) {
			return *pp;
		}
		return nullptr;
	}
	std::weak_ptr<T*> self_;
};
