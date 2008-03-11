/* $Id$ */
/*
   copyright (c) 2007 - 2008 by mark de wever <koraq@xs4all.nl>
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

void tbutton::draw(surface& canvas)
{
	DBG_GUI << "Drawing button\n";

	canvas_up_.draw();

	// now blit the cached image on the screen
	SDL_Rect rect = get_rect();
	SDL_BlitSurface(canvas_up_.surf(), 0, canvas, &rect);
}

} // namespace gui2
