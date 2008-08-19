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

#include "gui/widgets/container.hpp"

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

tpoint tcontainer_::get_minimum_size() const
{
	tpoint size = grid_.get_maximum_size();
	tpoint border_size = border_space();

	if(size.x) {
		size.x += border_size.x;
	}

	if(size.y) {
		size.y += border_size.y;
	}

	return size;
}

tpoint tcontainer_::get_best_size() const
{
	tpoint size = grid_.get_best_size();
	const tpoint border_size = border_space();

	// If the best size has a value of 0 it's means no limit so don't add the
	// border_size might set a very small best size.
	if(size.x) {
		size.x += border_size.x;
	}

	if(size.y) {
		size.y += border_size.y;
	}

	return size;
}

tpoint tcontainer_::get_best_size(const tpoint& maximum_size) const
{
	log_scope2(gui, "Container: Get best size");	
	
	// We need a copy and adjust if for the borders, no use to ask the grid for
	// the best size if it won't fit in the end due to our borders.
	const tpoint border_size = border_space();
	tpoint max = maximum_size;
	if(max.x) {
		max.x -= border_size.x;
	}

	if(max.y) {
		max.y -= border_size.y;
	}

	// Calculate the best size
	tpoint size = grid_.get_best_size(max);

	// If the best size has a value of 0 it's means no limit so don't add the
	// border_size might set a very small best size.
	if(size.x) {
		size.x += border_size.x;
	}

	if(size.y) {
		size.y += border_size.y;
	}
	
	DBG_G << "Container : maximum size " 
		<< maximum_size << " returning " << size << ".\n";

	return size;
}

void tcontainer_::draw(surface& surface, const bool force,
		const bool invalidate_background)
{
	// Inherited.
	tcontrol::draw(surface, force, invalidate_background);

	const bool redraw_background = invalidate_background || has_background_changed();
	set_background_changed(false);

	grid_.draw(surface, force, redraw_background);
}

void tcontainer_::set_active(const bool active)
{
	// Not all our children might have the proper state so let them run
	// unconditionally.
	grid_.set_active(active);

	if(active == get_active()) {
		return;
	}

	set_dirty();

	set_self_active(active);
}

void tcontainer_::set_dirty(const bool dirty)
{
	// Inherited.
	twidget::set_dirty(dirty);

	if(!dirty) {
		grid_.set_dirty(dirty);
	}
}

} // namespace gui2

