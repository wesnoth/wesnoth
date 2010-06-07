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

	int count_segments(const config::const_child_itors &story) 
	{
		config::const_child_iterator itor = story.first;
		int count = 0;
		while(itor != story.second) {
			++itor;
			++count;
		}
		return count;
	}
} // end anonymous namespace


storyscreen::STORY_RESULT show_story(display& disp, 
				     const std::string& scenario_name,
				     const config::const_child_itors &story) {
	const int total_segments = count_segments(story);
	int segment_count = 0;
	config::const_child_iterator itor = story.first;
	storyscreen::START_POSITION startpos = storyscreen::START_BEGINNING;
	while(itor != story.second) {
		storyscreen::STORY_RESULT result = show_storyscreen(disp, vconfig(*itor, true), scenario_name, 
								    startpos, segment_count, total_segments);
		switch(result) {
		case storyscreen::NEXT:
			if(itor != story.second) {
				++itor;
				++segment_count;
				startpos = storyscreen::START_BEGINNING;
			}
			break;
		case storyscreen::BACK:
			if(itor != story.first) {
				--itor;
				--segment_count;
				startpos = storyscreen::START_END;
			}
			break;
		case storyscreen::LAST:
			itor = story.second;
			--itor;
			segment_count = total_segments - 1;
			startpos = storyscreen::START_END;
			break;
		case storyscreen::FIRST:
			itor = story.first;
			segment_count = 0;
			startpos = storyscreen::START_BEGINNING;
			break;
		case storyscreen::QUIT:
			return storyscreen::QUIT;
		default:
			assert(false);
			itor = story.second;
			break;
		}
	}
	return storyscreen::NEXT;
}

storyscreen::STORY_RESULT show_storyscreen(display& disp, const vconfig& story_cfg, 
					   const std::string& scenario_name,
					   storyscreen::START_POSITION startpos,
					   int segment_index, int total_segments)
{
	LOG_NG << "entering storyscreen procedure...\n";

	storyscreen::controller ctl(disp, story_cfg, scenario_name, segment_index, total_segments);

	storyscreen::STORY_RESULT ret = ctl.show(startpos);

	LOG_NG << "leaving storyscreen procedure...\n";

	return ret;
}

void show_endscreen(display& /*disp*/, const t_string& /*text*/, unsigned int /*duration*/)
{
	STUB();
	LOG_NG << "show_endscreen() invoked...\n";

	config story_cfg;

	// FIXME: stub!

	LOG_NG << "show_endscreen() completed...\n";
}
