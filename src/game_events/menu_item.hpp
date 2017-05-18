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
#include "variable.hpp"

class filter_context;
class game_data;
struct map_location;

namespace game_events
{
class wml_menu_item
{
public:
	/**
	 * Constructor for reading from a saved config.
	 * This is the reverse of to_config() and corresponds to reading [menu_item].
	 * Handlers are not initialized.
	 */
	wml_menu_item(const std::string& id, const config& cfg);

	/**
	 * Constructor for items defined in an event.
	 * This is where default values are defined (the other constructors should have
	 * all values to work with).
	 * @param[in]  id          The id of the menu item.
	 * @param[in]  definition  The WML defining this menu item.
 	*/
	wml_menu_item(const std::string& id, const vconfig& definition);

	/**
	 * Constructor for items modified by an event.
	 * (To avoid problems with a menu item's command changing itself, we make a
	 * new menu item instead of modifying the existing one.)
	 * @param[in]  id          The id of the menu item.
	 * @param[in]  definition  The WML defining this menu item.
	 * @param[in]  original    The previous version of the menu item with this id.
	 */
	wml_menu_item(const std::string& id, const vconfig& definition, const wml_menu_item& original);

	/** The id of this item. */
	const std::string& id() const
	{
		return item_id_;
	}

	/**
 	 * The image associated with this menu item.
 	 * The returned string will not be empty; a default will be supplied if needed.
 	 */
	const std::string& image() const;

	/** If true, allow using the menu to trigger this item. */
	bool use_wml_menu() const
	{
		return use_wml_menu_;
	}

	/**
	 * Returns whether or not *this is applicable given the context.
	 * Assumes game variables x1, y1, and unit have been set.
	 * @param[in]  hex  The hex where the menu will appear.
	 */
	bool can_show(const map_location& hex, const game_data& data, filter_context& context) const;

	/**
	 * Causes the event associated with this item to fire.
	 * Also records the event.
	 * This includes recording the previous select event, if applicable.
	 * The undo stack will be cleared if the event mutated the gamestate.
	 *
	 * @param[in] event_hex    The location of the event (where the menu was opened).
	 * @param[in] last_select  The location of the most recent "select" event.
	 */
	void fire_event(const map_location& event_hex, const game_data& data) const;

	/** Removes the implicit event handler for an inlined [command]. */
	void finish_handler() const;

	/** Initializes the implicit event handler for an inlined [command]. */
	void init_handler() const;

	/**
	 * The text to put in a menu for this item.
	 * This will be either translated text or a hotkey identifier.
	 */
	std::string menu_text() const
	{
		// The space is to prevent accidental hotkey binding.
		return use_hotkey_ ? hotkey_id_ : description_.str() + ' ';
	}

	/**
	 * Writes *this to the provided config.
	 * This is the reverse of the constructor from a config and corresponds to
	 * what will appear in [menu_item].
	 */
	void to_config(config& cfg) const;

	bool is_synced() const
	{
		return is_synced_;
	}

private:
	/**
	 * Updates *this based on @a vcfg.
	 * This corresponds to what can appear in [set_menu_item].
	 */
	void update(const vconfig& vcfg);

	/** Updates our command to @a new_command. */
	void update_command(const config& new_command);

private:
	/** The id of this menu item. */
	const std::string item_id_;

	/** The name of this item's event(s); based on the item's id. */
	const std::string event_name_;

	/** The id for this item's hotkey; based on the item's id. */
	const std::string hotkey_id_;

	/** The image to display in the menu next to this item's description. */
	std::string image_;

	/** The text to display in the menu for this item. */
	t_string description_;

	/** Whether or not this event says it makes use of the last selected unit. */
	bool needs_select_;

	/**
	 * A condition that must hold in order for this menu item to be visible.
	 * (An empty condition always holds.)
	 *
	 * When used, we need a vconfig instead of a config.
	 */
	vconfig show_if_;

	/**
	 * A location filter to be applied to the hex where the menu is invoked.
	 * (An empty filter always passes.)
	 *
	 * When used, we need a vconfig instead of a config.
	 */
	vconfig filter_location_;

	/** Actions to take when this item is chosen. */
	config command_;

	/** Config object containing the default hotkey. */
	config default_hotkey_;

	/** If true, allow using a hotkey to trigger this item. */
	bool use_hotkey_;

	/** If true, allow using the menu to trigger this item. */
	bool use_wml_menu_;

	/**
	 * If true, this item will be sended ot ther clients.
	 * The command shall not change the gamestate if !is_synced_
	 */
	bool is_synced_;
};

} // end namespace game_events
