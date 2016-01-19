/*
   Copyright (C) 2003 - 2016 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "global.hpp"

#include "widgets/combo_drag.hpp"



namespace gui {

	const float combo_drag::MIN_DRAG_DISTANCE = 10.0;
	const float combo_drag::RETURN_SPEED = 25.0;

	combo_drag::combo_drag(CVideo& v
			, const std::vector<std::string>& items
			, const drop_group_manager_ptr group)
		: combo(v, items)
		, drop_target(group, location())
		, drag_target_(-1)
		, old_drag_target_(-1)
		, old_location_()
		, mouse_x_(-1)
		, mouse_y_(-1)
		, drag_(NONE)
	{
	}

	int combo_drag::get_drop_target()
	{
		old_drag_target_ = drag_target_;
		drag_target_ = -1;
		return old_drag_target_;
	}

	void combo_drag::handle_move(const SDL_MouseMotionEvent& event)
	{
		if (drag_ == PRESSED)
		{
			aquire_mouse_lock();
			old_location_ = location();
			drag_ = PRESSED_MOVE;
		}
		const int diff_x = event.x - mouse_x_;
		const int diff_y = event.y - mouse_y_;
		if (drag_ == PRESSED_MOVE
			&& std::sqrt(static_cast<float>(diff_x*diff_x + diff_y*diff_y)) > MIN_DRAG_DISTANCE)
		{
			return;
		}
		drag_ = MOVED;
		SDL_Rect loc = old_location_;
		loc.x += diff_x;
		loc.y += diff_y;


		// Don't allow moving outside clip are

		if (clip_rect())
		{
			const SDL_Rect *clip = clip_rect();
			if (loc.x < clip->x)
				loc.x = clip->x;
			if (loc.x + loc.w > clip->x + clip->w)
				loc.x = clip->x + clip->w - loc.w;
			if (loc.y < clip->y)
				loc.y = clip->y;
			if (loc.y + loc.h > clip->y + clip->h)
				loc.y = clip->y + clip->h - loc.h;
		}

		set_location(loc);
	}

	void combo_drag::process(events::pump_info& /*info*/)
	{
		if (drag_ == RETURN)
		{
			SDL_Rect loc = location();
			int x_diff = loc.x - old_location_.x;
			int y_diff = loc.y - old_location_.y;
			const float length = std::sqrt(static_cast<float>(x_diff*x_diff + y_diff*y_diff));

			if (length > RETURN_SPEED)
			{
				loc.x -= static_cast<Sint16>(x_diff*(RETURN_SPEED/length));
				loc.y -= static_cast<Sint16>(y_diff*(RETURN_SPEED/length));
				set_location(loc);
			}
			else
			{
				drag_ = NONE;
				set_location(old_location_);
			}
		}
	}

	void combo_drag::handle_drop()
	{
		drag_target_ = drop_target::handle_drop();
	}

	void combo_drag::mouse_motion(const SDL_MouseMotionEvent& event)
	{
		if (drag_ == PRESSED
				|| drag_ == MOVED
				|| drag_ == PRESSED_MOVE)
		{
			handle_move(event);
		} else {
			button::mouse_motion(event);
		}
	}

	void combo_drag::mouse_up(const SDL_MouseButtonEvent& event)
	{
		if ((drag_ == PRESSED || drag_ == PRESSED_MOVE || drag_ == MOVED) && event.button == SDL_BUTTON_LEFT)
		{
			if (drag_ == PRESSED
					|| drag_ == PRESSED_MOVE)
			{
				free_mouse_lock();
				drag_ = DROP_DOWN;
			}
			else if (drag_ == MOVED)
			{
				free_mouse_lock();
				handle_drop();
				drag_ = RETURN;
				hide();
			}
		}
		button::mouse_up(event);

	}

	void combo_drag::mouse_down(const SDL_MouseButtonEvent& event)
	{
		if (hit(event.x, event.y) && event.button == SDL_BUTTON_LEFT)
		{
			drag_ = PRESSED;
			mouse_x_ = event.x;
			mouse_y_ = event.y;
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
