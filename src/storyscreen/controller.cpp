/* $Id$ */
/*
   Copyright (C) 2003 - 2009 by David White <dave@whitevine.net>
   Copyright (C) 2009 by Ignacio R. Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file storyscreen/controller.cpp
 * This code is work in progress, and the interfaces may change.
 * It is supposed to completely replace the old story screens code
 * at intro.cpp, introducing new WML conventions while at it.
 */

#include "global.hpp"
#include "SDL.h"

#include "storyscreen/controller.hpp"
#include "storyscreen/page.hpp"

#include "asserts.hpp"
#include "foreach.hpp"
#include "variable.hpp"

#include "display.hpp"
#include "game_events.hpp"
#include "gamestatus.hpp"
#include "gettext.hpp"
#include "intro.hpp"
#include "language.hpp"
#include "log.hpp"
#include "sound.hpp"
#include "text.hpp"

#define ERR_NG LOG_STREAM(err , engine)
#define LOG_NG LOG_STREAM(info, engine)
#define ERR_DI LOG_STREAM(err , display)

// TODO: remove when completed
#include "stub.hpp"

namespace storyscreen {

controller::controller(display& disp, const vconfig& data, const std::string& scenario_name)
	: disp_(disp)
	, disp_resize_lock_()
	, evt_context_()
	, data_(data)
	, scenario_name_(scenario_name)
	, pages_()
	, gamestate_(game_events::get_state_of_game())
{
	ASSERT_LOG(gamestate_ != NULL, "Ouch: gamestate is NULL when initializing storyscreen controller");
	build_pages();
}

controller::~controller()
{
	clear_pages();
}

void controller::build_pages()
{


	for(vconfig::all_children_iterator i = data_.ordered_begin(); i != data_.ordered_end(); i++) {
		const std::pair<const std::string, const vconfig> item = *i;

		if(item.first == "page" && !item.second.empty()) {
			vconfig cfg = item.second;
			// Use scenario name as page title if the WML doesn't supply a custom one.
// 			if(cfg["title"].empty()) {
// 				cfg["title"] = scenario_name_;
// 			}

//			page* story_page = new page(*gamestate_, cfg);
		}
		
	}
}

void controller::clear_pages()
{
	foreach(page* p, pages_) {
		delete p;
	}
	pages_.clear();
}

} // end namespace storyscreen
