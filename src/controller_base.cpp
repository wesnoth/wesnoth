/*
   Copyright (C) 2006 - 2013 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
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

#include "dialogs.hpp"
#include "display.hpp"
#include "game_preferences.hpp"
#include "log.hpp"
#include "mouse_handler_base.hpp"

#include <boost/foreach.hpp>

static lg::log_domain log_display("display");
#define ERR_DP LOG_STREAM(err, log_display)

controller_base::controller_base(
		const int ticks, const config& game_config, CVideo& /*video*/) :
	game_config_(game_config),
	ticks_(ticks),
	key_(),
	browse_(false),
	scrolling_(false),
	joystick_manager_()
{
}

controller_base::~controller_base()
{
}

int controller_base::get_ticks() {
	return ticks_;
}

void controller_base::handle_event(const SDL_Event& event)
{
	if(gui::in_dialog()) {
		return;
	}

	switch(event.type) {
	case SDL_KEYDOWN:
		// Detect key press events, unless there something that has keyboard focus
		// in which case the key press events should go only to it.
		if(have_keyboard_focus()) {
			process_keydown_event(event);
			hotkey::key_event(get_display(), event.key,this);
		} else {
			process_focus_keydown_event(event);
			break;
		}
		// intentionally fall-through
	case SDL_KEYUP:
		process_keyup_event(event);
		break;
	case SDL_JOYBUTTONDOWN:
		process_keydown_event(event);
		hotkey::jbutton_event(get_display(), event.jbutton, this);
		break;
	case SDL_JOYHATMOTION:
		process_keydown_event(event);
		hotkey::jhat_event(get_display(), event.jhat, this);
		break;
	case SDL_MOUSEMOTION:
		// Ignore old mouse motion events in the event queue
		SDL_Event new_event;
		if(SDL_PeepEvents(&new_event,1,SDL_GETEVENT,
					SDL_EVENTMASK(SDL_MOUSEMOTION)) > 0) {
			while(SDL_PeepEvents(&new_event,1,SDL_GETEVENT,
						SDL_EVENTMASK(SDL_MOUSEMOTION)) > 0) {};
			get_mouse_handler_base().mouse_motion_event(new_event.motion, browse_);
		} else {
			get_mouse_handler_base().mouse_motion_event(event.motion, browse_);
		}
		break;
	case SDL_MOUSEBUTTONDOWN:
		process_keydown_event(event);
		hotkey::mbutton_event(get_display(), event.button, this);
		// intentionally fall-through
	case SDL_MOUSEBUTTONUP:
		get_mouse_handler_base().mouse_press(event.button, browse_);
		if (get_mouse_handler_base().get_show_menu()){
			show_menu(get_display().get_theme().context_menu()->items(),event.button.x,event.button.y,true, get_display());
		}
		break;
	case SDL_ACTIVEEVENT:
		if (event.active.state == SDL_APPMOUSEFOCUS && event.active.gain == 0) {
			if (get_mouse_handler_base().is_dragging()) {
				//simulate mouse button up when the app has lost mouse focus
				//this should be a general fix for the issue when the mouse
				//is dragged out of the game window and then the button is released
				int x, y;
				Uint8 mouse_flags = SDL_GetMouseState(&x, &y);
				if ((mouse_flags & SDL_BUTTON_LEFT) == 0) {
					get_mouse_handler_base().mouse_press(event.button, browse_);
				}
			}
		}
		break;
	default:
		break;
	}
}

bool controller_base::have_keyboard_focus()
{
	return true;
}

void controller_base::process_focus_keydown_event(const SDL_Event& /*event*/) {
	//no action by default
}

void controller_base::process_keydown_event(const SDL_Event& /*event*/) {
	//no action by default
}

void controller_base::process_keyup_event(const SDL_Event& /*event*/) {
	//no action by default
}

bool controller_base::handle_scroll(CKey& key, int mousex, int mousey, int mouse_flags, double x_axis, double y_axis)
{
	bool mouse_in_window = (SDL_GetAppState() & SDL_APPMOUSEFOCUS) != 0
		|| preferences::get("scroll_when_mouse_outside", true);
	bool keyboard_focus = have_keyboard_focus();
	int scroll_speed = preferences::scroll_speed();
	int dx = 0, dy = 0;
	int scroll_threshold = (preferences::mouse_scroll_enabled())
		? preferences::mouse_scroll_threshold() : 0;
	BOOST_FOREACH(const theme::menu& m, get_display().get_theme().menus()) {
		if (point_in_rect(mousex, mousey, m.get_location())) {
			scroll_threshold = 0;
		}
	}
	if ((key[SDLK_UP] && keyboard_focus) ||
	    (mousey < scroll_threshold && mouse_in_window))
	{
		dy -= scroll_speed;
	}
	if ((key[SDLK_DOWN] && keyboard_focus) ||
	    (mousey > get_display().h() - scroll_threshold && mouse_in_window))
	{
		dy += scroll_speed;
	}
	if ((key[SDLK_LEFT] && keyboard_focus) ||
	    (mousex < scroll_threshold && mouse_in_window))
	{
		dx -= scroll_speed;
	}
	if ((key[SDLK_RIGHT] && keyboard_focus) ||
	    (mousex > get_display().w() - scroll_threshold && mouse_in_window))
	{
		dx += scroll_speed;
	}
	if ((mouse_flags & SDL_BUTTON_MMASK) != 0 && preferences::middle_click_scrolls()) {
		const SDL_Rect& rect = get_display().map_outside_area();
		if (point_in_rect(mousex, mousey,rect)) {
			// relative distance from the center to the border
			// NOTE: the view is a rectangle, so can be more sensible in one direction
			// but seems intuitive to use and it's useful since you must
			// more often scroll in the direction where the view is shorter
			const double xdisp = ((1.0*mousex / rect.w) - 0.5);
			const double ydisp = ((1.0*mousey / rect.h) - 0.5);
			// 4.0 give twice the normal speed when mouse is at border (xdisp=0.5)
			int speed = 4 * scroll_speed;
			dx += round_double(xdisp * speed);
			dy += round_double(ydisp * speed);
		}
	}

	dx += round_double( x_axis * scroll_speed);
	dy += round_double( y_axis * scroll_speed);

	return get_display().scroll(dx, dy);
}

