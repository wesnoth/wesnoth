/*
   Copyright (C) 2006 - 2018 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
   wesnoth playlevel Copyright (C) 2003 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "controller_base.hpp"

#include "display.hpp"
#include "events.hpp"
#include "gui/core/event/handler.hpp" // gui2::is_in_dialog
#include "gui/dialogs/loading_screen.hpp"
#include "hotkey/command_executor.hpp"
#include "hotkey/hotkey_command.hpp"
#include "log.hpp"
#include "map/map.hpp"
#include "mouse_handler_base.hpp"
#include "preferences/game.hpp"
#include "scripting/plugins/context.hpp"
#include "soundsource.hpp"

static lg::log_domain log_display("display");
#define ERR_DP LOG_STREAM(err, log_display)

controller_base::controller_base(const config& game_config)
	: events::sdl_handler(false)
	, game_config_(game_config)
	, key_()
	, scrolling_(false)
	, scroll_up_(false)
	, scroll_down_(false)
	, scroll_left_(false)
	, scroll_right_(false)
	, joystick_manager_()
	, key_release_listener_(*this)
{
}

controller_base::~controller_base()
{
}

void controller_base::handle_event(const SDL_Event& event)
{
	/* TODO: since GUI2 and the main game are now part of the same event context, there is some conflict
	 * between the GUI2 and event handlers such as these. By design, the GUI2 sdl handler is always on top
	 * of the handler queue, so its events are handled last. This means events here have a chance to fire
	 * first. have_keyboard_focus currently returns false if a dialog open, but this is just as stopgap
	 * measure. We need to figure out a better way to filter out events.
	 */
	//if(gui2::is_in_dialog()) {
	//	return;
	//}

	if(gui2::dialogs::loading_screen::displaying()) {
		return;
	}

	events::mouse_handler_base& mh_base = get_mouse_handler_base();

	switch(event.type) {
	case SDL_TEXTINPUT:
		// TODO: remove, not necessary anymore
		if(have_keyboard_focus()) {
			hotkey::key_event(event, get_hotkey_command_executor());
		}
		break;

	case SDL_TEXTEDITING:
		if(have_keyboard_focus()) {
			SDL_Event evt = event;
			evt.type = SDL_TEXTINPUT;
			hotkey::key_event(evt, get_hotkey_command_executor());
			SDL_StopTextInput();
			SDL_StartTextInput();
		}
		break;

	case SDL_KEYDOWN:
		// Detect key press events, unless there something that has keyboard focus
		// in which case the key press events should go only to it.
		if(have_keyboard_focus()) {
			if(event.key.keysym.sym == SDLK_ESCAPE) {
				get_hotkey_command_executor()->execute_quit_command();
				break;
			}

			process_keydown_event(event);
			hotkey::key_event(event, get_hotkey_command_executor());
			process_keyup_event(event);
		}
		break;

	case SDL_KEYUP:
		process_keyup_event(event);
		hotkey::key_event(event, get_hotkey_command_executor());
		break;

	case SDL_JOYBUTTONDOWN:
		process_keydown_event(event);
		hotkey::jbutton_event(event, get_hotkey_command_executor());
		break;

	case SDL_JOYHATMOTION:
		process_keydown_event(event);
		hotkey::jhat_event(event, get_hotkey_command_executor());
		break;

	case SDL_MOUSEMOTION:
		// Ignore old mouse motion events in the event queue
		SDL_Event new_event;
		if(SDL_PeepEvents(&new_event, 1, SDL_GETEVENT, SDL_MOUSEMOTION, SDL_MOUSEMOTION) > 0) {
			while(SDL_PeepEvents(&new_event, 1, SDL_GETEVENT, SDL_MOUSEMOTION, SDL_MOUSEMOTION) > 0) {
			};
			mh_base.mouse_motion_event(new_event.motion, is_browsing());
		} else {
			mh_base.mouse_motion_event(event.motion, is_browsing());
		}
		break;

	case SDL_MOUSEBUTTONDOWN:
		process_keydown_event(event);
		mh_base.mouse_press(event.button, is_browsing());
		hotkey::mbutton_event(event, get_hotkey_command_executor());
		break;

	case SDL_MOUSEBUTTONUP:
		mh_base.mouse_press(event.button, is_browsing());
		if(mh_base.get_show_menu()) {
			show_menu(get_display().get_theme().context_menu()->items(), event.button.x, event.button.y, true,
					get_display());
		}
		break;

	case SDL_MOUSEWHEEL:
#if defined(_WIN32) || defined(__APPLE__)
		mh_base.mouse_wheel(-event.wheel.x, event.wheel.y, is_browsing());
#else
		mh_base.mouse_wheel(event.wheel.x, event.wheel.y, is_browsing());
#endif
		break;

	default:
		break;
	}
}

