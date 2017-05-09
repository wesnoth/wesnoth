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
 * Declarations for a class that implements WML-defined (right-click) menu items.
 */

#pragma once

#include "config.hpp"
#include "tstring.hpp"
#include "variable.hpp"

class filter_context;
class game_data;
struct map_location;
class unit_map;

namespace game_events
{

class wml_menu_item
{
public:
	/// Constructor for when read from a saved config.
	wml_menu_item(const std::string& id, const config & cfg);
	/// Constructor for when defined in an event.
	wml_menu_item(const std::string& id, const vconfig & definition);
	/// Constructor for when modified by an event.
	wml_menu_item(const std::string& id, const vconfig & definition,
	              const wml_menu_item & original);

	/// The id of this item.
	const std::string & id() const { return item_id_; }
	/// The image associated with this menu item.
	const std::string & image() const;
	/// If true, allow using the menu to trigger this item.
	bool use_wml_menu() const { return use_wml_menu_; }

	/// Returns whether or not *this is applicable given the context.
	bool can_show(const map_location & hex, const game_data & data, filter_context & context) const;
	/// Causes the event associated with this item to fire.
	void fire_event(const map_location & event_hex, const game_data & data) const;
	/// Removes the implicit event handler for an inlined [command].
	void finish_handler() const;
	/// Initializes the implicit event handler for an inlined [command].
	void init_handler() const;
	/// The text to put in a menu for this item.
	/// This will be either translated text or a hotkey identifier.
	std::string menu_text() const
	{ return use_hotkey_ ? hotkey_id_ : description_.str() + ' '; } // The space is to prevent accidental hotkey binding.
	/// Writes *this to the provided config.
	void to_config(config & cfg) const;
	bool is_synced() const { return is_synced_; }
private: // Functions
	/// Updates *this based on @a vcfg.
	void update(const vconfig & vcfg);
	/// Updates our command to @a new_command.
	void update_command(const config & new_command);

private: // Data
	/// The id of this menu item.
	const std::string item_id_;
	/// The name of this item's event(s); based on the item's id.
	const std::string event_name_;
	/// The id for this item's hotkey; based on the item's id.
	const std::string hotkey_id_;
	/// The image to display in the menu next to this item's description.
	std::string image_;
	/// The text to display in the menu for this item.
	t_string description_;
	/// Whether or not this event says it makes use of the last selected unit.
	bool needs_select_;
	/// A condition that must hold in order for this menu item to be visible.
	/// (An empty condition always holds.)
	vconfig show_if_;        	// When used, we need a vconfig instead of a config.
	/// A location filter to be applied to the hex where the menu is invoked.
	/// (An empty filter always passes.)
	vconfig filter_location_;	// When used, we need a vconfig instead of a config.
	/// Actions to take when this item is chosen.
	config command_;
	/// Config object containing the default hotkey.
	config default_hotkey_;
	/// If true, allow using a hotkey to trigger this item.
	bool use_hotkey_;
	/// If true, allow using the menu to trigger this item.
	bool use_wml_menu_;
	/// If true, this item will be sended ot ther clients.
	/// The command shall not change the gamestate if !is_synced_
	bool is_synced_;
};

} // end namespace game_events