void controller_base::play_slice(bool is_delay_enabled)
{
	CKey key;
	events::pump();
	events::raise_process_event();
	events::raise_draw_event();

	slice_before_scroll();
	const theme::menu* const m = get_display().menu_pressed();
	if(m != NULL) {
		const SDL_Rect& menu_loc = m->location(get_display().screen_area());
		show_menu(m->items(),menu_loc.x+1,menu_loc.y + menu_loc.h + 1,false, get_display());

		return;
	}
	const theme::action* const a = get_display().action_pressed();
	if(a != NULL) {
		const SDL_Rect& action_loc = a->location(get_display().screen_area());
		execute_action(a->items(), action_loc.x+1, action_loc.y + action_loc.h + 1,false);

		return;
	}

	bool was_scrolling = scrolling_;

	std::pair<double, double> values = joystick_manager_.get_scroll_axis_pair();
	const double joystickx = values.first;
	const double joysticky = values.second;

	int mousex, mousey;
	Uint8 mouse_flags = SDL_GetMouseState(&mousex, &mousey);

	/* TODO fendrin enable after an axis choosing mechanism is implemented
	std::pair<double, double> values = joystick_manager_.get_mouse_axis_pair();
	mousex += values.first * 10;
	mousey += values.second * 10;
	SDL_WarpMouse(mousex, mousey);
	*/
	scrolling_ = handle_scroll(key, mousex, mousey, mouse_flags, joystickx, joysticky);

	map_location highlighted_hex = get_display().mouseover_hex();

	/* TODO fendrin enable when the relative cursor movement is implemented well enough
	const map_location& selected_hex = get_display().selected_hex();

	if (selected_hex != map_location::null_location) {
		if (joystick_manager_.next_highlighted_hex(highlighted_hex, selected_hex)) {
			get_mouse_handler_base().mouse_motion(0,0, true, true, highlighted_hex);
			get_display().scroll_to_tile(highlighted_hex, display::ONSCREEN_WARP, false, true);
			scrolling_ = true;
		}
	} else */

	if (joystick_manager_.update_highlighted_hex(highlighted_hex)
			&& get_display().get_map().on_board(highlighted_hex)) {
		get_mouse_handler_base().mouse_motion(0,0, true, true, highlighted_hex);
		get_display().scroll_to_tile(highlighted_hex, display::ONSCREEN_WARP, false, true);
		scrolling_ = true;
		}

	get_display().draw();

	// be nice when window is not visible
	// NOTE should be handled by display instead, to only disable drawing
	if (is_delay_enabled && (SDL_GetAppState() & SDL_APPACTIVE) == 0) {
		get_display().delay(200);
	}

	if (!scrolling_ && was_scrolling) {
		// scrolling ended, update the cursor and the brightened hex
		get_mouse_handler_base().mouse_update(browse_, highlighted_hex);
	}
	slice_end();
}

void controller_base::slice_before_scroll()
{
	//no action by default
}

void controller_base::slice_end()
{
	//no action by default
}

void controller_base::show_menu(const std::vector<std::string>& items_arg, int xloc, int yloc, bool context_menu, display& disp)
{
	std::vector<std::string> items = items_arg;
	std::vector<std::string>::iterator i = items.begin();
	while(i != items.end()) {
		hotkey::hotkey_command& command = hotkey::get_hotkey_command(*i);
		if(!can_execute_command(command)
			|| (context_menu && !in_context_menu(command.id))) {
			i = items.erase(i);
			continue;
		}
		++i;
	}
	if(items.empty())
		return;
	command_executor::show_menu(items, xloc, yloc, context_menu, disp);
}

void controller_base::execute_action(const std::vector<std::string>& items_arg, int xloc, int yloc, bool context_menu)
{

	std::vector<std::string> items;
	BOOST_FOREACH(const std::string& item, items_arg) {

		hotkey::hotkey_command& command = hotkey::get_hotkey_command(item);
		if(can_execute_command(command))
			items.push_back(item);
	}

	if(items.empty())
		return;
	command_executor::execute_action(items, xloc, yloc, context_menu, get_display());
}



bool controller_base::in_context_menu(hotkey::HOTKEY_COMMAND /*command*/) const
{
	return true;
}

const config& controller_base::get_theme(const config& game_config, std::string theme_name)
{
	if (theme_name.empty()) theme_name = preferences::theme();

	if (const config &c = game_config.find_child("theme", "name", theme_name))
		return c;

	ERR_DP << "Theme '" << theme_name << "' not found. Trying the default theme.\n";

	if (const config &c = game_config.find_child("theme", "name", "Default"))
		return c;

	ERR_DP << "Default theme not found.\n";

	static config empty;
	return empty;
}
