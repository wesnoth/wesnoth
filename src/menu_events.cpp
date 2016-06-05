/*
   Copyright (C) 2006 - 2016 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
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
#include "ai/manager.hpp"
#include "chat_command_handler.hpp"
#include "config_assign.hpp"
#include "dialogs.hpp"
#include "display_chat_manager.hpp"
#include "filechooser.hpp"
#include "formatter.hpp"
#include "formula/string_utils.hpp"
#include "game_board.hpp"
#include "game_config_manager.hpp"
#include "game_end_exceptions.hpp"
#include "game_events/manager.hpp"
#include "game_events/pump.hpp"
#include "game_preferences.hpp"
#include "game_state.hpp"
#include "gettext.hpp"
#include "gui/dialogs/chat_log.hpp"
#include "gui/dialogs/edit_label.hpp"
#include "gui/dialogs/label_settings.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/dialogs/gamestate_inspector.hpp"
#include "gui/dialogs/multiplayer/mp_change_control.hpp"
#include "gui/dialogs/simple_item_selector.hpp"
#include "gui/dialogs/edit_text.hpp"
#include "gui/dialogs/unit_create.hpp"
#include "gui/dialogs/unit_recruit.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "help/help.hpp"
#include "log.hpp"
#include "map/map.hpp"
#include "map/label.hpp"
#include "map_command_handler.hpp"
#include "marked-up_text.hpp"
#include "menu_events.hpp"
#include "mouse_events.hpp"
#include "play_controller.hpp"
#include "playsingle_controller.hpp"
#include "preferences_display.hpp"
#include "replay.hpp"
#include "replay_helper.hpp"
#include "resources.hpp"
#include "savegame.hpp"
#include "save_index.hpp"
#include "scripting/game_lua_kernel.hpp"
#include "scripting/plugins/manager.hpp"
#include "sound.hpp"
#include "statistics_dialog.hpp"
#include "synced_context.hpp"
#include "terrain/builder.hpp"
#include "units/unit.hpp"
#include "units/udisplay.hpp"
#include "wml_separators.hpp"
#include "whiteboard/manager.hpp"
#include "widgets/combo.hpp"

#include <boost/range/algorithm/find_if.hpp>

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)

namespace events{

menu_handler::menu_handler(game_display* gui, play_controller & pc,	const config& game_config) :
	gui_(gui),
	pc_(pc),
	game_config_(game_config),
	textbox_info_(),
	last_search_(),
	last_search_hit_()
{
}

menu_handler::~menu_handler()
{
}

game_state & menu_handler::gamestate() const { return pc_.gamestate(); }
game_data & menu_handler::gamedata() { return gamestate().gamedata_; }
game_board & menu_handler::board() const { return gamestate().board_; }
unit_map& menu_handler::units() { return gamestate().board_.units_; }
std::vector<team>& menu_handler::teams() const { return gamestate().board_.teams_; }
const gamemap& menu_handler::map() { return gamestate().board_.map(); }

gui::floating_textbox& menu_handler::get_textbox(){
	return textbox_info_;
}

std::string menu_handler::get_title_suffix(int side_num)
{
	int controlled_recruiters = 0;
	for(size_t i = 0; i < teams().size(); ++i) {
		if(teams()[i].is_local_human() && !teams()[i].recruits().empty()
		&& units().find_leader(i + 1) != units().end()) {
		++controlled_recruiters;
		}
	}
	std::stringstream msg;
	if(controlled_recruiters >= 2) {
		unit_map::const_iterator leader = units().find_leader(side_num);
		if (leader != units().end() && !leader->name().empty()) {
			msg << " (" << leader->name(); msg << ")";
		}
	}
	return msg.str();
}

void menu_handler::objectives(int side_num)
{
	if (!gamestate().lua_kernel_) {
		return ;
	}

	config cfg;
	cfg["side"] = std::to_string(side_num);
	gamestate().lua_kernel_->run_wml_action("show_objectives", vconfig(cfg),
		game_events::queued_event("_from_interface", map_location(),
			map_location(), config()));
	team &current_team = teams()[side_num - 1];
	dialogs::show_objectives(pc_.get_scenario_name(), current_team.objectives());
	current_team.reset_objectives_changed();
}

void menu_handler::show_statistics(int side_num)
{
	team &current_team = teams()[side_num - 1];
	// Current Player name
	const std::string &player = current_team.side_name();
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
		dialog(disp.video(), title, "", gui::NULL_DIALOG),
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

	const team& viewing_team = teams()[gui_->viewing_team()];

	unsigned total_villages = 0;
	// a variable to check if there are any teams to show in the table
	bool status_table_empty = true;

	//if the player is under shroud or fog, they don't get
	//to see details about the other sides, only their own
	//side, allied sides and a ??? is shown to demonstrate
	//lack of information about the other sides But he see
	//all names with in colors
	for(size_t n = 0; n != teams().size(); ++n) {
		if(teams()[n].hidden()) {
			continue;
		}
		status_table_empty=false;

		const bool known = viewing_team.knows_about_team(n);
		const bool enemy = viewing_team.is_enemy(n+1);

		std::stringstream str;

		const team_data data = board().calculate_team_data(teams()[n],n+1);

		unit_map::const_iterator leader = units().find_leader(n + 1);
		std::string leader_name;
		//output the number of the side first, and this will
		//cause it to be displayed in the correct color
		if(leader != units().end()) {
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
				str << "~RC(magenta>" << teams()[n].color() << ")";
#endif
				leader_bools.push_back(false);
				leader_name = "Unknown";
			}

			if (pc_.get_classification().campaign_type == game_classification::CAMPAIGN_TYPE::MULTIPLAYER)
				leader_name = teams()[n].side_name();

		} else {
			leader_bools.push_back(false);
		}
		str << COLUMN_SEPARATOR	<< team::get_side_highlight(n)
			<< leader_name << COLUMN_SEPARATOR
			<< (data.teamname.empty() ? teams()[n].team_name() : data.teamname)
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
			str << "/" << map().villages().size();
		}
		str << COLUMN_SEPARATOR
			<< data.units << COLUMN_SEPARATOR << data.upkeep << COLUMN_SEPARATOR
			<< (data.net_income < 0 ? font::BAD_TEXT : font::NULL_MARKUP) << utils::signed_value(data.net_income);
		total_villages += data.villages;
		items.push_back(str.str());
	}
	if (total_villages > map().villages().size()) {
		ERR_NG << "Logic error: map has " << map().villages().size() << " villages but status table shows " << total_villages << " owned in total" << std::endl;
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
		gui_->scroll_to_leader(selected+1);
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

	const team& viewing_team = teams()[gui_->viewing_team()];
	bool settings_table_empty = true;
	bool fogged;

	for(size_t n = 0; n != teams().size(); ++n) {
		if(teams()[n].hidden()) {
			continue;
		}
		settings_table_empty = false;

		std::stringstream str;
		unit_map::const_iterator leader = units().find_leader(n + 1);

		if(leader != units().end()) {
			// Add leader image. If it's fogged
			// show only a random leader image.
			fogged=viewing_team.fogged(leader->get_location());
			if (!fogged || viewing_team.knows_about_team(n)) {
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
			<< teams()[n].side_name() << COLUMN_SEPARATOR
			<< n + 1 << COLUMN_SEPARATOR
			<< teams()[n].start_gold() << COLUMN_SEPARATOR
			<< teams()[n].base_income() << COLUMN_SEPARATOR
			<< teams()[n].village_gold() << COLUMN_SEPARATOR
			<< teams()[n].village_support() << COLUMN_SEPARATOR
			<< (teams()[n].uses_fog()    ? _("yes") : _("no")) << COLUMN_SEPARATOR
			<< (teams()[n].uses_shroud() ? _("yes") : _("no")) << COLUMN_SEPARATOR;

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
			gui_->scroll_to_leader(selected+1);
		else if (result == gui::DIALOG_BACK)
			status_table(selected);
}

void menu_handler::save_map()
{
	std::string input_name = filesystem::get_dir(filesystem::get_dir(filesystem::get_user_data_dir() + "/editor") + "/maps/");
	int res = 0;
	int overwrite = 1;
	do {
		res = dialogs::show_file_chooser_dialog_save(gui_->video(), input_name, _("Save the Map As"), ".map");
		if (res == 0) {

			if (filesystem::file_exists(input_name)) {
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
			filesystem::write_file(input_name, map().write());
			gui2::show_transient_message(gui_->video(), "", _("Map saved."));
		} catch (filesystem::io_exception& e) {
			utils::string_map symbols;
			symbols["msg"] = e.what();
			const std::string msg = vgettext("Could not save the map: $msg",symbols);
			gui2::show_transient_message(gui_->video(), "", msg);
		}
	}
}

void menu_handler::preferences()
{
	preferences::show_preferences_dialog(gui_->video(), game_config_);
	// Needed after changing fullscreen/windowed mode or display resolution
	gui_->redraw_everything();
}

void menu_handler::show_chat_log()
{
	config c;
	c["name"] = "prototype of chat log";
	gui2::tchat_log chat_log_dialog(vconfig(c), resources::recorder);
	chat_log_dialog.show(gui_->video());
	//std::string text = resources::recorder->build_chat_log();
	//gui::show_dialog(*gui_,nullptr,_("Chat Log"),"",gui::CLOSE_ONLY,nullptr,nullptr,"",&text);
}

void menu_handler::show_help()
{
	help::show_help(gui_->video());
}

void menu_handler::speak()
{
	textbox_info_.show(gui::TEXTBOX_MESSAGE,_("Message:"),
		has_friends() ? board().is_observer() ? _("Send to observers only") : _("Send to allies only")
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
	if(board().is_observer()) {
		return !gui_->observers().empty();
	}

	for(size_t n = 0; n != teams().size(); ++n) {
		if(n != gui_->viewing_team() && teams()[gui_->viewing_team()].team_name() == teams()[n].team_name() && teams()[n].is_network()) {
			return true;
		}
	}

	return false;
}

void menu_handler::recruit(int side_num, const map_location &last_hex)
{
	std::vector<const unit_type*> sample_units;

	gui_->draw(); //clear the old menu

	std::set<std::string> recruits = actions::get_recruits(side_num, last_hex);

	for(std::set<std::string>::const_iterator it = recruits.begin(); it != recruits.end(); ++it) {
		const unit_type* type = unit_types.find(*it);
		if (!type) {
			ERR_NG << "could not find unit '" << *it << "'" << std::endl;
			return;
		}

		sample_units.push_back(type);
	}

	if(sample_units.empty()) {
		gui2::show_transient_message(gui_->video(), "", _("You have no units available to recruit."));
		return;
	}

	gui2::tunit_recruit dlg(sample_units, teams()[side_num - 1]);

	dlg.show(gui_->video());

	if(dlg.get_retval() == gui2::twindow::OK) {
		do_recruit(sample_units[dlg.get_selected_index()]->id(), side_num, last_hex);
	}
}


void menu_handler::repeat_recruit(int side_num, const map_location &last_hex)
{
	const std::string & last_recruit = teams()[side_num - 1].last_recruit();
	if ( last_recruit.empty() == false )
		do_recruit(last_recruit, side_num, last_hex);
}

bool menu_handler::do_recruit(const std::string &name, int side_num,
	const map_location &last_hex)
{
	team &current_team = teams()[side_num - 1];

	//search for the unit to be recruited in recruits
	if ( !util::contains(actions::get_recruits(side_num, last_hex), name) )
		return false;

	const unit_type *u_type = unit_types.find(name);
	assert(u_type);

	if (u_type->cost() > current_team.gold() - (pc_.get_whiteboard() ? pc_.get_whiteboard()->get_spent_gold_for(side_num) : 0)) {
		gui2::show_transient_message(gui_->video(), "",
			_("You don’t have enough gold to recruit that unit"));
		return false;
	}

	current_team.last_recruit(name);
	const events::command_disabler disable_commands;

	map_location loc = last_hex;
	map_location recruited_from = map_location::null_location();
	std::string msg;
	{ wb::future_map_if_active future; /* start planned unit map scope if in planning mode */
		msg = actions::find_recruit_location(side_num, loc, recruited_from, name);
	} // end planned unit map scope
	if (!msg.empty()) {
		gui2::show_transient_message(gui_->video(), "", msg);
		return false;
	}

	if (!pc_.get_whiteboard() || !pc_.get_whiteboard()->save_recruit(name, side_num, loc)) {
		//MP_COUNTDOWN grant time bonus for recruiting
		current_team.set_action_bonus_count(1 + current_team.action_bonus_count());

		// Do the recruiting.

		synced_context::run_and_throw("recruit", replay_helper::get_recruit(u_type->id(), loc, recruited_from));
		return true;
	}
	return false;
}

