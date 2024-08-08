/*
	Copyright (C) 2006 - 2024
	by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
	Copyright (C) 2003 by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

#include "menu_events.hpp"

#include "actions/create.hpp"
#include "actions/move.hpp"
#include "actions/undo.hpp"
#include "ai/manager.hpp"
#include "chat_command_handler.hpp"
#include "color.hpp"
#include "display_chat_manager.hpp"
#include "font/standard_colors.hpp"
#include "formula/string_utils.hpp"
#include "game_board.hpp"
#include "game_config_manager.hpp"
#include "game_end_exceptions.hpp"
#include "preferences/preferences.hpp"
#include "game_initialization/multiplayer.hpp"
#include "game_state.hpp"
#include "gettext.hpp"
#include "gui/dialogs/chat_log.hpp"
#include "gui/dialogs/edit_label.hpp"
#include "gui/dialogs/edit_text.hpp"
#include "gui/dialogs/file_dialog.hpp"
#include "gui/dialogs/game_stats.hpp"
#include "gui/dialogs/gamestate_inspector.hpp"
#include "gui/dialogs/label_settings.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/multiplayer/mp_change_control.hpp"
#include "gui/dialogs/preferences_dialog.hpp"
#include "gui/dialogs/simple_item_selector.hpp"
#include "gui/dialogs/statistics_dialog.hpp"
#include "gui/dialogs/terrain_layers.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/dialogs/unit_create.hpp"
#include "gui/dialogs/unit_list.hpp"
#include "gui/dialogs/unit_recall.hpp"
#include "gui/dialogs/unit_recruit.hpp"
#include "gui/widgets/retval.hpp"
#include "help/help.hpp"
#include "log.hpp"
#include "map/label.hpp"
#include "map/map.hpp"
#include "map_command_handler.hpp"
#include "mouse_events.hpp"
#include "play_controller.hpp"
#include "playsingle_controller.hpp"
#include "replay.hpp"
#include "replay_controller.hpp"
#include "replay_helper.hpp"
#include "resources.hpp"
#include "savegame.hpp"
#include "scripting/game_lua_kernel.hpp"
#include "scripting/plugins/manager.hpp"
#include "synced_context.hpp"
#include "units/unit.hpp"
#include "units/types.hpp"
#include "whiteboard/manager.hpp"
#include "sound.hpp"

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)

namespace events
{
menu_handler::menu_handler(game_display* gui, play_controller& pc)
	: gui_(gui)
	, pc_(pc)
	, game_config_(game_config_manager::get()->game_config())
	, textbox_info_()
	, last_search_()
	, last_search_hit_()
{
}

menu_handler::~menu_handler()
{
}

game_state& menu_handler::gamestate() const
{
	return pc_.gamestate();
}

game_data& menu_handler::gamedata()
{
	return gamestate().gamedata_;
}

game_board& menu_handler::board() const
{
	return gamestate().board_;
}

gui::floating_textbox& menu_handler::get_textbox()
{
	return textbox_info_;
}

void menu_handler::objectives()
{
	if(!gamestate().lua_kernel_) {
		return;
	}

	pc_.refresh_objectives();
	pc_.show_objectives();
}

void menu_handler::show_statistics(int side_num)
{
	gui2::dialogs::statistics_dialog::display(pc_.statistics(), board().get_team(side_num));
}

void menu_handler::unit_list()
{
	gui2::dialogs::show_unit_list(*gui_);
}

void menu_handler::status_table()
{
	int selected_side;

	if(gui2::dialogs::game_stats::execute(board(), gui_->viewing_team(), selected_side)) {
		gui_->scroll_to_leader(selected_side);
	}
}

void menu_handler::save_map()
{
	const std::string& input_name
			= filesystem::get_dir(filesystem::get_dir(filesystem::get_user_data_dir() + "/editor") + "/maps/");

	gui2::dialogs::file_dialog dlg;

	dlg.set_title(_("Save Map As"))
	   .set_save_mode(true)
	   .set_path(input_name)
	   .set_extension(filesystem::map_extension);

	if(!dlg.show()) {
		return;
	}

	try {
		filesystem::write_file(dlg.path(), pc_.get_map().write());
		gui2::show_transient_message("", _("Map saved."));
	} catch(const filesystem::io_exception& e) {
		utils::string_map symbols;
		symbols["msg"] = e.what();
		const std::string msg = VGETTEXT("Could not save the map: $msg", symbols);
		gui2::show_transient_error_message(msg);
	}
}

void menu_handler::preferences()
{
	gui2::dialogs::preferences_dialog::display();
	// Needed after changing fullscreen/windowed mode or display resolution
	gui_->queue_rerender();
}

void menu_handler::show_chat_log()
{
	config c;
	c["name"] = "prototype of chat log";
	gui2::dialogs::chat_log::display(vconfig(c), *resources::recorder);
	// std::string text = resources::recorder->build_chat_log();
	// gui::show_dialog(*gui_,nullptr,_("Chat Log"),"",gui::CLOSE_ONLY,nullptr,nullptr,"",&text);
}

void menu_handler::show_help()
{
	help::show_help();
}

void menu_handler::speak()
{
	textbox_info_.show(gui::TEXTBOX_MESSAGE, _("Message:"), has_friends()
		? board().is_observer()
			? _("Send to observers only")
			: _("Send to allies only")
		: "", prefs::get().message_private(), *gui_);
}

void menu_handler::whisper()
{
	prefs::get().set_message_private(true);
	speak();
}

void menu_handler::shout()
{
	prefs::get().set_message_private(false);
	speak();
}

bool menu_handler::has_friends() const
{
	if(board().is_observer()) {
		return !gui_->observers().empty();
	}

	for(std::size_t n = 0; n != pc_.get_teams().size(); ++n) {
		if(n != gui_->viewing_team() && pc_.get_teams()[gui_->viewing_team()].team_name() == pc_.get_teams()[n].team_name()
				&& pc_.get_teams()[n].is_network()) {
			return true;
		}
	}

	return false;
}

void menu_handler::recruit(int side_num, const map_location& last_hex)
{
	std::map<const unit_type*, t_string> sample_units;

	std::set<std::string> recruits = actions::get_recruits(side_num, last_hex);

	std::vector<t_string> unknown_units;
	for(const auto& recruit : recruits) {
		const unit_type* type = unit_types.find(recruit);
		if(!type) {
			ERR_NG << "could not find unit '" << recruit << "'";
			unknown_units.emplace_back(recruit);
			continue;
		}

		map_location ignored;
		map_location recruit_hex = last_hex;
		sample_units[type] = (can_recruit(type->id(), side_num, recruit_hex, ignored));
	}
	if(!unknown_units.empty()) {
		auto unknown_ids = utils::format_conjunct_list("", unknown_units);
		// TRANSLATORS: An error that should never happen, might happen when loading an old savegame. If there are
		// any units that the player can recruit then their standard recruitment dialog will be shown after this
		// error message, otherwise they'll get the "You have no units available to recruit." error after this one.
		const auto message = VNGETTEXT("Error: there’s an unknown unit type on your recruit list: $unknown_ids",
			"Error: there are several unknown unit types on your recruit list: $unknown_ids",
			unknown_units.size(),
			utils::string_map { { "unknown_ids", unknown_ids }});
		gui2::show_transient_message("", message);
	}

	if(sample_units.empty()) {
		gui2::show_transient_message("", _("You have no units available to recruit."));
		return;
	}

	gui2::dialogs::unit_recruit dlg(sample_units, board().get_team(side_num));

	if(dlg.show()) {
		map_location recruit_hex = last_hex;
		const unit_type *type = dlg.get_selected_unit_type();
		if (!type) {
			gui2::show_transient_message("", _("No unit recruited."));
			return;
		}
		do_recruit(type->id(), side_num, recruit_hex);
	}
}

void menu_handler::repeat_recruit(int side_num, const map_location& last_hex)
{
	const std::string& last_recruit = board().get_team(side_num).last_recruit();
	if(last_recruit.empty() == false) {
		map_location recruit_hex = last_hex;
		do_recruit(last_recruit, side_num, recruit_hex);
	}
}

// TODO: Return multiple strings here, in case more than one error applies? For
// example, if you start AOI S5 with 0GP and recruit a Mage, two reasons apply,
// leader not on keep (extrarecruit=Mage) and not enough gold.
t_string menu_handler::can_recruit(const std::string& name, int side_num, map_location& loc, map_location& recruited_from)
{
	team& current_team = board().get_team(side_num);

	const unit_type* u_type = unit_types.find(name);
	if(u_type == nullptr) {
		return _("Internal error. Please report this as a bug! Details:\n")
			+ "menu_handler::can_recruit: u_type == nullptr for " + name;
	}

	// search for the unit to be recruited in recruits
	if(!utils::contains(actions::get_recruits(side_num, loc), name)) {
		return VGETTEXT("You cannot recruit a $unit_type_name at this time.",
				utils::string_map { { "unit_type_name", u_type->type_name() }});
	}

	// TODO take a wb::future_map RAII as unit_recruit::pre_show does
	int wb_gold = 0;
	{
		wb::future_map future;
		wb_gold = (pc_.get_whiteboard() ? pc_.get_whiteboard()->get_spent_gold_for(side_num) : 0);
	}
	if(u_type->cost() > current_team.gold() - wb_gold)
	{
		if(wb_gold > 0)
			// TRANSLATORS: "plan" refers to Planning Mode
			return _("At this point in your plan, you will not have enough gold to recruit this unit.");
		else
			return _("You do not have enough gold to recruit this unit.");
	}

	current_team.last_recruit(name);
	const events::command_disabler disable_commands;

	{
		wb::future_map_if_active future; /* start planned unit map scope if in planning mode */
		std::string msg = actions::find_recruit_location(side_num, loc, recruited_from, name);
		if(!msg.empty()) {
			return msg;
		}
	} // end planned unit map scope

	return "";
}

