/*
   Copyright (C) 2006 - 2014 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
   wesnoth playturn Copyright (C) 2003 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Operations activated from menus/hotkeys while playing a game.
 * E.g. Unitlist, status_table, save_game, save_map, chat, show_help, etc.
 */

#include "global.hpp"

#include "actions/attack.hpp"
#include "actions/create.hpp"
#include "actions/move.hpp"
#include "actions/undo.hpp"
#include "actions/vision.hpp"
#include "builder.hpp"
#include "ai/manager.hpp"
#include "dialogs.hpp"
#include "formatter.hpp"
#include "filechooser.hpp"
#include "game_end_exceptions.hpp"
#include "game_events/pump.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "gui/dialogs/chat_log.hpp"
#include "gui/dialogs/edit_label.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/dialogs/wml_message.hpp"
#include "gui/dialogs/gamestate_inspector.hpp"
#include "gui/dialogs/mp_change_control.hpp"
#include "gui/dialogs/data_manage.hpp"
#include "gui/dialogs/simple_item_selector.hpp"
#include "gui/dialogs/edit_text.hpp"
#include "gui/dialogs/unit_create.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "help.hpp"
#include "log.hpp"
#include "map.hpp"
#include "map_label.hpp"
#include "marked-up_text.hpp"
#include "menu_events.hpp"
#include "mouse_events.hpp"
#include "play_controller.hpp"
#include "preferences_display.hpp"
#include "replay.hpp"
#include "replay_helper.hpp"
#include "resources.hpp"
#include "savegame.hpp"
#include "sound.hpp"
#include "statistics_dialog.hpp"
#include "synced_context.hpp"
#include "unit_display.hpp"
#include "wml_separators.hpp"
#include "formula_string_utils.hpp"
#include "scripting/lua.hpp"
#include "whiteboard/manager.hpp"
#include "widgets/combo.hpp"

#include <boost/foreach.hpp>

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)

