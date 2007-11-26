/* $Id$ */
/*
   Copyright (C) 2006 - 2007 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
   wesnoth playturn Copyright (C) 2003 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file menu_events.cpp
//! Operations activated from menus/hotkeys while playing a game.
//! E.g. Unitlist, status_table, save_game, save_map, chat, show_help, etc.

#include "global.hpp"

#include "construct_dialog.hpp"
#include "dialogs.hpp"
#include "game_display.hpp"
#include "game_config.hpp"
#include "game_errors.hpp"
#include "game_events.hpp"
#include "gettext.hpp"
#include "help.hpp"
#include "marked-up_text.hpp"
#include "menu_events.hpp"
#include "preferences_display.hpp"
#include "replay.hpp"
#include "sound.hpp"
#include "team.hpp"
#include "unit_display.hpp"
#include "unit_types.hpp"
#include "wassert.hpp"
#include "wml_separators.hpp"
#include "wesconfig.h"
#include "util.hpp"

#include <algorithm>
#include <sstream>

namespace {

static void remove_old_saves()
{
	int countdown = preferences::savemax();
	if (countdown == preferences::INFINITE_SAVES)
		return;

	std::vector<save_info> games = get_saves_list();
	std::cerr << "Removing old saves.\n";
	for (std::vector<save_info>::iterator i = games.begin(); i != games.end(); i++) {
		if (countdown-- < 0) {
			std::cerr << "Deleting savegame '" << i->name << "'\n";
			delete_game(i->name);
		}
	}
}

std::vector<std::string> create_unit_table(const game_data& gameinfo, const statistics::stats::str_int_map& m, unsigned int team)
{
	std::vector<std::string> table;
	for(statistics::stats::str_int_map::const_iterator i = m.begin(); i != m.end(); ++i) {
		const game_data::unit_type_map::const_iterator type = gameinfo.unit_types.find(i->first);
		if(type == gameinfo.unit_types.end()) {
			continue;
		}

		std::stringstream str;

		str << IMAGE_PREFIX << type->second.image();
#ifndef LOW_MEM
		str << "~RC(" << type->second.flag_rgb() << ">" << team << ")";
#endif
		str << COLUMN_SEPARATOR	<< type->second.language_name() << COLUMN_SEPARATOR << i->second << "\n";
		table.push_back(str.str());
	}

	return table;
}

class statistics_dialog : public gui::dialog
{
public:
	statistics_dialog(game_display &disp, const std::string& title, const game_data& gameinfo, const unsigned int team,
		const std::string& player);
	~statistics_dialog();
protected:
	void action(gui::dialog_process_info &dp_info);
private:
	gui::dialog_button *detail_btn_;
	const game_data& gameinfo_;
	std::string player_name_;
	statistics::stats stats_;
	unsigned int team_num_;
	std::vector<int> unit_count_;
};

void statistics_dialog::action(gui::dialog_process_info &dp_info)
{
	int sel = get_menu().selection();
	bool has_details = sel < 5 && sel >= 0 && unit_count_[sel] > 0;
	detail_btn_->enable(has_details);
	if(dp_info.double_clicked && has_details) {
		set_result(sel);
	} else if(dp_info.new_key_down && !dp_info.key_down) {
		set_result(gui::CLOSE_DIALOG);
	}

	// Prepare the sub-dialog for Statistic Details
	std::string title;
	std::vector<std::string> items_sub;
	switch(result()) {
	case gui::CLOSE_DIALOG:
		break;
	case 0:
		items_sub = create_unit_table(gameinfo_, stats_.recruits, team_num_);
		title = _("Recruits");
		break;
	case 1:
		items_sub = create_unit_table(gameinfo_, stats_.recalls, team_num_);
		title = _("Recalls");
		break;
	case 2:
		items_sub = create_unit_table(gameinfo_, stats_.advanced_to, team_num_);
		title = _("Advancements");
		break;
	case 3:
		items_sub = create_unit_table(gameinfo_, stats_.deaths, team_num_);
		title = _("Losses");
		break;
	case 4:
		items_sub = create_unit_table(gameinfo_, stats_.killed, team_num_);
		//! @todo FIXME? Perhaps killed units shouldn't have the same team-color as your own.
		title = _("Kills");
		break;
	default:
		break;
	}
	if (items_sub.empty() == false) {
		gui::dialog d(get_display(), title + " (" + player_name_ + ")", "", gui::CLOSE_ONLY);
		d.set_menu(items_sub);
		d.show();
		dp_info.clear_buttons();
		set_result(gui::CONTINUE_DIALOG);
	}
}

statistics_dialog::statistics_dialog(game_display &disp, const std::string& title,
const game_data& gameinfo, const unsigned int team, const std::string& player)
: dialog(disp, title, "", gui::NULL_DIALOG), gameinfo_(gameinfo), player_name_(player),
team_num_(team), unit_count_(5,0)
{
	detail_btn_ = new gui::standard_dialog_button(disp.video(), _("Details"), 0 , false);
	add_button(detail_btn_, gui::dialog::BUTTON_EXTRA);
	add_button(new gui::standard_dialog_button(disp.video(), _("Close"), 1, true),
				gui::dialog::BUTTON_STANDARD);

	stats_ = statistics::calculate_stats(0, team_num_);
	int n;
	std::vector<std::string> items;
	// Prepare the menu items
	{
		std::stringstream str;
		n = statistics::sum_str_int_map(stats_.recruits);
		unit_count_[0] = n;
		str << _("Recruits") << COLUMN_SEPARATOR << n;
		items.push_back(str.str());
	}

	{
		std::stringstream str;
		n = statistics::sum_str_int_map(stats_.recalls);
		unit_count_[1] = n;
		str << _("Recalls") << COLUMN_SEPARATOR << n;
		items.push_back(str.str());
	}

	{
		std::stringstream str;
		n = statistics::sum_str_int_map(stats_.advanced_to);
		unit_count_[2] = n;
        str << font::BOLD_TEXT << _("Advancements") << COLUMN_SEPARATOR
            << font::BOLD_TEXT << n;
		items.push_back(str.str());
	}

	{
		std::stringstream str;
		n = statistics::sum_str_int_map(stats_.deaths);
		unit_count_[3] = n;
		str << _("Losses") << COLUMN_SEPARATOR << n;
		items.push_back(str.str());
	}

	{
		std::stringstream str;
		n = statistics::sum_str_int_map(stats_.killed);
		unit_count_[4] = n;
		str << _("Kills") << COLUMN_SEPARATOR << n;
		items.push_back(str.str());
	}

	{
		std::stringstream str;
        str << font::BOLD_TEXT << _("Damage Inflicted") << COLUMN_SEPARATOR
            << font::BOLD_TEXT << stats_.damage_inflicted;
		items.push_back(str.str());
	}

	{
		std::stringstream str;
		str << _("Damage Taken") << COLUMN_SEPARATOR << stats_.damage_taken;
		items.push_back(str.str());
	}

	{
		std::stringstream str;
        str << font::BOLD_TEXT << _("Damage Inflicted (EV)") << COLUMN_SEPARATOR
            << font::BOLD_TEXT << (stats_.expected_damage_inflicted / 100.0);
		items.push_back(str.str());
	}

	{
		std::stringstream str;
		str << _("Damage Taken (EV)") <<  COLUMN_SEPARATOR
			<< (stats_.expected_damage_taken / 100.0);
		items.push_back(str.str());
	}
	set_menu(items);
}

statistics_dialog::~statistics_dialog()
{
}

} // end anonymous namespace

namespace events{

	class delete_recall_unit : public gui::dialog_button_action
	{
	public:
		delete_recall_unit(game_display& disp, std::vector<unit>& units) : disp_(disp), units_(units) {}
	private:
		gui::dialog_button_action::RESULT button_pressed(int menu_selection);

		game_display& disp_;
		std::vector<unit>& units_;
	};

	gui::dialog_button_action::RESULT delete_recall_unit::button_pressed(int menu_selection)
	{
		const size_t index = size_t(menu_selection);
		if(index < units_.size()) {
			const unit& u = units_[index];

			//If the unit is of level > 1, or is close to advancing,
			//we warn the player about it
			std::stringstream message;
			if(u.level() > 1) {
				message << _("My lord, this unit is an experienced one, having advanced levels! ") << (u.gender() == unit_race::MALE ? _("Do you really want to dismiss him?")
						: _("Do you really want to dismiss her?"));

			} else if(u.experience() > u.max_experience()/2) {
				message << _("My lord, this unit is close to advancing a level! ") << (u.gender() == unit_race::MALE ? _("Do you really want to dismiss him?")
						: _("Do you really want to dismiss her?"));
			}

			if(!message.str().empty()) {
				const int res = gui::dialog(disp_,"",message.str(),gui::YES_NO).show();
				if(res != 0) {
					return gui::CONTINUE_DIALOG;
				}
			}

			units_.erase(units_.begin() + index);
			recorder.add_disband(index);
			return gui::DELETE_ITEM;
		} else {
			return gui::CONTINUE_DIALOG;
		}
	}

	static bool is_illegal_file_char(char c)
	{
		return c == '/' || c == '\\' || c == ':';
	}

	menu_handler::menu_handler(game_display* gui, unit_map& units, std::vector<team>& teams,
		const config& level, const game_data& gameinfo, const gamemap& map,
		const config& game_config, const gamestatus& status, game_state& gamestate,
		undo_list& undo_stack, undo_list& redo_stack) :
	gui_(gui), units_(units), teams_(teams), level_(level), gameinfo_(gameinfo), map_(map),
		game_config_(game_config), status_(status), gamestate_(gamestate), undo_stack_(undo_stack),
		redo_stack_(redo_stack)
	{
	}

	menu_handler::~menu_handler()
	{
	}
	const undo_list& menu_handler::get_undo_list() const{
		 return undo_stack_;
	}

	gui::floating_textbox& menu_handler::get_textbox(){
		return textbox_info_;
	}

	void menu_handler::objectives(const unsigned int team_num)
	{
		dialogs::show_objectives(*gui_, level_, teams_[team_num - 1].objectives());
		teams_[team_num - 1].reset_objectives_changed();
	}

	void menu_handler::show_statistics(const unsigned int team_num)
	{
		// Current Player name
		const std::string player = teams_[team_num - 1].current_player();
		//add player's name to title of dialog
		std::stringstream title_str;
		title_str <<  _("Statistics") << " (" << player << ")";
		statistics_dialog stats_dialog(*gui_, title_str.str(), gameinfo_, team_num, player);
		stats_dialog.show();
	}

	void menu_handler::unit_list()
	{
		const std::string heading = std::string(1,HEADING_PREFIX) +
									_("Type")          + COLUMN_SEPARATOR +
									_("Name")          + COLUMN_SEPARATOR +
									_("Level^Lv.")     + COLUMN_SEPARATOR +
									_("HP")            + COLUMN_SEPARATOR +
									_("XP")            + COLUMN_SEPARATOR +
									_("unit list^Traits") + COLUMN_SEPARATOR +
									_("Moves")         + COLUMN_SEPARATOR +
									_("Location^Loc.") + COLUMN_SEPARATOR +
									_("Status");

		gui::menu::basic_sorter sorter;
		sorter.set_alpha_sort(0).set_alpha_sort(1).set_numeric_sort(2).set_numeric_sort(3)
			  .set_alpha_sort(4).set_numeric_sort(5).set_numeric_sort(6);

		std::vector<std::string> items;
		items.push_back(heading);

		std::vector<gamemap::location> locations_list;
		std::vector<unit> units_list;

		int selected = 0;

		for(unit_map::const_iterator i = units_.begin(); i != units_.end(); ++i) {
			if(i->second.side() != (gui_->viewing_team()+1))
				continue;

			std::stringstream row;
			// If a unit is already selected on the map, we do the same in the unit list dialog
			if (gui_->selected_hex() == i->first) {
				 row << DEFAULT_ITEM;
				 selected = units_list.size();
			}
//%%
			// If unit is leader, show name in special color, e.g. gold/silver
			//! @todo TODO: hero just has overlay "misc/hero-icon.png" - needs an ability to query

			if(i->second.can_recruit() ) {
//				row << "<255,255,200>";
                row << "<205,173,0>";   // gold3
			}
			row << i->second.language_name() << COLUMN_SEPARATOR;
			if(i->second.can_recruit() ) {
//				row << "<255,255,200>";
                row << "<205,173,0>";   // gold3
			}
			row << i->second.description()   << COLUMN_SEPARATOR;

			// Show units of level (0=gray, 1 normal, 2 bold, 2+ bold&wbright)
			const int level = i->second.level();
			if(level < 1) {
				row << "<150,150,150>";
			} else if(level == 1) {
				row << font::NORMAL_TEXT;
			} else if(level == 2) {
				row << font::BOLD_TEXT;
			} if(i->second.level() > 2 ) {
				row << font::BOLD_TEXT << "<255,255,255>";
			}
			row << level << COLUMN_SEPARATOR;

			// Display HP
			// see also unit_preview_pane in dialogs.cpp
			row << font::color2markup(i->second.hp_color());
			row << i->second.hitpoints()  << "/" << i->second.max_hitpoints() << COLUMN_SEPARATOR;

			// Display XP
			row << font::color2markup(i->second.xp_color());
			row << i->second.experience() << "/";
			if(i->second.can_advance()) {
				row << i->second.max_experience();
			} else {
				row << "-";
			}
			row << COLUMN_SEPARATOR;

			// TODO: show 'loyal' in green / xxx in red  //  how to handle translations ??
			row << i->second.traits_description() << COLUMN_SEPARATOR;

			// display move left (0=red, moved=yellow, not moved=green)
			if(i->second.movement_left() == 0) {
				row << font::RED_TEXT;
			} else if(i->second.movement_left() < i->second.total_movement() ) {
				row << "<255,255,0>";
			} else {
				row << font::GREEN_TEXT;
			}
			row << i->second.movement_left() << "/" << i->second.total_movement() << COLUMN_SEPARATOR;

			const int def =  100 - i->second.defense_modifier(map_.get_terrain(i->first));
			int val = (game_config::defense_color_scale.size()-1) * def/100;
			row << rgb2highlight(game_config::defense_color_scale[val]);
			row << i->first << COLUMN_SEPARATOR;

			// show icons if unit is slowed, poisoned, stoned, invisible:
			if(utils::string_bool(i->second.get_state("stoned")))
				row << IMAGE_PREFIX << "misc/stone.png"    << IMG_TEXT_SEPARATOR;
			if(utils::string_bool(i->second.get_state("slowed")))
				row << IMAGE_PREFIX << "misc/slowed.png"   << IMG_TEXT_SEPARATOR;
			if(utils::string_bool(i->second.get_state("poisoned")))
				row << IMAGE_PREFIX << "misc/poisoned.png" << IMG_TEXT_SEPARATOR;

			//! @todo FIXME: condition for "invisible" does not work
			//if(utils::string_bool(i->second.get_state("hides")))	// "hides" gives ability, not status
			if(utils::string_bool(i->second.get_state("invisible")))
				row << IMAGE_PREFIX << "misc/invisible.png";
//%%
			items.push_back(row.str());

			locations_list.push_back(i->first);
			units_list.push_back(i->second);
		}

		{
			dialogs::units_list_preview_pane unit_preview(*gui_, &map_, units_list);
			unit_preview.set_selection(selected);

			gui::dialog umenu(*gui_, _("Unit List"), "", gui::NULL_DIALOG);
			umenu.set_menu(items, &sorter);
			umenu.add_pane(&unit_preview);
			umenu.add_button(new gui::standard_dialog_button(gui_->video(), _("Scroll To"), 0, false),
			                 gui::dialog::BUTTON_STANDARD);
			umenu.add_button(new gui::standard_dialog_button(gui_->video(), _("Close"), 1, true),
			                 gui::dialog::BUTTON_STANDARD);
			umenu.set_basic_behavior(gui::OK_CANCEL);
			selected = umenu.show();
		} // this will kill the dialog before scrolling

		if(selected >= 0 && selected < int(locations_list.size())) {
			const gamemap::location& loc = locations_list[selected];
			gui_->scroll_to_tile(loc,game_display::WARP);
			gui_->select_hex(loc);
		}
	}

namespace {
class leader_scroll_dialog : public gui::dialog {
public:
	leader_scroll_dialog(display &disp, const std::string &title, std::vector<bool> &leader_bools, int selected, gui::DIALOG_RESULT extra_result)
	: dialog(disp, title, "", gui::NULL_DIALOG), leader_bools_(leader_bools), extra_result_(extra_result)
	{
		scroll_btn_ = new gui::standard_dialog_button(disp.video(), _("Scroll To"), 0, false);
		scroll_btn_->enable(leader_bools[selected]);
		add_button(scroll_btn_, gui::dialog::BUTTON_STANDARD);
		add_button(new gui::standard_dialog_button(disp.video(),
			_("Close"), 1, true), gui::dialog::BUTTON_STANDARD);
	}
	void action(gui::dialog_process_info &info) {
		const bool leader_bool = leader_bools_[get_menu().selection()];
		scroll_btn_->enable(leader_bool);
		if(leader_bool && (info.double_clicked || !info.key_down
		&& (info.key[SDLK_RETURN] || info.key[SDLK_KP_ENTER]))) {
			set_result(get_menu().selection());
		} else if(!info.key_down && info.key[SDLK_ESCAPE]) {
			set_result(gui::CLOSE_DIALOG);
		} else if(!info.key_down && info.key[SDLK_SPACE]) {
			set_result(extra_result_);
		} else if(result() == gui::CONTINUE_DIALOG) {
			dialog::action(info);
		}
	}
private:
	gui::standard_dialog_button *scroll_btn_;
	std::vector<bool> &leader_bools_;
	gui::DIALOG_RESULT extra_result_;
};
} //end anonymous namespace
	void menu_handler::status_table(int selected)
	{
		std::stringstream heading;
		heading << HEADING_PREFIX << _("Leader") << COLUMN_SEPARATOR << ' ' << COLUMN_SEPARATOR
				<< _("Team")         << COLUMN_SEPARATOR
				<< _("Gold")         << COLUMN_SEPARATOR
				<< _("Villages")     << COLUMN_SEPARATOR
				<< _("status^Units") << COLUMN_SEPARATOR
				<< _("Upkeep")       << COLUMN_SEPARATOR
				<< _("Income");

		gui::menu::basic_sorter sorter;
		sorter.set_redirect_sort(0,1).set_alpha_sort(1).set_alpha_sort(2).set_numeric_sort(3)
			  .set_numeric_sort(4).set_numeric_sort(5).set_numeric_sort(6).set_numeric_sort(7);

		std::vector<std::string> items;
		std::vector<bool> leader_bools;
		items.push_back(heading.str());

		const team& viewing_team = teams_[gui_->viewing_team()];

		//if the player is under shroud or fog, they don't get
		//to see details about the other sides, only their own
		//side, allied sides and a ??? is shown to demonstrate
		//lack of information about the other sides But he see
		//all names with in colours
		for(size_t n = 0; n != teams_.size(); ++n) {
			if(teams_[n].is_empty()) {
				continue;
			}

			const bool known = viewing_team.knows_about_team(n);
			const bool enemy = viewing_team.is_enemy(n+1);

			std::stringstream str;

			const team_data data = calculate_team_data(teams_[n],n+1,units_);

			const unit_map::const_iterator leader = team_leader(n+1,units_);
			//output the number of the side first, and this will
			//cause it to be displayed in the correct colour
			if(leader != units_.end()) {

				// Add leader image. If it's fogged
				// show only a random leader image.
				if (known || game_config::debug) {
					str << IMAGE_PREFIX << leader->second.absolute_image();
					leader_bools.push_back(true);
				} else {
					str << IMAGE_PREFIX << std::string("random-enemy.png");
					leader_bools.push_back(false);
				}

#ifndef LOW_MEM
				str << "~RC(" << leader->second.team_color() << ">" << team::get_side_colour_index(n+1) << ")";
#endif
			} else {
				leader_bools.push_back(false);
			}
			str << COLUMN_SEPARATOR	<< team::get_side_highlight(n)
			    << teams_[n].current_player() << COLUMN_SEPARATOR
			    << data.teamname << COLUMN_SEPARATOR;

			if(!known && !game_config::debug) {
				// We don't spare more info (only name)
				// so let's go on next side ...
				items.push_back(str.str());
				continue;
			}

			if(game_config::debug) {
				str << data.gold << COLUMN_SEPARATOR;
			} else if(enemy && viewing_team.uses_fog()) {
				str << ' ' << COLUMN_SEPARATOR;
			} else {
				str << data.gold << COLUMN_SEPARATOR;
			}
			str << data.villages << COLUMN_SEPARATOR
				<< data.units << COLUMN_SEPARATOR << data.upkeep << COLUMN_SEPARATOR
				<< (data.net_income < 0 ? font::BAD_TEXT : font::NULL_MARKUP) << data.net_income;

			items.push_back(str.str());
		}

		int result = 0;
		{
			leader_scroll_dialog slist(*gui_, _("Current Status"), leader_bools, selected, gui::DIALOG_FORWARD);
			slist.add_button(new gui::dialog_button(gui_->video(), _("More >"),
			                     gui::button::TYPE_PRESS, gui::DIALOG_FORWARD),
			                 gui::dialog::BUTTON_EXTRA_LEFT);
			slist.set_menu(items, &sorter);
			slist.get_menu().move_selection(selected);
			result = slist.show();
			selected = slist.get_menu().selection();
		} // this will kill the dialog before scrolling

		if (result >= 0)
			gui_->scroll_to_leader(units_, selected+1);
		else if (result == gui::DIALOG_FORWARD)
			scenario_settings_table(selected);
	}

	void menu_handler::scenario_settings_table(int selected)
	{
		std::stringstream heading;
		heading << HEADING_PREFIX << _("scenario settings^Leader") << COLUMN_SEPARATOR
		        << COLUMN_SEPARATOR
		        << _("scenario settings^Side")              << COLUMN_SEPARATOR
		        << _("scenario settings^Start\nGold")       << COLUMN_SEPARATOR
		        << _("scenario settings^Base\nIncome")      << COLUMN_SEPARATOR
		        << _("scenario settings^Gold Per\nVillage") << COLUMN_SEPARATOR
		        << _("scenario settings^Fog")               << COLUMN_SEPARATOR
		        << _("scenario settings^Shroud");

		gui::menu::basic_sorter sorter;
		sorter.set_redirect_sort(0,1).set_alpha_sort(1).set_numeric_sort(2)
		      .set_numeric_sort(3).set_numeric_sort(4).set_numeric_sort(5)
		      .set_alpha_sort(6).set_alpha_sort(7);

		std::vector<std::string> items;
		std::vector<bool> leader_bools;
		items.push_back(heading.str());

		const team& viewing_team = teams_[gui_->viewing_team()];

		for(size_t n = 0; n != teams_.size(); ++n) {
			if(teams_[n].is_empty()) {
				continue;
			}

			std::stringstream str;
			const unit_map::const_iterator leader = team_leader(n+1, units_);

			if(leader != units_.end()) {
				// Add leader image. If it's fogged
				// show only a random leader image.
				if (viewing_team.knows_about_team(n) || game_config::debug) {
					str << IMAGE_PREFIX << leader->second.absolute_image();
					leader_bools.push_back(true);
				} else {
					str << IMAGE_PREFIX << std::string("random-enemy.png");
					leader_bools.push_back(false);
				}
#ifndef LOW_MEM
				str << "~RC(" << leader->second.team_color() << ">"
				    << team::get_side_colour_index(n+1) << ")";
#endif
			} else {
				leader_bools.push_back(false);
			}

			str << COLUMN_SEPARATOR	<< team::get_side_highlight(n)
			    << teams_[n].current_player() << COLUMN_SEPARATOR
			    << n + 1 << COLUMN_SEPARATOR
			    << teams_[n].start_gold() << COLUMN_SEPARATOR
			    << teams_[n].base_income() << COLUMN_SEPARATOR
			    << teams_[n].village_gold() << COLUMN_SEPARATOR
			    << (teams_[n].uses_fog()    ? _("yes") : _("no")) << COLUMN_SEPARATOR
			    << (teams_[n].uses_shroud() ? _("yes") : _("no")) << COLUMN_SEPARATOR;

			items.push_back(str.str());
		}

		int result = 0;
		{
			leader_scroll_dialog slist(*gui_, _("Scenario Settings"), leader_bools, selected, gui::DIALOG_BACK);
			slist.set_menu(items, &sorter);
			slist.get_menu().move_selection(selected);
			slist.add_button(new gui::dialog_button(gui_->video(), _(" < Back"),
			                     gui::button::TYPE_PRESS, gui::DIALOG_BACK),
			                 gui::dialog::BUTTON_EXTRA_LEFT);
			result = slist.show();
			selected = slist.get_menu().selection();
		} // this will kill the dialog before scrolling

		if (result >= 0)
			gui_->scroll_to_leader(units_, selected+1);
		else if (result == gui::DIALOG_BACK)
			status_table(selected);
	}

	void menu_handler::save_game(const std::string& message, gui::DIALOG_TYPE dialog_type,
		const bool has_exit_button)
	{
		std::stringstream stream;

		const std::string ellipsed_name = font::make_text_ellipsis(gamestate_.label,
				font::SIZE_NORMAL, 200);
		stream << ellipsed_name << " " << _("Turn")
			   << " " << status_.turn();
		std::string label = stream.str();
		if(dialog_type == gui::NULL_DIALOG && message != "") {
			label = message;
		}

		label.erase(std::remove_if(label.begin(),label.end(),is_illegal_file_char),label.end());

		const int res = dialog_type == gui::NULL_DIALOG ? 0 : dialogs::get_save_name(*gui_,message,_("Name:"),&label,dialog_type, "", has_exit_button);

		if(res == 0) {

			if(std::count_if(label.begin(),label.end(),is_illegal_file_char)) {
				gui::message_dialog(*gui_,_("Error"),_("Save names may not contain colons, slashes, or backslashes. Please choose a different name.")).show();
				save_game(message,dialog_type);
				return;
			}

			config snapshot;
			write_game_snapshot(snapshot);
			try {
				recorder.save_game(label, snapshot, gamestate_.starting_pos);
				if(dialog_type != gui::NULL_DIALOG) {
					gui::message_dialog(*gui_,_("Saved"),_("The game has been saved")).show();
				}
			} catch(game::save_game_failed&) {
				gui::message_dialog to_show(*gui_,_("Error"),_("The game could not be saved"));
				to_show.show();
				//do not bother retrying, since the user can just try to save the game again
			};
		} else if(res == 2) {
			throw end_level_exception(QUIT);
		}
	}

	void menu_handler::save_map()
	{
		std::string input_name = get_dir(get_dir(get_user_data_dir() + "/editor") + "/maps/");
		int res = 0;
		int overwrite = 1;
		do {
			res = dialogs::show_file_chooser_dialog(*gui_, input_name, _("Save the Map As"));
			if (res == 0) {

				if (file_exists(input_name)) {
					overwrite = gui::dialog(*gui_, "",
						_("The map already exists. Do you want to overwrite it?"),
						gui::YES_NO).show();
				}
				else
					overwrite = 0;
			}
		} while (res == 0 && overwrite != 0);

		// Try to save the map, if it fails we reset the filename.
		if (res == 0) {
			try {
				write_file(input_name, map_.write());
				gui::message_dialog(*gui_, "", _("Map saved.")).show();
			} catch (io_exception& e) {
				utils::string_map symbols;
				symbols["msg"] = e.what();
				const std::string msg = vgettext("Could not save the map: $msg",symbols);
				gui::message_dialog(*gui_, "", msg).show();
			}
		}
	}

	void menu_handler::write_game_snapshot(config& start) const
	{
		start.values = level_.values;

		start["snapshot"] = "yes";

		std::stringstream buf;
		buf << gui_->playing_team();
		start["playing_team"] = buf.str();

		for(std::vector<team>::const_iterator t = teams_.begin(); t != teams_.end(); ++t) {
			const unsigned int side_num = t - teams_.begin() + 1;

			config& side = start.add_child("side");
			t->write(side);
			side["no_leader"] = "yes";
			buf.str(std::string());
			buf << side_num;
			side["side"] = buf.str();

			//current visible units
			for(unit_map::const_iterator i = units_.begin(); i != units_.end(); ++i) {
				if(i->second.side() == side_num) {
					config& u = side.add_child("unit");
					i->first.write(u);
					i->second.write(u);
				}
			}
			//recall list
			{
				for(std::map<std::string, player_info>::const_iterator i=gamestate_.players.begin();
				i!=gamestate_.players.end(); ++i) {
					for(std::vector<unit>::const_iterator j = i->second.available_units.begin();
						j != i->second.available_units.end(); ++j) {
						if (j->side() == side_num){
							config& u = side.add_child("unit");
							j->write(u);
						}
					}
				}
			}
		}

		status_.write(start);
		game_events::write_events(start);

		// Write terrain_graphics data in snapshot, too
		const config::child_list& terrains = level_.get_children("terrain_graphics");
		for(config::child_list::const_iterator tg = terrains.begin();
				tg != terrains.end(); ++tg) {

			start.add_child("terrain_graphics", **tg);
		}

		sound::write_music_play_list(start);

		write_game(gamestate_, start, WRITE_SNAPSHOT_ONLY);

		// Clobber gold values to make sure the snapshot uses the values
		// in [side] instead.
		const config::child_list& players=start.get_children("player");
		for(config::child_list::const_iterator pi=players.begin();
			pi!=players.end(); ++pi) {
			(**pi)["gold"] = "-1000000";
		}

		//write out the current state of the map
		start["map_data"] = map_.write();

		gui_->labels().write(start);
	}

	void menu_handler::autosave(const std::string &label, unsigned turn, const config &starting_pos) const
	{
		if(game_config::disable_autosave)
			return;

		Uint32 start, end;
		start = SDL_GetTicks();
		config snapshot;
		std::string savename;
		if (label.empty())
			savename = _("Auto-Save");
		else
			savename = label + "-" + _("Auto-Save") + lexical_cast<std::string>(turn);
		write_game_snapshot(snapshot);
		try {
			recorder.save_game(savename, snapshot, starting_pos);
		} catch(game::save_game_failed&) {
			gui::message_dialog(*gui_,"",_("Could not auto save the game. Please save the game manually.")).show();
			//do not bother retrying, since the user can just save the game
		}
		end = SDL_GetTicks();
		std::cerr << "Milliseconds to save " << savename << ": " << end - start << "\n";

		remove_old_saves();
	}

	void menu_handler::load_game(){
		bool show_replay = false;
		const std::string game = dialogs::load_game_dialog(*gui_, game_config_, gameinfo_, &show_replay);
		if(game != "") {
			throw game::load_game_exception(game,show_replay);
		}
	}

	void menu_handler::preferences()
	{
		preferences::show_preferences_dialog(*gui_, game_config_);
		gui_->redraw_everything();
	}

	void menu_handler::show_chat_log()
	{
		std::string text = recorder.build_chat_log(
		    is_observer() ? game_config::observer_team_name
		                  : teams_[gui_->viewing_team()].team_name());
		gui::show_dialog(*gui_,NULL,_("Chat Log"),"",gui::CLOSE_ONLY,NULL,NULL,"",&text);
	}

	void menu_handler::show_help()
	{
		help::show_help(*gui_);
	}

	void menu_handler::speak()
	{
		textbox_info_.show(gui::TEXTBOX_MESSAGE,_("Message:"),
			has_friends() ? is_observer() ? _("Send to observers only") : _("Send to allies only")
						  : "", preferences::message_private(), *gui_);
	}

	void menu_handler::whisper()
	{
		preferences::set_message_private(true);
		speak();
	}

	void menu_handler::shout()
	{
		preferences::set_message_private(false);
		speak();
	}

	bool menu_handler::has_friends() const
	{
		if(is_observer()) {
			return !gui_->observers().empty();
		}

		for(size_t n = 0; n != teams_.size(); ++n) {
			if(n != gui_->viewing_team() && teams_[gui_->viewing_team()].team_name() == teams_[n].team_name() && teams_[n].is_network()) {
				return true;
			}
		}

		return false;
	}

	bool menu_handler::has_team() const
	{
		if(is_observer()) {
			return false;
		}

		for(size_t n = 0; n != teams_.size(); ++n) {
			if(n != gui_->viewing_team() && teams_[gui_->viewing_team()].team_name() == teams_[n].team_name()) {
				return true;
			}
		}

		return false;
	}

	void menu_handler::recruit(const bool browse, const unsigned int team_num, const gamemap::location& last_hex)
	{
		if(browse)
			return;

		team& current_team = teams_[team_num-1];

		std::vector<const unit_type*> sample_units;

		gui_->draw(); //clear the old menu
		std::vector<std::string> item_keys;
		std::vector<std::string> items;
		const std::set<std::string>& recruits = current_team.recruits();
		for(std::set<std::string>::const_iterator it = recruits.begin(); it != recruits.end(); ++it) {
			const std::map<std::string,unit_type>::const_iterator
					u_type = gameinfo_.unit_types.find(*it);
			if(u_type == gameinfo_.unit_types.end()) {
				LOG_STREAM(err, engine) << "could not find unit '" << *it << "'\n";
				return;
			}

			item_keys.push_back(*it);

			const unit_type* type = &u_type->second;

			//display units that we can't afford to recruit in red
			const char prefix = (type->cost() > current_team.gold() ? font::BAD_TEXT : font::NULL_MARKUP);

			std::stringstream description;
			description << font::IMAGE << type->image();
#ifndef LOW_MEM
			description << "~RC(" << type->flag_rgb() << ">" << team::get_side_colour_index(team_num) << ")";
#endif
			description << COLUMN_SEPARATOR << font::LARGE_TEXT << prefix << type->language_name() << "\n"
					<< prefix << type->cost() << " " << sngettext("unit^Gold", "Gold", type->cost());

			items.push_back(description.str());
			sample_units.push_back(type);
		}

		if(sample_units.empty()) {
			gui::message_dialog to_show(*gui_,"",_("You have no units available to recruit."));
			to_show.show();
			return;
		}

		int recruit_res = 0;

		{
			dialogs::unit_types_preview_pane unit_preview(*gui_,&map_,sample_units,team_num);
			std::vector<gui::preview_pane*> preview_panes;
			preview_panes.push_back(&unit_preview);

			gui::dialog rmenu(*gui_,_("Recruit"),
					  _("Select unit:") + std::string("\n"),
					  gui::OK_CANCEL,
					  gui::dialog::default_style);
			rmenu.add_button(new help::help_button(*gui_,"recruit_and_recall"),
				gui::dialog::BUTTON_HELP);
			rmenu.set_menu(items);
			rmenu.set_panes(preview_panes);
			recruit_res = rmenu.show();
		}

		if(recruit_res != -1) {
			do_recruit(item_keys[recruit_res], team_num, last_hex);
		}
	}

	void menu_handler::repeat_recruit(const unsigned int team_num, const gamemap::location& last_hex)
	{
		if(last_recruit_.empty() == false)
			do_recruit(last_recruit_, team_num, last_hex);
	}

	void menu_handler::do_recruit(const std::string& name, const int unsigned team_num, const gamemap::location& last_hex)
	{
		team& current_team = teams_[team_num-1];

		//search for the unit to be recruited in recruits
		int recruit_num = 0;
		const std::set<std::string>& recruits = current_team.recruits();
		for(std::set<std::string>::const_iterator r = recruits.begin(); ; ++r) {
			if (r == recruits.end()) {
				return;
			}

			if (name == *r) {
				break;
			}
			++recruit_num;
		}

		const std::map<std::string,unit_type>::const_iterator
				u_type = gameinfo_.unit_types.find(name);
		wassert(u_type != gameinfo_.unit_types.end());

		if(u_type->second.cost() > current_team.gold()) {
			gui::message_dialog(*gui_,"",
				 _("You don't have enough gold to recruit that unit")).show();
		} else {
			last_recruit_ = name;

			//create a unit with traits
			recorder.add_recruit(recruit_num, last_hex);
			unit new_unit(&gameinfo_,&units_,&map_,&status_,&teams_,&(u_type->second),team_num,true);
			gamemap::location loc = last_hex;
			const std::string& msg = recruit_unit(map_,team_num,units_,new_unit,loc,(gui_!=NULL));
			if(msg.empty()) {
				current_team.spend_gold(u_type->second.cost());
				statistics::recruit_unit(new_unit);

				//MP_COUNTDOWN grant time bonus for recruiting
				current_team.set_action_bonus_count(1 + current_team.action_bonus_count());

				redo_stack_.clear();
				wassert(new_unit.type());

				const bool shroud_cleared = clear_shroud(team_num);
				if(shroud_cleared || new_unit.type()->genders().size() > 1 || new_unit.type()->has_random_traits()) {
					clear_undo_stack(team_num);
				} else {
					undo_stack_.push_back(undo_action(new_unit,loc,RECRUIT_POS));
				}


				gui_->recalculate_minimap();
				gui_->invalidate_game_status();
				gui_->invalidate_all();
				recorder.add_checksum_check(loc);
			} else {
				recorder.undo();
				gui::message_dialog(*gui_,"",msg).show();
			}
		}
	}

	void menu_handler::recall(const unsigned int team_num, const gamemap::location& last_hex)
	{
		player_info *player = gamestate_.get_player(teams_[team_num-1].save_id());
		if(!player) {
			LOG_STREAM(err, engine) << "cannot recall a unit for side " << team_num
				<< ", which has no recall list!\n";
			return;
		}

		team& current_team = teams_[team_num-1];
		std::vector<unit>& recall_list = player->available_units;

		//sort the available units into order by value
		//so that the most valuable units are shown first
		sort_units(recall_list);

		gui_->draw(); //clear the old menu

		if(utils::string_bool(level_["disallow_recall"])) {
			gui::message_dialog(*gui_,"",_("You are separated from your soldiers and may not recall them")).show();
		} else if(recall_list.empty()) {
			gui::message_dialog(*gui_,"",_("There are no troops available to recall\n(You must have veteran survivors from a previous scenario)")).show();
		} else {
			std::vector<std::string> options;

			std::ostringstream heading;
			heading << HEADING_PREFIX << COLUMN_SEPARATOR << _("Type")
					<< COLUMN_SEPARATOR << _("Name")
					<< COLUMN_SEPARATOR << _("Level")
					<< COLUMN_SEPARATOR << _("XP");

			gui::menu::basic_sorter sorter;
			sorter.set_alpha_sort(1).set_alpha_sort(2).set_id_sort(3).set_numeric_sort(4);

			options.push_back(heading.str());

			for(std::vector<unit>::const_iterator u = recall_list.begin(); u != recall_list.end(); ++u) {
				std::stringstream option;
				const std::string& description = u->description().empty() ? "-" : u->description();

				option << IMAGE_PREFIX << u->absolute_image();
#ifndef LOW_MEM
				option << "~RC("  << u->team_color() << ">" << team::get_side_colour_index(team_num) << ")";
#endif
				option << COLUMN_SEPARATOR
					<< u->language_name() << COLUMN_SEPARATOR
					<< description << COLUMN_SEPARATOR
					<< u->level() << COLUMN_SEPARATOR
					<< u->experience() << "/";

				if(u->can_advance() == false) {
					option << "-";
				} else {
					option << u->max_experience();
				}

				options.push_back(option.str());
			}

			delete_recall_unit recall_deleter(*gui_,recall_list);
			gui::dialog_button_info delete_button(&recall_deleter,_("Dismiss Unit"));
			int res = 0;

			{
				dialogs::units_list_preview_pane unit_preview(*gui_,&map_,recall_list);
				gui::dialog rmenu(*gui_,_("Recall"),
						  _("Select unit:") + std::string("\n"),
						  gui::OK_CANCEL,
						  gui::dialog::default_style);
				rmenu.add_button(new help::help_button(*gui_,"recruit_and_recall"),
					gui::dialog::BUTTON_HELP);
				rmenu.set_menu(options, &sorter);
				rmenu.add_pane(&unit_preview);
				rmenu.add_button(delete_button);
				res = rmenu.show();
			}

			if(res >= 0) {
				if(current_team.gold() < game_config::recall_cost) {
					std::stringstream msg;
					utils::string_map i18n_symbols;
					i18n_symbols["cost"] = lexical_cast<std::string>(game_config::recall_cost);
					msg << vngettext("You must have at least 1 gold piece to recall a unit",
						"You must have at least $cost gold pieces to recall a unit",
						game_config::recall_cost,
						i18n_symbols);
					gui::dialog(*gui_,"",msg.str()).show();
				} else {
					std::cerr << "recall index: " << res << "\n";
					unit& un = recall_list[res];
					gamemap::location loc = last_hex;
					recorder.add_recall(res,loc);
					un.set_game_context(&gameinfo_,&units_,&map_,&status_,&teams_);
					const std::string err = recruit_unit(map_,team_num,units_,un,loc,(gui_!=NULL));
					if(!err.empty()) {
						recorder.undo();
						gui::dialog(*gui_,"",err,gui::OK_ONLY).show();
					} else {
						statistics::recall_unit(un);
						current_team.spend_gold(game_config::recall_cost);

						const bool shroud_cleared = clear_shroud(team_num);
						if (shroud_cleared) {
							clear_undo_stack(team_num);
						} else {
							undo_stack_.push_back(undo_action(un,loc,res));
						}

						redo_stack_.clear();

						recall_list.erase(recall_list.begin()+res);
						gui_->invalidate_game_status();
						gui_->invalidate_all();
						recorder.add_checksum_check(loc);
					}
				}
			}
		}
	}
	void menu_handler::undo(const unsigned int team_num, mouse_handler& mousehandler)
	{
		if(undo_stack_.empty())
			return;

		const events::command_disabler disable_commands;

		undo_action& action = undo_stack_.back();
		if(action.is_recall()) {
			player_info* const player = gamestate_.get_player(teams_[team_num - 1].save_id());

			if(player == NULL) {
				LOG_STREAM(err, engine) << "trying to undo a recall for side " << team_num
					<< ", which has no recall list!\n";
			} else {
				// Undo a recall action
				if(units_.count(action.recall_loc) == 0) {
					return;
				}

				const unit& un = units_.find(action.recall_loc)->second;
				statistics::un_recall_unit(un);
				teams_[team_num - 1].spend_gold(-game_config::recall_cost);

				std::vector<unit>& recall_list = player->available_units;
				recall_list.insert(recall_list.begin()+action.recall_pos,un);
				// invalidate before erasing allow us
				// to also do the ovelerlapped hexes
				gui_->invalidate(action.recall_loc);
				units_.erase(action.recall_loc);
				gui_->draw();
			}
		} else if(action.is_recruit()) {
			// Undo a recruit action
			team& current_team = teams_[team_num-1];
			if(units_.count(action.recall_loc) == 0) {
				return;
			}

			const unit& un = units_.find(action.recall_loc)->second;
			statistics::un_recruit_unit(un);
			wassert(un.type());
			current_team.spend_gold(-un.type()->cost());

			//MP_COUNTDOWN take away recruit bonus
			if(action.countdown_time_bonus)
			{
				teams_[team_num-1].set_action_bonus_count(teams_[team_num-1].action_bonus_count() - 1);
			}

			// invalidate before erasing allow us
			// to also do the ovelerlapped hexes
			gui_->invalidate(action.recall_loc);
			units_.erase(action.recall_loc);
			gui_->draw();
		} else {
			// Undo a move action
			const int starting_moves = action.starting_moves;
			std::vector<gamemap::location> route = action.route;
			std::reverse(route.begin(),route.end());
			const unit_map::iterator u = units_.find(route.front());
			const unit_map::iterator u_end = units_.find(route.back());
			if(u == units_.end() || u_end != units_.end()) {
				//this can actually happen if the scenario designer has abused the [allow_undo] command
				LOG_STREAM(err, engine) << "Illegal 'undo' found. Possible abuse of [allow_undo]?\n";
				return;
			}

			if(map_.is_village(route.front())) {
				get_village(route.front(),*gui_,teams_,action.original_village_owner,units_);
				//MP_COUNTDOWN take away capture bonus
				if(action.countdown_time_bonus)
				{
					teams_[team_num-1].set_action_bonus_count(teams_[team_num-1].action_bonus_count() - 1);
				}
			}

			action.starting_moves = u->second.movement_left();

			unit_display::move_unit(map_,route,u->second,teams_);
			std::pair<gamemap::location,unit> *up = units_.extract(u->first);
			up->second.set_goto(gamemap::location());
			up->second.set_movement(starting_moves);
			up->first = route.back();
			units_.add(up);
			up->second.set_standing(*gui_,up->first);
			gui_->invalidate(route.back());
			gui_->draw();
		}

		gui_->invalidate_unit();
		gui_->invalidate_game_status();

		redo_stack_.push_back(action);
		undo_stack_.pop_back();

		mousehandler.set_selected_hex(gamemap::location());
		mousehandler.set_current_paths(paths());

		recorder.undo();

		const bool shroud_cleared = clear_shroud(team_num);

		if(shroud_cleared) {
			gui_->recalculate_minimap();
		} else {
			gui_->redraw_minimap();
		}
	}

	void menu_handler::redo(const unsigned int team_num, mouse_handler& mousehandler)
	{
		if(redo_stack_.empty())
			return;

		const events::command_disabler disable_commands;

		//clear routes, selected hex, etc
		mousehandler.set_selected_hex(gamemap::location());
		mousehandler.set_current_paths(paths());

		undo_action& action = redo_stack_.back();
		if(action.is_recall()) {
			player_info *player=gamestate_.get_player(teams_[team_num - 1].save_id());
			if(!player) {
				LOG_STREAM(err, engine) << "trying to redo a recall for side " << team_num
					<< ", which has no recall list!\n";
			} else {
				// Redo recall
				std::vector<unit>& recall_list = player->available_units;
				unit un = recall_list[action.recall_pos];

				recorder.add_recall(action.recall_pos,action.recall_loc);
				un.set_game_context(&gameinfo_,&units_,&map_,&status_,&teams_);
				const std::string& msg = recruit_unit(map_,team_num,units_,un,action.recall_loc,(gui_!=NULL));
				if(msg.empty()) {
					statistics::recall_unit(un);
					teams_[team_num - 1].spend_gold(game_config::recall_cost);
					recall_list.erase(recall_list.begin()+action.recall_pos);

					gui_->invalidate(action.recall_loc);
					gui_->draw();
					recorder.add_checksum_check(action.recall_loc);
				} else {
					recorder.undo();
					gui::dialog(*gui_,"",msg,gui::OK_ONLY).show();
				}
			}
		} else if(action.is_recruit()) {
			// Redo recruit action
			team& current_team = teams_[team_num-1];
			gamemap::location loc = action.recall_loc;
			const std::string name = action.affected_unit.id();

			//search for the unit to be recruited in recruits
			int recruit_num = 0;
			const std::set<std::string>& recruits = current_team.recruits();
			for(std::set<std::string>::const_iterator r = recruits.begin(); ; ++r) {
				if (r == recruits.end()) {
					LOG_STREAM(err, engine) << "trying to redo a recruit for side " << team_num
						<< ", which does not recruit type \"" << name << "\"\n";
					wassert(0);
				}
				if (name == *r) {
					break;
				}
				++recruit_num;
			}
			last_recruit_ = name;
			recorder.add_recruit(recruit_num,loc);
			unit new_unit = action.affected_unit;
			//unit new_unit(action.affected_unit.type(),team_num_,true);
			const std::string& msg = recruit_unit(map_,team_num,units_,new_unit,loc,(gui_!=NULL));
			if(msg.empty()) {
				current_team.spend_gold(new_unit.type()->cost());
				statistics::recruit_unit(new_unit);

				//MP_COUNTDOWN: restore recruitment bonus
				current_team.set_action_bonus_count(1 + current_team.action_bonus_count());

				gui_->invalidate(action.recall_loc);
				gui_->draw();
				//gui_.invalidate_game_status();
				//gui_.invalidate_all();
				recorder.add_checksum_check(loc);
			} else {
				recorder.undo();
				gui::dialog(*gui_,"",msg,gui::OK_ONLY).show();
			}
		} else {
			// Redo movement action
			const int starting_moves = action.starting_moves;
			std::vector<gamemap::location> route = action.route;
			const unit_map::iterator u = units_.find(route.front());
			if(u == units_.end()) {
				wassert(false);
				return;
			}

			action.starting_moves = u->second.movement_left();

			unit_display::move_unit(map_,route,u->second,teams_);
			std::pair<gamemap::location,unit> *up = units_.extract(u->first);
			up->second.set_goto(gamemap::location());
			up->second.set_movement(starting_moves);
			up->first = route.back();
			units_.add(up);
			up->second.set_standing(*gui_,up->first);

			if(map_.is_village(route.back())) {
				get_village(route.back(),*gui_,teams_,up->second.side()-1,units_);
				//MP_COUNTDOWN restore capture bonus
				if(action.countdown_time_bonus)
				{
					teams_[team_num-1].set_action_bonus_count(1 + teams_[team_num-1].action_bonus_count());
				}
			}

			gui_->invalidate(route.back());
			gui_->draw();

			recorder.add_movement(route.front(),route.back());
		}
		gui_->invalidate_unit();
		gui_->invalidate_game_status();

		undo_stack_.push_back(action);
		redo_stack_.pop_back();
	}

	bool menu_handler::clear_shroud(const unsigned int team_num)
	{
		bool cleared = teams_[team_num - 1].auto_shroud_updates() &&
			::clear_shroud(*gui_,status_,map_,gameinfo_,units_,teams_,team_num-1);
		return cleared;
	}

	void menu_handler::clear_undo_stack(const unsigned int team_num)
	{
		if(teams_[team_num - 1].auto_shroud_updates() == false)
			apply_shroud_changes(undo_stack_,gui_,status_,map_,gameinfo_,units_,teams_,team_num-1);
		undo_stack_.clear();
	}

	// Highlights squares that an enemy could move to on their turn, showing how many can reach each square.
	void menu_handler::show_enemy_moves(bool ignore_units, const unsigned int team_num)
	{
		gui_->unhighlight_reach();

		// Compute enemy movement positions
		for(unit_map::iterator u = units_.begin(); u != units_.end(); ++u) {
			bool invisible = u->second.invisible(u->first, units_, teams_);

			if(teams_[team_num - 1].is_enemy(u->second.side()) && !gui_->fogged(u->first) && !u->second.incapacitated() && !invisible) {
				const unit_movement_resetter move_reset(u->second);
				const bool teleports = u->second.get_ability_bool("teleport",u->first);
				unit_map units(u->first, u->second);
				const paths& path = paths(map_,status_,gameinfo_,ignore_units?units:units_,
										  u->first,teams_,false,teleports,teams_[gui_->viewing_team()]);

				gui_->highlight_another_reach(path);
			}
		}
	}

	void menu_handler::toggle_shroud_updates(const unsigned int team_num) {
		bool auto_shroud = teams_[team_num - 1].auto_shroud_updates();
		// If we're turning automatic shroud updates on, then commit all moves
		if(auto_shroud == false) update_shroud_now(team_num);
		teams_[team_num - 1].set_auto_shroud_updates(!auto_shroud);
	}

	void menu_handler::update_shroud_now(const unsigned int team_num)
	{
		clear_undo_stack(team_num);
	}

	bool menu_handler::end_turn(const unsigned int team_num)
	{
		bool unmoved_units = false, partmoved_units = false, some_units_have_moved = false;
		for(unit_map::const_iterator un = units_.begin(); un != units_.end(); ++un) {
			if(un->second.side() == team_num) {
				if(unit_can_move(un->first,units_,map_,teams_)) {
					if(!un->second.has_moved()) {
						unmoved_units = true;
					}

					partmoved_units = true;
				}
				if(un->second.has_moved()) {
					some_units_have_moved = true;
				}
			}
		}

		//Ask for confirmation if the player hasn't made any moves (other than gotos).
		if(preferences::confirm_no_moves() && ! some_units_have_moved) {
			const int res = gui::dialog(*gui_,"",_("You have not started your turn yet. Do you really want to end your turn?"), gui::YES_NO).show();
			if(res != 0) {
				return false;
			}
		}

		// Ask for confirmation if units still have movement left
		if(preferences::yellow_confirm() && partmoved_units) {
			const int res = gui::dialog(*gui_,"",_("Some units have movement left. Do you really want to end your turn?"),gui::YES_NO).show();
			if (res != 0) {
				return false;
			}
		} else if (preferences::green_confirm() && unmoved_units) {
			const int res = gui::dialog(*gui_,"",_("Some units have movement left. Do you really want to end your turn?"),gui::YES_NO).show();
			if (res != 0) {
				return false;
			}
		}

		//force any pending fog updates
		clear_undo_stack(team_num);
		gui_->set_route(NULL);

		recorder.end_turn();

		return true;
	}

	void menu_handler::goto_leader(const unsigned int team_num)
	{
		const unit_map::const_iterator i = team_leader(team_num,units_);
		if(i != units_.end()) {
			clear_shroud(team_num);
			gui_->scroll_to_tile(i->first,game_display::WARP);
		}
	}

	void menu_handler::unit_description(mouse_handler& mousehandler)
	{
		const unit_map::const_iterator un = current_unit(mousehandler);
		if(un != units_.end()) {
			dialogs::show_unit_description(*gui_, un->second);
		}
	}

	void menu_handler::rename_unit(mouse_handler& mousehandler)
	{
		const unit_map::iterator un = current_unit(mousehandler);
		if(un == units_.end() || gui_->viewing_team()+1 != un->second.side())
			return;
		if(un->second.unrenamable())
			return;

		std::string name = un->second.description();
		const int res = gui::show_dialog(*gui_,NULL,_("Rename Unit"),"", gui::OK_CANCEL,NULL,NULL,"",&name);
		if(res == 0) {
			recorder.add_rename(name, un->first);
			un->second.rename(name);
			gui_->invalidate_unit();
		}
	}

	unit_map::iterator menu_handler::current_unit(mouse_handler& mousehandler)
	{
		unit_map::iterator res = find_visible_unit(units_, mousehandler.get_last_hex(),
			map_, teams_, teams_[gui_->viewing_team()]);
		if(res != units_.end()) {
			return res;
		} else {
			return find_visible_unit(units_, mousehandler.get_selected_hex(),
			map_, teams_, teams_[gui_->viewing_team()]);
		}
	}

	unit_map::const_iterator menu_handler::current_unit(const mouse_handler& mousehandler) const
	{
		unit_map::const_iterator res = find_visible_unit(units_, mousehandler.get_last_hex(),
			map_, teams_, teams_[gui_->viewing_team()]);
		if(res != units_.end()) {
			return res;
		} else {
			return find_visible_unit(units_, mousehandler.get_selected_hex(),
			map_, teams_, teams_[gui_->viewing_team()]);
		}
	}

	void menu_handler::create_unit(mouse_handler& mousehandler)
	{
		std::vector<std::string> options;
		std::vector<const unit_type*> unit_choices;
		const std::string heading = std::string(1,HEADING_PREFIX) +
									_("Race")      + COLUMN_SEPARATOR +
									_("Type");
		options.push_back(heading);

		for(game_data::unit_type_map::const_iterator i = gameinfo_.unit_types.begin(); i != gameinfo_.unit_types.end(); ++i) {
			std::stringstream row;

			std::string race;
			const race_map::const_iterator race_it = gameinfo_.races.find(i->second.race());
			if (race_it != gameinfo_.races.end()) {
				race = race_it->second.name();
			}
			row << race << COLUMN_SEPARATOR;
			row << i->second.language_name() << COLUMN_SEPARATOR;

			options.push_back(row.str());
			unit_choices.push_back(&(i->second));
		}

		int choice = 0;
		{
			gui::menu::basic_sorter sorter;
			sorter.set_alpha_sort(0).set_alpha_sort(1);

			dialogs::unit_types_preview_pane unit_preview(*gui_, &map_, unit_choices);
			gui::dialog umenu(*gui_, _("Create Unit (Debug!)"), "", gui::OK_CANCEL);
			umenu.set_menu(options, &sorter);
			umenu.add_pane(&unit_preview);
			//sort by race then by type name
			umenu.get_menu().sort_by(1);
			umenu.get_menu().sort_by(0);
			umenu.get_menu().reset_selection();
			unit_preview.set_selection(umenu.get_menu().selection());
			choice = umenu.show();
		}

		if (size_t(choice) < unit_choices.size()) {
			units_.erase(mousehandler.get_last_hex());

			unit chosen(&gameinfo_,&units_,&map_,&status_,&teams_,unit_choices[choice],1,false);
			chosen.new_turn();
			units_.add(new std::pair<gamemap::location,unit>(mousehandler.get_last_hex(),chosen));

			gui_->invalidate(mousehandler.get_last_hex());
			gui_->invalidate_unit();
		}
	}

	void menu_handler::change_unit_side(mouse_handler& mousehandler)
	{
		const unit_map::iterator i = units_.find(mousehandler.get_last_hex());
		if(i == units_.end()) {
			return;
		}

		int side = i->second.side();
		++side;
		if(side > team::nteams()) {
			side = 1;
		}

		i->second.set_side(side);
	}

	void menu_handler::label_terrain(mouse_handler& mousehandler, bool team_only)
	{
		if(map_.on_board(mousehandler.get_last_hex()) == false) {
			return;
		}
		gui::dialog d(*gui_, _("Place Label"), "", gui::OK_CANCEL);
		const terrain_label* old_label = gui_->labels().get_label(mousehandler.get_last_hex());
		if (old_label) {
			d.set_textbox(_("Label:"), old_label->text(), map_labels::get_max_chars());
			team_only = !old_label->team_name().empty();
		} else {
			d.set_textbox(_("Label:"), "", map_labels::get_max_chars());
		}
		if (has_team() || (old_label && team_only)) {
			d.add_option(_("Team only"), team_only, gui::dialog::BUTTON_CHECKBOX_LEFT);
		}
		if(!d.show()) {
			std::string team_name;
			SDL_Color colour = font::LABEL_COLOUR;
			std::ostringstream last_team_id;
			last_team_id << gamemap::MAX_PLAYERS;
			std::map<std::string, color_range>::iterator gp = game_config::team_rgb_range.find(last_team_id.str());

			if ((has_team() || (old_label && team_only)) && d.option_checked()) {
				team_name = gui_->labels().team_name();
			}
			else {
				colour = int_to_color(team::get_side_rgb(gui_->viewing_team()+1));
			}
			const terrain_label *res = gui_->labels().set_label(mousehandler.get_last_hex(), d.textbox_text(), team_name, colour);
			if (res)
				recorder.add_label(res);
		}
	}

	void menu_handler::clear_labels()
	{
		if (gui_->team_valid()
		   && !is_observer())
		{
			gui_->labels().clear(gui_->current_team_name());
			recorder.clear_labels(gui_->current_team_name());
		}
	}

	void menu_handler::continue_move(mouse_handler& mousehandler, const unsigned int team_num)
	{
		unit_map::iterator i = current_unit(mousehandler);
		if(i == units_.end() || i->second.move_interrupted() == false) {
			i = units_.find(mousehandler.get_selected_hex());
			if (i == units_.end() || i->second.move_interrupted() == false) return;
		}
		move_unit_to_loc(i,i->second.get_interrupted_move(),true, team_num, mousehandler);
	}

	void menu_handler::move_unit_to_loc(const unit_map::const_iterator& ui, const gamemap::location& target, bool continue_move, const unsigned int team_num, mouse_handler& mousehandler)
	{
		wassert(ui != units_.end());

		paths::route route = mousehandler.get_route(ui, target, teams_[team_num - 1]);

		if(route.steps.empty())
			return;

		wassert(route.steps.front() == ui->first);

		gui_->set_route(&route);
		move_unit(gui_,gameinfo_,status_,map_,units_,teams_,route.steps,&recorder,&undo_stack_,NULL,continue_move);
		gui_->invalidate_game_status();
	}

	void menu_handler::toggle_grid()
	{
		preferences::set_grid(!preferences::grid());
		gui_->invalidate_all();
	}

	void menu_handler::unit_hold_position(mouse_handler& mousehandler, const unsigned int team_num)
	{
		const unit_map::iterator un = units_.find(mousehandler.get_selected_hex());
		if(un != units_.end() && un->second.side() == team_num && un->second.movement_left() >= 0) {
			un->second.set_hold_position(!un->second.hold_position());
			gui_->invalidate(mousehandler.get_selected_hex());

			mousehandler.set_current_paths(paths());
			gui_->draw();

			if(un->second.hold_position()) {
				un->second.set_user_end_turn(true);
				mousehandler.cycle_units(false);
			}
		}
	}

	void menu_handler::end_unit_turn(mouse_handler& mousehandler, const unsigned int team_num)
	{
		const unit_map::iterator un = units_.find(mousehandler.get_selected_hex());
		if(un != units_.end() && un->second.side() == team_num && un->second.movement_left() >= 0) {
			un->second.set_user_end_turn(!un->second.user_end_turn());
			if(un->second.hold_position() && !un->second.user_end_turn()){
			  un->second.set_hold_position(false);
			}
			gui_->invalidate(mousehandler.get_selected_hex());

			mousehandler.set_current_paths(paths());
			gui_->draw();

			if(un->second.user_end_turn()) {
				mousehandler.cycle_units(false);
			}
		}
	}

	void menu_handler::search()
	{
		std::ostringstream msg;
		msg << _("Search");
		if(last_search_hit_.valid()) {
			msg << " [" << last_search_ << "]";
		}
		msg << ':';
		textbox_info_.show(gui::TEXTBOX_SEARCH,msg.str(), "", false, *gui_);
	}

	void menu_handler::do_speak(){
		//None of the two parameters really needs to be passed since the informations belong to members of the class.
		//But since it makes the called method more generic, it is done anyway.
		chat_handler::do_speak(textbox_info_.box()->text(),textbox_info_.check() != NULL ? textbox_info_.check()->checked() : false);
	}

	void menu_handler::add_chat_message(const std::string& speaker, int side, const std::string& message, game_display::MESSAGE_TYPE type)
	{
		gui_->add_chat_message(speaker,side,message,type,false);
	}

	chat_handler::~chat_handler()
	{
	}

	void chat_handler::do_speak(const std::string& message, bool allies_only)
	{
		if(message == "") {
			return;
		}

		static const std::string query = "/query";
		static const std::string whisper = "/whisper";
		static const std::string whisper2 = "/msg";
		static const std::string list = "/list";
		static const std::string help = "/help";
		static const std::string emote = "/emote";
		static const std::string emote2 = "/me";

		static const std::string addignore = "addignore";
		static const std::string addfriend = "addfriend";
		static const std::string remove = "remove";
		static const std::string display = "display";
		static const std::string clear = "clear";

		static const std::string help_chat_help = _("Commands: whisper list emote. Type /help [command] for more help.");

		bool is_command = (message.at(0) == '/');
		unsigned int argc = 0;
		std::string cmd, arg1, arg2;

		// Use utils::split()?
		if(is_command){
			std::string::size_type sp1 = message.find_first_of(' ');
			cmd = message.substr(0,sp1);
			if(sp1 != std::string::npos) {
				std::string::size_type arg1_start = message.find_first_not_of(' ',sp1);
				if(arg1_start != std::string::npos) {
					++argc;
					std::string::size_type substr_len, sp2;
					sp2 = message.find(' ',arg1_start);
					substr_len = (sp2 == std::string::npos) ? sp2 : sp2 - arg1_start;
					arg1 = message.substr(arg1_start,substr_len);
					if(sp2 != std::string::npos) {
						std::string::size_type arg2_end = message.find_last_not_of(' ');
						if(arg2_end > sp2) {
							++argc;
							arg2 = message.substr(sp2+1, arg2_end - sp2);
						}
					}
				}
			}
		}


		if(cmd == query && argc > 0) {
			const std::string args = (argc < 2) ? arg1 : arg1 + " " + arg2;
			send_chat_query(args);
		} else if ((cmd == whisper || cmd == whisper2) && argc > 1 /*&& is_observer()*/) {
			config cwhisper,data;
			cwhisper["message"] = arg2;
			cwhisper["sender"] = preferences::login();
			cwhisper["receiver"] = arg1;
			data.add_child("whisper", cwhisper);
			add_chat_message("whisper to "+cwhisper["receiver"],0,cwhisper["message"], game_display::MESSAGE_PRIVATE);
			network::send_data(data);

		} else if (cmd == help) {

			bool have_command = (argc > 0);
			bool have_subcommand = (argc > 1);

			const std::string command = arg1;
			const std::string subcommand = arg2;

			if (have_command) {
				if (command == "whisper" || command == "msg") {
					add_chat_message("help",0,_("Sends private message. You can't send messages to players that control any side in game. Usage: /whisper [nick] [message]"));
				} else if (command == "list") {
					if (have_subcommand) {
						if (subcommand == "addfriend"){
							add_chat_message("help",0,_("Add player to your friends list. Usage: /list addfriend [argument]"));
						} else if (subcommand == "addignore"){
							add_chat_message("help",0,_("Add player to your ignore list. Usage: /list ignore [argument]"));
						} else if (subcommand == "remove") {
							add_chat_message("help",0,_("Remove player from your ignore or friends list. Usage: /list remove [argument]"));
						} else if (subcommand == "clear") {
							add_chat_message("help",0,_("Clear your ignore and friends list. Usage: /list clear"));
						} else if (subcommand == "display") {
							add_chat_message("help",0,_("Show your ignore and friends list. Usage: /list display"));
						} else {
							add_chat_message("help",0,_("Unknown subcommand"));
						}
					} else {
						add_chat_message("help",0,_("Ignore messages from players on the ignore list and highlight players on the friends list. Usage: /list [subcommand] [argument](optional) Subcommands: addfriend addignore remove display clear. Type /help list [subcommand] for more info."));
					}
				} else if (command == "emote" || command == "me") {
					add_chat_message("help",0,_("Send an emotion or personal action in chat. Usage: /emote [message]"));
				} else {
					add_chat_message("help",0,_("Unknown command"));
				}
			} else {
				add_chat_message("help",0,help_chat_help);
			}
		} else if (message.size() > list.size() && std::equal(list.begin(),list.end(), message.begin())) {

			config* cignore;

			if (arg1 == addignore){
				if(preferences::_set_relationship(arg2, "ignored"))
				{
					add_chat_message("ignores list",0, _("Added to ignore list: ")+arg2,game_display::MESSAGE_PRIVATE);
				} else {
					add_chat_message("ignores list",0, _("Invalid username: ")+arg2,game_display::MESSAGE_PRIVATE);
				}
			} else if (arg1 == addfriend){
				if(preferences::_set_relationship(arg2, "friend"))
				{
					add_chat_message("friends list",0, _("Added to friends list: ")+arg2,game_display::MESSAGE_PRIVATE);
				} else {
					add_chat_message("friends list",0, _("Invalid username: ")+arg2,game_display::MESSAGE_PRIVATE);
                }
			} else if (arg1 == remove){
				if(preferences::_set_relationship(arg2, "no"))
				{
					add_chat_message("list",0, _("Removed from list: ")+arg2,game_display::MESSAGE_PRIVATE);
				} else {
					add_chat_message("list",0, _("Invalid username: ")+arg2,game_display::MESSAGE_PRIVATE);
				}
			} else if (arg1 == display){
				std::string text_ignore;
				std::string text_friend;
				if ((cignore = preferences::get_prefs()->child("relationship"))){
					for(std::map<std::string,t_string>::const_iterator i = cignore->values.begin();
							i != cignore->values.end(); ++i){
						if (i->second == "ignored"){
							text_ignore+=i->first+",";
						}
						if (i->second == "friend"){
							text_friend+=i->first+",";
						}
					}
					if(!text_ignore.empty()){
						text_ignore.erase(text_ignore.length()-1,1);
						add_chat_message("ignores list",0, text_ignore,game_display::MESSAGE_PRIVATE);
					}
					if(!text_friend.empty()){
						text_friend.erase(text_friend.length()-1,1);
						add_chat_message("friends list",0, text_friend,game_display::MESSAGE_PRIVATE);
					}
					if (text_friend.empty() && text_ignore.empty()) {
						add_chat_message("list",0, _("There are no players on your friends or ignore list."));
					}
				}
			} else if (arg1 == clear){

				if ((cignore = preferences::get_prefs()->child("relationship"))){
					string_map::iterator nick;
					for(nick= cignore->values.begin() ; nick!= cignore->values.end(); nick++) {
						if((*cignore)[nick->first] != "no") {
                            if((*cignore)[nick->first] == "ignored") {
							    add_chat_message("ignore list",0, _("Removed from ignore list: ")+nick->first,game_display::MESSAGE_PRIVATE);
						    }
                            if((*cignore)[nick->first] == "friend") {
							    add_chat_message("friend list",0, _("Removed from friends list: ")+nick->first,game_display::MESSAGE_PRIVATE);
						    }
							(*cignore)[nick->first] = "no";
						}
					}
				}
			} else {
				add_chat_message("list",0,_("Unknown command: ")+arg1,game_display::MESSAGE_PRIVATE);
			}
		} else if ((cmd == emote || cmd == emote2) && argc > 0) {
			//emote message
			send_chat_message("/me" + message.substr(cmd.size()), allies_only);
		} else if (is_command) {
			//command not accepted, show help chat help
			add_chat_message("help",0,help_chat_help);
		} else {
			//not a command, send as normal
			send_chat_message(message, allies_only);
		}
	}

	void menu_handler::send_chat_message(const std::string& message, bool allies_only)
	{
		config cfg;
		cfg["description"] = preferences::login();
		cfg["message"] = message;

		const int side = is_observer() ? 0 : gui_->viewing_team()+1;
		if(!is_observer()) {
			cfg["side"] = lexical_cast<std::string>(side);
		}

		bool private_message = has_friends() && allies_only;

		if(private_message) {
			if (is_observer()) {
				cfg["team_name"] = game_config::observer_team_name;
			} else {
				cfg["team_name"] = teams_[gui_->viewing_team()].team_name();
			}
		}

		recorder.speak(cfg);
		add_chat_message(cfg["description"],side,message,
							  private_message ? game_display::MESSAGE_PRIVATE : game_display::MESSAGE_PUBLIC);

	}


	void menu_handler::do_search(const std::string& new_search)
	{
		if(new_search.empty() == false && new_search != last_search_)
			last_search_ = new_search;

		if(last_search_.empty()) return;

		bool found = false;
		gamemap::location loc = last_search_hit_;
		//If this is a location search, just center on that location.
		std::vector<std::string> args = utils::split(last_search_, ',');
		if(args.size() == 2) {
			int x, y;
			x = lexical_cast_default<int>(args[0], 0)-1;
			y = lexical_cast_default<int>(args[1], 0)-1;
			if(x >= 0 && x < map_.w() && y >= 0 && y < map_.h()) {
				loc = gamemap::location(x,y);
				found = true;
			}
		}
		//Start scanning the game map
		if(loc.valid() == false)
			loc = gamemap::location(map_.w()-1,map_.h()-1);
		gamemap::location start = loc;
		while (!found) {
			//Move to the next location
			loc.x = (loc.x + 1) % map_.w();
			if(loc.x == 0)
				loc.y = (loc.y + 1) % map_.h();

			//Search label
			if (!gui_->shrouded(loc)) {
				const terrain_label* label = gui_->labels().get_label(loc);
				if(label) {
					if(std::search(label->text().begin(), label->text().end(),
							last_search_.begin(), last_search_.end(),
							chars_equal_insensitive) != label->text().end()) {
						found = true;
					}
				}
			}
			//Search unit name
			if (!gui_->fogged(loc)) {
				unit_map::const_iterator ui = units_.find(loc);
				if(ui != units_.end()) {
					const std::string name = ui->second.description();
					if(std::search(name.begin(), name.end(),
							last_search_.begin(), last_search_.end(),
							chars_equal_insensitive) != name.end()) {
						if (!teams_[gui_->viewing_team()].is_enemy(ui->second.side())
						    || !ui->second.invisible(ui->first, units_,teams_)) {
							found = true;
						}
					}
				}
			}

			if(loc == start)
				break;
		}

		if(found) {
			last_search_hit_ = loc;
			gui_->scroll_to_tile(loc,game_display::ONSCREEN,false);
			gui_->highlight_hex(loc);
		} else {
			last_search_hit_ = gamemap::location();
			//Not found, inform the player
			utils::string_map symbols;
			symbols["search"] = last_search_;
			const std::string msg = utils::interpolate_variables_into_string(
				_("Couldn't find label or unit containing the string '$search'."),&symbols);
			gui::dialog(*gui_,"",msg).show();
		}
	}

	void menu_handler::do_command(const std::string& str, const unsigned int team_num, mouse_handler& mousehandler)
	{
		const std::string::const_iterator i = std::find(str.begin(),str.end(),' ');
		const std::string cmd(str.begin(),i);
		const std::string data(i == str.end() ? str.end() : i+1,str.end());

		if(cmd == "refresh") {
			image::flush_cache();
			gui_->redraw_everything();
		} else if (cmd == "droid") {
			// default to the current side
			const unsigned int side = lexical_cast_default<unsigned int>(data, team_num);
			if (side < 1 || side > teams_.size() || teams_[side - 1].is_network()) {
				return;
			} else if (teams_[side - 1].is_human()) {
				//this is our side, so give it to AI
				teams_[side - 1].make_ai();
				textbox_info_.close(*gui_);
				if(team_num == side) {
					//if it is our turn at the moment, we have to indicate to the
					//play_controller, that we are no longer in control
					throw end_turn_exception(side);
				}
			} else if (teams_[side - 1].is_ai()) {
				teams_[side - 1].make_human();
			}
		} else if (cmd == "theme") {
		  preferences::show_theme_dialog(*gui_);
		} else if((cmd == "ban" || cmd == "kick") && network::nconnections() != 0) {
			config cfg;
			config& ban = cfg.add_child(cmd);
			ban["username"] = data;

			network::send_data(cfg);
		} else if (cmd == "mute" && network::nconnections() != 0) {
			config cfg;
			config& mute = cfg.add_child(cmd);
			if (!data.empty()) {
				mute["username"] = data;
			}

			network::send_data(cfg);
		} else if (cmd == "muteall" && network::nconnections() != 0) {
			network::send_data(config(cmd));
		} else if(cmd == "control" && network::nconnections() != 0) {
			const std::string::const_iterator j = std::find(data.begin(),data.end(),' ');
			if(j == data.end())
				return;
			const std::string side(data.begin(),j);
			const std::string player(j+1,data.end());
			unsigned int side_num;
			try {
				side_num = lexical_cast<unsigned int, std::string>(side);
			} catch(bad_lexical_cast&) {
				return;
			}
			if (side_num < 1 || side_num > teams_.size()) {
				return;
			}
			//if this is our side we are always allowed to change the controller
			if(teams_[side_num - 1].is_human()){
				if (player == preferences::login())
					return;
				change_side_controller(side,player,true);
				teams_[side_num - 1].make_network();
				textbox_info_.close(*gui_);
				if(team_num == side_num) {
					//if it is our turn at the moment, we have to indicate to the
					//play_controller, that we are no longer in control
					gui_->set_team(0);
					throw end_turn_exception(side_num);
				}
			} else {
				//it is not our side, the server will decide if we can change the
				//controller (that is if we are host of the game)
				change_side_controller(side,player);
			}
		} else if(cmd == "clear") {
			gui_->clear_chat_messages();
		} else if(game_config::debug && cmd == "sunset") {
			int delay = lexical_cast_default<int>(data);
			gui_->sunset(delay);
		} else if(cmd == "fps") {
			preferences::set_show_fps(!preferences::show_fps());
		} else if(cmd == "benchmark") {
			gui_->toggle_benchmark();
		} else if(cmd == "w") {
			save_game(data,gui::NULL_DIALOG);
		} else if(cmd == "wq") {
			save_game(data,gui::NULL_DIALOG);
			throw end_level_exception(QUIT);
		} else if(cmd == "q!" || cmd == "q") {
			throw end_level_exception(QUIT);
		} else if(cmd == "ignore_replay_errors") {
			game_config::ignore_replay_errors = (data != "off") ? true : false;
		} else if(cmd == "nosaves") {
			game_config::disable_autosave = (data != "off") ? true : false;
		} else if(cmd == "n" && game_config::debug) {
			throw end_level_exception(LEVEL_CONTINUE_NO_SAVE);
		} else if(cmd == "debug" && network::nconnections() == 0) {
			game_config::debug = true;
		} else if(cmd == "nodebug") {
			game_config::debug = false;
		} else if(game_config::debug && cmd == "set_var") {
				const std::string::const_iterator j = std::find(data.begin(),data.end(),'=');
				if(j != data.end()) {
					const std::string name(data.begin(),j);
					const std::string value(j+1,data.end());
					gamestate_.set_variable(name,value);
				}
		} else if(game_config::debug && cmd == "show_var") {
			gui::message_dialog to_show(*gui_,"",gamestate_.get_variable(data));
			to_show.show();
		} else if(game_config::debug && cmd == "unit") {
			const unit_map::iterator i = current_unit(mousehandler);
			if(i != units_.end()) {
				const std::string::const_iterator j = std::find(data.begin(),data.end(),'=');
				if(j != data.end()) {
					const std::string name(data.begin(),j);
					const std::string value(j+1,data.end());
					config cfg;
					i->second.write(cfg);
					cfg[name] = value;
					i->second = unit(&gameinfo_,&units_,&map_,&status_,&teams_,cfg);

					gui_->invalidate(i->first);
					gui_->invalidate_unit();
				}
			}
		} else if(game_config::debug && cmd == "buff") {
			const unit_map::iterator i = current_unit(mousehandler);
			if(i != units_.end()) {
			  	i->second.add_trait(data);
				gui_->invalidate(i->first);
				gui_->invalidate_unit();
			}
		} else if(game_config::debug && cmd == "unbuff") {
			const unit_map::iterator i = current_unit(mousehandler);
			if(i != units_.end()) {
				// FIXME: 'data' is the trait.  Clear it.

				gui_->invalidate(i->first);
				gui_->invalidate_unit();
			}
		} else if(game_config::debug && cmd == "create" && map_.on_board(mousehandler.get_last_hex())) {
			const game_data::unit_type_map::const_iterator i = gameinfo_.unit_types.find(data);
			if(i == gameinfo_.unit_types.end()) {
				return;
			}

			units_.erase(mousehandler.get_last_hex());
			units_.add(new std::pair<gamemap::location,unit>(mousehandler.get_last_hex(),unit(&gameinfo_,&units_,&map_,&status_,&teams_,&i->second,1,false)));
			gui_->invalidate(mousehandler.get_last_hex());
			gui_->invalidate_unit();
		} else if(game_config::debug && cmd == "fog") {
			teams_[team_num - 1].set_fog( !teams_[team_num - 1].uses_fog() );
			recalculate_fog(map_,status_,gameinfo_,units_,teams_, team_num - 1);
			gui_->redraw_everything();
		} else if(game_config::debug && cmd == "shroud") {
			teams_[team_num - 1].set_shroud( !teams_[team_num - 1].uses_shroud() );
			gui_->redraw_everything();
		} else if(game_config::debug && cmd == "gold") {
			teams_[team_num - 1].spend_gold(-lexical_cast_default<int>(data,1000));
			gui_->redraw_everything();
		} else if(game_config::debug && cmd == "throw") {
			game_events::fire(data);
			gui_->redraw_everything();
		}
	}

	void menu_handler::user_command()
	{
		textbox_info_.show(gui::TEXTBOX_COMMAND,sgettext("prompt^Command:"), "", false, *gui_);
	}

	void menu_handler::clear_messages()
	{
		gui_->clear_chat_messages();	// also clear debug-messages and WML-error-messages
	}

#ifdef USRCMD2
	// not used yet - for future hotkey-commands:
	void menu_handler::user_command_2()
	{
		gui::message_dialog(*gui_, "Test", "User-Command#2").show();
		//sound::play_bell(game_config::sounds::turn_bell);
		sound::play_bell("bell.wav");
	}

	void menu_handler::user_command_3()
	{
		gui::message_dialog(*gui_, "Info", _("User-Command#3")).show();
		//gui::show_error_message(disp(), "User-Command#3");
		//sound::play_sound("heal.wav");
		sound::play_sound("select.wav");
	}
#endif

	void menu_handler::change_side_controller(const std::string& side, const std::string& player, bool own_side)
	{
		config cfg;
		config& change = cfg.add_child("change_controller");
		change["side"] = side;
		change["player"] = player;

		if(own_side) {
			change["own_side"] = "yes";
		}

		network::send_data(cfg);
	}
} // end namespace events

