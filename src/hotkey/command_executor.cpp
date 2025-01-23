/*
	Copyright (C) 2003 - 2024
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "hotkey/command_executor.hpp"
#include "hotkey/hotkey_item.hpp"

#include "gui/gui.hpp"
#include "gui/dialogs/achievements_dialog.hpp"
#include "gui/dialogs/lua_interpreter.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/screenshot_notification.hpp"
#include "gui/dialogs/drop_down_menu.hpp"
#include "gui/widgets/retval.hpp"
#include "filesystem.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "preferences/preferences.hpp"
#include "display.hpp"
#include "quit_confirmation.hpp"
#include "sdl/surface.hpp"
#include "../resources.hpp"
#include "../playmp_controller.hpp"
#include "sdl/input.hpp" // get_mouse_state
#include "video.hpp" // toggle_fullscreen



#include <ios>
#include <set>

static lg::log_domain log_config("config");
static lg::log_domain log_hotkey("hotkey");
#define ERR_G  LOG_STREAM(err,   lg::general())
#define WRN_G  LOG_STREAM(warn,   lg::general())
#define LOG_G  LOG_STREAM(info,  lg::general())
#define DBG_G  LOG_STREAM(debug, lg::general())
#define ERR_CF LOG_STREAM(err,   log_config)
#define LOG_HK LOG_STREAM(info, log_hotkey)

namespace {

void make_screenshot(const std::string& name, bool map_screenshot)
{
	surface screenshot = display::get_singleton()->screenshot(map_screenshot);
	if(screenshot) {
		std::string filename = filesystem::get_screenshot_dir() + "/" + name + "_";
		filename = filesystem::get_next_filename(filename, ".jpg");
		gui2::dialogs::screenshot_notification::display(filename, screenshot);
	}
}
}
namespace hotkey {

static void event_queue(const SDL_Event& event, command_executor* executor);

bool command_executor::do_execute_command(const hotkey::ui_command& cmd, bool press, bool release)
{
	// hotkey release handling
	if (release) {
		switch(cmd.hotkey_command) {
			// release a scroll key, un-apply scrolling in the given direction
			case HOTKEY_SCROLL_UP:
				scroll_up(false);
				break;
			case HOTKEY_SCROLL_DOWN:
				scroll_down(false);
				break;
			case HOTKEY_SCROLL_LEFT:
				scroll_left(false);
				break;
			case HOTKEY_SCROLL_RIGHT:
				scroll_right(false);
				break;
			default:
				return false; // nothing else handles a hotkey release
		}

		return true;
	}

	// handling of hotkeys which activate even on hold events
	switch(cmd.hotkey_command) {
		case HOTKEY_REPEAT_RECRUIT:
			repeat_recruit();
			return true;
		case HOTKEY_SCROLL_UP:
			scroll_up(true);
			return true;
		case HOTKEY_SCROLL_DOWN:
			scroll_down(true);
			return true;
		case HOTKEY_SCROLL_LEFT:
			scroll_left(true);
			return true;
		case HOTKEY_SCROLL_RIGHT:
			scroll_right(true);
			return true;
		default:
			break;
	}

	if(!press) {
		return false; // nothing else handles hotkey hold events
	}

	// hotkey press handling
	switch(cmd.hotkey_command) {
		case HOTKEY_CYCLE_UNITS:
			cycle_units();
			break;
		case HOTKEY_CYCLE_BACK_UNITS:
			cycle_back_units();
			break;
		case HOTKEY_ENDTURN:
			end_turn();
			break;
		case HOTKEY_UNIT_HOLD_POSITION:
			unit_hold_position();
			break;
		case HOTKEY_END_UNIT_TURN:
			end_unit_turn();
			break;
		case HOTKEY_LEADER:
			goto_leader();
			break;
		case HOTKEY_UNDO:
			undo();
			break;
		case HOTKEY_REDO:
			redo();
			break;
		case HOTKEY_TERRAIN_DESCRIPTION:
			terrain_description();
			break;
		case HOTKEY_UNIT_DESCRIPTION:
			unit_description();
			break;
		case HOTKEY_RENAME_UNIT:
			rename_unit();
			break;
		case HOTKEY_SAVE_GAME:
			save_game();
			break;
		case HOTKEY_SAVE_REPLAY:
			save_replay();
			break;
		case HOTKEY_SAVE_MAP:
			save_map();
			break;
		case HOTKEY_LOAD_GAME:
			load_game();
			break;
		case HOTKEY_TOGGLE_ELLIPSES:
			toggle_ellipses();
			break;
		case HOTKEY_TOGGLE_GRID:
			toggle_grid();
			break;
		case HOTKEY_STATUS_TABLE:
			status_table();
			break;
		case HOTKEY_RECALL:
			recall();
			break;
		case HOTKEY_LABEL_SETTINGS:
			label_settings();
			break;
		case HOTKEY_RECRUIT:
			recruit();
			break;
		case HOTKEY_SPEAK:
			speak();
			break;
		case HOTKEY_SPEAK_ALLY:
			whisper();
			break;
		case HOTKEY_SPEAK_ALL:
			shout();
			break;
		case HOTKEY_CREATE_UNIT:
			create_unit();
			break;
		case HOTKEY_CHANGE_SIDE:
			change_side();
			break;
		case HOTKEY_KILL_UNIT:
			kill_unit();
			break;
		case HOTKEY_TELEPORT_UNIT:
			select_teleport();
			break;
		case HOTKEY_PREFERENCES:
			preferences();
			break;
		case HOTKEY_OBJECTIVES:
			objectives();
			break;
		case HOTKEY_UNIT_LIST:
			unit_list();
			break;
		case HOTKEY_STATISTICS:
			show_statistics();
			break;
		case HOTKEY_STOP_NETWORK:
			stop_network();
			break;
		case HOTKEY_START_NETWORK:
			start_network();
			break;
		case HOTKEY_LABEL_TEAM_TERRAIN:
			label_terrain(true);
			break;
		case HOTKEY_LABEL_TERRAIN:
			label_terrain(false);
			break;
		case HOTKEY_CLEAR_LABELS:
			clear_labels();
			break;
		case HOTKEY_SHOW_ENEMY_MOVES:
			show_enemy_moves(false);
			break;
		case HOTKEY_BEST_ENEMY_MOVES:
			show_enemy_moves(true);
			break;
		case HOTKEY_DELAY_SHROUD:
			toggle_shroud_updates();
			break;
		case HOTKEY_UPDATE_SHROUD:
			update_shroud_now();
			break;
		case HOTKEY_CONTINUE_MOVE:
			continue_move();
			break;
		case HOTKEY_SEARCH:
			search();
			break;
		case HOTKEY_HELP:
			show_help();
			break;
		case HOTKEY_HELP_ABOUT_SAVELOAD:
			// although HOTKEY_HELP uses a virtual call to allow context-specific help, this one is already a specific topic
			help::show_help("saveload");
			break;
		case HOTKEY_CHAT_LOG:
			show_chat_log();
			break;
		case HOTKEY_USER_CMD:
			user_command();
			break;
		case HOTKEY_CUSTOM_CMD:
			custom_command();
			break;
		case HOTKEY_AI_FORMULA:
			ai_formula();
			break;
		case HOTKEY_CLEAR_MSG:
			clear_messages();
			break;
		case HOTKEY_LANGUAGE:
			change_language();
			break;
		case HOTKEY_REPLAY_PLAY:
			play_replay();
			break;
		case HOTKEY_REPLAY_RESET:
			reset_replay();
			break;
		case HOTKEY_REPLAY_STOP:
			stop_replay();
			break;
		case HOTKEY_REPLAY_NEXT_TURN:
			replay_next_turn();
			break;
		case HOTKEY_REPLAY_NEXT_SIDE:
			replay_next_side();
			break;
		case HOTKEY_REPLAY_NEXT_MOVE:
			replay_next_move();
			break;
		case HOTKEY_REPLAY_SHOW_EVERYTHING:
			replay_show_everything();
			break;
		case HOTKEY_REPLAY_SHOW_EACH:
			replay_show_each();
			break;
		case HOTKEY_REPLAY_SHOW_TEAM1:
			replay_show_team1();
			break;
		case HOTKEY_REPLAY_SKIP_ANIMATION:
			replay_skip_animation();
			break;
		case HOTKEY_REPLAY_EXIT:
			replay_exit();
			break;
		case HOTKEY_WB_TOGGLE:
			whiteboard_toggle();
			break;
		case HOTKEY_WB_EXECUTE_ACTION:
			whiteboard_execute_action();
			break;
		case HOTKEY_WB_EXECUTE_ALL_ACTIONS:
			whiteboard_execute_all_actions();
			break;
		case HOTKEY_WB_DELETE_ACTION:
			whiteboard_delete_action();
			break;
		case HOTKEY_WB_BUMP_UP_ACTION:
			whiteboard_bump_up_action();
			break;
		case HOTKEY_WB_BUMP_DOWN_ACTION:
			whiteboard_bump_down_action();
			break;
		case HOTKEY_WB_SUPPOSE_DEAD:
			whiteboard_suppose_dead();
			break;
		case HOTKEY_SELECT_HEX:
			select_hex();
			break;
		case HOTKEY_DESELECT_HEX:
			deselect_hex();
			break;
		case HOTKEY_MOVE_ACTION:
			move_action();
			break;
		case HOTKEY_SELECT_AND_ACTION:
			select_and_action();
			break;
		case HOTKEY_TOUCH_HEX:
			touch_hex();
			break;
		case HOTKEY_ACCELERATED:
			toggle_accelerated_speed();
			break;
		case LUA_CONSOLE:
			lua_console();
			break;
		case HOTKEY_ZOOM_IN:
			zoom_in();
			break;
		case HOTKEY_ZOOM_OUT:
			zoom_out();
			break;
		case HOTKEY_ZOOM_DEFAULT:
			zoom_default();
			break;
		case HOTKEY_MAP_SCREENSHOT:
			map_screenshot();
			break;
		case HOTKEY_QUIT_TO_DESKTOP:
			quit_confirmation::quit_to_desktop();
			break;
		case HOTKEY_QUIT_GAME:
			gui2::switch_theme(prefs::get().gui2_theme());
			quit_confirmation::quit_to_title();
			break;
		case HOTKEY_SURRENDER:
			surrender_game();
			break;
		case HOTKEY_MINIMAP_DRAW_TERRAIN:
			prefs::get().set_minimap_draw_terrain(!prefs::get().minimap_draw_terrain());
			recalculate_minimap();
			break;
		case HOTKEY_MINIMAP_CODING_TERRAIN:
			prefs::get().set_minimap_terrain_coding(!prefs::get().minimap_terrain_coding());
			recalculate_minimap();
			break;
		case HOTKEY_MINIMAP_CODING_UNIT:
			prefs::get().set_minimap_movement_coding(!prefs::get().minimap_movement_coding());
			recalculate_minimap();
			break;
		case HOTKEY_MINIMAP_DRAW_UNITS:
			prefs::get().set_minimap_draw_units(!prefs::get().minimap_draw_units());
			recalculate_minimap();
			break;
		case HOTKEY_MINIMAP_DRAW_VILLAGES:
			prefs::get().set_minimap_draw_villages(!prefs::get().minimap_draw_villages());
			recalculate_minimap();
			break;
		case HOTKEY_ACHIEVEMENTS:
			{
				gui2::dialogs::achievements_dialog ach;
				ach.show();
			}
			break;
		default:
			return false;
	}
	return true;
}

void command_executor::surrender_game() {
	if(gui2::show_message(_("Surrender"), _("Do you really want to surrender the game?"), gui2::dialogs::message::yes_no_buttons) != gui2::retval::CANCEL) {
		playmp_controller* pmc = dynamic_cast<playmp_controller*>(resources::controller);
		if(pmc && !pmc->is_linger_mode() && !pmc->is_observer()) {
			pmc->surrender(display::get_singleton()->viewing_team_index());
		}
	}
}

void command_executor::show_menu(const std::vector<config>& items_arg, int xloc, int yloc, bool /*context_menu*/, display& gui)
{
	std::vector<config> items = items_arg;
	if (items.empty()) return;

	get_menu_images(gui, items);

	int res = -1;
	point selection_pos;
	{
		SDL_Rect pos {xloc, yloc, 1, 1};
		gui2::dialogs::drop_down_menu mmenu(pos, items, -1, true, false); // TODO: last value should be variable
		if(mmenu.show()) {
			res = mmenu.selected_item();
			if(res >= 0) {
				// Get selection coordinates for a potential submenu below
				selection_pos = mmenu.selected_item_pos();
				// Compensate for borders
				selection_pos.x--;
				selection_pos.y--;
			}
		}
	} // This will kill the dialog.
	if (res < 0 || std::size_t(res) >= items.size()) return;

	std::string id = items[res]["id"];
	const theme::menu* submenu = gui.get_theme().get_menu_item(id);
	if (submenu) {
		this->show_menu(submenu->items(), selection_pos.x, selection_pos.y, submenu->is_context(), gui);
	} else {
		hotkey::ui_command cmd = hotkey::ui_command(id, res);
		do_execute_command(cmd);
		set_button_state();
	}
}

