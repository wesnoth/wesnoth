/* $Id$ */
/*
   Copyright (C) 2003 - 2011 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file dialogs.cpp
 * Various dialogs: advance_unit, show_objectives, save+load game, network::connection.
 */

#include "global.hpp"

#include "dialogs.hpp"
#include "foreach.hpp"
#include "game_events.hpp"
#include "game_display.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "help.hpp"
#include "language.hpp"
#include "log.hpp"
#include "map.hpp"
#include "map_exception.hpp"
#include "marked-up_text.hpp"
#include "menu_events.hpp"
#include "mouse_handler_base.hpp"
#include "minimap.hpp"
#include "replay.hpp"
#include "resources.hpp"
#include "savegame.hpp"
#include "thread.hpp"
#include "wml_separators.hpp"
#include "widgets/progressbar.hpp"
#include "wml_exception.hpp"
#include "formula_string_utils.hpp"
#include "gui/dialogs/game_save.hpp"
#include "gui/dialogs/transient_message.hpp"


//#ifdef _WIN32
//#include "locale.h"
//#endif

#include <clocale>

static lg::log_domain log_engine("engine");
#define LOG_NG LOG_STREAM(info, log_engine)
#define ERR_NG LOG_STREAM(err, log_engine)

static lg::log_domain log_display("display");
#define LOG_DP LOG_STREAM(info, log_display)

#define ERR_G  LOG_STREAM(err, lg::general)

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)

