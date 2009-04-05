/* $Id$ */
/*
   Copyright (C) 2006 - 2009 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
   wesnoth playturn Copyright (C) 2003 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file menu_events.cpp
 * Operations activated from menus/hotkeys while playing a game.
 * E.g. Unitlist, status_table, save_game, save_map, chat, show_help, etc.
 */

#include "global.hpp"

#include "ai_manager.hpp"
#include "dialogs.hpp"
#include "formatter.hpp"
#include "filechooser.hpp"
#include "foreach.hpp"
#include "game_end_exceptions.hpp"
#include "game_events.hpp"
#include "gettext.hpp"
#include "gui/widgets/window.hpp"
#include "help.hpp"
#include "log.hpp"
#include "map.hpp"
#include "map_label.hpp"
#include "marked-up_text.hpp"
#include "menu_events.hpp"
#include "mouse_events.hpp"
#include "preferences_display.hpp"
#include "replay.hpp"
#include "savegame.hpp"
#include "sound.hpp"
#include "statistics_dialog.hpp"
#include "unit_display.hpp"
#include "wml_separators.hpp"
#include "formula_string_utils.hpp"
#include "widgets/combo.hpp"

#define ERR_NG LOG_STREAM(err, engine)
#define LOG_NG LOG_STREAM(info, engine)
#define DBG_NG LOG_STREAM(info, engine)

namespace {

void remove_old_auto_saves()
{
	const std::string auto_save = _("Auto-Save");
	int countdown = preferences::autosavemax();
	if (countdown == preferences::INFINITE_AUTO_SAVES)
		return;

	std::vector<save_info> games = get_saves_list(NULL, &auto_save);
	for (std::vector<save_info>::iterator i = games.begin(); i != games.end(); i++) {
		if (countdown-- <= 0) {
			LOG_NG << "Deleting savegame '" << i->name << "'\n";
			delete_game(i->name);
		}
	}
}

} // end anonymous namespace

namespace events{

	class delete_recall_unit : public gui::dialog_button_action
	{
	public:
		delete_recall_unit(game_display& disp, gui::filter_textbox& filter, std::vector<unit>& units) : disp_(disp), filter_(filter), units_(units) {}
	private:
		gui::dialog_button_action::RESULT button_pressed(int menu_selection);

		game_display& disp_;
		gui::filter_textbox& filter_;
		std::vector<unit>& units_;
	};

