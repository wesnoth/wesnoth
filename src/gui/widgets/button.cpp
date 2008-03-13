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
	canvas_enabled_.set_width(width);
	canvas_disabled_.set_width(width);
	canvas_pressed_.set_width(width);
	canvas_focussed_.set_width(width);

	// inherited
	tcontrol::set_width(width);
}

void tbutton::set_height(const int height) 
{ 
	// resize canvasses
	canvas_enabled_.set_height(height);
	canvas_disabled_.set_height(height);
	canvas_pressed_.set_height(height);
	canvas_focussed_.set_height(height);

	// inherited
	tcontrol::set_height(height);
}

void tbutton::mouse_down(const tevent_info& /*event*/, bool& /*handled*/) 
{ 
	DBG_GUI << "mouse down\n"; 

	set_state(PRESSED);
}

void tbutton::mouse_up(const tevent_info& /*event*/, bool& /*handled*/) 
{ 
	DBG_GUI << "mouse up\n";

	set_state(FOCUSSED);
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

	set_state(FOCUSSED);
}

void tbutton::mouse_leave(const tevent_info& /*event*/, bool& /*handled*/) 
{ 
	DBG_GUI << "mouse leave\n"; 

	set_state(ENABLED);
}

void tbutton::draw(surface& canvas)
{
	DBG_GUI << "Drawing button\n";

	SDL_Rect rect = get_rect();
	switch(state_) {

		case ENABLED : 
			DBG_GUI << "Enabled.\n";
			canvas_enabled_.draw(true);
			SDL_BlitSurface(canvas_enabled_.surf(), 0, canvas, &rect);
			break;

		case DISABLED : 
			DBG_GUI << "Disabled.\n";
			canvas_disabled_.draw(true);
			SDL_BlitSurface(canvas_disabled_.surf(), 0, canvas, &rect);
			break;

		case PRESSED :
			DBG_GUI << "Pressed.\n";
			canvas_pressed_.draw(true);
			SDL_BlitSurface(canvas_pressed_.surf(), 0, canvas, &rect);
			break;

		case FOCUSSED :
			DBG_GUI << "Focussed.\n";
			canvas_focussed_.draw(true);
			SDL_BlitSurface(canvas_focussed_.surf(), 0, canvas, &rect);
			break;
	}

	set_dirty(false);
}

tpoint tbutton::get_best_size() const
{
	if(definition_ == std::vector<tbutton_definition::tresolution>::const_iterator()) {
		return tpoint(get_button(definition())->default_width, get_button(definition())->default_height); 
	} else {
		return tpoint(definition_->default_width, definition_->default_height); 
	}
}

void tbutton::set_best_size(const tpoint& origin)
{
	resolve_definition();

	set_x(origin.x);
	set_y(origin.y);
	set_width(definition_->default_width);
	set_height(definition_->default_height);
}

void tbutton::set_state(tstate state)
{
	if(state != state_) {
		state_ = state;
		set_dirty(true);
	}
}

void tbutton::resolve_definition()
{
	if(definition_ == std::vector<tbutton_definition::tresolution>::const_iterator()) {
		definition_ = get_button(definition());

		canvas_enabled_ = definition_->enabled.canvas;
		canvas_disabled_ = definition_->disabled.canvas;
		canvas_pressed_ = definition_->pressed.canvas;
		canvas_focussed_ = definition_->focussed.canvas;
	}
}

} // namespace gui2
