/*
	Copyright (C) 2020 - 2025
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

/**
 * Allows creation of lua_ptr<T> instances, but does not affect the lifetime of the T itself.
 * This allows the instance of T to be deleted while the lua_ptr<T>s still exist.
 *
 * The implementation details are making a shared_ptr<non-owning raw pointer to T>, with that
 * shared_ptr owned by the instance of T that's being pointed to, which is then used to create
 * weak_ptrs. The lua_ptr is a wrapper to make sure that no-one gets a second non-temporary
 * shared_ptr<non-owning raw pointer T>. As there's only one shared_ptr, the weak_ptrs become
 * invalid (but defined behavior) when the instance of T is deleted, because that deletes the
 * shared_ptr.
 */
template<typename T>
class enable_lua_ptr
{
public:
	enable_lua_ptr(T* tp) : self_(std::make_shared<T*>(tp)) {}

	/**
	 * The weak_ptrs are pointing to o's shared_ptr's control block, so to keep existing
	 * weak_ptrs valid the existing control block is reused instead of replaced.
	 *
	 * After the move, the existing control block will point to the new T, and o.self_ will be
	 * empty (which is guaranteed by the specification for std::shared_ptr's move assignment
	 * operator).
	 */
	enable_lua_ptr(enable_lua_ptr&& o) : self_(std::move(o.self_))
	{
		*self_ = static_cast<T*>(this);
	}
	enable_lua_ptr& operator=(enable_lua_ptr&& o)
	{
		self_ = std::move(o.self_);
		*self_ = static_cast<T*>(this);
		return *this;
	}
private:
	enable_lua_ptr(const enable_lua_ptr& o) = delete;
	enable_lua_ptr& operator=(const enable_lua_ptr& o) = delete;
	friend class lua_ptr<T>;
	std::shared_ptr<T*> self_;
};

/** T must inherit enable_lua_ptr<T> */
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
	T* operator->()
	{
		return get_ptr();
	}
	operator bool() const
	{
		return bool(self_.lock());
	}
	bool operator!() const
	{
		return !operator bool();
	}
	std::weak_ptr<T*> self_;
};
