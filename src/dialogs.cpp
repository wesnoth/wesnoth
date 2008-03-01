/* $Id$ */
/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file dialogs.cpp 
//! Various dialogs: advance_unit, show_objectives, save+load game, network::connection.

#include "global.hpp"

#include "dialogs.hpp"
#include "game_errors.hpp"
#include "game_events.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "help.hpp"
#include "language.hpp"
#include "log.hpp"
#include "marked-up_text.hpp"
#include "menu_events.hpp"
#include "minimap.hpp"
#include "replay.hpp"
#include "thread.hpp"
#include "unit.hpp"
#include "wml_separators.hpp"
#include "widgets/progressbar.hpp"
#include "wml_exception.hpp"

#include <assert.h>
#include <clocale>

#define LOG_NG LOG_STREAM(info, engine)
#define LOG_DP LOG_STREAM(info, display)
#define ERR_G  LOG_STREAM(err, general)
#define ERR_CF LOG_STREAM(err, config)

namespace dialogs
{

void advance_unit(const game_data& info,
		  const gamemap& map,
                  unit_map& units,
                  gamemap::location loc,
                  game_display& gui,
		  bool random_choice,
		  const bool add_replay_event)
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

		units_list_preview_pane unit_preview(gui,&map,sample_units);
		std::vector<gui::preview_pane*> preview_panes;
		preview_panes.push_back(&unit_preview);

		gui::dialog advances = gui::dialog(gui,
				      _("Advance Unit"),
		                      _("What should our victorious unit become?"),
		                      gui::OK_ONLY);
		advances.set_menu(lang_options);
		advances.set_panes(preview_panes);
		res = advances.show();
	}

	if(add_replay_event) {
		recorder.add_advancement(loc);
	}

	recorder.choose_option(res);

	LOG_DP << "animating advancement...\n";
	animate_unit_advancement(info,units,loc,gui,size_t(res));
	
	// In some rare cases the unit can have enough XP to advance again, 
	// so try to do that. 
	// Make sure that we don't enter an infinite level loop.
	u = units.find(loc);
	if(u != units.end()) {
		// Level 10 unit gives 80 XP and the highest mainline is level 5
		if(u->second.experience() < 81) {
			// For all leveling up we have to add advancement to replay here because replay
			// doesn't handle multi advancemnet
			advance_unit(info, map, units, loc, gui, random_choice, true);
		} else {
			LOG_STREAM(err, config) << "Unit has an too high amount of " << u->second.experience()
				<< " XP left, cascade leveling disabled\n";
		}
	} else {
		LOG_STREAM(err, engine) << "Unit advanced no longer exists\n";
	}
}

bool animate_unit_advancement(const game_data& info,unit_map& units, gamemap::location loc, game_display& gui, size_t choice)
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

	// When the unit advances, it fades to white, and then switches 
	// to the new unit, then fades back to the normal colour

	if(!gui.video().update_locked()) {
		unit_animator animator;
		animator.add_animation(&u->second,"levelout",u->first);
		animator.start_animations();
		animator.wait_for_end();
	}

	if(choice < options.size()) {
		const std::string& chosen_unit = options[choice];
		::advance_unit(info,units,loc,chosen_unit);
	} else {
		unit amla_unit(u->second);

		LOG_NG << "firing advance event (AMLA)\n";
		game_events::fire("advance",loc);

		amla_unit.get_experience(-amla_unit.max_experience()); // subtract xp required
		amla_unit.add_modification("advance",*mod_options[choice - options.size()]);
		units.replace(new std::pair<gamemap::location,unit>(loc,amla_unit));

		LOG_NG << "firing post_advance event (AMLA)\n";
		game_events::fire("post_advance",loc);
	}

	u = units.find(loc);
	gui.invalidate_unit();

	if(u != units.end() && !gui.video().update_locked()) {
		unit_animator animator;
		animator.add_animation(&u->second,"levelin",u->first);
		animator.start_animations();
		animator.wait_for_end();
		u->second.set_standing(u->first);
		gui.invalidate(loc);
		gui.draw();
		events::pump();
	}

	gui.invalidate_all();
	gui.draw();

	return true;
}

