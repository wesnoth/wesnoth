/*
   Copyright (C) 2006 - 2017 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
   wesnoth playturn Copyright (C) 2003 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "mouse_handler_base.hpp"

#include "cursor.hpp"
#include "display.hpp"
#include "log.hpp"
#include "preferences/general.hpp"
#include "tooltips.hpp"
#include "sdl/rect.hpp"

static lg::log_domain log_display("display");
#define WRN_DP LOG_STREAM(warn, log_display)

namespace events {

command_disabler::command_disabler()
{
	++commands_disabled;
}

command_disabler::~command_disabler()
{
	--commands_disabled;
}

int commands_disabled= 0;

static bool command_active()
{
#ifdef __APPLE__
	return (SDL_GetModState()&KMOD_CTRL) != 0;
#else
	return false;
#endif
}

mouse_handler_base::mouse_handler_base() :
	simple_warp_(false),
	minimap_scrolling_(false),
	dragging_left_(false),
	dragging_started_(false),
	dragging_right_(false),
	drag_from_x_(0),
	drag_from_y_(0),
	drag_from_hex_(),
	last_hex_(),
	show_menu_(false),
	scroll_start_x_(0),
	scroll_start_y_(0),
	scroll_started_(false)
{
}

bool mouse_handler_base::is_dragging() const
{
	return dragging_left_ || dragging_right_;
}

void mouse_handler_base::mouse_motion_event(const SDL_MouseMotionEvent& event, const bool browse)
{
	mouse_motion(event.x,event.y, browse);
}

void mouse_handler_base::mouse_update(const bool browse, map_location loc)
{
	int x, y;
	SDL_GetMouseState(&x,&y);
	mouse_motion(x, y, browse, true, loc);
}

#define MOUSE_TOUCH_EMULATION

bool mouse_handler_base::mouse_motion_default(int x, int y, bool /*update*/)
{
	tooltips::process(x, y);

	if(simple_warp_) {
		return true;
	}

	if(minimap_scrolling_) {
		//if the game is run in a window, we could miss a LMB/MMB up event
		// if it occurs outside our window.
		// thus, we need to check if the LMB/MMB is still down
		minimap_scrolling_ = ((SDL_GetMouseState(nullptr,nullptr) & (SDL_BUTTON(1) | SDL_BUTTON(2))) != 0);
		if(minimap_scrolling_) {
			const map_location& loc = gui().minimap_location_on(x,y);
			if(loc.valid()) {
				if(loc != last_hex_) {
					last_hex_ = loc;
					gui().scroll_to_tile(loc,display::WARP,false);
				}
			} else {
				// clicking outside of the minimap will end minimap scrolling
				minimap_scrolling_ = false;
			}
		}
		if(minimap_scrolling_) return true;
	}

	// Fire the drag & drop only after minimal drag distance
	// While we check the mouse buttons state, we also grab fresh position data.
	int mx = drag_from_x_; // some default value to prevent unlikely SDL bug
	int my = drag_from_y_;
	if (is_dragging() && !dragging_started_) {
		Uint32 mouse_state = dragging_left_ || dragging_right_ ? SDL_GetMouseState(&mx, &my) : 0;
#ifdef MOUSE_TOUCH_EMULATION
		if (dragging_left_ && (mouse_state & SDL_BUTTON(SDL_BUTTON_RIGHT))) {
			// Monkey-patch touch controls again to make them look like left button.
			mouse_state = SDL_BUTTON(SDL_BUTTON_LEFT);
		}
#endif
		if ((dragging_left_ && (mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0)
			|| (dragging_right_ && (mouse_state & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0)) {
			const double drag_distance = std::pow(static_cast<double>(drag_from_x_- mx), 2)
					+ std::pow(static_cast<double>(drag_from_y_- my), 2);
			if (drag_distance > drag_threshold()*drag_threshold()) {
				dragging_started_ = true;
				cursor::set_dragging(true);
			}
		}
	}
	return false;
}

void mouse_handler_base::mouse_press(const SDL_MouseButtonEvent& event, const bool browse)
{
	if(is_middle_click(event) && !preferences::middle_click_scrolls()) {
		simple_warp_ = true;
	}
	show_menu_ = false;
	map_location loc = gui().hex_clicked_on(event.x,event.y);
	mouse_update(browse, loc);
	
	static time_t touch_timestamp = 0;
	
	if (is_touch_click(event)) {
		if (event.state == SDL_PRESSED) {
			cancel_dragging();
			touch_timestamp = time(NULL);
			init_dragging(dragging_left_);
			left_click(event.x, event.y, browse);
		} else if (event.state == SDL_RELEASED) {
			minimap_scrolling_ = false;

			if (!dragging_started_ && touch_timestamp > 0) {
				time_t dt = clock() - touch_timestamp;
				// I couldn't make this work. Sorry for some C.
//				auto dt_cpp = high_resolution_clock::now() - touch_timestamp_cpp;
//				auto dt2 = duration_cast<milliseconds>(dt_cpp);
//				auto menu_hold = milliseconds(300);
//				if (dt2 > menu_hold) {
				if (dt > CLOCKS_PER_SEC * 3 / 10) {
					right_click(event.x, event.y, browse); // show_menu_ = true;
				}
			} else {
				touch_timestamp = 0;
			}

			clear_dragging(event, browse);
			left_mouse_up(event.x, event.y, browse);
		}
	} else if (is_left_click(event)) {
		if (event.state == SDL_PRESSED) {
			cancel_dragging();
			init_dragging(dragging_left_);
			left_click(event.x, event.y, browse);
		} else if (event.state == SDL_RELEASED) {
			minimap_scrolling_ = false;
			clear_dragging(event, browse);
			left_mouse_up(event.x, event.y, browse);
		}
	} else if (is_right_click(event)) {
		if (event.state == SDL_PRESSED) {
			cancel_dragging();
			init_dragging(dragging_right_);
			right_click(event.x, event.y, browse);
		} else if (event.state == SDL_RELEASED) {
			minimap_scrolling_ = false;
			clear_dragging(event, browse);
			right_mouse_up(event.x, event.y, browse);
		}
	} else if (is_middle_click(event)) {
		if (event.state == SDL_PRESSED) {
			set_scroll_start(event.x, event.y);
			scroll_started_ = true;

			map_location minimap_loc = gui().minimap_location_on(event.x,event.y);
			minimap_scrolling_ = false;
			if(minimap_loc.valid()) {
				simple_warp_ = false;
				minimap_scrolling_ = true;
				last_hex_ = minimap_loc;
				gui().scroll_to_tile(minimap_loc,display::WARP,false);
			} else if(simple_warp_) {
				// middle click not on minimap, check gamemap instead
				if(loc.valid()) {
					last_hex_ = loc;
					gui().scroll_to_tile(loc,display::WARP,false);
				}
			}
		} else if (event.state == SDL_RELEASED) {
			minimap_scrolling_ = false;
			simple_warp_ = false;
			scroll_started_ = false;
		}
	}
	if (!dragging_left_ && !dragging_right_ && dragging_started_) {
		dragging_started_ = false;
		cursor::set_dragging(false);
	}
	mouse_update(browse, loc);
}

bool mouse_handler_base::is_left_click(
		const SDL_MouseButtonEvent& event) const
{
	return (event.button == SDL_BUTTON_LEFT && !command_active())
			|| event.which == SDL_TOUCH_MOUSEID
#ifdef MOUSE_TOUCH_EMULATION
			|| event.button == SDL_BUTTON_RIGHT
#endif
	;
}

bool mouse_handler_base::is_middle_click(
		const SDL_MouseButtonEvent& event) const
{
	return event.button == SDL_BUTTON_MIDDLE;
}

bool mouse_handler_base::is_right_click(
		const SDL_MouseButtonEvent& event) const
{
#ifdef MOUSE_TOUCH_EMULATION
	(void) event;
	return false;
#else
	return event.button == SDL_BUTTON_RIGHT
			|| (event.button == SDL_BUTTON_LEFT && command_active());
#endif
}

bool mouse_handler_base::is_touch_click(const SDL_MouseButtonEvent& event) const
{
	return event.which == SDL_TOUCH_MOUSEID
#ifdef MOUSE_TOUCH_EMULATION
		   || event.button == SDL_BUTTON_RIGHT
#endif
			;
}

bool mouse_handler_base::allow_mouse_wheel_scroll(int /*x*/, int /*y*/)
{
	return true;
}

bool mouse_handler_base::right_click_show_menu(int /*x*/, int /*y*/, const bool /*browse*/)
{
	return true;
}

bool mouse_handler_base::left_click(int x, int y, const bool /*browse*/)
{

	if(gui().view_locked()) {
		return false;
	}

	// clicked on a hex on the minimap? then initiate minimap scrolling
	const map_location& loc = gui().minimap_location_on(x, y);
	minimap_scrolling_ = false;
	if(loc.valid()) {
		minimap_scrolling_ = true;
		last_hex_ = loc;
		gui().scroll_to_tile(loc,display::WARP, false);
		return true;
	}
	return false;
}

void mouse_handler_base::move_action(const bool /*browse*/)
{
	// Overridden with unit move code elsewhere
}

void mouse_handler_base::left_drag_end(int /*x*/, int /*y*/, const bool browse)
{
	move_action(browse);
}

void mouse_handler_base::left_mouse_up(int /*x*/, int /*y*/, const bool /*browse*/)
{
}

void mouse_handler_base::mouse_wheel(int scrollx, int scrolly, bool browse)
{
	int x, y;
	SDL_GetMouseState(&x, &y);

	int movex = scrollx * preferences::scroll_speed();
	int movey = scrolly * preferences::scroll_speed();

	// Don't scroll map if cursor is not in gamemap area
	if(!sdl::point_in_rect(x, y, gui().map_area())) {
		return;
	}

	if (movex != 0 || movey != 0) {
		CKey pressed;
		// Alt + mousewheel do an 90° rotation on the scroll direction
		if (pressed[SDLK_LALT] || pressed[SDLK_RALT]) {
			gui().scroll(-movey,-movex);
		} else {
			gui().scroll(-movex,-movey);
		}
	}

	if (scrollx < 0) {
		mouse_wheel_left(x, y, browse);
	} else if (scrollx > 0) {
		mouse_wheel_right(x, y, browse);
	}

	if (scrolly < 0) {
		mouse_wheel_down(x, y, browse);
	} else if (scrolly > 0) {
		mouse_wheel_up(x, y, browse);
	}
}

void mouse_handler_base::mouse_wheel_up(int /*x*/, int /*y*/, const bool /*browse*/)
{
}

void mouse_handler_base::mouse_wheel_down(int /*x*/, int /*y*/, const bool /*browse*/)
{
}

void mouse_handler_base::mouse_wheel_left(int /*x*/, int /*y*/, const bool /*browse*/)
{
}

void mouse_handler_base::mouse_wheel_right(int /*x*/, int /*y*/, const bool /*browse*/)
{
}

bool mouse_handler_base::right_click(int x, int y, const bool browse)
{
	if (right_click_show_menu(x, y, browse)) {
		gui().draw(); // redraw highlight (and maybe some more)
		const theme::menu* const m = gui().get_theme().context_menu();
		if (m != nullptr) {
			show_menu_ = true;
		} else {
			WRN_DP << "no context menu found..." << std::endl;
		}
		return true;
	}
	return false;
}

void mouse_handler_base::right_drag_end(int /*x*/, int /*y*/, const bool /*browse*/)
{
	//FIXME: This is called when we select an option in context-menu,
	//       which is bad because that was not a real dragging
}

void mouse_handler_base::right_mouse_up(int /*x*/, int /*y*/, const bool /*browse*/)
{
}

void mouse_handler_base::init_dragging(bool& dragging_flag)
{
	dragging_flag = true;
	SDL_GetMouseState(&drag_from_x_, &drag_from_y_);
	drag_from_hex_ = gui().hex_clicked_on(drag_from_x_, drag_from_y_);
}

void mouse_handler_base::cancel_dragging()
{
	dragging_started_ = false;
	dragging_left_ = false;
	dragging_right_ = false;
	cursor::set_dragging(false);
}

void mouse_handler_base::clear_dragging(const SDL_MouseButtonEvent& event, bool browse)
{
	// we reset dragging info before calling functions
	// because they may take time to return, and we
	// could have started other drag&drop before that
	cursor::set_dragging(false);
	if (dragging_started_) {
		dragging_started_ = false;
		if (dragging_left_) {
			dragging_left_ = false;
			left_drag_end(event.x, event.y, browse);
		}
		if (dragging_right_) {
			dragging_right_ = false;
			right_drag_end(event.x, event.y, browse);
		}
	} else {
		dragging_left_ = false;
		dragging_right_ = false;
	}
}


} //end namespace events
