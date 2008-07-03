/* $Id$ */
/*
   Copyright (C) 2006 - 2008 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
   wesnoth playturn Copyright (C) 2003 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "mouse_handler_base.hpp"

#include "cursor.hpp"
#include "log.hpp"
#include "preferences.hpp"


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

const int mouse_handler_base::drag_threshold_ = 0;
	
static bool command_active()
{
#ifdef __APPLE__
	return (SDL_GetModState()&KMOD_META) != 0;
#else
	return false;
#endif
}

mouse_handler_base::mouse_handler_base(display* disp, gamemap& map)
: gui_(disp), map_(map)
{
}

void mouse_handler_base::mouse_motion_event(const SDL_MouseMotionEvent& event, const bool browse)
{
	mouse_motion(event.x,event.y, browse);
}

void mouse_handler_base::mouse_update(const bool browse)
{
	int x, y;
	SDL_GetMouseState(&x,&y);
	mouse_motion(x, y, browse, true);
}

bool mouse_handler_base::mouse_motion_default(int x, int y, bool& /*update*/) 
{
	if(minimap_scrolling_) {
		//if the game is run in a window, we could miss a LMB/MMB up event
		// if it occurs outside our window.
		// thus, we need to check if the LMB/MMB is still down
		minimap_scrolling_ = ((SDL_GetMouseState(NULL,NULL) & (SDL_BUTTON(1) | SDL_BUTTON(2))) != 0);
		if(minimap_scrolling_) {
			const gamemap::location& loc = gui_->minimap_location_on(x,y);
			if(loc.valid()) {
				if(loc != last_hex_) {
					last_hex_ = loc;
					gui_->scroll_to_tile(loc,display::WARP,false);
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
	if (dragging_ && !dragging_started_ && (SDL_GetMouseState(&mx,&my) & SDL_BUTTON_LEFT) != 0) {
		const double drag_distance = std::pow((double) (drag_from_x_- mx), 2) + std::pow((double) (drag_from_y_- my), 2);
		if (drag_distance > drag_threshold_*drag_threshold_) {
			dragging_started_ = true;
			cursor::set_dragging(true);
		}
	}
	return false;
}

void mouse_handler_base::mouse_press(const SDL_MouseButtonEvent& event, const bool browse)
{
	show_menu_ = false;
	mouse_update(browse);
	int scrollx = 0;
	int scrolly = 0;

	if(is_left_click(event) && event.state == SDL_RELEASED) {
		minimap_scrolling_ = false;
		dragging_ = false;
		cursor::set_dragging(false);
		if (dragging_started_ && !browse && !commands_disabled) {
			left_click(event, browse);
		}
		dragging_started_= false;
	} else if(is_middle_click(event) && event.state == SDL_RELEASED) {
		minimap_scrolling_ = false;
	} else if(is_left_click(event) && event.state == SDL_PRESSED) {
		left_click(event, browse);
		if (!browse && !commands_disabled) {
			dragging_ = true;
			dragging_started_ = false;
			SDL_GetMouseState(&drag_from_x_, &drag_from_y_);
		}
	} else if(is_right_click(event) && event.state == SDL_PRESSED) {
		// The first right-click cancel the selection if any,
		// the second open the context menu
		dragging_ = false;
		dragging_started_ = false;
		cursor::set_dragging(false);
		if (right_click_before_menu(event, browse)) {
			gui_->draw(); // redraw highlight (and maybe some more)
			const theme::menu* const m = gui_->get_theme().context_menu();
			if (m != NULL)
				show_menu_ = true;
			else
				LOG_STREAM(warn, display) << "no context menu found...\n";
		}
	} else if(is_middle_click(event) && event.state == SDL_PRESSED) {
		// clicked on a hex on the minimap? then initiate minimap scrolling
		const gamemap::location& loc = gui_->minimap_location_on(event.x,event.y);
		minimap_scrolling_ = false;
		if(loc.valid()) {
			minimap_scrolling_ = true;
			last_hex_ = loc;
			gui_->scroll_to_tile(loc,display::WARP,false);
		}
	} else if (event.button == SDL_BUTTON_WHEELUP) {
		scrolly = - preferences::scroll_speed();
	} else if (event.button == SDL_BUTTON_WHEELDOWN) {
		scrolly = preferences::scroll_speed();
	} else if (event.button == SDL_BUTTON_WHEELLEFT) {
		scrollx = - preferences::scroll_speed();
	} else if (event.button == SDL_BUTTON_WHEELRIGHT) {
		scrollx = preferences::scroll_speed();
	}

	if (scrollx != 0 || scrolly != 0) {
		CKey pressed;
		// Alt + mousewheel do an 90° rotation on the scroll direction
		if (pressed[SDLK_LALT] || pressed[SDLK_RALT])
			gui_->scroll(scrolly,scrollx);
		else
			gui_->scroll(scrollx,scrolly);
	}
	if (!dragging_ && dragging_started_) {
		dragging_started_ = false;
		cursor::set_dragging(false);
	}
	mouse_update(browse);
}

bool mouse_handler_base::is_left_click(const SDL_MouseButtonEvent& event)
{
	return event.button == SDL_BUTTON_LEFT && !command_active();
}

bool mouse_handler_base::is_middle_click(const SDL_MouseButtonEvent& event)
{
	return event.button == SDL_BUTTON_MIDDLE;
}

bool mouse_handler_base::is_right_click(const SDL_MouseButtonEvent& event)
{
	return event.button == SDL_BUTTON_RIGHT || (event.button == SDL_BUTTON_LEFT && command_active());
}

bool mouse_handler_base::right_click_before_menu(const SDL_MouseButtonEvent& /*event*/, const bool /*browse*/)
{
	return true;
}

bool mouse_handler_base::left_click(const SDL_MouseButtonEvent& event, const bool /*browse*/)
{
	dragging_ = false;
	dragging_started_ = false;
	cursor::set_dragging(false);

	// clicked on a hex on the minimap? then initiate minimap scrolling
	const gamemap::location& loc = gui_->minimap_location_on(event.x,event.y);
	minimap_scrolling_ = false;
	if(loc.valid()) {
		minimap_scrolling_ = true;
		last_hex_ = loc;
		gui_->scroll_to_tile(loc,display::WARP,false);
		return true;
	}
	return false;
}

bool mouse_handler_base::right_click(const SDL_MouseButtonEvent& /*event*/, const bool /*browse*/)
{
	return false;
}


} //end namespace events
