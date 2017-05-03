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

#include "storyscreen/parser.hpp"

#include "game_data.hpp"
#include "game_events/conditional_wml.hpp"
#include "game_events/manager.hpp"
#include "game_events/pump.hpp"
#include "log.hpp"
#include "resources.hpp"
#include "variable.hpp"

namespace storyscreen
{

void story_parser::resolve_wml(const vconfig& cfg)
{
	// Execution flow/branching/[image]
	for(vconfig::all_children_iterator i = cfg.ordered_begin(); i != cfg.ordered_end(); ++i) {
		// i->first and i->second are goddamn temporaries; do not make references
		const std::string key = i->first;
		const vconfig node = i->second;

		// Execute any special actions derived classes provide.
		resolve_wml_helper(key, node);

		// [if]
		if(key == "if") {
			// check if the [if] tag has a [then] child;
			// if we try to execute a non-existing [then], we get a segfault
			if(game_events::conditional_passed(node)) {
				if(node.has_child("then")) {
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
				for(const auto& elseif : elseif_children) {
					if(game_events::conditional_passed(elseif)) {
						if(elseif.has_child("then")) {
							resolve_wml(elseif.child("then"));
						}

						elseif_flag = true;
						break;
					}
				}

				// if we have an [else] tag and no [elseif] was successful (flag not raised), execute it
				if(node.has_child("else") && !elseif_flag) {
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
				if(j->first != "case") {
					continue;
				}

				// Enter all matching cases.
				const std::string var_expected_value = (j->second)["value"];
				if(var_actual_value == var_expected_value) {
					case_not_found = false;
					resolve_wml(j->second);
				}
			}

			if(case_not_found) {
				for(vconfig::all_children_iterator j = node.ordered_begin(); j != node.ordered_end(); ++j) {
					if(j->first != "else") {
						continue;
					}

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
			resources::game_events->pump().put_wml_message(
				node["logger"], node["message"], node["in_chat"].to_bool(false));
		}
	}
}

} // namespace storyscreen
