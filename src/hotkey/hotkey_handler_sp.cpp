/*
	Copyright (C) 2014 - 2024
	by Chris Beck <render787@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "hotkey/hotkey_handler_sp.hpp"

#include "filesystem.hpp" // for get_saves_dir()
#include "font/standard_colors.hpp"
#include "formula/string_utils.hpp"
#include "gui/dialogs/message.hpp"
#include "hotkey/hotkey_command.hpp"
#include "hotkey/hotkey_item.hpp"
#include "map/label.hpp"
#include "play_controller.hpp"
#include "playsingle_controller.hpp"
#include "whiteboard/manager.hpp"
#include "game_events/menu_item.hpp"
#include "game_events/wmi_manager.hpp"
#include "map/map.hpp"
#include "save_index.hpp"
#include "saved_game.hpp"
#include "resources.hpp"
#include "replay.hpp"

#include "units/unit.hpp"

#include <boost/algorithm/string/predicate.hpp>

namespace balg = boost::algorithm;

playsingle_controller::hotkey_handler::hotkey_handler(playsingle_controller & pc, saved_game & sg)
	: play_controller::hotkey_handler(pc, sg)
	, playsingle_controller_(pc)
	, whiteboard_manager_(pc.get_whiteboard())
{}

playsingle_controller::hotkey_handler::~hotkey_handler(){}

bool playsingle_controller::hotkey_handler::is_observer() const { return playsingle_controller_.is_observer(); }

void playsingle_controller::hotkey_handler::recruit(){
	if (!browse())
		menu_handler_.recruit(play_controller_.current_side(), mouse_handler_.get_last_hex());
	else if (whiteboard_manager_->is_active())
		menu_handler_.recruit(gui()->viewing_team().side(), mouse_handler_.get_last_hex());
}

void playsingle_controller::hotkey_handler::repeat_recruit(){
	if (!browse())
		menu_handler_.repeat_recruit(play_controller_.current_side(), mouse_handler_.get_last_hex());
	else if (whiteboard_manager_->is_active())
		menu_handler_.repeat_recruit(gui()->viewing_team().side(), mouse_handler_.get_last_hex());
}

void playsingle_controller::hotkey_handler::recall(){
	if (!browse())
		menu_handler_.recall(play_controller_.current_side(), mouse_handler_.get_last_hex());
	else if (whiteboard_manager_->is_active())
		menu_handler_.recall(gui()->viewing_team().side(), mouse_handler_.get_last_hex());
}

void playsingle_controller::hotkey_handler::toggle_shroud_updates(){
	menu_handler_.toggle_shroud_updates(gui()->viewing_team().side());
}

void playsingle_controller::hotkey_handler::update_shroud_now(){
	menu_handler_.update_shroud_now(gui()->viewing_team().side());
}

void playsingle_controller::hotkey_handler::end_turn(){
	playsingle_controller_.end_turn();
}

void playsingle_controller::hotkey_handler::rename_unit(){
	menu_handler_.rename_unit();
}

void playsingle_controller::hotkey_handler::create_unit(){
	menu_handler_.create_unit(mouse_handler_);
}

void playsingle_controller::hotkey_handler::change_side(){
	menu_handler_.change_side(mouse_handler_);
}

void playsingle_controller::hotkey_handler::kill_unit(){
	menu_handler_.kill_unit(mouse_handler_);
}

void playsingle_controller::hotkey_handler::select_teleport(){
	mouse_handler_.select_teleport();
}

void playsingle_controller::hotkey_handler::label_terrain(bool team_only){
	menu_handler_.label_terrain(mouse_handler_, team_only);
}

void playsingle_controller::hotkey_handler::clear_labels(){
	menu_handler_.clear_labels();
}

void playsingle_controller::hotkey_handler::continue_move(){
	menu_handler_.continue_move(mouse_handler_, play_controller_.current_side());
}

void playsingle_controller::hotkey_handler::unit_hold_position(){
	if (!browse())
		menu_handler_.unit_hold_position(mouse_handler_, play_controller_.current_side());
}

void playsingle_controller::hotkey_handler::end_unit_turn(){
	if (!browse())
		menu_handler_.end_unit_turn(mouse_handler_, play_controller_.current_side());
}

void playsingle_controller::hotkey_handler::user_command(){
	menu_handler_.user_command();
}

void playsingle_controller::hotkey_handler::custom_command(){
	menu_handler_.custom_command();
}

void playsingle_controller::hotkey_handler::ai_formula(){
	menu_handler_.ai_formula();
}

void playsingle_controller::hotkey_handler::clear_messages(){
	menu_handler_.clear_messages();
}

void playsingle_controller::hotkey_handler::label_settings(){
	menu_handler_.label_settings();
}

void playsingle_controller::hotkey_handler::whiteboard_toggle() {
	whiteboard_manager_->set_active(!whiteboard_manager_->is_active());

	if (whiteboard_manager_->is_active()) {
		std::string hk = hotkey::get_names(hotkey::hotkey_command::get_command_by_command(hotkey::HOTKEY_WB_TOGGLE).id);
		utils::string_map symbols;
		symbols["hotkey"] = hk;

		gui()->announce(_("Planning mode activated!") + std::string("\n") + VGETTEXT("(press $hotkey to deactivate)", symbols), font::NORMAL_COLOR);
	} else {
		gui()->announce(_("Planning mode deactivated!"), font::NORMAL_COLOR);
	}
	//@todo Stop printing whiteboard help in the chat once we have better documentation/help
	whiteboard_manager_->print_help_once();
}

void playsingle_controller::hotkey_handler::whiteboard_execute_action(){
	whiteboard_manager_->contextual_execute();
}

void playsingle_controller::hotkey_handler::whiteboard_execute_all_actions(){
	whiteboard_manager_->execute_all_actions();
}

void playsingle_controller::hotkey_handler::whiteboard_delete_action(){
	whiteboard_manager_->contextual_delete();
}

void playsingle_controller::hotkey_handler::whiteboard_bump_up_action()
{
	whiteboard_manager_->contextual_bump_up_action();
}

void playsingle_controller::hotkey_handler::whiteboard_bump_down_action()
{
	whiteboard_manager_->contextual_bump_down_action();
}

void playsingle_controller::hotkey_handler::whiteboard_suppose_dead()
{
	const unit* curr_unit;
	map_location loc;
	{ wb::future_map future; //start planned unit map scope
		curr_unit = &*menu_handler_.current_unit();
		loc = curr_unit->get_location();
	} // end planned unit map scope
	whiteboard_manager_->save_suppose_dead(*curr_unit,loc);
}

hotkey::ACTION_STATE playsingle_controller::hotkey_handler::get_action_state(const hotkey::ui_command& cmd) const
{
	switch(cmd.hotkey_command) {
	case hotkey::HOTKEY_WB_TOGGLE:
		return whiteboard_manager_->is_active() ? hotkey::ACTION_ON : hotkey::ACTION_OFF;
	default:
		return play_controller::hotkey_handler::get_action_state(cmd);
	}
}

bool playsingle_controller::hotkey_handler::can_execute_command(const hotkey::ui_command& cmd) const
{
	hotkey::HOTKEY_COMMAND command = cmd.hotkey_command;
	bool res = true;
	int prefixlen = wml_menu_hotkey_prefix.length();
	switch (command){
		case hotkey::HOTKEY_NULL:
		case hotkey::HOTKEY_WML:
		{
			if(cmd.id.compare(0, prefixlen, wml_menu_hotkey_prefix) == 0) {
				game_events::wmi_manager::item_ptr item = gamestate().get_wml_menu_items().get_item(std::string(cmd.id.substr(prefixlen)));
				if(!item) {
					return false;
				}
				return !item->is_synced() || play_controller_.can_use_synced_wml_menu();
			}
			return play_controller::hotkey_handler::can_execute_command(cmd);

		}
		case hotkey::HOTKEY_SAVE_GAME:
			return !events::commands_disabled || (playsingle_controller_.is_replay() && events::commands_disabled <  2);
		case hotkey::HOTKEY_UNIT_HOLD_POSITION:
		case hotkey::HOTKEY_END_UNIT_TURN:
			return !browse() && !linger() && !events::commands_disabled;
		case hotkey::HOTKEY_RECRUIT:
		case hotkey::HOTKEY_REPEAT_RECRUIT:
		case hotkey::HOTKEY_RECALL:
			return (!browse() || whiteboard_manager_->is_active()) && !linger() && !events::commands_disabled;
		case hotkey::HOTKEY_ENDTURN:
			//playmp_controller::hotkey_handler checks whether we are the host.
			return (!browse() || linger()) && !events::commands_disabled;

		case hotkey::HOTKEY_DELAY_SHROUD:
			return !linger()
				&& (gui()->viewing_team().uses_fog() || gui()->viewing_team().uses_shroud())
				&& gui()->viewing_team_is_playing()
				&& gui()->viewing_team().is_local_human()
				&& !events::commands_disabled;
		case hotkey::HOTKEY_UPDATE_SHROUD:
			return !linger()
				&& gui()->viewing_team_is_playing()
				&& gui()->viewing_team().is_local_human()
				&& !events::commands_disabled
				&& gui()->viewing_team().auto_shroud_updates() == false;

		// Commands we can only do if in debug mode
		case hotkey::HOTKEY_CREATE_UNIT:
		case hotkey::HOTKEY_CHANGE_SIDE:
		case hotkey::HOTKEY_KILL_UNIT:
		case hotkey::HOTKEY_TELEPORT_UNIT:
			return !events::commands_disabled && game_config::debug && play_controller_.get_map().on_board(mouse_handler_.get_last_hex()) && play_controller_.current_team().is_local();

		case hotkey::HOTKEY_CLEAR_LABELS:
			res = !is_observer();
			break;
		case hotkey::HOTKEY_LABEL_TEAM_TERRAIN:
		case hotkey::HOTKEY_LABEL_TERRAIN: {
			const terrain_label *label = gui()->labels().get_label(mouse_handler_.get_last_hex());
			res = !events::commands_disabled && play_controller_.get_map().on_board(mouse_handler_.get_last_hex())
				&& !gui()->shrouded(mouse_handler_.get_last_hex())
				&& !is_observer()
				&& (!label || !label->immutable());
			break;
		}
		case hotkey::HOTKEY_CONTINUE_MOVE: {
			if(browse() || events::commands_disabled)
				return false;

			if( (menu_handler_.current_unit().valid())
				&& (menu_handler_.current_unit()->move_interrupted()))
				return true;
			const unit_map::const_iterator i = play_controller_.get_units().find(mouse_handler_.get_selected_hex());
			if (!i.valid()) return false;
			return i->move_interrupted();
		}
		case hotkey::HOTKEY_WB_TOGGLE:
			return !is_observer();
		case hotkey::HOTKEY_WB_EXECUTE_ACTION:
		case hotkey::HOTKEY_WB_EXECUTE_ALL_ACTIONS:
			return whiteboard_manager_->can_enable_execution_hotkeys() && !events::commands_disabled && !browse();
		case hotkey::HOTKEY_WB_DELETE_ACTION:
			return whiteboard_manager_->can_enable_modifier_hotkeys();
		case hotkey::HOTKEY_WB_BUMP_UP_ACTION:
		case hotkey::HOTKEY_WB_BUMP_DOWN_ACTION:
			return whiteboard_manager_->can_enable_reorder_hotkeys();
		case hotkey::HOTKEY_WB_SUPPOSE_DEAD:
		{
			//@todo re-enable this once we figure out a decent UI for suppose_dead
			//@todo when re-enabling this, change 'true' to 'false' in master_hotkey_list for this hotkey
			return false;
		}

		case hotkey::HOTKEY_REPLAY_STOP:
		case hotkey::HOTKEY_REPLAY_PLAY:
		case hotkey::HOTKEY_REPLAY_NEXT_TURN:
		case hotkey::HOTKEY_REPLAY_NEXT_SIDE:
		case hotkey::HOTKEY_REPLAY_NEXT_MOVE:
		case hotkey::HOTKEY_REPLAY_SKIP_ANIMATION:
		case hotkey::HOTKEY_REPLAY_SHOW_EVERYTHING:
		case hotkey::HOTKEY_REPLAY_SHOW_EACH:
		case hotkey::HOTKEY_REPLAY_SHOW_TEAM1:
		case hotkey::HOTKEY_REPLAY_RESET:
			return playsingle_controller_.get_replay_controller() && playsingle_controller_.get_replay_controller()->can_execute_command(cmd);
		case hotkey::HOTKEY_REPLAY_EXIT:
			return playsingle_controller_.is_replay() && (!playsingle_controller_.is_networked_mp() || resources::recorder->at_end());
		default:
			return play_controller::hotkey_handler::can_execute_command(cmd);
	}
	return res;
}

void playsingle_controller::hotkey_handler::load_autosave(const std::string& filename, bool start_replay)
{
	if(!start_replay) {
		play_controller::hotkey_handler::load_autosave(filename);
	}
	auto invalid_save_file = [this, filename](const std::string& msg){
		if(playsingle_controller_.is_networked_mp()) {
			gui2::show_error_message(msg);
		} else {
			const int res = gui2::show_message("", msg + _("Do you want to load it anyway?"), gui2::dialogs::message::yes_no_buttons);
			if(res != gui2::retval::CANCEL) {
				play_controller::hotkey_handler::load_autosave(filename);
			}
		}
	};

	config savegame;
	std::string error_log;
	savegame::read_save_file(filesystem::get_saves_dir(), filename, savegame, &error_log);

	if(!error_log.empty()) {
		invalid_save_file(_("The file you have tried to load is corrupt: '") + error_log);
		return;
	}
	if(savegame.child_or_empty("snapshot")["replay_pos"].to_int(-1) < 0 ) {
		invalid_save_file(_("The file you have tried to load has no replay information. "));
		return;
	}
	if(!playsingle_controller_.get_saved_game().get_replay().is_ancestor(savegame.child_or_empty("replay"))) {
		invalid_save_file(_("The file you have tried to load is not from the current session."));
		return;
	}

	auto res = std::make_shared<config>(savegame.child_or_empty("snapshot"));
	auto stats = std::make_shared<config>(savegame.child_or_empty("statistics"));
	throw reset_gamestate_exception(res, stats, false);
}

void playsingle_controller::hotkey_handler::replay_exit()
{
	if(!playsingle_controller_.is_networked_mp()) {
		resources::recorder->delete_upcoming_commands();
	}
	playsingle_controller_.set_player_type_changed();
}
