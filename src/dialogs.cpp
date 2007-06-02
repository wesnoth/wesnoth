/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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
#include "game_config.hpp"
#include "game_errors.hpp"
#include "game_events.hpp"
#include "gettext.hpp"
#include "help.hpp"
#include "language.hpp"
#include "log.hpp"
#include "map.hpp"
#include "marked-up_text.hpp"
#include "menu_events.hpp"
#include "minimap.hpp"
#include "preferences.hpp"
#include "replay.hpp"
#include "construct_dialog.hpp"
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

#define LOG_NG LOG_STREAM(info, engine)
#define LOG_DP LOG_STREAM(info, display)
#define ERR_G  LOG_STREAM(err, general)


namespace dialogs
{

void advance_unit(const game_data& info,
				  const gamemap& map,
                  unit_map& units,
                  gamemap::location loc,
                  display& gui, bool random_choice)
{
	unit_map::iterator u = units.find(loc);
	if(u == units.end() || u->second.advances() == false)
		return;

	LOG_DP << "advance_unit: " << u->second.id() << "\n";

	const std::vector<std::string>& options = u->second.advances_to();

	std::vector<std::string> lang_options;

	std::vector<unit> sample_units;
	for(std::vector<std::string>::const_iterator op = options.begin(); op != options.end(); ++op) {
		sample_units.push_back(::get_advanced_unit(info,units,loc,*op));
		const unit& type = sample_units.back();

#ifdef LOW_MEM
		lang_options.push_back(IMAGE_PREFIX + type.absolute_image() + COLUMN_SEPARATOR + type.language_name());
#else
		lang_options.push_back(IMAGE_PREFIX + type.absolute_image() + u->second.image_mods() + COLUMN_SEPARATOR + type.language_name());
#endif
		preferences::encountered_units().insert(*op);
	}

	const config::child_list& mod_options = u->second.get_modification_advances();

	for(config::child_list::const_iterator mod = mod_options.begin(); mod != mod_options.end(); ++mod) {
		sample_units.push_back(::get_advanced_unit(info,units,loc,u->second.id()));
		sample_units.back().add_modification("advance",**mod);
		const unit& type = sample_units.back();
		if((**mod)["image"].str().size()){
		  lang_options.push_back(IMAGE_PREFIX + (**mod)["image"].str() + COLUMN_SEPARATOR + (**mod)["description"].str());
		}else{
#ifdef LOW_MEM
		  lang_options.push_back(IMAGE_PREFIX + type.absolute_image() + COLUMN_SEPARATOR + (**mod)["description"].str());
#else
		  lang_options.push_back(IMAGE_PREFIX + type.absolute_image() + u->second.image_mods() + COLUMN_SEPARATOR + (**mod)["description"].str());
#endif
		}
	}

	LOG_DP << "options: " << options.size() << "\n";

	int res = 0;

	if(lang_options.empty()) {
		return;
	} else if(random_choice) {
		res = rand()%lang_options.size();
	} else if(lang_options.size() > 1) {

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
	
	// in some rare cases the unit can have enough XP to advance again so try to do that, 
	// make sure that we're no entering an infinite level loop
	u = units.find(loc);
	if(u != units.end()) {
		// level 10 unit gives 80 XP and the highest mainline is level 5
		if(u->second.experience() < 81) {
			advance_unit(info, map, units, loc, gui,  random_choice);
		} else {
			LOG_STREAM(err, config) << "Unit has an too high amount of " << u->second.experience() 
				<< " XP left, cascade leveling disabled\n";
		}
	} else {
		LOG_STREAM(err, engine) << "Unit advanced no longer exists\n";
	}
}

bool animate_unit_advancement(const game_data& info,unit_map& units, gamemap::location loc, display& gui, size_t choice)
{
	const events::command_disabler cmd_disabler;

	unit_map::iterator u = units.find(loc);
	if(u == units.end() || u->second.advances() == false) {
		return false;
	}

	const std::vector<std::string>& options = u->second.advances_to();
	const config::child_list& mod_options = u->second.get_modification_advances();

	if(choice >= options.size() + mod_options.size()) {
		return false;
	}

	//when the unit advances, it fades to white, and then switches to the
	//new unit, then fades back to the normal colour

	if(!gui.video().update_locked()) {
		u->second.set_leveling_out(gui,u->first);
		while(!u->second.get_animation()->animation_would_finish()) {
			gui.invalidate(loc);
			gui.draw();
			events::pump();
			gui.delay(10);
		}
	}

	if(choice < options.size()) {
		const std::string& chosen_unit = options[choice];
		::advance_unit(info,units,loc,chosen_unit);
	} else {
		unit amla_unit(u->second);

		LOG_NG << "firing advance event (AMLA)\n";
		game_events::fire("advance",loc);

		amla_unit.get_experience(-amla_unit.max_experience()); //subtract xp required
		amla_unit.add_modification("advance",*mod_options[choice - options.size()]);
		units.replace(new std::pair<gamemap::location,unit>(loc,amla_unit));

		LOG_NG << "firing post_advance event (AMLA)\n";
		game_events::fire("post_advance",loc);
	}

	u = units.find(loc);
	gui.invalidate_unit();

	if(u != units.end() && !gui.video().update_locked()) {
		u->second.set_leveling_in(gui,u->first);
		while(!u->second.get_animation()->animation_would_finish()) {
			gui.invalidate(loc);
			gui.draw();
			events::pump();
			gui.delay(10);
		}
		u->second.set_standing(gui,u->first);
		gui.invalidate(loc);
		gui.draw();
		events::pump();
	}

	gui.invalidate_all();
	gui.draw();

	return true;
}

void show_objectives(display& disp, const config& level, const std::string& objectives)
{
	static const std::string no_objectives(_("No objectives available"));
	const std::string& name = level["name"];

	gui::show_dialog(disp, NULL, "", "*~" + name + "\n" +
	                 (objectives.empty() ? no_objectives : objectives),
	                 gui::OK_ONLY);
}

int get_save_name(display & disp,const std::string& message, const std::string& txt_label,
				  std::string* fname, gui::DIALOG_TYPE dialog_type, const std::string& title,
				  const bool has_exit_button)
{
	static int quit_prompt = 0;
	const std::string& tmp_title = (title.empty()) ? _("Save Game") : title;
	bool ignore_opt = false;
    int overwrite=0;
    int res=0;
    do {
		gui::dialog d(disp, tmp_title, message, dialog_type);
		d.set_textbox(txt_label, *fname);
		if(has_exit_button) {
			d.add_button( new gui::dialog_button(disp.video(), _("Quit Game"),
				gui::button::TYPE_PRESS, 2), gui::dialog::BUTTON_STANDARD);
			if(quit_prompt < 0) {
				res = 1;
			} else if(quit_prompt > 5) {
				d.add_button( new gui::dialog_button(disp.video(), _("Ignore All"),
					gui::button::TYPE_CHECK), gui::dialog::BUTTON_CHECKBOX);
				res = d.show();
				ignore_opt = d.option_checked();
			} else {
				res = d.show();
				if(res == 1) {
					++quit_prompt;
				} else {
					quit_prompt = 0;
				}
			}
		} else {
			res = d.show();
		}
		*fname = d.textbox_text();
		if (res == 0 && save_game_exists(*fname)) {
			overwrite = gui::show_dialog(disp,NULL,_("Overwrite?"),
				_("Save already exists. Do you want to overwrite it ?"),gui::YES_NO);
		} else {
			overwrite = 0;
		}
    } while ((res==0)&&(overwrite!=0));
	if(ignore_opt) {
		quit_prompt = -1;
	}
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
			                                 NULL,NULL,"",NULL,-1,&options);

			//see if the user doesn't want to be asked this again
			wassert(options.empty() == false);
			if(options.front().checked) {
				preferences::set_ask_delete_saves(false);
			}

			if(res != 0) {
				return gui::CONTINUE_DIALOG;
			}
		}

