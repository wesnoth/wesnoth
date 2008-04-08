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

#include "gui/widgets/control.hpp"

#include "foreach.hpp"
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

tcontrol::tcontrol(const unsigned canvas_count) :
	visible_(true),
	label_(),
	tooltip_(),
	help_message_(),
	canvas_(canvas_count),
	restorer_(),
	config_(0)
{
}

void tcontrol::set_width(const unsigned width)
{ 
	// resize canvasses
	foreach(tcanvas& canvas, canvas_) {
		canvas.set_width(width);
	}

	// inherited
	twidget::set_width(width);
}

void tcontrol::set_height(const unsigned height) 
{ 
	// resize canvasses
	foreach(tcanvas& canvas, canvas_) {
		canvas.set_height(height);
	}

	// inherited
	twidget::set_height(height);
}

void tcontrol::set_size(const SDL_Rect& rect)
{
	// resize canvasses
	foreach(tcanvas& canvas, canvas_) {
		canvas.set_width(rect.w);
		canvas.set_height(rect.h);
	}

	// inherited
	twidget::set_size(rect);
}

void tcontrol::set_label(const t_string& label)
{
	if(label == label_) {
		return;
	}

	label_ = label; 
	set_canvas_text();
	set_dirty();
}

tpoint tcontrol::get_minimum_size() const
{
	assert(config_);
	return tpoint(config_->min_width, config_->min_height);
}

tpoint tcontrol::get_best_size() const
{
	assert(config_);
	return tpoint(config_->default_width, config_->default_height);
}

tpoint tcontrol::get_maximum_size() const
{
	assert(config_);
	return tpoint(config_->max_width, config_->max_height);
}


//! Sets the text variable for the canvases.
void tcontrol::set_canvas_text()
{
	// set label in canvases
	foreach(tcanvas& canvas, canvas_) {
		canvas.set_variable("text", variant(label_));
	}
}

void tcontrol::draw(surface& surface)
{
	SDL_Rect rect = get_rect();

	DBG_G_D << "Control: drawing.\n";
	if(!restorer_) {
		restorer_ = get_surface_portion(surface, rect);
	} 
	if(full_redraw()) {
		SDL_BlitSurface(restorer_, 0, surface, &rect);
		rect = get_rect();
	}
	canvas(get_state()).draw(true);
	SDL_BlitSurface(canvas(get_state()).surf(), 0, surface, &rect);

	set_dirty(false);
}

} // namespace gui2


