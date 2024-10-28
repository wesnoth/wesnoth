/*
	Copyright (C) 2006 - 2024
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

#include "mouse_handler_base.hpp"

#include "cursor.hpp"
#include "display.hpp"
#include "log.hpp"
#include "preferences/preferences.hpp"
#include "sdl/rect.hpp"
#include "tooltips.hpp"
#include "sdl/input.hpp" // get_mouse_state

static lg::log_domain log_display("display");
#define WRN_DP LOG_STREAM(warn, log_display)

namespace events
{
command_disabler::command_disabler()
{
	++commands_disabled;
}

command_disabler::~command_disabler()
{
	--commands_disabled;
}

int commands_disabled = 0;

static bool command_active()
{
#ifdef __APPLE__
	return (SDL_GetModState() & KMOD_CTRL) != 0;
#else
	return false;
#endif
}

mouse_handler_base::mouse_handler_base()
	: simple_warp_(false)
	, minimap_scrolling_(false)
	, dragging_left_(false)
	, dragging_touch_(false)
	, dragging_started_(false)
	, dragging_right_(false)
	, drag_from_(0, 0)
	, drag_from_hex_()
	, last_hex_()
	, show_menu_(false)
	, scroll_start_x_(0)
	, scroll_start_y_(0)
	, scroll_started_(false)
{
}

bool mouse_handler_base::dragging_started() const
{
	return dragging_started_;
}

bool mouse_handler_base::is_dragging() const
{
	return dragging_left_ || dragging_right_ || dragging_touch_;
}

void mouse_handler_base::mouse_motion_event(const SDL_MouseMotionEvent& event, const bool browse)
{
	mouse_motion(event.x, event.y, browse);
}

void mouse_handler_base::touch_motion_event(const SDL_TouchFingerEvent& event, const bool browse)
{
	// This is wrong (needs to be scaled from -1..1 to screen size), but it's discarded in touch_motion anyway.
	// Let's not waste CPU cycles.
	touch_motion(event.x, event.y, browse);
}

void mouse_handler_base::mouse_update(const bool browse, map_location loc)
{
	auto [x, y] = sdl::get_mouse_location();
	mouse_motion(x, y, browse, true, loc);
}

bool mouse_handler_base::mouse_motion_default(int x, int y, bool /*update*/)
{
	tooltips::process(x, y);

	if(simple_warp_) {
		return true;
	}

	if(minimap_scrolling_) {
		// if the game is run in a window, we could miss a LMB/MMB up event
		// if it occurs outside our window.
		// thus, we need to check if the LMB/MMB is still down
		minimap_scrolling_ = ((sdl::get_mouse_button_mask() & (SDL_BUTTON(SDL_BUTTON_LEFT) | SDL_BUTTON(SDL_BUTTON_MIDDLE))) != 0);
		if(minimap_scrolling_) {
			const map_location& loc = gui().minimap_location_on(x, y);
			if(loc.valid()) {
				if(loc != last_hex_) {
					last_hex_ = loc;
					gui().scroll_to_tile(loc, display::WARP, false);
				}
			} else {
				// clicking outside of the minimap will end minimap scrolling
				minimap_scrolling_ = false;
			}
		}

		if(minimap_scrolling_) {
			return true;
		}
	}

	// Fire the drag & drop only after minimal drag distance
	// While we check the mouse buttons state, we also grab fresh position data.

	if(is_dragging() && !dragging_started_) {
		point pos = drag_from_; // some default value to prevent unlikely SDL bug
		uint32_t mouse_state = dragging_left_ || dragging_right_ ? sdl::get_mouse_state(&pos.x, &pos.y) : 0;

#ifdef MOUSE_TOUCH_EMULATION
		if(dragging_left_ && (mouse_state & SDL_BUTTON(SDL_BUTTON_RIGHT))) {
			// Monkey-patch touch controls again to make them look like left button.
			mouse_state = SDL_BUTTON(SDL_BUTTON_LEFT);
		}
#endif
		if((dragging_left_  && (mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT))  != 0) ||
		   (dragging_right_ && (mouse_state & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0))
		{
			const double drag_distance =
					std::pow(static_cast<double>(drag_from_.x - pos.x), 2) +
					std::pow(static_cast<double>(drag_from_.y - pos.y), 2);

			if(drag_distance > drag_threshold() * drag_threshold()) {
				dragging_started_ = true;
				cursor::set_dragging(true);
			}
		}
	}

	return false;
}

bool mouse_handler_base::mouse_button_event(const SDL_MouseButtonEvent& event, uint8_t button,
											map_location loc, bool click)
{
	(void)event;
	(void)button;
	(void)loc;
	(void)click;

	return false;
}

