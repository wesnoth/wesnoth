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
#include "../hotkeys.hpp"
#include "../log.hpp"
#include "../play_controller.hpp"

#include <boost/foreach.hpp>

static lg::log_domain log_engine("engine");
#define WRN_NG LOG_STREAM(warn, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)


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

/** Erases the item pointed to by @a pos. */
void wmi_container::erase(const iterator & pos)
{
	// Convert iterator to map_t::iterator.
	const map_t::iterator & iter = pos.get(key());

	// Release the wml_menu_item.
	delete iter->second;
	// Remove the now-defunct pointer from the map.
	wml_menu_items_.erase(iter);
}

/** Erases the item with id @a key. */
wmi_container::size_type wmi_container::erase(const std::string & key)
{
	// Locate the item to erase.
	iterator pos = find(key);

	if ( pos == end() )
		// No such item.
		return 0;

	// Pass the buck.
	erase(pos);
	return 1; // Erased one item.
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
 * Initializes the implicit event handlers for inlined [command]s.
 */
void wmi_container::init_handlers() const
{
	unsigned wmi_count = 0;

	// Loop through each menu item.
	BOOST_FOREACH( const wml_menu_item & wmi, *this ) {
		// If this menu item has a [command], add a handler for it.
		const config & wmi_command = wmi.command();
		if ( !wmi_command.empty() ) {
			add_event_handler(wmi_command, true);
			if ( wmi.use_hotkey() ) {
				// Applying default hotkeys here currently does not work because
				// the hotkeys are reset by play_controler::init_managers() ->
				// display_manager::display_manager, which is called after this.
				// The result is that default wml hotkeys will be ignored if wml
				// hotkeys are set to default in the preferences menu. (They are
				// still reapplied if set_menu_item is called again, for example
				// by starting a new campaign.) Since it isn't that important
				// I'll just leave it for now.
				hotkey::add_wml_hotkey(play_controller::wml_menu_hotkey_prefix + wmi.id(), wmi.description(), wmi.default_hotkey());
			}
		}
		// Count the menu items (for the diagnostic message).
		++wmi_count;
	}

	// Diagnostic:
	if ( wmi_count > 0 ) {
		LOG_NG << wmi_count << " WML menu items found, loaded." << std::endl;
	}
}

void wmi_container::to_config(config& cfg)
{
	// Loop through our items.
	for ( const_iterator j = begin(), wmi_end = end(); j != wmi_end; ++j )
		// Add this item as a child of cfg.
		j->to_config(cfg.add_child("menu_item"));
}

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

