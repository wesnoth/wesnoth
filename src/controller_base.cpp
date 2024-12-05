/*
	Copyright (C) 2003 - 2024
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

#include "controller_base.hpp"

#include "display.hpp"
#include "events.hpp"
#include "game_config_manager.hpp"
#include "hotkey/command_executor.hpp"
#include "log.hpp"
#include "mouse_handler_base.hpp"
#include "preferences/preferences.hpp"
#include "scripting/plugins/context.hpp"
#include "gui/core/event/handler.hpp" // gui2::is_in_dialog
#include "soundsource.hpp"
#include "gui/core/timer.hpp"
#include "sdl/input.hpp" // get_mouse_state
#include "video.hpp"

static lg::log_domain log_display("display");
#define ERR_DP LOG_STREAM(err, log_display)

using namespace std::chrono_literals;
static constexpr auto long_touch_duration = 800ms;

controller_base::controller_base()
	: game_config_(game_config_manager::get()->game_config())
	, key_()
	, scrolling_(false)
	, scroll_up_(false)
	, scroll_down_(false)
	, scroll_left_(false)
	, scroll_right_(false)
	, last_scroll_tick_()
	, scroll_carry_x_(0.0)
	, scroll_carry_y_(0.0)
	, key_release_listener_(*this)
	, last_mouse_is_touch_(false)
	, long_touch_timer_(0)
{
}

controller_base::~controller_base()
{
	if(long_touch_timer_ != 0) {
		gui2::remove_timer(long_touch_timer_);
		long_touch_timer_ = 0;
	}
}

void controller_base::long_touch_callback(int x, int y)
{
	if(long_touch_timer_ != 0 && !get_mouse_handler_base().dragging_started()) {
		int x_now;
		int y_now;
		uint32_t mouse_state = sdl::get_mouse_state(&x_now, &y_now);

#ifdef MOUSE_TOUCH_EMULATION
		if(mouse_state & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
			// Monkey-patch touch controls again to make them look like left button.
			mouse_state = SDL_BUTTON(SDL_BUTTON_LEFT);
		}
#endif

		// Workaround for double-menu b/c of slow events processing, or I don't know.
		int dx = x - x_now;
		int dy = y - y_now;
		int threshold = get_mouse_handler_base().drag_threshold();
		bool yes_actually_dragging = dx * dx + dy * dy >= threshold * threshold;

		if(!yes_actually_dragging
		   && (mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0
		   && get_display().map_area().contains(x_now, y_now))
		{
			const theme::menu* const m = get_mouse_handler_base().gui().get_theme().context_menu();
			if(m != nullptr) {
				show_menu(get_display().get_theme().context_menu()->items(), x_now, y_now, true, get_display());
			}
		}
	}

	long_touch_timer_ = 0;
}

void controller_base::handle_event(const SDL_Event& event)
{
	if(gui2::is_in_dialog()) {
		return;
	}

	events::mouse_handler_base& mh_base = get_mouse_handler_base();

	SDL_Event new_event = {};

	switch(event.type) {
	case SDL_TEXTINPUT:
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
		} else {
			process_focus_keydown_event(event);
		}
		break;

	case SDL_KEYUP:
		process_keyup_event(event);
		hotkey::key_event(event, get_hotkey_command_executor());
		break;

	case SDL_JOYBUTTONDOWN:
		hotkey::jbutton_event(event, get_hotkey_command_executor());
		break;

	case SDL_JOYHATMOTION:
		hotkey::jhat_event(event, get_hotkey_command_executor());
		break;

	case SDL_MOUSEMOTION:
		// Ignore old mouse motion events in the event queue
		if(SDL_PeepEvents(&new_event, 1, SDL_GETEVENT, SDL_MOUSEMOTION, SDL_MOUSEMOTION) > 0) {
			while(SDL_PeepEvents(&new_event, 1, SDL_GETEVENT, SDL_MOUSEMOTION, SDL_MOUSEMOTION) > 0) {
			};
			if(new_event.motion.which != SDL_TOUCH_MOUSEID) {
				mh_base.mouse_motion_event(new_event.motion, is_browsing());
			}
		} else {
			if(new_event.motion.which != SDL_TOUCH_MOUSEID) {
				mh_base.mouse_motion_event(event.motion, is_browsing());
			}
		}
		break;

	case SDL_FINGERMOTION:
		if(SDL_PeepEvents(&new_event, 1, SDL_GETEVENT, SDL_FINGERMOTION, SDL_FINGERMOTION) > 0) {
			while(SDL_PeepEvents(&new_event, 1, SDL_GETEVENT, SDL_FINGERMOTION, SDL_FINGERMOTION) > 0) {
			};
			mh_base.touch_motion_event(new_event.tfinger, is_browsing());
		} else {
			mh_base.touch_motion_event(event.tfinger, is_browsing());
		}
		break;

	case SDL_MOUSEBUTTONDOWN:
		last_mouse_is_touch_ = event.button.which == SDL_TOUCH_MOUSEID;

		if(last_mouse_is_touch_ && long_touch_timer_ == 0) {
			long_touch_timer_ = gui2::add_timer(
					long_touch_duration,
					std::bind(&controller_base::long_touch_callback, this, event.button.x, event.button.y));
		}

		mh_base.mouse_press(event.button, is_browsing());
		hotkey::mbutton_event(event, get_hotkey_command_executor());
		break;

	case SDL_FINGERDOWN:
		// handled by mouse case
		break;

	case SDL_MOUSEBUTTONUP:
		if(long_touch_timer_ != 0) {
			gui2::remove_timer(long_touch_timer_);
			long_touch_timer_ = 0;
		}

		last_mouse_is_touch_ = event.button.which == SDL_TOUCH_MOUSEID;

		mh_base.mouse_press(event.button, is_browsing());
		if(mh_base.get_show_menu()) {
			show_menu(get_display().get_theme().context_menu()->items(), event.button.x, event.button.y, true,
					get_display());
		}
		break;
	case DOUBLE_CLICK_EVENT:
		{
			int x = static_cast<int>(reinterpret_cast<std::intptr_t>(event.user.data1));
			int y = static_cast<int>(reinterpret_cast<std::intptr_t>(event.user.data2));
			if(event.user.code == static_cast<int>(SDL_TOUCH_MOUSEID)
			   // TODO: Move to right_click_show_menu?
			   && get_display().map_area().contains(x, y)
			   // TODO: This chain repeats in several places, move to a method.
			   && get_display().get_theme().context_menu() != nullptr) {
				show_menu(get_display().get_theme().context_menu()->items(),
						  x,
						  y,
						  true,
						  get_display());
			}
		}
		break;

	case SDL_FINGERUP:
		// handled by mouse case
		break;

	case SDL_MOUSEWHEEL:
		// Right and down are positive in Wesnoth's map.
		// Right and up are positive in SDL_MouseWheelEvent on all platforms:
		//     https://wiki.libsdl.org/SDL2/SDL_MouseWheelEvent
#if defined(_WIN32) || defined(__APPLE__)
		mh_base.mouse_wheel(event.wheel.x, -event.wheel.y, is_browsing());
#else
		// Except right is wrongly negative on X11 in SDL < 2.0.18:
		//     https://github.com/libsdl-org/SDL/pull/4700
		//     https://github.com/libsdl-org/SDL/commit/515b7e9
		// and on Wayland in SDL < 2.0.20:
		//     https://github.com/libsdl-org/SDL/commit/3e1b3bc
		// Fixes issues #3362 and #7404, which are a regression caused by pull #2481 that fixed issue #2218.
		{
			static int xmul = 0;
			if(xmul == 0) {
				xmul = 1;
				const char* video_driver = SDL_GetCurrentVideoDriver();
				SDL_version ver;
				SDL_GetVersion(&ver);
				if(video_driver != nullptr && ver.major <= 2 && ver.minor <= 0) {
					if(std::strcmp(video_driver, "x11") == 0 && ver.patch < 18) {
						xmul = -1;
					} else if(std::strcmp(video_driver, "wayland") == 0 && ver.patch < 20) {
						xmul = -1;
					}
				}
			}
			mh_base.mouse_wheel(xmul * event.wheel.x, -event.wheel.y, is_browsing());
		}
#endif
		break;

	case TIMER_EVENT:
		gui2::execute_timer(reinterpret_cast<size_t>(event.user.data1));
		break;

	// TODO: Support finger specifically, like pan the map. For now, SDL's "shadow mouse" events will do.
	case SDL_MULTIGESTURE:
	default:
		break;
	}
}

void controller_base::process()
{
	if(gui2::is_in_dialog()) {
		return;
	}

	hotkey::run_events(get_hotkey_command_executor());
}

void controller_base::keyup_listener::handle_event(const SDL_Event& event)
{
	if(event.type == SDL_KEYUP) {
		hotkey::keyup_event(event, controller_.get_hotkey_command_executor());
	}
}

bool controller_base::have_keyboard_focus()
{
	return true;
}

bool controller_base::handle_scroll(int mousex, int mousey, int mouse_flags)
{
	const bool mouse_in_window =
		video::window_has_mouse_focus()
		|| prefs::get().get_scroll_when_mouse_outside(true);

	int scroll_speed = prefs::get().scroll_speed();
	double dx = 0.0, dy = 0.0;

	int scroll_threshold = prefs::get().mouse_scrolling()
		? prefs::get().mouse_scroll_threshold()
		: 0;

	for(const theme::menu& m : get_display().get_theme().menus()) {
		if(m.get_location().contains(mousex, mousey)) {
			scroll_threshold = 0;
		}
	}

	// Scale scroll distance according to time passed
	auto tick_now = std::chrono::steady_clock::now();

	// If we weren't previously scrolling, start small.
	auto dt = 1ms;
	if (scrolling_) {
		dt = std::chrono::duration_cast<std::chrono::milliseconds>(tick_now - last_scroll_tick_);
	}

	// scroll_speed is in percent. Ticks are in milliseconds.
	// Let's assume the maximum speed (100) moves 50 hexes per second,
	// i.e. 3600 pixels per 1000 ticks.
	double scroll_amount = dt.count() * 0.036 * double(scroll_speed);
	last_scroll_tick_ = tick_now;

	// Apply keyboard scrolling
	dy -= scroll_up_    * scroll_amount;
	dy += scroll_down_  * scroll_amount;
	dx -= scroll_left_  * scroll_amount;
	dx += scroll_right_ * scroll_amount;

	// Scroll if mouse is placed near the edge of the screen
	if(mouse_in_window) {
		if(mousey < scroll_threshold) {
			dy -= scroll_amount;
		}

		if(mousey > video::game_canvas_size().y - scroll_threshold) {
			dy += scroll_amount;
		}

		if(mousex < scroll_threshold) {
			dx -= scroll_amount;
		}

		if(mousex > video::game_canvas_size().x - scroll_threshold) {
			dx += scroll_amount;
		}
	}

	events::mouse_handler_base& mh_base = get_mouse_handler_base();

	// Scroll with middle-mouse if enabled
	if((mouse_flags & SDL_BUTTON_MMASK) != 0 && prefs::get().middle_click_scrolls()) {
		const SDL_Point original_loc = mh_base.get_scroll_start();

		if(mh_base.scroll_started()) {
			if(get_display().map_outside_area().contains(mousex, mousey)
				&& mh_base.scroll_started())
			{
				// Scroll speed is proportional from the distance from the first
				// middle click and scrolling speed preference.
				const double speed = 0.01 * scroll_amount;
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

	// If nothing is scrolling, just return.
	if (!dx && !dy) {
		return false;
	}

	// If we are continuing a scroll, carry over any subpixel movement.
	if (scrolling_) {
		dx += scroll_carry_x_;
		dy += scroll_carry_y_;
	}
	point dist{int(dx), int(dy)};
	scroll_carry_x_ = dx - double(dist.x);
	scroll_carry_y_ = dy - double(dist.y);

	// Scroll the display
	get_display().scroll(dist);

	// Even if the integer parts are both zero, we are still scrolling.
	// The subpixel amounts will add up.
	return true;
}

void controller_base::play_slice()
{
	CKey key;

	if(plugins_context* l = get_plugins_context()) {
		l->play_slice();
	}

	events::pump();
	events::raise_process_event();
	events::draw();

	// Update sound sources before scrolling
	if(soundsource::manager* l = get_soundsource_man()) {
		l->update();
	}

	const theme::menu* const m = get_display().menu_pressed();
	if(m != nullptr) {
		const rect& menu_loc = m->location(video::game_canvas());
		show_menu(m->items(), menu_loc.x + 1, menu_loc.y + menu_loc.h + 1, false, get_display());

		return;
	}

	const theme::action* const a = get_display().action_pressed();
	if(a != nullptr) {
		const rect& action_loc = a->location(video::game_canvas());
		execute_action(a->items(), action_loc.x + 1, action_loc.y + action_loc.h + 1, false);

		return;
	}

	auto str_vec = additional_actions_pressed();
	if(!str_vec.empty()) {
		execute_action(str_vec, 0, 0, false);
		return;
	}

	bool was_scrolling = scrolling_;

	int mousex, mousey;
	uint8_t mouse_flags = sdl::get_mouse_state(&mousex, &mousey);

	scrolling_ = handle_scroll(mousex, mousey, mouse_flags);

	map_location highlighted_hex = get_display().mouseover_hex();

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
		const hotkey::ui_command cmd = hotkey::ui_command(id);

		if(cmd_exec->can_execute_command(cmd) && (!context_menu || in_context_menu(cmd))) {
			items.emplace_back(c);
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
		hotkey::ui_command cmd = hotkey::ui_command(item);
		if(cmd_exec->can_execute_command(cmd)) {
			items.push_back(item);
		}
	}

	if(items.empty()) {
		return;
	}

	cmd_exec->execute_action(items, xloc, yloc, context_menu, get_display());
}

bool controller_base::in_context_menu(const hotkey::ui_command& /*command*/) const
{
	return true;
}