void mouse_handler_base::mouse_press(const SDL_MouseButtonEvent& event, const bool browse)
{
	if(is_middle_click(event) && !prefs::get().middle_click_scrolls()) {
		simple_warp_ = true;
	}

	show_menu_ = false;
	map_location loc = gui().hex_clicked_on(event.x, event.y);
	mouse_update(browse, loc);

	static clock_t touch_timestamp = 0;

	if(is_touch_click(event)) {
		if (event.state == SDL_PRESSED) {
			cancel_dragging();
			touch_timestamp = clock();
			init_dragging(dragging_touch_);
			if (!mouse_button_event(event, SDL_BUTTON_LEFT, loc, true)) {
				left_click(event.x, event.y, browse);
			}
		} else if (event.state == SDL_RELEASED) {
			minimap_scrolling_ = false;

			if (!dragging_started_ && touch_timestamp > 0) {
				clock_t dt = clock() - touch_timestamp;
				if (dt > CLOCKS_PER_SEC * 3 / 10) {
					if (!mouse_button_event(event, SDL_BUTTON_RIGHT, loc, true)) {
						// BUG: This function won't do anything in the game, need right_mouse_up()
						right_click(event.x, event.y, browse); // show_menu_ = true;
					}
				}
			} else {
				touch_timestamp = 0;
			}

			clear_dragging(event, browse);
			mouse_button_event(event, SDL_BUTTON_LEFT, loc);
			left_mouse_up(event.x, event.y, browse);
			clear_drag_from_hex();
		}
	} else if(is_left_click(event)) {
		if(event.state == SDL_PRESSED) {
			cancel_dragging();
			init_dragging(dragging_left_);
			if (!mouse_button_event(event, SDL_BUTTON_LEFT, loc, true)) {
				left_click(event.x, event.y, browse);
			}
		} else if(event.state == SDL_RELEASED) {
			minimap_scrolling_ = false;
			clear_dragging(event, browse);
			mouse_button_event(event, SDL_BUTTON_LEFT, loc);
			left_mouse_up(event.x, event.y, browse);
			clear_drag_from_hex();
		}
	} else if(is_right_click(event)) {
		if(event.state == SDL_PRESSED) {
			mouse_button_event(event, SDL_BUTTON_RIGHT, loc);
			cancel_dragging();
			init_dragging(dragging_right_);
			right_click(event.x, event.y, browse);
		} else if(event.state == SDL_RELEASED) {
			minimap_scrolling_ = false;
			clear_dragging(event, browse);
			if (!mouse_button_event(event, SDL_BUTTON_RIGHT, loc, true)) {
				right_mouse_up(event.x, event.y, browse);
			}
			clear_drag_from_hex();
		}
	} else if(is_middle_click(event)) {
		if(event.state == SDL_PRESSED) {
			drag_from_hex_ = loc;
			set_scroll_start(event.x, event.y);
			scroll_started_ = true;

			map_location minimap_loc = gui().minimap_location_on(event.x, event.y);
			minimap_scrolling_ = false;
			if(minimap_loc.valid()) {
				simple_warp_ = false;
				minimap_scrolling_ = true;
				last_hex_ = minimap_loc;
				gui().scroll_to_tile(minimap_loc, display::WARP, false);
			} else if(mouse_button_event(event, SDL_BUTTON_MIDDLE, loc, true)) {
				scroll_started_ = false;
				simple_warp_ = false;
			} else if(simple_warp_) {
				// middle click not on minimap, check gamemap instead
				if(loc.valid()) {
					last_hex_ = loc;
					gui().scroll_to_tile(loc, display::WARP, false);
				}
			} else {
				// Deselect the current tile as we're scrolling
				gui().highlight_hex({-1,-1});
			}
		} else if(event.state == SDL_RELEASED) {
			minimap_scrolling_ = false;
			simple_warp_ = false;
			scroll_started_ = false;
			mouse_button_event(event, SDL_BUTTON_MIDDLE, loc);
			clear_drag_from_hex();
		}
	} else if(event.button == SDL_BUTTON_X1 || event.button == SDL_BUTTON_X2) {
		if(event.state == SDL_PRESSED) {
			cancel_dragging();
			// record mouse-down hex in drag_from_hex_
			drag_from_hex_ = loc;
			mouse_button_event(event, event.button, loc);
		} else {
			mouse_button_event(event, event.button, loc, true);
			clear_drag_from_hex();
		}
	}
	if(!dragging_left_ && !dragging_right_ && !dragging_touch_ && dragging_started_) {
		dragging_started_ = false;
		cursor::set_dragging(false);
	}

	mouse_update(browse, loc);
}