void menu_handler::recall(int side_num, const map_location &last_hex)
{
	if (pc_.get_disallow_recall()) {
		gui2::show_transient_message(gui_->video(),"",_("You are separated from your soldiers and may not recall them"));
		return;
	}

	team &current_team = teams()[side_num - 1];

	boost::shared_ptr<std::vector<unit_const_ptr > > recall_list_team = boost::make_shared<std::vector<unit_const_ptr> >();
	{ wb::future_map future; // ensures recall list has planned recalls removed
		*recall_list_team = actions::get_recalls(side_num, last_hex);
	}

	gui_->draw(); //clear the old menu


	DBG_WB <<"menu_handler::recall: Contents of wb-modified recall list:\n";
	for(const unit_const_ptr & unit : *recall_list_team)
	{
		DBG_WB << unit->name() << " [" << unit->id() <<"]\n";
	}

	if(current_team.recall_list().empty()) {
		gui2::show_transient_message(gui_->video(), "",
			_("There are no troops available to recall\n(You must have"
			" veteran survivors from a previous scenario)"));
		return;
	}
	if(recall_list_team->empty()) {
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
	else if(recall_list_team->at(res)->recall_cost() > -1) {
		unit_cost = recall_list_team->at(res)->recall_cost();
	}

	int wb_gold = pc_.get_whiteboard() ? pc_.get_whiteboard()->get_spent_gold_for(side_num) : 0;
	if (current_team.gold() - wb_gold < unit_cost) {
		utils::string_map i18n_symbols;
		i18n_symbols["cost"] = std::to_string(unit_cost);
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
	map_location recall_from = map_location::null_location();
	std::string err;
	{ wb::future_map_if_active future; // future unit map removes invisible units from map, don't do this outside of planning mode
		err = actions::find_recall_location(side_num, recall_location, recall_from, *(recall_list_team->at(res)));
	} // end planned unit map scope
	if(!err.empty()) {
		gui2::show_transient_message(gui_->video(), "", err);
		return;
	}

	if (!pc_.get_whiteboard() || !pc_.get_whiteboard()->save_recall(*recall_list_team->at(res), side_num, recall_location)) {
		bool success = synced_context::run_and_throw("recall",
			replay_helper::get_recall(recall_list_team->at(res)->id(), recall_location, recall_from),
			true,
			true,
			synced_context::ignore_error_function);

		if(!success)
		{
			ERR_NG << "menu_handler::recall(): Unit does not exist in the recall list." << std::endl;
		}
	}
}


// Highlights squares that an enemy could move to on their turn, showing how many can reach each square.
void menu_handler::show_enemy_moves(bool ignore_units, int side_num)
{
	wb::future_map future; // use unit positions as if all planned actions were executed

	gui_->unhighlight_reach();

	// Compute enemy movement positions
	for(unit_map::iterator u = units().begin(); u != units().end(); ++u) {
		bool invisible = u->invisible(u->get_location());

		if (teams()[side_num - 1].is_enemy(u->side()) &&
		    !gui_->fogged(u->get_location()) && !u->incapacitated() && !invisible)
		{
			const unit_movement_resetter move_reset(*u);
			const pathfind::paths& path = pathfind::paths(*u, false, true,
				teams()[gui_->viewing_team()], 0, false, ignore_units);

			gui_->highlight_another_reach(path);
		}
	}
}

void menu_handler::toggle_shroud_updates(int side_num)
{
	team &current_team = teams()[side_num - 1];
	bool auto_shroud = current_team.auto_shroud_updates();
	// If we're turning automatic shroud updates on, then commit all moves
	if (!auto_shroud) update_shroud_now(side_num);

	// Toggle the setting and record this.
	synced_context::run_and_throw("auto_shroud", replay_helper::get_auto_shroud(!auto_shroud));
}

void menu_handler::update_shroud_now(int /* side_num */)
{
	synced_context::run_and_throw("update_shroud", replay_helper::get_update_shroud());
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
	bool partmoved_units(int side_num, const unit_map & units, const game_board & board, const boost::shared_ptr<wb::manager> & whiteb)
	{
		for ( unit_map::const_iterator un = units.begin(); un != units.end(); ++un ) {
			if ( un->side() == side_num ) {
				// @todo whiteboard should take into consideration units that have
				// a planned move but can still plan more movement in the same turn
				if ( board.unit_can_move(*un) && !un->user_end_turn()
						&& (!whiteb || !whiteb->unit_has_actions(&*un)) )
					return true;
			}
		}
		return false;
	}
	/**
	 * Returns true if @a side_num has at least one unit that (can but) has not
	 * moved.
	 */
	bool unmoved_units(int side_num, const unit_map & units, const game_board & board, const boost::shared_ptr<wb::manager> & whiteb)
	{
		for ( unit_map::const_iterator un = units.begin(); un != units.end(); ++un ) {
			if ( un->side() == side_num ) {
				if ( board.unit_can_move(*un)  &&  !un->has_moved()  && !un->user_end_turn()
						&& (!whiteb || !whiteb->unit_has_actions(&*un)) )
					return true;
			}
		}
		return false;
	}
}

bool menu_handler::end_turn(int side_num)
{
	if(!gamedata().allow_end_turn()) {
		gui2::show_transient_message((*gui_).video(), "", _("You cannot end your turn yet!"));
		return false;
	}

	size_t team_num = static_cast<size_t>(side_num - 1);
	if ( team_num < teams().size()  &&  teams()[team_num].no_turn_confirmation() ) {
		// Skip the confirmations that follow.
	}
	// Ask for confirmation if the player hasn't made any moves.
	else if ( preferences::confirm_no_moves()  &&
	          !pc_.get_undo_stack().player_acted()  &&
	          (!pc_.get_whiteboard() || !pc_.get_whiteboard()->current_side_has_actions())  &&
	          units_alive(side_num, units()) )
	{
		const int res = gui2::show_message((*gui_).video(), "", _("You have not started your turn yet. Do you really want to end your turn?"), gui2::tmessage::yes_no_buttons);
		if(res == gui2::twindow::CANCEL) {
			return false;
		}
	}
	// Ask for confirmation if units still have some movement left.
	else if ( preferences::yellow_confirm() && partmoved_units(side_num, units(), board(), pc_.get_whiteboard()) ) {
		const int res = gui2::show_message((*gui_).video(), "", _("Some units have movement left. Do you really want to end your turn?"), gui2::tmessage::yes_no_buttons);
		if(res == gui2::twindow::CANCEL) {
			return false;
		}
	}
	// Ask for confirmation if units still have all movement left.
	else if ( preferences::green_confirm() && unmoved_units(side_num, units(), board(), pc_.get_whiteboard()) ) {
		const int res = gui2::show_message((*gui_).video(), "", _("Some units have not moved. Do you really want to end your turn?"), gui2::tmessage::yes_no_buttons);
		if(res == gui2::twindow::CANCEL) {
			return false;
		}
	}

	// Auto-execute remaining whiteboard planned actions
	// Only finish turn if they all execute successfully, i.e. no ambush, etc.
	if (pc_.get_whiteboard() && !pc_.get_whiteboard()->allow_end_turn()) {
		return false;
	}

	return true;
}

void menu_handler::goto_leader(int side_num)
{
	unit_map::const_iterator i = units().find_leader(side_num);
	if(i != units().end()) {
		gui_->scroll_to_tile(i->get_location(), game_display::WARP);
	}
}

void menu_handler::unit_description()
{
	const unit_map::const_iterator un = current_unit();
	if(un != units().end()) {
		help::show_unit_description(pc_.get_display().video(), *un);
	}
}

void menu_handler::terrain_description(mouse_handler& mousehandler)
{
	const map_location& loc = mousehandler.get_last_hex();
	if (map().on_board(loc) == false || gui_->shrouded(loc)) {
		return;
	}

	const terrain_type& type = map().get_terrain_info(loc);
	//const terrain_type& info = board().map().get_terrain_info(terrain);
	help::show_terrain_description(pc_.get_display().video(), type);
}

void menu_handler::rename_unit()
{
	const unit_map::iterator un = current_unit();
	if (un == units().end() || gui_->viewing_side() != un->side())
		return;
	if (un->unrenamable())
		return;

	std::string name = un->name();
	const std::string title(N_("Rename Unit"));
	const std::string label(N_("Name:"));

	if(gui2::tedit_text::execute(title, label, name, gui_->video())) {
		resources::recorder->add_rename(name, un->get_location());
		un->rename(name);
		gui_->invalidate_unit();
	}
}

unit_map::iterator menu_handler::current_unit()
{
	const mouse_handler& mousehandler = pc_.get_mouse_handler_base();

	unit_map::iterator res = board().find_visible_unit(mousehandler.get_last_hex(),
		teams()[gui_->viewing_team()]);
	if(res != units().end()) {
		return res;
	} else {
		return board().find_visible_unit(mousehandler.get_selected_hex(),
			teams()[gui_->viewing_team()]);
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
	 *          returned type is nullptr.
	 */
	type_and_gender choose_unit(game_display& gui)
	{
		//
		// The unit creation dialog makes sure unit types
		// are properly cached.
		//
		gui2::tunit_create create_dlg;
		create_dlg.show(gui.video());

		if(create_dlg.no_choice()) {
			return type_and_gender(nullptr, unit_race::NUM_GENDERS);
		}

		const std::string& ut_id = create_dlg.choice();
		const unit_type *utp = unit_types.find(ut_id);
		if (!utp) {
			ERR_NG << "Create unit dialog returned nonexistent or unusable unit_type id '" << ut_id << "'." << std::endl;
			return type_and_gender(static_cast<const unit_type *>(nullptr), unit_race::NUM_GENDERS);
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
	 * Creates a unit and places it on the board.
	 * (Intended for use with any units created via debug mode.)
	 */
	void create_and_place(game_display& , const gamemap & , unit_map & ,
	                      const map_location & loc, const unit_type & u_type,
	                      unit_race::GENDER gender = unit_race::NUM_GENDERS)
	{
		synced_context::run_and_throw("debug_create_unit", config_of("x", loc.x + 1)("y", loc.y + 1)("type", u_type.id())("gender", gender_string(gender)));
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
	assert(gui_ != nullptr);

	// Let the user select the kind of unit to create.
	type_and_gender selection = choose_unit(*gui_);
	if ( selection.first != nullptr )
		// Make it so.
		create_and_place(*gui_, map(), units(), destination,
		                 *selection.first, selection.second);
}

void menu_handler::change_side(mouse_handler& mousehandler)
{
	const map_location& loc = mousehandler.get_last_hex();
	const unit_map::iterator i = units().find(loc);
	if(i == units().end()) {
		if(!map().is_village(loc))
			return;

		// village_owner returns -1 for free village, so team 0 will get it
		int team = board().village_owner(loc) + 1;
		// team is 0-based so team=team::nteams() is not a team
		// but this will make get_village free it
		if(team > static_cast<int> (teams().size())) {
			team = 0;
		}
		actions::get_village(loc, team + 1);
	} else {
		int side = i->side();
		++side;
		if(side > static_cast<int> (teams().size())) {
			side = 1;
		}
		i->set_side(side);

		if(map().is_village(loc)) {
			actions::get_village(loc, side);
		}
	}
}

void menu_handler::kill_unit(mouse_handler& mousehandler)
{
	const map_location& loc = mousehandler.get_last_hex();
	const unit_map::iterator i = units().find(loc);
	if(i != units().end()) {
		const int dying_side = i->side();
		pc_.pump().fire("last breath", loc, loc);
		if (i.valid()) {
			unit_display::unit_die(loc, *i);
		}
		gui_->redraw_minimap();
		pc_.pump().fire("die", loc, loc);
		if (i.valid()) {
			units().erase(i);
		}
		actions::recalculate_fog(dying_side);
		pc_.check_victory();
	}
}

void menu_handler::label_terrain(mouse_handler& mousehandler, bool team_only)
{
	const map_location& loc = mousehandler.get_last_hex();
	if (map().on_board(loc) == false) {
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
		const terrain_label* res = gui_->labels().set_label(loc, label, gui_->viewing_team(), team_name, color);
		if (res)
			resources::recorder->add_label(res);
	}
}

void menu_handler::clear_labels()
{
	if (gui_->team_valid()
	   && !board().is_observer())
	{
		gui_->labels().clear(gui_->current_team_name(), false);
		resources::recorder->clear_labels(gui_->current_team_name(), false);
	}
}

void menu_handler::label_settings() {
	if(gui2::tlabel_settings::execute(board(), gui_->video()))
		gui_->labels().recalculate_labels();
}

void menu_handler::continue_move(mouse_handler &mousehandler, int side_num)
{
	unit_map::iterator i = current_unit();
	if (i == units().end() || !i->move_interrupted()) {
		i = units().find(mousehandler.get_selected_hex());
		if (i == units().end() || !i->move_interrupted()) return;
	}
	move_unit_to_loc(i, i->get_interrupted_move(), true,
		side_num, mousehandler);
}

void menu_handler::move_unit_to_loc(const unit_map::iterator &ui,
	const map_location& target, bool continue_move, int side_num,
	mouse_handler &mousehandler)
{
	assert(ui != units().end());

	pathfind::marked_route route = mousehandler.get_route(&*ui, target, teams()[side_num - 1]);

	if(route.steps.empty())
		return;

	assert(route.steps.front() == ui->get_location());

	gui_->set_route(&route);
	gui_->unhighlight_reach();
	{
		LOG_NG << "move_unit_to_loc " << route.steps.front() << " to " << route.steps.back() << "\n";
		actions::move_unit_and_record(route.steps, &pc_.get_undo_stack(), continue_move);
	}
	gui_->set_route(nullptr);
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
		for(unit_map::iterator ui = units().begin(); ui != units().end(); ++ui) {
			if (ui->side() != side  || ui->movement_left() == 0)
				continue;

			const map_location &current_loc = ui->get_location();
			const map_location &goto_loc = ui->get_goto();

			if(goto_loc == current_loc){
				ui->set_goto(map_location());
				continue;
			}

			if(!map().on_board(goto_loc))
				continue;

			// avoid pathfinding calls for finished units
			if(fully_moved.count(current_loc))
				continue;

			pathfind::marked_route route = mousehandler.get_route(&*ui, goto_loc, teams()[side - 1]);

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
			if(units().count(next_stop)) {
				blocked_unit = true;
				if (wait_blocker_move)
					continue;
			}

			gui_->set_route(&route);

			{
				LOG_NG << "execute goto from " << route.steps.front() << " to " << route.steps.back() << "\n";
				int moves = actions::move_unit_and_record(route.steps, &pc_.get_undo_stack());
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
	gui_->set_route(nullptr);
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
	const unit_map::iterator un = units().find(mousehandler.get_selected_hex());
	if (un != units().end() && un->side() == side_num && un->movement_left() >= 0)
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
	const unit_map::iterator un = units().find(mousehandler.get_selected_hex());
	if (un != units().end() && un->side() == side_num && un->movement_left() >= 0)
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
	chat_handler::do_speak(textbox_info_.box()->text(),textbox_info_.check() != nullptr ? textbox_info_.check()->checked() : false);
}


void menu_handler::add_chat_message(const time_t& time,
		const std::string& speaker, int side, const std::string& message,
		events::chat_handler::MESSAGE_TYPE type)
{
	gui_->get_chat_manager().add_chat_message(time, speaker, side, message, type, false);

	plugins_manager::get()->notify_event("chat", config_of("sender", preferences::login())
								("message", message)
								("whisper", type == events::chat_handler::MESSAGE_PRIVATE));
}

//command handler for user :commands. Also understands all chat commands
//via inheritance. This complicates some things a bit.
class console_handler : public map_command_handler<console_handler>, private chat_command_handler
{
	public:
		//convenience typedef
		typedef map_command_handler<console_handler> chmap;
		console_handler(menu_handler& menu_handler)
		: chmap(), chat_command_handler(menu_handler, true), menu_handler_(menu_handler), team_num_(menu_handler.pc_.current_side())
		{}
		using chmap::dispatch; //disambiguate
		using chmap::get_commands_list;
		using chmap::command_failed;
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
				 + (c.has_flag('A') ? space + _("(admin only)") : "")
				 + (c.has_flag('S') ? space + _("(not during other events)") : "");
		}
		using map::is_enabled;
		bool is_enabled(const chmap::command& c) const
		{
			return !((c.has_flag('D') && !game_config::debug)
				  || (c.has_flag('N') && menu_handler_.pc_.is_networked_mp())
				  || (c.has_flag('A') && !preferences::is_authenticated())
				  || (c.has_flag('S') && (synced_context::get_synced_state() != synced_context::UNSYNCED)));
		}
		void print(const std::string& title, const std::string& message)
		{
			menu_handler_.add_chat_message(time(nullptr), title, 0, message);
		}
		void init_map()
		{
			chat_command_handler::init_map();//grab chat_ /command handlers
			chmap::get_command("log")->flags = ""; //clear network-only flag from log
			chmap::get_command("version")->flags = ""; //clear network-only flag
			chmap::get_command("ignore")->flags = ""; //clear network-only flag
			chmap::get_command("friend")->flags = ""; //clear network-only flag
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
				_("Advance to the next scenario, or scenario identified by 'id'"), _("<id>"), "DS");
			register_alias("next_level", "n");
			register_command("choose_level", &console_handler::do_choose_level,
				_("Choose next scenario"), "", "DS");
			register_alias("choose_level", "cl");
			register_command("turn", &console_handler::do_turn,
				_("Change turn number (and time of day), or increase by one if no number is specified."), _("[turn]"), "DS");
			register_command("turn_limit", &console_handler::do_turn_limit,
				_("Change turn limit, or turn the turn limit off if no number is specified or it’s −1."), _("[limit]"), "DS");
			register_command("debug", &console_handler::do_debug,
				_("Turn debug mode on."));
			register_command("nodebug", &console_handler::do_nodebug,
				_("Turn debug mode off."), "", "D");
			register_command("lua", &console_handler::do_lua,
				_("Execute a Lua statement."), _("<command>[;<command>...]"), "DS");
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
			register_command("alias", &console_handler::do_set_alias,
				_("Set or show alias to a command"), _("<name>[=<command>]"));
			register_command("set_var", &console_handler::do_set_var,
				_("Set a scenario variable."), _("<var>=<value>"), "DS");
			register_command("show_var", &console_handler::do_show_var,
				_("Show a scenario variable."), _("<var>"), "D");
			register_command("unit", &console_handler::do_unit,
				_("Modify a unit variable. (Only top level keys are supported.)"), "", "DS");

			// register_command("buff", &console_handler::do_buff,
			//    _("Add a trait to a unit."), "", "D");
			// register_command("unbuff", &console_handler::do_unbuff,
			//    _("Remove a trait from a unit. (Does not work yet.)"), "", "D");
			register_command("discover", &console_handler::do_discover,
				_("Discover all units in help."), "");
			register_command("undiscover", &console_handler::do_undiscover,
				  _("'Undiscover' all units in help."), "");
			register_command("create", &console_handler::do_create,
				_("Create a unit."), "", "DS");
			register_command("fog", &console_handler::do_fog,
				_("Toggle fog for the current player."), "", "DS");
			register_command("shroud", &console_handler::do_shroud,
				_("Toggle shroud for the current player."), "", "DS");
			register_command("gold", &console_handler::do_gold,
				_("Give gold to the current player."), "", "DS");
			register_command("throw", &console_handler::do_event,
				_("Fire a game event."), "", "DS");
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
				for(const config::attribute &a : alias_list.attribute_range()) {
					register_alias(a.second, a.first);
				}
			}
		}
	private:
		menu_handler& menu_handler_;
		const unsigned int team_num_;
};

void menu_handler::send_chat_message(const std::string& message, bool allies_only)
{
	config cfg;
	cfg["id"] = preferences::login();
	cfg["message"] = message;
	const time_t time = ::time(nullptr);
	std::stringstream ss;
	ss << time;
	cfg["time"] = ss.str();

	const int side = board().is_observer() ? 0 : gui_->viewing_side();
	if(!board().is_observer()) {
		cfg["side"] = side;
	}

	bool private_message = has_friends() && allies_only;

	if(private_message) {
		if (board().is_observer()) {
			cfg["to_sides"] = game_config::observer_team_name;
		} else {
			cfg["to_sides"] = teams()[gui_->viewing_team()].allied_human_teams();
		}
	}

	resources::recorder->speak(cfg);

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
		if(x >= 0 && x < map().w() && y >= 0 && y < map().h()) {
			loc = map_location(x,y);
			found = true;
		}
	}
	//Start scanning the game map
	if(loc.valid() == false)
		loc = map_location(map().w()-1,map().h()-1);
	map_location start = loc;
	while (!found) {
		//Move to the next location
		loc.x = (loc.x + 1) % map().w();
		if(loc.x == 0)
			loc.y = (loc.y + 1) % map().h();

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
			unit_map::const_iterator ui = units().find(loc);
			if(ui != units().end()) {
				const std::string name = ui->name();
				if(std::search(name.begin(), name.end(),
						last_search_.begin(), last_search_.end(),
						chars_equal_insensitive) != name.end()) {
					if (!teams()[gui_->viewing_team()].is_enemy(ui->side()) ||
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

	menu_handler_.gui_->create_buttons();

	menu_handler_.gui_->redraw_everything();
}

void console_handler::do_droid() {
	// :droid [<side> [on/off]]
	const std::string side_s = get_arg(1);
	const std::string action = get_arg(2);
	// default to the current side if empty
	const unsigned int side = side_s.empty() ?
		team_num_ : lexical_cast_default<unsigned int>(side_s);

	if (side < 1 || side > menu_handler_.teams().size()) {
		utils::string_map symbols;
		symbols["side"] = side_s;
		command_failed(vgettext("Can't droid invalid side: '$side'.", symbols));
		return;
	} else if (menu_handler_.teams()[side - 1].is_network()) {
		utils::string_map symbols;
		symbols["side"] = std::to_string(side);
		command_failed(vgettext("Can't droid networked side: '$side'.", symbols));
		return;
	} else if (menu_handler_.teams()[side - 1].is_local_human()) {
		if (menu_handler_.teams()[side - 1].is_droid() ? action == " on" : action == " off") {
			return;
		}
		menu_handler_.teams()[side - 1].toggle_droid();
		if(team_num_ == side) {
			if(playsingle_controller* psc = dynamic_cast<playsingle_controller*>(&menu_handler_.pc_)) {
				psc->set_player_type_changed();
			}
		}
	} else if (menu_handler_.teams()[side - 1].is_local_ai()) {
//		menu_handler_.teams()[side - 1].make_human();
//		menu_handler_.change_controller(std::to_string(side),"human");

		utils::string_map symbols;
		symbols["side"] = side_s;
		command_failed(vgettext("Can't droid a local ai side: '$side'.", symbols));
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

	if (side < 1 || side > menu_handler_.teams().size()) {
		utils::string_map symbols;
		symbols["side"] = side_s;
		command_failed(vgettext("Can't idle invalid side: '$side'.", symbols));
		return;
	} else if (menu_handler_.teams()[side - 1].is_network()) {
		utils::string_map symbols;
		symbols["side"] = std::to_string(side);
		command_failed(vgettext("Can't idle networked side: '$side'.", symbols));
		return;
	} else if (menu_handler_.teams()[side - 1].is_local_ai()) {
		utils::string_map symbols;
		symbols["side"] = std::to_string(side);
		command_failed(vgettext("Can't idle local ai side: '$side'.", symbols));
		return;
	} else if (menu_handler_.teams()[side - 1].is_local_human()) {
		if (menu_handler_.teams()[side - 1].is_idle() ? action == " on" : action == " off") {
			return;
		}
		//toggle the proxy controller between idle / non idle
		menu_handler_.teams()[side - 1].toggle_idle();
		if(team_num_ == side) {
			if(playsingle_controller* psc = dynamic_cast<playsingle_controller*>(&menu_handler_.pc_)) {
				psc->set_player_type_changed();
			}
		}
	}
	menu_handler_.textbox_info_.close(*menu_handler_.gui_);
}

void console_handler::do_theme() {
	preferences::show_theme_dialog(menu_handler_.gui_->video());
}

struct save_id_matches
{
	save_id_matches(const std::string& save_id) : save_id_(save_id) {}
	bool operator()(const team& t)
	{
		return t.save_id() == save_id_;
	}
	std::string save_id_;
};

void console_handler::do_control() {
	// :control <side> <nick>
	if (!menu_handler_.pc_.is_networked_mp()) return;
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
		std::vector<team>::const_iterator it_t = boost::find_if(*resources::teams, save_id_matches(side));
		if(it_t == resources::teams->end()) {
			utils::string_map symbols;
			symbols["side"] = side;
			command_failed(vgettext("Can't change control of invalid side: '$side'.", symbols));
			return;
		}
		else {
			side_num = it_t->side();
		}
	}
	if (side_num < 1 || side_num > menu_handler_.teams().size()) {
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
	if (side_num < 1 || side_num > menu_handler_.teams().size()) {
		utils::string_map symbols;
		symbols["side"] = side;
		command_failed(vgettext("Can't query control of out-of-bounds side: '$side'.",	symbols));
		return;
	}

	std::string report = menu_handler_.teams()[side_num - 1].controller().to_string();
	if (!menu_handler_.teams()[side_num - 1].is_proxy_human()) {
		report += " (" + menu_handler_.teams()[side_num - 1].proxy_controller().to_string() + ")";
	}
	if (menu_handler_.teams()[side_num - 1].is_network()) {
		report += " (networked)";
	}

	print(get_cmd(), report);
}

void console_handler::do_clear() {
	menu_handler_.gui_->get_chat_manager().clear_chat_messages();
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
	const mouse_handler& mousehandler = menu_handler_.pc_.get_mouse_handler_base();
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
	for(const terrain_builder::tile::log_details det : tile_logs) {
		const terrain_builder::tile::rule_image_rand& ri = *det.first;
		const terrain_builder::rule_image_variant& variant = *det.second;

		/** @todo also use random image variations (not just take 1st) */
		const image::locator& img = variant.images.front().get_first_frame();
		const std::string& name = img.get_filename();
		/** @todo deal with (rarely used) ~modifications */
		//const std::string& modif = img.get_modifications();
		const map_location& loc_cut = img.get_loc();

		std::ostringstream str;

		int tz = game_config::tile_size;
		SDL_Rect r = sdl::create_rect(0,0,tz,tz);

		surface surf = image::get_image(img.get_filename());

		// calculate which part of the image the terrain engine uses
		if(loc_cut.valid()) {
			// copied from image.cpp : load_image_sub_file()
			r = sdl::create_rect(
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
		SDL_Rect r2 = sdl::intersect_rects(r, sdl::create_rect(0,0,surf->w,surf->h));
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
		gui::dialog menu(menu_handler_.gui_->video(), _("Layers"), info.str(), gui::OK_CANCEL);
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
	menu_handler_.pc_.do_consolesave(get_data());
}
void console_handler::do_save_quit() {
	do_save();
	do_quit();
}
void console_handler::do_quit() {
	throw_quit_game_exception();
}
void console_handler::do_ignore_replay_errors() {
	game_config::ignore_replay_errors = (get_data() != "off") ? true : false;
}
void console_handler::do_nosaves() {
	game_config::disable_autosave = (get_data() != "off") ? true : false;
}

void console_handler::do_next_level()
{
	synced_context::run_and_throw("debug_next_level", config_of("next_level", get_data()));
}

void console_handler::do_choose_level() {
	std::vector<std::string> options;
	int next = 0, nb = 0;
	for(const config &sc : menu_handler_.game_config_.child_range("scenario"))
	{
		const std::string &id = sc["id"];
		options.push_back(id);
		if (id == menu_handler_.gamedata().next_scenario())
			next = nb;
		++nb;
	}
	// find scenarios of multiplayer campaigns
	// (assumes that scenarios are ordered properly in the game_config)
	std::string scenario = menu_handler_.pc_.get_mp_settings().mp_scenario;
	for(const config &mp : menu_handler_.game_config_.child_range("multiplayer"))
	{
		if (mp["id"] == scenario)
		{
			const std::string &id = mp["id"];
			options.push_back(id);
			if (id == menu_handler_.gamedata().next_scenario())
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
		synced_context::run_and_throw("debug_next_level", config_of("next_level", options[choice]));
	}
}

void console_handler::do_turn()
{
	tod_manager& tod_man = menu_handler_.gamestate().tod_manager_;

	int turn = tod_man.turn() + 1;
	const std::string& data = get_data();
	if (!data.empty()) {
		turn = lexical_cast_default<int>(data, 1);
	}
	synced_context::run_and_throw("debug_turn", config_of("turn", turn));
}

void console_handler::do_turn_limit()
{
	int limit = get_data().empty() ? -1 : lexical_cast_default<int>(get_data(), 1);
	synced_context::run_and_throw("debug_turn_limit", config_of("turn_limit", limit));
}

void console_handler::do_debug() {
	if (!menu_handler_.pc_.is_networked_mp() || game_config::mp_debug) {
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
	if (!menu_handler_.gamestate().lua_kernel_) {
		return ;
	}
	synced_context::run_and_throw("debug_lua", config_of("code", get_data()));
}

void console_handler::do_unsafe_lua()
{
	if (!menu_handler_.gamestate().lua_kernel_) {
		return ;
	}
	if (gui2::show_message(menu_handler_.gui_->video(), _("Unsafe Lua scripts."),
		_("You are about to open a security breach in Wesnoth. Are you sure you want to continue? If you have downloaded add-ons, do not click 'ok'! They would instantly take over your computer. You have been warned."),
		gui2::tmessage::ok_cancel_buttons) == gui2::twindow::OK)
	{
		print(get_cmd(), _("Unsafe mode enabled!"));
		menu_handler_.gamestate().lua_kernel_->load_package();
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
		synced_context::run_and_throw("debug_set_var", config_of("name", name)("value", value));
	}
	else {
		command_failed(_("Variable not found"));
	}
}
void console_handler::do_show_var() {
	gui2::show_transient_message((*menu_handler_.gui_).video(),"",menu_handler_.gamedata().get_variable_const(get_data()));
}


void console_handler::do_inspect() {
	vconfig cfg = vconfig::empty_vconfig();
	gui2::tgamestate_inspector inspect_dialog(cfg);
	inspect_dialog.show(menu_handler_.gui_->video());
}

void console_handler::do_control_dialog()
{
	gui2::tmp_change_control mp_change_control(&menu_handler_);
	mp_change_control.show(menu_handler_.gui_->video());
}

void console_handler::do_unit() {
	// prevent SIGSEGV due to attempt to set HP during a fight
	if (events::commands_disabled > 0)
		return;
	unit_map::iterator i = menu_handler_.current_unit();
	if (i == menu_handler_.units().end()) return;
	const map_location loc = i->get_location();
	const std::string data = get_data(1);
	std::vector<std::string> parameters = utils::split(data, '=', utils::STRIP_SPACES);
	if (parameters.size() < 2) {
		return;
	}

	if (parameters[0] == "alignment") {
		unit_type::ALIGNMENT alignment;
		if (!alignment.parse(parameters[1]))
		{
			utils::string_map symbols;
			symbols["alignment"] = get_arg(1);
			command_failed(VGETTEXT("Invalid alignment: '$alignment', needs to be one of lawful, neutral, chaotic, or liminal.", symbols));
			return;
		}
	}

	synced_context::run_and_throw("debug_unit", config_of("x", loc.x + 1)("y", loc.y + 1)("name", parameters[0])("value", parameters[1]));
}

void console_handler::do_discover() {
	for(const unit_type_data::unit_type_map::value_type &i : unit_types.types()) {
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
	const mouse_handler& mousehandler = menu_handler_.pc_.get_mouse_handler_base();
	const map_location &loc = mousehandler.get_last_hex();
	if (menu_handler_.map().on_board(loc)) {
		const unit_type *ut = unit_types.find(get_data());
		if (!ut) {
			command_failed(_("Invalid unit type"));
			return;
		}

		// Create the unit.
		create_and_place(*menu_handler_.gui_, menu_handler_.map(),
		                 menu_handler_.units(), loc, *ut);
	} else {
		command_failed(_("Invalid location"));
	}
}
void console_handler::do_fog() {
	synced_context::run_and_throw("debug_fog", config());
}
void console_handler::do_shroud() {
	synced_context::run_and_throw("debug_shroud", config());
}
void console_handler::do_gold() {
	synced_context::run_and_throw("debug_gold", config_of("gold", lexical_cast_default<int>(get_data(),1000)));
}
void console_handler::do_event() {
	synced_context::run_and_throw("debug_event", config_of("eventname", get_data()));
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
	if (const boost::shared_ptr<wb::manager> & whiteb = menu_handler_.pc_.get_whiteboard()) {
		whiteb->set_active(!whiteb->is_active());
		if (whiteb->is_active()) {
			print(get_cmd(), _("Planning mode activated!"));
			whiteb->print_help_once();
		} else {
			print(get_cmd(), _("Planning mode deactivated!"));
		}
	}
}

void console_handler::do_whiteboard_options()
{
	if (menu_handler_.pc_.get_whiteboard()) {
		menu_handler_.pc_.get_whiteboard()->options_dlg();
	}
}

void menu_handler::do_ai_formula(const std::string& str,
	int side_num, mouse_handler& /*mousehandler*/)
{
	try {
		add_chat_message(time(nullptr), _("wfl"), 0, ai::manager::evaluate_command(side_num, str));
	} catch(game_logic::formula_error&) {
	} catch(...) {
		add_chat_message(time(nullptr), _("wfl"), 0, "UNKNOWN ERROR IN FORMULA");
	}
}

void menu_handler::user_command()
{
	textbox_info_.show(gui::TEXTBOX_COMMAND, translation::sgettext("prompt^Command:"), "", false, *gui_);
}

void menu_handler::request_control_change ( int side_num, const std::string& player )
{
	std::string side = std::to_string(side_num);
	if (teams()[side_num - 1].is_local_human() && player == preferences::login()) {
		//this is already our side.
		return;
	} else {
		//The server will (or won't because we aren't allowed to change the controller)
		//send us a [change_controller] back, which we then handle in playturn.cpp
		pc_.send_to_wesnothd(config_of
			("change_controller", config_of
				("side", side)
				("player", player)
			)
		);

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
	if (!pc_.is_networked_mp()) {
		textbox_info_.show(gui::TEXTBOX_AI, translation::sgettext("prompt^Command:"), "", false, *gui_);
	}
}

void menu_handler::clear_messages()
{
	gui_->get_chat_manager().clear_chat_messages();	// also clear debug-messages and WML-error-messages
}

void menu_handler::send_to_server(const config& cfg)
{
	pc_.send_to_wesnothd(cfg);
}

} // end namespace events
