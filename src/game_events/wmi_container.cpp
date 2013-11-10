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
 * Definitions for a container for wml_menu_item.
 */

#include "../global.hpp"
#include "wmi_container.hpp"
#include "handlers.hpp"
#include "menu_item.hpp"

#include "../config.hpp"
#include "../gamestatus.hpp"
#include "../hotkeys.hpp"
#include "../log.hpp"
#include "../map_location.hpp"
#include "../resources.hpp"

#include <boost/foreach.hpp>

static lg::log_domain log_engine("engine");
#define WRN_NG LOG_STREAM(warn, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)

static const size_t MAX_WML_COMMANDS = 7;


// This file is in the game_events namespace.
namespace game_events
{

wmi_container::wmi_container()
	: wml_menu_items_()
{}

wmi_container::wmi_container(const wmi_container& container)
	: wml_menu_items_()
{
	copy(container.wml_menu_items_);
}


/**
 * Performs a deep copy, replacing our current contents.
 * Used by assignment and the copy constructor.
 */
void wmi_container::copy(const map_t & source)
{
	// Safety measure.
	if ( &source == &wml_menu_items_ )
		return;

	// Free up the old memory.
	clear_wmi();

	const map_t::const_iterator source_end = source.end();
	for ( map_t::const_iterator itor = source.begin(); itor != source_end; ++itor )
		// Deep copy.
		wml_menu_items_[itor->first] = new wml_menu_item(*(itor->second));
}

void wmi_container::clear_wmi()
{
	const map_t::iterator i_end = wml_menu_items_.end();
	for ( map_t::iterator i = wml_menu_items_.begin(); i != i_end; ++i ) {
		// Release the wml_menu_item.
		delete i->second;
	}

	wml_menu_items_.clear();
}

/** Erases the item with id @a key. */
wmi_container::size_type wmi_container::erase(const std::string & id)
{
	// Locate the item to erase.
	const map_t::iterator iter = wml_menu_items_.find(id);

	if ( iter == wml_menu_items_.end() ) {
		WRN_NG << "Trying to remove non-existent menu item '" << id << "'; ignoring.\n";
		// No such item.
		return 0;
	}

	// Clean up our bookkeeping.
	remove_wmi_change(id);
	remove_event_handler(id);

	// Release the wml_menu_item.
	delete iter->second;
	// Remove the now-defunct pointer from the map.
	wml_menu_items_.erase(iter);

	return 1; // Erased one item.
}

/**
 * Commits a single WML menu item command change.
 * Returns true if hotkeys have changed (so they need to be saved).
 */
bool wmi_container::commit_change(const std::string & id, config & command)
{
	const bool is_empty_command = command.empty();
	bool hotkeys_changed = false;

	wml_menu_item & item = get_item(id);
	const std::string & event_name = item.event_name();

	config::attribute_value & event_id = command["id"];
	if ( event_id.empty() && !id.empty() ) {
		event_id = id;
	}
	command["name"] = event_name;
	command["first_time_only"] = false;

	if ( !item.command().empty() ) {
		for ( manager::iteration hand(event_name); hand.valid(); ++hand ) {
			if ( hand->is_menu_item() ) {
				LOG_NG << "changing command for " << event_name << " to:\n" << command;
				*hand = event_handler(command, true);
			}
		}
	} else if(!is_empty_command) {
		LOG_NG << "setting command for " << event_name << " to:\n" << command;
		add_event_handler(command, true);
		if(item.use_hotkey()) {
			const config & default_hotkey = item.default_hotkey();
			hotkey::add_wml_hotkey(item.menu_text(), item.description(), default_hotkey);
			if ( !default_hotkey.empty() )
				hotkeys_changed = true;
		}
	}

	item.set_command(command);
	return hotkeys_changed;
}

/**
 * Fires the menu item with the given @a id.
 * @returns true if a matching item was found (even if it could not be fired).
 * NOTE: The return value could be altered if it is decided that
 * play_controller::execute_command() needs something different.
 */
bool wmi_container::fire_item(const std::string & id, const map_location & hex) const
{
	// Does this item exist?
	const_iterator iter = find(id);
	if ( iter == end() )
		return false;

	// Prepare for can show().
	resources::gamedata->get_variable("x1") = hex.x + 1;
	resources::gamedata->get_variable("y1") = hex.y + 1;
	scoped_xy_unit highlighted_unit("unit", hex.x, hex.y, *resources::units);

	// Can this item be shown?
	if ( iter->can_show(hex) )
		iter->fire_event(hex);

	return true;
}

/**
 * Returns an item with the given id.
 * If one does not already exist, one will be created.
 */
wml_menu_item & wmi_container::get_item(const std::string& id)
{
	// Try to insert a dummy value. This combines looking for an existing
	// entry with insertion.
	
	// the static cast fixes http://connect.microsoft.com/VisualStudio/feedback/details/520043/
	// c++11's nullptr would be a better solution as soon as we support it.
	map_t::iterator add_it = wml_menu_items_.insert(map_t::value_type(id, static_cast<wml_menu_item *>(NULL))).first;

	// If we ended up with a dummy value, create an entry for it.
	if ( add_it->second == NULL )
		add_it->second = new wml_menu_item(id);

	// Return the item.
	return *add_it->second;
}

/**
 * Returns the menu items that can be shown for the given location.
 * The number of items returned is limited by MAX_WML_COMMANDS.
 * @param[out] items        Pointers to applicable menu items will be pushed onto @a items.
 * @param[out] descriptions Menu item text will be pushed onto @descriptions (in the same order as @a items).
 */
void wmi_container::get_items(const map_location& hex,
                              std::vector<const wml_menu_item *> & items,
                              std::vector<std::string> & descriptions) const
{
	size_t item_count = 0;

	if ( empty() )
		// Nothing to do (skip setting game variables).
		return;

	// Prepare for can show().
	resources::gamedata->get_variable("x1") = hex.x + 1;
	resources::gamedata->get_variable("y1") = hex.y + 1;
	scoped_xy_unit highlighted_unit("unit", hex.x, hex.y, *resources::units);

	// Check each menu item.
	BOOST_FOREACH( const wml_menu_item & item, *this )
	{
		// Can this item be shown?
		if ( item.use_wml_menu() && item.can_show(hex) )
		{
			// Include this item.
			items.push_back(&item);
			descriptions.push_back(item.menu_text());

			// Limit how many items can be returned.
			if ( ++item_count >= MAX_WML_COMMANDS )
				return;
		}
	}
}

/**
 * Initializes the implicit event handlers for inlined [command]s.
 */
void wmi_container::init_handlers() const
{
	unsigned wmi_count = 0;

	// Loop through each menu item.
	BOOST_FOREACH( const wml_menu_item & wmi, *this ) {
		// If this menu item has a [command], add a handler for it.
		wmi.init_handler();
		// Count the menu items (for the diagnostic message).
		++wmi_count;
	}

	// Diagnostic:
	if ( wmi_count > 0 ) {
		LOG_NG << wmi_count << " WML menu items found, loaded." << std::endl;
	}
}

void wmi_container::to_config(config& cfg) const
{
	// Loop through our items.
	for ( const_iterator j = begin(), wmi_end = end(); j != wmi_end; ++j )
		// Add this item as a child of cfg.
		j->to_config(cfg.add_child("menu_item"));
}

/**
 * Updates or creates (as appropriate) the menu item with the given @a id.
 */
void wmi_container::set_item(const std::string& id, const vconfig& menu_item)
{
	// Get the item and update it.
	get_item(id).update(menu_item);
}

/**
 * Sets the current menu items to the "menu_item" children of @a cfg.
 */
void wmi_container::set_menu_items(const config& cfg)
{
	clear_wmi();
	BOOST_FOREACH(const config &item, cfg.child_range("menu_item"))
	{
		if(!item.has_attribute("id")){ continue; }

		std::string id = item["id"];
		wml_menu_item*& mref = wml_menu_items_[id];
		if(mref == NULL) {
			mref = new wml_menu_item(id, &item);
		} else {
			WRN_NG << "duplicate menu item (" << id << ") while loading from config\n";
		}
	}
}

} // end namespace game_events

