/* $Id$ */
/*
   Copyright (C) 2007 - 2008 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file window.cpp
//! Implementation of window.hpp.

#include "gui/widgets/window.hpp"

#include "config.hpp"
#include "log.hpp"
#include "serialization/parser.hpp"
#include "variable.hpp"

#include <cassert>

#define DBG_GUI LOG_STREAM(debug, widget)
#define LOG_GUI LOG_STREAM(info, widget)
#define WRN_GUI LOG_STREAM(warn, widget)
#define ERR_GUI LOG_STREAM(err, widget)

namespace gui2{

twindow::twindow(CVideo& video, 
		const int x, const int y, const int w, const int h) :
	tpanel(),
	events::handler(false), // don't join we haven't created a context yet
	video_(video),
	status_(NEW),
	event_info_(),
	event_context_()
{
	set_x(x);
	set_y(y);
	set_width(w);
	set_height(h);

	// The event context is created now we join it.
	join();
}

void twindow::show(const bool restore, void* /*flip_function*/)
{
	log_scope2(widget, "Starting drawing window");	

	// Sanity
	if(status_ != NEW) {
		// FIXME throw an exception

	}
	
	// We cut a piec of the screen and use that, that way all coordinates
	// are relative to the window.
	SDL_Rect rect = get_rect();
	surface screen = get_surface_portion(video_.getSurface(), rect);

	// Preserve area
	surface restorer;
	if(restore) {
		restorer = screen;
	}

	// draw
	SDL_Rect Xrect = {0, 0, screen->w, screen->h};
	fill_rect_alpha(Xrect, 0, 128, screen);

	// draw our children
	//fixme make draw const and use a const iterator
//	for(std::multimap<std::string, twidget *>::iterator itor = 
//			children_().begin(); itor != children_().end(); ++itor) {
	
	layout(Xrect);
	for(tsizer::iterator itor = begin(); itor != end(); ++itor) {

		log_scope2(widget, "Draw child");

		itor->draw(screen);
	}

	rect = get_rect();
	SDL_BlitSurface(screen, 0, video_.getSurface(), &rect);
	update_rect(get_rect());
	flip();

	DBG_GUI << "Drawing finished\n";
	
	// start our loop
	for(status_ = SHOWING; status_ != REQUEST_CLOSE; ) {
		events::pump();

	
	
	
		// fixme hack to disable 
		if(event_info_.mouse_right_button_down) {
			status_ = REQUEST_CLOSE;
		}

		// fixme manual destroy
		if(status_ == REQUEST_CLOSE) {
			break;
		}

		if(dirty()) {
#if 0			
			// Darkening for debugging redraw.
			SDL_Rect Xrect = {0, 0, screen->w, screen->h};
			fill_rect_alpha(Xrect, 0, 1, screen);
#endif

			for(tsizer::iterator itor = begin(); itor != end(); ++itor) {

				if(itor->dirty()) {
					log_scope2(widget, "Draw child");

					itor->draw(screen);
				}
			}
			rect = get_rect();
			SDL_BlitSurface(screen, 0, video_.getSurface(), &rect);
			update_rect(get_rect());
		}

		// delay until it's our frame see display.ccp code for how to do that
		SDL_Delay(10);
		flip();
	}

	// restore area
	if(restore) {
		rect = get_rect();
		SDL_BlitSurface(restorer, 0, screen, &rect);
		update_rect(get_rect());
		flip();
	}
}

void twindow::layout(const SDL_Rect position)
{
	tpoint best_size = get_best_size();

	if(best_size.x < position.w && best_size.y < position.h) {
		set_best_size(tpoint(0, 0));
		return;
	}

	DBG_GUI << "Failed for best size, try minimum.\n";

	// Implement the code.
	assert(false);

	// Failed at best size try minumum.
	
	// Failed at minimum log error and try to do the best possible thing.
}