void controller_base::keyup_listener::handle_event(const SDL_Event& event)
{
	if(event.type == SDL_KEYUP) {
		hotkey::keyup_event(event, controller_.get_hotkey_command_executor());
	}
}

bool controller_base::have_keyboard_focus()
{
	return !gui2::is_in_dialog();
}

bool controller_base::handle_scroll(int mousex, int mousey, int mouse_flags, double x_axis, double y_axis)
{
	const bool mouse_in_window =
		CVideo::get_singleton().window_has_flags(SDL_WINDOW_MOUSE_FOCUS)
		|| preferences::get("scroll_when_mouse_outside", true);

	int scroll_speed = preferences::scroll_speed();
	int dx = 0, dy = 0;

	int scroll_threshold = preferences::mouse_scroll_enabled()
		? preferences::mouse_scroll_threshold()
		: 0;

	for(const theme::menu& m : get_display().get_theme().menus()) {
		if(sdl::point_in_rect(mousex, mousey, m.get_location())) {
			scroll_threshold = 0;
		}
	}

	// Apply keyboard scrolling
	dy -= scroll_up_    * scroll_speed;
	dy += scroll_down_  * scroll_speed;
	dx -= scroll_left_  * scroll_speed;
	dx += scroll_right_ * scroll_speed;

	// Scroll if mouse is placed near the edge of the screen
	if(mouse_in_window) {
		if(mousey < scroll_threshold) {
			dy -= scroll_speed;
		}

		if(mousey > get_display().video().get_height() - scroll_threshold) {
			dy += scroll_speed;
		}

		if(mousex < scroll_threshold) {
			dx -= scroll_speed;
		}

		if(mousex > get_display().video().get_width() - scroll_threshold) {
			dx += scroll_speed;
		}
	}

	events::mouse_handler_base& mh_base = get_mouse_handler_base();

	// Scroll with middle-mouse if enabled
	if((mouse_flags & SDL_BUTTON_MMASK) != 0 && preferences::middle_click_scrolls()) {
		const SDL_Point original_loc = mh_base.get_scroll_start();

		if(mh_base.scroll_started()) {
			const SDL_Rect& rect = get_display().map_outside_area();

			if(sdl::point_in_rect(mousex, mousey, rect) && mh_base.scroll_started()) {
				// Scroll speed is proportional from the distance from the first
				// middle click and scrolling speed preference.
				const double speed = 0.04 * sqrt(static_cast<double>(scroll_speed));
				const double snap_dist = 16; // Snap to horizontal/vertical scrolling
				const double x_diff = (mousex - original_loc.x);
				const double y_diff = (mousey - original_loc.y);

				if(std::fabs(x_diff) > snap_dist || std::fabs(y_diff) <= snap_dist) {
					dx += speed * x_diff;
				}

				if(std::fabs(y_diff) > snap_dist || std::fabs(x_diff) <= snap_dist) {
					dy += speed * y_diff;
				}
			}
		} else { // Event may fire mouse down out of order with respect to initial click
			mh_base.set_scroll_start(mousex, mousey);
		}
	}

	// scroll with joystick
	dx += round_double(x_axis * scroll_speed);
	dy += round_double(y_axis * scroll_speed);

	return get_display().scroll(dx, dy);
}