bool mouse_handler_base::is_left_click(const SDL_MouseButtonEvent& event) const
{
#ifdef MOUSE_TOUCH_EMULATION
	if(event.button == SDL_BUTTON_RIGHT) {
		return true;
	}
#endif
	if(event.which == SDL_TOUCH_MOUSEID) {
		return false;
	}
	return event.button == SDL_BUTTON_LEFT && !command_active();
}

bool mouse_handler_base::is_middle_click(const SDL_MouseButtonEvent& event) const
{
	return event.button == SDL_BUTTON_MIDDLE;
}

bool mouse_handler_base::is_right_click(const SDL_MouseButtonEvent& event) const
{
#ifdef MOUSE_TOUCH_EMULATION
	(void) event;
	return false;
#else
	if(event.which == SDL_TOUCH_MOUSEID) {
		return false;
	}
	return event.button == SDL_BUTTON_RIGHT
			|| (event.button == SDL_BUTTON_LEFT && command_active());
#endif
}

bool mouse_handler_base::is_touch_click(const SDL_MouseButtonEvent& event) const
{
	return event.which == SDL_TOUCH_MOUSEID;
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
		gui().scroll_to_tile(loc, display::WARP, false);
		return true;
	}

	return false;
}

void mouse_handler_base::touch_action(const map_location /*hex*/, bool /*browse*/)
{
}

void mouse_handler_base::left_drag_end(int /*x*/, int /*y*/, const bool browse)
{
	move_action(browse);
}

void mouse_handler_base::mouse_wheel(int scrollx, int scrolly, bool browse)
{
	auto [x, y] = sdl::get_mouse_location();

	int movex = scrollx * prefs::get().scroll_speed();
	int movey = scrolly * prefs::get().scroll_speed();

	// Don't scroll map if cursor is not in gamemap area
	if(!gui().map_area().contains(x, y)) {
		return;
	}

	if(movex != 0 || movey != 0) {
		CKey pressed;
		// Alt + mousewheel do an 90° rotation on the scroll direction
		if(pressed[SDLK_LALT] || pressed[SDLK_RALT]) {
			gui().scroll(point{movey, movex});
		} else {
			gui().scroll(point{movex, movey});
		}
	}

	if(scrollx < 0) {
		mouse_wheel_left(x, y, browse);
	} else if(scrollx > 0) {
		mouse_wheel_right(x, y, browse);
	}

	if(scrolly < 0) {
		mouse_wheel_up(x, y, browse);
	} else if(scrolly > 0) {
		mouse_wheel_down(x, y, browse);
	}
}

void mouse_handler_base::right_mouse_up(int x, int y, const bool browse)
{
	if(!right_click_show_menu(x, y, browse)) {
		return;
	}

	const theme::menu* const m = gui().get_theme().context_menu();
	if(m != nullptr) {
		show_menu_ = true;
	} else {
		WRN_DP << "no context menu found...";
	}
}

void mouse_handler_base::init_dragging(bool& dragging_flag)
{
	dragging_flag = true;
	drag_from_ = sdl::get_mouse_location();
	drag_from_hex_ = gui().hex_clicked_on(drag_from_.x, drag_from_.y);
}

void mouse_handler_base::cancel_dragging()
{
	dragging_started_ = false;
	dragging_left_ = false;
	dragging_touch_ = false;
	dragging_right_ = false;
	cursor::set_dragging(false);
}

void mouse_handler_base::clear_dragging(const SDL_MouseButtonEvent& event, bool browse)
{
	// we reset dragging info before calling functions
	// because they may take time to return, and we
	// could have started other drag&drop before that
	cursor::set_dragging(false);

	if(dragging_started_) {
		dragging_started_ = false;

		if(dragging_touch_) {
			dragging_touch_ = false;
			// Maybe to do: create touch_drag_end(). Do panning and what else there. OTOH, it's fine now.
			left_drag_end(event.x, event.y, browse);
		}

		if(dragging_left_) {
			dragging_left_ = false;
			left_drag_end(event.x, event.y, browse);
		}

		if(dragging_right_) {
			dragging_right_ = false;
			right_drag_end(event.x, event.y, browse);
		}
	} else {
		dragging_left_ = false;
		dragging_right_ = false;
		dragging_touch_ = false;
	}
}

void mouse_handler_base::clear_drag_from_hex()
{
	drag_from_hex_ = map_location::null_location();
}

} // end namespace events