void command_executor::execute_action(const std::vector<std::string>& items_arg, int /*xloc*/, int /*yloc*/, bool /*context_menu*/, display&)
{
	std::vector<std::string> items = items_arg;
	if (items.empty()) {
		return;
	}

	std::vector<std::string>::iterator i = items.begin();
	while(i != items.end()) {
		hotkey::ui_command cmd = hotkey::ui_command(*i);
		if (can_execute_command(cmd)) {
			do_execute_command(cmd);
			set_button_state();
		}
		++i;
	}
}

std::string command_executor::get_menu_image(display& disp, const std::string& command, int index) const
{
	const std::string base_image_name = "icons/action/" + command + "_25.png";
	const std::string pressed_image_name = "icons/action/" + command + "_25-pressed.png";

	hotkey::ui_command cmd = hotkey::ui_command(command, index);
	const hotkey::ACTION_STATE state = get_action_state(cmd);

	const theme::menu* menu = disp.get_theme().get_menu_item(command);
	if (menu) {
		return "icons/arrows/short_arrow_right_25.png~CROP(3,3,18,18)"; // TODO should not be hardcoded
	}

	if (filesystem::file_exists(game_config::path + "/images/" + base_image_name)) {
		switch (state) {
			case ACTION_ON:
			case ACTION_SELECTED:
				return pressed_image_name + "~CROP(3,3,18,18)";
			default:
				return base_image_name + "~CROP(3,3,18,18)";
		}
	}

	switch (get_action_state(cmd)) {
		case ACTION_ON:
			return game_config::images::checked_menu;
		case ACTION_OFF:
			return game_config::images::unchecked_menu;
		case ACTION_SELECTED:
			return game_config::images::selected_menu;
		case ACTION_DESELECTED:
			return game_config::images::deselected_menu;
		default: return get_action_image(cmd);
	}
}

