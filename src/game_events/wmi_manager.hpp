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

#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

class config;
class filter_context;
class game_data;
struct map_location;
class unit_map;
class vconfig;

namespace game_events
{
class wml_menu_item;

class wmi_manager
{
public:
	/** wml_menu_item pointers */
	using item_ptr = std::shared_ptr<wml_menu_item>;

	wmi_manager();
	~wmi_manager();

	/** Returns true if no menu items are being managed. */
	bool empty() const
	{
		return wml_menu_items_.empty();
	}

	/** Erases the item with the provided @a id. */
	bool erase(const std::string& id);

	/** Fires the menu item with the given @a id. */
	bool fire_item(const std::string& id,
			const map_location& hex,
			game_data& gamedata,
			filter_context& fc,
			unit_map& units) const;

	/**
	 * Gets the menu item with the specified ID.
	 *
	 * @param               Item id.
	 * @returns             Pointer to the relavent item, or nullptr if not found.
	 */
	item_ptr get_item(const std::string& id) const;

	/** Returns the menu items that can be shown for the given location. */
	void get_items(const map_location& hex,
			std::vector<std::shared_ptr<const wml_menu_item>>& items,
			std::vector<config>& descriptions,
			filter_context& fc,
			game_data& gamedata,
			unit_map& units) const;

	/** Initializes the implicit event handlers for inlined [command]s. */
	void init_handlers() const;

	void to_config(config& cfg) const;

	/** Updates or creates (as appropriate) the menu item with the given @a id. */
	void set_item(const std::string& id, const vconfig& menu_item);

	/** Sets the current menu items to the "menu_item" children of @a cfg. */
	void set_menu_items(const config& cfg);

	/** Gets the number of menu items owned. */
	size_t size() const
	{
		return wml_menu_items_.size();
	}

private:
	std::map<std::string, item_ptr> wml_menu_items_;
};

} // end namespace game_events