void show_objectives(game_display& disp, const config& level, const std::string& objectives)
{
	static const std::string no_objectives(_("No objectives available"));
	const std::string& name = level["name"];
	std::string campaign_name = std::string(level["campaign"]);
	replace_underbar2space(campaign_name);

	gui::message_dialog(disp, "", "*~" + name +
			(campaign_name.empty() ? "\n" : " - " + campaign_name + "\n") +
	                (objectives.empty() ? no_objectives : objectives)
	                ).show();
}

bool is_illegal_file_char(char c)
{
	return c == '/' || c == '\\' || c == ':';
}

int get_save_name(display & disp,const std::string& message, const std::string& txt_label,
				  std::string* fname, gui::DIALOG_TYPE dialog_type, const std::string& title,
				  const bool has_exit_button, const bool ask_for_filename)
{
	static int quit_prompt = 0;
	const std::string& tmp_title = (title.empty()) ? _("Save Game") : title;
	bool ignore_opt = false;
	int overwrite=0;
	int res=0;
	bool ask = ask_for_filename;
	do {
		if (ask) {
			gui::dialog d(disp, tmp_title, message, dialog_type);
			d.set_textbox(txt_label, *fname);
			if(has_exit_button) {
				d.add_button(new gui::dialog_button(disp.video(), _("Quit Game"),
					gui::button::TYPE_PRESS, 2), gui::dialog::BUTTON_STANDARD);
				if(quit_prompt < 0) {
					res = 1;
				} else if(quit_prompt > 5) {
					d.add_button(new gui::dialog_button(disp.video(), _("Ignore All"),
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
		} else {
			ask = true;
		}

		if (std::count_if(fname->begin(),fname->end(),is_illegal_file_char)) {
			gui::message_dialog(disp, _("Error"), 
				_("Save names may not contain colons, slashes, or backslashes. "
				"Please choose a different name.")).show();
			overwrite = 1;
			continue;
		}

		if (is_gzip_file(*fname)) {
			gui::message_dialog(disp, _("Error"), 
				_("Save names should not end on '.gz'. "
				"Please choose a different name.")).show();
			overwrite = 1;
			continue;
		}

		if (res == 0 && save_game_exists(*fname)) {
			std::stringstream s;
			s << _("Save already exists. Do you want to overwrite it?")
			  << std::endl << _("Name: ") << *fname;
			overwrite = gui::dialog(disp,_("Overwrite?"),
			    s.str(), gui::YES_NO).show();
		} else {
			overwrite = 0;
		}
	} while ((res == 0) && (overwrite != 0));

	if(ignore_opt) {
		quit_prompt = -1;
	}
	return res;
}

//! Class to handle deleting a saved game.
namespace {

class load_game_filter_textbox : public gui::dialog_textbox {
public:
	load_game_filter_textbox(CVideo& video, const std::vector<std::string>& items, gui::dialog& dialog)
	  : gui::dialog_textbox(new gui::label(video, _("Filter: ")), video, 250),
	    items_(items),
	    dialog_(dialog),
		first_time_(true)
	{
		sorter_.set_alpha_sort(0).set_id_sort(1);
		set_text("");
	}

	int get_save_index(int index) const {
		//we must add one to the index to ignore the header row, and
		//then subtract one from the result to return the index not including
		//the header row.
		++index;
		if(size_t(index) >= index_map_.size()) {
			return -1;
		}
		return index_map_[index]-1;
	}
private:
	std::vector<std::string> items_, filtered_items_;
	std::vector<int> index_map_;
	gui::dialog& dialog_;
	gui::menu::basic_sorter sorter_;
	bool first_time_;
	virtual void handle_text_changed(const wide_string& text) {
		filtered_items_.clear();
		index_map_.clear();
		const std::string t = utils::wstring_to_string(text);
		for(size_t n = 0; n != items_.size(); ++n) {
			if(n == 0 || std::search(items_[n].begin(), items_[n].end(),
			                         t.begin(), t.end(), chars_equal_insensitive) != items_[n].end()) {
				filtered_items_.push_back(items_[n]);
				index_map_.push_back(n);
			}
		}

		if(first_time_) {
			dialog_.set_menu(filtered_items_, &sorter_);
			first_time_ = false;
		} else {
			dialog_.set_menu_items(filtered_items_);
		}
	}
};

class delete_save : public gui::dialog_button_action
{
public:
	delete_save(display& disp, load_game_filter_textbox& filter, std::vector<save_info>& saves, std::vector<config*>& save_summaries) : disp_(disp), saves_(saves), summaries_(save_summaries), filter_(filter) {}
private:
	gui::dialog_button_action::RESULT button_pressed(int menu_selection);

	display& disp_;
	std::vector<save_info>& saves_;
	std::vector<config*>& summaries_;
	load_game_filter_textbox& filter_;
};

gui::dialog_button_action::RESULT delete_save::button_pressed(int menu_selection)
{
	const size_t index = size_t(filter_.get_save_index(menu_selection));
	if(index < saves_.size()) {

		// See if we should ask the user for deletion confirmation
		if(preferences::ask_delete_saves()) {
			gui::dialog dmenu(disp_,"",
					       _("Do you really want to delete this game?"),
					       gui::YES_NO);
			dmenu.add_option(_("Don't ask me again!"), true);
			const int res = dmenu.show();
			// See if the user doesn't want to be asked this again
			if(dmenu.option_checked()) {
				preferences::set_ask_delete_saves(false);
			}

			if(res != 0) {
				return gui::CONTINUE_DIALOG;
			}
		}

		// Delete the file
		delete_game(saves_[index].name);

		// Remove it from the list of saves
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
	                  const std::vector<save_info>& info, const std::vector<config*>& summaries, const load_game_filter_textbox& textbox)
		: gui::preview_pane(video), game_config_(&game_config), map_(map), data_(&data), info_(&info), summaries_(&summaries), index_(0), textbox_(textbox)
	{
		set_measurements(minimum<int>(200,video.getx()/4),
				 minimum<int>(400,video.gety() * 4/5));
	}

	void draw_contents();
	void set_selection(int index) {
		index_ = textbox_.get_save_index(index);
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
	const load_game_filter_textbox& textbox_;
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
		const surface image(image::get_image(leader->second.image()));
#else
		const surface image(image::get_image(leader->second.image() + "~RC(" + leader->second.flag_rgb() + ">1)"));
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
			} catch(gamemap::incorrect_format_exception& e) {
				ERR_CF << "map could not be loaded: " << e.msg_ << '\n';
			} catch(twml_exception& e) {
				ERR_CF << "map could not be loaded: " << e.dev_message << '\n';
			}
		}
	}

	if(map_surf != NULL) {
		SDL_Rect map_rect = {area.x + area.w - map_surf->w,area.y,map_surf->w,map_surf->h};
		ypos = maximum<int>(ypos,map_rect.y + map_rect.h + save_preview_border);
		SDL_BlitSurface(map_surf,NULL,screen,&map_rect);
	}

	char* old_locale= std::setlocale(LC_TIME, get_locale().localename.c_str());
	char time_buf[256];
	const size_t res = strftime(time_buf,sizeof(time_buf),_("%a %b %d %H:%M %Y"),localtime(&((*info_)[index_].time_modified)));
	if(res == 0) {
		time_buf[0] = 0;
	}

	if(old_locale) {
		std::setlocale(LC_TIME, old_locale);
	}

	std::stringstream str;

	// Escape all special characters in filenames
	std::string name = (*info_)[index_].name;
	str << font::BOLD_TEXT << utils::escape(name) << "\n" << time_buf;

	const std::string& campaign_type = summary["campaign_type"];
	if(summary["corrupt"] == "yes") {
		str << "\n" << _("#(Invalid)");
	} else if (!campaign_type.empty()) {
		str << "\n";

		if(campaign_type == "scenario") {
			utils::string_map symbols;
			symbols["campaign_name"] = summary["campaign"];
			str << vgettext("Campaign: $campaign_name", symbols);
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
			// save is from today
			format_string = _("%H:%M");
		} else if(days_apart > 0 && days_apart <= current_time.tm_wday) {
			// save is from this week
			format_string = _("%A, %H:%M");
		} else {
			// save is from current year
			format_string = _("%b %d");
		}
	} else {
		// save is from a different year
		format_string = _("%b %d %y");
	}

	char buf[40];
	const size_t res = strftime(buf,sizeof(buf),format_string,&save_time);
	if(res == 0) {
		buf[0] = 0;
	}

	return buf;
}

} // end anon namespace

std::string load_game_dialog(display& disp, const config& game_config, const game_data& data, bool* show_replay)
{
	std::vector<save_info> games;
	{
		cursor::setter cur(cursor::WAIT);
		games = get_saves_list();
	}

	if(games.empty()) {
		gui::message_dialog(disp,
		                 _("No Saved Games"),
				 _("There are no saved games to load.\n\n(Games are saved automatically when you complete a scenario)")).show();
		return "";
	}

	std::vector<config*> summaries;
	std::vector<save_info>::const_iterator i;
	for(i = games.begin(); i != games.end(); ++i) {
		config& cfg = save_summary(i->name);
		summaries.push_back(&cfg);
	}

	const events::event_context context;

	std::vector<std::string> items;
	std::ostringstream heading;
	heading << HEADING_PREFIX << _("Name") << COLUMN_SEPARATOR << _("Date");
	items.push_back(heading.str());

	for(i = games.begin(); i != games.end(); ++i) {
		std::string name = i->name;
		utils::truncate_as_wstring(name, minimum<size_t>(name.size(), 40));

		std::ostringstream str;
		str << name << COLUMN_SEPARATOR << format_time_summary(i->time_modified);

		items.push_back(str.str());
	}

	gui::menu::basic_sorter sorter;
	sorter.set_alpha_sort(0).set_id_sort(1);

	gamemap map_obj(game_config, "");


	gui::dialog lmenu(disp,
			  _("Load Game"),
			  _("Choose the game to load"), gui::NULL_DIALOG);
	lmenu.set_basic_behavior(gui::OK_CANCEL);
	load_game_filter_textbox* filter = new load_game_filter_textbox(disp.video(), items, lmenu);
	save_preview_pane save_preview(disp.video(),game_config,&map_obj,data,games,summaries,*filter);
	lmenu.set_textbox(filter);
	lmenu.add_pane(&save_preview);
	// create an option for whether the replay should be shown or not
	if(show_replay != NULL) {
		#ifdef USE_SMALL_GUI
			lmenu.add_option(_("Show replay"), false, gui::dialog::BUTTON_STANDARD);
		#else
			lmenu.add_option(_("Show replay"), false);
		#endif
	}
	lmenu.add_button(new gui::standard_dialog_button(disp.video(),_("OK"),0,false), gui::dialog::BUTTON_STANDARD);
	lmenu.add_button(new gui::standard_dialog_button(disp.video(),_("Cancel"),1,true), gui::dialog::BUTTON_STANDARD);

	delete_save save_deleter(disp,*filter,games,summaries);
	gui::dialog_button_info delete_button(&save_deleter,_("Delete Save"));
	#ifdef USE_SMALL_GUI
		//placing the buttons in one line so that none is coverd by any of the others
		lmenu.add_button(delete_button,gui::dialog::BUTTON_HELP);
	#else
		lmenu.add_button(delete_button);
	#endif

	int res = lmenu.show();

	write_save_index();

	if(res == -1)
		return "";

	res = filter->get_save_index(res);

	if(show_replay != NULL) {
	  *show_replay = lmenu.option_checked();

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

//! Show unit-stats in a side-pane to unit-list, recall-list, etc.

unit_preview_pane::unit_preview_pane(game_display& disp, const gamemap* map, TYPE type, bool on_left_side)
				    : gui::preview_pane(disp.video()), disp_(disp), map_(map), index_(0),
				      details_button_(disp.video(),_("Profile"),gui::button::TYPE_PRESS,"lite_small",gui::button::MINIMUM_SPACE),
				      left_(on_left_side), weapons_(type == SHOW_ALL)
{
	unsigned w = font::relative_size(weapons_ ? 200 : 190);
	unsigned h = font::relative_size(weapons_ ? 370 : 140);
	set_measurements(w, h);
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
	index = minimum<int>(int(size()-1),index);
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
	if(index_ < 0 || index_ >= int(size())) {
		return;
	}

	const details det = get_details();

	const bool right_align = left_side();

	surface const screen = video().getSurface();

	SDL_Rect const &loc = location();
	const SDL_Rect area = { loc.x + unit_preview_border, loc.y + unit_preview_border,
	                        loc.w - unit_preview_border * 2, loc.h - unit_preview_border * 2 };
	SDL_Rect clip_area = area;
	const clip_rect_setter clipper(screen,clip_area);

	surface unit_image = det.image;
	if (!left_)
		unit_image = image::reverse_image(unit_image);

	SDL_Rect image_rect = {area.x,area.y,0,0};

	if(unit_image != NULL) {
		SDL_Rect rect = {right_align ? area.x : area.x + area.w - unit_image->w,area.y,unit_image->w,unit_image->h};
		SDL_BlitSurface(unit_image,NULL,screen,&rect);
		image_rect = rect;
	}

	// Place the 'unit profile' button
	if(map_ != NULL) {
		const SDL_Rect button_loc = {right_align ? area.x : area.x + area.w - details_button_.location().w,
		                             image_rect.y + image_rect.h,
		                             details_button_.location().w,details_button_.location().h};
		details_button_.set_location(button_loc);
	}

	SDL_Rect description_rect = {image_rect.x,image_rect.y+image_rect.h+details_button_.location().h,0,0};

	if(det.description.empty() == false) {
		std::stringstream desc;
		desc << font::NORMAL_TEXT << det.description;
		const std::string description = desc.str();
		description_rect = font::text_area(description, font::SIZE_NORMAL);
		description_rect = font::draw_text(&video(), area, 
							font::SIZE_NORMAL, font::NORMAL_COLOUR, 
							desc.str(), right_align ?  image_rect.x : 
							image_rect.x + image_rect.w - description_rect.w, 
							image_rect.y + image_rect.h + details_button_.location().h);
	}

	std::stringstream text;
	text << det.name << "\n"
		<< font::BOLD_TEXT << _("level") << " " << det.level << "\n"
		<< det.alignment << "\n"
		<< det.traits << "\n";

	for(std::vector<std::string>::const_iterator a = det.abilities.begin(); a != det.abilities.end(); a++) {
		if(a != det.abilities.begin()) {
			text << ", ";
		}
		text << gettext(a->c_str());
	}
	text << "\n";

	// Use same coloring as in generate_report.cpp:
	text << det.hp_color << _("HP: ")
		<< det.hitpoints << "/" << det.max_hitpoints << "\n";

	text << det.xp_color << _("XP: ")
		<< det.experience << "/" << det.max_experience << "\n";

	if(weapons_) {
		text << _("Moves: ")
			<< det.movement_left << "/" << det.total_movement << "\n";

		for(std::vector<attack_type>::const_iterator at_it = det.attacks.begin();
		    at_it != det.attacks.end(); ++at_it) {
			// specials_context seems not needed here
			//at_it->set_specials_context(gamemap::location(),u);

			// see generate_report() in generate_report.cpp
			text << "<245,230,193>" << at_it->name()
				<< " (" << gettext(at_it->type().c_str()) << ")\n";

			std::string special = at_it->weapon_specials(true);
			if (!special.empty()) {
				text << "<166,146,117>  " << special << "\n";
			}
			text << "<166,146,117>  " << at_it->damage() << "-" << at_it->num_attacks()
				<< " -- " << _(at_it->range().c_str()) << "\n";
		}
	}

	// we don't remove empty lines, so all fields stay at the same place
	const std::vector<std::string> lines = utils::split(text.str(), '\n',
		utils::STRIP_SPACES & !utils::REMOVE_EMPTY);

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

units_list_preview_pane::units_list_preview_pane(game_display& disp, const gamemap* map, const unit& u, TYPE type, bool on_left_side)
					: unit_preview_pane(disp, map, type, on_left_side),
					  units_(&unit_store_)
{
	unit_store_.push_back(u);
}

units_list_preview_pane::units_list_preview_pane(game_display& disp, const gamemap* map, std::vector<unit>& units, TYPE type, bool on_left_side)
					: unit_preview_pane(disp, map, type, on_left_side),
					  units_(&units)
{}

size_t units_list_preview_pane::size() const
{
	return (units_!=NULL) ? units_->size() : 0;
}

//unit_preview_pane::
const unit_preview_pane::details units_list_preview_pane::get_details() const
{
	unit& u = (*units_)[index_];
	details det;

	det.image = u.still_image();

	det.description = u.description();
	det.name = u.language_name();
	det.level = u.level();
	det.alignment = unit_type::alignment_description(u.alignment());
	det.traits = u.traits_description();

	//we filter to remove the tooltips (increment by 2)
	const std::vector<std::string>& abilities = u.unit_ability_tooltips();
	for(std::vector<std::string>::const_iterator a = abilities.begin();
		 a != abilities.end(); a+=2) {
		det.abilities.push_back(*a);
	}
	
	det.hitpoints = u.hitpoints();
	det.max_hitpoints = u.max_hitpoints();
	det.hp_color = font::color2markup(u.hp_color());
	
	det.experience = u.experience();
	det.max_experience = u.max_experience();
	det.xp_color = font::color2markup(u.xp_color());

	det.movement_left = u.movement_left();
	det.total_movement= u.total_movement();

	det.attacks = u.attacks();
	return det;
}

void units_list_preview_pane::process_event()
{
	if(map_ != NULL && details_button_.pressed() && index_ >= 0 && index_ < int(size())) {
		show_unit_description(disp_, (*units_)[index_]);
	}
}

unit_types_preview_pane::unit_types_preview_pane(game_display& disp, const gamemap* map, std::vector<const unit_type*>& unit_types, int side, TYPE type, bool on_left_side)
					: unit_preview_pane(disp, map, type, on_left_side),
					  unit_types_(&unit_types), side_(side)
{}

size_t unit_types_preview_pane::size() const
{
	return (unit_types_!=NULL) ? unit_types_->size() : 0;
}

const unit_types_preview_pane::details unit_types_preview_pane::get_details() const
{
	const unit_type* t = (*unit_types_)[index_];
	details det;

	if (t==NULL)
		return det;

	std::string mod = "~RC(" + t->flag_rgb() + ">" + team::get_side_colour_index(side_) + ")";
	det.image = image::get_image(t->image()+mod);

	det.description = "";
	det.name = t->language_name();
	det.level = t->level();
	det.alignment = unit_type::alignment_description(t->alignment());

	//FIXME: This probably must be move into a unit_type function
	const std::vector<config*> traits = t->possible_traits();
	for(std::vector<config*>::const_iterator i = traits.begin(); i != traits.end(); i++) {
		if((**(i))["availability"] == "musthave") {
			std::string gender_string = (!t->genders().empty() && t->genders().front()== unit_race::FEMALE) ? "female_name" : "male_name";
			t_string name = (**i)[gender_string];
			if (name.empty()) {
				name = (**i)["name"];
			}
			if (!name.empty()) {
				if (i != traits.begin()) {
					det.traits += ", ";
				}
				det.traits += name;
			}
		}
	}

	det.abilities = t->abilities();
	
	det.hitpoints = t->hitpoints();
	det.max_hitpoints = t->hitpoints();
	det.hp_color = "<33,225,0>"; // from unit::hp_color()

	det.experience = 0;
	det.max_experience = t->experience_needed();
	det.xp_color = "<0,160,225>"; // from unit::xp_color()

	// Check if AMLA color is needed
	// FIXME: not sure if it's fully accurate (but not very important for unit_type)
	// xp_color also need a simpler function for doing this
	const config::child_list& advances = t->modification_advancements();
	for(config::child_list::const_iterator j = advances.begin(); j != advances.end(); ++j) {
		if (!utils::string_bool((**j)["strict_amla"]) || !t->can_advance()) {
			det.xp_color = "<100,0,150>"; // from unit::xp_color()
			break;
		}
	}

	det.movement_left = 0;
	det.total_movement= t->movement();

	det.attacks = t->attacks();
	return det;
}

void unit_types_preview_pane::process_event()
{
	if(map_ != NULL && details_button_.pressed() && index_ >= 0 && index_ < int(size())) { 
		const unit_type* type = (*unit_types_)[index_];
		if (type != NULL)
			show_unit_description(disp_, *type);
	}
}


void show_unit_description(game_display &disp, const unit& u)
{
	const unit_type* t = u.type();
	if (t != NULL)
		show_unit_description(disp, *t);
	else
		// can't find type, try open the id page to have feedback and unit error page
		help::show_unit_help(disp, u.id());
}

void show_unit_description(game_display &disp, const unit_type& t)
{
	help::show_unit_help(disp, t.id(), t.hide_help());
}


namespace {
	static const int campaign_preview_border = font::relative_size(10);
}

campaign_preview_pane::campaign_preview_pane(CVideo &video,std::vector<std::pair<std::string,std::string> >* desc) : gui::preview_pane(video),descriptions_(desc),index_(0)
{
// size of the campaign info window with the campaign description and image in pixel
#ifdef USE_TINY_GUI
	set_measurements(160, 200);
#else
	set_measurements(430, 440);
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
		location().y,
		location().w-campaign_preview_border*2,
		location().h };

	/* background frame */
	gui::dialog_frame f(video(), "", gui::dialog_frame::preview_style, false);
	f.layout(area);
	f.draw_background();
	f.draw_border();

	/* description text */
	std::string desc_text;
	try {
		desc_text = font::word_wrap_text((*descriptions_)[index_].first,
			font::SIZE_SMALL, area.w - 2 * campaign_preview_border);
	} catch (utils::invalid_utf8_exception&) {
		LOG_STREAM(err, engine) << "Invalid utf-8 found, campaign description is ignored.\n";
	}
	const std::vector<std::string> lines = utils::split(desc_text, '\n',utils::STRIP_SPACES);
	SDL_Rect txt_area = { area.x+campaign_preview_border,area.y+campaign_preview_border,0,0 };

	for(std::vector<std::string>::const_iterator line = lines.begin(); line != lines.end(); ++line) {
	  txt_area = font::draw_text(&video(),location(),font::SIZE_SMALL,font::NORMAL_COLOUR,*line,txt_area.x,txt_area.y);
		txt_area.y += txt_area.h;
	}

	/* description image */
	surface img(NULL);
	const std::string desc_img_name = (*descriptions_)[index_].second;
	if(!desc_img_name.empty()) {
		img.assign(image::get_image(desc_img_name));
	}
	if (!img.null()) {
		SDL_Rect src_rect,dst_rect;
		int max_height = area.h-(txt_area.h+txt_area.y-area.y);

		src_rect.x = src_rect.y = 0;
		src_rect.w = minimum<int>(area.w,img->w);
		src_rect.h = minimum<int>(max_height,img->h);
		dst_rect.x = area.x+(area.w-src_rect.w)/2;
		dst_rect.y = txt_area.y+((max_height-src_rect.h)*8)/13;
		if(dst_rect.y - txt_area.h - txt_area.y >= 120) {
			//for really tall dialogs, just put it under the text
			dst_rect.y = txt_area.y + font::get_max_height(font::SIZE_SMALL)*5;
		}

		SDL_BlitSurface(img,&src_rect,video().getSurface(),&dst_rect);

	}
}

network::connection network_data_dialog(display& disp, const std::string& msg, config& cfg, network::connection connection_num, network::statistics (*get_stats)(network::connection handle))
{
#ifdef USE_TINY_GUI
	const size_t width = 200;
	const size_t height = 40;
	const size_t border = 10;
#else
	const size_t width = 300;
	const size_t height = 80;
	const size_t border = 20;
#endif
	const int left = disp.w()/2 - width/2;
	const int top  = disp.h()/2 - height/2;

	const events::event_context dialog_events_context;

	gui::button cancel_button(disp.video(),_("Cancel"));
	std::vector<gui::button*> buttons_ptr(1,&cancel_button);

	gui::dialog_frame frame(disp.video(), msg, gui::dialog_frame::default_style, false, &buttons_ptr);
	frame.layout(left,top,width,height);
	frame.draw();

	const SDL_Rect progress_rect = {left+border,top+border,width-border*2,height-border*2};
	gui::progress_bar progress(disp.video());
	progress.set_location(progress_rect);

	events::raise_draw_event();
	disp.flip();

	network::statistics old_stats = get_stats(connection_num);

	cfg.clear();
	for(;;) {
		const network::connection res = network::receive_data(cfg,connection_num,100);
		const network::statistics stats = get_stats(connection_num);
		if(stats.current_max != 0 && stats != old_stats) {
			old_stats = stats;
			progress.set_progress_percent((stats.current*100)/stats.current_max);
			std::ostringstream stream;
			stream << stats.current/1024 << "/" << stats.current_max/1024 << _("KB");
			progress.set_text(stream.str());
		}

		events::raise_draw_event();
		disp.flip();
		events::pump();
		
		if(res != 0) {
			return res;
		}


		if(cancel_button.pressed()) {
			return res;
		}
	}
}

network::connection network_send_dialog(display& disp, const std::string& msg, config& cfg, network::connection connection_num)
{
	return network_data_dialog(disp, msg, cfg, connection_num,
							   network::get_send_stats);
}

network::connection network_receive_dialog(display& disp, const std::string& msg, config& cfg, network::connection connection_num)
{
	return network_data_dialog(disp, msg, cfg, connection_num,
							   network::get_receive_stats);
}

} // end namespace dialogs

namespace {

class connect_waiter : public threading::waiter
{
public:
	connect_waiter(display& disp, gui::button& button) : disp_(disp), button_(button)
	{}
	ACTION process();

private:
	display& disp_;
	gui::button& button_;
};

connect_waiter::ACTION connect_waiter::process()
{
	events::raise_draw_event();
	disp_.flip();
	events::pump();
	if(button_.pressed()) {
		return ABORT;
	} else {
		return WAIT;
	}
}

}

namespace dialogs
{

network::connection network_connect_dialog(display& disp, const std::string& msg, const std::string& hostname, int port)
{
#ifdef USE_TINY_GUI
	const size_t width = 200;
	const size_t height = 20;
#else
	const size_t width = 250;
	const size_t height = 20;
#endif
	const int left = disp.w()/2 - width/2;
	const int top  = disp.h()/2 - height/2;

	const events::event_context dialog_events_context;

	gui::button cancel_button(disp.video(),_("Cancel"));
	std::vector<gui::button*> buttons_ptr(1,&cancel_button);

	gui::dialog_frame frame(disp.video(), msg, gui::dialog_frame::default_style, false, &buttons_ptr);
	frame.layout(left,top,width,height);
	frame.draw();

	events::raise_draw_event();
	disp.flip();

	connect_waiter waiter(disp,cancel_button);
	return network::connect(hostname,port,waiter);
}

} // end namespace dialogs
