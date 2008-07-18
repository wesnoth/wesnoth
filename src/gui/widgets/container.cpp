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
	tpoint border_size = border_space();

	if(size.x) {
		size.x += border_size.x;
	}

	if(size.y) {
		size.y += border_size.y;
	}

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

} // namespace gui2

