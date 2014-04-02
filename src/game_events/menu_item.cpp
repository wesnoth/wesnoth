/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
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
 * Definitions for a class that implements WML-defined (right-click) menu items.
 */

#include "../global.hpp"
#include "menu_item.hpp"
#include "conditional_wml.hpp"
#include "handlers.hpp"
#include "pump.hpp"

#include "../actions/undo.hpp"
#include "../game_config.hpp"
#include "../gamestatus.hpp"
#include "../log.hpp"
#include "../mouse_handler_base.hpp"
#include "../play_controller.hpp"
#include "../preferences.hpp"
#include "../replay.hpp"
#include "../replay_helper.hpp"
#include "../resources.hpp"
#include "../synced_context.hpp"
#include "../terrain_filter.hpp"

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)


// This file is in the game_events namespace.
namespace game_events
{

namespace { // Some helpers for construction.

	/**
	 * Build the event name associated with the given menu item id.
	 * This is a separate function so it can be easily shared by multiple
	 * constructors.
	 */
	inline std::string make_item_name(const std::string & id)
	{
		return std::string("menu_item") + (id.empty() ? "" : ' ' + id);
	}

	/**
	 * Build the hotkey id associated with the given menu item id.
	 * This is a separate function so it can be easily shared by multiple
	 * constructors.
	 */
	inline std::string make_item_hotkey(const std::string & id)
	{
		return play_controller::wml_menu_hotkey_prefix + id;
	}

}// anonymous namespace


/**
 * Constructor for when read from a saved config.
 * This is the reverse of to_config() and corresponds to reading [menu_item].
 * Handlers are not initialized.
 */
wml_menu_item::wml_menu_item(const std::string& id, const config & cfg) :
		item_id_(id),
		event_name_(make_item_name(id)),
		hotkey_id_(make_item_hotkey(id)),
		image_(cfg["image"].str()),
		description_(cfg["description"].t_str()),
		needs_select_(cfg["needs_select"].to_bool(false)),
		show_if_(cfg.child_or_empty("show_if"), true),
		filter_location_(cfg.child_or_empty("filter_location"), true),
		command_(cfg.child_or_empty("command")),
		default_hotkey_(cfg.child_or_empty("default_hotkey")),
		use_hotkey_(cfg["use_hotkey"].to_bool(true)),
		use_wml_menu_(cfg["use_hotkey"].str() != "only")
{
}

/**
 * Constructor for when defined in an event.
 * This is where default values are defined (the other constructors should have
 * all values to work with).
 * @param[in]  id          The id of the menu item.
 * @param[in]  definition  The WML defining this menu item.
 */
wml_menu_item::wml_menu_item(const std::string& id, const vconfig & definition) :
		item_id_(id),
		event_name_(make_item_name(id)),
		hotkey_id_(make_item_hotkey(id)),
		image_(),
		description_(),
		needs_select_(false),
		show_if_(vconfig::empty_vconfig()),
		filter_location_(vconfig::empty_vconfig()),
		command_(),
		default_hotkey_(),
		use_hotkey_(true),
		use_wml_menu_(true)
{
	// On the off-chance that update() doesn't do it, add the hotkey here.
	// (Update can always modify it.)
	hotkey::add_wml_hotkey(hotkey_id_, description_, default_hotkey_);

	// Apply WML.
	update(definition);
}

/**
 * Constructor for when modified by an event.
 * (To avoid problems with a menu item's command changing itself, we make a
 * new menu item instead of modifying the existing one.)
 * @param[in]  id          The id of the menu item.
 * @param[in]  definition  The WML defining this menu item.
 * @param[in]  original    The previous version of the menu item with this id.
 */
wml_menu_item::wml_menu_item(const std::string& id, const vconfig & definition,
                             const wml_menu_item & original) :
		item_id_(id),
		event_name_(make_item_name(id)),
		hotkey_id_(make_item_hotkey(id)),
		image_(original.image_),
		description_(original.description_),
		needs_select_(original.needs_select_),
		show_if_(original.show_if_),
		filter_location_(original.filter_location_),
		command_(original.command_),
		default_hotkey_(original.default_hotkey_),
		use_hotkey_(original.use_hotkey_),
		use_wml_menu_(original.use_wml_menu_)
{
	// Apply WML.
	update(definition);
}


/**
 * The image associated with this menu item.
 * The returned string will not be empty; a default will be supplied if needed.
 */
const std::string & wml_menu_item::image() const
{
	// Default the image?
	return image_.empty() ? game_config::images::wml_menu : image_;
}

/**
 * Returns whether or not *this is applicable given the context.
 * Assumes game variables x1, y1, and unit have been set.
 * @param[in]  hex  The hex where the menu will appear.
 */
bool wml_menu_item::can_show(const map_location & hex) const
{
	// Failing the [show_if] tag means no show.
	if ( !show_if_.empty() && !conditional_passed(show_if_) )
		return false;

	// Failing the [fiter_location] tag means no show.
	if ( !filter_location_.empty() &&
	     !terrain_filter(filter_location_, *resources::units)(hex) )
		return false;

	// Failing to have a required selection means no show.
	if ( needs_select_ && !resources::gamedata->last_selected.valid() )
		return false;

	// Passed all tests.
	return true;
}

/**
 * Causes the event associated with this item to fire.
 * Also records the event.
 * This includes recording the previous select event, if applicable.
 * The undo stack will be cleared if the event mutated the gamestate.
 *
 * @param[in] event_hex    The location of the event (where the menu was opened).
 * @param[in] last_select  The location of the most recent "select" event.
 */
void wml_menu_item::fire_event(const map_location & event_hex) const
{
	const map_location & last_select = resources::gamedata->last_selected;

	// No new player-issued commands allowed while this is firing.
	const events::command_disabler disable_commands;

	// instead of adding a second "select" event like it sdone before, we just fire the select event again, and this time in a synced context.
	// note that there coudnt be and user choiced durign teh last "select" event because it didn't run in a synced context.
	if ( needs_select_  &&  last_select.valid() )
	{
		synced_context::run_in_synced_context("fire_event",  replay_helper::get_event(event_name_, event_hex, &last_select));
	}
	else
	{
		synced_context::run_in_synced_context("fire_event",  replay_helper::get_event(event_name_, event_hex, NULL));
	}
}

/**
 * Removes the implicit event handler for an inlined [command].
 */
void wml_menu_item::finish_handler() const
{
	if ( !command_.empty() )
		remove_event_handler(command_["id"]);

	// Hotkey support
	if ( use_hotkey_ )
		hotkey::remove_wml_hotkey(hotkey_id_);
}

/**
 * Initializes the implicit event handler for an inlined [command].
 */
void wml_menu_item::init_handler() const
{
	// If this menu item has a [command], add a handler for it.
	if ( !command_.empty() )
		add_event_handler(command_, true);

	// Hotkey support
	if ( use_hotkey_ ) {
		hotkey::add_wml_hotkey(hotkey_id_, description_, default_hotkey_);
	}
}

/**
 * Writes *this to the provided config.
 * This is the reverse of the constructor from a config and corresponds to
 * what will appear in [menu_item].
 */
void wml_menu_item::to_config(config & cfg) const
{
	cfg["id"] = item_id_;
	cfg["image"] = image_;
	cfg["description"] = description_;
	cfg["needs_select"] = needs_select_;

	if ( use_hotkey_ && use_wml_menu_ )
		cfg["use_hotkey"] = true;
	if ( use_hotkey_ && !use_wml_menu_ )
		cfg["use_hotkey"] = "only";
	if ( !use_hotkey_ && use_wml_menu_ )
		cfg["use_hotkey"] = false;
	if ( !use_hotkey_ && !use_wml_menu_ )
	{
		ERR_NG << "Bad data: wml_menu_item with both use_wml_menu and use_hotkey set to false is not supposed to be possible.";
		cfg["use_hotkey"] = false;
	}
	if ( !show_if_.empty() )
		cfg.add_child("show_if", show_if_.get_config());
	if ( !filter_location_.empty() )
		cfg.add_child("filter_location", filter_location_.get_config());
	if ( !command_.empty() )
		cfg.add_child("command", command_);
	if ( !default_hotkey_.empty() )
		cfg.add_child("default_hotkey", default_hotkey_);
}

/**
 * Updates *this based on @a vcfg.
 * This corresponds to what can appear in [set_menu_item].
 */
void wml_menu_item::update(const vconfig & vcfg)
{
	const bool old_use_hotkey = use_hotkey_;
	// Tracks whether or not the hotkey has been updated.
	bool hotkey_updated = false;

	if ( vcfg.has_attribute("image") )
		image_ = vcfg["image"].str();

	if ( vcfg.has_attribute("description") ) {
		description_ = vcfg["description"].t_str();
		hotkey_updated = true;
	}

	if ( vcfg.has_attribute("needs_select") )
		needs_select_ = vcfg["needs_select"].to_bool();

	if ( const vconfig & child = vcfg.child("show_if") ) {
		show_if_ = child;
		show_if_.make_safe();
	}

	if ( const vconfig & child = vcfg.child("filter_location") ) {
		filter_location_ = child;
		filter_location_.make_safe();
	}

	if ( const vconfig & child = vcfg.child("default_hotkey") ) {
		default_hotkey_ = child.get_parsed_config();
		hotkey_updated = true;
	}

	if ( vcfg.has_attribute("use_hotkey") ) {
		const config::attribute_value & use_hotkey_av = vcfg["use_hotkey"];

		use_hotkey_ = use_hotkey_av.to_bool(true);
		use_wml_menu_ = use_hotkey_av.str() != "only";
	}

	if ( const vconfig & cmd = vcfg.child("command") ) {
		const bool delayed = cmd["delayed_variable_substitution"].to_bool(true);
		update_command(delayed ? cmd.get_config() : cmd.get_parsed_config());
	}


	// Update the registered hotkey?

	if ( use_hotkey_ && !old_use_hotkey )
		// The hotkey needs to be enabled.
		hotkey::add_wml_hotkey(hotkey_id_, description_, default_hotkey_);

	else if ( use_hotkey_ && hotkey_updated )
		// The hotkey needs to be updated.
		hotkey::add_wml_hotkey(hotkey_id_, description_, default_hotkey_);

	else if ( !use_hotkey_ && old_use_hotkey )
		// The hotkey needs to be disabled.
		hotkey::remove_wml_hotkey(hotkey_id_);
}

/**
 * Updates our command to @a new_command.
 */
void wml_menu_item::update_command(const config & new_command)
{
	// If there is an old command, remove it from the event handlers.
	if ( !command_.empty() ) {
		manager::iteration iter(event_name_);
		while ( handler_ptr hand = *iter ) {
			if ( hand->is_menu_item() ) {
				LOG_NG << "Removing command for " << event_name_ << ".\n";
				remove_event_handler(command_["id"].str());
			}
			++iter;
		}
	}

	// Update our stored command.
	if ( new_command.empty() )
		command_.clear();
	else {
		command_ = new_command;

		// Set some fields required by event processing.
		config::attribute_value & event_id = command_["id"];
		if ( event_id.empty() && !item_id_.empty() ) {
			event_id = item_id_;
		}
		command_["name"] = event_name_;
		command_["first_time_only"] = false;

		// Register the event.
		LOG_NG << "Setting command for " << event_name_ << " to:\n" << command_;
		add_event_handler(command_, true);
	}
}

} // end namespace game_events

