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
#include "show_dialog.hpp"
#include "util.hpp"

#include <map>
#include <sstream>
#include <string>
#include <time.h>
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

	std::vector<std::string> lang_options;

	std::vector<unit> sample_units;
	for(std::vector<std::string>::const_iterator op = options.begin();
	    op != options.end(); ++op) {
		sample_units.push_back(::get_advanced_unit(info,units,loc,*op));
		lang_options.push_back(sample_units.back().type().language_name());
	}

	int res = 0;

	if(options.empty()) {
		return;
	} else if(random_choice) {
		res = rand()%options.size();
	} else if(options.size() > 1) {

		res = gui::show_dialog(gui,NULL,string_table["advance_unit_heading"],
		                       string_table["advance_unit_message"],
		                       gui::OK_ONLY, &lang_options, &sample_units);
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
	const std::string& id = level_info["id"];
	const std::string& lang_name = string_table[id];
	const std::string& name = lang_name.empty() ? level_info["name"] :
	                                              lang_name;
	const std::string& lang_objectives = string_table[id + "_objectives"];

	const std::string& objectives = lang_objectives.empty() ?
	        level_info["objectives"] : lang_objectives;
	gui::show_dialog(disp, NULL, "", "+" + name + "\n" +
	         (objectives.empty() ? no_objectives : objectives), gui::OK_ONLY);

}

int get_save_name(display & disp,const std::string& caption, const std::string& message,
				  std::string* name, gui::DIALOG_TYPE dialog_type)
{
    int overwrite=0;
    int res=0;
    do {
        res = gui::show_dialog(disp,NULL,"",caption,dialog_type,NULL,NULL,message,name);
            if (res == 0 && save_game_exists(*name))
                overwrite = gui::show_dialog(disp,NULL,"",
                    string_table["save_confirm_overwrite"],gui::YES_NO);
        else overwrite = 0;
    } while ((res==0)&&(overwrite!=0));
	return res;
}

std::string load_game_dialog(display& disp, bool* show_replay)
{
	const std::vector<save_info>& games = get_saves_list();

	if(games.empty()) {
		gui::show_dialog(disp,NULL,
		                 string_table["no_saves_heading"],
						 string_table["no_saves_message"],
		                 gui::OK_ONLY);
		return "";
	}

	std::vector<std::string> items;
	for(std::vector<save_info>::const_iterator i = games.begin(); i != games.end(); ++i) {
		std::string name = i->name;
		name.resize(minimum<size_t>(name.size(),40));

		char time_buf[256];
		const size_t res = strftime(time_buf,sizeof(time_buf),string_table["date_format"].c_str(),localtime(&(i->time_modified)));
		if(res == 0)
			time_buf[0] = 0;

		std::stringstream str;
		str << name << "," << time_buf;
		items.push_back(str.str());
	}

	//create an option for whether the replay should be shown or not
	std::vector<gui::check_item> options;

	if(show_replay != NULL)
		options.push_back(gui::check_item(string_table["show_replay"],false));

	const int res = gui::show_dialog(disp,NULL,
					 string_table["load_game_heading"],
					 string_table["load_game_message"],
			         gui::OK_CANCEL,&items,NULL,"",NULL,NULL,&options);

	if(res == -1)
		return "";

	if(show_replay != NULL)
		*show_replay = options.front().checked;

	return games[res].name;
}

void unit_speak(const config& message_info, display& disp, const unit_map& units)
{
	for(unit_map::const_iterator i = units.begin(); i != units.end(); ++i) {
		if(i->second.matches_filter(message_info)) {
			disp.scroll_to_tile(i->first.x,i->first.y);
			const scoped_sdl_surface surface(image::get_image(i->second.type().image_profile(),image::UNSCALED));
			gui::show_dialog(disp,surface,i->second.underlying_description(),message_info["message"],gui::MESSAGE);
		}
	}
}

} //end namespace dialogs
