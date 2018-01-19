/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
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

#include "game_events/wmi_manager.hpp"
#include "game_events/menu_item.hpp"
#include "play_controller.hpp"
#include "resources.hpp"

#include "config.hpp"
#include "game_data.hpp"
#include "log.hpp"
#include "map/location.hpp"

static lg::log_domain log_engine("engine");
#define WRN_NG LOG_STREAM(warn, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)

// This file is in the game_events namespace.
namespace game_events
{
wmi_manager::wmi_manager()
	: wml_menu_items_()
{
}

/**
 * Destructor.
 * Default implementation, but defined here because this function needs to be
 * able to see wml_menu_item's destructor.
 */
wmi_manager::~wmi_manager()
{
}

/** Erases the item with id @a key. */
bool wmi_manager::erase(const std::string& id)
{
	// Locate the item to erase.
	const auto iter = wml_menu_items_.find(id);

	if(iter == wml_menu_items_.end()) {
		WRN_NG << "Trying to remove non-existent menu item '" << id << "'; ignoring." << std::endl;
		// No such item.
		return false;
	}

	// Clean up our bookkeeping.
	iter->second->finish_handler();

	// Remove the item from the map.
	wml_menu_items_.erase(iter);

	return true; // Erased one item.
}

/**
 * Fires the menu item with the given @a id.
 * @returns true if a matching item was found (even if it could not be fired).
 * NOTE: The return value could be altered if it is decided that
 * play_controller::execute_command() needs something different.
 */
bool wmi_manager::fire_item(
		const std::string& id, const map_location& hex, game_data& gamedata, filter_context& fc, unit_map& units) const
{
	// Does this item exist?
	item_ptr wmi = get_item(id);
	if(!wmi) {
		return false;
	}

	// Prepare for can show().
	gamedata.get_variable("x1") = hex.wml_x();
	gamedata.get_variable("y1") = hex.wml_y();
	scoped_xy_unit highlighted_unit("unit", hex, units);

	// Can this item be shown?
	if(wmi->can_show(hex, gamedata, fc)) {
		wmi->fire_event(hex, gamedata);
	}
	return true;
}

/**
 * Returns the menu items that can be shown for the given location.
 *
 * @param[out] items        Pointers to applicable menu items will be pushed onto @a items.
 * @param[out] descriptions Menu item text will be pushed onto @a descriptions (in the same order as @a items).
 */
void wmi_manager::get_items(const map_location& hex,
		std::vector<std::shared_ptr<const wml_menu_item>>& items,
		std::vector<config>& descriptions,
		filter_context& fc,
		game_data& gamedata,
		unit_map& units) const
{
	if(empty()) {
		// Nothing to do (skip setting game variables).
		return;
	}

	// Prepare for can show().
	gamedata.get_variable("x1") = hex.wml_x();
	gamedata.get_variable("y1") = hex.wml_y();
	scoped_xy_unit highlighted_unit("unit", hex, units);

	// Check each menu item.
	for(const auto& item_pair : wml_menu_items_) {
		item_ptr item = item_pair.second;

		// Can this item be shown?
		if(item->use_wml_menu() && (!item->is_synced() || resources::controller->can_use_synced_wml_menu())
				&& item->can_show(hex, gamedata, fc)) {
			// Include this item.
			items.push_back(item);
			descriptions.emplace_back(config {"id", item->menu_text()});
		}
	}
}

wmi_manager::item_ptr wmi_manager::get_item(const std::string& id) const
{
	auto iter = wml_menu_items_.find(id);
	if(iter != wml_menu_items_.end()) {
		return iter->second;
	}

	return nullptr;
}

/**
 * Initializes the implicit event handlers for inlined [command]s.
 */
void wmi_manager::init_handlers() const
{
	// Applying default hotkeys here currently does not work because
	// the hotkeys are reset by play_controler::init_managers() ->
	// display_manager::display_manager, which is called after this.
	// The result is that default wml hotkeys will be ignored if wml
	// hotkeys are set to default in the preferences menu. (They are
	// still reapplied if set_menu_item is called again, for example
	// by starting a new campaign.) Since it isn't that important
	// I'll just leave it for now.

	unsigned wmi_count = 0;

	// Loop through each menu item.
	for(const auto& item : wml_menu_items_) {
		// If this menu item has a [command], add a handler for it.
		item.second->init_handler();

		// Count the menu items (for the diagnostic message).
		++wmi_count;
	}

	// Diagnostic:
	if(wmi_count > 0) {
		LOG_NG << wmi_count << " WML menu items found, loaded." << std::endl;
	}
}

void wmi_manager::to_config(config& cfg) const
{
	// Loop through our items.
	for(const auto& item : wml_menu_items_) {
		// Add this item as a child of cfg.
		item.second->to_config(cfg.add_child("menu_item"));
	}
}

/**
 * Updates or creates (as appropriate) the menu item with the given @a id.
 */
void wmi_manager::set_item(const std::string& id, const vconfig& menu_item)
{
	auto iter = wml_menu_items_.begin();
	bool success;

	// First, try to insert a brand new menu item.
	std::tie(iter, success) = wml_menu_items_.emplace(id, item_ptr(new wml_menu_item(id, menu_item)));

	// If an entry already exists, reset it.
	if(!success) {
		// Create a new menu item based on the old. This leaves the old item
		// alone in case someone else is holding on to (and processing) it.
		iter->second.reset(new wml_menu_item(id, menu_item, *iter->second));
	}
}

/**
 * Sets the current menu items to the "menu_item" children of @a cfg.
 */
void wmi_manager::set_menu_items(const config& cfg)
{
	wml_menu_items_.clear();
	for(const config& item : cfg.child_range("menu_item")) {
		if(!item.has_attribute("id")) {
			continue;
		}

		const std::string& id = item["id"];
		bool success;

		std::tie(std::ignore, success) = wml_menu_items_.emplace(id, item_ptr(new wml_menu_item(id, item)));

		if(!success) {
			WRN_NG << "duplicate menu item (" << id << ") while loading from config" << std::endl;
		}
	}
}

} // end namespace game_events