void controller_base::play_slice(bool is_delay_enabled)
{
	CKey key;

	if(plugins_context* l = get_plugins_context()) {
		l->play_slice();
	}

	events::run_event_loop();
	events::raise_process_event();

	// Update sound sources before scrolling
	if(soundsource::manager* l = get_soundsource_man()) {
		l->update();
	}

#if 0
	const theme::menu* const m = get_display().menu_pressed();
	if(m != nullptr) {
		const SDL_Rect& menu_loc = m->location(get_display().video().screen_area());
		show_menu(m->items(), menu_loc.x + 1, menu_loc.y + menu_loc.h + 1, false, get_display());

		return;
	}

	const theme::action* const a = get_display().action_pressed();
	if(a != nullptr) {
		const SDL_Rect& action_loc = a->location(get_display().video().screen_area());
		execute_action(a->items(), action_loc.x + 1, action_loc.y + action_loc.h + 1, false);

		return;
	}
#endif

	auto str_vec = additional_actions_pressed();
	if(!str_vec.empty()) {
		execute_action(str_vec, 0, 0, false);
		return;
	}

	bool was_scrolling = scrolling_;

	std::pair<double, double> values = joystick_manager_.get_scroll_axis_pair();
	const double joystickx = values.first;
	const double joysticky = values.second;

	int mousex, mousey;
	uint8_t mouse_flags = SDL_GetMouseState(&mousex, &mousey);

	// TODO enable after an axis choosing mechanism is implemented
#if 0
	std::pair<double, double> values = joystick_manager_.get_mouse_axis_pair();
	mousex += values.first * 10;
	mousey += values.second * 10;
	SDL_WarpMouse(mousex, mousey);
#endif

	scrolling_ = handle_scroll(mousex, mousey, mouse_flags, joystickx, joysticky);

	map_location highlighted_hex = get_display().mouseover_hex();

	// TODO: enable when the relative cursor movement is implemented well enough
#if 0
	const map_location& selected_hex = get_display().selected_hex();

	if (selected_hex != map_location::null_location()) {
		if (joystick_manager_.next_highlighted_hex(highlighted_hex, selected_hex)) {
			get_mouse_handler_base().mouse_motion(0,0, true, true, highlighted_hex);
			get_display().scroll_to_tile(highlighted_hex, display::ONSCREEN_WARP, false, true);
			scrolling_ = true;
		}
	} else
#endif

	if(joystick_manager_.update_highlighted_hex(highlighted_hex) && get_display().get_map().on_board(highlighted_hex)) {
		get_mouse_handler_base().mouse_motion(0, 0, true, true, highlighted_hex);
		get_display().scroll_to_tile(highlighted_hex, display::ONSCREEN_WARP, false, true);
		scrolling_ = true;
	}

	// be nice when window is not visible
	// NOTE should be handled by display instead, to only disable drawing
	if(is_delay_enabled && !CVideo::get_singleton().window_has_flags(SDL_WINDOW_SHOWN)) {
		CVideo::delay(200);
	}

	// Scrolling ended, update the cursor and the brightened hex
	if(!scrolling_ && was_scrolling) {
		get_mouse_handler_base().mouse_update(is_browsing(), highlighted_hex);
	}
}

void controller_base::show_menu(
		const std::vector<config>& items_arg, int xloc, int yloc, bool context_menu, display& disp)
{
	hotkey::command_executor* cmd_exec = get_hotkey_command_executor();
	if(!cmd_exec) {
		return;
	}

	std::vector<config> items;
	for(const config& c : items_arg) {
		const std::string& id = c["id"];
		const hotkey::hotkey_command& command = hotkey::get_hotkey_command(id);

		if(cmd_exec->can_execute_command(command) && (!context_menu || in_context_menu(command.id))) {
			items.emplace_back("id", id);
		}
	}

	if(items.empty()) {
		return;
	}

	cmd_exec->show_menu(items, xloc, yloc, context_menu, disp);
}

void controller_base::execute_action(const std::vector<std::string>& items_arg, int xloc, int yloc, bool context_menu)
{
	hotkey::command_executor* cmd_exec = get_hotkey_command_executor();
	if(!cmd_exec) {
		return;
	}

	std::vector<std::string> items;
	for(const std::string& item : items_arg) {
		const hotkey::hotkey_command& command = hotkey::get_hotkey_command(item);
		if(cmd_exec->can_execute_command(command)) {
			items.push_back(item);
		}
	}

	if(items.empty()) {
		return;
	}

	cmd_exec->execute_action(items, xloc, yloc, context_menu, get_display());
}

bool controller_base::in_context_menu(hotkey::HOTKEY_COMMAND /*command*/) const
{
	return true;
}

const config& controller_base::get_theme(const config& game_config, std::string theme_name)
{
	if(theme_name.empty()) {
		theme_name = preferences::theme();
	}

	if(const config& c = game_config.find_child("theme", "id", theme_name)) {
		return c;
	}

	ERR_DP << "Theme '" << theme_name << "' not found. Trying the default theme." << std::endl;

	if(const config& c = game_config.find_child("theme", "id", "Default")) {
		return c;
	}

	ERR_DP << "Default theme not found." << std::endl;

	static config empty;
	return empty;
}
