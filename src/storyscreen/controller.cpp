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
 * Storyscreen controller (implementation).
 */

#include "global.hpp"
#include "storyscreen/controller.hpp"
#include "storyscreen/page.hpp"
#include "storyscreen/render.hpp"

#include "asserts.hpp"
#include "foreach.hpp"
#include "variable.hpp"

#include "display.hpp"
#include "game_events.hpp"
#include "gamestatus.hpp"
#include "gettext.hpp"
#include "intro.hpp"
#include "log.hpp"
#include "widgets/button.hpp"

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)

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

void controller::resolve_wml(const vconfig& cfg)
{
	for(vconfig::all_children_iterator i = cfg.ordered_begin(); i != cfg.ordered_end(); i++)
	{
		// i->first and i->second are goddamn temporaries; do not make references
		const std::string key = i->first;
		const vconfig node = i->second;

		// [page] and deprecated [part] (remove in 1.7.4)
		if((key == "page" || key == "part") && !node.empty()) {
			if(key == "part") {
				lg::wml_error << "[part] in [story] has been deprecated and support for it will be removed in 1.7.4; use [page] instead.\n";
			}

			page* const story_page = new page(*gamestate_, node);
			// Use scenario name as page title if the WML doesn't supply a custom one.
			if((*story_page).show_title() && (*story_page).title().empty()) {
				(*story_page).set_title( scenario_name_ );
			}
			pages_.push_back(story_page);
		}
		// [if]
		else if(key == "if") {
			const std::string branch_label =
				game_events::conditional_passed(NULL, node) ?
				"then" : "else";
			const vconfig branch = node.child(branch_label);
			resolve_wml(branch);
		}
		// [switch]
		else if(key == "switch") {
			const std::string var_name = node["variable"];
			const std::string var_actual_value = (*gamestate_).get_variable_const(var_name);
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
			game_events::handle_deprecated_message(node.get_parsed_config());
		}
		// [wml_message]
		else if(key == "wml_message") {
			// Pass to game events handler. As with [deprecated_message],
			// it won't appear until the scenario start event is complete.
			game_events::handle_wml_log_message(node.get_parsed_config());
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

void controller::show_all_pages()
{
	if(pages_.empty()) {
		LOG_NG << "no storyscreen pages to show\n";
	}

	size_t page_n = 0, pages_c = pages_.size();
	while((page_n = show_page(page_n)) < pages_c)
		;
}

size_t controller::show_page(size_t page_num)
{
	if(page_num >= pages_.size()) {
		ERR_NG << "attempted to display inexistant storyscreen page: " << page_num+1 << " (of " << pages_.size() << ")\n";
		return pages_.size();
	}

	LOG_NG << "displaying storyscreen page " << page_num+1 << " of " << pages_.size() << '\n';

	page* const p = pages_[page_num];
	ASSERT_LOG( p != NULL, "Ouch: hit NULL storyscreen page in collection" );

	// TODO:
	//  gui::button back_button(disp_.video(),std::string("< ")+_("Next"));
	gui::button next_button(disp_.video(),_("Next") + std::string(" >"));
	gui::button skip_button(disp_.video(),_("Skip"));

	page_ui ui(*p, disp_, next_button, skip_button);
	switch(ui.show()) {
	case page_ui::NEXT:
		return page_num+1;
	case page_ui::BACK:
		return(page_num > 0 ? page_num-1 : page_num);
	case page_ui::SKIP:
		return pages_.size();
	default:
		throw quit();
	}

	return 0;
}

} // end namespace storyscreen
