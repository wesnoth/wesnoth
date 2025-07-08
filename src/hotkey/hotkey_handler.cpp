/*
	Copyright (C) 2014 - 2025
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

#include "hotkey/hotkey_handler.hpp"

#include "font/standard_colors.hpp"
#include "formula/string_utils.hpp"
#include "game_display.hpp"
#include "game_events/wmi_manager.hpp"
#include "game_state.hpp"
#include "hotkey/hotkey_command.hpp"
#include "hotkey/hotkey_item.hpp"
#include "log.hpp"
#include "map/map.hpp"
#include "play_controller.hpp"
#include "preferences/preferences.hpp"
#include "savegame.hpp"
#include "units/unit.hpp"
#include "whiteboard/manager.hpp"

#include <boost/algorithm/string/predicate.hpp>

namespace balg = boost::algorithm;

#define ERR_G  LOG_STREAM(err,   lg::general())
#define WRN_G  LOG_STREAM(warn,   lg::general())
#define LOG_G  LOG_STREAM(info,  lg::general())
#define DBG_G  LOG_STREAM(debug, lg::general())

const std::string play_controller::hotkey_handler::wml_menu_hotkey_prefix = "wml_menu:";

static const std::string quickload_prefix = "quickload:";
static const std::string quickreplay_prefix = "quickreplay:";

play_controller::hotkey_handler::hotkey_handler(play_controller & pc, saved_game & sg)
	: play_controller_(pc)
	, menu_handler_(pc.get_menu_handler())
	, mouse_handler_(pc.get_mouse_handler_base())
	, saved_game_(sg)
{}

play_controller::hotkey_handler::~hotkey_handler() {}

game_display * play_controller::hotkey_handler::gui() const {
	return &play_controller_.get_display();
}

game_state & play_controller::hotkey_handler::gamestate() {
	return play_controller_.gamestate();
}

const game_state & play_controller::hotkey_handler::gamestate() const {
	return play_controller_.gamestate();
}

bool play_controller::hotkey_handler::browse() const { return play_controller_.is_browsing(); }
bool play_controller::hotkey_handler::linger() const { return play_controller_.is_linger_mode(); }

void play_controller::hotkey_handler::objectives() {
	menu_handler_.objectives();
}

void play_controller::hotkey_handler::show_statistics() {
	menu_handler_.show_statistics(gui()->viewing_team().side());
}

void play_controller::hotkey_handler::unit_list() {
	menu_handler_.unit_list();
}

void play_controller::hotkey_handler::status_table() {
	menu_handler_.status_table();
}

void play_controller::hotkey_handler::save_game() {
	play_controller_.save_game();
}

void play_controller::hotkey_handler::save_replay() {
	play_controller_.save_replay();
}

void play_controller::hotkey_handler::save_map() {
	play_controller_.save_map();
}

void play_controller::hotkey_handler::load_game() {
	play_controller_.load_game();
}

void play_controller::hotkey_handler::preferences() {
	menu_handler_.preferences();
}

void play_controller::hotkey_handler::select_and_action() {
	mouse_handler_.select_or_action(browse());
}

void play_controller::hotkey_handler::touch_hex() {
	auto touched_hex = gui()->mouseover_hex();
	mouse_handler_.touch_action(touched_hex, false);
}

void play_controller::hotkey_handler::move_action() {
	mouse_handler_.move_action(browse());
}

void play_controller::hotkey_handler::deselect_hex() {
	mouse_handler_.deselect_hex();
}
void play_controller::hotkey_handler::select_hex() {
	mouse_handler_.select_hex(gui()->mouseover_hex(), false);
}

void play_controller::hotkey_handler::cycle_units() {
	mouse_handler_.cycle_units(browse());
}

void play_controller::hotkey_handler::cycle_back_units() {
	mouse_handler_.cycle_back_units(browse());
}

void play_controller::hotkey_handler::speak() {
	menu_handler_.speak();
}

void play_controller::hotkey_handler::show_chat_log() {
	menu_handler_.show_chat_log();
}

void play_controller::hotkey_handler::show_help() {
	menu_handler_.show_help();
}

void play_controller::hotkey_handler::undo() {
	play_controller_.undo();
}

void play_controller::hotkey_handler::redo() {
	play_controller_.redo();
}

void play_controller::hotkey_handler::show_enemy_moves(bool ignore_units){
	menu_handler_.show_enemy_moves(ignore_units, play_controller_.current_side());
}

void play_controller::hotkey_handler::goto_leader() {
	menu_handler_.goto_leader(play_controller_.current_side());
}

void play_controller::hotkey_handler::unit_description() {
	menu_handler_.unit_description();
}

void play_controller::hotkey_handler::terrain_description() {
	menu_handler_.terrain_description(mouse_handler_);
}

void play_controller::hotkey_handler::toggle_ellipses() {
	menu_handler_.toggle_ellipses();
}

void play_controller::hotkey_handler::toggle_grid() {
	menu_handler_.toggle_grid();
}

void play_controller::hotkey_handler::search() {
	menu_handler_.search();
}

void play_controller::hotkey_handler::toggle_accelerated_speed()
{
	prefs::get().set_turbo(!prefs::get().turbo());

	display::announce_options ao;
	ao.discard_previous = true;

	if(prefs::get().turbo()) {
		utils::string_map symbols;
		symbols["hk"] = hotkey::get_names(hotkey::get_hotkey_command(hotkey::HOTKEY_ACCELERATED).id);
		gui()->announce(_("Accelerated speed enabled!") + "\n" + VGETTEXT("(press $hk to disable)", symbols), font::NORMAL_COLOR, ao);
	} else {
		gui()->announce(_("Accelerated speed disabled!"), font::NORMAL_COLOR, ao);
	}
}

void play_controller::hotkey_handler::scroll_up(bool on)
{
	play_controller_.set_scroll_up(on);
}

void play_controller::hotkey_handler::scroll_down(bool on)
{
	play_controller_.set_scroll_down(on);
}

void play_controller::hotkey_handler::scroll_left(bool on)
{
	play_controller_.set_scroll_left(on);
}

void play_controller::hotkey_handler::scroll_right(bool on)
{
	play_controller_.set_scroll_right(on);
}

bool play_controller::hotkey_handler::do_execute_command(const hotkey::ui_command& cmd, bool press, bool release)
{
	DBG_G << "play_controller::do_execute_command: Found command:" << cmd.id;

	// TODO c++20: Use string::starts_with
	if(balg::starts_with(cmd.id, quickload_prefix)) {
		std::string savename = cmd.id.substr(quickload_prefix.size());
		// Load the game by throwing load_game_exception
		load_autosave(savename, false);
	}

	if(balg::starts_with(cmd.id, quickreplay_prefix)) {
		std::string savename = cmd.id.substr(quickreplay_prefix.size());
		// Load the game by throwing load_game_exception
		load_autosave(savename, true);
	}

	// wml commands that don't allow hotkey bindings use hotkey::HOTKEY_NULL. othes use HOTKEY_WML
	if(balg::starts_with(cmd.id, wml_menu_hotkey_prefix)) {
		std::string name = cmd.id.substr(wml_menu_hotkey_prefix.length());
		const map_location& hex = mouse_handler_.get_last_hex();

		return gamestate()
			.get_wml_menu_items()
			.fire_item(name, hex, gamestate().gamedata_, gamestate(), play_controller_.get_units(), !press);
	}

	return command_executor::do_execute_command(cmd, press, release);
}

bool play_controller::hotkey_handler::can_execute_command(const hotkey::ui_command& cmd) const
{
	switch(cmd.hotkey_command) {

	// Commands we can always do:
	case hotkey::HOTKEY_LEADER:
	case hotkey::HOTKEY_CYCLE_UNITS:
	case hotkey::HOTKEY_CYCLE_BACK_UNITS:
	case hotkey::HOTKEY_ZOOM_IN:
	case hotkey::HOTKEY_ZOOM_OUT:
	case hotkey::HOTKEY_ZOOM_DEFAULT:
	case hotkey::HOTKEY_FULLSCREEN:
	case hotkey::HOTKEY_SCREENSHOT:
	case hotkey::HOTKEY_MAP_SCREENSHOT:
	case hotkey::HOTKEY_ACCELERATED:
	case hotkey::HOTKEY_SAVE_MAP:
	case hotkey::HOTKEY_TOGGLE_ELLIPSES:
	case hotkey::HOTKEY_TOGGLE_GRID:
	case hotkey::HOTKEY_MOUSE_SCROLL:
	case hotkey::HOTKEY_ANIMATE_MAP:
	case hotkey::HOTKEY_STATUS_TABLE:
	case hotkey::HOTKEY_MUTE:
	case hotkey::HOTKEY_PREFERENCES:
	case hotkey::HOTKEY_OBJECTIVES:
	case hotkey::HOTKEY_UNIT_LIST:
	case hotkey::HOTKEY_STATISTICS:
	case hotkey::HOTKEY_QUIT_GAME:
	case hotkey::HOTKEY_QUIT_TO_DESKTOP:
	case hotkey::HOTKEY_SEARCH:
	case hotkey::HOTKEY_HELP:
	case hotkey::HOTKEY_HELP_ABOUT_SAVELOAD:
	case hotkey::HOTKEY_USER_CMD:
	case hotkey::HOTKEY_CUSTOM_CMD:
	case hotkey::HOTKEY_AI_FORMULA:
	case hotkey::HOTKEY_CLEAR_MSG:
	case hotkey::HOTKEY_SELECT_HEX:
	case hotkey::HOTKEY_DESELECT_HEX:
	case hotkey::HOTKEY_MOVE_ACTION:
	case hotkey::HOTKEY_SELECT_AND_ACTION:
	case hotkey::HOTKEY_TOUCH_HEX:
	case hotkey::HOTKEY_MINIMAP_CODING_TERRAIN:
	case hotkey::HOTKEY_MINIMAP_CODING_UNIT:
	case hotkey::HOTKEY_MINIMAP_DRAW_UNITS:
	case hotkey::HOTKEY_MINIMAP_DRAW_TERRAIN:
	case hotkey::HOTKEY_MINIMAP_DRAW_VILLAGES:
	case hotkey::HOTKEY_NULL: // HOTKEY_NULL is used for menu items that don't allow hotkey bindings (for example load autosave, wml menu items and menus)
	case hotkey::HOTKEY_SAVE_REPLAY:
	case hotkey::HOTKEY_LABEL_SETTINGS:
	case hotkey::LUA_CONSOLE:
	case hotkey::HOTKEY_SCROLL_UP:
	case hotkey::HOTKEY_SCROLL_DOWN:
	case hotkey::HOTKEY_SCROLL_LEFT:
	case hotkey::HOTKEY_SCROLL_RIGHT:
	case hotkey::HOTKEY_ACHIEVEMENTS:
		return true;

	case hotkey::HOTKEY_SURRENDER: {
		std::size_t humans_notme_cnt = 0;
		for(const auto& t : play_controller_.get_teams()) {
			if(t.is_network_human()) {
				++humans_notme_cnt;
			}
		}

		return !(humans_notme_cnt < 1 || play_controller_.is_linger_mode() || play_controller_.is_observer());
	}

	// Commands that have some preconditions:
	case hotkey::HOTKEY_SAVE_GAME:
		return !events::commands_disabled;

	case hotkey::HOTKEY_SHOW_ENEMY_MOVES:
	case hotkey::HOTKEY_BEST_ENEMY_MOVES:
		return !linger() && play_controller_.enemies_visible();

	case hotkey::HOTKEY_LOAD_AUTOSAVES:
	case hotkey::HOTKEY_LOAD_GAME:
		return !play_controller_.is_networked_mp(); // Can only load games if not in a network game

	case hotkey::HOTKEY_SPEAK:
	case hotkey::HOTKEY_SPEAK_ALLY:
	case hotkey::HOTKEY_SPEAK_ALL:
	case hotkey::HOTKEY_CHAT_LOG:
		return true;

	case hotkey::HOTKEY_REDO:
		return play_controller_.can_redo();
	case hotkey::HOTKEY_UNDO:
		return play_controller_.can_undo();

	case hotkey::HOTKEY_UNIT_DESCRIPTION:
		return menu_handler_.current_unit().valid();

	case hotkey::HOTKEY_TERRAIN_DESCRIPTION:
		return mouse_handler_.get_last_hex().valid();

	case hotkey::HOTKEY_RENAME_UNIT:
		return !events::commands_disabled &&
			menu_handler_.current_unit().valid() &&
			!(menu_handler_.current_unit()->unrenamable()) &&
			menu_handler_.current_unit()->side() == gui()->viewing_team().side() &&
			play_controller_.get_teams()[menu_handler_.current_unit()->side() - 1].is_local_human();

	default:
		return false;
	}
}

namespace
{
template<typename T>
void append_items(std::vector<T>&& newitems, std::vector<T>& out)
{
	auto input = std::move(newitems);

	// Make sure list doesn't get too long: keep top two, midpoint and bottom.
	if(input.size() > 5) {
		out.push_back(std::move(input[0]));
		out.push_back(std::move(input[1]));
		out.push_back(std::move(input[input.size() / 3]));
		out.push_back(std::move(input[input.size() * 2 / 3]));
		out.push_back(std::move(input.back()));
		return;
	}

	// Range is small enough; append the whole thing
	std::move(input.begin(), input.end(), std::back_inserter(out));
}

template<typename F>
void foreach_autosave(int turn, saved_game& sg, F func)
{
	compression::format compression_format = prefs::get().save_compression_format();
	auto autosave = savegame::autosave_savegame(sg, compression_format);
	auto starting = savegame::scenariostart_savegame(sg, compression_format);

	for(; turn >= 0; --turn) {
		const std::string name = turn > 0
			? autosave.create_filename(turn)
			: starting.create_filename();

		if(savegame::save_game_exists(name, compression_format)) {
			func(turn, name + compression::format_extension(compression_format));
		}
	}
}

} // namespace

void play_controller::hotkey_handler::expand_autosaves(std::vector<config>& items) const
{
	std::vector<config> newitems;

	foreach_autosave(play_controller_.turn(), saved_game_, [&](int turn, const std::string& filename) {
		std::string label = turn > 0
			? VGETTEXT("Back to Turn $number", {{"number", std::to_string(turn)}})
			: _("Back to Start");

		newitems.emplace_back("label", label, "id", quickload_prefix + filename);
	});

	append_items(std::move(newitems), items);
}

void play_controller::hotkey_handler::expand_quickreplay(std::vector<config>& items) const
{
	std::vector<config> newitems;

	foreach_autosave(play_controller_.turn(), saved_game_, [&](int turn, const std::string& filename) {
		std::string label = turn > 0
			? VGETTEXT("Replay from Turn $number", {{"number", std::to_string(turn)}})
			: _("Replay from Start");

		newitems.emplace_back("label", label, "id", quickreplay_prefix + filename);
	});

	append_items(std::move(newitems), items);
}

void play_controller::hotkey_handler::expand_wml_commands(std::vector<config>& items)
{
	gamestate()
		.get_wml_menu_items()
		.get_items(mouse_handler_.get_last_hex(), items,gamestate(), gamestate().gamedata_, play_controller_.get_units());
}

void play_controller::hotkey_handler::show_menu(const std::vector<config>& items_arg, const point& menu_loc, bool context_menu)
{
	std::vector<config> items;
	for(const auto& item : items_arg) {
		std::string id = item["id"];
		auto cmd = hotkey::ui_command(id);

		if(id == "AUTOSAVES") {
			expand_autosaves(items);
		} else if(id == "QUICKREPLAY") {
			expand_quickreplay(items);
		} else if(id == "wml") {
			expand_wml_commands(items);
		} else if(can_execute_command(cmd) && (!context_menu || in_context_menu(cmd))) {
			items.emplace_back("id", id);
		}
	}

	if(items.empty()) {
		return;
	}

	command_executor::show_menu(items, menu_loc, context_menu);
}

bool play_controller::hotkey_handler::in_context_menu(const hotkey::ui_command& cmd) const
{
	switch(cmd.hotkey_command) {
	// Only display these if the mouse is over a castle or keep tile
	case hotkey::HOTKEY_RECRUIT:
	case hotkey::HOTKEY_REPEAT_RECRUIT:
	case hotkey::HOTKEY_RECALL: {
		// last_hex_ is set by mouse_events::mouse_motion
		const map_location& last_hex = mouse_handler_.get_last_hex();

		// A quick check to save us having to create the future map and
		// possibly loop through all units.
		if(!play_controller_.get_map().is_keep(last_hex)
			&& !play_controller_.get_map().is_castle(last_hex))
		{
			return false;
		}

		wb::future_map future; /* lasts until method returns. */

		return gamestate().side_can_recruit_on(gui()->viewing_team().side(), last_hex);
	}
	default:
		return true;
	}
}

