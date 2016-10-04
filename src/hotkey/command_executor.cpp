/*
   Copyright (C) 2003 - 2016 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "command_executor.hpp"
#include "hotkey_item.hpp"

#include "gui/dialogs/lua_interpreter.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/screenshot_notification.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/dialogs/drop_down_list.hpp"
#include "gui/widgets/window.hpp"
#include "wml_separators.hpp"
#include "filesystem.hpp"
#include "config_assign.hpp"
#include "construct_dialog.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "preferences.hpp"
#include "game_end_exceptions.hpp"
#include "display.hpp"
#include "quit_confirmation.hpp"

#include "utils/functional.hpp"

#include <cassert>

static lg::log_domain log_config("config");
#define ERR_G  LOG_STREAM(err,   lg::general())
#define WRN_G  LOG_STREAM(warn,   lg::general())
#define LOG_G  LOG_STREAM(info,  lg::general())
#define DBG_G  LOG_STREAM(debug, lg::general())
#define ERR_CF LOG_STREAM(err,   log_config)

namespace {

bool screenshot(const std::string& filename, CVideo& video)
{
	return image::save_image(video.getSurface(), filename);
}

template<typename TFunc>
void make_screenshot(const std::string& name, CVideo& video, const TFunc& func)
{
	std::string filename = filesystem::get_screenshot_dir() + "/" + name + "_";
#ifdef HAVE_LIBPNG
	const std::string ext = ".png";
#else
	const std::string ext = ".bmp";
#endif
	filename = filesystem::get_next_filename(filename, ext);
	const bool res = func(filename, video);
	if (res) {
		gui2::tscreenshot_notification::display(filename, video);
	} else {
		gui2::show_error_message(video,
			_("Screenshot creation failed.\n\n"
			"Make sure there is enough space on the drive holding Wesnoth’s player resource files and that file permissions are set up correctly."));
	}
}
}
namespace hotkey {

static void event_execute(const SDL_Event& event, command_executor* executor);

bool command_executor::execute_command(const hotkey_command&  cmd, int /*index*/, bool press)
{
	// hotkey release handling
	if (!press) {
		switch(cmd.id) {
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

	// hotkey press handling
	switch(cmd.id) {
		case HOTKEY_SCROLL_UP:
			scroll_up(true);
			break;
		case HOTKEY_SCROLL_DOWN:
			scroll_down(true);
			break;
		case HOTKEY_SCROLL_LEFT:
			scroll_left(true);
			break;
		case HOTKEY_SCROLL_RIGHT:
			scroll_right(true);
			break;
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
		case hotkey::HOTKEY_REPEAT_RECRUIT:
			repeat_recruit();
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
			quit_confirmation::quit_to_title();
			break;
		default:
			return false;
	}
	return true;
}


int command_executor::show_menu(const std::vector<config>& items, SDL_Rect& pos, bool context_menu, display& gui)
{
	if (items.empty()) return -1;

	int res = -1;
	{
		if(context_menu) {
			pos.w = pos.h = 1;
		}
		gui2::tdrop_down_list mmenu(pos, items, -1, false);
		mmenu.show(gui.video());
		if(mmenu.get_retval() == gui2::twindow::OK) {
			res = mmenu.selected_item();
			if(res >= 0 && !items[res]["repeating"].to_bool()) {
				pos = mmenu.selected_item_rect();
			}
		}
	} // This will kill the dialog.
	if (res < 0 || size_t(res) >= items.size()) return -1;
	return res;
}

void command_executor::execute_action(const std::vector<std::string>& items, int /*xloc*/, int /*yloc*/, bool /*context_menu*/, display&)
{
	for(const std::string& cmd_id : items) {
		const hotkey_command &command = hotkey::get_hotkey_command(cmd_id);
		if (can_execute_command(command)) {
			hotkey::execute_command(command, this);
			set_button_state();
		}
	}
}

std::string command_executor::get_menu_image(display& disp, const std::string& command, int index) const {
//
//	// TODO: Find a way to do away with the fugly special markup
//	if(command[0] == '&') {
//		size_t n = command.find_first_of('=');
//		if(n != std::string::npos)
//			return command.substr(1, n - 1);
//	}

	const std::string base_image_name = "icons/action/" + command + "_25.png";
	const std::string pressed_image_name = "icons/action/" + command + "_25-pressed.png";

	const hotkey::HOTKEY_COMMAND hk = hotkey::get_id(command);
	const hotkey::ACTION_STATE state = get_action_state(hk, index);

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

	switch (get_action_state(hk, index)) {
		case ACTION_ON:
			return game_config::images::checked_menu;
		case ACTION_OFF:
			return game_config::images::unchecked_menu;
		case ACTION_SELECTED:
			return game_config::images::selected_menu;
		case ACTION_DESELECTED:
			return game_config::images::deselected_menu;
		default: return get_action_image(hk, index);
	}
}

std::vector<config> command_executor::get_menu_images(display& disp, const std::vector<std::string>& items) {
	std::vector<config> result;

	for(size_t i = 0; i < items.size(); ++i) {
		std::string const& item = items[i];
		const hotkey::HOTKEY_COMMAND hk = hotkey::get_id(item);
		result.emplace_back(config_of("id", item));

		//see if this menu item has an associated image
		std::string img(get_menu_image(disp, item, i));
		if (img.empty() == false) {
			result.back()["icon"] = img;
		}

		const theme::menu* menu = disp.get_theme().get_menu_item(item);
		if (menu) {
			result.back()["label"] = menu->title();
		} else if(hk != HOTKEY_NULL) {
			std::string desc = hotkey::get_description(item);
			if (hk == HOTKEY_ENDTURN) {
				const theme::action *b = disp.get_theme().get_action_item("button-endturn");
				if (b) {
					desc = b->title();
				}
			}
			result.back()["label"] = desc;
			result.back()["details"] = hotkey::get_names(item);
		}
	}
	return result;
}
basic_handler::basic_handler(command_executor* exec) : exec_(exec) {}

void basic_handler::handle_event(const SDL_Event& event)
{

	switch (event.type) {
	case SDL_KEYDOWN:
	case SDL_KEYUP:
		// If we're in a dialog we only want to handle items that are explicitly
		// handled by the executor.
		// If we're not in a dialog we can call the regular key event handler.
		if (!gui::in_dialog()) {
			key_event(event,exec_);
		} else if (exec_ != nullptr) {
			event_execute(event,exec_);
		}
		break;
	case SDL_JOYBUTTONDOWN:
	case SDL_JOYBUTTONUP:
		if (!gui::in_dialog()) {
			jbutton_event(event,exec_);
		} else if (exec_ != nullptr) {
			event_execute(event,exec_);
		}
		break;
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
		if (!gui::in_dialog()) {
			mbutton_event(event,exec_);
		} else if (exec_ != nullptr) {
			event_execute(event,exec_);
		}
		break;
	}
}

void mbutton_event(const SDL_Event& event, command_executor* executor)
{
	event_execute(event, executor);
}

void jbutton_event(const SDL_Event& event, command_executor* executor)
{
	event_execute(event, executor);
}

void jhat_event(const SDL_Event& event, command_executor* executor)
{
	event_execute(event, executor);
}

void key_event(const SDL_Event& event, command_executor* executor)
{
	if (!executor) return;
	event_execute(event,executor);
}

static void event_execute( const SDL_Event& event, command_executor* executor)
{
	if (!executor) return;
	const hotkey_ptr hk = get_hotkey(event);
	if (!hk->active() || hk->is_disabled()) {
		return;
	}

	bool press = event.type == SDL_KEYDOWN || event.type == SDL_JOYBUTTONDOWN || event.type == SDL_MOUSEBUTTONDOWN;

	execute_command(hotkey::get_hotkey_command(hk->get_command()), executor, -1, press);
	executor->set_button_state();
}

void execute_command(const hotkey_command& command, command_executor* executor, int index, bool press)
{
	if (executor != nullptr) {
		if (!executor->can_execute_command(command, index)
				|| executor->execute_command(command, index, press)) {
			return;
		}
	}

	if (!press) {
		return; // none of the commands here respond to a key release
    }

	switch (command.id) {

		case HOTKEY_MINIMAP_DRAW_TERRAIN:
			preferences::toggle_minimap_draw_terrain();
			executor->recalculate_minimap();
			break;
		case HOTKEY_MINIMAP_CODING_TERRAIN:
			preferences::toggle_minimap_terrain_coding();
			executor->recalculate_minimap();
			break;
		case HOTKEY_MINIMAP_CODING_UNIT:
			preferences::toggle_minimap_movement_coding();
			executor->recalculate_minimap();
			break;
		case HOTKEY_MINIMAP_DRAW_UNITS:
			preferences::toggle_minimap_draw_units();
			executor->recalculate_minimap();
			break;
		case HOTKEY_MINIMAP_DRAW_VILLAGES:
			preferences::toggle_minimap_draw_villages();
			executor->recalculate_minimap();
			break;
		case HOTKEY_FULLSCREEN:
			executor->get_video().set_fullscreen(!preferences::fullscreen());
			break;
		case HOTKEY_SCREENSHOT:
			make_screenshot(_("Screenshot"), executor->get_video(), &::screenshot);
			break;
		case HOTKEY_ANIMATE_MAP:
			preferences::set_animate_map(!preferences::animate_map());
			break;
		case HOTKEY_MOUSE_SCROLL:
			preferences::enable_mouse_scroll(!preferences::mouse_scroll_enabled());
			break;
		case HOTKEY_MUTE:
			{
				// look if both is not playing
				static struct before_muted_s
				{
					bool playing_sound,playing_music;
					before_muted_s() : playing_sound(false),playing_music(false){}
				} before_muted;
				if (preferences::music_on() || preferences::sound_on())
				{
					// then remember settings and mute both
					before_muted.playing_sound = preferences::sound_on();
					before_muted.playing_music = preferences::music_on();
					preferences::set_sound(false);
					preferences::set_music(false);
				}
				else
				{
					// then set settings before mute
					preferences::set_sound(before_muted.playing_sound);
					preferences::set_music(before_muted.playing_music);
				}
			}
			break;
		default:
			DBG_G << "command_executor: unknown command number " << command.id << ", ignoring.\n";
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
		for (const std::string& command : menu.items()) {

			const hotkey::hotkey_command& command_obj = hotkey::get_hotkey_command(command);
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

			const hotkey::hotkey_command& command_obj = hotkey::get_hotkey_command(command);
			std::string tooltip = action.tooltip(i);
			if (filesystem::file_exists(game_config::path + "/images/icons/action/" + command + "_25.png" ))
				button->set_overlay("icons/action/" + command);
			if (!tooltip.empty())
				button->set_tooltip_string(tooltip);

			bool can_execute = can_execute_command(command_obj);
			i++;
			if (!can_execute) continue;
			enabled = true;

			ACTION_STATE state = get_action_state(command_obj.id, -1);
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

void command_executor_default::recalculate_minimap()
{
	get_display().recalculate_minimap();
}
CVideo& command_executor_default::get_video()
{
	return get_display().video();
}
void command_executor_default::lua_console()
{
	if (get_display().in_game()) {
		gui2::tlua_interpreter::display(get_video(), gui2::tlua_interpreter::GAME);
	} else {
		command_executor::lua_console();
	}

}
void command_executor::lua_console()
{
	gui2::tlua_interpreter::display(get_video(), gui2::tlua_interpreter::APP);
}

void command_executor_default::zoom_in()
{
	get_display().set_zoom(zoom_amount);
}
void command_executor_default::zoom_out()
{
	get_display().set_zoom(-zoom_amount);
}
void command_executor_default::zoom_default()
{
	get_display().set_default_zoom();
}
void command_executor_default::map_screenshot()
{
	make_screenshot(_("Map-Screenshot"), get_video(),
		[this](const std::string& filename, const CVideo&)
	{
		return get_display().screenshot(filename, true);
	});
}
}
