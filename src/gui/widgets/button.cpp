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

#include "gui/widgets/window.hpp"
#include "log.hpp"

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

	// If a button has a retval do the default handling.
	if(retval_ != 0) {
		twindow* window = get_window();
		if(window) {
			window->set_retval(retval_);
			return;
		}
	}

	// Do the custom handling (not implemented yet) FIXME
}

void tbutton::mouse_left_button_double_click(tevent_handler&) 
{ 
	DBG_G_E << "Button: left mouse button double click.\n"; 
}

tpoint tbutton::get_minimum_size() const
{
	if(definition_ == std::vector<tbutton_definition::tresolution>::const_iterator()) {
		return tpoint(get_button(definition())->min_width, get_button(definition())->min_height); 
	} else {
		return tpoint(definition_->min_width, definition_->min_height); 
	}
}

tpoint tbutton::get_best_size() const
{
	if(definition_ == std::vector<tbutton_definition::tresolution>::const_iterator()) {
		return tpoint(get_button(definition())->default_width, get_button(definition())->default_height); 
	} else {
		return tpoint(definition_->default_width, definition_->default_height); 
	}
}

tpoint tbutton::get_maximum_size() const
{
	if(definition_ == std::vector<tbutton_definition::tresolution>::const_iterator()) {
		return tpoint(get_button(definition())->max_width, get_button(definition())->max_height); 
	} else {
		return tpoint(definition_->max_width, definition_->max_height); 
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

tbutton::RETVAL tbutton::get_retval_by_id(const std::string& id)
{
	//! Note it might change to a map later depending on the number
	//! of items.
	if(id == "ok") {
		return OK;
	} else if(id == "cancel") {
		return CANCEL;
	} else {
		return NONE;
	}

}

void tbutton::set_active(const bool active)
{
	if(active && state_ == DISABLED) {
		set_state(ENABLED);
	} else if(!active && state_ != DISABLED) {
		set_state(DISABLED);
	}
}

bool tbutton::get_active() const
{
	return state_ != DISABLED;
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

		assert(canvas().size() == definition_->state.size());
		for(size_t i = 0; i < canvas().size(); ++i) {
			canvas(i) = definition_->state[i].canvas;
		}

		set_canvas_text();
	}
}

} // namespace gui2