bool menu_handler::do_recruit(const std::string& name, int side_num, map_location& loc)
{
	map_location recruited_from = map_location::null_location();
	const std::string res = can_recruit(name, side_num, loc, recruited_from);
	team& current_team = board().get_team(side_num);

	if(res.empty() && (!pc_.get_whiteboard() || !pc_.get_whiteboard()->save_recruit(name, side_num, loc))) {
		// MP_COUNTDOWN grant time bonus for recruiting
		current_team.set_action_bonus_count(1 + current_team.action_bonus_count());

		// Do the recruiting.

		synced_context::run_and_throw("recruit", replay_helper::get_recruit(name, loc, recruited_from));
		return true;
	} else if(res.empty()) {
		return false;
	} else {
		gui2::show_transient_message("", res);
		return false;
	}

}

void menu_handler::recall(int side_num, const map_location& last_hex)
{
	if(pc_.get_disallow_recall()) {
		gui2::show_transient_message("", _("You are separated from your soldiers and may not recall them."));
		return;
	}

	team& current_team = board().get_team(side_num);

	std::vector<unit_const_ptr> recall_list_team;
	bool empty;
	{
		wb::future_map future; // ensures recall list has planned recalls removed
		recall_list_team = actions::get_recalls(side_num, last_hex);
		empty = current_team.recall_list().empty();
	}

	DBG_WB << "menu_handler::recall: Contents of wb-modified recall list:";
	for(const unit_const_ptr& unit : recall_list_team) {
		DBG_WB << unit->name() << " [" << unit->id() << "]";
	}

	if(empty) {
		gui2::show_transient_message("",
			_("There are no troops available to recall.\n(You must have veteran survivors from a previous scenario.)"));
		return;
	}
	if(recall_list_team.empty()) {
		gui2::show_transient_message("", _("You currently can't recall at the highlighted location."));
		return;
	}

	gui2::dialogs::unit_recall dlg(recall_list_team, current_team);

	if(!dlg.show()) {
		return;
	}

	int res = dlg.get_selected_index();
	if (res < 0) {
		gui2::show_transient_message("", _("No unit recalled."));
		return;
	}
	int unit_cost = current_team.recall_cost();

	// we need to check if unit has a specific recall cost
	// if it does we use it elsewise we use the team.recall_cost()
	// the magic number -1 is what it gets set to if the unit doesn't
	// have a special recall_cost of its own.
	if(recall_list_team[res]->recall_cost() > -1) {
		unit_cost = recall_list_team[res]->recall_cost();
	}

	int wb_gold = pc_.get_whiteboard() ? pc_.get_whiteboard()->get_spent_gold_for(side_num) : 0;
	if(current_team.gold() - wb_gold < unit_cost) {
		utils::string_map i18n_symbols;
		i18n_symbols["cost"] = std::to_string(unit_cost);
		std::string msg = VNGETTEXT("You must have at least 1 gold piece to recall a unit.",
				"You must have at least $cost gold pieces to recall this unit.", unit_cost, i18n_symbols);
		gui2::show_transient_message("", msg);
		return;
	}

	LOG_NG << "recall index: " << res;
	const events::command_disabler disable_commands;

	map_location recall_location = last_hex;
	map_location recall_from = map_location::null_location();
	std::string err;
	{
		wb::future_map_if_active
				future; // future unit map removes invisible units from map, don't do this outside of planning mode
		err = actions::find_recall_location(side_num, recall_location, recall_from, *recall_list_team[res].get());
	} // end planned unit map scope

	if(!err.empty()) {
		gui2::show_transient_message("", err);
		return;
	}

	if(!pc_.get_whiteboard()
			|| !pc_.get_whiteboard()->save_recall(*recall_list_team[res].get(), side_num, recall_location)) {
		bool success = synced_context::run_and_throw("recall",
				replay_helper::get_recall(recall_list_team[res]->id(), recall_location, recall_from), true, true,
				synced_context::ignore_error_function);

		if(!success) {
			ERR_NG << "menu_handler::recall(): Unit does not exist in the recall list.";
		}
	}
}

// Highlights squares that an enemy could move to on their turn, showing how many can reach each square.
void menu_handler::show_enemy_moves(bool ignore_units, int side_num)
{
	wb::future_map future; // use unit positions as if all planned actions were executed

	mouse_handler& mh = pc_.get_mouse_handler_base();
	const map_location& hex_under_mouse = mh.hovered_hex();

	gui_->unhighlight_reach();

	// Compute enemy movement positions
	for(auto& u : pc_.get_units()) {
		bool invisible = u.invisible(u.get_location());

		if(board().get_team(side_num).is_enemy(u.side()) && !gui_->fogged(u.get_location()) && !u.incapacitated()
				&& !invisible) {
			const unit_movement_resetter move_reset(u);
			const pathfind::paths& path
					= pathfind::paths(u, false, true, pc_.get_teams()[gui_->viewing_team()], 0, false, ignore_units);

			gui_->highlight_another_reach(path, hex_under_mouse);
		}

		// Need to recompute ellipses for highlighted enemy units
		gui_->invalidate(u.get_location());
	}

	// Find possible unit (no matter whether friend or foe) under the
	// mouse cursor.
	const bool selected_hex_has_unit = mh.hex_hosts_unit(hex_under_mouse);

	if(selected_hex_has_unit) {
		// At this point, a single pixel move would remove the enemy
		// [best possible] movements hex tiles highlights, so some
		// prevention on normal unit mouseover movement highlight
		// has to be toggled temporarily.
		mh.disable_units_highlight();
	}
}

void menu_handler::toggle_shroud_updates(int side_num)
{
	team& current_team = board().get_team(side_num);
	bool auto_shroud = current_team.auto_shroud_updates();
	// If we're turning automatic shroud updates on, then commit all moves
	if(!auto_shroud) {
		update_shroud_now(side_num);
	}

	// Toggle the setting and record this.
	synced_context::run_and_throw("auto_shroud", replay_helper::get_auto_shroud(!auto_shroud));
}

void menu_handler::update_shroud_now(int /* side_num */)
{
	synced_context::run_and_throw("update_shroud", replay_helper::get_update_shroud());
}

// Helpers for menu_handler::end_turn()
namespace
{
/** Returns true if @a side_num has at least one living unit. */
bool units_alive(int side_num, const unit_map& units)
{
	for(auto& unit : units) {
		if(unit.side() == side_num) {
			return true;
		}
	}
	return false;
}

/** Returns true if @a side_num has at least one unit that can still move. */
bool partmoved_units(
		int side_num, const unit_map& units, const game_board& board, const std::shared_ptr<wb::manager>& whiteb)
{
	for(auto& unit : units) {
		if(unit.side() == side_num) {
			// @todo whiteboard should take into consideration units that have
			// a planned move but can still plan more movement in the same turn
			if(board.unit_can_move(unit) && !unit.user_end_turn() && (!whiteb || !whiteb->unit_has_actions(&unit)))
				return true;
		}
	}
	return false;
}

/**
 * Returns true if @a side_num has at least one unit that (can but) has not moved.
 */
bool unmoved_units(
		int side_num, const unit_map& units, const game_board& board, const std::shared_ptr<wb::manager>& whiteb)
{
	for(auto& unit : units) {
		if(unit.side() == side_num) {
			if(board.unit_can_move(unit) && !unit.has_moved() && !unit.user_end_turn()
					&& (!whiteb || !whiteb->unit_has_actions(&unit))) {
				return true;
			}
		}
	}
	return false;
}

} // end anon namespace

