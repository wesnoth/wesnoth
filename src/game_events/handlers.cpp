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
 * The structure that tracks WML event handlers.
 * (Typically, handlers are defined by [event] tags.)
 */

#include "game_events/handlers.hpp"
#include "game_events/manager_impl.hpp"
#include "game_events/menu_item.hpp"
#include "game_events/pump.hpp"

#include "formula/string_utils.hpp"
#include "game_data.hpp"
#include "log.hpp"
#include "reports.hpp"
#include "scripting/game_lua_kernel.hpp"
#include "serialization/string_utils.hpp"
#include "sound.hpp"
#include "soundsource.hpp"

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

event_handler::event_handler(const config& cfg, bool imi)
	: first_time_only_(cfg["first_time_only"].to_bool(true))
	, is_menu_item_(imi)
	, disabled_(false)
	, cfg_(cfg)
{
}

void event_handler::disable()
{
	assert(!disabled_);
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

bool event_handler::matches_name(const std::string& name, const game_data* gd) const
{
	const std::string my_names = !gd
		? cfg_["name"].str()
		: utils::interpolate_variables_into_string(cfg_["name"], *gd);

	std::string::const_iterator
		itor, it_begin = my_names.begin(),
		it_end         = my_names.end(),
		match_it       = name.begin(),
		match_begin    = name.begin(),
		match_end      = name.end();

	int skip_count = 0;
	for(itor = it_begin; itor != it_end; ++itor) {
		bool do_eat = false, do_skip = false;

		switch(*itor) {
		case ',':
			if(itor - it_begin - skip_count == match_it - match_begin && match_it == match_end) {
				return true;
			}
			it_begin = itor + 1;
			match_it = match_begin;
			skip_count = 0;
			continue;
		case '\f':
		case '\n':
		case '\r':
		case '\t':
		case '\v':
			do_skip = (match_it == match_begin || match_it == match_end);
			break;
		case ' ':
			do_skip = (match_it == match_begin || match_it == match_end);
			FALLTHROUGH;
		case '_':
			do_eat = (match_it != match_end && (*match_it == ' ' || *match_it == '_'));
			break;
		default:
			do_eat = (match_it != match_end && *match_it == *itor);
			break;
		}

		if(do_eat) {
			++match_it;
		} else if(do_skip) {
			++skip_count;
		} else {
			itor = std::find(itor, it_end, ',');
			if(itor == it_end) {
				return false;
			}
			it_begin = itor + 1;
			match_it = match_begin;
			skip_count = 0;
		}
	}

	if(itor - it_begin - skip_count == match_it - match_begin && match_it == match_end) {
		return true;
	}

	return false;
}

} // end namespace game_events