namespace dialogs
{

void advance_unit(const map_location &loc, bool random_choice, bool add_replay_event)
{
	unit_map::iterator u = resources::units->find(loc);
	if (u == resources::units->end() || u->second.advances() == false)
		return;

	LOG_DP << "advance_unit: " << u->second.type_id() << "\n";

	const std::vector<std::string>& options = u->second.advances_to();

	std::vector<std::string> lang_options;

	std::vector<unit> sample_units;
	for(std::vector<std::string>::const_iterator op = options.begin(); op != options.end(); ++op) {
		sample_units.push_back(::get_advanced_unit(u->second, *op));
		const unit& type = sample_units.back();

#ifdef LOW_MEM
		lang_options.push_back(IMAGE_PREFIX + type.absolute_image() + COLUMN_SEPARATOR + type.type_name());
#else
		lang_options.push_back(IMAGE_PREFIX + type.absolute_image() + u->second.image_mods() + COLUMN_SEPARATOR + type.type_name());
#endif
		preferences::encountered_units().insert(*op);
	}

	bool always_display = false;
	BOOST_FOREACH (const config &mod, u->second.get_modification_advances())
	{
		if (utils::string_bool(mod["always_display"])) always_display = true;
		sample_units.push_back(::get_advanced_unit(u->second, u->second.type_id()));
		sample_units.back().add_modification("advance", mod);
		const unit& type = sample_units.back();
		if (!mod["image"].empty()) {
			lang_options.push_back(IMAGE_PREFIX + mod["image"].str() + COLUMN_SEPARATOR + mod["description"].str());
		} else {
#ifdef LOW_MEM
			lang_options.push_back(IMAGE_PREFIX + type.absolute_image() + COLUMN_SEPARATOR + mod["description"].str());
#else
			lang_options.push_back(IMAGE_PREFIX + type.absolute_image() + u->second.image_mods() + COLUMN_SEPARATOR + mod["description"].str());
#endif
		}
	}

	LOG_DP << "options: " << options.size() << "\n";

	int res = 0;

	if(lang_options.empty()) {
		return;
	} else if(random_choice) {
		res = rand()%lang_options.size();
	} else if(lang_options.size() > 1 || always_display) {

		units_list_preview_pane unit_preview(sample_units);
		std::vector<gui::preview_pane*> preview_panes;
		preview_panes.push_back(&unit_preview);

		gui::dialog advances = gui::dialog(*resources::screen,
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
	animate_unit_advancement(loc, size_t(res));

	// In some rare cases the unit can have enough XP to advance again,
	// so try to do that.
	// Make sure that we don't enter an infinite level loop.
	u = resources::units->find(loc);
	if (u != resources::units->end()) {
		// Level 10 unit gives 80 XP and the highest mainline is level 5
		if(u->second.experience() < 81) {
			// For all leveling up we have to add advancement to replay here because replay
			// doesn't handle cascading advancement since it just calls animate_unit_advancement().
			advance_unit(loc, random_choice, true);
		} else {
			ERR_CF << "Unit has an too high amount of " << u->second.experience()
				<< " XP left, cascade leveling disabled\n";
		}
	} else {
		ERR_NG << "Unit advanced no longer exists\n";
	}
}

bool animate_unit_advancement(const map_location &loc, size_t choice)
{
	const events::command_disabler cmd_disabler;

	unit_map::iterator u = resources::units->find(loc);
	if (u == resources::units->end() || u->second.advances() == false) {
		return false;
	}

	const std::vector<std::string>& options = u->second.advances_to();
	std::vector<config> mod_options = u->second.get_modification_advances();

	if(choice >= options.size() + mod_options.size()) {
		return false;
	}

	// When the unit advances, it fades to white, and then switches
	// to the new unit, then fades back to the normal colour

	if (!resources::screen->video().update_locked()) {
		unit_animator animator;
		bool with_bars = true;
		animator.add_animation(&u->second,"levelout", u->first, map_location(), 0, with_bars);
		animator.start_animations();
		animator.wait_for_end();
	}

	if(choice < options.size()) {
		// chosen_unit is not a reference, since the unit may disappear at any moment.
		std::string chosen_unit = options[choice];
		::advance_unit(loc, chosen_unit);
	} else {
		unit amla_unit(u->second);
		const config &mod_option = mod_options[choice - options.size()];

		LOG_NG << "firing advance event (AMLA)\n";
		game_events::fire("advance",loc);

		amla_unit.get_experience(-amla_unit.max_experience()); // subtract xp required
		amla_unit.add_modification("advance",mod_option);
		resources::units->replace(loc, amla_unit);

		LOG_NG << "firing post_advance event (AMLA)\n";
		game_events::fire("post_advance",loc);
	}

	u = resources::units->find(loc);
	resources::screen->invalidate_unit();

	if (u != resources::units->end() && !resources::screen->video().update_locked()) {
		unit_animator animator;
		animator.add_animation(&u->second,"levelin",u->first, map_location(), 0, true);
		animator.start_animations();
		animator.wait_for_end();
		animator.set_all_standing();
		resources::screen->invalidate(loc);
		resources::screen->draw();
		events::pump();
	}

	resources::screen->invalidate_all();
	resources::screen->draw();

	return true;
}

void show_objectives(const config &level, const std::string &objectives)
{
	static const std::string no_objectives(_("No objectives available"));
	gui2::show_transient_message(resources::screen->video(), level["name"],
		(objectives.empty() ? no_objectives : objectives), true);
}

namespace {

/** Class to handle deleting a saved game. */
class delete_save : public gui::dialog_button_action
{
public:
	delete_save(display& disp, gui::filter_textbox& filter, std::vector<savegame::save_info>& saves, std::vector<config*>& save_summaries) : disp_(disp), saves_(saves), summaries_(save_summaries), filter_(filter) {}
private:
	gui::dialog_button_action::RESULT button_pressed(int menu_selection);

	display& disp_;
	std::vector<savegame::save_info>& saves_;
	std::vector<config*>& summaries_;
	gui::filter_textbox& filter_;
};

gui::dialog_button_action::RESULT delete_save::button_pressed(int menu_selection)
{
	const size_t index = size_t(filter_.get_index(menu_selection));
	if(index < saves_.size()) {

		// See if we should ask the user for deletion confirmation
		if(preferences::ask_delete_saves()) {
			gui::dialog dmenu(disp_,"",
					       _("Do you really want to delete this game?"),
					       gui::YES_NO);
			dmenu.add_option(_("Don't ask me again!"), false);
			const int res = dmenu.show();
			// See if the user doesn't want to be asked this again
			if(dmenu.option_checked()) {
				preferences::set_ask_delete_saves(false);
			}

			if(res != 0) {
				return gui::CONTINUE_DIALOG;
			}
		}

		// Remove the item from filter_textbox memory
		filter_.delete_item(menu_selection);

		// Delete the file
		savegame::manager::delete_game(saves_[index].name);

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
	save_preview_pane(CVideo &video, const config& game_config, gamemap* map,
			const std::vector<savegame::save_info>& info,
			const std::vector<config*>& summaries,
			const gui::filter_textbox& textbox) :
		gui::preview_pane(video),
		game_config_(&game_config),
		map_(map), info_(&info),
		summaries_(summaries),
		index_(0),
		map_cache_(),
		textbox_(textbox)
	{
		set_measurements(std::min<int>(200,video.getx()/4),
				 std::min<int>(400,video.gety() * 4/5));
	}

	void draw_contents();
	void set_selection(int index) {
		index_ = textbox_.get_index(index);
		set_dirty();
	}

	bool left_side() const { return true; }

private:
	const config* game_config_;
	gamemap* map_;
	const std::vector<savegame::save_info>* info_;
	const std::vector<config*>& summaries_;
	int index_;
	std::map<std::string,surface> map_cache_;
	const gui::filter_textbox& textbox_;
};

void save_preview_pane::draw_contents()
{
	if (size_t(index_) >= summaries_.size() || info_->size() != summaries_.size()) {
		return;
	}

	std::string dummy;
	config& summary = *summaries_[index_];
	if (summary["label"] == ""){
		try {
			savegame::manager::load_summary((*info_)[index_].name, summary, &dummy);
			*summaries_[index_] = summary;
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

	const unit_type *leader = unit_types.find(summary["leader"]);
	if (leader)
	{
#ifdef LOW_MEM
		const surface image(image::get_image(leader->image()));
#else
		const surface image(image::get_image(leader->image() + "~RC(" + leader->flag_rgb() + ">1)"));
#endif

		if(image != NULL) {
			SDL_Rect image_rect = {area.x,area.y,image->w,image->h};
			ypos += image_rect.h + save_preview_border;

			SDL_BlitSurface(image,NULL,screen,&image_rect);
		}
	}

	std::string map_data = summary["map_data"];
	if(map_data.empty()) {
		const config &scenario = game_config_->find_child(summary["campaign_type"], "id", summary["scenario"]);
		if (scenario && !scenario.find_child("side", "shroud", "yes")) {
			map_data = scenario["map_data"];
			if (map_data.empty() && !scenario["map"].empty()) {
				try {
					map_data = read_map(scenario["map"]);
				} catch(io_exception& e) {
					ERR_G << "could not read map '" << scenario["map"] << "': " << e.what() << '\n';
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
			} catch(incorrect_map_format_exception& e) {
				ERR_CF << "map could not be loaded: " << e.msg_ << '\n';
			} catch(twml_exception& e) {
				ERR_CF << "map could not be loaded: " << e.dev_message << '\n';
			}
		}
	}

	if(map_surf != NULL) {
		SDL_Rect map_rect = {area.x + area.w - map_surf->w,area.y,map_surf->w,map_surf->h};
		ypos = std::max<int>(ypos,map_rect.y + map_rect.h + save_preview_border);
		SDL_BlitSurface(map_surf,NULL,screen,&map_rect);
	}

	char time_buf[256] = {0};
	const savegame::save_info& save = (*info_)[index_];
	tm* tm_l = localtime(&save.time_modified);
	if (tm_l) {
		const size_t res = strftime(time_buf,sizeof(time_buf),_("%a %b %d %H:%M %Y"),tm_l);
		if(res == 0) {
			time_buf[0] = 0;
		}
	} else {
		LOG_NG << "localtime() returned null for time " << save.time_modified << ", save " << save.name;
	}

	std::stringstream str;

	str << font::BOLD_TEXT << font::NULL_MARKUP
		<< (*info_)[index_].name << '\n' << time_buf;

	const std::string& campaign_type = summary["campaign_type"];
	if(utils::string_bool(summary["corrupt"], false)) {
		str << "\n" << _("#(Invalid)");
	} else if (!campaign_type.empty()) {
		str << "\n";

		if(campaign_type == "scenario") {
			const std::string campaign_id = summary["campaign"];
			const config *campaign = NULL;
			if (!campaign_id.empty()) {
				if (const config &c = game_config_->find_child("campaign", "id", campaign_id))
					campaign = &c;
			}
			utils::string_map symbols;
			if (campaign != NULL) {
				symbols["campaign_name"] = (*campaign)["name"];
			} else {
				// Fallback to nontranslatable campaign id.
				symbols["campaign_name"] = "(" + campaign_id + ")";
			}
			str << vgettext("Campaign: $campaign_name", symbols);

			// Display internal id for debug purposes if we didn't above
			if (game_config::debug && (campaign != NULL)) {
				str << '\n' << "(" << campaign_id << ")";
			}
		} else if(campaign_type == "multiplayer") {
			str << _("Multiplayer");
		} else if(campaign_type == "tutorial") {
			str << _("Tutorial");
		} else {
			str << campaign_type;
		}

		str << "\n";

		if(utils::string_bool(summary["replay"], false) && !utils::string_bool(summary["snapshot"], true)) {
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

std::string load_game_dialog(display& disp, const config& game_config, bool* show_replay, bool* cancel_orders)
{
	std::vector<savegame::save_info> games;
	{
		cursor::setter cur(cursor::WAIT);
		games = savegame::manager::get_saves_list();
	}

	if(games.empty()) {
		gui2::show_transient_message(disp.video(),
		                 _("No Saved Games"),
				 _("There are no saved games to load.\n\n(Games are saved automatically when you complete a scenario)"));
		return "";
	}

	std::vector<config*> summaries;
	std::vector<savegame::save_info>::const_iterator i;
	//FIXME: parent_to_child is not used yet
	std::map<std::string,std::string> parent_to_child;
	for(i = games.begin(); i != games.end(); ++i) {
		config& cfg = savegame::save_index::save_summary(i->name);
		parent_to_child[cfg["parent"]] = i->name;
		summaries.push_back(&cfg);
	}

	const events::event_context context;

	std::vector<std::string> items;
	std::ostringstream heading;
	heading << HEADING_PREFIX << _("Name") << COLUMN_SEPARATOR << _("Date");
	items.push_back(heading.str());

	for(i = games.begin(); i != games.end(); ++i) {
		std::string name = i->name;
		utils::truncate_as_wstring(name, std::min<size_t>(name.size(), 40));

		std::ostringstream str;
		str << name << COLUMN_SEPARATOR << format_time_summary(i->time_modified);

		items.push_back(str.str());
	}

	gamemap map_obj(game_config, "");


	gui::dialog lmenu(disp,
			  _("Load Game"),
			  _("Choose the game to load"), gui::NULL_DIALOG);
	lmenu.set_basic_behavior(gui::OK_CANCEL);

	gui::menu::basic_sorter sorter;
	sorter.set_alpha_sort(0).set_id_sort(1);
	lmenu.set_menu(items, &sorter);

	gui::filter_textbox* filter = new gui::filter_textbox(disp.video(), _("Filter: "), items, items, 1, lmenu);
	lmenu.set_textbox(filter);

	save_preview_pane save_preview(disp.video(),game_config,&map_obj,games,summaries,*filter);
	lmenu.add_pane(&save_preview);
	// create an option for whether the replay should be shown or not
	if(show_replay != NULL) {
		lmenu.add_option(_("Show replay"), false,
			game_config::small_gui ? gui::dialog::BUTTON_CHECKBOX : gui::dialog::BUTTON_STANDARD);
	}
	if(cancel_orders != NULL) {
		lmenu.add_option(_("Cancel orders"), false,
			game_config::small_gui ? gui::dialog::BUTTON_STANDARD : gui::dialog::BUTTON_EXTRA);
	}
	lmenu.add_button(new gui::standard_dialog_button(disp.video(),_("OK"),0,false), gui::dialog::BUTTON_STANDARD);
	lmenu.add_button(new gui::standard_dialog_button(disp.video(),_("Cancel"),1,true), gui::dialog::BUTTON_STANDARD);

	delete_save save_deleter(disp,*filter,games,summaries);
	gui::dialog_button_info delete_button(&save_deleter,_("Delete Save"));

	lmenu.add_button(delete_button,
		game_config::small_gui ? gui::dialog::BUTTON_HELP : gui::dialog::BUTTON_EXTRA);

	int res = lmenu.show();

	savegame::save_index::write_save_index();

	if(res == -1)
		return "";

	res = filter->get_index(res);
	int option_index = 0;
	if(show_replay != NULL) {
	  *show_replay = lmenu.option_checked(option_index++);

		const config& summary = *summaries[res];
		if(utils::string_bool(summary["replay"], false) && !utils::string_bool(summary["snapshot"], true)) {
			*show_replay = true;
		}
	}
	if (cancel_orders != NULL) {
		*cancel_orders = lmenu.option_checked(option_index++);
	}

	return games[res].name;
}

namespace {
	static const int unit_preview_border = 10;
}

unit_preview_pane::details::details() :
	image(),
	name(),
	type_name(),
	race(),
	level(0),
	alignment(),
	traits(),
	abilities(),
	hitpoints(0),
	max_hitpoints(0),
	experience(0),
	max_experience(0),
	hp_color(),
	xp_color(),
	movement_left(0),
	total_movement(0),
	attacks()
{
}

unit_preview_pane::unit_preview_pane(const gui::filter_textbox *filter, TYPE type, bool on_left_side) :
	gui::preview_pane(resources::screen->video()), index_(0),
	details_button_(resources::screen->video(), _("Profile"),
				      gui::button::TYPE_PRESS,"lite_small", gui::button::MINIMUM_SPACE),
				      filter_(filter), weapons_(type == SHOW_ALL), left_(on_left_side)
{
	unsigned w = font::relative_size(weapons_ ? 200 : 190);
// advance test
	unsigned h = font::relative_size(weapons_ ? 440 : 140);
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
	index = std::min<int>(static_cast<int>(size())-1,index);
	if (filter_) {
		index = filter_->get_index(index);
	}
	if(index != index_) {
		index_ = index;
		set_dirty();
		if (index >= 0) {
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
	const SDL_Rect button_loc = {right_align ? area.x : area.x + area.w - details_button_.location().w,
		                             image_rect.y + image_rect.h,
		                             details_button_.location().w,details_button_.location().h};
	details_button_.set_location(button_loc);

	SDL_Rect description_rect = {image_rect.x,image_rect.y+image_rect.h+details_button_.location().h,0,0};

	if(det.name.empty() == false) {
		std::stringstream desc;
		desc << font::BOLD_TEXT << det.name;
		const std::string description = desc.str();
		description_rect = font::text_area(description, font::SIZE_NORMAL);
		description_rect = font::draw_text(&video(), area,
							font::SIZE_NORMAL, font::NORMAL_COLOUR,
							desc.str(), right_align ?  image_rect.x :
							image_rect.x + image_rect.w - description_rect.w,
							image_rect.y + image_rect.h + details_button_.location().h);
	}

	std::stringstream text;
	text << font::unit_type << det.type_name << "\n"
		<< font::race
		<< (right_align && !weapons_ ? det.race+"  " : "  "+det.race) << "\n"
		<< _("level") << " " << det.level << "\n"
		<< det.alignment << "\n"
		<< det.traits << "\n";

	for(std::vector<t_string>::const_iterator a = det.abilities.begin(); a != det.abilities.end(); ++a) {
		if(a != det.abilities.begin()) {
			text << ", ";
		}
		text << (*a);
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
			//at_it->set_specials_context(map_location(),u);

			// see generate_report() in generate_report.cpp
			text << font::weapon << at_it->name()
				<< " (" << gettext(at_it->type().c_str()) << ")\n";

			std::string special = at_it->weapon_specials(true);
			if (!special.empty()) {
				text << font::weapon_details << "  " << special << "\n";
			}
			std::string accuracy = at_it->accuracy_parry_description();
			if(accuracy.empty() == false) {
				accuracy += " ";
			}

			text << font::weapon_details << "  " << at_it->damage() << "-" << at_it->num_attacks()
				<< " " << accuracy << "-- " << _(at_it->range().c_str()) << "\n";
		}
	}

	// we don't remove empty lines, so all fields stay at the same place
	const std::vector<std::string> lines = utils::split(text.str(), '\n',
		utils::STRIP_SPACES & !utils::REMOVE_EMPTY);


	int ypos = area.y;

	if(weapons_) {
		ypos += image_rect.h + description_rect.h + details_button_.location().h;
	}

	for(std::vector<std::string>::const_iterator line = lines.begin(); line != lines.end(); ++line) {
		int xpos = area.x;
		if(right_align && !weapons_) {
			const SDL_Rect& line_area = font::text_area(*line,font::SIZE_SMALL);
			// right align, but if too long, don't hide line's beginning
			if (line_area.w < area.w)
				xpos = area.x + area.w - line_area.w;
		}

		SDL_Rect cur_area = font::draw_text(&video(),location(),font::SIZE_SMALL,font::NORMAL_COLOUR,*line,xpos,ypos);
		ypos += cur_area.h;
	}
}

units_list_preview_pane::units_list_preview_pane(const unit &u, TYPE type, bool on_left_side) :
	unit_preview_pane(NULL, type, on_left_side),
	units_(&unit_store_),
	unit_store_(1, u)
{
}

units_list_preview_pane::units_list_preview_pane(std::vector<unit> &units,
		const gui::filter_textbox* filter, TYPE type, bool on_left_side) :
	unit_preview_pane(filter, type, on_left_side),
	units_(&units),
	unit_store_()
{
}

size_t units_list_preview_pane::size() const
{
	return (units_!=NULL) ? units_->size() : 0;
}

const unit_preview_pane::details units_list_preview_pane::get_details() const
{
	unit& u = (*units_)[index_];
	details det;

	det.image = u.still_image();

	det.name = u.name();
	det.type_name = u.type_name();
	if(u.race() != NULL) {
		det.race = u.race()->name(u.gender());
	}
	det.level = u.level();
	det.alignment = unit_type::alignment_description(u.alignment(), u.gender());
	det.traits = u.traits_description();

	//we filter to remove the tooltips (increment by 2)
	const std::vector<std::string> &abilities = u.ability_tooltips(true);
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
	if (details_button_.pressed() && index_ >= 0 && index_ < int(size())) {
		show_unit_description((*units_)[index_]);
	}
}

unit_types_preview_pane::unit_types_preview_pane(
					std::vector<const unit_type*>& unit_types, const gui::filter_textbox* filter,
					int side, TYPE type, bool on_left_side)
	: unit_preview_pane(filter, type, on_left_side),
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

    //FIXME: There should be a better way to deal with this
	unit_types.find(t->id(), unit_type::WITHOUT_ANIMATIONS);

	std::string mod = "~RC(" + t->flag_rgb() + ">" + team::get_side_colour_index(side_) + ")";
	det.image = image::get_image(t->image()+mod);

	det.name = "";
	det.type_name = t->type_name();
	det.level = t->level();
	det.alignment = unit_type::alignment_description(t->alignment(), t->genders().front());

	if (const unit_race *r = unit_types.find_race(t->race())) {
		assert(!t->genders().empty());
		det.race = r->name(t->genders().front());
	}

	//FIXME: This probably must be move into a unit_type function
	BOOST_FOREACH (const config &tr, t->possible_traits())
	{
		if (tr["availability"] != "musthave") continue;
		std::string gender_string = (!t->genders().empty() && t->genders().front()== unit_race::FEMALE) ? "female_name" : "male_name";
		t_string name = tr[gender_string];
		if (name.empty()) {
			name = tr["name"];
		}
		if (!name.empty()) {
			if (!det.traits.empty()) {
				det.traits += ", ";
			}
			det.traits += name;
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
	BOOST_FOREACH (const config &adv, t->modification_advancements())
	{
		if (!utils::string_bool(adv["strict_amla"]) || !t->can_advance()) {
			det.xp_color = "<170,0,255>"; // from unit::xp_color()
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
	if (details_button_.pressed() && index_ >= 0 && index_ < int(size())) {
		const unit_type* type = (*unit_types_)[index_];
		if (type != NULL)
			show_unit_description(*type);
	}
}


void show_unit_description(const unit &u)
{
	const unit_type* t = u.type();
	if (t != NULL)
		show_unit_description(*t);
	else
		// can't find type, try open the id page to have feedback and unit error page
	  help::show_unit_help(*resources::screen, u.type_id());
}

void show_unit_description(const unit_type &t)
{
	help::show_unit_help(*resources::screen, t.id(), t.hide_help());
}

static network::connection network_data_dialog(display& disp, const std::string& msg, config& cfg, network::connection connection_num, network::statistics (*get_stats)(network::connection handle))
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

	gui::dialog_frame frame(disp.video(), msg, gui::dialog_frame::default_style, true, &buttons_ptr);
	SDL_Rect centered_layout = frame.layout(left,top,width,height).interior;
	centered_layout.x = disp.w() / 2 - centered_layout.w / 2;
	centered_layout.y = disp.h() / 2 - centered_layout.h / 2;
	// HACK: otherwise we get an empty useless space in the dialog below the progressbar
	centered_layout.h = height;
	frame.layout(centered_layout);
	frame.draw();

	const SDL_Rect progress_rect = {centered_layout.x+border,centered_layout.y+border,centered_layout.w-border*2,centered_layout.h-border*2};
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

	gui::dialog_frame frame(disp.video(), msg, gui::dialog_frame::default_style, true, &buttons_ptr);
	frame.layout(left,top,width,height);
	frame.draw();

	events::raise_draw_event();
	disp.flip();

	connect_waiter waiter(disp,cancel_button);
	return network::connect(hostname,port,waiter);
}

} // end namespace dialogs
