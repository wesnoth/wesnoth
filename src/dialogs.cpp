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

#include "dialogs.hpp"
#include "language.hpp"
#include "replay.hpp"

#include <map>
#include <string>
#include <vector>

namespace dialogs
{

void advance_unit(const game_data& info,
                  std::map<gamemap::location,unit>& units,
                  const gamemap::location& loc,
                  display& gui, bool random_choice)
{
	std::map<gamemap::location,unit>::iterator u = units.find(loc);
	if(u == units.end() || u->second.advances() == false)
		return;

	const std::vector<std::string>& options = u->second.type().advances_to();

	std::vector<unit> sample_units;
	for(std::vector<std::string>::const_iterator op = options.begin();
	    op != options.end(); ++op) {
		sample_units.push_back(::get_advanced_unit(info,units,loc,*op));
	}

	int res = 0;

	if(options.empty()) {
		return;
	} else if(random_choice) {
		res = rand()%options.size();
	} else if(options.size() > 1) {

		res = gui::show_dialog(gui,NULL,string_table["advance_unit_heading"],
		                       string_table["advance_unit_message"],
		                       gui::OK_ONLY, &options, &sample_units);
	}

	//when the unit advances, it fades to white, and then switches to the
	//new unit, then fades back to the normal colour
	double intensity;
	for(intensity = 1.0; intensity >= 0.0; intensity -= 0.05) {
		gui.set_advancing_unit(loc,intensity);
		gui.draw(false);
		gui.update_display();
		SDL_Delay(30);
	}

	recorder.choose_option(res);

	::advance_unit(info,units,loc,options[res]);

	gui.invalidate_unit();

	for(intensity = 0.0; intensity <= 1.0; intensity += 0.05) {
		gui.set_advancing_unit(loc,intensity);
		gui.draw(false);
		gui.update_display();
		SDL_Delay(30);
	}

	gui.set_advancing_unit(gamemap::location(),0.0);

	gui.invalidate_all();
	gui.draw();
}

void show_objectives(display& disp, config& level_info)
{
	static const std::string no_objectives(string_table["no_objectives"]);
	const std::string& id = level_info.values["id"];
	const std::string& lang_name = string_table[id];
	const std::string& name = lang_name.empty() ? level_info.values["name"] :
	                                              lang_name;
	const std::string& lang_objectives = string_table[id + "_objectives"];

	const std::string& objectives = lang_objectives.empty() ?
	        level_info.values["objectives"] : lang_objectives;
	gui::show_dialog(disp, NULL, "", "+" + name + "\n" +
	         (objectives.empty() ? no_objectives : objectives), gui::OK_ONLY);

}

} //end namespace dialogs
