/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include "language.hpp"
#include "menu.hpp"
#include "multiplayer.hpp"
#include "playlevel.hpp"
#include "replay.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

void play_multiplayer(display& disp, game_data& units_data, config& cfg,
                      game_state& state)
{
	std::vector<std::string> options;
	std::vector<config*>& levels = cfg.children["multiplayer"];
	for(std::vector<config*>::iterator i = levels.begin(); i!=levels.end();++i){
		const std::string& lang_name = string_table[(*i)->values["id"]];
		if(lang_name.empty() == false)
			options.push_back(lang_name);
		else
			options.push_back((*i)->values["name"]);
	}

	int res = gui::show_dialog(disp,NULL,"",
	                        string_table["choose_scenario"],gui::OK_CANCEL,
							&options);
	if(res == -1)
		return;

	config& level = *levels[res];
	state.label = level.values["name"];

	state.scenario = res;
	
	std::vector<config*>& sides = level.children["side"];
	std::vector<config*>& possible_sides = cfg.children["multiplayer_side"];
	if(sides.empty() || possible_sides.empty()) {
		std::cerr << "no multiplayer sides found\n";
		return;
	}

	for(std::vector<config*>::iterator sd = sides.begin();
	    sd != sides.end(); ++sd) {
		(*sd)->values["name"] = possible_sides.front()->values["name"];
		(*sd)->values["type"] = possible_sides.front()->values["type"];
		(*sd)->values["recruit"] = possible_sides.front()->values["recruit"];
	}

	res = 0;
	while(res != sides.size()) {
		std::vector<std::string> sides_list;
		for(std::vector<config*>::iterator sd = sides.begin();
		    sd != sides.end(); ++sd) {
			std::stringstream details;
			details << (*sd)->values["side"] << ","
					<< (*sd)->values["name"] << ","
					<< ((*sd)->values["controller"] == "human" ?
			            string_table["human_controlled"] :
						string_table["ai_controlled"]);
			sides_list.push_back(details.str());
		}

		sides_list.push_back(string_table["start_game"]);
		
		res = gui::show_dialog(disp,NULL,"",string_table["configure_sides"],
		                       gui::MESSAGE,&sides_list);

		if(res >= 0 && res < sides.size()) {
			std::vector<std::string> choices;
			
			for(int n = 0; n != 2; ++n) {
				for(std::vector<config*>::iterator i = possible_sides.begin();
				    i != possible_sides.end(); ++i) {
					std::stringstream choice;
					choice << (*i)->values["name"] << " - "
					       << (n == 0 ? string_table["human_controlled"] :
					                    string_table["ai_controlled"]);
					choices.push_back(choice.str());
				}
			}

			int result = gui::show_dialog(disp,NULL,"",
			                               string_table["choose_side"],
										   gui::MESSAGE,&choices);
			if(result >= 0) {
				sides[res]->values["controller"] = (result >= choices.size()/2)
				                                    ?  "ai" : "human";
				if(result >= choices.size()/2)
					result -= choices.size()/2;

				assert(result < possible_sides.size());

				std::map<std::string,std::string>& values =
				                                possible_sides[result]->values;
				sides[res]->values["name"] = values["name"];
				sides[res]->values["type"] = values["type"];
				sides[res]->values["recruit"] = values["recruit"];
			}
		}
	}
	
	state.starting_pos = level;

	recorder.set_save_info(state);

	std::vector<config*> story;
	play_level(units_data,cfg,&level,disp.video(),state,story);
	recorder.clear();
}