bool menu_handler::end_turn(int side_num)
{
	if(!gamedata().allow_end_turn()) {
		t_string reason = gamedata().cannot_end_turn_reason();
		if(reason.empty()) {
			reason = _("You cannot end your turn yet!");
		}
		gui2::show_transient_message("", reason);
		return false;
	}

	std::size_t team_num = static_cast<std::size_t>(side_num - 1);
	if(team_num < pc_.get_teams().size() && pc_.get_teams()[team_num].no_turn_confirmation()) {
		// Skip the confirmations that follow.
	}
	// Ask for confirmation if the player hasn't made any moves.
	else if(prefs::get().confirm_no_moves() && !pc_.get_undo_stack().player_acted()
			&& (!pc_.get_whiteboard() || !pc_.get_whiteboard()->current_side_has_actions())
			&& units_alive(side_num, pc_.get_units())) {
		const int res = gui2::show_message("",
				_("You have not started your turn yet. Do you really want to end your turn?"),
				gui2::dialogs::message::yes_no_buttons);
		if(res == gui2::retval::CANCEL) {
			return false;
		}
	}
	// Ask for confirmation if units still have some movement left.
	else if(prefs::get().yellow_confirm() && partmoved_units(side_num, pc_.get_units(), board(), pc_.get_whiteboard())) {
		const int res = gui2::show_message("",
				_("Some units have movement left. Do you really want to end your turn?"),
				gui2::dialogs::message::yes_no_buttons);
		if(res == gui2::retval::CANCEL) {
			return false;
		}
	}
	// Ask for confirmation if units still have all movement left.
	else if(prefs::get().green_confirm() && unmoved_units(side_num, pc_.get_units(), board(), pc_.get_whiteboard())) {
		const int res = gui2::show_message("",
				_("Some units have not moved. Do you really want to end your turn?"),
				gui2::dialogs::message::yes_no_buttons);
		if(res == gui2::retval::CANCEL) {
			return false;
		}
	}

	// Auto-execute remaining whiteboard planned actions
	// Only finish turn if they all execute successfully, i.e. no ambush, etc.
	if(pc_.get_whiteboard() && !pc_.get_whiteboard()->allow_end_turn()) {
		return false;
	}

	return true;
}

void menu_handler::goto_leader(int side_num)
{
	unit_map::const_iterator i = pc_.get_units().find_leader(side_num);
	const display_context& dc = gui_->get_disp_context();
	if(i != pc_.get_units().end() && i->is_visible_to_team(dc.get_team(gui_->viewing_side()), false)) {
		gui_->scroll_to_tile(i->get_location(), game_display::WARP);
	}
}

void menu_handler::unit_description()
{
	const unit_map::const_iterator un = current_unit();
	if(un != pc_.get_units().end()) {
		help::show_unit_description(*un);
	}
}

void menu_handler::terrain_description(mouse_handler& mousehandler)
{
	const map_location& loc = mousehandler.get_last_hex();
	if(pc_.get_map().on_board(loc) == false || gui_->shrouded(loc)) {
		return;
	}

	const terrain_type& type = pc_.get_map().get_terrain_info(loc);
	// const terrain_type& info = board().pc_.get_map().get_terrain_info(terrain);
	help::show_terrain_description(type);
}

void menu_handler::rename_unit()
{
	const unit_map::iterator un = current_unit();
	if(un == pc_.get_units().end() || gui_->viewing_side() != un->side()) {
		return;
	}

	if(un->unrenamable()) {
		return;
	}

	std::string name = un->name();
	const std::string title(_("Rename Unit"));
	const std::string label(_("Name:"));

	if(gui2::dialogs::edit_text::execute(title, label, name)) {
		resources::recorder->add_rename(name, un->get_location());
		un->rename(name);
		gui_->invalidate_unit();
	}
}

unit_map::iterator menu_handler::current_unit()
{
	const mouse_handler& mousehandler = pc_.get_mouse_handler_base();
	const bool see_all = gui_->show_everything() || (pc_.is_replay() && pc_.get_replay_controller()->see_all());

	unit_map::iterator res = board().find_visible_unit(mousehandler.get_last_hex(), pc_.get_teams()[gui_->viewing_team()], see_all);
	if(res != pc_.get_units().end()) {
		return res;
	}

	return board().find_visible_unit(mousehandler.get_selected_hex(), pc_.get_teams()[gui_->viewing_team()], see_all);
}

// Helpers for create_unit()
namespace
{
/** Allows a function to return both a type and a gender. */
typedef std::tuple<const unit_type*, unit_race::GENDER, std::string> type_gender_variation;

/**
 * Allows the user to select a type of unit, using GUI2.
 * (Intended for use when a unit is created in debug mode via hotkey or
 * context menu.)
 * @returns the selected type and gender. If this is canceled, the
 *          returned type is nullptr.
 */
type_gender_variation choose_unit()
{
	//
	// The unit creation dialog makes sure unit types
	// are properly cached.
	//
	gui2::dialogs::unit_create create_dlg;
	create_dlg.show();

	if(create_dlg.no_choice()) {
		return type_gender_variation(nullptr, unit_race::NUM_GENDERS, "");
	}

	const std::string& ut_id = create_dlg.choice();
	const unit_type* utp = unit_types.find(ut_id);
	if(!utp) {
		ERR_NG << "Create unit dialog returned nonexistent or unusable unit_type id '" << ut_id << "'.";
		return type_gender_variation(static_cast<const unit_type*>(nullptr), unit_race::NUM_GENDERS, "");
	}
	const unit_type& ut = *utp;

	unit_race::GENDER gender = create_dlg.gender();
	// Do not try to set bad genders, may mess up l10n
	// TODO: Is this actually necessary?
	// (Maybe create_dlg can enforce proper gender selection?)
	if(ut.genders().end() == std::find(ut.genders().begin(), ut.genders().end(), gender)) {
		gender = ut.genders().front();
	}

	return type_gender_variation(utp, gender, create_dlg.variation());
}

/**
 * Creates a unit and places it on the board.
 * (Intended for use with any units created via debug mode.)
 */
void create_and_place(game_display&,
		const gamemap&,
		unit_map&,
		const map_location& loc,
		const unit_type& u_type,
		unit_race::GENDER gender = unit_race::NUM_GENDERS,
		const std::string& variation = "")
{
	synced_context::run_and_throw("debug_create_unit",
		config {
			"x", loc.wml_x(),
			"y", loc.wml_y(),
			"type", u_type.id(),
			"gender", gender_string(gender),
			"variation", variation,
		}
	);
}

} // Anonymous namespace

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
	if(const auto& [type, gender, variation] = choose_unit(); type != nullptr) {
		// Make it so.
		create_and_place(*gui_, pc_.get_map(), pc_.get_units(), destination, *type, gender, variation);
	}
}

void menu_handler::change_side(mouse_handler& mousehandler)
{
	const map_location& loc = mousehandler.get_last_hex();
	const unit_map::iterator i = pc_.get_units().find(loc);
	if(i == pc_.get_units().end()) {
		if(!pc_.get_map().is_village(loc)) {
			return;
		}

		// village_owner returns 0 for free village, so team 0 will get it
		int team = board().village_owner(loc);
		// team is 0-based so team=team::nteams() is not a team
		// but this will make get_village free it
		if(team > static_cast<int>(pc_.get_teams().size())) {
			team = 0;
		}
		actions::get_village(loc, team);
	} else {
		int side = i->side();
		++side;
		if(side > static_cast<int>(pc_.get_teams().size())) {
			side = 1;
		}
		i->set_side(side);

		if(pc_.get_map().is_village(loc)) {
			actions::get_village(loc, side);
		}
	}
}

void menu_handler::kill_unit(mouse_handler& mousehandler)
{
	const map_location loc = mousehandler.get_last_hex();
	synced_context::run_and_throw("debug_kill", config {"x", loc.wml_x(), "y", loc.wml_y()});
}