namespace events{

menu_handler::menu_handler(game_display* gui, unit_map& units, std::vector<team>& teams,
		const config& level, const gamemap& map,
		const config& game_config, game_state& gamestate) :
	gui_(gui),
	units_(units),
	teams_(teams),
	level_(level),
	map_(map),
	game_config_(game_config),
	gamestate_(gamestate),
	textbox_info_(),
	last_search_(),
	last_search_hit_()
{
}

menu_handler::~menu_handler()
{
}

gui::floating_textbox& menu_handler::get_textbox(){
	return textbox_info_;
}

std::string menu_handler::get_title_suffix(int side_num)
{
	int controlled_recruiters = 0;
	for(size_t i = 0; i < teams_.size(); ++i) {
		if(teams_[i].is_human() && !teams_[i].recruits().empty()
		&& units_.find_leader(i + 1) != units_.end()) {
		++controlled_recruiters;
		}
	}
	std::stringstream msg;
	if(controlled_recruiters >= 2) {
		unit_map::const_iterator leader = units_.find_leader(side_num);
		if (leader != units_.end() && !leader->name().empty()) {
			msg << " (" << leader->name(); msg << ")";
		}
	}
	return msg.str();
}

void menu_handler::objectives(int side_num)
{
	config cfg;
	cfg["side"] = str_cast(side_num);
	game_events::handle_event_command("show_objectives",
		game_events::queued_event("_from_interface", map_location(),
			map_location(), config()), vconfig(cfg));
	team &current_team = teams_[side_num - 1];
	dialogs::show_objectives(level_, current_team.objectives());
	current_team.reset_objectives_changed();
}

void menu_handler::show_statistics(int side_num)
{
	team &current_team = teams_[side_num - 1];
	// Current Player name
	const std::string &player = current_team.current_player();
	//add player's name to title of dialog
	std::stringstream title_str;
	title_str <<  _("Statistics") << " (" << player << ")";
	statistics_dialog stats_dialog(*gui_, title_str.str(),
		side_num, current_team.save_id(), player);
	stats_dialog.show();
}

void menu_handler::unit_list()
{
	dialogs::show_unit_list(*gui_);
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
	//all names with in colors
	for(size_t n = 0; n != teams_.size(); ++n) {
		if(teams_[n].hidden()) {
			continue;
		}
		status_table_empty=false;

		const bool known = viewing_team.knows_about_team(n, network::nconnections() > 0);
		const bool enemy = viewing_team.is_enemy(n+1);

		std::stringstream str;

		const team_data data = calculate_team_data(teams_[n],n+1);

		unit_map::const_iterator leader = units_.find_leader(n + 1);
		std::string leader_name;
		//output the number of the side first, and this will
		//cause it to be displayed in the correct color
		if(leader != units_.end()) {
			const bool fogged = viewing_team.fogged(leader->get_location());
			// Add leader image. If it's fogged
			// show only a random leader image.
			if (!fogged || known || game_config::debug) {
				str << IMAGE_PREFIX << leader->absolute_image();
#ifndef LOW_MEM
				str << leader->image_mods();
#endif
				leader_bools.push_back(true);
				leader_name = leader->name();
			} else {
				str << IMAGE_PREFIX << std::string("units/unknown-unit.png");
#ifndef LOW_MEM
				str << "~RC(magenta>" << teams_[n].color() << ")";
#endif
				leader_bools.push_back(false);
				leader_name = "Unknown";
			}

			if (gamestate_.classification().campaign_type == "multiplayer")
				leader_name = teams_[n].current_player();

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
			str << utils::half_signed_value(data.gold) << COLUMN_SEPARATOR;
		} else if(enemy && viewing_team.uses_fog()) {
			str << ' ' << COLUMN_SEPARATOR;
		} else {
			str << utils::half_signed_value(data.gold) << COLUMN_SEPARATOR;
		}
		str << data.villages;
                if(!(viewing_team.uses_fog() || viewing_team.uses_shroud())) {
                        str << "/" << map_.villages().size();
                }
		str << COLUMN_SEPARATOR
			<< data.units << COLUMN_SEPARATOR << data.upkeep << COLUMN_SEPARATOR
			<< (data.net_income < 0 ? font::BAD_TEXT : font::NULL_MARKUP) << utils::signed_value(data.net_income);
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
			<< _("scenario settings^Support Per\nVillage") << COLUMN_SEPARATOR
			<< _("scenario settings^Fog")               << COLUMN_SEPARATOR
			<< _("scenario settings^Shroud");

	gui::menu::basic_sorter sorter;
	sorter.set_redirect_sort(0,1).set_alpha_sort(1).set_numeric_sort(2)
		  .set_numeric_sort(3).set_numeric_sort(4).set_numeric_sort(5)
		  .set_numeric_sort(6).set_alpha_sort(7).set_alpha_sort(8);

	std::vector<std::string> items;
	std::vector<bool> leader_bools;
	items.push_back(heading.str());

	const team& viewing_team = teams_[gui_->viewing_team()];
	bool settings_table_empty = true;
	bool fogged;

	for(size_t n = 0; n != teams_.size(); ++n) {
		if(teams_[n].hidden()) {
			continue;
		}
		settings_table_empty = false;

		std::stringstream str;
		unit_map::const_iterator leader = units_.find_leader(n + 1);

		if(leader != units_.end()) {
			// Add leader image. If it's fogged
			// show only a random leader image.
			fogged=viewing_team.fogged(leader->get_location());
			if (!fogged || viewing_team.knows_about_team(n, network::nconnections() > 0) || game_config::debug) {
				str << IMAGE_PREFIX << leader->absolute_image();
				leader_bools.push_back(true);
			} else {
				str << IMAGE_PREFIX << std::string("units/unknown-unit.png");
				leader_bools.push_back(false);
			}
#ifndef LOW_MEM
			str << "~RC(" << leader->team_color() << '>'
			    << team::get_side_color_index(n+1) << ")";
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
			<< teams_[n].village_support() << COLUMN_SEPARATOR
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

void menu_handler::save_map()
{
	std::string input_name = get_dir(get_dir(get_user_data_dir() + "/editor") + "/maps/");
	int res = 0;
	int overwrite = 1;
	do {
		res = dialogs::show_file_chooser_dialog_save(*gui_, input_name, _("Save the Map As"), ".map");
		if (res == 0) {

			if (file_exists(input_name)) {
				const int res = gui2::show_message((*gui_).video(), "", _("The map already exists. Do you want to overwrite it?"), gui2::tmessage::yes_no_buttons);
				overwrite = res == gui2::twindow::CANCEL ? 1 : 0;
			}
			else
				overwrite = 0;
		}
	} while (res == 0 && overwrite != 0);

	// Try to save the map, if it fails we reset the filename.
	if (res == 0) {
		try {
			write_file(input_name, map_.write());
			gui2::show_transient_message(gui_->video(), "", _("Map saved."));
		} catch (io_exception& e) {
			utils::string_map symbols;
			symbols["msg"] = e.what();
			const std::string msg = vgettext("Could not save the map: $msg",symbols);
			gui2::show_transient_message(gui_->video(), "", msg);
		}
	}

	/*
	std::string input_name = get_dir(get_dir(get_user_data_dir() + "/editor") + "/maps/");
	int res = 0;
	int overwrite = 1;
	do {
		res = dialogs::show_file_chooser_dialog_save(*gui_, input_name, _("Save the Map As"));
		if (res == 0) {

			if (file_exists(input_name)) {
				const int res = gui2::show_message((*gui_).video(), "", _("The map already exists. Do you want to overwrite it?"), gui2::tmessage::yes_no_buttons);
				overwrite = res == gui2::twindow::CANCEL ? 1 : 0;
			}
			else
				overwrite = 0;
		}
	} while (res == 0 && overwrite != 0);

	// Try to save the map, if it fails we reset the filename.
	if (res == 0) {
		try {
			config file;
			config& map = file.add_child("map");
			map_.write(map);

			std::stringstream str;
			{
				config_writer writer(str, false);
				writer.write(file);
			}
			write_file(input_name, str.str());

			gui2::show_transient_message(gui_->video(), "", _("Map saved."));
		} catch (io_exception& e) {
			utils::string_map symbols;
			symbols["msg"] = e.what();
			const std::string msg = vgettext("Could not save the map: $msg",symbols);
			gui2::show_transient_message(gui_->video(), "", msg);
		}
	}
	*/
}

void menu_handler::preferences()
{
	preferences::show_preferences_dialog(*gui_, game_config_);
	// Needed after changing fullscreen/windowed mode or display resolution
	gui_->redraw_everything();
}

void menu_handler::show_chat_log()
{
        config c;
        c["name"] = "prototype of chat log";
        gui2::tchat_log chat_log_dialog(vconfig(c),&recorder);
        chat_log_dialog.show(resources::screen->video());
        //std::string text = recorder.build_chat_log();
        //gui::show_dialog(*gui_,NULL,_("Chat Log"),"",gui::CLOSE_ONLY,NULL,NULL,"",&text);

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

void menu_handler::recruit(int side_num, const map_location &last_hex)
{
	team &current_team = teams_[side_num - 1];

	std::vector<const unit_type*> sample_units;

	gui_->draw(); //clear the old menu
	std::vector<std::string> item_keys;
	std::vector<std::string> items;

	std::set<std::string> recruits = actions::get_recruits(side_num, last_hex);

	for(std::set<std::string>::const_iterator it = recruits.begin(); it != recruits.end(); ++it) {
		const unit_type *type = unit_types.find(*it);
		if (!type) {
			ERR_NG << "could not find unit '" << *it << "'\n";
			return;
		}

		item_keys.push_back(*it);

		char prefix;
		{ wb::future_map future; // so gold takes into account planned spending
			int wb_gold = resources::whiteboard->get_spent_gold_for(side_num);
			//display units that we can't afford to recruit in red
			prefix = (type->cost() > current_team.gold() - wb_gold
					? font::BAD_TEXT : font::NULL_MARKUP);
		} // end planned unit map scope

		std::stringstream description;
		description << font::IMAGE << type->image();
#ifndef LOW_MEM
		description << "~RC(" << type->flag_rgb() << '>'
			<< team::get_side_color_index(side_num) << ')';
#endif
		description << COLUMN_SEPARATOR << font::LARGE_TEXT << prefix << type->type_name() << "\n"
				<< prefix << type->cost() << " " << sngettext("unit^Gold", "Gold", type->cost());

		items.push_back(description.str());
		sample_units.push_back(type);
	}

	if(sample_units.empty()) {
		gui2::show_transient_message(gui_->video(),"",_("You have no units available to recruit."));
		return;
	}

	int recruit_res = dialogs::recruit_dialog(*gui_, sample_units, items, side_num, get_title_suffix(side_num));

	if(recruit_res != -1) {
		do_recruit(item_keys[recruit_res], side_num, last_hex);
	}
}


void menu_handler::repeat_recruit(int side_num, const map_location &last_hex)
{
	const std::string & last_recruit = teams_[side_num - 1].last_recruit();
	if ( last_recruit.empty() == false )
		do_recruit(last_recruit, side_num, last_hex);
}

bool menu_handler::do_recruit(const std::string &name, int side_num,
	const map_location &last_hex)
{
	team &current_team = teams_[side_num - 1];

	//search for the unit to be recruited in recruits
	if ( !util::contains(actions::get_recruits(side_num, last_hex), name) )
		return false;

	const unit_type *u_type = unit_types.find(name);
	assert(u_type);

	if (u_type->cost() > current_team.gold() - resources::whiteboard->get_spent_gold_for(side_num)) {
		gui2::show_transient_message(gui_->video(), "",
			_("You don’t have enough gold to recruit that unit"));
		return false;
	}

	current_team.last_recruit(name);
	const events::command_disabler disable_commands;

	map_location loc = last_hex;
	map_location recruited_from = map_location::null_location;
	std::string msg;
	{ wb::future_map_if_active future; /* start planned unit map scope if in planning mode */
		msg = actions::find_recruit_location(side_num, loc, recruited_from, name);
	} // end planned unit map scope
	if (!msg.empty()) {
		gui2::show_transient_message(gui_->video(), "", msg);
		return false;
	}

	if (!resources::whiteboard->save_recruit(name, side_num, loc)) {
		//MP_COUNTDOWN grant time bonus for recruiting
		current_team.set_action_bonus_count(1 + current_team.action_bonus_count());

		// Do the recruiting.
		
		synced_context::run_in_synced_context("recruit", replay_helper::get_recruit(u_type->id(), loc, recruited_from));
		return true;
	}
	return false;
}

void menu_handler::recall(int side_num, const map_location &last_hex)
{
	if (level_["disallow_recall"].to_bool()) {
		gui2::show_transient_message(gui_->video(),"",_("You are separated from your soldiers and may not recall them"));
		return;
	}

	team &current_team = teams_[side_num - 1];
	if(!current_team.persistent()) {
		ERR_NG << "cannot recall a unit for side " << side_num
			<< ", which has no recall list!\n";
		return;
	}

	std::vector<const unit*> recall_list_team;
	{ wb::future_map future; // ensures recall list has planned recalls removed
		recall_list_team = actions::get_recalls(side_num, last_hex);
	}

	gui_->draw(); //clear the old menu


	DBG_WB <<"menu_handler::recall: Contents of wb-modified recall list:\n";
	BOOST_FOREACH(const unit* unit, recall_list_team)
	{
		DBG_WB << unit->name() << " [" << unit->id() <<"]\n";
	}

	if(current_team.recall_list().empty()) {
		gui2::show_transient_message(gui_->video(), "",
			_("There are no troops available to recall\n(You must have"
			" veteran survivors from a previous scenario)"));
		return;
	}
	if(recall_list_team.empty()) {
		gui2::show_transient_message(gui_->video(), "",
			_("You currently can't recall at the highlighted location"));
		return;
	}

	int res = dialogs::recall_dialog(*gui_, recall_list_team, side_num, get_title_suffix(side_num), current_team.recall_cost());
	int unit_cost = current_team.recall_cost();
	if (res < 0) { 
		return;
	}
	// we need to check if unit has a specific recall cost
	// if it does we use it elsewise we use the team.recall_cost()
	// the magic number -1 is what it gets set to if the unit doesn't
	// have a special recall_cost of its own.
	else if(recall_list_team[res]->recall_cost() > -1) {
		unit_cost = recall_list_team[res]->recall_cost();
	}

	int wb_gold = resources::whiteboard->get_spent_gold_for(side_num);
	if (current_team.gold() - wb_gold < unit_cost) {
		utils::string_map i18n_symbols;
		i18n_symbols["cost"] = lexical_cast<std::string>(unit_cost);
		std::string msg = vngettext(
			"You must have at least 1 gold piece to recall a unit",
			"You must have at least $cost gold pieces to recall this unit",
			unit_cost, i18n_symbols);
		gui2::show_transient_message(gui_->video(), "", msg);
		return;
	}

	LOG_NG << "recall index: " << res << "\n";
	const events::command_disabler disable_commands;

	map_location recall_location = last_hex;
	map_location recall_from = map_location::null_location;
	std::string err;
	{ wb::future_map_if_active future; // future unit map removes invisible units from map, don't do this outside of planning mode
		err = actions::find_recall_location(side_num, recall_location, recall_from, *(recall_list_team[res]));
	} // end planned unit map scope
	if(!err.empty()) {
		gui2::show_transient_message(gui_->video(), "", err);
		return;
	}

	if (!resources::whiteboard->save_recall(*recall_list_team[res], side_num, recall_location)) {
		bool success = synced_context::run_in_synced_context("recall", 
			replay_helper::get_recall(recall_list_team[res]->id(), recall_location, recall_from),
			true,
			true,
			true,
			synced_context::ignore_error_function);

		if(!success)
		{
			ERR_NG << "menu_handler::recall(): Unit does not exist in the recall list.\n";
		}
	}
}


// Highlights squares that an enemy could move to on their turn, showing how many can reach each square.
void menu_handler::show_enemy_moves(bool ignore_units, int side_num)
{
	wb::future_map future; // use unit positions as if all planned actions were executed

	gui_->unhighlight_reach();

	// Compute enemy movement positions
	for(unit_map::iterator u = units_.begin(); u != units_.end(); ++u) {
		bool invisible = u->invisible(u->get_location());

		if (teams_[side_num - 1].is_enemy(u->side()) &&
		    !gui_->fogged(u->get_location()) && !u->incapacitated() && !invisible)
		{
			const unit_movement_resetter move_reset(*u);
			const pathfind::paths& path = pathfind::paths(*u, false, true,
				teams_[gui_->viewing_team()], 0, false, ignore_units);

			gui_->highlight_another_reach(path);
		}
	}
}

void menu_handler::toggle_shroud_updates(int side_num)
{
	team &current_team = teams_[side_num - 1];
	bool auto_shroud = current_team.auto_shroud_updates();
	// If we're turning automatic shroud updates on, then commit all moves
	if (!auto_shroud) update_shroud_now(side_num);

	// Toggle the setting and record this.
	synced_context::run_in_synced_context("auto_shroud", replay_helper::get_auto_shroud(!auto_shroud));
	resources::undo_stack->add_auto_shroud(!auto_shroud);
}

void menu_handler::update_shroud_now(int /* side_num */)
{
	synced_context::run_in_synced_context("update_shroud", replay_helper::get_update_shroud());
}


namespace { // Helpers for menu_handler::end_turn()
	/**
	 * Returns true if @a side_num has at least one living unit.
	 */
	bool units_alive(int side_num, const unit_map & units)
	{
		for ( unit_map::const_iterator un = units.begin(); un != units.end(); ++un ) {
			if ( un->side() == side_num )
				return true;
		}
		return false;
	}
	/**
	 * Returns true if @a side_num has at least one unit that can still move.
	 */
	bool partmoved_units(int side_num, const unit_map & units)
	{
		for ( unit_map::const_iterator un = units.begin(); un != units.end(); ++un ) {
			if ( un->side() == side_num ) {
				// @todo whiteboard should take into consideration units that have
				// a planned move but can still plan more movement in the same turn
				if ( actions::unit_can_move(*un) && !un->user_end_turn()
						&& !resources::whiteboard->unit_has_actions(&*un) )
					return true;
			}
		}
		return false;
	}
	/**
	 * Returns true if @a side_num has at least one unit that (can but) has not
	 * moved.
	 */
	bool unmoved_units(int side_num, const unit_map & units)
	{
		for ( unit_map::const_iterator un = units.begin(); un != units.end(); ++un ) {
			if ( un->side() == side_num ) {
				if ( actions::unit_can_move(*un)  &&  !un->has_moved()  && !un->user_end_turn()
						&& !resources::whiteboard->unit_has_actions(&*un) )
					return true;
			}
		}
		return false;
	}
}

bool menu_handler::end_turn(int side_num)
{
	if(!resources::gamedata->allow_end_turn()) {
		gui2::show_message((*gui_).video(), "", _("You cannot end your turn yet!"), gui2::tmessage::ok_button);
		return false;
	}

	size_t team_num = static_cast<size_t>(side_num - 1);
	if ( team_num < teams_.size()  &&  teams_[team_num].no_turn_confirmation() ) {
		// Skip the confirmations that follow.
	}
	// Ask for confirmation if the player hasn't made any moves.
	else if ( preferences::confirm_no_moves()  &&
	          !resources::undo_stack->player_acted()  &&
	          !resources::whiteboard->current_side_has_actions()  &&
	          units_alive(side_num, units_) )
	{
		const int res = gui2::show_message((*gui_).video(), "", _("You have not started your turn yet. Do you really want to end your turn?"), gui2::tmessage::yes_no_buttons);
		if(res == gui2::twindow::CANCEL) {
			return false;
		}
	}
	// Ask for confirmation if units still have some movement left.
	else if ( preferences::yellow_confirm() && partmoved_units(side_num, units_) ) {
		const int res = gui2::show_message((*gui_).video(), "", _("Some units have movement left. Do you really want to end your turn?"), gui2::tmessage::yes_no_buttons);
		if(res == gui2::twindow::CANCEL) {
			return false;
		}
	}
	// Ask for confirmation if units still have all movement left.
	else if ( preferences::green_confirm() && unmoved_units(side_num, units_) ) {
		const int res = gui2::show_message((*gui_).video(), "", _("Some units have not moved. Do you really want to end your turn?"), gui2::tmessage::yes_no_buttons);
		if(res == gui2::twindow::CANCEL) {
			return false;
		}
	}

	// Auto-execute remaining whiteboard planned actions
	// Only finish turn if they all execute successfully, i.e. no ambush, etc.
	if (!resources::whiteboard->allow_end_turn()) {
		return false;
	}

	return true;
}

void menu_handler::goto_leader(int side_num)
{
	unit_map::const_iterator i = units_.find_leader(side_num);
	if(i != units_.end()) {
		gui_->scroll_to_tile(i->get_location(), game_display::WARP);
	}
}

void menu_handler::unit_description()
{
	const unit_map::const_iterator un = current_unit();
	if(un != units_.end()) {
		dialogs::show_unit_description(*un);
	}
}

void menu_handler::terrain_description(mouse_handler& mousehandler)
{
	const map_location& loc = mousehandler.get_last_hex();
	if (map_.on_board(loc) == false) {
		return;
	}

	const terrain_type& type = map_.get_terrain_info(loc);
	//const terrain_type& info = resources::game_map->get_terrain_info(terrain);
	dialogs::show_terrain_description(type);
}

void menu_handler::rename_unit()
{
	const unit_map::iterator un = current_unit();
	if (un == units_.end() || gui_->viewing_side() != un->side())
		return;
	if (un->unrenamable())
		return;

	std::string name = un->name();
	const std::string title(N_("Rename Unit"));
	const std::string label(N_("Name:"));

	if(gui2::tedit_text::execute(title, label, name, gui_->video())) {
		recorder.add_rename(name, un->get_location());
		un->rename(name);
		gui_->invalidate_unit();
	}
}

unit_map::iterator menu_handler::current_unit()
{
	const mouse_handler& mousehandler = resources::controller->get_mouse_handler_base();

	unit_map::iterator res = find_visible_unit(mousehandler.get_last_hex(),
		teams_[gui_->viewing_team()]);
	if(res != units_.end()) {
		return res;
	} else {
		return find_visible_unit(mousehandler.get_selected_hex(),
			teams_[gui_->viewing_team()]);
	}
}

namespace { // Helpers for create_unit()
	/// Allows a function to return both a type and a gender.
	typedef std::pair<const unit_type *, unit_race::GENDER> type_and_gender;

	/**
	 * Allows the user to select a type of unit, using GUI2.
	 * (Intended for use when a unit is created in debug mode via hotkey or
	 * context menu.)
	 * @returns the selected type and gender. If this is canceled, the
	 *          returned type is NULL.
	 *
	 * @todo Replace choose_unit() when complete.
	 */
	type_and_gender choose_unit_2(game_display& gui)
	{
		//
		// The unit creation dialog makes sure unit types
		// are properly cached.
		//
		gui2::tunit_create create_dlg;
		create_dlg.show(gui.video());

		if(create_dlg.no_choice()) {
			// the static cast fixes http://connect.microsoft.com/VisualStudio/feedback/details/520043/
			// c++11's nullptr would be a better solution as soon as we support it.
			return type_and_gender(static_cast<const unit_type *>(NULL), unit_race::NUM_GENDERS);
		}

		const std::string& ut_id = create_dlg.choice();
		const unit_type *utp = unit_types.find(ut_id);
		if (!utp) {
			ERR_NG << "Create unit dialog returned nonexistent or unusable unit_type id '" << ut_id << "'.\n";
			return type_and_gender(static_cast<const unit_type *>(NULL), unit_race::NUM_GENDERS);
		}
		const unit_type &ut = *utp;

		unit_race::GENDER gender = create_dlg.gender();
		// Do not try to set bad genders, may mess up l10n
		/// @todo Is this actually necessary?
		/// (Maybe create_dlg can enforce proper gender selection?)
		if(ut.genders().end() == std::find(ut.genders().begin(), ut.genders().end(), gender)) {
			gender = ut.genders().front();
		}

		return type_and_gender(utp, gender);
	}

	/**
	 * Allows the user to select a type of unit.
	 * (Intended for use when a unit is created in debug mode via hotkey or
	 * context menu.)
	 * @returns the selected type and gender. If this is canceled, the
	 *          returned type is NULL.
	 */
	type_and_gender choose_unit(game_display& gui)
	{
		std::vector<std::string> options;
		static int last_selection = -1;
		static bool random_gender = false;
		std::vector<const unit_type*> unit_choices;
		const std::string heading = std::string(1,HEADING_PREFIX) +
									_("Race")      + COLUMN_SEPARATOR +
									_("Type");
		options.push_back(heading);

		BOOST_FOREACH(const unit_type_data::unit_type_map::value_type &i, unit_types.types())
		{
			std::stringstream row;

			// Make sure the unit type was built for the data we need.
			unit_types.build_unit_type(i.second, unit_type::HELP_INDEXED);

			row << i.second.race()->plural_name() << COLUMN_SEPARATOR;
			row << i.second.type_name() << COLUMN_SEPARATOR;

			options.push_back(row.str());
			unit_choices.push_back(&i.second);
		}

		int choice = 0;
		bool random_gender_choice = random_gender;
		{
			gui::dialog umenu(gui, _("Create Unit (Debug!)"), "", gui::OK_CANCEL);

			umenu.add_option(
				(formatter()<<_("Gender: ")<<_("gender^Random")).str(),
				random_gender_choice,
				gui::dialog::BUTTON_EXTRA
			);

			gui::menu::basic_sorter sorter;
			sorter.set_alpha_sort(0).set_alpha_sort(1);
			umenu.set_menu(options, &sorter);

			gui::filter_textbox* filter = new gui::filter_textbox(gui.video(),
				_("Filter: "), options, options, 1, umenu, 200);
			umenu.set_textbox(filter);

			//sort by race then by type name
			umenu.get_menu().sort_by(1);
			umenu.get_menu().sort_by(0);
			if (last_selection >= 0)
				umenu.get_menu().move_selection(last_selection);
			else
				umenu.get_menu().reset_selection();

			dialogs::unit_types_preview_pane unit_preview(unit_choices, filter, 1, dialogs::unit_types_preview_pane::SHOW_ALL);
			umenu.add_pane(&unit_preview);
			unit_preview.set_selection(umenu.get_menu().selection());

			choice = umenu.show();
			choice = filter->get_index(choice);
			random_gender_choice = umenu.option_checked(0);
		}

		if (size_t(choice) < unit_choices.size()) {
			last_selection = choice;
			random_gender  = random_gender_choice;

			return type_and_gender(unit_choices[choice],
			                       random_gender ? unit_race::NUM_GENDERS :
			                                       unit_choices[choice]->genders().front());
		}
		else
			return type_and_gender(static_cast<const unit_type *>(NULL), unit_race::NUM_GENDERS);
	}

	/**
	 * Creates a unit and places it on the board.
	 * (Intended for use with any units created via debug mode.)
	 */
	void create_and_place(game_display& gui, const gamemap & map, unit_map & units,
	                      const map_location & loc, const unit_type & u_type,
	                      unit_race::GENDER gender = unit_race::NUM_GENDERS)
	{
		// Create the unit.
		unit created(u_type, 1, true, gender);
		created.new_turn();

		// Add the unit to the board.
		std::pair<unit_map::iterator, bool> add_result =
			units.replace(loc, created);
		gui.invalidate_unit();
		unit_display::unit_recruited(loc);

		// Village capture?
		if ( map.is_village(loc) )
			actions::get_village(loc, created.side());

		// Update fog/shroud.
		actions::shroud_clearer clearer;
		clearer.clear_unit(loc, created);
		clearer.fire_events();
		if ( add_result.first.valid() ) // In case sighted events messed with the unit.
			actions::actor_sighted(*add_result.first);
	}

}// Anonymous namespace


/**
 * Creates a unit (in debug mode via hotkey or context menu).
 */
void menu_handler::create_unit(mouse_handler& mousehandler)
{
	// Save the current mouse location before popping up the choice menu (which
	// gives time for the mouse to move, changing the location).
	const map_location destination = mousehandler.get_last_hex();
	assert(gui_ != NULL);

	// Let the user select the kind of unit to create.
	type_and_gender selection = gui2::new_widgets ? choose_unit_2(*gui_) :
	                                                choose_unit(*gui_);
	if ( selection.first != NULL )
		// Make it so.
		create_and_place(*gui_, map_, units_, destination,
		                 *selection.first, selection.second);
}

void menu_handler::change_side(mouse_handler& mousehandler)
{
	const map_location& loc = mousehandler.get_last_hex();
	const unit_map::iterator i = units_.find(loc);
	if(i == units_.end()) {
		if(!map_.is_village(loc))
			return;

		// village_owner returns -1 for free village, so team 0 will get it
		int team = village_owner(loc) + 1;
		// team is 0-based so team=team::nteams() is not a team
		// but this will make get_village free it
		if(team > team::nteams()) {
			team = 0;
		}
		actions::get_village(loc, team + 1);
	} else {
		int side = i->side();
		++side;
		if(side > team::nteams()) {
			side = 1;
		}
		i->set_side(side);

		if(map_.is_village(loc)) {
			actions::get_village(loc, side);
		}
	}
}

void menu_handler::kill_unit(mouse_handler& mousehandler)
{
	const map_location& loc = mousehandler.get_last_hex();
	const unit_map::iterator i = units_.find(loc);
	if(i != units_.end()) {
		const int dying_side = i->side();
		game_events::fire("last breath", loc, loc);
		if (i.valid()) {
			unit_display::unit_die(loc, *i);
		}
		resources::screen->redraw_minimap();
		game_events::fire("die", loc, loc);
		if (i.valid()) {
			resources::units->erase(i);
		}
		actions::recalculate_fog(dying_side);
		resources::controller->check_victory();
	}
}

void menu_handler::label_terrain(mouse_handler& mousehandler, bool team_only)
{
	const map_location& loc = mousehandler.get_last_hex();
	if (map_.on_board(loc) == false) {
		return;
	}

	const terrain_label* old_label = gui_->labels().get_label(loc);
	std::string label = old_label ? old_label->text() : "";

	if(gui2::tedit_label::execute(label, team_only, gui_->video())) {
		std::string team_name;
		SDL_Color color = font::LABEL_COLOR;

		if (team_only) {
			team_name = gui_->labels().team_name();
		} else {
			color = int_to_color(team::get_side_rgb(gui_->viewing_side()));
		}
		const terrain_label* res = gui_->labels().set_label(loc, label, team_name, color);
		if (res)
			recorder.add_label(res);
	}
}

void menu_handler::clear_labels()
{
	if (gui_->team_valid()
	   && !is_observer())
	{
		gui_->labels().clear(gui_->current_team_name(), false);
		recorder.clear_labels(gui_->current_team_name(), false);
	}
}

void menu_handler::continue_move(mouse_handler &mousehandler, int side_num)
{
	unit_map::iterator i = current_unit();
	if (i == units_.end() || !i->move_interrupted()) {
		i = units_.find(mousehandler.get_selected_hex());
		if (i == units_.end() || !i->move_interrupted()) return;
	}
	move_unit_to_loc(i, i->get_interrupted_move(), true,
		side_num, mousehandler);
}

void menu_handler::move_unit_to_loc(const unit_map::iterator &ui,
	const map_location& target, bool continue_move, int side_num,
	mouse_handler &mousehandler)
{
	assert(ui != units_.end());

	pathfind::marked_route route = mousehandler.get_route(&*ui, target, teams_[side_num - 1]);

	if(route.steps.empty())
		return;

	assert(route.steps.front() == ui->get_location());

	gui_->set_route(&route);
	gui_->unhighlight_reach();
	{
		LOG_NG << "move_unit_to_loc " << route.steps.front() << " to " << route.steps.back() << "\n";
		actions::move_unit_and_record(route.steps, resources::undo_stack, continue_move);
	}
	gui_->set_route(NULL);
	gui_->invalidate_game_status();
}

void menu_handler::execute_gotos(mouse_handler &mousehandler, int side)
{
	// we will loop on all gotos and try to fully move a maximum of them,
	// but we want to avoid multiple blocking of the same unit,
	// so, if possible, it's better to first wait that the blocker move

	bool wait_blocker_move = true;
	std::set<map_location> fully_moved;

	bool change = false;
	bool blocked_unit = false;
	do {
		change = false;
		blocked_unit = false;
		for(unit_map::iterator ui = units_.begin(); ui != units_.end(); ++ui) {
			if (ui->side() != side  || ui->movement_left() == 0)
				continue;

			const map_location &current_loc = ui->get_location();
			const map_location &goto_loc = ui->get_goto();

			if(goto_loc == current_loc){
				ui->set_goto(map_location());
				continue;
			}

			if(!map_.on_board(goto_loc))
				continue;

			// avoid pathfinding calls for finished units
			if(fully_moved.count(current_loc))
				continue;

			pathfind::marked_route route = mousehandler.get_route(&*ui, goto_loc, teams_[side - 1]);

			if(route.steps.size() <= 1) { // invalid path
				fully_moved.insert(current_loc);
				continue;
			}

			// look where we will stop this turn (turn_1 waypoint or goto)
			map_location next_stop = goto_loc;
			pathfind::marked_route::mark_map::const_iterator w = route.marks.begin();
			for(; w != route.marks.end(); ++w) {
				if (w->second.turns == 1) {
					next_stop = w->first;
					break;
				}
			}

			if(next_stop == current_loc) {
				fully_moved.insert(current_loc);
				continue;
			}

			// we delay each blocked move because some other change
			// may open a another not blocked path
			if(units_.count(next_stop)) {
				blocked_unit = true;
				if (wait_blocker_move)
					continue;
			}

			gui_->set_route(&route);

			{
				LOG_NG << "execute goto from " << route.steps.front() << " to " << route.steps.back() << "\n";
				int moves = actions::move_unit_and_record(route.steps, resources::undo_stack);
				change = moves > 0;
			}

			if (change) {
				// something changed, resume waiting blocker (maybe one can move now)
				wait_blocker_move = true;
			}
		}

		if(!change && wait_blocker_move) {
			// no change when waiting, stop waiting and retry
			wait_blocker_move = false;
			change = true;
		}
	} while(change && blocked_unit);

	// erase the footsteps after movement
	gui_->set_route(NULL);
	gui_->invalidate_game_status();
}


void menu_handler::toggle_ellipses()
{
	preferences::set_ellipses(!preferences::ellipses());
	gui_->invalidate_all();
}

void menu_handler::toggle_grid()
{
	preferences::set_grid(!preferences::grid());
	gui_->invalidate_all();
}

void menu_handler::unit_hold_position(mouse_handler &mousehandler, int side_num)
{
	const unit_map::iterator un = units_.find(mousehandler.get_selected_hex());
	if (un != units_.end() && un->side() == side_num && un->movement_left() >= 0)
	{
		un->toggle_hold_position();
		gui_->invalidate(mousehandler.get_selected_hex());

		mousehandler.set_current_paths(pathfind::paths());
		gui_->draw();

		if (un->hold_position()) {
			mousehandler.cycle_units(false);
		}
	}
}

void menu_handler::end_unit_turn(mouse_handler &mousehandler, int side_num)
{
	const unit_map::iterator un = units_.find(mousehandler.get_selected_hex());
	if (un != units_.end() && un->side() == side_num && un->movement_left() >= 0)
	{
		un->toggle_user_end_turn();
		gui_->invalidate(mousehandler.get_selected_hex());

		mousehandler.set_current_paths(pathfind::paths());
		gui_->draw();

		if (un->user_end_turn()) {
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
	//None of the two parameters really needs to be passed since the information belong to members of the class.
	//But since it makes the called method more generic, it is done anyway.
	chat_handler::do_speak(textbox_info_.box()->text(),textbox_info_.check() != NULL ? textbox_info_.check()->checked() : false);
}


void menu_handler::add_chat_message(const time_t& time,
		const std::string& speaker, int side, const std::string& message,
		events::chat_handler::MESSAGE_TYPE type)
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
				std::string data(str_, args[n]);
				return utils::strip(data);
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
				utils::string_map symbols;
				symbols["command"] = get_cmd();
				symbols["help_command"] = cmd_prefix_ + "help";
				print("help", VGETTEXT("Unknown command '$command', try $help_command "
					"for a list of available commands.", symbols));
			}
		}

		std::vector<std::string> get_commands_list() const
		{
			std::vector<std::string> res;
			BOOST_FOREACH(typename command_map::value_type i, command_map_) {
				res.push_back(i.first);
			}
			return res;
		}

	protected:
		void init_map_default()
		{
			register_command("help", &map_command_handler<Worker>::help,
				_("Available commands list and command-specific help. "
				"Use \"help all\" to include currently unavailable commands."),
				_("do not translate the 'all'^[all|<command>]"));
		}
		//derived classes initialize the map overriding this function
		virtual void init_map() = 0;
		//overridden in derived classes to actually print the messages somwehere
		virtual void print(const std::string& title, const std::string& message) = 0;
		//should be overridden in derived classes if the commands have flags
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
		//this should be overridden if e.g. flags are used to control command
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
			print(get_cmd(), _("Error:") + std::string(" ") + message);
		}
		void command_failed_need_arg(int argn)
		{
			utils::string_map symbols;
			symbols["arg_id"] = lexical_cast<std::string>(argn);
			command_failed(VGETTEXT("Missing argument $arg_id", symbols));
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
			utils::string_map symbols;
			symbols["flags_description"] = get_flags_description();
			symbols["list_of_commands"] = ss.str();
			symbols["help_command"] = cmd_prefix_ + "help";
			print(_("help"), VGETTEXT("Available commands $flags_description:\n$list_of_commands", symbols));
			print(_("help"), VGETTEXT("Type $help_command <command> for more info.", symbols));
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
					ss << " " << _("Usage:") << " " << cmd_prefix_ << cmd << " " << c->usage;
				}
				ss << get_command_flags_description(*c);
				const std::vector<std::string> l = get_aliases(cmd);
				if (!l.empty()) {
					ss << " (" << _("aliases:") << " " << utils::join(l," ") << ")";
				}
				print(_("help"), ss.str());
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
		void do_room_query();
		void do_room_query_noarg();
		void do_gen_room_query();
		void do_whisper();
		void do_chanmsg();
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
			return _("(A) — admin command");
		}

		std::string get_command_flags_description(
			const map_command_handler<chat_command_handler>::command& c) const
		{
			if (c.has_flag('A')) {
				return std::string(" ") + _("(admin only)");
			} else {
				return "";
			}
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
			register_alias("query", "q");
			register_command("ban", &chat_command_handler::do_network_send_req_arg,
				_("Ban and kick a player or observer. If he is not in the"
				" game but on the server he will only be banned."), _("<nickname>"));
			register_command("unban", &chat_command_handler::do_network_send_req_arg,
				_("Unban a user. He does not have to be in the game but on"
				" the server."), _("<nickname>"));
			register_command("kick", &chat_command_handler::do_network_send_req_arg,
				_("Kick a player or observer."), _("<nickname>"));
			register_command("mute", &chat_command_handler::do_network_send,
				_("Mute an observer. Without an argument displays the mute status."), _("<nickname>"));
			register_command("unmute", &chat_command_handler::do_network_send,
				_("Unmute an observer. Without an argument unmutes everyone."), _("<nickname>"));
			register_command("muteall", &chat_command_handler::do_network_send,
				_("Mute/Unmute all observers. (toggles)"), "");
			register_command("ping", &chat_command_handler::do_network_send,
				"");
			register_command("green", &chat_command_handler::do_network_send_req_arg,
				"", "", "A");
			register_command("red", &chat_command_handler::do_network_send_req_arg,
				"", "", "A");
			register_command("yellow", &chat_command_handler::do_network_send_req_arg,
				"", "", "A");
			register_command("report", &chat_command_handler::do_network_send_req_arg,
				_("Report abuse, rule violations, etc. to the server moderators. "
				"Make sure to mention relevant nicknames, etc."), "");
			register_alias("report", "adminmsg");  // deprecated
			register_command("emote", &chat_command_handler::do_emote,
				_("Send an emotion or personal action in chat."), _("<message>"));
			register_alias("emote", "me");
			register_command("whisper", &chat_command_handler::do_whisper,
				_("Sends a private message. "
				"You cannot send private messages to players in a running game you observe or play in."),
				_("<nickname> <message>"));
			register_alias("whisper", "msg");
			register_alias("whisper", "m");
			register_command("log", &chat_command_handler::do_log,
				_("Change the log level of a log domain."), _("<level> <domain>"));
			register_command("ignore", &chat_command_handler::do_ignore,
				_("Add a nickname to your ignores list."), _("<nickname>"));
			register_command("friend", &chat_command_handler::do_friend,
				_("Add a nickname to your friends list."), _("<nickname>"));
			register_command("remove", &chat_command_handler::do_remove,
				_("Remove a nickname from your ignores or friends list."), _("<nickname>"));
			register_command("list", &chat_command_handler::do_display,
				_("Show your ignores and friends list."));
			register_alias("list", "display");
			register_command("version", &chat_command_handler::do_version,
				_("Display version information."));
			register_command("register", &chat_command_handler::do_register,
				_("Register your nickname"), _("<password> <email (optional)>"));
			register_command("drop", &chat_command_handler::do_drop,
				_("Drop your nickname."));
			register_command("set", &chat_command_handler::do_set,
				_("Update details for your nickname. For possible details see '/details'."),
				_("<detail> <value>"));
			register_command("info", &chat_command_handler::do_info,
				_("Request information about a nickname."), _("<nickname>"));
			register_command("details", &chat_command_handler::do_details,
				_("Request a list of details you can set for your registered nickname."));
			register_command("join", &chat_command_handler::do_network_send_req_arg,
				_("Join a room."), _("<room>"));
			register_alias("join", "j");
			register_command("part", &chat_command_handler::do_network_send_req_arg,
				_("Part a room."), _("<room>"));
			register_command("names", &chat_command_handler::do_room_query,
				_("List room members."), _("<room>"));
			register_command("rooms", &chat_command_handler::do_room_query_noarg,
				_("List available rooms."));
			register_command("room", &chat_command_handler::do_chanmsg,
				_("Room message."), _("<room> <msg>"));
			register_command("room_query", &chat_command_handler::do_gen_room_query,
				_("Room query."), _("<room> <type> [value]"));
			register_alias("room_query", "rq");
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
		console_handler(menu_handler& menu_handler)
		: chmap(), chat_command_handler(menu_handler, true), menu_handler_(menu_handler), team_num_(resources::controller->current_side())
		{}
		using chmap::dispatch; //disambiguate
		using chmap::get_commands_list;

	protected:
		//chat_command_handler's init_map() and handlers will end up calling these.
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
		void do_idle();
		void do_theme();
		void do_control();
		void do_controller();
		void do_clear();
		void do_sunset();
		void do_foreground();
		void do_layers();
		void do_fps();
		void do_benchmark();
		void do_save();
		void do_save_quit();
		void do_quit();
		void do_ignore_replay_errors();
		void do_nosaves();
		void do_next_level();
		void do_choose_level();
		void do_turn();
		void do_turn_limit();
		void do_debug();
		void do_nodebug();
		void do_lua();
		void do_unsafe_lua();
		void do_custom();
		void do_set_alias();
		void do_set_var();
		void do_show_var();
		void do_inspect();
		void do_control_dialog();
		void do_manage();
		void do_unit();
		// void do_buff();
		// void do_unbuff();
		void do_discover();
		void do_undiscover();
		void do_create();
		void do_fog();
		void do_shroud();
		void do_gold();
		void do_event();
		void do_toggle_draw_coordinates();
		void do_toggle_draw_terrain_codes();
		void do_toggle_whiteboard();
		void do_whiteboard_options();

		std::string get_flags_description() const {
			return _("(D) — debug only, (N) — network only, (A) — admin only");
		}
		using chat_command_handler::get_command_flags_description; //silence a warning
		std::string get_command_flags_description(const chmap::command& c) const
		{
			std::string space(" ");
			return (c.has_flag('D') ? space + _("(debug command)") : "")
				 + (c.has_flag('N') ? space + _("(network only)") : "")
				 + (c.has_flag('A') ? space + _("(admin only)") : "");
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
				_("Switch a side to/from AI control."), _("do not translate the on/off^[<side> [on/off]]"));
			register_command("idle", &console_handler::do_idle,
				_("Switch a side to/from idle state."), _("do not translate the on/off^[<side> [on/off]]"));
			register_command("theme", &console_handler::do_theme);
			register_command("control", &console_handler::do_control,
				_("Assign control of a side to a different player or observer."), _("<side> <nickname>"), "N");
			register_command("controller", &console_handler::do_controller,
				_("Query the controller status of a side."), _("<side>"));
			register_command("clear", &console_handler::do_clear,
				_("Clear chat history."));
			register_command("sunset", &console_handler::do_sunset,
				_("Visualize the screen refresh procedure."), "", "D");
			register_command("foreground", &console_handler::do_foreground,
				_("Debug foreground terrain."), "", "D");
			register_command("layers", &console_handler::do_layers,
				_("Debug layers from terrain under the mouse."), "", "D");
			register_command("fps", &console_handler::do_fps, _("Show fps."));
			register_command("benchmark", &console_handler::do_benchmark);
			register_command("save", &console_handler::do_save, _("Save game."));
			register_alias("save", "w");
			register_command("quit", &console_handler::do_quit, _("Quit game."));
			// Note the next value is used hardcoded in the init tests.
			register_alias("quit", "q!");
			register_command("save_quit", &console_handler::do_save_quit,
				_("Save and quit."));
			register_alias("save_quit", "wq");
			register_command("ignore_replay_errors", &console_handler::do_ignore_replay_errors,
				_("Ignore replay errors."));
			register_command("nosaves", &console_handler::do_nosaves,
				_("Disable autosaves."));
			register_command("next_level", &console_handler::do_next_level,
				_("Advance to the next scenario, or scenario identified by 'id'"), _("<id>"), "D");
			register_alias("next_level", "n");
			register_command("choose_level", &console_handler::do_choose_level,
				_("Choose next scenario"), "", "D");
			register_alias("choose_level", "cl");
			register_command("turn", &console_handler::do_turn,
				_("Change turn number (and time of day), or increase by one if no number is specified."), _("[turn]"), "D");
			register_command("turn_limit", &console_handler::do_turn_limit,
				_("Change turn limit, or turn the turn limit off if no number is specified or it’s −1."), _("[limit]"), "D");
			register_command("debug", &console_handler::do_debug,
				_("Turn debug mode on."));
			register_command("nodebug", &console_handler::do_nodebug,
				_("Turn debug mode off."), "", "D");
			register_command("lua", &console_handler::do_lua,
				_("Execute a Lua statement."), _("<command>[;<command>...]"), "D");
			register_command("unsafe_lua", &console_handler::do_unsafe_lua,
				_("Grant higher privileges to Lua scripts."), "", "D");
			register_command("custom", &console_handler::do_custom,
				_("Set the command used by the custom command hotkey"), _("<command>[;<command>...]"));
			register_command("give_control"
					, &console_handler::do_control_dialog
					, _("Invoke a dialog allowing changing control of MP sides.")
					, ""
					, "N");
			register_command("inspect", &console_handler::do_inspect,
				_("Launch the gamestate inspector"), "", "D");
			register_command("manage", &console_handler::do_manage,
				_("Manage persistence data"), "", "D");
			register_command("alias", &console_handler::do_set_alias,
				_("Set or show alias to a command"), _("<name>[=<command>]"));
			register_command("set_var", &console_handler::do_set_var,
				_("Set a scenario variable."), _("<var>=<value>"), "D");
			register_command("show_var", &console_handler::do_show_var,
				_("Show a scenario variable."), _("<var>"), "D");
			register_command("unit", &console_handler::do_unit,
				_("Modify a unit variable. (Only top level keys are supported.)"), "", "D");

			// register_command("buff", &console_handler::do_buff,
			//    _("Add a trait to a unit."), "", "D");
			// register_command("unbuff", &console_handler::do_unbuff,
			//    _("Remove a trait from a unit. (Does not work yet.)"), "", "D");
			register_command("discover", &console_handler::do_discover,
				_("Discover all units in help."), "");
			register_command("undiscover", &console_handler::do_undiscover,
				  _("'Undiscover' all units in help."), "");
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
			register_command("whiteboard", &console_handler::do_toggle_whiteboard,
				_("Toggle planning mode."));
			register_alias("whiteboard", "wb");
			register_command("whiteboard_options", &console_handler::do_whiteboard_options,
				_("Access whiteboard options dialog."));
			register_alias("whiteboard_options", "wbo");

			if (const config &alias_list = preferences::get_alias())
			{
				BOOST_FOREACH(const config::attribute &a, alias_list.attribute_range()) {
					register_alias(a.second, a.first);
				}
			}
		}
	private:
		menu_handler& menu_handler_;
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
 * @param data string of the form: "@<level@> @<domain@>"
 */
void chat_handler::change_logging(const std::string& data) {
	const std::string::const_iterator j =
			std::find(data.begin(), data.end(), ' ');
	if (j == data.end()) return;
	const std::string level(data.begin(),j);
	const std::string domain(j+1,data.end());
	int severity;
	if (level == "error") severity = lg::err.get_severity();
	else if (level == "warning") severity = lg::warn.get_severity();
	else if (level == "info") severity = lg::info.get_severity();
	else if (level == "debug") severity = lg::debug.get_severity();
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
	} else if (cmd == "ban" || cmd == "unban" || cmd == "kick"
	|| cmd == "mute" || cmd == "unmute") {
		data.add_child(cmd)["username"] = args;
	} else if (cmd == "ping") {
		data[cmd] = lexical_cast<std::string>(time(NULL));
	} else if (cmd == "green") {
		data.add_child("query")["type"] = "lobbymsg @" + args;
	} else if (cmd == "red") {
		data.add_child("query")["type"] = "lobbymsg #" + args;
	} else if (cmd == "yellow") {
		data.add_child("query")["type"] = "lobbymsg <255,255,0>" + args;
	} else if (cmd == "report") {
		data.add_child("query")["type"] = "report " + args;
	} else if (cmd == "join") {
		data.add_child("room_join")["room"] = args;
	} else if (cmd == "part") {
		data.add_child("room_part")["room"] = args;
	}
	network::send_data(data, 0);
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

void chat_handler::user_relation_changed(const std::string& /*name*/)
{
}

void chat_handler::send_whisper(const std::string& receiver, const std::string& message)
{
	config cwhisper, data;
	cwhisper["receiver"] = receiver;
	cwhisper["message"] = message;
	cwhisper["sender"] = preferences::login();
	data.add_child("whisper", cwhisper);
	network::send_data(data, 0);
}

void chat_handler::add_whisper_sent(const std::string& receiver, const std::string& message)
{
	utils::string_map symbols;
	symbols["receiver"] = receiver;
	add_chat_message(time(NULL), VGETTEXT("whisper to $receiver", symbols), 0, message);
}

void chat_handler::add_whisper_received(const std::string& sender, const std::string& message)
{
	utils::string_map symbols;
	symbols["sender"] = sender;
	add_chat_message(time(NULL), VGETTEXT("whisper: $sender", symbols), 0, message);
}

void chat_handler::send_chat_room_message(const std::string& room,
	const std::string& message)
{
	config cmsg, data;
	cmsg["room"] = room;
	cmsg["message"] = message;
	cmsg["sender"] = preferences::login();
	data.add_child("message", cmsg);
	network::send_data(data, 0);
}

void chat_handler::add_chat_room_message_sent(const std::string &room, const std::string &message)
{
	add_chat_room_message_received(room, preferences::login(), message);
}

void chat_handler::add_chat_room_message_received(const std::string &room,
	const std::string &speaker, const std::string &message)
{
	add_chat_message(time(NULL), room + ": " + speaker, 0, message, events::chat_handler::MESSAGE_PRIVATE);
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

void chat_command_handler::do_room_query_noarg()
{
	config data;
	config& q = data.add_child("room_query");
	q.add_child(get_cmd());
	network::send_data(data, 0);
}

void chat_command_handler::do_room_query()
{
	if (get_data(1).empty()) return command_failed_need_arg(1);
	config data;
	config& q = data.add_child("room_query");
	q["room"] = get_arg(1);
	q.add_child(get_cmd());
	network::send_data(data, 0);
}

void chat_command_handler::do_gen_room_query()
{
	if (get_data(1).empty()) return command_failed_need_arg(1);
	config data;
	config& q = data.add_child("room_query");
	q["room"] = get_arg(1);
	config& c = q.add_child(get_arg(2));
	c["value"] = get_data(3);
	network::send_data(data, 0);
}

void chat_command_handler::do_whisper()
{
	if (get_data(1).empty()) return command_failed_need_arg(1);
	if (get_data(2).empty()) return command_failed_need_arg(2);
	chat_handler_.send_whisper(get_arg(1), get_data(2));
	chat_handler_.add_whisper_sent(get_arg(1), get_data(2));
}

void chat_command_handler::do_chanmsg()
{
	if (get_data(1).empty()) return command_failed_need_arg(1);
	if (get_data(2).empty()) return command_failed_need_arg(2);
	chat_handler_.send_chat_room_message(get_arg(1), get_data(2));
	chat_handler_.add_chat_room_message_sent(get_arg(1), get_data(2));
}

void chat_command_handler::do_log()
{
	chat_handler_.change_logging(get_data());
}

void chat_command_handler::do_ignore()
{
	if (get_arg(1).empty()) {
		const std::map<std::string, std::string>& tmp = preferences::get_acquaintances_nice("ignore");
		print(_("ignores list"), tmp.empty() ? _("(empty)") : utils::join_map(tmp, ")\n", " (") + ")");
	} else {
		utils::string_map symbols;
		symbols["nick"] = get_arg(1);

		if (preferences::add_ignore(get_arg(1), get_data(2))) {
			print(_("ignores list"),  VGETTEXT("Added to ignore list: $nick", symbols));
			chat_handler_.user_relation_changed(get_arg(1));
		} else {
			command_failed(VGETTEXT("Invalid username: $nick", symbols));
		}
	}
}

void chat_command_handler::do_friend()
{
	if (get_arg(1).empty()) {
		const std::map<std::string, std::string>& tmp = preferences::get_acquaintances_nice("friend");
		print(_("friends list"), tmp.empty() ? _("(empty)") : utils::join_map(tmp, ")\n", " (") + ")");
	} else {
		utils::string_map symbols;
		symbols["nick"] = get_arg(1);

		if (preferences::add_friend(get_arg(1), get_data(2))) {
			print(_("friends list"),  VGETTEXT("Added to friends list: $nick", symbols));
			chat_handler_.user_relation_changed(get_arg(1));
		} else {
			command_failed(VGETTEXT("Invalid username: $nick", symbols));
		}
	}
}

void chat_command_handler::do_remove()
{
	for(int i = 1;!get_arg(i).empty();i++){
		preferences::remove_acquaintance(get_arg(i));
		chat_handler_.user_relation_changed(get_arg(i));
		utils::string_map symbols;
		symbols["nick"] = get_arg(i);
		print(_("friends and ignores list"), VGETTEXT("Removed from list: $nick", symbols));
	}
}

void chat_command_handler::do_display()
{
	const std::map<std::string, std::string>& friends = preferences::get_acquaintances_nice("friend");
	const std::map<std::string, std::string>& ignores = preferences::get_acquaintances_nice("ignore");

	if (!friends.empty()) {
		print(_("friends list"), utils::join_map(friends, ")\n", " (") + ")");
	}

	if (!ignores.empty()) {
		print(_("ignores list"), utils::join_map(ignores, ")\n", " (") + ")");
	}

	if (friends.empty() && ignores.empty()) {
		print(_("friends and ignores list"), _("There are no players on your friends or ignore list."));
	}
}

void chat_command_handler::do_version() {
	print(_("version"), game_config::revision);
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
	std::string msg;
	if (get_data(2).empty()) {
		msg = _("registering with password *** and no email address");
	} else {
		utils::string_map symbols;
		symbols["email"] = get_data(2);
		msg = VGETTEXT("registering with password *** and "
			"email address $email", symbols);
	}
	print(_("nick registration"), msg);

	network::send_data(data, 0);
}

void chat_command_handler::do_drop() {
	config data;
	config& nickserv = data.add_child("nickserv");

	nickserv.add_child("drop");

	print(_("nick registration"), _("dropping your username"));

	network::send_data(data, 0);
}

void chat_command_handler::do_set() {
	config data;
	config& nickserv = data.add_child("nickserv");

	if (get_data(1).empty()) return command_failed_need_arg(1);
	if (get_data(2).empty()) return command_failed_need_arg(2);

	config &set = nickserv.add_child("set");
	set["detail"] = get_arg(1);
	set["value"] = get_data(2);
	utils::string_map symbols;
	symbols["var"] = get_arg(1);
	symbols["value"] = get_arg(2);
	print(_("nick registration"), VGETTEXT("setting $var to $value", symbols));

	network::send_data(data, 0);
}

void chat_command_handler::do_info() {
	if (get_data(1).empty()) return command_failed_need_arg(1);

	config data;
	config& nickserv = data.add_child("nickserv");

	nickserv.add_child("info")["name"] = get_data(1);
	utils::string_map symbols;
	symbols["nick"] = get_arg(1);
	print(_("nick registration"), VGETTEXT("requesting information for user $nick", symbols));

	network::send_data(data, 0);
}

void chat_command_handler::do_details() {

	config data;
	config& nickserv = data.add_child("nickserv");
	nickserv.add_child("details");

	network::send_data(data, 0);
}

void menu_handler::send_chat_message(const std::string& message, bool allies_only)
{
	config cfg;
	cfg["id"] = preferences::login();
	cfg["message"] = message;
	const time_t time = ::time(NULL);
	std::stringstream ss;
	ss << time;
	cfg["time"] = ss.str();

	const int side = is_observer() ? 0 : gui_->viewing_side();
	if(!is_observer()) {
		cfg["side"] = side;
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
	add_chat_message(time, cfg["id"], side, message,
			private_message ? events::chat_handler::MESSAGE_PRIVATE : events::chat_handler::MESSAGE_PUBLIC);

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
				std::string label_text = label->text().str();
				if(std::search(label_text.begin(), label_text.end(),
						last_search_.begin(), last_search_.end(),
						chars_equal_insensitive) != label_text.end()) {
					found = true;
				}
			}
		}
		//Search unit name
		if (!gui_->fogged(loc)) {
			unit_map::const_iterator ui = units_.find(loc);
			if(ui != units_.end()) {
				const std::string name = ui->name();
				if(std::search(name.begin(), name.end(),
						last_search_.begin(), last_search_.end(),
						chars_equal_insensitive) != name.end()) {
					if (!teams_[gui_->viewing_team()].is_enemy(ui->side()) ||
					    !ui->invisible(ui->get_location())) {
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
		const std::string msg = vgettext("Could not find label or unit "
				"containing the string ‘$search’.", symbols);
		gui2::show_message(gui_->video(), "", msg, gui2::tmessage::auto_close);
	}
}

void menu_handler::do_command(const std::string& str)
{
	console_handler ch(*this);
	ch.dispatch(str);
}

std::vector<std::string> menu_handler::get_commands_list()
{
	console_handler ch(*this);
	// HACK: we need to call dispatch() at least once to get the
	// command list populated *at all*. Terrible design.
	// An empty command is silently ignored and has negligible
	// overhead, so we use that for this purpose here.
	ch.dispatch("");
	return ch.get_commands_list();
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
	} else if ((menu_handler_.teams_[side - 1].is_human() || menu_handler_.teams_[side - 1].is_idle()) && action != " off") {
		//this is our side, so give it to AI
		menu_handler_.teams_[side - 1].make_ai();
		menu_handler_.change_controller(lexical_cast<std::string>(side),"ai");
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

void console_handler::do_idle() {
	// :idle [<side> [on/off]]
	const std::string side_s = get_arg(1);
	const std::string action = get_arg(2);
	// default to the current side if empty
	const unsigned int side = side_s.empty() ?
		team_num_ : lexical_cast_default<unsigned int>(side_s);

	if (side < 1 || side > menu_handler_.teams_.size()) {
		utils::string_map symbols;
		symbols["side"] = side_s;
		command_failed(vgettext("Can't idle invalid side: '$side'.", symbols));
		return;
	} else if (menu_handler_.teams_[side - 1].is_network()) {
		utils::string_map symbols;
		symbols["side"] = lexical_cast<std::string>(side);
		command_failed(vgettext("Can't droid networked side: '$side'.", symbols));
		return;
	} else if (menu_handler_.teams_[side - 1].is_human() && action != " off") {
		//this is our side, so give it to idle
		menu_handler_.teams_[side - 1].make_idle();
		menu_handler_.change_controller(lexical_cast<std::string>(side),"idle");
		if(team_num_ == side) {
			//if it is our turn at the moment, we have to indicate to the
			//play_controller, that we are no longer in control
			throw end_turn_exception(side);
		}
	} else if (menu_handler_.teams_[side - 1].is_ai() && action != " off") {
		//this is our side, so give it to idle, without end turn exception. tell network it is human
		menu_handler_.teams_[side - 1].make_idle();
		menu_handler_.change_controller(lexical_cast<std::string>(side),"human");
	} else if (menu_handler_.teams_[side - 1].is_idle() && action != " on") {
		menu_handler_.teams_[side - 1].make_human();
		menu_handler_.change_controller(lexical_cast<std::string>(side),"human");
		if(team_num_ == side) {
			//if it is our turn at the moment, we have to indicate to the
			//play_controller, that idle should no longer be in control
			throw end_turn_exception(side);
		}
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
	menu_handler_.request_control_change(side_num,player);
	menu_handler_.textbox_info_.close(*(menu_handler_.gui_));
}
void console_handler::do_controller()
{
	const std::string side = get_arg(1);
	unsigned int side_num;
	try {
		side_num = lexical_cast<unsigned int>(side);
	} catch(bad_lexical_cast&) {
		utils::string_map symbols;
		symbols["side"] = side;
		command_failed(vgettext("Can't query control of invalid side: '$side'.", symbols));
		return;
	}
	if (side_num < 1 || side_num > menu_handler_.teams_.size()) {
		utils::string_map symbols;
		symbols["side"] = side;
		command_failed(vgettext("Can't query control of out-of-bounds side: '$side'.",	symbols));
		return;
	}
	print(get_cmd(), menu_handler_.teams_[side_num - 1].controller_string());
}

void console_handler::do_clear() {
	menu_handler_.gui_->clear_chat_messages();
}
void console_handler::do_sunset() {
	int delay = lexical_cast_default<int>(get_data());
	menu_handler_.gui_->sunset(delay);
	gui2::twindow::set_sunset(delay);
}
void console_handler::do_foreground() {
	menu_handler_.gui_->toggle_debug_foreground();
}

void console_handler::do_layers() {
	const mouse_handler& mousehandler = resources::controller->get_mouse_handler_base();
	const map_location &loc = mousehandler.get_last_hex();

	std::vector<std::string> layers;
	//NOTE: columns reflect WML keys, don't translate them
	std::string heading = std::string(1,HEADING_PREFIX) +
		"^#"      + COLUMN_SEPARATOR + // 0
		"Image"   + COLUMN_SEPARATOR + // 1
		"Name"    + COLUMN_SEPARATOR + // 2
		"Loc"     + COLUMN_SEPARATOR + // 3
		"Layer"   + COLUMN_SEPARATOR + // 4
		"Base.x"  + COLUMN_SEPARATOR + // 5
		"Base.y"  + COLUMN_SEPARATOR + // 6
		"Center"                       // 7

	;
	layers.push_back(heading);

	display& disp = *(menu_handler_.gui_);
	terrain_builder& builder = disp.get_builder();
	terrain_builder::tile* tile = builder.get_tile(loc);

	const std::string& tod_id = disp.get_time_of_day(loc).id;
	terrain_builder::tile::logs tile_logs;
	tile->rebuild_cache(tod_id, &tile_logs);

	int order = 1;
	BOOST_FOREACH(const terrain_builder::tile::log_details det, tile_logs) {
		const terrain_builder::tile::rule_image_rand& ri = *det.first;
		const terrain_builder::rule_image_variant& variant = *det.second;

		/** @todo also use random image variations (not just take 1st) */
		const image::locator& img = variant.images.front().get_first_frame();
		const std::string& name = img.get_filename();
		/** @todo deal with (rarely used) ~modifications */
		//const std::string& modif = img.get_modifications();
		const map_location& loc_cut = img.get_loc();

		std::ostringstream str;

		int tz = image::tile_size;
		SDL_Rect r = create_rect(0,0,tz,tz);

		surface surf = image::get_image(img.get_filename());

		// calculate which part of the image the terrain engine uses
		if(loc_cut.valid()) {
			// copied from image.cpp : load_image_sub_file()
			r = create_rect(
					((tz*3) / 4) * loc_cut.x
					, tz * loc_cut.y + (tz / 2) * (loc_cut.x % 2)
					, tz, tz);

			if(img.get_center_x() >= 0 && img.get_center_y()>= 0){
				r.x += surf->w/2 - img.get_center_x();
				r.y += surf->h/2 - img.get_center_y();
			}
		}

		str << (ri->is_background() ? "B ": "F ") << order
			<< COLUMN_SEPARATOR
			<< IMAGE_PREFIX << "terrain/foreground.png";

		// cut and mask the image
		// ~CROP and ~BLIT have limitations, we do some math to avoid them
		SDL_Rect r2 = intersect_rects(r, create_rect(0,0,surf->w,surf->h));
		if(r2.w > 0 && r2.h > 0) {
			str << "~BLIT("
					<< name << "~CROP("
						<< r2.x << "," << r2.y << ","
						<< r2.w << "," << r2.h
					<< ")"
					<< "," << r2.x-r.x << "," << r2.y-r.y
				<< ")"
				<< "~MASK(" << "terrain/alphamask.png" << ")";
		}

		str	<< COLUMN_SEPARATOR
			<< IMAGE_PREFIX  << name << "~SCALE(72,72)"
			<< IMG_TEXT_SEPARATOR << name
			<< COLUMN_SEPARATOR << img.get_loc()
			<< COLUMN_SEPARATOR << ri->layer
			<< COLUMN_SEPARATOR << ri->basex
			<< COLUMN_SEPARATOR << ri->basey
			<< COLUMN_SEPARATOR
			<< ri->center_x << ", " << ri->center_y;
		layers.push_back(str.str());
		++order;
	}

	std::vector<std::string> flags(tile->flags.begin(),tile->flags.end());
	std::ostringstream info;
	// NOTE using ", " also allows better word wrapping
	info << "Flags :" << utils::join(flags, ", ");
	{
		gui::dialog menu(*menu_handler_.gui_, _("Layers"), info.str(), gui::OK_CANCEL);
		menu.set_menu(layers);
		menu.show();
	}
}
void console_handler::do_fps() {
	preferences::set_show_fps(!preferences::show_fps());
}
void console_handler::do_benchmark() {
	menu_handler_.gui_->toggle_benchmark();
}
void console_handler::do_save() {
	savegame::ingame_savegame save(menu_handler_.gamestate_, *menu_handler_.gui_,
	                               resources::controller->to_config(),
	                               preferences::save_compression_format());
	save.save_game_automatic(menu_handler_.gui_->video(), true, get_data());
}
void console_handler::do_save_quit() {
	do_save();
	do_quit();
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

void console_handler::do_next_level()
{
	if (!get_data().empty())
		resources::gamedata->set_next_scenario(get_data());
	end_level_data &e = resources::controller->get_end_level_data();
	e.carryover_percentage = 100;
	e.carryover_add = false;
	e.gold_bonus = false;
	e.transient.carryover_report = false;
	e.prescenario_save = true;
	e.transient.linger_mode = false;
	throw end_level_exception(VICTORY);
}

void console_handler::do_choose_level() {
	std::vector<std::string> options;
	int next = 0, nb = 0;
	BOOST_FOREACH(const config &sc, menu_handler_.game_config_.child_range("scenario"))
	{
		const std::string &id = sc["id"];
		options.push_back(id);
		if (id == resources::gamedata->next_scenario())
			next = nb;
		++nb;
	}
	// find scenarios of multiplayer campaigns
	// (assumes that scenarios are ordered properly in the game_config)
	std::string& scenario = menu_handler_.gamestate_.mp_settings().mp_scenario;
	BOOST_FOREACH(const config &mp, menu_handler_.game_config_.child_range("multiplayer"))
	{
		if (mp["id"] == scenario)
		{
			const std::string &id = mp["id"];
			options.push_back(id);
			if (id == resources::gamedata->next_scenario())
				next = nb;
			++nb;
			scenario = mp["next_scenario"].str();
		}
	}
	std::sort(options.begin(), options.end());
	int choice = 0;
	{
		gui2::tsimple_item_selector dlg(_("Choose Scenario (Debug!)"), "", options);
		dlg.set_selected_index(next);
		dlg.show(menu_handler_.gui_->video());
		choice = dlg.selected_index();
	}

	if(choice == -1)
		return;

	if (size_t(choice) < options.size()) {
		resources::gamedata->set_next_scenario(options[choice]);
		end_level_data &e = resources::controller->get_end_level_data();
		e.carryover_percentage = 100;
		e.carryover_add = false;
		e.gold_bonus = false;
		e.transient.carryover_report = false;
		e.prescenario_save = true;
		e.transient.linger_mode = false;
		throw end_level_exception(VICTORY);
	}
}

void console_handler::do_turn()
{
	int turn = resources::tod_manager->turn() + 1;
	const std::string& data = get_data();
	if (!data.empty()) {
		turn = lexical_cast_default<int>(data, 1);
	}
	resources::tod_manager->set_turn(turn);

	menu_handler_.gui_->new_turn();
	menu_handler_.gui_->redraw_everything();
}

void console_handler::do_turn_limit()
{
	tod_manager& tod_man = *resources::tod_manager;
	int limit =
		get_data().empty() ? -1 : lexical_cast_default<int>(get_data(), 1);
	tod_man.set_number_of_turns(limit);
	menu_handler_.gui_->redraw_everything();
}

void console_handler::do_debug() {
	if (network::nconnections() == 0 || game_config::mp_debug) {
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
void console_handler::do_lua() {
	resources::lua_kernel->run(get_data().c_str());
	game_events::flush_messages();
}

void console_handler::do_unsafe_lua()
{
	if (gui2::show_message(resources::screen->video(), _("Unsafe Lua scripts."),
		_("You are about to open a security breach in Wesnoth. Are you sure you want to continue? If you have downloaded add-ons, do not click 'ok'! They would instantly take over your computer. You have been warned."),
		gui2::tmessage::ok_cancel_buttons) == gui2::twindow::OK)
	{
		print(get_cmd(), _("Unsafe mode enabled!"));
		resources::lua_kernel->load_package();
	}
}

void console_handler::do_custom() {
	preferences::set_custom_command(get_data());
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
		resources::gamedata->set_variable(name,value);
	} else {
		command_failed(_("Variable not found"));
	}
}
void console_handler::do_show_var() {
	gui2::show_transient_message((*menu_handler_.gui_).video(),"",resources::gamedata->get_variable(get_data()));
}


void console_handler::do_inspect() {
	vconfig cfg = vconfig::empty_vconfig();
	gui2::tgamestate_inspector inspect_dialog(cfg);
	inspect_dialog.show(resources::screen->video());
}

void console_handler::do_control_dialog()
{
	gui2::tmp_change_control mp_change_control(&menu_handler_);
	mp_change_control.show(resources::screen->video());
}

void console_handler::do_manage() {
	config cfg;
	gui2::tdata_manage manager;
	manager.show(resources::screen->video());
}

void console_handler::do_unit() {
	// prevent SIGSEGV due to attempt to set HP during a fight
	if (events::commands_disabled > 0)
		return;
	unit_map::iterator i = menu_handler_.current_unit();
	if (i == menu_handler_.units_.end()) return;
	const map_location loc = i->get_location();
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
	if (name == "alignment" && (value != "lawful" && value != "neutral" && value != "chaotic" && value != "liminal")) {
		utils::string_map symbols;
		symbols["alignment"] = get_arg(1);
		command_failed(VGETTEXT("Invalid alignment: '$alignment',"
			" needs to be one of lawful, neutral, chaotic, or liminal.", symbols));
		return;
	}
	if (name == "advances" ){
		if(synced_context::get_synced_state() == synced_context::SYNCED)
		{
			command_failed("unit advances=n doesn't work while another action is executed.");
			return;
		}
		int int_value = lexical_cast<int>(value);
		for (int levels=0; levels<int_value; levels++) {
			i->set_experience(i->max_experience());
			
			advance_unit_at(loc,true);
			i = menu_handler_.units_.find(loc);
			if (!i.valid()) {
				break;
			}
		}
	} else {
		config cfg;
		i->write(cfg);
		menu_handler_.units_.erase(loc);
		cfg[name] = value;
		unit new_u(cfg, true);
		menu_handler_.units_.add(loc, new_u);
	}
	if (name == "fail") { //testcase for bug #18488
		assert(i.valid());
	}
	menu_handler_.gui_->invalidate(loc);
	menu_handler_.gui_->invalidate_unit();
}
/*void console_handler::do_buff() {
	print(get_cmd(), _("Debug mode activated!"));
	const unit_map::iterator i = menu_handler_.current_unit();
	if(i != menu_handler_.units_.end()) {
		//i->second.add_trait(get_data());
		menu_handler_.gui_->invalidate(i->first);
		menu_handler_.gui_->invalidate_unit();
	} else {
		command_failed("No unit selected");
	}
}
void console_handler::do_unbuff() {
	const unit_map::iterator i = menu_handler_.current_unit();
	if(i != menu_handler_.units_.end()) {
		// FIXME: 'data_' is the trait.  Clear it.

		menu_handler_.gui_->invalidate(i->first);
		menu_handler_.gui_->invalidate_unit();
	} else {
		command_failed(_("No unit selected"));
	}
}*/
void console_handler::do_discover() {
	BOOST_FOREACH(const unit_type_data::unit_type_map::value_type &i, unit_types.types()) {
		preferences::encountered_units().insert(i.second.id());
	}
}
void console_handler::do_undiscover() {
	const int res = gui2::show_message((*menu_handler_.gui_).video(), "Undiscover", _("Do you wish to clear all of your discovered units from help?"), gui2::tmessage::yes_no_buttons);
	if(res != gui2::twindow::CANCEL) {
		preferences::encountered_units().clear();
	}
}
/**
 * Implements the (debug mode) console command that creates a unit.
 */
void console_handler::do_create() {
	const mouse_handler& mousehandler = resources::controller->get_mouse_handler_base();
	const map_location &loc = mousehandler.get_last_hex();
	if (menu_handler_.map_.on_board(loc)) {
		const unit_type *ut = unit_types.find(get_data());
		if (!ut) {
			command_failed(_("Invalid unit type"));
			return;
		}

		// Create the unit.
		create_and_place(*menu_handler_.gui_, menu_handler_.map_,
		                 menu_handler_.units_, loc, *ut);
	} else {
		command_failed(_("Invalid location"));
	}
}
void console_handler::do_fog() {
	menu_handler_.teams_[team_num_ - 1].set_fog( !menu_handler_.teams_[team_num_ - 1].uses_fog() );
	actions::recalculate_fog(team_num_);
	menu_handler_.gui_->recalculate_minimap();
	menu_handler_.gui_->redraw_everything();
}
void console_handler::do_shroud() {
	menu_handler_.teams_[team_num_ - 1].set_shroud( !menu_handler_.teams_[team_num_ - 1].uses_shroud() );
	actions::clear_shroud(team_num_);
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

void console_handler::do_toggle_whiteboard() {
	resources::whiteboard->set_active(!resources::whiteboard->is_active());
	if (resources::whiteboard->is_active()) {
		print(get_cmd(), _("Planning mode activated!"));
		resources::whiteboard->print_help_once();
	} else {
		print(get_cmd(), _("Planning mode deactivated!"));
	}
}

void console_handler::do_whiteboard_options()
{
	resources::whiteboard->options_dlg();
}

void menu_handler::do_ai_formula(const std::string& str,
	int side_num, mouse_handler& /*mousehandler*/)
{
	try {
		add_chat_message(time(NULL), _("ai"), 0, ai::manager::evaluate_command(side_num, str));
	} catch(end_turn_exception&) {
		resources::controller->force_end_turn();
	} catch(...) {
		//add_chat_message(time(NULL), _("ai"), 0, "ERROR IN FORMULA");
	}
}

void menu_handler::user_command()
{
	textbox_info_.show(gui::TEXTBOX_COMMAND,sgettext("prompt^Command:"), "", false, *gui_);
}

void menu_handler::request_control_change ( int side_num, const std::string& player )
{
	std::string side = str_cast(side_num);
	//if this is our side we are always allowed to change the controller
	if (teams_[side_num - 1].is_human()) {
		if (player == preferences::login())
			return;
		change_side_controller(side,player);
	} else if (teams_[side_num - 1].is_idle()) { //if this is our side and it is idle, make human and throw an end turn exception
		LOG_NG << " *** Got an idle side with requested control change " << std::endl;
		teams_[side_num - 1].make_human();
		change_controller(lexical_cast<std::string>(side_num),"human");

		if (player == preferences::login()) {
			LOG_NG << " *** It's us, throwing end turn exception " << std::endl;
		} else {
			LOG_NG << " *** It's not us, changing sides now as usual, then throwing end_turn " << std::endl;
			change_side_controller(side,player);		 
		}
		throw end_turn_exception(side_num);
	} else {
		//it is not our side, the server will decide if we can change the
		//controller (that is if we are host of the game)
		change_side_controller(side,player);
	}
}

void menu_handler::custom_command()
{
	std::vector<std::string> commands = utils::split(preferences::custom_command(), ';');
	std::vector<std::string>::iterator c = commands.begin();
	for (; c != commands.end() ; ++c) {
		do_command(*c);
	}
}

void menu_handler::ai_formula()
{
			if (network::nconnections() == 0) {
				textbox_info_.show(gui::TEXTBOX_AI,sgettext("prompt^Command:"), "", false, *gui_);
	}
}

void menu_handler::clear_messages()
{
	gui_->clear_chat_messages();	// also clear debug-messages and WML-error-messages
}

void menu_handler::change_controller(const std::string& side, const std::string& controller)
{
	config cfg;
	config& change = cfg.add_child("change_controller");
	change["side"] = side;
	change["controller"] = controller;
	network::send_data(cfg, 0);
}

void menu_handler::change_side_controller(const std::string& side, const std::string& player)
{
	config cfg;
	config& change = cfg.add_child("change_controller");
	change["side"] = side;
	change["player"] = player;

	network::send_data(cfg, 0);
}
} // end namespace events
