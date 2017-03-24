/*
   Copyright (C) 2009 - 2017 by Ignacio R. Morelle <shadowm2006@gmail.com>
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
 * Storyscreen controller (wrapper interface).
 */

#include "variable.hpp"

#include "storyscreen/interface.hpp"
#include "storyscreen/controller.hpp"

#include "gui/widgets/settings.hpp"
#include "gui/dialogs/story_viewer.hpp"

#include "font/text.hpp"
#include "gettext.hpp"
#include "intro.hpp"
#include "language.hpp"
#include "log.hpp"
#include "sound.hpp"
#include "video.hpp"

static lg::log_domain log_engine("engine");
#define LOG_NG LOG_STREAM(info, log_engine)

void show_story(CVideo& video, const std::string &scenario_name,
	const config::const_child_itors &story)
{
	if(gui2::new_widgets) {
		// Combine all the [story] tags into a single config
		config cfg;
		for(const auto& iter : story) {
			cfg.append_children(iter);
		}

		storyscreen::controller controller(video, vconfig(cfg, true), scenario_name, 0);

		gui2::dialogs::story_viewer::display(controller, video);

		video2::trigger_full_redraw();

		return;
	}

	events::event_context story_context;

	int segment_count = 0;
	config::const_child_iterator itor = story.begin();
	storyscreen::START_POSITION startpos = storyscreen::START_BEGINNING;
	while (itor != story.end())
	{
		storyscreen::controller ctl(video, vconfig(*itor, true),
			scenario_name, segment_count);
		storyscreen::STORY_RESULT result = ctl.show(startpos);

		switch(result) {
		case storyscreen::NEXT:
			if(itor != story.end()) {
				++itor;
				++segment_count;
				startpos = storyscreen::START_BEGINNING;
			}
			break;
		case storyscreen::BACK:
			if(itor != story.begin()) {
				--itor;
				--segment_count;
				startpos = storyscreen::START_END;
			}
			break;
		case storyscreen::QUIT:
			video2::trigger_full_redraw();
			return;
		}
	}
	video2::trigger_full_redraw();
	return;
}