	gui::dialog_button_action::RESULT delete_recall_unit::button_pressed(int menu_selection)
	{
		const size_t index = size_t(filter_.get_index(menu_selection));
		if(index < units_.size()) {
			const unit& u = units_[index];

			//If the unit is of level > 1, or is close to advancing,
			//we warn the player about it
			std::stringstream message;
			if (u.loyal()) {
				message << _("My lord, this unit is loyal and requires no upkeep! ") << (u.gender() == unit_race::MALE ? _("Do you really want to dismiss him?")
						: _("Do you really want to dismiss her?"));
			} else if(u.level() > 1) {
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
			// Remove the item from filter_textbox memory
			filter_.delete_item(menu_selection);

			units_.erase(units_.begin() + index);
			recorder.add_disband(index);
			return gui::DELETE_ITEM;
		} else {
			return gui::CONTINUE_DIALOG;
		}
	}

	menu_handler::menu_handler(game_display* gui, unit_map& units, std::vector<team>& teams,
			const config& level, const gamemap& map,
			const config& game_config, const gamestatus& status, game_state& gamestate,
			undo_list& undo_stack, undo_list& redo_stack) :
		gui_(gui),
		units_(units),
		teams_(teams),
		level_(level),
		map_(map),
		game_config_(game_config),
		status_(status),
		gamestate_(gamestate),
		undo_stack_(undo_stack),
		redo_stack_(redo_stack),
		textbox_info_(),
		last_search_(),
		last_search_hit_(),
		last_recruit_()
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

	std::string menu_handler::get_title_suffix(int team_num)
	{
		int controlled_recruiters = 0;
		for(size_t i = 0; i < teams_.size(); ++i) {
			if(teams_[i].is_human() && !teams_[i].recruits().empty()
			&& team_leader(i+1, units_) != units_.end()) {
			++controlled_recruiters;
			}
		}
		std::stringstream msg;
		if(controlled_recruiters >= 2) {
			const unit_map::const_iterator leader = team_leader(team_num, units_);
			if(leader != units_.end() && !leader->second.name().empty()) {
				msg << " (" << leader->second.name(); msg << ")";
			}
		}
		return msg.str();
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
		statistics_dialog stats_dialog(*gui_,
					       title_str.str(),
					       team_num,
					       teams_[team_num - 1].save_id(),
					       player);
		stats_dialog.show();
	}

	void menu_handler::unit_list()
	{
		const std::string heading = std::string(1,HEADING_PREFIX) +
									_("Type")          + COLUMN_SEPARATOR +
									_("Name")          + COLUMN_SEPARATOR +
									_("Level^Lvl.")    + COLUMN_SEPARATOR +
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

		std::vector<map_location> locations_list;
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
			/** @todo TODO: hero just has overlay "misc/hero-icon.png" - needs an ability to query */

			if(i->second.can_recruit() ) {
//				row << "<255,255,200>";
                row << "<205,173,0>";   // gold3
			}
			row << i->second.type_name() << COLUMN_SEPARATOR;
			if(i->second.can_recruit() ) {
//				row << "<255,255,200>";
                row << "<205,173,0>";   // gold3
			}
			row << i->second.name()   << COLUMN_SEPARATOR;

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

			/** @todo FIXME: condition for "invisible" does not work */
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
			const map_location& loc = locations_list[selected];
			gui_->scroll_to_tile(loc,game_display::WARP);
			gui_->select_hex(loc);
		}
	}

namespace {
class leader_scroll_dialog : public gui::dialog {
public:
	leader_scroll_dialog(display &disp, const std::string &title,
			std::vector<bool> &leader_bools, int selected,
			gui::DIALOG_RESULT extra_result) :
		dialog(disp, title, "", gui::NULL_DIALOG),
		scroll_btn_(new gui::standard_dialog_button(disp.video(), _("Scroll To"), 0, false)),
		leader_bools_(leader_bools),
		extra_result_(extra_result)
	{
		scroll_btn_->enable(leader_bools[selected]);
		add_button(scroll_btn_, gui::dialog::BUTTON_STANDARD);
		add_button(new gui::standard_dialog_button(disp.video(),
			_("Close"), 1, true), gui::dialog::BUTTON_STANDARD);
	}
	void action(gui::dialog_process_info &info) {
		const bool leader_bool = leader_bools_[get_menu().selection()];
		scroll_btn_->enable(leader_bool);
		if(leader_bool && (info.double_clicked || (!info.key_down
		&& (info.key[SDLK_RETURN] || info.key[SDLK_KP_ENTER])))) {
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

		unsigned total_villages = 0;
		// a variable to check if there are any teams to show in the table
		bool status_table_empty = true;

		//if the player is under shroud or fog, they don't get
		//to see details about the other sides, only their own
		//side, allied sides and a ??? is shown to demonstrate
		//lack of information about the other sides But he see
		//all names with in colours
		for(size_t n = 0; n != teams_.size(); ++n) {
			if(teams_[n].is_empty()||teams_[n].hidden()) {
				continue;
			}
			status_table_empty=false;

			const bool known = viewing_team.knows_about_team(n, network::nconnections() > 0);
			const bool enemy = viewing_team.is_enemy(n+1);

			std::stringstream str;

			const team_data data = calculate_team_data(teams_[n],n+1,units_);

			const unit_map::const_iterator leader = team_leader(n+1,units_);
			std::string leader_name;
			//output the number of the side first, and this will
			//cause it to be displayed in the correct colour
			if(leader != units_.end()) {

				// Add leader image. If it's fogged
				// show only a random leader image.
				if (known || game_config::debug) {
					str << IMAGE_PREFIX << leader->second.absolute_image();
					leader_bools.push_back(true);
					leader_name = leader->second.name();
				} else {
					str << IMAGE_PREFIX << std::string("units/unknown-unit.png");
					leader_bools.push_back(false);
					leader_name = "Unknown";
				}
			if (gamestate_.campaign_type == "multiplayer")
		       		leader_name = teams_[n].current_player();

#ifndef LOW_MEM
				str << "~RC(" << leader->second.team_color() << ">" << team::get_side_colour_index(n+1) << ")";
#endif
			} else {
				leader_bools.push_back(false);
			}
			str << COLUMN_SEPARATOR	<< team::get_side_highlight(n)
			    << leader_name << COLUMN_SEPARATOR
			    << (data.teamname.empty() ? teams_[n].team_name() : data.teamname)
			    << COLUMN_SEPARATOR;

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
			total_villages += data.villages;
			items.push_back(str.str());
		}
		if (total_villages > map_.villages().size()) {
			ERR_NG << "Logic error: map has " << map_.villages().size() << " villages but status table shows " << total_villages << " owned in total\n";
		}

		if (status_table_empty)
 		{
 			// no sides to show - display empty table
 			std::stringstream str;
 			str << " ";
 			for (int i=0;i<7;++i)
 				str << COLUMN_SEPARATOR << " ";
 			leader_bools.push_back(false);
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
		bool settings_table_empty = true;

		for(size_t n = 0; n != teams_.size(); ++n) {
			if(teams_[n].is_empty()||teams_[n].hidden()) {
				continue;
			}
			settings_table_empty = false;

			std::stringstream str;
			const unit_map::const_iterator leader = team_leader(n+1, units_);

			if(leader != units_.end()) {
				// Add leader image. If it's fogged
				// show only a random leader image.
				if (viewing_team.knows_about_team(n, network::nconnections() > 0) || game_config::debug) {
					str << IMAGE_PREFIX << leader->second.absolute_image();
					leader_bools.push_back(true);
				} else {
					str << IMAGE_PREFIX << std::string("units/unknown-unit.png");
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

		if (settings_table_empty)
 		{
 			// no sides to show - display empty table
 			std::stringstream str;
 			for (int i=0;i<8;++i)
 				str << " " << COLUMN_SEPARATOR;
 			leader_bools.push_back(false);
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
		stream << ellipsed_name << " " << _("Turn") << " " << status_.turn();
		std::string label = stream.str();
		if(dialog_type == gui::NULL_DIALOG && message != "") {
			label = message;
		}

		label.erase(std::remove_if(label.begin(), label.end(),
		            dialogs::is_illegal_file_char), label.end());

		const int res = dialog_type == gui::NULL_DIALOG ? 0
			: dialogs::get_save_name(*gui_,message, _("Name: "), &label,dialog_type, "", has_exit_button);

		if(res == 0) {
			config snapshot;
			write_game_snapshot(snapshot);
			gamestate_.replay_data = recorder.get_replay_data();
			try {
				::save_game(label, snapshot, gamestate_);

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

	void menu_handler::save_replay(const std::string& message, gui::DIALOG_TYPE dialog_type,
		const bool has_exit_button)
	{
		std::stringstream stream;

		const std::string ellipsed_name = font::make_text_ellipsis(gamestate_.label,
				font::SIZE_NORMAL, 200);
		stream << ellipsed_name << " " << _("replay");
		std::string label = stream.str();
		if(dialog_type == gui::NULL_DIALOG && message != "") {
			label = message;
		}

		label.erase(std::remove_if(label.begin(), label.end(),
		            dialogs::is_illegal_file_char), label.end());

		const int res = dialog_type == gui::NULL_DIALOG ? 0
			: dialogs::get_save_name(*gui_,message, _("Name: "), &label,dialog_type, "", has_exit_button);

		if(res == 0) {
			config snapshot;
			gamestate_.replay_data = recorder.get_replay_data();
			try {
				::save_replay(label, gamestate_);

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
			res = dialogs::show_file_chooser_dialog_save(*gui_, input_name, _("Save the Map As"));
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
		start.merge_attributes(level_);

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
		foreach (const config &tg, level_.child_range("terrain_graphics")) {
			start.add_child("terrain_graphics", tg);
		}

		sound::write_music_play_list(start);

		gamestate_.write_snapshot(start);

		//write out the current state of the map
		start["map_data"] = map_.write();

		gui_->labels().write(start);
	}

	void menu_handler::autosave(unsigned turn) const
	{
		if(game_config::disable_autosave)
			return;

		Uint32 start, end;
		start = SDL_GetTicks();
		config snapshot;
		write_game_snapshot(snapshot);

		try {
			std::string savename = ::save_autosave(turn, snapshot, gamestate_);
			end = SDL_GetTicks();
			LOG_NG << "Milliseconds to save " << savename << ": " << end - start << "\n";
		} catch(game::save_game_failed&) {
			gui::message_dialog(*gui_,"",_("Could not auto save the game. Please save the game manually.")).show();
			//do not bother retrying, since the user can just save the game
			//maybe show a yes-no dialog for "disable autosaves now"?
		}

		remove_old_auto_saves();
	}

	void menu_handler::load_game(){
		bool show_replay = false;
		bool cancel_orders = false;
		const std::string game = dialogs::load_game_dialog(*gui_, game_config_, &show_replay, &cancel_orders);
		if(game != "") {
			throw game::load_game_exception(game,show_replay,cancel_orders);
		}
	}

	void menu_handler::preferences()
	{
		preferences::show_preferences_dialog(*gui_, game_config_);
		gui_->redraw_everything();
	}

	void menu_handler::show_chat_log()
	{
		std::string text = recorder.build_chat_log();
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

	void menu_handler::recruit(const bool browse, const unsigned int team_num, const map_location& last_hex)
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
					u_type = unit_type_data::types().find_unit_type(*it);
			if(u_type == unit_type_data::types().end()) {
				ERR_NG << "could not find unit '" << *it << "'\n";
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
			description << COLUMN_SEPARATOR << font::LARGE_TEXT << prefix << type->type_name() << "\n"
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
			dialogs::unit_types_preview_pane unit_preview(*gui_,&map_,sample_units,NULL,team_num);
			std::vector<gui::preview_pane*> preview_panes;
			preview_panes.push_back(&unit_preview);

			gui::dialog rmenu(*gui_,_("Recruit") + get_title_suffix(team_num),
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

	void menu_handler::repeat_recruit(const unsigned int team_num, const map_location& last_hex)
	{
		if(last_recruit_.empty() == false)
			do_recruit(last_recruit_, team_num, last_hex);
	}

	void menu_handler::do_recruit(const std::string& name, const int unsigned team_num, const map_location& last_hex)
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
				u_type = unit_type_data::types().find_unit_type(name);
		assert(u_type != unit_type_data::types().end());

		if(u_type->second.cost() > current_team.gold()) {
			gui::message_dialog(*gui_,"",
				 _("You don't have enough gold to recruit that unit")).show();
		} else {
			last_recruit_ = name;

			//create a unit with traits
			recorder.add_recruit(recruit_num, last_hex);
			unit new_unit(&units_,&map_,&status_,&teams_,&(u_type->second),team_num,true);
			map_location loc = last_hex;
			const std::string& msg = recruit_unit(map_,team_num,units_,new_unit,loc,false,(gui_!=NULL));
			if(msg.empty()) {
				current_team.spend_gold(u_type->second.cost());
				statistics::recruit_unit(new_unit);

				//MP_COUNTDOWN grant time bonus for recruiting
				current_team.set_action_bonus_count(1 + current_team.action_bonus_count());

				redo_stack_.clear();
				assert(new_unit.type());

				// Dissallow undoing of recruits. Can be enabled again once the unit's
				// description= key doesn't use random anymore.
				const bool shroud_cleared = clear_shroud(team_num);
				if(shroud_cleared || new_unit.type()->genders().size() > 1
						|| new_unit.type()->has_random_traits()) {
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

	void menu_handler::recall(const unsigned int team_num, const map_location& last_hex)
	{
		if(utils::string_bool(level_["disallow_recall"])) {
			gui::message_dialog(*gui_,"",_("You are separated from your soldiers and may not recall them")).show();
			return;
		}

		player_info *player = gamestate_.get_player(teams_[team_num-1].save_id());
		if(!player) {
			ERR_NG << "cannot recall a unit for side " << team_num
				<< ", which has no recall list!\n";
			return;
		}

		team& current_team = teams_[team_num-1];
		std::vector<unit>& recall_list = player->available_units;

		//sort the available units into order by value
		//so that the most valuable units are shown first
		sort_units(recall_list);

		gui_->draw(); //clear the old menu

		if(recall_list.empty()) {
			gui::message_dialog(*gui_,"",_("There are no troops available to recall\n(You must have veteran survivors from a previous scenario)")).show();
		} else {
			std::vector<std::string> options, options_to_filter;

			std::ostringstream heading;
			heading << HEADING_PREFIX << COLUMN_SEPARATOR << _("Type")
					<< COLUMN_SEPARATOR << _("Name")
					<< COLUMN_SEPARATOR << _("Level^Lvl.")
					<< COLUMN_SEPARATOR << _("XP");
#ifndef USE_TINY_GUI
			heading << COLUMN_SEPARATOR << _("Traits");
#endif

			gui::menu::basic_sorter sorter;
			sorter.set_alpha_sort(1).set_alpha_sort(2).set_id_sort(3).set_numeric_sort(4).set_alpha_sort(5);

			options.push_back(heading.str());
			options_to_filter.push_back(options.back());

			for(std::vector<unit>::const_iterator u = recall_list.begin(); u != recall_list.end(); ++u) {
				std::stringstream option, option_to_filter;
				const std::string& name = u->name().empty() ? "-" : u->name();

				option << IMAGE_PREFIX << u->absolute_image();
#ifndef LOW_MEM
				option << "~RC("  << u->team_color() << ">" << team::get_side_colour_index(team_num) << ")";
#endif
				option << COLUMN_SEPARATOR
					<< u->type_name() << COLUMN_SEPARATOR
					<< name << COLUMN_SEPARATOR
					<< u->level() << COLUMN_SEPARATOR
					<< u->experience() << "/";
				if (u->can_advance())
					option << u->max_experience();
				else
					option <<  "-";


				option_to_filter << u->type_name() << " " << name << " " << u->level();

#ifndef USE_TINY_GUI
				option << COLUMN_SEPARATOR;
				const std::vector<std::string> traits =
						 utils::split(std::string(u->traits_description()), ',');
				foreach(const std::string& trait, traits) {
					option << trait << '\n';
					option_to_filter << " " << trait;
				}
#endif

				options.push_back(option.str());
				options_to_filter.push_back(option_to_filter.str());
			}

			int res = 0;

			{
				gui::dialog rmenu(*gui_,_("Recall") + get_title_suffix(team_num),
						  _("Select unit:") + std::string("\n"),
						  gui::OK_CANCEL,
						  gui::dialog::default_style);
				rmenu.set_menu(options, &sorter);

				gui::filter_textbox* filter = new gui::filter_textbox(gui_->video(),
				_("Filter: "), options, options_to_filter, 1, rmenu, 200);
				rmenu.set_textbox(filter);

				delete_recall_unit recall_deleter(*gui_, *filter, recall_list);
				gui::dialog_button_info delete_button(&recall_deleter,_("Dismiss Unit"));
				rmenu.add_button(delete_button);

				rmenu.add_button(new help::help_button(*gui_,"recruit_and_recall"),
					gui::dialog::BUTTON_HELP);

				dialogs::units_list_preview_pane unit_preview(*gui_,&map_,recall_list, filter);
				rmenu.add_pane(&unit_preview);

				res = rmenu.show();
				res = filter->get_index(res);
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
					LOG_NG << "recall index: " << res << "\n";
					unit& un = recall_list[res];
					map_location loc = last_hex;
					recorder.add_recall(res,loc);
					un.set_game_context(&units_,&map_,&status_,&teams_);
					const std::string err = recruit_unit(map_,team_num,units_,un,loc,true,(gui_!=NULL));
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
	void menu_handler::undo(const unsigned int team_num)
	{
		if(undo_stack_.empty())
			return;

		const events::command_disabler disable_commands;

		undo_action& action = undo_stack_.back();
		if(action.is_recall()) {
			player_info* const player = gamestate_.get_player(teams_[team_num - 1].save_id());

			if(player == NULL) {
				ERR_NG << "trying to undo a recall for side " << team_num
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
			assert(un.type());
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
			std::vector<map_location> route = action.route;
			std::reverse(route.begin(),route.end());
			const unit_map::iterator u = units_.find(route.front());
			const unit_map::iterator u_end = units_.find(route.back());
			if(u == units_.end() || u_end != units_.end()) {
				//this can actually happen if the scenario designer has abused the [allow_undo] command
				ERR_NG << "Illegal 'undo' found. Possible abuse of [allow_undo]?\n";
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

			unit_display::move_unit(route,u->second,teams_);
			std::pair<map_location,unit> *up = units_.extract(u->first);
			up->second.set_goto(map_location());
			up->second.set_movement(starting_moves);
			up->first = route.back();
			units_.insert(up);
			unit::clear_status_caches();
			up->second.set_standing(up->first);
			gui_->invalidate(route.back());
			gui_->draw();
		}

		gui_->invalidate_unit();
		gui_->invalidate_game_status();

		redo_stack_.push_back(action);
		undo_stack_.pop_back();

		recorder.undo();

		const bool shroud_cleared = clear_shroud(team_num);

		if(shroud_cleared) {
			gui_->recalculate_minimap();
		} else {
			gui_->redraw_minimap();
		}
	}

	void menu_handler::redo(const unsigned int team_num)
	{
		if(redo_stack_.empty())
			return;

		const events::command_disabler disable_commands;

		undo_action& action = redo_stack_.back();
		if(action.is_recall()) {
			player_info *player=gamestate_.get_player(teams_[team_num - 1].save_id());
			if(!player) {
				ERR_NG << "trying to redo a recall for side " << team_num
					<< ", which has no recall list!\n";
			} else {
				// Redo recall
				std::vector<unit>& recall_list = player->available_units;
				unit un = recall_list[action.recall_pos];

				recorder.add_recall(action.recall_pos,action.recall_loc);
				un.set_game_context(&units_,&map_,&status_,&teams_);
				const std::string& msg = recruit_unit(map_,team_num,units_,un,action.recall_loc,true,(gui_!=NULL));
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
			map_location loc = action.recall_loc;
			const std::string name = action.affected_unit.type_id();

			//search for the unit to be recruited in recruits
			int recruit_num = 0;
			const std::set<std::string>& recruits = current_team.recruits();
			for(std::set<std::string>::const_iterator r = recruits.begin(); ; ++r) {
				if (r == recruits.end()) {
					ERR_NG << "trying to redo a recruit for side " << team_num
						<< ", which does not recruit type \"" << name << "\"\n";
					assert(false);
					return;
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
			const std::string& msg = recruit_unit(map_,team_num,units_,new_unit,loc,false,(gui_!=NULL));
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
			std::vector<map_location> route = action.route;
			const unit_map::iterator u = units_.find(route.front());
			if(u == units_.end()) {
				assert(false);
				return;
			}

			action.starting_moves = u->second.movement_left();

			unit_display::move_unit(route,u->second,teams_);
			std::pair<map_location,unit> *up = units_.extract(u->first);
			up->second.set_goto(map_location());
			up->second.set_movement(starting_moves);
			up->first = route.back();
			units_.insert(up);
			unit::clear_status_caches();
			up->second.set_standing(up->first);

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

			recorder.add_movement(action.route);
		}
		gui_->invalidate_unit();
		gui_->invalidate_game_status();

		undo_stack_.push_back(action);
		redo_stack_.pop_back();
	}

	bool menu_handler::clear_shroud(const unsigned int team_num)
	{
		bool cleared = teams_[team_num - 1].auto_shroud_updates() &&
			::clear_shroud(*gui_,map_,units_,teams_,team_num-1);
		return cleared;
	}

	void menu_handler::clear_undo_stack(const unsigned int team_num)
	{
		if(teams_[team_num - 1].auto_shroud_updates() == false)
			apply_shroud_changes(undo_stack_,gui_,map_,units_,teams_,team_num-1);
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
				const paths& path = paths(map_,units_,
										  u->first,teams_,false,teleports,teams_[gui_->viewing_team()], 0,false, ignore_units);

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
		int units_alive = 0;
		for(unit_map::const_iterator un = units_.begin(); un != units_.end(); ++un) {
			if(un->second.side() == team_num) {
				units_alive++;
				if(unit_can_move(un->first,un->second,units_,map_,teams_)) {
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
		if (preferences::confirm_no_moves() && units_alive && !some_units_have_moved) {
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

		std::string name = un->second.name();
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
		static int last_selection = -1;
		static bool random_gender = false;
		std::vector<const unit_type*> unit_choices;
		const std::string heading = std::string(1,HEADING_PREFIX) +
									_("Race")      + COLUMN_SEPARATOR +
									_("Type");
		options.push_back(heading);

		for(unit_type_data::unit_type_map::const_iterator i = unit_type_data::types().begin(); i != unit_type_data::types().end(); ++i) {
			std::stringstream row;

            unit_type_data::types().find_unit_type(i->first, unit_type::HELP_INDEX);

			std::string race;
			const race_map::const_iterator race_it = unit_type_data::types().races().find(i->second.race());
			if (race_it != unit_type_data::types().races().end()) {
				race = race_it->second.plural_name();
			}
			row << race << COLUMN_SEPARATOR;
			row << i->second.type_name() << COLUMN_SEPARATOR;

			options.push_back(row.str());
			unit_choices.push_back(&(i->second));
		}

		int choice = 0;
		bool random_gender_choice = random_gender;
		{
			gui::dialog umenu(*gui_, _("Create Unit (Debug!)"), "", gui::OK_CANCEL);

			umenu.add_option(
				(formatter()<<_("Gender: ")<<_("gender^Random")).str(),
				random_gender_choice,
				gui::dialog::BUTTON_EXTRA
			);

			gui::menu::basic_sorter sorter;
			sorter.set_alpha_sort(0).set_alpha_sort(1);
			umenu.set_menu(options, &sorter);

			gui::filter_textbox* filter = new gui::filter_textbox(gui_->video(),
				_("Filter: "), options, options, 1, umenu, 200);
			umenu.set_textbox(filter);

			//sort by race then by type name
			umenu.get_menu().sort_by(1);
			umenu.get_menu().sort_by(0);
			if (last_selection >= 0)
				umenu.get_menu().move_selection(last_selection);
			else
				umenu.get_menu().reset_selection();

			dialogs::unit_types_preview_pane unit_preview(*gui_, &map_, unit_choices, filter, 1, dialogs::unit_types_preview_pane::SHOW_ALL);
			umenu.add_pane(&unit_preview);
			unit_preview.set_selection(umenu.get_menu().selection());

			choice = umenu.show();
			choice = filter->get_index(choice);
			random_gender_choice = umenu.option_checked(0);
		}

		if (size_t(choice) < unit_choices.size()) {
			last_selection = choice;
			random_gender  = random_gender_choice;

			const std::vector<unit_race::GENDER>& genders = (*unit_choices[choice]).genders();
			const unit_race::GENDER gender =
				(!genders.empty() ? genders[gamestate_.rng().get_random() % genders.size()] : unit_race::MALE);

			unit chosen(&units_,&map_,&status_,&teams_,unit_choices[choice],1,false,false,gender,"",random_gender);
			chosen.new_turn();
			units_.replace(mousehandler.get_last_hex(), chosen);

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
			d.set_textbox(_("Label: "), old_label->text(), map_labels::get_max_chars());
			team_only = !old_label->team_name().empty();
		} else {
			d.set_textbox(_("Label: "), "", map_labels::get_max_chars());
		}
		d.add_option(_("Team only"), team_only, gui::dialog::BUTTON_CHECKBOX_LEFT);

		if(!d.show()) {
			std::string team_name;
			SDL_Color colour = font::LABEL_COLOUR;
			std::ostringstream last_team_id;
			last_team_id << gamemap::MAX_PLAYERS;
			std::map<std::string, color_range>::iterator gp = game_config::team_rgb_range.find(last_team_id.str());

			if (d.option_checked()) {
				team_name = gui_->labels().team_name();
			} else {
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

	void menu_handler::move_unit_to_loc(const unit_map::const_iterator& ui, const map_location& target, bool continue_move, const unsigned int team_num, mouse_handler& mousehandler)
	{
		assert(ui != units_.end());

		paths::route route = mousehandler.get_route(ui, target, teams_[team_num - 1]);

		if(route.steps.empty())
			return;

		assert(route.steps.front() == ui->first);

		gui_->set_route(&route);
		move_unit(gui_,map_,units_,teams_,route.steps,&recorder,&undo_stack_,NULL,continue_move);
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


	void menu_handler::add_chat_message(const time_t& time,
			const std::string& speaker, int side, const std::string& message,
			game_display::MESSAGE_TYPE type)
	{
		gui_->add_chat_message(time, speaker, side, message, type, false);
	}

	//simple command args parser, separated from command_handler for clarity.
	//a word begins with a nonspace
	//n-th arg is n-th word up to the next space
	//n-th data is n-th word up to the end
	//cmd is 0-th arg, begins at 0 always.
	class cmd_arg_parser
	{
		public:
			cmd_arg_parser() :
				str_(""),
				args(1, 0),
				args_end(false)
			{
			}

			explicit cmd_arg_parser(const std::string& str) :
				str_(str),
				args(1, 0),
				args_end(false)
			{
			}

			void parse(const std::string& str)
			{
				str_ = str;
				args.clear();
				args.push_back(0);
				args_end = false;
			}

			const std::string& get_str() const
			{
				return str_;
			}
			std::string get_arg(unsigned n) const
			{
				advance_to_arg(n);
				if (n < args.size()) {
					return std::string(str_, args[n], str_.find(' ', args[n]) - args[n]);
				} else {
					return "";
				}
			}
			std::string get_data(unsigned n) const
			{
				advance_to_arg(n);
				if (n < args.size()) {
					return std::string(str_, args[n]);
				} else {
					return "";
				}
			}
			std::string get_cmd() const
			{
				return get_arg(0);
			}
		private:
			cmd_arg_parser& operator=(const cmd_arg_parser&);
			cmd_arg_parser(const cmd_arg_parser&);
			void advance_to_arg(unsigned n) const
			{
				while (n < args.size() && !args_end) {
					size_t first_space = str_.find_first_of(' ', args.back());
					size_t next_arg_begin = str_.find_first_not_of(' ', first_space);
					if (next_arg_begin != std::string::npos) {
						args.push_back(next_arg_begin);
					} else {
						args_end = true;
					}
				}
			}
			std::string str_;
			mutable std::vector<size_t> args;
			mutable bool args_end;
	};

	//A helper class template with a slim public interface
	//This represents a map of strings to void()-member-function-of-Worker-pointers
	//with all the common functionality like general help, command help and aliases
	//Usage (of a derived class): Derived(specific-arguments) d; d.dispatch(command);
	//Derived classes should override virtual functions where noted.
	//The template parameter currently must be the dervived class itself,
	//i.e. class X : public map_command_handler<X>
	//To add a new command in a derived class:
	//  * add a new private void function() to the derived class
	//  * add it to the function map in init_map there, setting flags like
	//    "D" for debug only (checking the flag is also done in the derived class)
	//  * remember to add some help and/or usage information in init_map()
	template <class Worker>
	class map_command_handler
	{
		public:
			typedef void (Worker::*command_handler)();
			struct command
			{
				command_handler handler;
				std::string help; //long help text
				std::string usage; //only args info
				std::string flags;
				explicit command(command_handler h, const std::string help="",
					const std::string& usage="", const std::string flags="")
				: handler(h), help(help), usage(usage), flags(flags)
				{
				}
				bool has_flag(const char f) const
				{
					return flags.find(f) != flags.npos;
				}
				command& add_flag(const char f)
				{
					flags += f;
					return *this;
				}
			};
			typedef std::map<std::string, command> command_map;
			typedef std::map<std::string, std::string> command_alias_map;

			map_command_handler() : cap_("")
			{
			}

			virtual ~map_command_handler() {}

			bool empty() const
			{
				return command_map_.empty();
			}
			bool has_command(const std::string& cmd) const
			{
				return get_command(cmd) != 0;
			}
			//actual work function
			void dispatch(std::string cmd)
			{
				if (empty()) {
					init_map_default();
					init_map();
				}

				// We recursively resolve alias (100 max to avoid infinite recursion)
				for (int i=0; i < 100; ++i) {
					parse_cmd(cmd);
					std::string actual_cmd = get_actual_cmd(get_cmd());
					if (actual_cmd == get_cmd())
						break;
					std::string data = get_data(1);
					// translate the command and add space + data if any
					cmd = actual_cmd + (data.empty() ? "" : " ") + data;
				}

				if (get_cmd().empty()) {
					return;
				}

				if (const command* c = get_command(get_cmd())) {
					if (is_enabled(*c)) {
						(static_cast<Worker*>(this)->*(c->handler))();
					} else {
						print(get_cmd(), _("This command is currently unavailable."));
					}
				} else if (help_on_unknown_) {
					print("help", "Unknown command (" + get_cmd() + "), try " + cmd_prefix_ + "help "
						"for a list of available commands.");
				}
			}
		protected:
			void init_map_default()
			{
				register_command("help", &map_command_handler<Worker>::help,
					_("Available commands list and command-specific help. "
					"Use \"help all\" to include currently unavailable commands."), "[all|<command>]");
			}
			//derived classes initialize the map overriding this function
			virtual void init_map() = 0;
			//overriden in derived classes to actually print the messages somwehere
			virtual void print(const std::string& title, const std::string& message) = 0;
			//should be overriden in derived classes if the commands have flags
			//this should return a string describing what all the flags mean
			virtual std::string get_flags_description() const
			{
				return "";
			}
			//this should return a string describing the flags of the given command
			virtual std::string get_command_flags_description(const command& /*c*/) const
			{
				return "";
			}
			//this should be overriden if e.g. flags are used to control command
			//availability. Return false if the command should not be executed by dispatch()
			virtual bool is_enabled(const command& /*c*/) const
			{
				return true;
			}
			virtual void parse_cmd(const std::string& cmd_string)
			{
				cap_.parse(cmd_string);
			}
			//safe n-th argunment getter
			virtual std::string get_arg(unsigned argn) const
			{
				return cap_.get_arg(argn);
			}
			//"data" is n-th arg and everything after it
			virtual std::string get_data(unsigned argn = 1) const
			{
				return cap_.get_data(argn);
			}
			virtual std::string get_cmd() const
			{
				return cap_.get_cmd();
			}
			//command error reporting shorthands
			void command_failed(const std::string& message)
			{
				print(get_cmd(), "Error: " + message);
			}
			void command_failed_need_arg(int argn)
			{
				command_failed("Missing argument " + lexical_cast<std::string>(argn));
			}
			void print_usage()
			{
				help_command(get_cmd());
			}
			//take aliases into account
			std::string get_actual_cmd(const std::string& cmd) const
			{
				command_alias_map::const_iterator i = command_alias_map_.find(cmd);
				return i != command_alias_map_.end() ? i->second : cmd;
			}
			const command* get_command(const std::string& cmd) const
			{
				typename command_map::const_iterator i = command_map_.find(cmd);
				return i != command_map_.end() ? &i->second : 0;
			}
			command* get_command(const std::string& cmd)
			{
				typename command_map::iterator i = command_map_.find(cmd);
				return i != command_map_.end() ? &i->second : 0;
			}
			void help()
			{
				//print command-specific help if available, otherwise list commands
				if (help_command(get_arg(1))) {
					return;
				}
				std::stringstream ss;
				bool show_unavail = show_unavailable_ || get_arg(1) == "all";
				BOOST_FOREACH(typename command_map::value_type i, command_map_) {
					if (show_unavail || is_enabled(i.second)) {
						ss << i.first;
						//if (!i.second.usage.empty()) {
						//	ss << " " << i.second.usage;
						//}
						//uncomment the above to display usage information in command list
						//which might clutter it somewhat
						if (!i.second.flags.empty()) {
							ss << " (" << i.second.flags << ") ";
						}
						ss << "; ";
					}
				}
				print("help", "Available commands " + get_flags_description() + ":\n" + ss.str());
				print("help", "Type " + cmd_prefix_ + "help <command> for more info.");
			}
			//returns true if the command exists.
			bool help_command(const std::string& acmd)
			{
				std::string cmd = get_actual_cmd(acmd);
				const command* c = get_command(cmd);
				if (c) {
					std::stringstream ss;
					ss << cmd_prefix_ << cmd;
					if (c->help.empty() && c->usage.empty()) {
						ss << _(" No help available.");
					} else {
						ss << " - " << c->help;
					}
					if (!c->usage.empty()) {
						ss << " Usage: " << cmd_prefix_ << cmd << " " << c->usage;
					}
					ss << get_command_flags_description(*c);
					const std::vector<std::string> l = get_aliases(cmd);
					if (!l.empty()) {
						ss << " (aliases: " << utils::join(l,' ') << ")";
					}
					print("help", ss.str());
				}
				return c != 0;
			}
			cmd_arg_parser cap_;
		protected:
			//show a "try help" message on unknown command?
			static void set_help_on_unknown(bool value)
			{
				help_on_unknown_ = value;
			}
			//show all commands in help regardless whether they are active
			static void set_show_unavailable(bool value)
			{
				show_unavailable_ = value;
			}
			//this is display-only
			static void set_cmd_prefix(std::string value)
			{
				cmd_prefix_ = value;
			}
			virtual void register_command(const std::string& cmd,
				command_handler h, const std::string& help="",
				const std::string& usage="", const std::string& flags="")
			{
				command c = command(h, help, usage, flags);
				std::pair<typename command_map::iterator, bool> r;
				r = command_map_.insert(typename command_map::value_type(cmd, c));
				if (!r.second) { //overwrite if exists
					r.first->second = c;
				}
			}
			virtual void assert_existence(const std::string& cmd) {
				assert(command_map_.count(cmd));
			}
			virtual void register_alias(const std::string& to_cmd,
				const std::string& cmd)
			{
				// disable the assert to allow alias to "command + args"
				// the fonction assert_existence seems unused now
				//assert_existence(to_cmd);
				command_alias_map_[cmd] = to_cmd;
			}
			//get all aliases of a command.
			static const std::vector<std::string> get_aliases(const std::string& cmd)
			{
				std::vector<std::string> aliases;
				typedef command_alias_map::value_type p;
				BOOST_FOREACH(p i, command_alias_map_) {
					if (i.second == cmd) {
						aliases.push_back(i.first);
					}
				}
				return aliases;
			}
		private:
			static command_map command_map_;
			static command_alias_map command_alias_map_;
			static bool help_on_unknown_;
			static bool show_unavailable_;
			static std::string cmd_prefix_;
	};

	//static member definitions
	template <class Worker>
	typename map_command_handler<Worker>::command_map map_command_handler<Worker>::command_map_;

	template <class Worker>
	typename map_command_handler<Worker>::command_alias_map map_command_handler<Worker>::command_alias_map_;

	template <class Worker>
	bool map_command_handler<Worker>::help_on_unknown_ = true;

	template <class Worker>
	bool map_command_handler<Worker>::show_unavailable_ = false;

	template <class Worker>
	std::string map_command_handler<Worker>::cmd_prefix_;

	//command handler for chat /commands
	class chat_command_handler : public map_command_handler<chat_command_handler>
	{
		public:
			typedef map_command_handler<chat_command_handler> map;
			chat_command_handler(chat_handler& chathandler, bool allies_only)
			: map(), chat_handler_(chathandler), allies_only_(allies_only)
			{
			}

		protected:
			void do_emote();
			void do_network_send();
			void do_network_send_req_arg();
			void do_whisper();
			void do_log();
			void do_ignore();
			void do_friend();
			void do_remove();
			void do_display();
			void do_version();

			/** Ask the server to register the currently used nick. */
			void do_register();

			/** Ask the server do drop the currently used (and registered) nick. */
			void do_drop();

			/** Update details for the currently used username. */
			void do_set();

			/** Request information about a user from the server. */
			void do_info();

			/**
			 * Request a list of details that can be set for a username
			 * as these might vary depending on the configuration of the server.
			 */
			void do_details();

			std::string get_flags_description() const {
				return "(A) - auth command";
			}

			std::string get_command_flags_description(
				const map_command_handler<chat_command_handler>::command& c) const
			{
				return std::string(c.has_flag('A') ? " (auth only)" : "");
			}

			bool is_enabled(
				const map_command_handler<chat_command_handler>::command& c) const
			{
				return !(c.has_flag('A') && !preferences::is_authenticated());
			}

			void print(const std::string& title, const std::string& message)
			{
				chat_handler_.add_chat_message(time(NULL), title, 0, message);
			}
			void init_map()
			{
				set_cmd_prefix("/");
				register_command("query", &chat_command_handler::do_network_send,
					_("Send a query to the server. Without arguments the server"
					" should tell you the available commands."));
				register_command("ban", &chat_command_handler::do_network_send_req_arg,
					_("Ban and kick a player or observer. If he is not in the"
					" game but on the server he will only be banned."), "<nick>");
				register_command("kick", &chat_command_handler::do_network_send_req_arg,
					_("Kick a player or observer."), "<nick>");
				register_command("mute", &chat_command_handler::do_network_send,
					_("Mute an observer. Without an argument displays the mute status."), "<nick>");
				register_command("muteall", &chat_command_handler::do_network_send,
					_("Mute all observers."), "");
				register_command("ping", &chat_command_handler::do_network_send,
					"");
				register_command("green", &chat_command_handler::do_network_send_req_arg,
					"", "", "A");
				register_command("red", &chat_command_handler::do_network_send_req_arg,
					"", "", "A");
				register_command("yellow", &chat_command_handler::do_network_send_req_arg,
					"", "", "A");
				register_command("adminmsg", &chat_command_handler::do_network_send_req_arg,
					_("Send a message to the server admins currently online"), "");
				register_command("emote", &chat_command_handler::do_emote,
					_("Send an emotion or personal action in chat."), "<message>");
				register_alias("emote", "me");
				register_command("whisper", &chat_command_handler::do_whisper,
					_("Sends a private message. "
					"You can't send messages to players that don't control "
					"a side in a running game you are in."), "<nick> <message>");
				register_alias("whisper", "msg");
				register_alias("whisper", "m");
				register_command("log", &chat_command_handler::do_log,
					_("Change the log level of a log domain."), "<level> <domain>");
				register_command("ignore", &chat_command_handler::do_ignore,
					_("Add a nick to your ignores list."), "<nick>");
				register_command("friend", &chat_command_handler::do_friend,
					_("Add a nick to your friends list."), "<nick>");
				register_command("remove", &chat_command_handler::do_remove,
					_("Remove a nick from your ignores or friends list."), "<nick>");
				register_command("list", &chat_command_handler::do_display,
					_("Show your ignores and friends list."));
				register_alias("list", "display");
				register_command("version", &chat_command_handler::do_version,
					_("Display version information."));
				register_command("register", &chat_command_handler::do_register,
					_("Register your nick"), "<password> <email (optional)>");
				register_command("drop", &chat_command_handler::do_drop,
					_("Drop your nick."));
				register_command("set", &chat_command_handler::do_set,
					_("Update details for your nick. For possible details see '/details'."),
					"<detail> <value>");
				register_command("info", &chat_command_handler::do_info,
					_("Request information about a nick."), "<nick>");
				register_command("details", &chat_command_handler::do_details,
					_("Request a list of details you can set for your registered nick."));
			}
		private:
			chat_handler& chat_handler_;
			bool allies_only_;
	};

	//command handler for user :commands. Also understands all chat commands
	//via inheritance. This complicates some things a bit.
	class console_handler : public map_command_handler<console_handler>, private chat_command_handler
	{
		public:
			//convenience typedef
			typedef map_command_handler<console_handler> chmap;
			console_handler(menu_handler& menu_handler,
				mouse_handler& mouse_handler, const unsigned int team_num)
			: chmap(), chat_command_handler(menu_handler, true), menu_handler_(menu_handler), mouse_handler_(mouse_handler)
				, team_num_(team_num)
			{
			}
			using chmap::dispatch; //disambiguate

		protected:
			//chat_command_handler's init_map() and hanlers will end up calling these.
			//this makes sure the commands end up in our map
			virtual void register_command(const std::string& cmd,
				chat_command_handler::command_handler h, const std::string& help="",
				const std::string& usage="", const std::string& flags="")
			{
				chmap::register_command(cmd, h, help, usage, flags + "N"); //add chat commands as network_only
			}
			virtual void assert_existence(const std::string& cmd) {
				chmap::assert_existence(cmd);
			}
			virtual void register_alias(const std::string& to_cmd,
				const std::string& cmd)
			{
				chmap::register_alias(to_cmd, cmd);
			}
			virtual std::string get_arg(unsigned i) const
			{
				return chmap::get_arg(i);
			}
			virtual std::string get_cmd() const
			{
				return chmap::get_cmd();
			}
			virtual std::string get_data(unsigned n = 1) const
			{
				return chmap::get_data(n);
			}

			//these are needed to avoid ambiguities introduced by inheriting from console_command_handler
			using chmap::register_command;
			using chmap::register_alias;
			using chmap::help;
			using chmap::is_enabled;
			using chmap::command_failed;
			using chmap::command_failed_need_arg;

			void do_refresh();
			void do_droid();
			void do_theme();
			void do_control();
			void do_clear();
			void do_sunset();
			void do_fps();
			void do_benchmark();
			void do_save();
			void do_save_quit();
			void do_quit();
			void do_ignore_replay_errors();
			void do_nosaves();
			void do_next_level();
			void do_choose_level();
			void do_debug();
			void do_nodebug();
			void do_custom();
			void do_set_alias();
			void do_set_var();
			void do_show_var();
			void do_unit();
//			void do_buff();
			void do_unbuff();
			void do_create();
			void do_fog();
			void do_shroud();
			void do_gold();
			void do_event();
			void do_toggle_draw_coordinates();
			void do_toggle_draw_terrain_codes();

			std::string get_flags_description() const {
				return "(D) - debug only, (N) - network only, (A) - auth only";
			}
			using chat_command_handler::get_command_flags_description; //silence a warning
			std::string get_command_flags_description(const chmap::command& c) const
			{
				return std::string(c.has_flag('D') ? " (debug command)" : "")
				     + std::string(c.has_flag('N') ? " (network only)" : "")
					 + std::string(c.has_flag('A') ? " (auth only)" : "");
			}
			using map::is_enabled;
			bool is_enabled(const chmap::command& c) const
			{
				return !((c.has_flag('D') && !game_config::debug)
					  || (c.has_flag('N') && network::nconnections() == 0)
					  || (c.has_flag('A') && !preferences::is_authenticated()));
			}
			void print(const std::string& title, const std::string& message)
			{
				menu_handler_.add_chat_message(time(NULL), title, 0, message);
			}
			void init_map()
			{
				chat_command_handler::init_map();//grab chat_ /command handlers
				chmap::get_command("log")->flags = ""; //clear network-only flag from log
				chmap::get_command("version")->flags = ""; //clear network-only flag
				chmap::get_command("ignore")->flags = ""; //clear network-only flag
				chmap::get_command("friend")->flags = ""; //clear network-only flag
				chmap::get_command("list")->flags = ""; //clear network-only flag
				chmap::get_command("remove")->flags = ""; //clear network-only flag
				chmap::set_cmd_prefix(":");
				register_command("refresh", &console_handler::do_refresh,
					_("Refresh gui."));
				register_command("droid", &console_handler::do_droid,
					_("Switch a side to/from AI control."), "[<side> [on/off]]");
				register_command("theme", &console_handler::do_theme);
				register_command("control", &console_handler::do_control,
					_("Assign control of a side to a different player or observer."), "<side> <nick>", "N");
				register_command("clear", &console_handler::do_clear,
					_("Clear chat history."));
				register_command("sunset", &console_handler::do_sunset,
					_("Visualize the screen refresh procedure."), "", "D");
				register_command("fps", &console_handler::do_fps, "Show fps.");
				register_command("benchmark", &console_handler::do_benchmark);
				register_command("save", &console_handler::do_save, _("Save game."));
				register_alias("save", "w");
				register_command("quit", &console_handler::do_quit, _("Quit game."));
				register_alias("quit", "q");
				register_alias("quit", "q!");
				register_command("save_quit", &console_handler::do_save_quit,
					_("Save and quit."));
				register_alias("save_quit", "wq");
				register_command("ignore_replay_errors", &console_handler::do_ignore_replay_errors,
					_("Ignore replay errors."));
				register_command("nosaves", &console_handler::do_nosaves,
					_("Disable autosaves."));
				register_command("next_level", &console_handler::do_next_level,
					_("Advance to the next scenario, or scenario identified by 'id'"), "<id>", "D");
				register_alias("next_level", "n");
				register_command("choose_level", &console_handler::do_choose_level,
					_("Choose next scenario"), "", "D");
				register_alias("choose_level", "cl");
				register_command("debug", &console_handler::do_debug,
					_("Turn debug mode on."));
				register_command("nodebug", &console_handler::do_nodebug,
					_("Turn debug mode off."), "", "D");
				register_command("custom", &console_handler::do_custom,
					_("Set the command used by the custom command hotkey"), "<command>[;<command>...]");
				register_command("alias", &console_handler::do_set_alias,
					_("Set or show alias to a command"), "<name>[=<command>]");
				register_command("set_var", &console_handler::do_set_var,
					_("Set a scenario variable."), "<var>=<value>", "D");
				register_command("show_var", &console_handler::do_show_var,
					_("Show a scenario variable."), "<var>", "D");
				register_command("unit", &console_handler::do_unit,
					_("Modify a unit variable. (Only top level keys are supported.)"), "", "D");
/*				register_command("buff", &console_handler::do_buff,
					_("Add a trait to a unit."), "", "D");*/
				register_command("unbuff", &console_handler::do_unbuff,
					_("Remove a trait from a unit. (Does not work yet.)"), "", "D");
				register_command("create", &console_handler::do_create,
					_("Create a unit."), "", "D");
				register_command("fog", &console_handler::do_fog,
					_("Toggle fog for the current player."), "", "D");
				register_command("shroud", &console_handler::do_shroud,
					_("Toggle shroud for the current player."), "", "D");
				register_command("gold", &console_handler::do_gold,
					_("Give gold to the current player."), "", "D");
				register_command("throw", &console_handler::do_event,
					_("Fire a game event."), "", "D");
				register_alias("throw", "fire");
				register_command("show_coordinates", &console_handler::do_toggle_draw_coordinates,
					_("Toggle overlaying of x,y coordinates on hexes."));
				register_alias("show_coordinates", "sc");
				register_command("show_terrain_codes", &console_handler::do_toggle_draw_terrain_codes,
					_("Toggle overlaying of terrain codes on hexes."));
				register_alias("show_terrain_codes", "tc");

				if (const config &alias_list = preferences::get_alias())
				{
					foreach (const config::attribute &a, alias_list.attribute_range()) {
						register_alias(a.second, a.first);
					}
				}
			}
		private:
			menu_handler& menu_handler_;
			mouse_handler& mouse_handler_;
			const unsigned int team_num_;
	};

	chat_handler::chat_handler()
	{
	}

	chat_handler::~chat_handler()
	{
	}

	/**
	 * Change the log level of a log domain.
	 *
	 * @param data               String of the form: '<level> <domain>'
	 */
	void chat_handler::change_logging(const std::string& data) {
		const std::string::const_iterator j =
				std::find(data.begin(), data.end(), ' ');
		if (j == data.end()) return;
		const std::string level(data.begin(),j);
		const std::string domain(j+1,data.end());
		int severity;
		if (level == "error") severity = 0;
		else if (level == "warning") severity = 1;
		else if (level == "info") severity = 2;
		else if (level == "debug") severity = 3;
		else {
			utils::string_map symbols;
			symbols["level"] = level;
			const std::string& msg =
					vgettext("Unknown debug level: '$level'.", symbols);
			ERR_NG << msg << "\n";
			add_chat_message(time(NULL), _("error"), 0, msg);
			return;
		}
		if (!lg::set_log_domain_severity(domain, severity)) {
			utils::string_map symbols;
			symbols["domain"] = domain;
			const std::string& msg =
					vgettext("Unknown debug domain: '$domain'.", symbols);
			ERR_NG << msg << "\n";
			add_chat_message(time(NULL), _("error"), 0, msg);
			return;
		} else {
			utils::string_map symbols;
			symbols["level"] = level;
			symbols["domain"] = domain;
			const std::string& msg =
					vgettext("Switched domain: '$domain' to level: '$level'.", symbols);
			LOG_NG << msg << "\n";
			add_chat_message(time(NULL), "log", 0, msg);
		}
	}

	void chat_handler::send_command(const std::string& cmd, const std::string& args /* = "" */) {
		config data;
		if (cmd == "muteall") {
			data.add_child(cmd);
		} else if (cmd == "query") {
			data.add_child(cmd)["type"] = args;
		} else if (cmd == "ban" || cmd == "kick" || cmd == "mute") {
			data.add_child(cmd)["username"] = args;
		} else if (cmd == "ping") {
			data[cmd] = lexical_cast<std::string>(time(NULL));
		} else if (cmd == "green") {
			data.add_child("query")["type"] = "lobbymsg @" + args;
		} else if (cmd == "red") {
			data.add_child("query")["type"] = "lobbymsg #" + args;
		} else if (cmd == "yellow") {
			data.add_child("query")["type"] = "lobbymsg <255,255,0>" + args;
		} else if (cmd == "adminmsg") {
			data.add_child("query")["type"] = "adminmsg " + args;
		}
		network::send_data(data, 0, true);
	}

	void chat_handler::do_speak(const std::string& message, bool allies_only)
	{
		if(message == "" || message == "/") {
			return;
		}
		bool is_command = (message[0] == '/');
		bool quoted_command = (is_command && message[1] == ' ');

		if(!is_command) {
			send_chat_message(message, allies_only);
			return;
		} else if (quoted_command) {
			send_chat_message(std::string(message.begin() + 2, message.end()), allies_only);
			return;
		}
		std::string cmd(message.begin() + 1, message.end());
		chat_command_handler cch(*this, allies_only);
		cch.dispatch(cmd);
	}

	void chat_command_handler::do_emote()
	{
		chat_handler_.send_chat_message("/me " + get_data(), allies_only_);
	}

	void chat_command_handler::do_network_send()
	{
		chat_handler_.send_command(get_cmd(), get_data());
	}

	void chat_command_handler::do_network_send_req_arg()
	{
		if (get_data(1).empty()) return command_failed_need_arg(1);
		do_network_send();
	}

	void chat_command_handler::do_whisper()
	{
		if (get_data(1).empty()) return command_failed_need_arg(1);
		if (get_data(2).empty()) return command_failed_need_arg(2);
		config cwhisper, data;
		cwhisper["receiver"] = get_arg(1);
		cwhisper["message"] = get_data(2);
		cwhisper["sender"] = preferences::login();
		data.add_child("whisper", cwhisper);
		chat_handler_.add_chat_message(time(NULL),
			"whisper to " + cwhisper["receiver"], 0,
			cwhisper["message"], game_display::MESSAGE_PRIVATE);
		network::send_data(data, 0, true);
	}

	void chat_command_handler::do_log()
	{
		chat_handler_.change_logging(get_data());
	}

	void chat_command_handler::do_ignore()
	{
		if (get_arg(1).empty()) {
			const std::set<std::string>& tmp = preferences::get_ignores();
			print("ignores list", tmp.empty() ? "(empty)" : utils::join(tmp));
		} else {
			for(int i = 1; !get_arg(i).empty(); i++){
				if (preferences::add_ignore(get_arg(i))) {
					print("ignore",  _("Added to ignore list: ") + get_arg(i));
				} else {
					command_failed(_("Invalid username: ") + get_arg(i));
				}
			}
		}
	}

	void chat_command_handler::do_friend()
	{
		if (get_arg(1).empty()) {
			const std::set<std::string>& tmp = preferences::get_friends();
			print("friends list", tmp.empty() ? "(empty)" : utils::join(tmp));
		} else {
			for(int i = 1;!get_arg(i).empty();i++){
				if (preferences::add_friend(get_arg(i))) {
					print("friend",  _("Added to friends list: ") + get_arg(i));
				} else {
					command_failed(_("Invalid username: ") + get_arg(i));
				}
			}
		}
	}

	void chat_command_handler::do_remove()
	{
		for(int i = 1;!get_arg(i).empty();i++){
			preferences::remove_friend(get_arg(i));
			preferences::remove_ignore(get_arg(i));
			print("list", _("Removed from list: ") + get_arg(i));
		}
	}

	void chat_command_handler::do_display()
	{
		const std::set<std::string> & friends = preferences::get_friends();
		const std::set<std::string> & ignores = preferences::get_ignores();

		if (!friends.empty()) {
			print("friends list", utils::join(friends));
		}

		if (!ignores.empty()) {
			print("ignores list", utils::join(ignores));
		}

		if (friends.empty() && ignores.empty()) {
			print("list", _("There are no players on your friends or ignore list."));
		}
	}

	void chat_command_handler::do_version() {
		print("version", game_config::revision);
	}

	void chat_command_handler::do_register() {
		config data;
		config& nickserv = data.add_child("nickserv");

		if (get_data(1).empty()) return command_failed_need_arg(1);

		config &reg = nickserv.add_child("register");
		reg["password"] = get_arg(1);
		if(!get_data(2).empty()) {
			reg["mail"] = get_arg(2);
		}
		print("nick registration", "registering with password *** and " +
				(get_data(2).empty() ? "no email address" : "email address " + get_data(2)));

		network::send_data(data, 0, true);
	}

	void chat_command_handler::do_drop() {
		config data;
		config& nickserv = data.add_child("nickserv");

		nickserv.add_child("drop");

		print("nick registration", "dropping your username");

		network::send_data(data, 0, true);
	}

	void chat_command_handler::do_set() {
		config data;
		config& nickserv = data.add_child("nickserv");

		if (get_data(1).empty()) return command_failed_need_arg(1);
		if (get_data(2).empty()) return command_failed_need_arg(2);

		config &set = nickserv.add_child("set");
		set["detail"] = get_arg(1);
		set["value"] = get_data(2);
		print("nick registration", "setting " + get_arg(1) + " to " + get_data(2));

		network::send_data(data, 0, true);
	}

	void chat_command_handler::do_info() {
		if (get_data(1).empty()) return command_failed_need_arg(1);

		config data;
		config& nickserv = data.add_child("nickserv");

		nickserv.add_child("info")["name"] = get_data(1);
		print("nick registration", "requesting information for user " + get_data(1));

		network::send_data(data, 0, true);
	}

	void chat_command_handler::do_details() {

		config data;
		config& nickserv = data.add_child("nickserv");
		nickserv.add_child("details");

		network::send_data(data, 0, true);
	}

	void menu_handler::send_chat_message(const std::string& message, bool allies_only)
	{
		config cfg;
		cfg["id"] = preferences::login();
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
		add_chat_message(time(NULL), cfg["id"], side, message,
				private_message ? game_display::MESSAGE_PRIVATE : game_display::MESSAGE_PUBLIC);

	}


	void menu_handler::do_search(const std::string& new_search)
	{
		if(new_search.empty() == false && new_search != last_search_)
			last_search_ = new_search;

		if(last_search_.empty()) return;

		bool found = false;
		map_location loc = last_search_hit_;
		//If this is a location search, just center on that location.
		std::vector<std::string> args = utils::split(last_search_, ',');
		if(args.size() == 2) {
			int x, y;
			x = lexical_cast_default<int>(args[0], 0)-1;
			y = lexical_cast_default<int>(args[1], 0)-1;
			if(x >= 0 && x < map_.w() && y >= 0 && y < map_.h()) {
				loc = map_location(x,y);
				found = true;
			}
		}
		//Start scanning the game map
		if(loc.valid() == false)
			loc = map_location(map_.w()-1,map_.h()-1);
		map_location start = loc;
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
					const std::string name = ui->second.name();
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
			last_search_hit_ = map_location();
			//Not found, inform the player
			utils::string_map symbols;
			symbols["search"] = last_search_;
			const std::string msg = vgettext("Couldn't find label or unit "
					"containing the string '$search'.", symbols);
			gui::dialog(*gui_,"",msg).show();
		}
	}

	void menu_handler::do_command(const std::string& str,
			const unsigned int team_num, mouse_handler& mousehandler)
	{
		console_handler ch(*this, mousehandler, team_num);
		ch.dispatch(str);
	}

	void console_handler::do_refresh() {
		image::flush_cache();
		menu_handler_.gui_->redraw_everything();
	}

	void console_handler::do_droid() {
		// :droid [<side> [on/off]]
		const std::string side_s = get_arg(1);
		const std::string action = get_arg(2);
		// default to the current side if empty
		const unsigned int side = side_s.empty() ?
			team_num_ : lexical_cast_default<unsigned int>(side_s);

		if (side < 1 || side > menu_handler_.teams_.size()) {
			utils::string_map symbols;
			symbols["side"] = side_s;
			command_failed(vgettext("Can't droid invalid side: '$side'.", symbols));
			return;
		} else if (menu_handler_.teams_[side - 1].is_network()) {
			utils::string_map symbols;
			symbols["side"] = lexical_cast<std::string>(side);
			command_failed(vgettext("Can't droid networked side: '$side'.", symbols));
			return;
		} else if (menu_handler_.teams_[side - 1].is_human() && action != " off") {
			//this is our side, so give it to AI
			menu_handler_.teams_[side - 1].make_human_ai();
			menu_handler_.change_controller(lexical_cast<std::string>(side),"human_ai");
			if(team_num_ == side) {
				//if it is our turn at the moment, we have to indicate to the
				//play_controller, that we are no longer in control
				throw end_turn_exception(side);
			}
		} else if (menu_handler_.teams_[side - 1].is_ai() && action != " on") {
			menu_handler_.teams_[side - 1].make_human();
			menu_handler_.change_controller(lexical_cast<std::string>(side),"human");
		}
		menu_handler_.textbox_info_.close(*menu_handler_.gui_);
	}
	void console_handler::do_theme() {
		preferences::show_theme_dialog(*menu_handler_.gui_);
	}
	void console_handler::do_control() {
		// :control <side> <nick>
		if (network::nconnections() == 0) return;
		const std::string side = get_arg(1);
		const std::string player = get_arg(2);
		if(player.empty())
		{
			command_failed_need_arg(2);
			return;
		}

		unsigned int side_num;
		try {
			side_num = lexical_cast<unsigned int>(side);
		} catch(bad_lexical_cast&) {
			utils::string_map symbols;
			symbols["side"] = side;
			command_failed(vgettext("Can't change control of invalid side: '$side'.", symbols));
			return;
		}
		if (side_num < 1 || side_num > menu_handler_.teams_.size()) {
			utils::string_map symbols;
			symbols["side"] = side;
			command_failed(vgettext("Can't change control of out-of-bounds side: '$side'.",	symbols));
			return;
		}
		//if this is our side we are always allowed to change the controller
		if(menu_handler_.teams_[side_num - 1].is_local()){
			if (player == preferences::login())
				return;
			menu_handler_.change_side_controller(side,player,true);
		} else {
			//it is not our side, the server will decide if we can change the
			//controller (that is if we are host of the game)
			menu_handler_.change_side_controller(side,player);
		}
		menu_handler_.textbox_info_.close(*(menu_handler_.gui_));
	}
	void console_handler::do_clear() {
		menu_handler_.gui_->clear_chat_messages();
	}
	void console_handler::do_sunset() {
		int delay = lexical_cast_default<int>(get_data());
		menu_handler_.gui_->sunset(delay);
		gui2::twindow::set_sunset(delay);
	}
	void console_handler::do_fps() {
		preferences::set_show_fps(!preferences::show_fps());
	}
	void console_handler::do_benchmark() {
		menu_handler_.gui_->toggle_benchmark();
	}
	void console_handler::do_save() {
		menu_handler_.save_game(get_data(),gui::NULL_DIALOG);
	}
	void console_handler::do_save_quit() {
		menu_handler_.save_game(get_data(),gui::NULL_DIALOG);
		throw end_level_exception(QUIT);
	}
	void console_handler::do_quit() {
		throw end_level_exception(QUIT);
	}
	void console_handler::do_ignore_replay_errors() {
		game_config::ignore_replay_errors = (get_data() != "off") ? true : false;
	}
	void console_handler::do_nosaves() {
		game_config::disable_autosave = (get_data() != "off") ? true : false;
	}
	void console_handler::do_next_level() {
		if (!get_data().empty())
			menu_handler_.gamestate_.next_scenario = get_data();
		throw end_level_exception(VICTORY, "", 100, false, false, false, true, false);
	}
	void console_handler::do_choose_level() {
		std::vector<std::string> options;
		int next = 0, nb = 0;
		foreach (const config &sc, menu_handler_.game_config_.child_range("scenario"))
		{
			const std::string &id = sc["id"];
			options.push_back(id);
			if (id == menu_handler_.gamestate_.next_scenario)
				next = nb;
			++nb;
		}
		int choice = 0;
		{
			gui::dialog menu(*menu_handler_.gui_, _("Choose Scenario (Debug!)"), "", gui::OK_CANCEL);
			menu.set_menu(options);
			menu.get_menu().move_selection(next);
			choice = menu.show();
		}

		if (size_t(choice) < options.size()) {
			menu_handler_.gamestate_.next_scenario = options[choice];
			throw end_level_exception(VICTORY, "", 100, false, false, false, true, false);
		}
	}
	void console_handler::do_debug() {
		if (network::nconnections() == 0) {
			print(get_cmd(), _("Debug mode activated!"));
			game_config::debug = true;
		} else {
			command_failed(_("Debug mode not available in network games"));
		}
	}
	void console_handler::do_nodebug() {
		if (game_config::debug) {
			print(get_cmd(), _("Debug mode deactivated!"));
			game_config::debug = false;
		}
	}
	void console_handler::do_custom() {
		preferences::set("custom_command", get_data());
	}
	void console_handler::do_set_alias() {
		const std::string data = get_data();
		const std::string::const_iterator j = std::find(data.begin(),data.end(),'=');
		const std::string alias(data.begin(),j);
		if(j != data.end()) {
			const std::string command(j+1,data.end());
			if (!command.empty()) {
				register_alias(command, alias);
			} else {
				// "alias something=" deactivate this alias. We just set it
				// equal to itself here. Later preferences will filter empty alias.
				register_alias(alias, alias);
			}
			preferences::add_alias(alias, command);
			// directly save it for the moment, but will slow commands sequence
			preferences::write_preferences();
		} else {
			// "alias something" display its value
			// if no alias, will be "'something' = 'something'"
			const std::string command = chmap::get_actual_cmd(alias);
			print(get_cmd(), "'"+alias+"'" + " = " + "'"+command+"'");
		}
	}
	void console_handler::do_set_var() {
		const std::string data = get_data();
		if (data.empty()) {
			command_failed_need_arg(1);
			return;
		}
		const std::string::const_iterator j = std::find(data.begin(),data.end(),'=');
		if(j != data.end()) {
			const std::string name(data.begin(),j);
			const std::string value(j+1,data.end());
			menu_handler_.gamestate_.set_variable(name,value);
		} else {
			command_failed("Variable not found");
		}
	}
	void console_handler::do_show_var() {
		gui::message_dialog to_show(*menu_handler_.gui_,"",menu_handler_.gamestate_.get_variable(get_data()));
		to_show.show();
	}
	void console_handler::do_unit() {
		// prevent SIGSEGV due to attempt to set HP during a fight
		if (events::commands_disabled > 0)
			return;
		const unit_map::iterator i = menu_handler_.current_unit(mouse_handler_);
		if (i == menu_handler_.units_.end()) return;
		const std::string data = get_data(1);
		std::vector<std::string> parameters = utils::split(data, '=', utils::STRIP_SPACES);
		if (parameters.size() < 2)
			return;

		const std::string& name = parameters[0];
		const std::string& value = parameters[1];

		// FIXME: Avoids a core dump on display
		// because alignment strings get reduced
		// to an enum, then used to index an
		// array of strings.
		// But someday the code ought to be
		// changed to allow general string
		// alignments for UMC.
		if (name == "alignment" && (value != "lawful" && value != "neutral" && value != "chaotic")) {
			command_failed("Invalid alignment: '" + value + "', needs to be one of lawful, neutral or chaotic.");
			return;
		}
		config cfg;
		i->second.write(cfg);
		cfg[name] = value;
		i->second = unit(&menu_handler_.units_,&menu_handler_.map_,&menu_handler_.status_,&menu_handler_.teams_,cfg);
		menu_handler_.gui_->invalidate(i->first);
		menu_handler_.gui_->invalidate_unit();
	}
	/*void console_handler::do_buff() {
		print(get_cmd(), _("Debug mode activated!"));
		const unit_map::iterator i = menu_handler_.current_unit(mouse_handler_);
		if(i != menu_handler_.units_.end()) {
			//i->second.add_trait(get_data());
			menu_handler_.gui_->invalidate(i->first);
			menu_handler_.gui_->invalidate_unit();
		} else {
			command_failed("No unit selected");
		}
	}*/
	void console_handler::do_unbuff() {
		const unit_map::iterator i = menu_handler_.current_unit(mouse_handler_);
		if(i != menu_handler_.units_.end()) {
			// FIXME: 'data_' is the trait.  Clear it.

			menu_handler_.gui_->invalidate(i->first);
			menu_handler_.gui_->invalidate_unit();
		} else {
			command_failed("No unit selected");
		}
	}
	void console_handler::do_create() {
		const map_location &loc = mouse_handler_.get_last_hex();
		if (menu_handler_.map_.on_board(loc)) {
			const unit_type_data::unit_type_map::const_iterator i = unit_type_data::types().find_unit_type(get_data());
			if(i == unit_type_data::types().end()) {
				command_failed("Invalid unit type");
				return;
			}

			menu_handler_.units_.erase(loc);
			menu_handler_.units_.add(loc,
				unit(&menu_handler_.units_, &menu_handler_.map_, &menu_handler_.status_,
				     &menu_handler_.teams_, &i->second, 1, false));
			menu_handler_.gui_->invalidate(loc);
			menu_handler_.gui_->invalidate_unit();
		} else {
			command_failed("Invalid location");
		}
	}
	void console_handler::do_fog() {
		menu_handler_.teams_[team_num_ - 1].set_fog( !menu_handler_.teams_[team_num_ - 1].uses_fog() );
		recalculate_fog(menu_handler_.map_,menu_handler_.units_,menu_handler_.teams_, team_num_ - 1);
		menu_handler_.gui_->recalculate_minimap();
		menu_handler_.gui_->redraw_everything();
	}
	void console_handler::do_shroud() {
		menu_handler_.teams_[team_num_ - 1].set_shroud( !menu_handler_.teams_[team_num_ - 1].uses_shroud() );
		menu_handler_.clear_shroud(team_num_);
		menu_handler_.gui_->recalculate_minimap();
		menu_handler_.gui_->redraw_everything();
	}
	void console_handler::do_gold() {
		menu_handler_.teams_[team_num_ - 1].spend_gold(-lexical_cast_default<int>(get_data(),1000));
		menu_handler_.gui_->redraw_everything();
	}
	void console_handler::do_event() {
		game_events::fire(get_data());
		menu_handler_.gui_->redraw_everything();
	}
	void console_handler::do_toggle_draw_coordinates() {
		menu_handler_.gui_->set_draw_coordinates(!menu_handler_.gui_->get_draw_coordinates());
		menu_handler_.gui_->invalidate_all();
	}
	void console_handler::do_toggle_draw_terrain_codes() {
		menu_handler_.gui_->set_draw_terrain_codes(!menu_handler_.gui_->get_draw_terrain_codes());
		menu_handler_.gui_->invalidate_all();
	}

	void menu_handler::do_ai_formula(const std::string& str,
			const unsigned int team_num, mouse_handler& /*mousehandler*/)
	{
		replay dummy_replay;
		replay_network_sender dummy_sender(dummy_replay);
		undo_list dummy_undo;

		turn_info turn_data(gamestate_, status_, *gui_, const_cast<gamemap&>(map_), teams_, team_num, units_, dummy_sender, dummy_undo);
		ai_interface::info info(*gui_, const_cast<gamemap&>(map_), units_, teams_, team_num,  const_cast<gamestatus&>(status_), turn_data, gamestate_);

		try {
			add_chat_message(time(NULL), _("ai"), 0, ai_manager::evaluate_command(info,str));
		} catch(...) {
			//add_chat_message(time(NULL), _("ai"), 0, "ERROR IN FORMULA");
		}
	}

	void menu_handler::user_command()
	{
		textbox_info_.show(gui::TEXTBOX_COMMAND,sgettext("prompt^Command:"), "", false, *gui_);
	}

	void menu_handler::custom_command(mouse_handler& mousehandler, const unsigned int team_num)
	{
		std::vector<std::string> commands = utils::split(preferences::custom_command(), ';');
		std::vector<std::string>::iterator c = commands.begin();
		for (; c != commands.end() ; ++c) {
			do_command(*c, team_num, mousehandler);
		}
	}

	void menu_handler::ai_formula()
	{
            	if (network::nconnections() == 0) {
                    std::cerr << "showing ai formula...\n";
                    textbox_info_.show(gui::TEXTBOX_AI,sgettext("prompt^Command:"), "", false, *gui_);
		}
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
	void menu_handler::change_controller(const std::string& side, const std::string& controller)
	{
		config cfg;
		config& change = cfg.add_child("change_controller");
		change["side"] = side;
		change["controller"] = controller;
		network::send_data(cfg, 0, true);
	}

	void menu_handler::change_side_controller(const std::string& side, const std::string& player, bool own_side)
	{
		config cfg;
		config& change = cfg.add_child("change_controller");
		change["side"] = side;
		change["player"] = player;

		if(own_side) {
			change["own_side"] = "yes";
		}

		network::send_data(cfg, 0, true);
	}
} // end namespace events