		//delete the file
		delete_game(saves_[index].name);

		//remove it from the list of saves
		saves_.erase(saves_.begin() + index);

		if(index < summaries_.size()) {
			summaries_.erase(summaries_.begin() + index);
		}

		return gui::DELETE_ITEM;
	} else {
		return gui::CONTINUE_DIALOG;
	}
}

static const int save_preview_border = 10;

class save_preview_pane : public gui::preview_pane
{
public:
	save_preview_pane(CVideo &video, const config& game_config, gamemap* map, const game_data& data,
	                  const std::vector<save_info>& info, const std::vector<config*>& summaries)
		: gui::preview_pane(video), game_config_(&game_config), map_(map), data_(&data), info_(&info), summaries_(&summaries), index_(0)
	{
		set_measurements(minimum<int>(200,video.getx()/4),
				 minimum<int>(400,video.gety() * 4/5));
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
	if (size_t(index_) >= summaries_->size() || info_->size() != summaries_->size()) {
		return;
	}

	std::string dummy;
	config& summary = *(*summaries_)[index_];
	if (summary["label"] == ""){
		try {
			load_game_summary((*info_)[index_].name, summary, &dummy);
			*(*summaries_)[index_] = summary;
		} catch(game::load_game_failed&) {
			summary["corrupt"] = "yes";
		}
	}

	surface const screen = video().getSurface();

	SDL_Rect const &loc = location();
	const SDL_Rect area = { loc.x + save_preview_border, loc.y + save_preview_border,
	                        loc.w - save_preview_border * 2, loc.h - save_preview_border * 2 };
	SDL_Rect clip_area = area;
	const clip_rect_setter clipper(screen,clip_area);

	int ypos = area.y;

	const game_data::unit_type_map::const_iterator leader = data_->unit_types.find(summary["leader"]);
	if(leader != data_->unit_types.end()) {

#ifdef LOW_MEM
		const surface image(image::get_image(leader->second.image(),image::UNSCALED));
#else
		const surface image(image::get_image(leader->second.image() + "~RC(" + leader->second.flag_rgb() + ">1)",image::UNSCALED));
#endif

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
#ifdef USE_TINY_GUI
				const int minimap_size = 60;
#else
				const int minimap_size = 100;
#endif
				map_->read(map_data);

				map_surf = image::getMinimap(minimap_size, minimap_size, *map_);
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

	char* old_locale= setlocale(LC_TIME, get_locale().localename.c_str());
	char time_buf[256];
	const size_t res = strftime(time_buf,sizeof(time_buf),_("%a %b %d %H:%M %Y"),localtime(&((*info_)[index_].time_modified)));
	if(res == 0) {
		time_buf[0] = 0;
	}

	if(old_locale) {
		setlocale(LC_TIME, old_locale);
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

	font::draw_text(&video(), area, font::SIZE_SMALL, font::NORMAL_COLOUR, str.str(), area.x, ypos, true);
}

std::string format_time_summary(time_t t)
{
	time_t curtime = time(NULL);
	const struct tm* timeptr = localtime(&curtime);
	if(timeptr == NULL) {
		return "";
	}

	const struct tm current_time = *timeptr;

	timeptr = localtime(&t);
	if(timeptr == NULL) {
		return "";
	}

	const struct tm save_time = *timeptr;

	const char* format_string = _("%b %d %y");

	if(current_time.tm_year == save_time.tm_year) {
		const int days_apart = current_time.tm_yday - save_time.tm_yday;
		if(days_apart == 0) {
			//save is from today
			format_string = _("%H:%M");
		} else if(days_apart > 0 && days_apart <= current_time.tm_wday) {
			//save is from this week
			format_string = _("%A, %H:%M");
		} else {
			//save is from current year
			format_string = _("%b %d");
		}
	} else {
		//save is from a different year
		format_string = _("%b %d %y");
	}

	char buf[40];
	const size_t res = strftime(buf,sizeof(buf),format_string,&save_time);
	if(res == 0) {
		buf[0] = 0;
	}

	return buf;
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

	std::vector<config*> summaries;
	std::vector<save_info>::const_iterator i;
	for(i = games.begin(); i != games.end(); ++i) {
		config& cfg = save_summary(i->name);
		summaries.push_back(&cfg);
	}

	delete_save save_deleter(disp,games,summaries);
	gui::dialog_button_info delete_button(&save_deleter,_("Delete Save"));
	std::vector<gui::dialog_button_info> buttons;
	buttons.push_back(delete_button);

	const events::event_context context;

	std::vector<std::string> items;
	std::ostringstream heading;
	heading << HEADING_PREFIX << _("Name") << COLUMN_SEPARATOR << _("Date");
	items.push_back(heading.str());

	for(i = games.begin(); i != games.end(); ++i) {
		std::string name = i->name;
		name.resize(minimum<size_t>(name.size(),40));

		std::ostringstream str;
		str << name << COLUMN_SEPARATOR << format_time_summary(i->time_modified);

		items.push_back(str.str());
	}

	gui::menu::basic_sorter sorter;
	sorter.set_alpha_sort(0).set_id_sort(1);

	gamemap map_obj(game_config, "");

	std::vector<gui::preview_pane*> preview_panes;
	save_preview_pane save_preview(disp.video(),game_config,&map_obj,data,games,summaries);
	preview_panes.push_back(&save_preview);

	//create an option for whether the replay should be shown or not
	std::vector<gui::check_item> options;

	if(show_replay != NULL)
		options.push_back(gui::check_item(_("Show replay"),false));

	const int res = gui::show_dialog(disp,NULL,
					 _("Load Game"),
					 _("Choose the game to load"),
			         gui::OK_CANCEL,&items,&preview_panes,"",NULL,-1,&options,-1,-1,NULL,&buttons,"",&sorter);

	write_save_index();

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

namespace {
	static const int unit_preview_border = 10;
}

unit_preview_pane::unit_preview_pane(display& disp, const gamemap* map, const unit& u, TYPE type, bool on_left_side)
				    : gui::preview_pane(disp.video()), disp_(disp),
				      details_button_(disp.video(),_("Profile"),gui::button::TYPE_PRESS,"lite_small",gui::button::MINIMUM_SPACE),
				      map_(map), units_(&unit_store_), index_(0), left_(on_left_side),
				      weapons_(type == SHOW_ALL)
{
	unsigned w = font::relative_size(weapons_ ? 200 : 190);
	unsigned h = font::relative_size(weapons_ ? 370 : 140);
	set_measurements(w, h);
	unit_store_.push_back(u);
}

unit_preview_pane::unit_preview_pane(display& disp, const gamemap* map, std::vector<unit>& units, TYPE type, bool on_left_side)
                                    : gui::preview_pane(disp.video()), disp_(disp),
				      details_button_(disp.video(),_("Profile"),gui::button::TYPE_PRESS,"lite_small",gui::button::MINIMUM_SPACE),
				      map_(map), units_(&units), index_(0), left_(on_left_side),
				      weapons_(type == SHOW_ALL)
{
	set_measurements(font::relative_size(200), font::relative_size(370));
}

handler_vector unit_preview_pane::handler_members()
{
	handler_vector h;
	h.push_back(&details_button_);
	return h;
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

	unit& u = (*units_)[index_];

	const bool right_align = left_side();

	surface const screen = video().getSurface();

	SDL_Rect const &loc = location();
	const SDL_Rect area = { loc.x + unit_preview_border, loc.y + unit_preview_border,
	                        loc.w - unit_preview_border * 2, loc.h - unit_preview_border * 2 };
	SDL_Rect clip_area = area;
	const clip_rect_setter clipper(screen,clip_area);

	surface unit_image = u.still_image();
	if (!left_) 
		unit_image = image::reverse_image(unit_image);
	
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
		description_rect = font::draw_text(&video(),area,font::SIZE_NORMAL,font::NORMAL_COLOUR,desc.str(),right_align ? image_rect.x : image_rect.x + image_rect.w - description_rect.w,image_rect.y+image_rect.h+details_button_.location().h);
	}

	std::stringstream details;
	details << u.language_name()
			<< "\n" << _("level") << " "
			<< u.level() << "\n"
			<< unit_type::alignment_description(u.alignment()) << "\n"
			<< u.traits_description() << " \n";

	const std::vector<std::string>& abilities = u.unit_ability_tooltips();
	for(std::vector<std::string>::const_iterator a = abilities.begin(); a != abilities.end(); ++a) {
		details << gettext(a->c_str());
		if(a+2 != abilities.end()) {
			details << ", ";
		}
		++a;
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

		std::vector<attack_type>& attacks = u.attacks();
		for(std::vector<attack_type>::iterator at_it = attacks.begin();
		    at_it != attacks.end(); ++at_it) {
			at_it->set_specials_context(gamemap::location(),u);

			details << "\n" << at_it->name()
			        << " (" << gettext(at_it->type().c_str()) << ")\n";

			details << at_it->weapon_specials(true);
			details << "\n"
			        << at_it->damage() << "-" << at_it->num_attacks() << " -- "
			        << _(at_it->range().c_str());
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

		cur_area = font::draw_text(&video(),location(),font::SIZE_SMALL,font::NORMAL_COLOUR,*line,xpos,cur_area.y);
		cur_area.y += cur_area.h;
	}
}

void unit_preview_pane::process_event()
{
	if(map_ != NULL && details_button_.pressed() && index_ >= 0 && index_ < int(units_->size())) {

		show_unit_description(disp_, (*units_)[index_]);
	}
}

void show_unit_description(display &disp, const unit& u)
{
	help::show_help(disp,"unit_" + u.id());
}


namespace {
	static const int campaign_preview_border = font::relative_size(10);
}

campaign_preview_pane::campaign_preview_pane(CVideo &video,std::vector<std::pair<std::string,std::string> >* desc) : gui::preview_pane(video),descriptions_(desc),index_(0)
{
#ifdef USE_TINY_GUI
	set_measurements(160, 200);
#else
	set_measurements(350, 400);
#endif
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
	if (size_t(index_) >= descriptions_->size()) {
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
	gui::dialog_frame f(video(), "", style);
	f.layout(area);
	f.draw_background();
	f.draw_border();

	/* description text */
	const std::string& desc_text = font::word_wrap_text((*descriptions_)[index_].first,font::SIZE_SMALL,area.w-2*campaign_preview_border);
	const std::vector<std::string> lines = utils::split(desc_text, '\n',utils::STRIP_SPACES);
	SDL_Rect txt_area = { area.x+campaign_preview_border,area.y,0,0 };

	for(std::vector<std::string>::const_iterator line = lines.begin(); line != lines.end(); ++line) {
	  txt_area = font::draw_text(&video(),location(),font::SIZE_SMALL,font::NORMAL_COLOUR,*line,txt_area.x,txt_area.y);
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

		SDL_BlitSurface(img,&src_rect,video().getSurface(),&dst_rect);

	}
}

} //end namespace dialogs