void command_executor::get_menu_images(display& disp, std::vector<config>& items)
{
	for(std::size_t i = 0; i < items.size(); ++i) {
		config& item = items[i];

		const std::string& item_id = item["id"];
		const hotkey::HOTKEY_COMMAND hk = hotkey::get_hotkey_command(item_id).command;

		//see if this menu item has an associated image
		std::string img(get_menu_image(disp, item_id, i));
		if (img.empty() == false) {
			item["icon"] = img;
		}

		const theme::menu* menu = disp.get_theme().get_menu_item(item_id);
		if(menu) {
			item["label"] = menu->title();
		} else if(hk != hotkey::HOTKEY_NULL) {
			std::string desc = hotkey::get_hotkey_command(item_id).description;
			if(hk == HOTKEY_ENDTURN) {
				const theme::action *b = disp.get_theme().get_action_item("button-endturn");
				if (b) {
					desc = b->title();
				}
			}

			item["label"] = desc;
			item["details"] = hotkey::get_names(item_id);
		}
	}
}

void mbutton_event(const SDL_Event& event, command_executor* executor)
{
	event_queue(event, executor);

	/* Run mouse events immediately.

	This is necessary because the sidebar doesn't allow set_button_state() to be called after a
	button has received the mouse press event but before it has received the mouse release event.
	When https://github.com/wesnoth/wesnoth/pull/2872 delayed the processing of input events,
	set_button_state() ended up being called at such a time. However, if we run the event handlers
	now, the button (if any) hasn't received the press event yet and we can call set_button_state()
	safely.

	See https://github.com/wesnoth/wesnoth/issues/2884 */

	run_events(executor);
}

