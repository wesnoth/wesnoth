/* $Id$ */
/*
   copyright (C) 2008 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
*/

#include "gui/widgets/panel.hpp"

#include "log.hpp"

#define DBG_G LOG_STREAM_INDENT(debug, gui)
#define LOG_G LOG_STREAM_INDENT(info, gui)
#define WRN_G LOG_STREAM_INDENT(warn, gui)
#define ERR_G LOG_STREAM_INDENT(err, gui)

#define DBG_G_D LOG_STREAM_INDENT(debug, gui_draw)
#define LOG_G_D LOG_STREAM_INDENT(info, gui_draw)
#define WRN_G_D LOG_STREAM_INDENT(warn, gui_draw)
#define ERR_G_D LOG_STREAM_INDENT(err, gui_draw)

#define DBG_G_E LOG_STREAM_INDENT(debug, gui_event)
#define LOG_G_E LOG_STREAM_INDENT(info, gui_event)
#define WRN_G_E LOG_STREAM_INDENT(warn, gui_event)
#define ERR_G_E LOG_STREAM_INDENT(err, gui_event)

#define DBG_G_P LOG_STREAM_INDENT(debug, gui_parse)
#define LOG_G_P LOG_STREAM_INDENT(info, gui_parse)
#define WRN_G_P LOG_STREAM_INDENT(warn, gui_parse)
#define ERR_G_P LOG_STREAM_INDENT(err, gui_parse)

namespace gui2 {

tpoint tpanel::get_minimum_size() const
{
	const tpoint max_size = get_maximum_size();
	tpoint size = grid_.get_minimum_size() + border_space();

	if(max_size.x) {
		size.x = minimum(size.x, max_size.x);
	}

	if(max_size.y) {
		size.y = minimum(size.y, max_size.y);
	}

	return size;
}

tpoint tpanel::get_best_size() const
{
	const tpoint max_size = get_maximum_size();
	tpoint size = grid_.get_best_size() + border_space();

	if(max_size.x) {
		size.x = minimum(size.x, max_size.x);
	}

	if(max_size.y) {
		size.y = minimum(size.y, max_size.y);
	}

	return size;
}

void tpanel::draw(surface& surface)
{
	// Need to preserve the state and inherited draw clear the flag.
	const bool is_dirty = dirty();
	// background.
	if(is_dirty) {
		tcontrol::draw(surface);
	}

	// children
	grid_.draw(surface);

	// foreground
	if(is_dirty) {
		SDL_Rect rect = get_rect();
    	canvas(1).draw(true);
	    blit_surface(canvas(1).surf(), 0, surface, &rect);
	}
}

SDL_Rect tpanel::get_client_rect() const
{
	const tpanel_definition::tresolution* conf = 
		dynamic_cast<const tpanel_definition::tresolution*>(config());
	assert(conf);

	SDL_Rect result = get_rect();
	result.x += conf->left_border;
	result.y += conf->top_border;
	result.w -= conf->left_border + conf->right_border;
	result.h -= conf->top_border + conf->bottom_border;

	return result;
}

void tpanel::set_size(const SDL_Rect& rect) 
{
	tcontrol::set_size(rect);

	grid_.set_size(get_client_rect());
}

tpoint tpanel::border_space() const
{
	const tpanel_definition::tresolution* conf = 
		dynamic_cast<const tpanel_definition::tresolution*>(config());
	assert(conf);

	return tpoint(conf->left_border + conf->right_border,
		conf->top_border + conf->bottom_border);
}

} // namespace gui2

