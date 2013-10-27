/*
   Copyright (C) 2003 - 2013 by David White <dave@whitevine.net>
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
 * Declarations for a container for wml_menu_item.
 */

#ifndef GAME_EVENTS_WMI_CONTAINER_HPP_INCLUDED
#define GAME_EVENTS_WMI_CONTAINER_HPP_INCLUDED

#include "iterator.hpp"

#include <map>

class config;


namespace game_events
{
class wml_menu_item;


/// A container of wml_menu_item.
class wmi_container{
	/// The underlying storage type.
	typedef std::map<std::string, wml_menu_item*> map_t;
	/// The key for interaction with our iterators.
	struct key {
		/// Instructions for converting a map_t iterator to a wml_menu_item.
		static const wml_menu_item & eval(const map_t::const_iterator & iter)
		{ return *iter->second; }
	};

public:
	// Typedefs required of a container:
	typedef wml_menu_item          value_type;
	typedef wml_menu_item *        pointer;
	typedef wml_menu_item &        reference;
	typedef const wml_menu_item &  const_reference;
	typedef map_t::difference_type difference_type;
	typedef map_t::size_type       size_type;

	typedef util::iterator_extend      <value_type, map_t, key, key> iterator;
	typedef util::const_iterator_extend<value_type, map_t, key, key> const_iterator;


public:
	wmi_container();
	wmi_container(const wmi_container& container);
	~wmi_container() { clear_wmi(); }

	/// Assignment operator to support deep copies.
	wmi_container & operator=(const wmi_container & that)
	{ copy(that.wml_menu_items_); return *this; }

	void clear_wmi();
	/// Returns true if *this contains no data.
	bool empty() const { return wml_menu_items_.empty(); }
	/// Erases the item pointed to by @a pos.
	void erase(const iterator & pos);
	/// Erases the item with id @a key.
	size_type erase(const std::string & key);

	/// Initializes the implicit event handlers for inlined [command]s.
	void init_handlers() const;
	void to_config(config& cfg);
	void set_menu_items(const config& cfg);

	iterator find(const std::string & id)             { return iterator(wml_menu_items_.find(id)); }
	const_iterator find(const std::string & id) const { return const_iterator(wml_menu_items_.find(id)); }
	/// Returns an item with the given id.
	wml_menu_item & get_item(const std::string& id);

	// Iteration support:
	iterator begin()  { return iterator(wml_menu_items_.begin()); }
	iterator end()    { return iterator(wml_menu_items_.end()); }
	const_iterator begin() const { return const_iterator(wml_menu_items_.begin()); }
	const_iterator end()   const { return const_iterator(wml_menu_items_.end()); }

private:
	/// Performs a deep copy, replacing our current contents.
	void copy(const map_t & source);

private: // data
	map_t wml_menu_items_;
};

} // end namespace game_events

#endif // GAME_EVENTS_WMI_CONTAINER_HPP_INCLUDED

