/* $Id$ */
/*
   copyright (c) 2008 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
*/

#include "gui/widgets/button.hpp"

#define DBG_GUI LOG_STREAM(debug, widget)
#define LOG_GUI LOG_STREAM(info, widget)
#define WRN_GUI LOG_STREAM(warn, widget)
#define ERR_GUI LOG_STREAM(err, widget)

namespace gui2 {

void tbutton::set_width(const int width)
{ 
	// resize canvasses
	canvas_up_.set_width(width);
	canvas_up_mouse_over_.set_width(width);
	canvas_down_.set_width(width);

	// inherited
	tcontrol::set_width(width);
}

void tbutton::set_height(const int height) 
{ 
	// resize canvasses
	canvas_up_.set_height(height);
	canvas_up_mouse_over_.set_height(height);
	canvas_down_.set_height(height);

	// inherited
	tcontrol::set_height(height);
}

void tbutton::mouse_down(const tevent_info& /*event*/, bool& /*handled*/) 
{ 
	DBG_GUI << "mouse down\n"; 

	set_state(DOWN);
}

void tbutton::mouse_up(const tevent_info& /*event*/, bool& /*handled*/) 
{ 
	DBG_GUI << "mouse up\n";

	set_state(MOUSE_OVER);
}

void tbutton::mouse_click(const tevent_info& /*event*/, bool& /*handled*/) 
{ 
	DBG_GUI << "mouse click\n"; 
}

void tbutton::mouse_double_click(const tevent_info& /*event*/, bool& /*handled*/) 
{ 
	DBG_GUI << "mouse double click\n"; 
}

void tbutton::mouse_enter(const tevent_info& /*event*/, bool& /*handled*/) 
{ 
	DBG_GUI << "mouse enter\n"; 

	set_state(MOUSE_OVER);
}

void tbutton::mouse_leave(const tevent_info& /*event*/, bool& /*handled*/) 
{ 
	DBG_GUI << "mouse leave\n"; 

	set_state(NORMAL);
}

void tbutton::draw(surface& canvas)
{
	DBG_GUI << "Drawing button\n";

	SDL_Rect rect = get_rect();
	switch(state_) {

		case NORMAL : 
			canvas_up_.draw(true);
			SDL_BlitSurface(canvas_up_.surf(), 0, canvas, &rect);
			break;

		case DOWN : 
			canvas_down_.draw(true);
			SDL_BlitSurface(canvas_down_.surf(), 0, canvas, &rect);
			break;

		case MOUSE_OVER :
			canvas_up_mouse_over_.draw(true);
			SDL_BlitSurface(canvas_up_mouse_over_.surf(), 0, canvas, &rect);
			break;
	}

	set_dirty(false);
}

void tbutton::set_state(tstate state)
{
	if(state != state_) {
		state_ = state;
		set_dirty(true);
	}
}

} // namespace gui2
