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

#define DBG_G LOG_STREAM(debug, gui)
#define LOG_G LOG_STREAM(info, gui)
#define WRN_G LOG_STREAM(warn, gui)
#define ERR_G LOG_STREAM(err, gui)

#define DBG_G_D LOG_STREAM(debug, gui_draw)
#define LOG_G_D LOG_STREAM(info, gui_draw)
#define WRN_G_D LOG_STREAM(warn, gui_draw)
#define ERR_G_D LOG_STREAM(err, gui_draw)

#define DBG_G_E LOG_STREAM(debug, gui_event)
#define LOG_G_E LOG_STREAM(info, gui_event)
#define WRN_G_E LOG_STREAM(warn, gui_event)
#define ERR_G_E LOG_STREAM(err, gui_event)

#define DBG_G_P LOG_STREAM(debug, gui_parse)
#define LOG_G_P LOG_STREAM(info, gui_parse)
#define WRN_G_P LOG_STREAM(warn, gui_parse)
#define ERR_G_P LOG_STREAM(err, gui_parse)

namespace gui2 {

void tbutton::set_width(const unsigned width)
{ 
	// resize canvasses
	canvas_enabled_.set_width(width);
	canvas_disabled_.set_width(width);
	canvas_pressed_.set_width(width);
	canvas_focussed_.set_width(width);

	// inherited
	tcontrol::set_width(width);
}

void tbutton::set_height(const unsigned height) 
{ 
	// resize canvasses
	canvas_enabled_.set_height(height);
	canvas_disabled_.set_height(height);
	canvas_pressed_.set_height(height);
	canvas_focussed_.set_height(height);

	// inherited
	tcontrol::set_height(height);
}

void tbutton::set_label(const t_string& label)
{

	// set label in canvases
	canvas_enabled_.set_variable("text", variant(label.str()));
	canvas_disabled_.set_variable("text", variant(label.str()));
	canvas_pressed_.set_variable("text", variant(label.str()));
	canvas_focussed_.set_variable("text", variant(label.str()));

	// inherited
	tcontrol::set_label(label);
}

void tbutton::mouse_enter(tevent_handler&) 
{ 
	DBG_G_E << "Button: mouse enter.\n"; 

	set_state(FOCUSSED);
}

void tbutton::mouse_hover(tevent_handler&)
{
	DBG_G_E << "Button: mouse hover.\n"; 

}

void tbutton::mouse_leave(tevent_handler&) 
{ 
	DBG_G_E << "Button: mouse leave.\n"; 

	set_state(ENABLED);
}

void tbutton::mouse_left_button_down(tevent_handler& event) 
{ 
	DBG_G_E << "Button: left mouse button down.\n"; 

	event.mouse_capture();

	set_state(PRESSED);
}

void tbutton::mouse_left_button_up(tevent_handler&) 
{ 
	DBG_G_E << "Button: left mouse button up.\n";

	set_state(FOCUSSED);
}

void tbutton::mouse_left_button_click(tevent_handler&) 
{ 
	DBG_G_E << "Button: left mouse button click.\n"; 
}

void tbutton::mouse_left_button_double_click(tevent_handler&) 
{ 
	DBG_G_E << "Button: left mouse button double click.\n"; 
}

void tbutton::draw(surface& canvas)
{
	SDL_Rect rect = get_rect();
	switch(state_) {

		case ENABLED : 
			DBG_G_D << "Button: drawing enabled state.\n";
			canvas_enabled_.draw(true);
			SDL_BlitSurface(canvas_enabled_.surf(), 0, canvas, &rect);
			break;

		case DISABLED : 
			DBG_G_D << "Button: drawing disabled state.\n";
			canvas_disabled_.draw(true);
			SDL_BlitSurface(canvas_disabled_.surf(), 0, canvas, &rect);
			break;

		case PRESSED :
			DBG_G_D << "Button: drawing pressed state.\n";
			canvas_pressed_.draw(true);
			SDL_BlitSurface(canvas_pressed_.surf(), 0, canvas, &rect);
			break;

		case FOCUSSED :
			DBG_G_D << "Button: drawing focussed state.\n";
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

		// FIXME we need some extra routines since a lot of code will
		// be duplicated here otherwise.
		canvas_enabled_.set_variable("text", variant(label()));
		canvas_disabled_.set_variable("text", variant(label()));
		canvas_pressed_.set_variable("text", variant(label()));
		canvas_focussed_.set_variable("text", variant(label()));

	}
}

} // namespace gui2
