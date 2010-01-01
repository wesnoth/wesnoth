/* $Id$ */
/*
   Copyright (C) 2009 - 2010 by Ignacio R. Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file storyscreen/interface.cpp
 * Storyscreen controller (wrapper interface).
 */

#include "global.hpp"
#include "foreach.hpp"
#include "variable.hpp"

#include "storyscreen/interface.hpp"
#include "storyscreen/controller.hpp"

#include "display.hpp"
#include "game_events.hpp"
#include "gettext.hpp"
#include "intro.hpp"
#include "language.hpp"
#include "log.hpp"
#include "sound.hpp"
#include "text.hpp"

static lg::log_domain log_engine("engine");
#define LOG_NG LOG_STREAM(info, log_engine)

// TODO: remove when completed
#include "stub.hpp"

namespace {
	void generate_endscreen_part_config(config& append_to_cfg)
	{
		config& partcfg = append_to_cfg.add_child("story").add_child("part");
		partcfg["text_align"] = "centered";
	}
} // end anonymous namespace

void show_storyscreen(display& disp, const vconfig& story_cfg, const std::string& scenario_name)
{
	LOG_NG << "entering storyscreen procedure...\n";

	storyscreen::controller ctl(disp, story_cfg, scenario_name);

	try {
		ctl.show();
	} catch(storyscreen::controller::quit const&) {
		LOG_NG << "leaving storyscreen for titlescreen...\n";
		STUB();
	}

	LOG_NG << "leaving storyscreen procedure...\n";
}

void show_endscreen(display& /*disp*/, const t_string& /*text*/, unsigned int /*duration*/)
{
	STUB();
	LOG_NG << "show_endscreen() invoked...\n";

	config story_cfg;

	// FIXME: stub!

	LOG_NG << "show_endscreen() completed...\n";
}
