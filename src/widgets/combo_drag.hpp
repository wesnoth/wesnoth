/*
   Copyright (C) 2008 - 2015 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef COMBO_DRAG_H_INCLUDED
#define COMBO_DRAG_H_INCLUDED

#include "widgets/combo.hpp"
#include "widgets/drop_target.hpp"

class display;

namespace gui {

	class combo_drag;
	typedef boost::shared_ptr<combo_drag> combo_drag_ptr;

	class combo_drag : public combo, public drop_target, public events::pump_monitor
	{
		public:
			combo_drag(display& disp, const std::vector<std::string>& items, const drop_group_manager_ptr group);

			/**
			 * used to query if this object was dropped to a target
			 * @return: -1 if not dropped and other the id of target object
			 **/
			int get_drop_target();
			/**
			 * Implements return after drop
			 **/
			virtual void process(events::pump_info& /*info*/);
		protected:
			virtual void process_event();
			virtual void mouse_motion(const SDL_MouseMotionEvent& event);
			virtual void mouse_down(const SDL_MouseButtonEvent& event);
			virtual void mouse_up(const SDL_MouseButtonEvent& event);
		private:

			void handle_move(const SDL_MouseMotionEvent& event);
			void handle_drop();
			int drag_target_, old_drag_target_;
			SDL_Rect old_location_;
			int mouse_x_, mouse_y_;
			enum drag_state {
				NONE,
				PRESSED,
				PRESSED_MOVE,
				MOVED,
				RETURN,
				DROP_DOWN
			};
			drag_state drag_;
			static const float MIN_DRAG_DISTANCE;
			static const float RETURN_SPEED;
	}; //end class combo

}

#endif
