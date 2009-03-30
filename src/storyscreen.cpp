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

// This code is work in progress, and shouldn't be enabled for production
// builds. It is supposed to completely replace the old story screens code
// at intro.cpp, introducing new WML conventions while at it.
//
// Do not remove the #ifdef below.
#ifdef SHADOWM_STORYSCREEN

#include "global.hpp"
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
#include "boost/current_function.hpp"
#define STUB() \
	std::cerr << "OUCH: entered stub " << BOOST_CURRENT_FUNCTION << " [at " << __FILE__ << ":" << __LINE__ << "]\n"

class storyscreen
{
public:
	storyscreen(display& disp, const vconfig& data, const config& scenario_cfg);

private:
	display& disp_;
	vconfig data_;
	config scenario_;
};

namespace {
	void write_generic_endscreen_config(config& append_to_cfg)
	{
		config& partcfg = append_to_cfg.add_child("story").add_child("page");
		partcfg["text_align"] = "centered";
	}
} // end anonymous namespace

void show_storyscreen(display& disp, const vconfig& data, const config& scenario_cfg)
{
	STUB();
	LOG_NG << "entering storyscreen procedure...\n";

	// FIXME: stub!

	LOG_NG << "leaving storyscreen procedure...\n";
}

void show_endscreen(display& disp, const t_string& text, unsigned int duration)
{
	STUB();
	LOG_NG << "show_endscreen() invoked...\n";

	// FIXME: stub!

	LOG_NG << "show_endscreen() completed...\n";
}

// Trivial drop-in compatibility with intro.cpp
void show_intro(display &disp, const vconfig& data, const config& level)
{
	show_storyscreen(disp,data,level);
}

void the_end(display &disp, std::string text, unsigned int duration)
{
	show_endscreen(disp, t_string(text) /* dumb! */, duration);
}

#endif /* SHADOWM_STORYSCREEN */
