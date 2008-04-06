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

#include "gui/widgets/label.hpp"

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

tpoint tlabel::get_best_size() const
{
	if(definition_ == std::vector<tlabel_definition::tresolution>::const_iterator()) {
		return tpoint(get_label(definition())->default_width, get_label(definition())->default_height); 
	} else {
		return tpoint(definition_->default_width, definition_->default_height); 
	}
}

void tlabel::mouse_hover(tevent_handler&)
{
	DBG_G_E << "Text_box: mouse hover.\n"; 
}

void tlabel::draw(surface& surface)
{
	SDL_Rect rect = get_rect();

	DBG_G_D << "Label: drawing enabled state.\n";
/*	if(!restorer_) {
		restorer_ = get_surface_portion(canvas, rect);
	} 
	if(definition_->enabled.full_redraw) {
		SDL_BlitSurface(restorer_, 0, canvas, &rect);
		rect = get_rect();
	}
*/
	canvas(0).draw(true);
	SDL_BlitSurface(canvas(0).surf(), 0, surface, &rect);

	set_dirty(false);
}

void tlabel::set_best_size(const tpoint& origin)
{
	resolve_definition();

	set_x(origin.x);
	set_y(origin.y);
	set_width(definition_->default_width);
	set_height(definition_->default_height);
}

void tlabel::set_state(tstate state)
{
	if(state != state_) {
		state_ = state;
		set_dirty(true);
	}
}

void tlabel::resolve_definition()
{
	if(definition_ == std::vector<tlabel_definition::tresolution>::const_iterator()) {
		definition_ = get_label(definition());

		assert(canvas().size() == definition_->state.size());
		for(size_t i = 0; i < canvas().size(); ++i) {
			canvas(i) = definition_->state[i].canvas;
		}

		 set_canvas_text();
	}
}

} // namespace gui2


