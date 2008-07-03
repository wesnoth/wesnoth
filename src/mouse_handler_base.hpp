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

#ifndef MOUSE_HANDLER_BASE_H_INCLUDED
#define MOUSE_HANDLER_BASE_H_INCLUDED
#include "display.hpp"
#include "map.hpp"
#include "SDL.h"

namespace events {
	
struct command_disabler
{
	command_disabler();
	~command_disabler();
};	

extern int commands_disabled;
	
class mouse_handler_base {
public:
	mouse_handler_base(display* disp, gamemap& map);
	virtual ~mouse_handler_base() {}

	void mouse_motion_event(const SDL_MouseMotionEvent& event, const bool browse);
	/** update the mouse with a fake mouse motion */
	void mouse_update(const bool browse);

	/**
	 * This handles minimap scrolling and click-drag.
	 * @returns true when the caller should not process the mouse motion 
	 * further (i.e. should return), false otherwise.
	 */
	bool mouse_motion_default(int x, int y, bool& update);
	virtual void mouse_motion(int x, int y, const bool browse, bool update=false) = 0;
	
	virtual void mouse_press(const SDL_MouseButtonEvent& event, const bool browse);
	bool is_left_click(const SDL_MouseButtonEvent& event);
	bool is_middle_click(const SDL_MouseButtonEvent& event);
	bool is_right_click(const SDL_MouseButtonEvent& event);
	
	/**
	 * @returns true when the (child) caller should not process the event further
	 */
	virtual bool left_click(const SDL_MouseButtonEvent& event, const bool browse);
	
	virtual bool right_click(const SDL_MouseButtonEvent& event, const bool browse);
	
	/**
	 * Called in right_click when the context menu is about to be shown, can be 
	 * used for preprocessing and preventing the menu from being displayed.
	 * @returns true when the menu should be displayed and false otherwise
	 */
	virtual bool right_click_before_menu(const SDL_MouseButtonEvent& event, const bool browse);

protected:
	display* gui_;
	
	bool minimap_scrolling_;
	bool dragging_;
	bool dragging_started_;
	int drag_from_x_;
	int drag_from_y_;

	// last highlighted hex
	gamemap::location last_hex_;
	
	bool show_menu_;
	
	gamemap& map_;
	
	static const int drag_threshold_;
};

} // end namespace events

#endif
