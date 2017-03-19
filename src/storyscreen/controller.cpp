/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
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
 * Storyscreen controller (implementation).
 */

#include "storyscreen/controller.hpp"
#include "storyscreen/part.hpp"
#include "storyscreen/render.hpp"

#include <cassert>
#include "variable.hpp"

#include "game_events/conditional_wml.hpp"
#include "game_events/manager.hpp"
#include "game_events/pump.hpp"
#include "game_data.hpp"
#include "gettext.hpp"
#include "intro.hpp"
#include "log.hpp"
#include "resources.hpp"
#include "widgets/button.hpp"

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)

namespace storyscreen {

controller::controller(CVideo& video, const vconfig& data, const std::string& scenario_name,
		       int segment_index)
	: video_(video)
	, evt_context_()
	, scenario_name_(scenario_name)
	, segment_index_(segment_index)
	, parts_()
{
	assert(resources::gamedata != nullptr && "Ouch: gamedata is nullptr when initializing storyscreen controller");
	resolve_wml(data);
}

void controller::resolve_wml(const vconfig& cfg)
{
	for(vconfig::all_children_iterator i = cfg.ordered_begin(); i != cfg.ordered_end(); ++i)
	{
		// i->first and i->second are goddamn temporaries; do not make references
		const std::string key = i->first;
		const vconfig node = i->second;

		if(key == "part" && !node.empty()) {
			part_pointer_type const story_part(new part(node));
			// Use scenario name as part title if the WML doesn't supply a custom one.
			if((*story_part).show_title() && (*story_part).title().empty()) {
				(*story_part).set_title( scenario_name_ );
			}
			parts_.push_back(story_part);
		}
		// [if]
		else if(key == "if") {
			// check if the [if] tag has a [then] child;
			// if we try to execute a non-existing [then], we get a segfault
			if (game_events::conditional_passed(node)) {
				if (node.has_child("then")) {
					resolve_wml(node.child("then"));
				}
			}
			// condition not passed, check [elseif] and [else]
			else {
				// get all [elseif] children and set a flag
				vconfig::child_list elseif_children = node.get_children("elseif");
				bool elseif_flag = false;
				// for each [elseif]: test if it has a [then] child
				// if the condition matches, execute [then] and raise flag
				for (vconfig::child_list::const_iterator elseif = elseif_children.begin(); elseif != elseif_children.end(); ++elseif) {
					if (game_events::conditional_passed(*elseif)) {
						if (elseif->has_child("then")) {
							resolve_wml(elseif->child("then"));
						}
						elseif_flag = true;
						break;
					}
				}
				// if we have an [else] tag and no [elseif] was successful (flag not raised), execute it
				if (node.has_child("else") && !elseif_flag) {
					resolve_wml(node.child("else"));
				}
			}
		}
		// [switch]
		else if(key == "switch") {
			const std::string var_name = node["variable"];
			const std::string var_actual_value = resources::gamedata->get_variable_const(var_name);
			bool case_not_found = true;

			for(vconfig::all_children_iterator j = node.ordered_begin(); j != node.ordered_end(); ++j) {
				if(j->first != "case") continue;

				// Enter all matching cases.
				const std::string var_expected_value = (j->second)["value"];
			    if(var_actual_value == var_expected_value) {
					case_not_found = false;
					resolve_wml(j->second);
			    }
			}

			if(case_not_found) {
				for(vconfig::all_children_iterator j = node.ordered_begin(); j != node.ordered_end(); ++j) {
					if(j->first != "else") continue;

					// Enter all elses.
					resolve_wml(j->second);
				}
			}
		}
		// [deprecated_message]
		else if(key == "deprecated_message") {
			// Won't appear until the scenario start event finishes.
			lg::wml_error() << node["message"] << '\n';
		}
		// [wml_message]
		else if(key == "wml_message") {
			// As with [deprecated_message],
			// it won't appear until the scenario start event is complete.
			resources::game_events->pump().put_wml_message(node["logger"], node["message"], node["in_chat"].to_bool(false));
		}
	}
}

STORY_RESULT controller::show(START_POSITION startpos)
{

	if(parts_.empty()) {
		LOG_NG << "no storyscreen parts to show\n";
		return NEXT;
	}

	gui::button back_button (video_, "", gui::button::TYPE_PRESS, "button_normal/button_small_H22"
		, gui::button::DEFAULT_SPACE, true, "icons/arrows/long_arrow_ornate_left");
	gui::button next_button (video_, "", gui::button::TYPE_PRESS, "button_normal/button_small_H22"
		, gui::button::DEFAULT_SPACE, true, "icons/arrows/long_arrow_ornate_right");
	gui::button play_button (video_, _("Skip"));

	// Build renderer cache unless built for a low-memory environment;
	// caching the scaled backgrounds can take over a decent amount of memory.
	std::vector< render_pointer_type > uis_;
	for(part_pointer_type p : parts_) {
		assert(p != nullptr && "Ouch: hit nullptr storyscreen part in collection");
		render_pointer_type const rpt(new part_ui(*p, video_, next_button, back_button, play_button));
		uis_.push_back(rpt);
	}

	size_t k = 0;
	switch(startpos) {
	case START_BEGINNING:
		break;
	case START_END:
		k = parts_.size() -1;
		break;
	default:
		assert(false);
		break;
	}

	while(k < parts_.size()) {
		part_ui &render_interface = *uis_[k];

		LOG_NG << "displaying storyscreen part " << k+1 << " of " << parts_.size() << '\n';

		back_button.enable(segment_index_ != 0 || k != 0);

		switch(render_interface.show()) {
		case part_ui::NEXT:
			++k;
			break;
		case part_ui::BACK:
			if(k > 0) {
				--k;
			}
			else if(segment_index_ > 0) {
				return BACK;
			}
			break;
		case part_ui::QUIT:
			return QUIT;
		}
	}
	return NEXT;
}

} // end namespace storyscreen
