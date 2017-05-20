/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Define the handlers for the game's events mechanism.
 *
 * Events might be units moving or fighting, or when victory or defeat occurs.
 * A scenario's configuration file will define actions to take when certain events occur.
 * This module is responsible for tracking these definitions.
 */

#pragma once

#include "config.hpp"
#include "utils/smart_list.hpp"

#include <memory>
#include <set>
#include <string>

class game_data;
class game_lua_kernel;

namespace game_events
{
struct queued_event;
class event_handler; // Defined a few lines down.
class manager;

/// Shared pointer to handler objects.
typedef std::shared_ptr<event_handler> handler_ptr;

/// Storage of event handlers.
typedef std::vector<handler_ptr> handler_vec;

class event_handler
{
public:
	event_handler(const config& cfg, bool is_menu_item, handler_vec::size_type index, manager&);

	/** The index of *this should only be of interest when controlling iterations. */
	handler_vec::size_type index() const
	{
		return index_;
	}

	bool matches_name(const std::string& name, const game_data* data) const;

	bool is_menu_item() const
	{
		return is_menu_item_;
	}

	/** Disables *this, removing it from the game. */
	void disable();
	void handle_event(const queued_event& event_info, handler_ptr& handler_p, game_lua_kernel&);

	const config& get_config() const
	{
		return cfg_;
	}

private:
	bool first_time_only_;
	bool is_menu_item_;
	handler_vec::size_type index_;
	manager* man_;
	config cfg_;
};

/**
 * This is a wrapper for a list of weak pointers to handlers. It allows forward
 * iterations of the list, with each element returned as a shared pointer.
 * (Weak pointers that fail to lock are silently removed from the list.) These
 * iterations can be used recursively, even when the innermost iteration might
 * erase arbitrary elements from the list.
 *
 * The interface is not the standard list interface because that would be
 * inconvenient. The functionality implemented is that required by Wesnoth.
 */
class handler_list
{
	/// The weak pointers that are used internally.
	typedef std::weak_ptr<event_handler> internal_ptr;

	/// The underlying list.
	typedef utils::smart_list<internal_ptr> list_t;

public:
	/**
	 * Handler list iterators are rather limited. They can be constructed
	 * from a reference iterator (not default constructed), incremented,
	 * and dereferenced. Consecutive dereferences are not guaranteed to
	 * return the same element (if the list mutates between them, the next
	 * element might be returned). An increment guarantees that the next
	 * dereference will differ from the previous (unless at the end of the
	 * list). The end of the list is indicated by dereferencing to a null
	 * pointer.
	 */
	class iterator
	{
		/// The current element.
		list_t::iterator iter_;

	public:
		/// Initialized constructor (to be called by handler_list).
		explicit iterator(const list_t::iterator& base_iter)
			: iter_(base_iter)
		{
		}

		/// Increment.
		iterator& operator++()
		{
			++iter_;
			return *this;
		}
		/// Dereference.
		handler_ptr operator*();
	};
	friend class iterator;
	typedef iterator const_iterator;

public:
	/**
	 * Default constructor.
	 * Note: This explicit definition is required (by the more pedantic
	 *       compilers) in order to declare a default-constructed, static,
	 *       and const variable in event_handlers::get(), in handlers.cpp.
	 */
	handler_list()
		: data_()
	{
	}

	const_iterator begin() const
	{
		return iterator(const_cast<list_t&>(data_).begin());
	}

	// The above const_cast is so the iterator can remove obsolete entries.
	const_iterator end() const
	{
		return iterator(const_cast<list_t&>(data_).end());
	}

	// push_front() is probably unneeded, but I'll leave the code here, just in case.
	// (These lists must be maintained in index order, which means pushing to the back.)
	void push_front(const handler_ptr& p)
	{
		data_.push_front(internal_ptr(p));
	}

	void push_back(const handler_ptr& p)
	{
		data_.push_back(internal_ptr(p));
	}

	void clear()
	{
		data_.clear();
	}

private:
	/// No implementation of operator=() since smart_list does not support it.
	handler_list& operator=(const handler_list&);

	/// The actual list.
	list_t data_;
};
}