hotkey::action_state play_controller::hotkey_handler::get_action_state(const hotkey::ui_command& cmd) const
{
	switch(cmd.hotkey_command) {
	case hotkey::HOTKEY_MINIMAP_DRAW_VILLAGES:
		return hotkey::on_if(prefs::get().minimap_draw_villages());
	case hotkey::HOTKEY_MINIMAP_CODING_UNIT:
		return hotkey::on_if(prefs::get().minimap_movement_coding());
	case hotkey::HOTKEY_MINIMAP_CODING_TERRAIN:
		return hotkey::on_if(prefs::get().minimap_terrain_coding());
	case hotkey::HOTKEY_MINIMAP_DRAW_UNITS:
		return hotkey::on_if(prefs::get().minimap_draw_units());
	case hotkey::HOTKEY_MINIMAP_DRAW_TERRAIN:
		return hotkey::on_if(prefs::get().minimap_draw_terrain());
	case hotkey::HOTKEY_ZOOM_DEFAULT:
		return hotkey::on_if(gui()->get_zoom_factor() == 1.0);
	case hotkey::HOTKEY_DELAY_SHROUD:
		return hotkey::on_if(gui()->viewing_team().auto_shroud_updates() == false);
	default:
		return hotkey::action_state::stateless;
	}
}

void play_controller::hotkey_handler::load_autosave(const std::string& filename, bool)
{
	throw savegame::load_game_exception({savegame::save_index_class::default_saves_dir(), filename});
}
