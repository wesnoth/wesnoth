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

#include "global.hpp"

#include "dialogs.hpp"
#include "events.hpp"
#include "filesystem.hpp"
#include "font.hpp"
#include "game_config.hpp"
#include "gettext.hpp"
#include "help.hpp"
#include "language.hpp"
#include "log.hpp"
#include "map.hpp"
#include "playturn.hpp"
#include "preferences.hpp"
#include "replay.hpp"
#include "show_dialog.hpp"
#include "util.hpp"
#include "video.hpp"
#include "wassert.hpp"
#include "wml_separators.hpp"
#include "serialization/string_utils.hpp"
#include "widgets/menu.hpp"
#include "widgets/progressbar.hpp"

#include <cstdio>
#include <map>
#include <sstream>
#include <string>
#include <time.h>
#include <vector>

#define LOG_DP lg::info(lg::display)
#define ERR_G  lg::err(lg::general)

namespace dialogs
{

void advance_unit(const game_data& info,
				  const gamemap& map,
                  std::map<gamemap::location,unit>& units,
                  gamemap::location loc,
                  display& gui, bool random_choice)
{
	std::map<gamemap::location,unit>::iterator u = units.find(loc);
	if(u == units.end() || u->second.advances() == false)
		return;

	LOG_DP << "advance_unit: " << u->second.type().name() << "\n";

	const std::vector<std::string>& options = u->second.type().advances_to();

	std::vector<std::string> lang_options;

	std::vector<unit> sample_units;
	for(std::vector<std::string>::const_iterator op = options.begin(); op != options.end(); ++op) {
		sample_units.push_back(::get_advanced_unit(info,units,loc,*op));
		const unit_type& type = sample_units.back().type();
		lang_options.push_back(IMAGE_PREFIX + type.image() + COLUMN_SEPARATOR + type.language_name());
	}

	const config::child_list& mod_options = u->second.get_modification_advances();

	for(config::child_list::const_iterator mod = mod_options.begin(); mod != mod_options.end(); ++mod) {
		sample_units.push_back(::get_advanced_unit(info,units,loc,u->second.type().name()));
		sample_units.back().add_modification("advance",**mod);
		const unit_type& type = sample_units.back().type();
		lang_options.push_back(IMAGE_PREFIX + type.image() + COLUMN_SEPARATOR + (**mod)["description"]);
	}

	LOG_DP << "options: " << options.size() << "\n";

	int res = 0;

	if(lang_options.empty()) {
		return;
	} else if(random_choice) {
		res = rand()%lang_options.size();
	} else if(lang_options.size() > 1) {

		const events::event_context dialog_events_context;
		unit_preview_pane unit_preview(gui,&map,sample_units);
		std::vector<gui::preview_pane*> preview_panes;
		preview_panes.push_back(&unit_preview);

		res = gui::show_dialog(gui,NULL,_("Advance Unit"),
		                       _("What should our victorious unit become?"),
		                       gui::OK_ONLY, &lang_options, &preview_panes);
	}

	recorder.choose_option(res);

	LOG_DP << "animating advancement...\n";
	animate_unit_advancement(info,units,loc,gui,size_t(res));
}

bool animate_unit_advancement(const game_data& info,unit_map& units, gamemap::location loc, display& gui, size_t choice)
{
	const command_disabler cmd_disabler;
	
	std::map<gamemap::location,unit>::iterator u = units.find(loc);
	if(u == units.end() || u->second.advances() == false) {
		return false;
	}

	const std::vector<std::string>& options = u->second.type().advances_to();
	const config::child_list& mod_options = u->second.get_modification_advances();

	if(choice >= options.size() + mod_options.size()) {
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

	const std::string& chosen_unit = choice < options.size() ? options[choice] : u->second.type().name();
	::advance_unit(info,units,loc,chosen_unit);

	u = units.find(loc);
	if(u != units.end() && choice >= options.size()) {
		u->second.add_modification("advance",*mod_options[choice - options.size()]);
	}

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
	static const std::string no_objectives(_("No objectives available"));
	const std::string& name = level_info["name"];
	const std::string& lang_objectives = level_info["objectives"];

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
        res = gui::show_dialog(disp,NULL,_("Save Game"),caption,dialog_type,NULL,NULL,message,name);
            if (res == 0 && save_game_exists(*name))
                overwrite = gui::show_dialog(disp,NULL,_("Overwrite?"),
                    _("Save already exists. Do you want to overwrite it ?"),gui::YES_NO);
        else overwrite = 0;
    } while ((res==0)&&(overwrite!=0));
	return res;
}

//a class to handle deleting a saved game
namespace {

class delete_save : public gui::dialog_button_action
{
public:
	delete_save(display& disp, std::vector<save_info>& saves, std::vector<config*>& save_summaries) : disp_(disp), saves_(saves), summaries_(save_summaries) {}
private:
	gui::dialog_button_action::RESULT button_pressed(int menu_selection);