void jbutton_event(const SDL_Event& event, command_executor* executor)
{
	event_queue(event, executor);
}

void jhat_event(const SDL_Event& event, command_executor* executor)
{
	event_queue(event, executor);
}

void key_event(const SDL_Event& event, command_executor* executor)
{
	if (!executor) return;
	event_queue(event,executor);
}

void keyup_event(const SDL_Event&, command_executor* executor)
{
	if(!executor) return;
	executor->handle_keyup();
}

void run_events(command_executor* executor)
{
	if(!executor) return;
	bool commands_ran = executor->run_queued_commands();
	if(commands_ran) {
		executor->set_button_state();
	}
}

static void event_queue(const SDL_Event& event, command_executor* executor)
{
	if (!executor) return;
	executor->queue_command(event);
	executor->set_button_state();
}

void command_executor::queue_command(const SDL_Event& event, int index)
{
	LOG_HK << "event 0x" << std::hex << event.type << std::dec;
	if(event.type == SDL_TEXTINPUT) {
		LOG_HK << "SDL_TEXTINPUT \"" << event.text.text << "\"";
	}

	const hotkey_ptr hk = get_hotkey(event);
	if(!hk->active() || hk->is_disabled()) {
		return;
	}

	const hotkey_command& command = hotkey::get_hotkey_command(hk->get_command());
	bool keypress = (event.type == SDL_KEYDOWN || event.type == SDL_TEXTINPUT) &&
		!press_event_sent_;
	bool press = keypress ||
		(event.type == SDL_JOYBUTTONDOWN || event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_FINGERDOWN);
	bool release = event.type == SDL_KEYUP;
	if(press) {
		LOG_HK << "sending press event (keypress = " <<
			std::boolalpha << keypress << std::noboolalpha << ")";
	}
	if(keypress) {
		press_event_sent_ = true;
	}

	command_queue_.emplace_back(command, index, press, release);
}

