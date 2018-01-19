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
 * The structure that tracks WML event handlers.
 * (Typically, handlers are defined by [event] tags.)
 */

#include "game_events/handlers.hpp"

#include "formula/string_utils.hpp"
#include "game_data.hpp"
#include "log.hpp"
#include "scripting/game_lua_kernel.hpp"
#include "serialization/string_utils.hpp"
#include "sound.hpp"
#include "variable.hpp"

#include <iostream>

static lg::log_domain log_engine("engine");
#define DBG_NG LOG_STREAM(debug, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)
#define WRN_NG LOG_STREAM(warn, log_engine)

static lg::log_domain log_event_handler("event_handler");
#define DBG_EH LOG_STREAM(debug, log_event_handler)

// This file is in the game_events namespace.
namespace game_events
{
/* ** event_handler ** */

event_handler::event_handler(config&& cfg, bool imi, const std::vector<std::string>& types)
	: first_time_only_(cfg["first_time_only"].to_bool(true))
	, is_menu_item_(imi)
	, disabled_(false)
	, cfg_(cfg)
	, types_(types)
{
}

void event_handler::disable()
{
	assert(!disabled_ && "Trying to disable a disabled event. Shouldn't happen!");
	disabled_ = true;
}

void event_handler::handle_event(const queued_event& event_info, game_lua_kernel& lk)
{
	if(disabled_) {
		return;
	}

	if(is_menu_item_) {
		DBG_NG << cfg_["name"] << " will now invoke the following command(s):\n" << cfg_;
	}

	if(first_time_only_) {
		disable();
	}

	lk.run_wml_action("command", vconfig(cfg_, false), event_info);
	sound::commit_music_changes();
}

} // end namespace game_events