	display& disp_;
	std::vector<save_info>& saves_;
	std::vector<config*>& summaries_;
};

gui::dialog_button_action::RESULT delete_save::button_pressed(int menu_selection)
{
	const size_t index = size_t(menu_selection);
	if(index < saves_.size()) {

		//see if we should ask the user for deletion confirmation
		if(preferences::ask_delete_saves()) {
			std::vector<gui::check_item> options;
			options.push_back(gui::check_item(_("Don't ask me again!"),false));

			const int res = gui::show_dialog(disp_,NULL,"",_("Do you really want to delete this game?"),gui::YES_NO,
			                                 NULL,NULL,"",NULL,-1,NULL,&options);

			//see if the user doesn't want to be asked this again
			wassert(options.empty() == false);
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

		if(index < summaries_.size()) {
			summaries_.erase(summaries_.begin() + index);
		}

		return gui::dialog_button_action::DELETE_ITEM;
	} else {
		return gui::dialog_button_action::NO_EFFECT;
	}
}

static const int save_preview_border = 10;

class save_preview_pane : public gui::preview_pane
{
public:
	save_preview_pane(display& disp, const config& game_config, gamemap* map, const game_data& data,
	                  const std::vector<save_info>& info, const std::vector<config*>& summaries)
		: gui::preview_pane(disp), game_config_(&game_config), map_(map), data_(&data), info_(&info), summaries_(&summaries), index_(0)
	{
		set_measurements(200, 400);
	}

	void draw_contents();
	void set_selection(int index) {
		index_ = index;
		set_dirty();
	}

	bool left_side() const { return true; }

private:
	const config* game_config_;
	gamemap* map_;
	const game_data* data_;
	const std::vector<save_info>* info_;
	const std::vector<config*>* summaries_;
	int index_;
	std::map<std::string,surface> map_cache_;
};

void save_preview_pane::draw_contents()
{
	if(index_ < 0 || index_ >= int(summaries_->size()) || info_->size() != summaries_->size()) {
		return;
	}

	const config& summary = *(*summaries_)[index_];

	surface const screen = disp().video().getSurface();

	SDL_Rect const &loc = location();
	const SDL_Rect area = { loc.x + save_preview_border, loc.y + save_preview_border,
	                        loc.w - save_preview_border * 2, loc.h - save_preview_border * 2 };
	SDL_Rect clip_area = area;
	const clip_rect_setter clipper(screen,clip_area);

	int ypos = area.y;

	const game_data::unit_type_map::const_iterator leader = data_->unit_types.find(summary["leader"]);
	if(leader != data_->unit_types.end()) {
		const surface image(image::get_image(leader->second.image(),image::UNSCALED));
		if(image != NULL) {
			SDL_Rect image_rect = {area.x,area.y,image->w,image->h};
			ypos += image_rect.h + save_preview_border;

			SDL_BlitSurface(image,NULL,screen,&image_rect);
		}
	}


	std::string map_data = summary["map_data"];
	if(map_data.empty()) {
		const config* const scenario = game_config_->find_child(summary["campaign_type"],"id",summary["scenario"]);
		if(scenario != NULL && scenario->find_child("side","shroud","yes") == NULL) {
			map_data = (*scenario)["map_data"];
			if(map_data.empty() && (*scenario)["map"].empty() == false) {
				try {
					map_data = read_map((*scenario)["map"]);
				} catch(io_exception& e) {
					ERR_G << "could not read map '" << (*scenario)["map"] << "': " << e.what() << "\n";
				}
			}
		}
	}

	surface map_surf(NULL);

	if(map_data.empty() == false) {
		const std::map<std::string,surface>::const_iterator itor = map_cache_.find(map_data);
		if(itor != map_cache_.end()) {
			map_surf = itor->second;
		} else if(map_ != NULL) {
			try {
				map_->read(map_data);

				map_surf = image::getMinimap(100, 100, *map_);
				if(map_surf != NULL) {
					map_cache_.insert(std::pair<std::string,surface>(map_data,surface(map_surf)));
				}
			} catch(gamemap::incorrect_format_exception&) {
			}
		}
	}

	if(map_surf != NULL) {
		SDL_Rect map_rect = {area.x + area.w - map_surf->w,area.y,map_surf->w,map_surf->h};
		ypos = maximum<int>(ypos,map_rect.y + map_rect.h + save_preview_border);
		SDL_BlitSurface(map_surf,NULL,screen,&map_rect);
	}

	char time_buf[256];
	const size_t res = strftime(time_buf,sizeof(time_buf),_("%a %b %d %H:%M %Y"),localtime(&((*info_)[index_].time_modified)));
	if(res == 0) {
		time_buf[0] = 0;
	}

	std::stringstream str;

	// escape all special characters in filenames
	std::string name = (*info_)[index_].name;
	str << font::BOLD_TEXT << utils::escape(name) << "\n" << time_buf;

	const std::string& campaign_type = summary["campaign_type"];
	if(summary["corrupt"] == "yes") {
		str << "\n" << _("#(Invalid)");
	} else if (!campaign_type.empty()) {
		str << "\n";
			
		if(campaign_type == "scenario") {
			str << _("Campaign");
		} else if(campaign_type == "multiplayer") {
			str << _("Multiplayer");
		} else if(campaign_type == "tutorial") {
			str << _("Tutorial");
		} else {
			str << campaign_type;
		}

		str << "\n";
			
		if(summary["snapshot"] == "no" && summary["replay"] == "yes") {
			str << _("replay");
		} else if (!summary["turn"].empty()) {
			str << _("Turn") << " " << summary["turn"];
		} else {
			str << _("Scenario Start");
		}

		str << "\n" << _("Difficulty: ") << string_table[summary["difficulty"]];
		if(!summary["version"].empty()) {
			str << "\n" << _("Version: ") << summary["version"];
		}
	}

	font::draw_text(&disp(), area, font::SIZE_SMALL, font::NORMAL_COLOUR, str.str(), area.x, ypos, true);
}

} //end anon namespace

std::string load_game_dialog(display& disp, const config& game_config, const game_data& data, bool* show_replay)
{
	std::vector<save_info> games = get_saves_list();

	if(games.empty()) {
		gui::show_dialog(disp,NULL,
		                 _("No Saved Games"),
						 _("There are no saved games to load.\n\
(Games are saved automatically when you complete a scenario)"),
		                 gui::OK_ONLY);
		return "";
	}

	std::vector<size_t> no_summary; //all items that aren't in the summary table
	std::vector<config*> summaries;
	std::vector<save_info>::const_iterator i;
	for(i = games.begin(); i != games.end(); ++i) {
		config& cfg = save_summary(i->name);
		if(cfg["campaign_type"].empty() && cfg["corrupt"] != "yes" || lexical_cast<int>(cfg["mod_time"]) != static_cast<int>(i->time_modified)) {
			no_summary.push_back(i - games.begin());
		}

		summaries.push_back(&cfg);
	}

	delete_save save_deleter(disp,games,summaries);
	gui::dialog_button delete_button(&save_deleter,_("Delete Save"));
	std::vector<gui::dialog_button> buttons;
	buttons.push_back(delete_button);

	bool generate_summaries = !no_summary.empty();

	//if there are more than 5 saves without a summary, it may take a substantial
	//amount of time to convert them over. Ask the user if they want to do this
	if(no_summary.size() > 5) {

		if(preferences::cache_saves() == preferences::CACHE_SAVES_NEVER) {
			generate_summaries = false;
		} else if(preferences::cache_saves() == preferences::CACHE_SAVES_ALWAYS) {
			generate_summaries = true;
		} else {
			char* caption = N_("Import Saved Games");
			char* message = N_("Your saves directory contains some files from an old version of Battle for Wesnoth. Would you like to update these to the latest version? This may take some time.");

			//if there are already some cached games, then we assume that the user is importing new
			//games, and it's not a total import, so tailor the message accordingly
			if(no_summary.size() < games.size()) {
				message = N_("Your saves directory contains some files that don't appear to have been generated by this version of Battle for Wesnoth. Would you like to register these files with the game?");
			}

			std::vector<gui::check_item> options;
			options.push_back(gui::check_item(_("Don't ask me again!"),false));
			const int res = gui::show_dialog(disp,NULL,_(caption),_(message),
			                                 gui::YES_NO,NULL,NULL,"",NULL,-1,NULL,&options);

			generate_summaries = res == 0;
			if(options.front().checked) {
				preferences::set_cache_saves(generate_summaries ? preferences::CACHE_SAVES_ALWAYS : preferences::CACHE_SAVES_NEVER);
			}
		}
	}

	const events::event_context context;

	if(generate_summaries) {
		gui::progress_bar bar(disp);
		const SDL_Rect bar_area = {disp.x()/2 - 100, disp.y()/2 - 20, 200, 40};
		bar.set_location(bar_area);

		for(std::vector<size_t>::const_iterator s = no_summary.begin(); s != no_summary.end(); ++s) {
			bar.set_progress_percent(((s - no_summary.begin())*100)/no_summary.size());
			events::raise_draw_event();
			events::pump();
			disp.update_display();

			log_scope("load");
			game_state state;

			config& summary = save_summary(games[*s].name);

			try {
				summary["mod_time"] = str_cast(lexical_cast<int>(games[*s].time_modified));
				load_game(data,games[*s].name,state);
				extract_summary_data_from_save(state,summary);
			} catch(io_exception&) {
				summary["corrupt"] = "yes";
				ERR_G << "save '" << games[*s].name << "' could not be loaded (io_exception)\n";
			} catch(config::error&) {
				summary["corrupt"] = "yes";
				ERR_G << "save '" << games[*s].name << "' could not be loaded (config parse error)\n";
			} catch(gamestatus::load_game_failed&) {
				summary["corrupt"] = "yes";
				ERR_G << "save '" << games[*s].name << "' could not be loaded (load_game_failed exception)\n";
			}
		}

		write_save_index();
	}

	std::vector<std::string> items;
	for(i = games.begin(); i != games.end(); ++i) {
		std::string name = i->name;
		name.resize(minimum<size_t>(name.size(),40));

		items.push_back(name);
	}

	gamemap map_obj(game_config,"");

	std::vector<gui::preview_pane*> preview_panes;
	save_preview_pane save_preview(disp,game_config,&map_obj,data,games,summaries);
	preview_panes.push_back(&save_preview);

	//create an option for whether the replay should be shown or not
	std::vector<gui::check_item> options;

	if(show_replay != NULL)
		options.push_back(gui::check_item(_("Show replay"),false));

	const int res = gui::show_dialog(disp,NULL,
					 _("Load Game"),
					 _("Choose the game to load"),
			         gui::OK_CANCEL,&items,&preview_panes,"",NULL,-1,NULL,&options,-1,-1,NULL,&buttons);

	if(res == -1)
		return "";

	if(show_replay != NULL) {
		*show_replay = options.front().checked;

		const config& summary = *summaries[res];
		if(summary["replay"] == "yes" && summary["snapshot"] == "no") {
			*show_replay = true;
		}
	}

	return games[res].name;
}

void unit_speak(const config& message_info, display& disp, const unit_map& units)
{
	for(unit_map::const_iterator i = units.begin(); i != units.end(); ++i) {
		if(i->second.matches_filter(message_info)) {
			disp.scroll_to_tile(i->first.x,i->first.y);
			const surface surface(image::get_image(i->second.type().image_profile(),image::UNSCALED));
			gui::show_dialog(disp,surface,i->second.underlying_description(),message_info["message"],gui::MESSAGE);
		}
	}
}


namespace {
	static const int unit_preview_border = 10;
}

unit_preview_pane::unit_preview_pane(display& disp, const gamemap* map, const unit& u, TYPE type, bool on_left_side)
                                        : gui::preview_pane(disp), details_button_(disp,_("Profile"),gui::button::TYPE_PRESS,"lite_small",gui::button::MINIMUM_SPACE),
										  map_(map), units_(&unit_store_), index_(0), left_(on_left_side),
										  weapons_(type == SHOW_ALL)
{
	unsigned w = font::relative_size(weapons_ ? 200 : 190);
	unsigned h = font::relative_size(weapons_ ? 370 : 140);
	set_measurements(w, h);
	unit_store_.push_back(u);
}

unit_preview_pane::unit_preview_pane(display& disp, const gamemap* map, const std::vector<unit>& units, TYPE type, bool on_left_side)
                                        : gui::preview_pane(disp), details_button_(disp,_("Profile"),gui::button::TYPE_PRESS,"lite_small",gui::button::MINIMUM_SPACE),
										  map_(map), units_(&units), index_(0), left_(on_left_side),
										  weapons_(type == SHOW_ALL)
{
	set_measurements(font::relative_size(200), font::relative_size(370));
}

bool unit_preview_pane::show_above() const
{
	return !weapons_;
}

bool unit_preview_pane::left_side() const
{
	return left_;
}

void unit_preview_pane::set_selection(int index)
{
	index = minimum<int>(int(units_->size()-1),index);
	if(index != index_ && index >= 0) {
		index_ = index;
		set_dirty();
		if(map_ != NULL) {
			details_button_.set_dirty();
		}
	}
}

void unit_preview_pane::draw_contents()
{
	if(index_ < 0 || index_ >= int(units_->size())) {
		return;
	}

	const unit& u = (*units_)[index_];

	const bool right_align = left_side();

	surface const screen = disp().video().getSurface();

	SDL_Rect const &loc = location();
	const SDL_Rect area = { loc.x + unit_preview_border, loc.y + unit_preview_border,
	                        loc.w - unit_preview_border * 2, loc.h - unit_preview_border * 2 };
	SDL_Rect clip_area = area;
	const clip_rect_setter clipper(screen,clip_area);

	surface unit_image(image::get_image(u.type().image(),image::UNSCALED));
	if(left_side() == false && unit_image != NULL) {
		unit_image.assign(image::reverse_image(unit_image));
	}

	SDL_Rect image_rect = {area.x,area.y,0,0};

	if(unit_image != NULL) {
		SDL_Rect rect = {right_align ? area.x : area.x + area.w - unit_image->w,area.y,unit_image->w,unit_image->h};
		SDL_BlitSurface(unit_image,NULL,screen,&rect);
		image_rect = rect;
	}

	//place the 'unit profile' button
	if(map_ != NULL) {
		const SDL_Rect button_loc = {right_align ? area.x : area.x + area.w - details_button_.location().w,
		                             image_rect.y + image_rect.h,
		                             details_button_.location().w,details_button_.location().h};
		details_button_.set_location(button_loc);
	}

	SDL_Rect description_rect = {image_rect.x,image_rect.y+image_rect.h+details_button_.location().h,0,0};

	if(u.description().empty() == false) {
		std::stringstream desc;
		desc << font::NORMAL_TEXT << u.description();
		const std::string description = desc.str();
		description_rect = font::text_area(description,font::SIZE_NORMAL);
		description_rect = font::draw_text(&disp(),area,font::SIZE_NORMAL,font::NORMAL_COLOUR,desc.str(),right_align ? image_rect.x : image_rect.x + image_rect.w - description_rect.w,image_rect.y+image_rect.h+details_button_.location().h);
	}

	std::stringstream details;
	details << u.type().language_name()
			<< "\n" << _("level") << " "
			<< u.type().level() << "\n"
			<< unit_type::alignment_description(u.type().alignment()) << "\n"
			<< u.traits_description() << " \n";

	const std::vector<std::string>& abilities = u.type().abilities();
	for(std::vector<std::string>::const_iterator a = abilities.begin(); a != abilities.end(); ++a) {
		details << gettext(a->c_str());
		if(a+1 != abilities.end()) {
			details << ", ";
		}
	}

	details << " \n";

	//display in green/white/red depending on hitpoints
	if(u.hitpoints() <= u.max_hitpoints()/3)
		details << font::BAD_TEXT;
	else if(u.hitpoints() > 2*(u.max_hitpoints()/3))
		details << font::GOOD_TEXT;

	details << _("HP: ") << u.hitpoints()
			<< "/" << u.max_hitpoints() << "\n";
	
	if(u.can_advance() == false) {
		details << _("XP: ") << u.experience() << "/-";
	} else {
		//if killing a unit the same level as us would level us up,
		//then display in green
		if(u.max_experience() - u.experience() < game_config::kill_experience) {
			details << font::GOOD_TEXT;
		}

		details << _("XP: ") << u.experience() << "/" << u.max_experience();
	}
	
	if(weapons_) {
		details << "\n"
				<< _("Moves: ") << u.movement_left() << "/"
				<< u.total_movement()
				<< "\n";

		const std::vector<attack_type>& attacks = u.attacks();
		for(std::vector<attack_type>::const_iterator at_it = attacks.begin();
		    at_it != attacks.end(); ++at_it) {

			details << "\n"
			        << gettext(at_it->name().c_str())
			        << " (" << gettext(at_it->type().c_str()) << ")\n";
			if (!at_it->special().empty())
				details << gettext(at_it->special().c_str());
			details << "\n"
			        << at_it->damage() << "-" << at_it->num_attacks() << " -- "
			        << (at_it->range() == attack_type::SHORT_RANGE ? _("melee") : _("ranged"));
		}
	}
	
	const std::string text = details.str();
	
	const std::vector<std::string> lines = utils::split(text, '\n');

	SDL_Rect cur_area = area;

	if(weapons_) {
		cur_area.y += image_rect.h + description_rect.h + details_button_.location().h;
	}

	for(std::vector<std::string>::const_iterator line = lines.begin(); line != lines.end(); ++line) {
		int xpos = cur_area.x;
		if(right_align && !weapons_) {
			const SDL_Rect& line_area = font::text_area(*line,font::SIZE_SMALL);
			xpos = cur_area.x + cur_area.w - line_area.w;
		}

		cur_area = font::draw_text(&disp(),location(),font::SIZE_SMALL,font::NORMAL_COLOUR,*line,xpos,cur_area.y);
		cur_area.y += cur_area.h;
	}
}

void unit_preview_pane::process_event()
{
	if(map_ != NULL && details_button_.pressed() && index_ >= 0 && index_ < int(units_->size())) {

		show_unit_description(disp(), (*units_)[index_]);
	}
}

void show_unit_description(display& disp, const unit& u)
{
	help::show_help(disp,"unit_" + u.type().id());
}


namespace {
	static const int campaign_preview_border = 10;
}

campaign_preview_pane::campaign_preview_pane(display& disp,std::vector<std::pair<std::string,std::string> >* desc) : gui::preview_pane(disp),descriptions_(desc),index_(0)
{
	set_measurements(350, 400);
}

bool campaign_preview_pane::show_above() const { return false; }
bool campaign_preview_pane::left_side() const { return false; }

void campaign_preview_pane::set_selection(int index)
{
	index = minimum<int>(descriptions_->size()-1,index);
	if(index != index_ && index >= 0) {
		index_ = index;
		set_dirty();
	}
}

void campaign_preview_pane::draw_contents()
{
	if(index_ < 0 || index_ >= descriptions_->size()) {
		return;
	}

	const SDL_Rect area = {
		location().x+campaign_preview_border,
		location().y+campaign_preview_border*5,
		location().w-campaign_preview_border*2,
		location().h-campaign_preview_border*6 };

	/* background frame */
	static const std::string default_style("mainmenu");
	const std::string* style = &default_style;
	gui::draw_dialog_frame(area.x,area.y,area.w,area.h,disp(),style);

	/* description text */
	const std::string& desc_text = font::word_wrap_text((*descriptions_)[index_].first,font::SIZE_SMALL,area.w-2*campaign_preview_border);
	const std::vector<std::string> lines = utils::split(desc_text, '\n');
	SDL_Rect txt_area = { area.x+campaign_preview_border,area.y,0,0 };

	for(std::vector<std::string>::const_iterator line = lines.begin(); line != lines.end(); ++line) {
		txt_area = font::draw_text(&disp(),location(),font::SIZE_SMALL,font::NORMAL_COLOUR,*line,txt_area.x,txt_area.y);
		txt_area.y += txt_area.h;
	}

	/* description image */
	surface img(NULL);
	const std::string desc_img_name = (*descriptions_)[index_].second;
	if(!desc_img_name.empty()) {
		img.assign(image::get_image(desc_img_name,image::UNSCALED));
	}
	if (!img.null()) {
		SDL_Rect src_rect,dst_rect;
		int max_height = area.h-(txt_area.h+txt_area.y-area.y);

		src_rect.x = src_rect.y = 0;
		src_rect.w = minimum<int>(area.w,img->w);
		src_rect.h = minimum<int>(max_height,img->h);
		dst_rect.x = area.x+(area.w-src_rect.w)/2;
		dst_rect.y = txt_area.y+(max_height-src_rect.h)/2;

		SDL_BlitSurface(img,&src_rect,disp().video().getSurface(),&dst_rect);

	}
}

} //end namespace dialogs