void command_executor::execute_command_wrap(const command_executor::queued_command& command)
{
	auto ui_cmd = hotkey::ui_command(*command.command, command.index);
	if (!can_execute_command(ui_cmd)
			|| do_execute_command(ui_cmd, command.press, command.release)) {
		return;
	}

	if (!command.press) {
		return; // none of the commands here respond to a key release
	}

	switch(command.command->command) {
		case HOTKEY_FULLSCREEN:
			video::toggle_fullscreen();
			break;
		case HOTKEY_SCREENSHOT:
			make_screenshot(_("Screenshot"), false);
			break;
		case HOTKEY_ANIMATE_MAP:
			prefs::get().set_animate_map(!prefs::get().animate_map());
			break;
		case HOTKEY_MOUSE_SCROLL:
			prefs::get().set_mouse_scrolling(!prefs::get().mouse_scrolling());
			break;
		case HOTKEY_MUTE:
			{
				// look if both is not playing
				static struct before_muted_s
				{
					bool playing_sound,playing_music;
					before_muted_s() : playing_sound(false),playing_music(false){}
				} before_muted;
				if (prefs::get().music_on() || prefs::get().sound())
				{
					// then remember settings and mute both
					before_muted.playing_sound = prefs::get().sound();
					before_muted.playing_music = prefs::get().music_on();
					prefs::get().set_sound(false);
					prefs::get().set_music(false);
				}
				else
				{
					// then set settings before mute
					prefs::get().set_sound(before_muted.playing_sound);
					prefs::get().set_music(before_muted.playing_music);
				}
			}
			break;
		default:
			DBG_G << "command_executor: unknown command number " << command.command->command << ", ignoring.";
			break;
	}
}