void menu_handler::label_terrain(mouse_handler& mousehandler, bool team_only)
{
	const map_location& loc = mousehandler.get_last_hex();
	if(pc_.get_map().on_board(loc) == false) {
		return;
	}

	const terrain_label* old_label = gui_->labels().get_label(loc);
	std::string label = old_label ? old_label->text() : "";

	if(gui2::dialogs::edit_label::execute(label, team_only)) {
		std::string team_name;
		color_t color = font::LABEL_COLOR;

		if(team_only) {
			team_name = gui_->labels().team_name();
		} else {
			color = team::get_side_color(gui_->viewing_side());
		}
		const terrain_label* res = gui_->labels().set_label(loc, label, gui_->viewing_team(), team_name, color);
		if(res) {
			resources::recorder->add_label(res);
		}
	}
}

void menu_handler::clear_labels()
{
	if(gui_->team_valid() && !board().is_observer()) {
		const int res = gui2::show_message(
			_("Clear Labels"),
			_("Are you sure you want to clear map labels?"),
			gui2::dialogs::message::yes_no_buttons
		);

		if(res == gui2::retval::OK) {
			gui_->labels().clear(gui_->current_team_name(), false);
			resources::recorder->clear_labels(gui_->current_team_name(), false);
		}
	}
}

void menu_handler::label_settings()
{
	if(gui2::dialogs::label_settings::execute(board())) {
		gui_->labels().recalculate_labels();
	}
}

void menu_handler::continue_move(mouse_handler& mousehandler, int side_num)
{
	unit_map::iterator i = current_unit();
	if(i == pc_.get_units().end() || !i->move_interrupted()) {
		i = pc_.get_units().find(mousehandler.get_selected_hex());
		if(i == pc_.get_units().end() || !i->move_interrupted()) {
			return;
		}
	}
	move_unit_to_loc(i, i->get_interrupted_move(), true, side_num, mousehandler);
}

void menu_handler::move_unit_to_loc(const unit_map::iterator& ui,
		const map_location& target,
		bool continue_move,
		int side_num,
		mouse_handler& mousehandler)
{
	assert(ui != pc_.get_units().end());

	pathfind::marked_route route = mousehandler.get_route(&*ui, target, board().get_team(side_num));

	if(route.steps.empty()) {
		return;
	}

	assert(route.steps.front() == ui->get_location());

	gui_->set_route(&route);
	gui_->unhighlight_reach();

	{
		LOG_NG << "move_unit_to_loc " << route.steps.front() << " to " << route.steps.back();
		actions::move_unit_and_record(route.steps, &pc_.get_undo_stack(), continue_move);
	}

	mousehandler.deselect_hex();
	gui_->set_route(nullptr);
	gui_->invalidate_game_status();
}

