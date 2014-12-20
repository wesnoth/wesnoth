/*
   Copyright (C) 2006 - 2014 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
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
#include "resources.hpp"
#include "play_controller.hpp"
#include "scripting/plugins/context.hpp"
#include "soundsource.hpp"

#include <boost/foreach.hpp>
#include <boost/scoped_ptr.hpp>

static lg::log_domain log_display("display");
#define ERR_DP LOG_STREAM(err, log_display)

controller_base::controller_base(
		const int ticks, const config& game_config, CVideo& /*video*/) :
	game_config_(game_config),
	ticks_(ticks),
	key_(),
	browse_(false),
	scrolling_(false),
	joystick_manager_(),
	plugins_context_(),
	soundsources_()
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
		get_mouse_handler_base().mouse_press(event.button, browse_);
		if (get_mouse_handler_base().get_show_menu()){
			show_menu(get_display().get_theme().context_menu()->items(),event.button.x,event.button.y,true, get_display());
		}
		hotkey::mbutton_event(get_display(), event.button, this);
		break;
	case SDL_MOUSEBUTTONUP:
		get_mouse_handler_base().mouse_press(event.button, browse_);
		if (get_mouse_handler_base().get_show_menu()){
			show_menu(get_display().get_theme().context_menu()->items(),event.button.x,event.button.y,true, get_display());
		}
		break;
#if !SDL_VERSION_ATLEAST(2, 0, 0)
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
#endif
#if SDL_VERSION_ATLEAST(2,0,0)
	case SDL_MOUSEWHEEL:
		get_mouse_handler_base().mouse_wheel(event.wheel.x, event.wheel.y, browse_);
		break;
#endif
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
		if (sdl::point_in_rect(mousex, mousey, m.get_location())) {
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
		const map_location original_loc = get_mouse_handler_base().get_scroll_start();

		if (get_mouse_handler_base().scroll_started()) {
			const SDL_Rect& rect = get_display().map_outside_area();
			if (sdl::point_in_rect(mousex, mousey,rect) &&
				get_mouse_handler_base().scroll_started()) {
				// Scroll speed is proportional from the distance from the first
				// middle click and scrolling speed preference.
				const double speed = 0.04 * sqrt(static_cast<double>(scroll_speed));
				const double snap_dist = 16; // Snap to horizontal/vertical scrolling
				const double x_diff = (mousex - original_loc.x);
				const double y_diff = (mousey - original_loc.y);

				if (fabs(x_diff) > snap_dist || fabs(y_diff) <= snap_dist) dx += speed * x_diff;
				if (fabs(y_diff) > snap_dist || fabs(x_diff) <= snap_dist) dy += speed * y_diff;
			}
		}
		else { // Event may fire mouse down out of order with respect to initial click
			get_mouse_handler_base().set_scroll_start(mousex, mousey);
		}
	}

	dx += round_double( x_axis * scroll_speed);
	dy += round_double( y_axis * scroll_speed);

	return get_display().scroll(dx, dy);
}

void controller_base::play_slice(bool is_delay_enabled)
{
	CKey key;

	if (boost::shared_ptr<plugins_context> l = plugins_context_.lock()) {
		l->play_slice();
	}

	events::pump();
	events::raise_process_event();
	events::raise_draw_event();

	// Update sound sources before scrolling
	if (boost::shared_ptr<soundsource::manager> l = soundsources_.lock()) {
		l->update();
	}

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

	if (selected_hex != map_location::null_location()) {
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
}

void controller_base::show_menu(const std::vector<std::string>& items_arg, int xloc, int yloc, bool context_menu, display& disp)
{
	std::vector<std::string> items = items_arg;
	std::vector<std::string>::iterator i = items.begin();
	while(i != items.end()) {
		const hotkey::hotkey_command& command = hotkey::get_hotkey_command(*i);
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

		const hotkey::hotkey_command& command = hotkey::get_hotkey_command(item);
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

	if (const config &c = game_config.find_child("theme", "id", theme_name))
		return c;

	// Themes created for version 1.11.9 and earlier use name= for
	// untranslatable ids.
	// TODO: remove support for this in 1.13.x (1.13.2?).
	if (const config &c = game_config.find_child("theme", "name", theme_name)) {
		ERR_DP << "Theme '" << theme_name << "' uses [theme] name= instead of id= to specify its id; this usage is deprecated and will be removed in version 1.13.x." << std::endl;
		return c;
	}

	ERR_DP << "Theme '" << theme_name << "' not found. Trying the default theme." << std::endl;

	if (const config &c = game_config.find_child("theme", "id", "Default"))
		return c;

	ERR_DP << "Default theme not found." << std::endl;

	static config empty;
	return empty;
}

plugins_context * controller_base::get_plugins_context() {
	if (boost::shared_ptr<plugins_context> l = plugins_context_.lock()) {
		return l.get();
	}
	return NULL;
}

void controller_base::set_plugins_context(const boost::shared_ptr<plugins_context> & ptr)
{
	plugins_context_ = ptr;
}

void controller_base::set_soundsource_manager(const boost::shared_ptr<soundsource::manager> & ptr)
{
	soundsources_ = ptr;
}
