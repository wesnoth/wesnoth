/* $Id$ */
/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "widgets/combo_drag.hpp"
#include "widgets/drop_target.hpp"
#include "display.hpp"
#include "show_dialog.hpp"
#include "video.hpp"



namespace gui {

	const int combo_drag::MIN_DRAG_DISTANCE = 30;

	combo_drag::combo_drag(display& disp, const std::vector<std::string>& items, const drop_target_group& group)
		: combo(disp, items), drop_target(group, location()),
		drag_target_(-1), old_drag_target_(-1), 
		old_location_(), drag_(NONE)
	{
	}

	int combo_drag::get_drag_target()
	{
		old_drag_target_ = drag_target_;
		drag_target_ = -1;
		return old_drag_target_;
	}

	void combo_drag::handle_move(const SDL_MouseMotionEvent& event)
	{
		if (drag_ == PRESSED)
		{
			old_location_ = location();
			drag_ = PRESSED_MOVE;
		}
		const int diff_x = event.x - mouse_x;
		const int diff_y = event.y - mouse_y;
		if (drag_ == PRESSED_MOVE
			&& std::sqrt(diff_x*diff_x + diff_y*diff_y) > MIN_DRAG_DISTANCE)
		{
			return;
		}
		drag_ = MOVED;
		SDL_Rect loc = location();
		loc.x += diff_x;
		loc.y += diff_y;

		assert(mouse_x -event.x + (mouse_y -event.y) < 50);

		set_location(loc);
		mouse_x = event.x;
		mouse_y = event.y;
	}

	void combo_drag::handle_drop()
	{
		drag_target_ = drop_target::handle_drop();
		set_location(old_location_);
	}

	void combo_drag::mouse_motion(const SDL_MouseMotionEvent& event)
	{
		if (drag_ == PRESSED 
				|| drag_ == MOVED
				|| drag_ == PRESSED_MOVE)
		{
			handle_move(event);
		}
		button::mouse_motion(event);
	}

	void combo_drag::mouse_up(const SDL_MouseButtonEvent& event)
	{
		if (hit(event.x, event.y) && event.button == SDL_BUTTON_LEFT)
		{
			if (drag_ == PRESSED
					|| drag_ == PRESSED_MOVE)
			{
				drag_ = DROP_DOWN;
			}
			else if (drag_ == MOVED)
			{
				handle_drop();
				drag_ = NONE;
			}
		}
		button::mouse_up(event);

	}

	void combo_drag::mouse_down(const SDL_MouseButtonEvent& event)
	{
		if (hit(event.x, event.y) && event.button == SDL_BUTTON_LEFT)
		{
			drag_ = PRESSED;
			mouse_x = event.x;
			mouse_y = event.y;
		}
		button::mouse_down(event);
	}

	void combo_drag::process_event()
	{
		if (drag_ == DROP_DOWN)
		{
			drag_ = NONE;
			make_drop_down_menu();
		}
	}

}
