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
#include "events.hpp"
#include "font.hpp"
#include "language.hpp"
#include "preferences.hpp"
#include "replay.hpp"
#include "show_dialog.hpp"
#include "util.hpp"
#include "widgets/file_chooser.hpp"

#include <cstdio>
#include <map>
#include <sstream>
#include <string>
#include <time.h>
#include <vector>

namespace dialogs
{

void advance_unit(const game_data& info,
                  std::map<gamemap::location,unit>& units,
                  gamemap::location loc,
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
		const unit_type& type = sample_units.back().type();
		lang_options.push_back("&" + type.image() + "," + type.language_name());
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

	recorder.choose_option(res);

	animate_unit_advancement(info,units,loc,gui,size_t(res));
}

bool animate_unit_advancement(const game_data& info,unit_map& units, gamemap::location loc, display& gui, size_t choice)
{
	std::map<gamemap::location,unit>::iterator u = units.find(loc);
	if(u == units.end() || u->second.advances() == false) {
		return false;
	}

	const std::vector<std::string>& options = u->second.type().advances_to();
	if(choice >= options.size()) {
		return false;
	}
	
	//when the unit advances, it fades to white, and then switches to the
	//new unit, then fades back to the normal colour
	
	if(!gui.update_locked()) {
		for(double intensity = 1.0; intensity >= 0.0; intensity -= 0.05) {
			gui.set_advancing_unit(loc,intensity);
			events::pump();
			gui.draw(false);
			gui.update_display();
			SDL_Delay(30);
		}
	}

	::advance_unit(info,units,loc,options[choice]);

	gui.invalidate_unit();

	if(!gui.update_locked()) {
		for(double intensity = 0.0; intensity <= 1.0; intensity += 0.05) {
			gui.set_advancing_unit(loc,intensity);
			events::pump();
			gui.draw(false);
			gui.update_display();
			SDL_Delay(30);
		}
	}

	gui.set_advancing_unit(gamemap::location(),0.0);

	gui.invalidate_all();
	gui.draw();

	return true;
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
	gui::show_dialog(disp, NULL, "", font::LARGE_TEXT + name + "\n" +
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

//a class to handle deleting a saved game
namespace {

class delete_save : public gui::dialog_button_action
{
public:
	delete_save(display& disp, std::vector<save_info>& saves) : disp_(disp), saves_(saves) {}
private:
	gui::dialog_button_action::RESULT button_pressed(int menu_selection);

	display& disp_;
	std::vector<save_info>& saves_;
};

gui::dialog_button_action::RESULT delete_save::button_pressed(int menu_selection)
{
	const size_t index = size_t(menu_selection);
	if(index < saves_.size()) {

		//see if we should ask the user for deletion confirmation
		if(preferences::ask_delete_saves()) {
			std::vector<gui::check_item> options;
			options.push_back(gui::check_item(string_table["dont_ask_again"],false));

			const int res = gui::show_dialog(disp_,NULL,"",string_table["really_delete_save"],gui::YES_NO,
			                                 NULL,NULL,"",NULL,NULL,&options);

			//see if the user doesn't want to be asked this again
			assert(options.empty() == false);
			if(options.front().checked) {
				preferences::set_ask_delete_saves(false);
			}

			if(res != 0) {
				return gui::dialog_button_action::NO_EFFECT;
			}
		}

		//delete the file
		delete_game(saves_[index].name);

		//remove it from the list of saves
		saves_.erase(saves_.begin() + index);
		return gui::dialog_button_action::DELETE_ITEM;
	} else {
		return gui::dialog_button_action::NO_EFFECT;
	}
}

} //end anon namespace

std::string load_game_dialog(display& disp, bool* show_replay)
{
	std::vector<save_info> games = get_saves_list();

	delete_save save_deleter(disp,games);
	gui::dialog_button delete_button(&save_deleter,string_table["delete_save"]);
	std::vector<gui::dialog_button> buttons;
	buttons.push_back(delete_button);

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
		// escape all special characters in filenames
		str << config::escape(name) << "," << time_buf;
		items.push_back(str.str());
	}

	//create an option for whether the replay should be shown or not
	std::vector<gui::check_item> options;

	if(show_replay != NULL)
		options.push_back(gui::check_item(string_table["show_replay"],false));

	const int res = gui::show_dialog(disp,NULL,
					 string_table["load_game_heading"],
					 string_table["load_game_message"],
			         gui::OK_CANCEL,&items,NULL,"",NULL,NULL,&options,-1,-1,NULL,&buttons);

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


int show_file_chooser_dialog(display &disp, std::string &filename,
							 const std::string title,
							 int xloc, int yloc) {
	const events::event_context dialog_events_context;
	const gui::dialog_manager manager;
	const events::resize_lock prevent_resizing;

	CVideo& screen = disp.video();
	SDL_Surface* const scr = screen.getSurface();

	const int width = 400;
	const int height = 400;
	const int left_padding = 10;
	const int right_padding = 10;
	const int top_padding = 10;
	const int bot_padding = 10;

	// If not both locations were supplied, put the dialog in the middle
	// of the screen.
	if (yloc <= -1 || xloc <= -1) {
		xloc = scr->w / 2 - width / 2;
		yloc = scr->h / 2 - height / 2;
	}
	std::vector<gui::button*> buttons_ptr;
	gui::button ok_button_(disp, string_table["ok_button"]);
	gui::button cancel_button_(disp, string_table["cancel_button"]);
	buttons_ptr.push_back(&ok_button_);
	buttons_ptr.push_back(&cancel_button_);
	surface_restorer restorer;
	gui::draw_dialog(xloc, yloc, width, height, disp, title, NULL, &buttons_ptr, &restorer);

	gui::file_chooser fc(disp, filename);
	fc.set_location(xloc + left_padding, yloc + top_padding);
	fc.set_width(width - left_padding - right_padding);
	fc.set_height(height - top_padding - bot_padding);
	fc.set_dirty(true);
	
	events::raise_draw_event();
	screen.flip();
	disp.invalidate_all();

	CKey key;
	for (;;) {
		events::pump();
		events::raise_process_event();
		events::raise_draw_event();
		if (fc.choice_made()) {
			filename = fc.get_choice();
			return 0; // We know that the OK button is on index 0.
		}
		if (key[SDLK_ESCAPE]) {
			// Escape quits from the dialog.
			return -1;
		}
		for (std::vector<gui::button*>::iterator button_it = buttons_ptr.begin();
			 button_it != buttons_ptr.end(); button_it++) {
			if ((*button_it)->pressed()) {
				// Return the index of the pressed button.
				filename = fc.get_choice();
				return button_it - buttons_ptr.begin();
			}
		}
		screen.flip();
		SDL_Delay(10);
	}
}


} //end namespace dialogs