void twindow::flip()
{
	// fixme we need to add the option to either call
	// video_.flip() or display.flip()
	video_.flip();

}

//! Implement events::handler::handle_event().
void twindow::handle_event(const SDL_Event& event)
{
	// set the type of event processing
	// either focus object (key pressed)
	// object under the mouse (eg mouse down, mouse move, mouse up) 
	// all objects (resize, redraw)

	event_info_.cycle();
	switch(event.type) {
		case SDL_MOUSEBUTTONDOWN:
			handle_event_mouse_down(event);
			break;
		case SDL_MOUSEBUTTONUP:
			handle_event_mouse_up(event);
			break;
		case SDL_MOUSEMOTION:
			handle_event_mouse_move(event);
			break;
	}

}

//! Handler for a mouse down.
void twindow::handle_event_mouse_down(const SDL_Event& event)
{
	event_info_.event_mouse_button = event.button.button;
	event_info_.mouse_x = event.button.x;
	event_info_.mouse_y = event.button.y;
	event_info_.mouse_focus = get_widget(tpoint(event_info_.mouse_x - get_x(), event_info_.mouse_y - get_y()));

	bool handled = false;
	switch(event_info_.event_mouse_button) {
		case SDL_BUTTON_LEFT   : 
			event_info_.mouse_left_button_down = true;  
			if(event_info_.mouse_focus) {
				event_info_.mouse_focus->mouse_down(event_info_, handled);
			}
			break;
		case SDL_BUTTON_MIDDLE : 
			event_info_.mouse_middle_button_down = true; 
			break;
		case SDL_BUTTON_RIGHT  : 
			event_info_.mouse_right_button_down = true;  
			break;

		// Note: other mouse buttons are ignored, the event
		// is send but the status won't be remembered.
	}
}

//! Handler for a mouse down.
void twindow::handle_event_mouse_up(const SDL_Event& event)
{
	event_info_.event_mouse_button = event.button.button;
	event_info_.mouse_x = event.button.x;
	event_info_.mouse_y = event.button.y;
	twidget* mouse_focus = get_widget(tpoint(event_info_.mouse_x - get_x(), event_info_.mouse_y - get_y()));

	bool handled = false;
	switch(event_info_.event_mouse_button) {
		case SDL_BUTTON_LEFT   :
			if(event_info_.mouse_focus) {

				event_info_.mouse_focus->mouse_up(event_info_, handled);

				if(event_info_.mouse_focus == mouse_focus) {

					if(event_info_.mouse_focus->want_double_click()) {

						// double click not implemented atm.
						assert(false);


					} else {
						event_info_.mouse_focus->mouse_click(event_info_, handled);

					}
				}
			}
			
			event_info_.mouse_left_button_down = false;
			break;
		case SDL_BUTTON_MIDDLE : 
			event_info_.mouse_middle_button_down = false; 
			break;
		case SDL_BUTTON_RIGHT  : 
			event_info_.mouse_right_button_down = false;  
			break;

		// Note: other mouse buttons are ignored, the event
		// is send but the status won't be remembered.
	}

	event_info_.mouse_focus = 0; // Note to see what to do after moving
}

//! Handler for a mouse movement.
void twindow::handle_event_mouse_move(const SDL_Event& event)
{
	event_info_.mouse_x = event.button.x;
	event_info_.mouse_y = event.button.y;
	twidget* mouse_focus = get_widget(tpoint(event_info_.mouse_x - get_x(), event_info_.mouse_y - get_y()));

	if(mouse_focus != event_info_.mouse_focus) {
		if(event_info_.mouse_focus) {
			bool handled = false;
			event_info_.mouse_focus->mouse_leave(event_info_, handled);
		}

		if(mouse_focus) {
			bool handled = false;
			mouse_focus->mouse_enter(event_info_, handled);
		}
	}

	event_info_.mouse_focus = mouse_focus;
}

} // namespace gui2