void command_executor_default::set_button_state()
{
	display& disp = get_display();
	for (const theme::menu& menu : disp.get_theme().menus()) {

		std::shared_ptr<gui::button> button = disp.find_menu_button(menu.get_id());
		if (!button) continue;
		bool enabled = false;
		for (const auto& command : menu.items()) {

			ui_command command_obj = ui_command(command["id"].str());
			bool can_execute = can_execute_command(command_obj);
			if (can_execute) {
				enabled = true;
				break;
			}
		}
		button->enable(enabled);
	}

	for (const theme::action& action : disp.get_theme().actions()) {

		std::shared_ptr<gui::button> button = disp.find_action_button(action.get_id());
		if (!button) continue;
		bool enabled = false;
		int i = 0;
		for (const std::string& command : action.items()) {

			ui_command command_obj = ui_command(command);
			std::string tooltip = action.tooltip(i);
			if (filesystem::file_exists(game_config::path + "/images/icons/action/" + command + "_25.png" ))
				button->set_overlay("icons/action/" + command);
			if (!tooltip.empty())
				button->set_tooltip_string(tooltip);

			bool can_execute = can_execute_command(command_obj);
			i++;
			if (!can_execute) continue;
			enabled = true;

			ACTION_STATE state = get_action_state(command_obj);
			switch (state) {
			case ACTION_SELECTED:
			case ACTION_ON:
				button->set_check(true);
				break;
			case ACTION_OFF:
			case ACTION_DESELECTED:
				button->set_check(false);
				break;
			case ACTION_STATELESS:
				break;
			default:
				break;
			}

			break;
		}
		button->enable(enabled);
	}
}

// Removes duplicate commands caused by both SDL_KEYDOWN and SDL_TEXTINPUT triggering hotkeys.
// See https://github.com/wesnoth/wesnoth/issues/1736
std::vector<command_executor::queued_command> command_executor::filter_command_queue()
{
	std::vector<queued_command> filtered_commands;

	/** A command plus "key released" flag. Otherwise, we will filter out key releases that are preceded by a keypress. */
	using command_with_keyrelease = std::pair<const hotkey_command*, bool>;
	std::set<command_with_keyrelease> seen_commands;

	for(const queued_command& cmd : command_queue_) {
		command_with_keyrelease command_key(cmd.command, cmd.release);
		if(seen_commands.find(command_key) == seen_commands.end()) {
			seen_commands.insert(command_key);
			filtered_commands.push_back(cmd);
		}
	}

	command_queue_.clear();

	return filtered_commands;
}

bool command_executor::run_queued_commands()
{
	std::vector<queued_command> commands = filter_command_queue();
	for(const queued_command& cmd : commands) {
		execute_command_wrap(cmd);
	}

	return !commands.empty();
}

void command_executor_default::recalculate_minimap()
{
	get_display().recalculate_minimap();
}

void command_executor_default::lua_console()
{
	if (get_display().in_game()) {
		gui2::dialogs::lua_interpreter::display(gui2::dialogs::lua_interpreter::GAME);
	} else {
		command_executor::lua_console();
	}

}

void command_executor::lua_console()
{
	gui2::dialogs::lua_interpreter::display(gui2::dialogs::lua_interpreter::APP);
}

void command_executor_default::zoom_in()
{
	if(!get_display().view_locked()) {
		get_display().set_zoom(true);
	}
}

void command_executor_default::zoom_out()
{
	if(!get_display().view_locked()) {
		get_display().set_zoom(false);
	}
}

void command_executor_default::zoom_default()
{
	if(!get_display().view_locked()) {
		get_display().toggle_default_zoom();
	}
}

void command_executor_default::map_screenshot()
{
	make_screenshot(_("Map-Screenshot"), true);
}
}
