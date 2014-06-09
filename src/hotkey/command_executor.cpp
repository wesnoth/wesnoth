/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
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

#include "boost/foreach.hpp"

#include "gui/dialogs/message.hpp"
#include "gui/dialogs/screenshot_notification.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/widgets/window.hpp"
#include "wml_separators.hpp"
#include "filesystem.hpp"
#include "construct_dialog.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "preferences.hpp"
#include "preferences_display.hpp"
#include "game_end_exceptions.hpp"

static lg::log_domain log_config("config");
#define ERR_G  LOG_STREAM(err,   lg::general)
#define LOG_G  LOG_STREAM(info,  lg::general)
#define DBG_G  LOG_STREAM(debug, lg::general)
#define ERR_CF LOG_STREAM(err,   log_config)


namespace hotkey {

static void key_event_execute(display& disp, const SDL_KeyboardEvent& event, command_executor* executor);
static void jbutton_event_execute(display& disp, const SDL_JoyButtonEvent& event, command_executor* executor);
static void jhat_event_execute(display& disp, const SDL_JoyHatEvent& event, command_executor* executor);
static void mbutton_event_execute(display& disp, const SDL_MouseButtonEvent& event, command_executor* executor);

bool command_executor::execute_command(const hotkey_command&  cmd, int /*index*/)
{
	switch(cmd.id) {
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
		default:
			return false;
	}
	return true;
}

void command_executor::set_button_state(display& disp) {

	BOOST_FOREACH(const theme::menu& menu, disp.get_theme().menus()) {

		gui::button* button = disp.find_menu_button(menu.get_id());
		if (!button) continue;
		bool enabled = false;
		BOOST_FOREACH(const std::string& command, menu.items()) {

			const hotkey::hotkey_command& command_obj = hotkey::get_hotkey_command(command);
			bool can_execute = can_execute_command(command_obj);
			if (can_execute) {
				enabled = true;
				break;
			}
		}
		button->enable(enabled);
	}

	BOOST_FOREACH(const theme::action& action, disp.get_theme().actions()) {

		gui::button* button = disp.find_action_button(action.get_id());
		bool enabled = false;
		int i = 0;
		BOOST_FOREACH(const std::string& command, action.items()) {

			const hotkey::hotkey_command& command_obj = hotkey::get_hotkey_command(command);
			std::string tooltip = action.tooltip(i);
			if (file_exists(game_config::path + "/images/icons/action/" + command + "_25.png" ))
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

void command_executor::show_menu(const std::vector<std::string>& items_arg, int xloc, int yloc, bool /*context_menu*/, display& gui)
{
	std::vector<std::string> items = items_arg;
	if (items.empty()) return;

	std::vector<std::string> menu = get_menu_images(gui, items);
	int res = 0;
	{
		gui::dialog mmenu = gui::dialog(gui,"","",
				gui::MESSAGE, gui::dialog::hotkeys_style);
		mmenu.set_menu(menu);
		res = mmenu.show(xloc, yloc);
	} // This will kill the dialog.
	if (res < 0 || size_t(res) >= items.size()) return;

	const theme::menu* submenu = gui.get_theme().get_menu_item(items[res]);
	if (submenu) {
		int y,x;
		SDL_GetMouseState(&x,&y);
		this->show_menu(submenu->items(), x, y, submenu->is_context(), gui);
	} else {
		const hotkey::hotkey_command& cmd = hotkey::get_hotkey_command(items[res]);
		hotkey::execute_command(gui,cmd,this,res);
		set_button_state(gui);
	}
}

void command_executor::execute_action(const std::vector<std::string>& items_arg, int /*xloc*/, int /*yloc*/, bool /*context_menu*/, display& gui)
{
	std::vector<std::string> items = items_arg;
	if (items.empty()) {
		return;
	}

	std::vector<std::string>::iterator i = items.begin();
	while(i != items.end()) {
		const hotkey_command &command = hotkey::get_hotkey_command(*i);
		if (can_execute_command(command)) {
			hotkey::execute_command(gui, command, this);
			set_button_state(gui);
		}
		++i;
	}
}

std::string command_executor::get_menu_image(display& disp, const std::string& command, int index) const {

	const std::string base_image_name = "icons/action/" + command + "_25.png";
	const std::string pressed_image_name = "icons/action/" + command + "_25-pressed.png";

	const hotkey::HOTKEY_COMMAND hk = hotkey::get_id(command);
	const hotkey::ACTION_STATE state = get_action_state(hk, index);

	const theme::menu* menu = disp.get_theme().get_menu_item(command);
	if (menu)
		return "buttons/fold-arrow.png"; // TODO should not be hardcoded

	if (file_exists(game_config::path + "/images/" + base_image_name)) {
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

std::vector<std::string> command_executor::get_menu_images(display& disp, const std::vector<std::string>& items) {
	std::vector<std::string> result;
	bool has_image = false;

	for (size_t i = 0; i < items.size(); ++i) {
		std::string const& item = items[i];
		const hotkey::HOTKEY_COMMAND hk = hotkey::get_id(item);

		std::stringstream str;
		//see if this menu item has an associated image
		std::string img(get_menu_image(disp, item, i));
		if (img.empty() == false) {
			has_image = true;
			str << IMAGE_PREFIX << img << COLUMN_SEPARATOR;
		}

		const theme::menu* menu = disp.get_theme().get_menu_item(item);
		if (hk == hotkey::HOTKEY_NULL) {
			if (menu)
				str << menu->title();
			else
				str << item.substr(0, item.find_last_not_of(' ') + 1) << COLUMN_SEPARATOR;
		} else {

			if (menu)
				str << menu->title();
			else {
				std::string desc = hotkey::get_description(item);
				if (hk == HOTKEY_ENDTURN) {
					const theme::action *b = disp.get_theme().get_action_item("button-endturn");
					if (b) {
						desc = b->title();
					}
				}
				str << desc << COLUMN_SEPARATOR << hotkey::get_names(item);
			}
		}

		result.push_back(str.str());
	}
	//If any of the menu items have an image, create an image column
	if (has_image) {
		for (std::vector<std::string>::iterator i = result.begin(); i != result.end(); ++i) {
			if (*(i->begin()) != IMAGE_PREFIX) {
				i->insert(i->begin(), COLUMN_SEPARATOR);
			}
		}
	}
	return result;
}
basic_handler::basic_handler(display* disp, command_executor* exec) : disp_(disp), exec_(exec) {}

void basic_handler::handle_event(const SDL_Event& event)
{
	//TODO this code path is never called?

	if (disp_ == NULL) {
		return;
	}

	switch (event.type) {
	case SDL_KEYDOWN:
		// If we're in a dialog we only want to handle items that are explicitly
		// handled by the executor.
		// If we're not in a dialog we can call the regular key event handler.
		if (!gui::in_dialog()) {
			key_event(*disp_, event.key,exec_);
		} else if (exec_ != NULL) {
			key_event_execute(*disp_, event.key,exec_);
		}
		break;
	case SDL_JOYBUTTONDOWN:
		if (!gui::in_dialog()) {
			jbutton_event(*disp_, event.jbutton,exec_);
		} else if (exec_ != NULL) {
			jbutton_event_execute(*disp_, event.jbutton,exec_);
		}
		break;
	case SDL_MOUSEBUTTONDOWN:
		if (!gui::in_dialog()) {
			mbutton_event(*disp_, event.button,exec_);
		} else if (exec_ != NULL) {
			mbutton_event_execute(*disp_, event.button,exec_);
		}
		break;
	}
}

void mbutton_event(display& disp, const SDL_MouseButtonEvent& event, command_executor* executor)
{
	mbutton_event_execute(disp, event, executor);
}

void jbutton_event(display& disp, const SDL_JoyButtonEvent& event, command_executor* executor)
{
	jbutton_event_execute(disp, event, executor);
}

void jhat_event(display& disp, const SDL_JoyHatEvent& event, command_executor* executor)
{
	jhat_event_execute(disp, event, executor);
}

void key_event(display& disp, const SDL_KeyboardEvent& event, command_executor* executor)
{
	if(event.keysym.sym == SDLK_ESCAPE && disp.in_game()) {
		LOG_G << "escape pressed..showing quit\n";
		const int res = gui2::show_message(disp.video(), _("Quit"),
				_("Do you really want to quit?"), gui2::tmessage::yes_no_buttons);
		if(res != gui2::twindow::CANCEL) {
			throw end_level_exception(QUIT);
		} else {
			return;
		}
	}

	key_event_execute(disp, event,executor);
}

void mbutton_event_execute(display& disp, const SDL_MouseButtonEvent& event, command_executor* executor)
{
	const hotkey_item* hk = &get_hotkey(event);
	if (!hk->active()) {
		return;
	}

	execute_command(disp, hotkey::get_hotkey_command(hk->get_command()), executor);
	executor->set_button_state(disp);
}

void jbutton_event_execute(display& disp, const SDL_JoyButtonEvent& event, command_executor* executor)
{
	const hotkey_item* hk = &get_hotkey(event);
	if (!hk->active()) {
		return;
	}

	execute_command(disp, get_hotkey_command(hk->get_command()), executor);
	executor->set_button_state(disp);
}

void jhat_event_execute(display& disp, const SDL_JoyHatEvent& event, command_executor* executor)
{
	const hotkey_item* hk = &get_hotkey(event);
	if (!hk->active()) {
		return;
	}

	execute_command(disp, get_hotkey_command(hk->get_command()), executor);
	executor->set_button_state(disp);
}

void key_event_execute(display& disp, const SDL_KeyboardEvent& event, command_executor* executor)
{
	const hotkey_item* hk = &get_hotkey(event);

#if 0
	// This is not generally possible without knowing keyboard layout.
	if (hk->null()) {
		//no matching hotkey was found, but try an in-exact match.
		hk = &get_hotkey(event, true);
	}
#endif

	if (!hk->active()) {
		return;
	}

	execute_command(disp, get_hotkey_command(hk->get_command()), executor);
	executor->set_button_state(disp);
}


void execute_command(display& disp, const hotkey_command& command, command_executor* executor, int index)
{
	const int zoom_amount = 4;
	bool map_screenshot = false;

	if (executor != NULL) {
		if (!executor->can_execute_command(command, index)
				|| executor->execute_command(command, index)) {
			return;
		}
	}
	switch (command.id) {

		case HOTKEY_MINIMAP_DRAW_TERRAIN:
			preferences::toggle_minimap_draw_terrain();
			disp.recalculate_minimap();
			break;
		case HOTKEY_MINIMAP_CODING_TERRAIN:
			preferences::toggle_minimap_terrain_coding();
			disp.recalculate_minimap();
			break;
		case HOTKEY_MINIMAP_CODING_UNIT:
			preferences::toggle_minimap_movement_coding();
			disp.recalculate_minimap();
			break;
		case HOTKEY_MINIMAP_DRAW_UNITS:
			preferences::toggle_minimap_draw_units();
			disp.redraw_minimap();
			break;
		case HOTKEY_MINIMAP_DRAW_VILLAGES:
			preferences::toggle_minimap_draw_villages();
			disp.recalculate_minimap();
			break;
		case HOTKEY_ZOOM_IN:
			disp.set_zoom(zoom_amount);
			break;
		case HOTKEY_ZOOM_OUT:
			disp.set_zoom(-zoom_amount);
			break;
		case HOTKEY_ZOOM_DEFAULT:
			disp.set_default_zoom();
			break;
		case HOTKEY_FULLSCREEN:
			preferences::set_fullscreen(!preferences::fullscreen());
			break;
		case HOTKEY_MAP_SCREENSHOT:
			if (!disp.in_game() && !disp.in_editor()) {
				break;
			}
			map_screenshot = true;
			// intentional fall-through
		case HOTKEY_SCREENSHOT: {
			std::string name = map_screenshot ? _("Map-Screenshot") : _("Screenshot");
			std::string filename = get_screenshot_dir() + "/" + name + "_";
			filename = get_next_filename(filename, ".bmp");
			int size = disp.screenshot(filename, map_screenshot);
			if (size > 0) {
				gui2::tscreenshot_notification::display(filename, size, disp.video());
			} else {
				gui2::show_message(disp.video(), _("Screenshot Done"), "");
			}
			break;
		}
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
		case HOTKEY_QUIT_GAME: {
			if (disp.in_game()) {
				DBG_G << "is in game -- showing quit message\n";
				const int res = gui2::show_message(disp.video(), _("Quit"),
						_("Do you really want to quit?"), gui2::tmessage::yes_no_buttons);
				if (res != gui2::twindow::CANCEL) {
					throw end_level_exception(QUIT);
				}
			}
			break;
		}
		default:
			DBG_G << "command_executor: unknown command number " << command.id << ", ignoring.\n";
			break;
	}
}
}