void menu_handler::execute_gotos(mouse_handler& mousehandler, int side)
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
		for(auto& unit : pc_.get_units()) {
			if(unit.side() != side || unit.movement_left() == 0) {
				continue;
			}

			const map_location& current_loc = unit.get_location();
			const map_location& goto_loc = unit.get_goto();

			if(goto_loc == current_loc) {
				unit.set_goto(map_location());
				continue;
			}

			if(!pc_.get_map().on_board(goto_loc)) {
				continue;
			}

			// avoid pathfinding calls for finished units
			if(fully_moved.count(current_loc)) {
				continue;
			}

			pathfind::marked_route route = mousehandler.get_route(&unit, goto_loc, board().get_team(side));

			if(route.steps.size() <= 1) { // invalid path
				fully_moved.insert(current_loc);
				continue;
			}

			// look where we will stop this turn (turn_1 waypoint or goto)
			map_location next_stop = goto_loc;
			pathfind::marked_route::mark_map::const_iterator w = route.marks.begin();
			for(; w != route.marks.end(); ++w) {
				if(w->second.turns == 1) {
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
			if(pc_.get_units().count(next_stop)) {
				blocked_unit = true;
				if(wait_blocker_move)
					continue;
			}

			gui_->set_route(&route);

			{
				LOG_NG << "execute goto from " << route.steps.front() << " to " << route.steps.back();
				int moves = actions::move_unit_and_record(route.steps, &pc_.get_undo_stack());
				change = moves > 0;
			}

			if(change) {
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
	prefs::get().set_show_side_colors(!prefs::get().show_side_colors());
	gui_->invalidate_all(); // TODO can fewer tiles be invalidated?
}

void menu_handler::toggle_grid()
{
	prefs::get().set_grid(!prefs::get().grid());
	gui_->invalidate_all();
}

void menu_handler::unit_hold_position(mouse_handler& mousehandler, int side_num)
{
	const unit_map::iterator un = pc_.get_units().find(mousehandler.get_selected_hex());
	if(un != pc_.get_units().end() && un->side() == side_num && un->movement_left() >= 0) {
		un->toggle_hold_position();
		gui_->invalidate(mousehandler.get_selected_hex());

		mousehandler.set_current_paths(pathfind::paths());

		if(un->hold_position()) {
			mousehandler.cycle_units(false);
		}
	}
}

void menu_handler::end_unit_turn(mouse_handler& mousehandler, int side_num)
{
	const unit_map::iterator un = pc_.get_units().find(mousehandler.get_selected_hex());
	if(un != pc_.get_units().end() && un->side() == side_num && un->movement_left() >= 0) {
		un->toggle_user_end_turn();
		gui_->invalidate(mousehandler.get_selected_hex());

		mousehandler.set_current_paths(pathfind::paths());

		if(un->user_end_turn()) {
			mousehandler.cycle_units(false);
		}

		// If cycle_units hasn't found a new unit to cycle to then the original unit is still selected, but
		// in a state where left-clicking on it does nothing. Make it respond to mouse clicks again.
		if(un == pc_.get_units().find(mousehandler.get_selected_hex())) {
			mousehandler.deselect_hex();
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
	textbox_info_.show(gui::TEXTBOX_SEARCH, msg.str(), "", false, *gui_);
}

void menu_handler::do_speak()
{
	// None of the two parameters really needs to be passed since the information belong to members of the class.
	// But since it makes the called method more generic, it is done anyway.
	chat_handler::do_speak(
			textbox_info_.box()->text(), textbox_info_.check() != nullptr ? textbox_info_.check()->checked() : false);
}

void menu_handler::add_chat_message(const std::time_t& time,
		const std::string& speaker,
		int side,
		const std::string& message,
		events::chat_handler::MESSAGE_TYPE type)
{
	gui_->get_chat_manager().add_chat_message(time, speaker, side, message, type, false);

	plugins_manager::get()->notify_event("chat",
		config {
			"sender", prefs::get().login(),
			"message", message,
			"whisper", type == events::chat_handler::MESSAGE_PRIVATE,
		}
	);
}

// command handler for user :commands. Also understands all chat commands
// via inheritance. This complicates some things a bit.
class console_handler : public map_command_handler<console_handler>, private chat_command_handler
{
public:
	// convenience typedef
	typedef map_command_handler<console_handler> chmap;
	console_handler(menu_handler& menu_handler)
		: chmap()
		, chat_command_handler(menu_handler, true)
		, menu_handler_(menu_handler)
		, team_num_(menu_handler.pc_.current_side())
	{
	}

	using chmap::dispatch; // disambiguate
	using chmap::get_commands_list;
	using chmap::command_failed;

protected:
	// chat_command_handler's init_map() and handlers will end up calling these.
	// this makes sure the commands end up in our map
	virtual void register_command(const std::string& cmd,
			chat_command_handler::command_handler h,
			const std::string& help = "",
			const std::string& usage = "",
			const std::string& flags = "")
	{
		chmap::register_command(cmd, h, help, usage, flags + "N"); // add chat commands as network_only
	}

	virtual void register_alias(const std::string& to_cmd, const std::string& cmd)
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

	// these are needed to avoid ambiguities introduced by inheriting from console_command_handler
	using chmap::register_command;
	using chmap::register_alias;
	using chmap::help;
	using chmap::is_enabled;
	using chmap::command_failed_need_arg;

	void do_refresh();
	void do_droid();
	void do_terrain();
	void do_idle();
	void do_theme();
	void do_control();
	void do_controller();
	void do_clear();
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
	void do_toggle_draw_num_of_bitmaps();
	void do_toggle_whiteboard();
	void do_whiteboard_options();

	std::string get_flags_description() const
	{
		return _("(D) — debug only, (N) — network only, (A) — admin only");
	}

	using chat_command_handler::get_command_flags_description; // silence a warning
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
		      || (c.has_flag('N') && !menu_handler_.pc_.is_networked_mp())
		      || (c.has_flag('A') && !mp::logged_in_as_moderator())
		      || (c.has_flag('S') && (synced_context::get_synced_state() != synced_context::UNSYNCED || !menu_handler_.pc_.current_team().is_local())));
	}

	void print(const std::string& title, const std::string& message)
	{
		menu_handler_.add_chat_message(std::time(nullptr), title, 0, message);
	}

	void init_map()
	{
		chat_command_handler::init_map();          // grab chat_ /command handlers

		chmap::get_command("log")->flags = "";     // clear network-only flag from log
		chmap::get_command("version")->flags = ""; // clear network-only flag
		chmap::get_command("ignore")->flags = "";  // clear network-only flag
		chmap::get_command("friend")->flags = "";  // clear network-only flag
		chmap::get_command("remove")->flags = "";  // clear network-only flag

		chmap::set_cmd_prefix(":");
		chmap::set_cmd_flag(true);

		register_command("refresh", &console_handler::do_refresh, _("Refresh gui."));
		register_command("droid", &console_handler::do_droid, _("Switch a side to/from AI control."),
				// TRANSLATORS: These are the arguments accepted by the "droid" command,
				// which must be a side-number and then optionally one of "on", "off" or "full".
				// As with the command's name, "on", "off" and "full" are hardcoded, and shouldn't change in the translation.
				_("[<side> [on/off/full]]\n“on” = enable but retain vision, “full” = as if it’s controlled by another player"));
		register_command("terrain", &console_handler::do_terrain, _("Change terrain type of current hex"),
				// TRANSLATORS: [both|base|overlay] are hardcoded literal arguments and shouldn't be translated.
				_("<terrain type> [both|base|overlay]"), "DS");
		register_command("idle", &console_handler::do_idle, _("Switch a side to/from idle state."),
				// TRANSLATORS: These are the arguments accepted by the "idle" command,
				// which must be a side-number and then optionally "on" or "off".
				// As with the command's name, "on" and "off" are hardcoded, and shouldn't change in the translation.
				_("command_idle^[<side> [on/off]]"));
		register_command("theme", &console_handler::do_theme, _("Change the in-game theme."));
		register_command("control", &console_handler::do_control,
				_("Assign control of a side to a different player or observer."), _("<side> <nickname>"), "N");
		register_command("controller", &console_handler::do_controller, _("Query the controller status of a side."),
				_("<side>"));
		register_command("clear", &console_handler::do_clear, _("Clear chat history."));
		register_command("foreground", &console_handler::do_foreground, _("Debug foreground terrain."), "", "D");
		register_command(
				"layers", &console_handler::do_layers, _("Debug layers from terrain under the mouse."), "", "D");
		register_command("fps", &console_handler::do_fps, _("Display and log fps (Frames Per Second)."));
		register_command("benchmark", &console_handler::do_benchmark, _("Similar to the 'fps' command, but also forces everything to redraw instead of only things that have changed."));
		register_command("save", &console_handler::do_save, _("Save game."));
		register_alias("save", "w");
		register_command("quit", &console_handler::do_quit, _("Quit game."));
		// Note the next value is used hardcoded in the init tests.
		register_alias("quit", "q!");
		register_command("save_quit", &console_handler::do_save_quit, _("Save and quit."));
		register_alias("save_quit", "wq");
		register_command("ignore_replay_errors", &console_handler::do_ignore_replay_errors, _("Ignore replay errors."));
		register_command("nosaves", &console_handler::do_nosaves, _("Disable autosaves."));
		register_command("next_level", &console_handler::do_next_level,
				_("Advance to the next scenario, or scenario identified by 'id'"), _("<id>"), "DS");
		register_alias("next_level", "n");
		register_command("choose_level", &console_handler::do_choose_level, _("Choose next scenario"), "", "DS");
		register_alias("choose_level", "cl");
		register_command("turn", &console_handler::do_turn,
				_("Change turn number (and time of day), or increase by one if no number is specified."), _("[turn]"),
				"DS");
		register_command("turn_limit", &console_handler::do_turn_limit,
				_("Change turn limit, or turn the turn limit off if no number is specified or it’s −1."), _("[limit]"),
				"DS");
		register_command("debug", &console_handler::do_debug, _("Turn debug mode on."));
		register_command("nodebug", &console_handler::do_nodebug, _("Turn debug mode off."), "", "D");
		register_command(
				"lua", &console_handler::do_lua, _("Execute a Lua statement."), _("<command>[;<command>...]"), "DS");
		register_command(
				"unsafe_lua", &console_handler::do_unsafe_lua, _("Grant higher privileges to Lua scripts."), "", "D");
		register_command("custom", &console_handler::do_custom, _("Set the command used by the custom command hotkey"),
				_("<command>[;<command>...]"));
		register_command("give_control", &console_handler::do_control_dialog,
				_("Invoke a dialog allowing changing control of MP sides."), "", "N");
		register_command("inspect", &console_handler::do_inspect, _("Launch the gamestate inspector"), "", "D");
		register_command(
				"alias", &console_handler::do_set_alias, _("Set or show alias to a command"), _("<name>[=<command>]"));
		register_command(
				"set_var", &console_handler::do_set_var, _("Set a scenario variable."), _("<var>=<value>"), "DS");
		register_command("show_var", &console_handler::do_show_var, _("Show a scenario variable."), _("<var>"), "D");
		register_command("unit", &console_handler::do_unit,
				// TRANSLATORS: Do not translate the word "advances"; it is a hardcoded literal argument.
				_("Modify a unit variable. (Only top level keys are supported, and advances=<number>.)"),
				_("<var>=<value>"), "DS");

		// register_command("buff", &console_handler::do_buff,
		//    _("Add a trait to a unit."), "", "D");
		// register_command("unbuff", &console_handler::do_unbuff,
		//    _("Remove a trait from a unit. (Does not work yet.)"), "", "D");
		register_command("discover", &console_handler::do_discover, _("Discover all units in help."), "");
		register_command("undiscover", &console_handler::do_undiscover, _("'Undiscover' all units in help."), "");
		register_command("create", &console_handler::do_create, _("Create a unit."), _("<unit type id>"), "DS");
		register_command("fog", &console_handler::do_fog, _("Toggle fog for the current player."), "", "DS");
		register_command("shroud", &console_handler::do_shroud, _("Toggle shroud for the current player."), "", "DS");
		register_command("gold", &console_handler::do_gold, _("Give gold to the current player."), _("<amount>"), "DS");
		register_command("throw", &console_handler::do_event, _("Fire a game event."), _("<event name>"), "DS");
		register_alias("throw", "fire");
		register_command("show_coordinates", &console_handler::do_toggle_draw_coordinates,
				_("Toggle overlaying of x,y coordinates on hexes."));
		register_alias("show_coordinates", "sc");
		register_command("show_terrain_codes", &console_handler::do_toggle_draw_terrain_codes,
				_("Toggle overlaying of terrain codes on hexes."));
		register_alias("show_terrain_codes", "tc");
		register_command("show_num_of_bitmaps", &console_handler::do_toggle_draw_num_of_bitmaps,
				_("Toggle overlaying of number of bitmaps on hexes."));
		register_alias("show_num_of_bitmaps", "bn");
		register_command("whiteboard", &console_handler::do_toggle_whiteboard, _("Toggle planning mode."));
		register_alias("whiteboard", "wb");
		register_command(
				"whiteboard_options", &console_handler::do_whiteboard_options, _("Access whiteboard options dialog."));
		register_alias("whiteboard_options", "wbo");

		if(auto alias_list = prefs::get().get_alias()) {
			for(const config::attribute& a : alias_list->attribute_range()) {
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
	cfg["id"] = prefs::get().login();
	cfg["message"] = message;
	const std::time_t time = ::std::time(nullptr);
	std::stringstream ss;
	ss << time;
	cfg["time"] = ss.str();

	const int side = board().is_observer() ? 0 : gui_->viewing_side();
	if(!board().is_observer()) {
		cfg["side"] = side;
	}

	bool private_message = has_friends() && allies_only;

	if(private_message) {
		if(board().is_observer()) {
			cfg["to_sides"] = game_config::observer_team_name;
		} else {
			cfg["to_sides"] = pc_.get_teams()[gui_->viewing_team()].allied_human_teams();
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

	if(last_search_.empty())
		return;

	bool found = false;
	map_location loc = last_search_hit_;
	// If this is a location search, just center on that location.
	std::vector<std::string> args = utils::split(last_search_, ',');
	if(args.size() == 2) {
		int x, y;
		x = lexical_cast_default<int>(args[0], 0) - 1;
		y = lexical_cast_default<int>(args[1], 0) - 1;
		if(x >= 0 && x < pc_.get_map().w() && y >= 0 && y < pc_.get_map().h()) {
			loc = map_location(x, y);
			found = true;
		}
	}
	// Start scanning the game map
	if(loc.valid() == false) {
		loc = map_location(pc_.get_map().w() - 1, pc_.get_map().h() - 1);
	}

	map_location start = loc;
	while(!found) {
		// Move to the next location
		loc.x = (loc.x + 1) % pc_.get_map().w();
		if(loc.x == 0)
			loc.y = (loc.y + 1) % pc_.get_map().h();

		// Search label
		if(!gui_->shrouded(loc)) {
			const terrain_label* label = gui_->labels().get_label(loc);
			if(label) {
				std::string label_text = label->text().str();
				if(std::search(label_text.begin(), label_text.end(), last_search_.begin(), last_search_.end(),
						   utils::chars_equal_insensitive)
						!= label_text.end()) {
					found = true;
				}
			}
		}
		// Search unit name
		if(!gui_->fogged(loc)) {
			unit_map::const_iterator ui = pc_.get_units().find(loc);
			if(ui != pc_.get_units().end()) {
				const std::string name = ui->name();
				if(std::search(
						   name.begin(), name.end(), last_search_.begin(), last_search_.end(), utils::chars_equal_insensitive)
						!= name.end()) {
					if(!pc_.get_teams()[gui_->viewing_team()].is_enemy(ui->side())
							|| !ui->invisible(ui->get_location())) {
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
		gui_->scroll_to_tile(loc, game_display::ONSCREEN, false);
		gui_->highlight_hex(loc);
	} else {
		last_search_hit_ = map_location();
		// Not found, inform the player
		utils::string_map symbols;
		symbols["search"] = last_search_;
		const std::string msg = VGETTEXT("Could not find label or unit "
										 "containing the string ‘$search’.",
				symbols);
		(void) gui2::show_message("", msg, gui2::dialogs::message::auto_close);
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

void console_handler::do_refresh()
{
	image::flush_cache();
	sound::flush_cache();

	menu_handler_.gui_->create_buttons();
	menu_handler_.gui_->queue_rerender();
}

void console_handler::do_droid()
{
	// :droid [<side> [on/off/full]]
	const std::string side_s = get_arg(1);
	std::string action = get_arg(2);
	std::transform(action.begin(), action.end(), action.begin(), tolower);
	// default to the current side if empty
	const unsigned int side = side_s.empty() ? team_num_ : lexical_cast_default<unsigned int>(side_s);
	const bool is_your_turn = menu_handler_.pc_.current_side() == static_cast<int>(menu_handler_.gui_->viewing_side());

	utils::string_map symbols;
	symbols["side"] = std::to_string(side);

	if(side < 1 || side > menu_handler_.pc_.get_teams().size()) {
		command_failed(VGETTEXT("Can't droid invalid side: '$side'.", symbols));
		return;
	} else if(menu_handler_.board().get_team(side).is_network()) {
		command_failed(VGETTEXT("Can't droid networked side: '$side'.", symbols));
		return;
	} else if(menu_handler_.board().get_team(side).is_local()) {
		bool changed = false;

		const bool is_human = menu_handler_.board().get_team(side).is_human();
		const bool is_droid = menu_handler_.board().get_team(side).is_droid();
		const bool is_proxy_human = menu_handler_.board().get_team(side).is_proxy_human();
		const bool is_ai = menu_handler_.board().get_team(side).is_ai();

		if(action == "on") {
			if(is_ai && !is_your_turn) {
				command_failed(_("It is not allowed to change a side from AI to human control when it's not your turn."));
				return;
			}
			if(!is_human || !is_droid) {
				menu_handler_.board().get_team(side).make_human();
				menu_handler_.board().get_team(side).make_droid();
				changed = true;
				if(is_ai) {
					menu_handler_.pc_.send_to_wesnothd(config {"change_controller", config {"side", side, "player", prefs::get().login(), "to", side_controller::human}});
				}
				print(get_cmd(), VGETTEXT("Side '$side' controller is now controlled by: AI.", symbols));
			} else {
				print(get_cmd(), VGETTEXT("Side '$side' is already droided.", symbols));
			}
		} else if(action == "off") {
			if(is_ai && !is_your_turn) {
				command_failed(_("It is not allowed to change a side from AI to human control when it's not your turn."));
				return;
			}
			if(!is_human || !is_proxy_human) {
				menu_handler_.board().get_team(side).make_human();
				menu_handler_.board().get_team(side).make_proxy_human();
				changed = true;
				if(is_ai) {
					menu_handler_.pc_.send_to_wesnothd(config {"change_controller", config {"side", side, "player", prefs::get().login(), "to", side_controller::human}});
				}
				print(get_cmd(), VGETTEXT("Side '$side' controller is now controlled by: human.", symbols));
			} else {
				print(get_cmd(), VGETTEXT("Side '$side' is already not droided.", symbols));
			}
		} else if(action == "full") {
			if(!is_your_turn) {
				command_failed(_("It is not allowed to change a side from human to AI control when it's not your turn."));
				return;
			}
			if(!is_ai || !is_droid) {
				menu_handler_.board().get_team(side).make_ai();
				menu_handler_.board().get_team(side).make_droid();
				changed = true;
				if(is_human || is_proxy_human) {
					menu_handler_.pc_.send_to_wesnothd(config {"change_controller", config {"side", side, "player", prefs::get().login(), "to", side_controller::ai}});
				}
				print(get_cmd(), VGETTEXT("Side '$side' controller is now fully controlled by: AI.", symbols));
			} else {
				print(get_cmd(), VGETTEXT("Side '$side' is already fully AI controlled.", symbols));
			}
		} else if(action == "") {
			if(is_ai && !is_your_turn) {
				command_failed(_("It is not allowed to change a side from AI to human control when it's not your turn."));
				return;
			}
			if(is_ai || is_droid) {
				menu_handler_.board().get_team(side).make_human();
				menu_handler_.board().get_team(side).make_proxy_human();
				changed = true;
				if(is_ai) {
					menu_handler_.pc_.send_to_wesnothd(config {"change_controller", config {"side", side, "player", prefs::get().login(), "to", side_controller::human}});
				}
				print(get_cmd(), VGETTEXT("Side '$side' controller is now controlled by: human.", symbols));
			} else {
				menu_handler_.board().get_team(side).make_human();
				menu_handler_.board().get_team(side).make_droid();
				changed = true;
				if(is_ai) {
					menu_handler_.pc_.send_to_wesnothd(config {"change_controller", config {"side", side, "player", prefs::get().login(), "to", side_controller::human}});
				}
				print(get_cmd(), VGETTEXT("Side '$side' controller is now controlled by: AI.", symbols));
			}
		} else {
			print(get_cmd(), VGETTEXT("Invalid action provided for side '$side'. Valid actions are: on, off, full.", symbols));
		}

		if(team_num_ == side && changed) {
			if(playsingle_controller* psc = dynamic_cast<playsingle_controller*>(&menu_handler_.pc_)) {
				psc->set_player_type_changed();
			}
		}
	} else {
		command_failed(VGETTEXT("Side '$side' is not a human or AI player.", symbols));
		return;
	}
	menu_handler_.textbox_info_.close();
}

void console_handler::do_terrain()
{
	// :terrain [<terrain type> [both|base|overlay]]
	const std::string terrain_type = get_arg(1);
	const std::string mode_str = get_arg(2);

	const mouse_handler& mousehandler = menu_handler_.pc_.get_mouse_handler_base();
	const map_location& loc = mousehandler.get_last_hex();

	synced_context::run_and_throw("debug_terrain",
		config {
			"x", loc.wml_x(),
			"y", loc.wml_y(),
			"terrain_type", terrain_type,
			"mode_str", mode_str,
		}
	);
}

void console_handler::do_idle()
{
	// :idle [<side> [on/off]]
	const std::string side_s = get_arg(1);
	const std::string action = get_arg(2);
	// default to the current side if empty
	const unsigned int side = side_s.empty() ? team_num_ : lexical_cast_default<unsigned int>(side_s);

	if(side < 1 || side > menu_handler_.pc_.get_teams().size()) {
		utils::string_map symbols;
		symbols["side"] = side_s;
		command_failed(VGETTEXT("Can't idle invalid side: '$side'.", symbols));
		return;
	} else if(menu_handler_.board().get_team(side).is_network()) {
		utils::string_map symbols;
		symbols["side"] = std::to_string(side);
		command_failed(VGETTEXT("Can't idle networked side: '$side'.", symbols));
		return;
	} else if(menu_handler_.board().get_team(side).is_local_ai()) {
		utils::string_map symbols;
		symbols["side"] = std::to_string(side);
		command_failed(VGETTEXT("Can't idle local ai side: '$side'.", symbols));
		return;
	} else if(menu_handler_.board().get_team(side).is_local_human()) {
		if(menu_handler_.board().get_team(side).is_idle() ? action == " on" : action == " off") {
			return;
		}
		// toggle the proxy controller between idle / non idle
		menu_handler_.board().get_team(side).toggle_idle();
		if(team_num_ == side) {
			if(playsingle_controller* psc = dynamic_cast<playsingle_controller*>(&menu_handler_.pc_)) {
				psc->set_player_type_changed();
			}
		}
	}
	menu_handler_.textbox_info_.close();
}

void console_handler::do_theme()
{
	prefs::get().show_theme_dialog();
}

struct save_id_matches
{
	save_id_matches(const std::string& save_id)
		: save_id_(save_id)
	{
	}

	bool operator()(const team& t) const
	{
		return t.save_id() == save_id_;
	}

	std::string save_id_;
};

void console_handler::do_control()
{
	// :control <side> <nick>
	if(!menu_handler_.pc_.is_networked_mp()) {
		return;
	}

	const std::string side = get_arg(1);
	const std::string player = get_arg(2);
	if(player.empty()) {
		command_failed_need_arg(2);
		return;
	}

	unsigned int side_num;
	try {
		side_num = lexical_cast<unsigned int>(side);
	} catch(const bad_lexical_cast&) {
		const auto& teams = menu_handler_.pc_.get_teams();
		const auto it_t = std::find_if(teams.begin(), teams.end(), save_id_matches(side));

		if(it_t == teams.end()) {
			utils::string_map symbols;
			symbols["side"] = side;
			command_failed(VGETTEXT("Can't change control of invalid side: '$side'.", symbols));
			return;
		} else {
			side_num = it_t->side();
		}
	}

	if(side_num < 1 || side_num > menu_handler_.pc_.get_teams().size()) {
		utils::string_map symbols;
		symbols["side"] = side;
		command_failed(VGETTEXT("Can't change control of out-of-bounds side: '$side'.", symbols));
		return;
	}

	menu_handler_.request_control_change(side_num, player);
	menu_handler_.textbox_info_.close();
}

void console_handler::do_controller()
{
	const std::string side = get_arg(1);
	unsigned int side_num;
	try {
		side_num = lexical_cast<unsigned int>(side);
	} catch(const bad_lexical_cast&) {
		utils::string_map symbols;
		symbols["side"] = side;
		command_failed(VGETTEXT("Can't query control of invalid side: '$side'.", symbols));
		return;
	}

	if(side_num < 1 || side_num > menu_handler_.pc_.get_teams().size()) {
		utils::string_map symbols;
		symbols["side"] = side;
		command_failed(VGETTEXT("Can't query control of out-of-bounds side: '$side'.", symbols));
		return;
	}

	std::string report = side_controller::get_string(menu_handler_.board().get_team(side_num).controller());
	if(!menu_handler_.board().get_team(side_num).is_proxy_human()) {
		report += " (" + side_proxy_controller::get_string(menu_handler_.board().get_team(side_num).proxy_controller()) + ")";
	}

	if(menu_handler_.board().get_team(side_num).is_network()) {
		report += " (networked)";
	}

	print(get_cmd(), report);
}

void console_handler::do_clear()
{
	menu_handler_.gui_->get_chat_manager().clear_chat_messages();
}

void console_handler::do_foreground()
{
	menu_handler_.gui_->toggle_debug_flag(display::DEBUG_FOREGROUND);
	menu_handler_.gui_->invalidate_all();
}

void console_handler::do_layers()
{
	display& disp = *(menu_handler_.gui_);

	const mouse_handler& mousehandler = menu_handler_.pc_.get_mouse_handler_base();
	const map_location& loc = mousehandler.get_last_hex();

	//
	// It's possible to invoke this dialog even if loc isn't a valid hex. I'm not sure
	// exactly how that happens, but it does seem to occur when moving the mouse outside
	// the window to the menu bar. Not sure if there's a bug when the set-last-hex code
	// in that case, but this check at least ensures the dialog is only ever shown for a
	// valid, on-map location. Otherwise, an assertion gets thrown.
	//
	// -- vultraz, 2017-09-21
	//
	if(disp.get_map().on_board_with_border(loc)) {
		gui2::dialogs::terrain_layers::display(disp, loc);
	}
}

void console_handler::do_fps()
{
	prefs::get().set_show_fps(!prefs::get().show_fps());
}

void console_handler::do_benchmark()
{
	menu_handler_.gui_->toggle_debug_flag(display::DEBUG_BENCHMARK);
}

void console_handler::do_save()
{
	menu_handler_.pc_.do_consolesave(get_data());
}

void console_handler::do_save_quit()
{
	do_save();
	do_quit();
}

void console_handler::do_quit()
{
	throw_quit_game_exception();
}

void console_handler::do_ignore_replay_errors()
{
	game_config::ignore_replay_errors = (get_data() != "off") ? true : false;
}

void console_handler::do_nosaves()
{
	game_config::disable_autosave = (get_data() != "off") ? true : false;
}

void console_handler::do_next_level()
{
	synced_context::run_and_throw("debug_next_level", config {"next_level", get_data()});
}

void console_handler::do_choose_level()
{
	std::string tag = menu_handler_.pc_.get_classification().get_tagname();
	std::vector<std::string> options;
	std::string next;
	if(tag != "multiplayer") {
		for(const config& sc : menu_handler_.game_config_.child_range(tag)) {
			const std::string& id = sc["id"];
			options.push_back(id);
			if(id == menu_handler_.gamedata().next_scenario()) {
				next = id;
			}
		}
	} else {
		// find scenarios of multiplayer campaigns
		// (assumes that scenarios are ordered properly in the game_config)
		std::string scenario_id = menu_handler_.pc_.get_mp_settings().mp_scenario;
		if(auto this_scenario = menu_handler_.game_config_.find_child(tag, "id", scenario_id)) {
			std::string addon_id = this_scenario["addon_id"].str();
			for(const config& sc : menu_handler_.game_config_.child_range(tag)) {
				if(sc["addon_id"] == addon_id) {
					std::string id = sc["id"];
					options.push_back(id);
					if(id == menu_handler_.gamedata().next_scenario()) {
						next = id;
					}
				}
			}
		}
	}
	std::sort(options.begin(), options.end());
	int choice = std::distance(options.begin(), std::lower_bound(options.begin(), options.end(), next));
	{
		gui2::dialogs::simple_item_selector dlg(_("Choose Scenario (Debug!)"), "", options);
		dlg.set_selected_index(choice);
		dlg.show();
		choice = dlg.selected_index();
	}

	if(choice == -1) {
		return;
	}

	if(std::size_t(choice) < options.size()) {
		synced_context::run_and_throw("debug_next_level", config {"next_level", options[choice]});
	}
}

void console_handler::do_turn()
{
	tod_manager& tod_man = menu_handler_.gamestate().tod_manager_;

	int turn = tod_man.turn() + 1;
	const std::string& data = get_data();
	if(!data.empty()) {
		turn = lexical_cast_default<int>(data, 1);
	}
	synced_context::run_and_throw("debug_turn", config {"turn", turn});
}

void console_handler::do_turn_limit()
{
	int limit = get_data().empty() ? -1 : lexical_cast_default<int>(get_data(), 1);
	synced_context::run_and_throw("debug_turn_limit", config {"turn_limit", limit});
}

void console_handler::do_debug()
{
	if(!menu_handler_.pc_.is_networked_mp() || game_config::mp_debug) {
		print(get_cmd(), _("Debug mode activated!"));
		game_config::set_debug(true);
	} else {
		command_failed(_("Debug mode not available in network games"));
	}
}

void console_handler::do_nodebug()
{
	if(game_config::debug) {
		print(get_cmd(), _("Debug mode deactivated!"));
		game_config::set_debug(false);
	}
}

void console_handler::do_lua()
{
	if(!menu_handler_.gamestate().lua_kernel_) {
		return;
	}

	synced_context::run_and_throw("debug_lua", config {"code", get_data()});
}

void console_handler::do_unsafe_lua()
{
	if(!menu_handler_.gamestate().lua_kernel_) {
		return;
	}

	const int retval = gui2::show_message(_("WARNING! Unsafe Lua Mode"),
		_("Executing Lua code in in this manner opens your computer to potential security breaches from any "
		"malicious add-ons or other programs you may have installed.\n\n"
		"Do not continue unless you really know what you are doing."), gui2::dialogs::message::ok_cancel_buttons);

	if(retval == gui2::retval::OK) {
		print(get_cmd(), _("Unsafe mode enabled!"));
		menu_handler_.gamestate().lua_kernel_->load_package();
	}
}

void console_handler::do_custom()
{
	prefs::get().set_custom_command(get_data());
}

void console_handler::do_set_alias()
{
	const std::string data = get_data();
	const std::string::const_iterator j = std::find(data.begin(), data.end(), '=');
	const std::string alias(data.begin(), j);
	if(j != data.end()) {
		const std::string command(j + 1, data.end());
		if(!command.empty()) {
			register_alias(command, alias);
		} else {
			// "alias something=" deactivate this alias. We just set it
			// equal to itself here. Later preferences will filter empty alias.
			register_alias(alias, alias);
		}
		prefs::get().add_alias(alias, command);
		// directly save it for the moment, but will slow commands sequence
		prefs::get().write_preferences();
	} else {
		// "alias something" display its value
		// if no alias, will be "'something' = 'something'"
		const std::string command = chmap::get_actual_cmd(alias);
		print(get_cmd(), "'" + alias + "'" + " = " + "'" + command + "'");
	}
}

void console_handler::do_set_var()
{
	const std::string data = get_data();
	if(data.empty()) {
		command_failed_need_arg(1);
		return;
	}

	const std::string::const_iterator j = std::find(data.begin(), data.end(), '=');
	if(j != data.end()) {
		const std::string name(data.begin(), j);
		const std::string value(j + 1, data.end());
		synced_context::run_and_throw("debug_set_var", config {"name", name, "value", value});
	} else {
		command_failed(_("Variable not found"));
	}
}

void console_handler::do_show_var()
{
	gui2::show_transient_message("", menu_handler_.gamedata().get_variable_const(get_data()));
}

void console_handler::do_inspect()
{
	vconfig cfg = vconfig::empty_vconfig();
	gui2::dialogs::gamestate_inspector::display(
		menu_handler_.gamedata().get_variables(), *resources::game_events, menu_handler_.board());
}

void console_handler::do_control_dialog()
{
	gui2::dialogs::mp_change_control::display(menu_handler_);
}

void console_handler::do_unit()
{
	// prevent SIGSEGV due to attempt to set HP during a fight
	if(events::commands_disabled > 0) {
		return;
	}

	unit_map::iterator i = menu_handler_.current_unit();
	if(i == menu_handler_.pc_.get_units().end()) {
		utils::string_map symbols;
		symbols["unit"] = get_arg(1);
		command_failed(VGETTEXT(
			"Debug command 'unit: $unit' failed: no unit selected or hovered over.",
			symbols));
		return;
	}

	const map_location loc = i->get_location();
	const std::string data = get_data(1);
	std::vector<std::string> parameters = utils::split(data, '=', utils::STRIP_SPACES);
	if(parameters.size() < 2) {
		return;
	}

	if(parameters[0] == "alignment") {
		auto alignment = unit_alignments::get_enum(parameters[1]);
		if(!alignment) {
			utils::string_map symbols;
			symbols["alignment"] = get_arg(1);
			command_failed(VGETTEXT(
					"Invalid alignment: '$alignment', needs to be one of lawful, neutral, chaotic, or liminal.",
					symbols));
			return;
		}
	}

	synced_context::run_and_throw("debug_unit",
		config {
			"x", loc.wml_x(),
			"y", loc.wml_y(),
			"name", parameters[0],
			"value", parameters[1],
		}
	);
}

void console_handler::do_discover()
{
	for(const unit_type_data::unit_type_map::value_type& i : unit_types.types()) {
		prefs::get().encountered_units().insert(i.second.id());
	}
}

void console_handler::do_undiscover()
{
	const int res = gui2::show_message("Undiscover",
			_("Do you wish to clear all of your discovered units from help?"), gui2::dialogs::message::yes_no_buttons);
	if(res != gui2::retval::CANCEL) {
		prefs::get().encountered_units().clear();
	}
}

/** Implements the (debug mode) console command that creates a unit. */
void console_handler::do_create()
{
	const mouse_handler& mousehandler = menu_handler_.pc_.get_mouse_handler_base();
	const map_location& loc = mousehandler.get_last_hex();
	if(menu_handler_.pc_.get_map().on_board(loc)) {
		const unit_type* ut = unit_types.find(get_data());
		if(!ut) {
			command_failed(_("Invalid unit type"));
			return;
		}

		// Create the unit.
		create_and_place(*menu_handler_.gui_, menu_handler_.pc_.get_map(), menu_handler_.pc_.get_units(), loc, *ut);
	} else {
		command_failed(_("Invalid location"));
	}
}

void console_handler::do_fog()
{
	synced_context::run_and_throw("debug_fog", config());
}

void console_handler::do_shroud()
{
	synced_context::run_and_throw("debug_shroud", config());
}

void console_handler::do_gold()
{
	synced_context::run_and_throw("debug_gold", config {"gold", lexical_cast_default<int>(get_data(), 1000)});
}

void console_handler::do_event()
{
	synced_context::run_and_throw("debug_event", config {"eventname", get_data()});
}

void console_handler::do_toggle_draw_coordinates()
{
	menu_handler_.gui_->toggle_debug_flag(display::DEBUG_COORDINATES);
	menu_handler_.gui_->invalidate_all();
}
void console_handler::do_toggle_draw_terrain_codes()
{
	menu_handler_.gui_->toggle_debug_flag(display::DEBUG_TERRAIN_CODES);
	menu_handler_.gui_->invalidate_all();
}

void console_handler::do_toggle_draw_num_of_bitmaps()
{
	menu_handler_.gui_->toggle_debug_flag(display::DEBUG_NUM_BITMAPS);
	menu_handler_.gui_->invalidate_all();
}

void console_handler::do_toggle_whiteboard()
{
	if(const std::shared_ptr<wb::manager>& whiteb = menu_handler_.pc_.get_whiteboard()) {
		whiteb->set_active(!whiteb->is_active());
		if(whiteb->is_active()) {
			print(get_cmd(), _("Planning mode activated!"));
			whiteb->print_help_once();
		} else {
			print(get_cmd(), _("Planning mode deactivated!"));
		}
	}
}

void console_handler::do_whiteboard_options()
{
	if(menu_handler_.pc_.get_whiteboard()) {
		menu_handler_.pc_.get_whiteboard()->options_dlg();
	}
}

void menu_handler::do_ai_formula(const std::string& str, int side_num, mouse_handler& /*mousehandler*/)
{
	try {
		add_chat_message(std::time(nullptr), "wfl", 0, ai::manager::get_singleton().evaluate_command(side_num, str));
	} catch(const wfl::formula_error&) {
	} catch(...) {
		add_chat_message(std::time(nullptr), "wfl", 0, "UNKNOWN ERROR IN FORMULA: "+utils::get_unknown_exception_type());
	}
}

void menu_handler::user_command()
{
	textbox_info_.show(gui::TEXTBOX_COMMAND, translation::sgettext("prompt^Command:"), "", false, *gui_);
}

void menu_handler::request_control_change(int side_num, const std::string& player)
{
	std::string side = std::to_string(side_num);
	if(board().get_team(side_num).is_local_human() && player == prefs::get().login()) {
		// this is already our side.
		return;
	} else {
		// The server will (or won't because we aren't allowed to change the controller)
		// send us a [change_controller] back, which we then handle in playturn.cpp
		pc_.send_to_wesnothd(config {"change_controller", config {"side", side, "player", player}});
	}
}

void menu_handler::custom_command()
{
	for(const std::string& command : utils::split(prefs::get().custom_command(), ';')) {
		do_command(command);
	}
}

void menu_handler::ai_formula()
{
	if(!pc_.is_networked_mp()) {
		textbox_info_.show(gui::TEXTBOX_AI, translation::sgettext("prompt^Command:"), "", false, *gui_);
	}
}

void menu_handler::clear_messages()
{
	gui_->get_chat_manager().clear_chat_messages(); // also clear debug-messages and WML-error-messages
}

void menu_handler::send_to_server(const config& cfg)
{
	pc_.send_to_wesnothd(cfg);
}

} // end namespace events
