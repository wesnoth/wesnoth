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
 * Declarations for a container for wml_menu_item.
 */

#ifndef GAME_EVENTS_WMI_CONTAINER_HPP_INCLUDED
#define GAME_EVENTS_WMI_CONTAINER_HPP_INCLUDED

#include "utils/iterator.hpp"

#include <map>
#include <memory>
#include <vector>
#include <string>

class config;
class filter_context;
class game_data;
struct map_location;
class unit_map;
class vconfig;


namespace game_events
{
class wml_menu_item;


/// A container of wml_menu_item.
class wmi_container{
	/// Pointers to our elements.
	typedef std::shared_ptr<wml_menu_item> item_ptr;
	/// The underlying storage type.
	typedef std::map<std::string, item_ptr> map_t;
	/// The key for interaction with our iterators.
	struct key {
		/// Instructions for converting a map_t iterator to an item_ptr.
		static const item_ptr & eval(const map_t::const_iterator & iter)
		{ return iter->second; }
	};

public:
	// Typedefs required of a container:
	typedef item_ptr               value_type;
	typedef value_type *           pointer;
	typedef value_type &           reference;
	typedef const value_type &     const_reference;
	typedef map_t::difference_type difference_type;
	typedef map_t::size_type       size_type;

	typedef utils::iterator_extend      <value_type, map_t, key, key> iterator;
	typedef utils::const_iterator_extend<value_type, map_t, key, key> const_iterator;


public:
	wmi_container();
	~wmi_container();

	/// Returns true if *this contains no data.
	bool empty() const { return wml_menu_items_.empty(); }
	/// Erases the item with the provided @a id.
	size_type erase(const std::string & id);

	/// Fires the menu item with the given @a id.
	bool fire_item(const std::string & id, const map_location & hex, game_data & gamedata, filter_context & fc, unit_map & units) const;
	/// Returns the menu items that can be shown for the given location.
	void get_items(const map_location& hex,
	               std::vector<std::shared_ptr<const wml_menu_item>>& items,
                   std::vector<config>& descriptions,
                   filter_context& fc, game_data& gamedata, unit_map& units) const;
	/// Initializes the implicit event handlers for inlined [command]s.
	void init_handlers() const;
	void to_config(config& cfg) const;
	/// Updates or creates (as appropriate) the menu item with the given @a id.
	void set_item(const std::string& id, const vconfig& menu_item);
	/// Sets the current menu items to the "menu_item" children of @a cfg.
	void set_menu_items(const config& cfg);

private:
	/// Returns an iterator to a menu item with the given @a id, if one exists.
	iterator find(const std::string & id)             { return iterator(wml_menu_items_.find(id)); }
public:
	/// Returns an iterator to a menu item with the given @a id, if one exists.
	const_iterator find(const std::string & id) const { return const_iterator(wml_menu_items_.find(id)); }
	// Iteration support:
	iterator begin()  { return iterator(wml_menu_items_.begin()); }
	iterator end()    { return iterator(wml_menu_items_.end()); }
	const_iterator begin() const { return const_iterator(wml_menu_items_.begin()); }
	const_iterator end()   const { return const_iterator(wml_menu_items_.end()); }

	size_t size() const { return wml_menu_items_.size(); }
private: // data
	map_t wml_menu_items_;
};

} // end namespace game_events

#endif // GAME_EVENTS_WMI_CONTAINER_HPP_INCLUDED

