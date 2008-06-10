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
#include "cursor.hpp"
#include "font.hpp"
#include "log.hpp"
#include "serialization/parser.hpp"
#include "variable.hpp"
#include "sdl_utils.hpp"

#include <cassert>

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


namespace gui2{

twindow::twindow(CVideo& video, 
		const int x, const int y, const int w, const int h,
		const bool automatic_placement, 
		const unsigned horizontal_placement,
		const unsigned vertical_placement) :
	tpanel(),
	tevent_handler(),
	video_(video),
	status_(NEW),
	retval_(0),
	owner_(0),
	need_layout_(true),
	tooltip_(),
	help_popup_(),
	automatic_placement_(automatic_placement),
	horizontal_placement_(horizontal_placement),
	vertical_placement_(vertical_placement)
{
	// We load the config in here as exception.
	load_config();

	set_size(::create_rect(x, y, w, h));

	tooltip_.set_definition("default");
	tooltip_.set_visible(false);

	help_popup_.set_definition("default");
	help_popup_.set_visible(false);
}

int twindow::show(const bool restore, void* /*flip_function*/)
{
	log_scope2(gui_draw, "Window: show.");	

	assert(status_ == NEW);

	// We cut a piece of the screen and use that, that way all coordinates
	// are relative to the window.
	SDL_Rect rect = get_rect();
	surface restorer = get_surface_portion(video_.getSurface(), rect);
	surface screen;

	// Start our loop drawing will happen here as well.
	for(status_ = SHOWING; status_ != REQUEST_CLOSE; ) {
		process_events();

		if(status_ == REQUEST_CLOSE) {
			break;
		}

		if(dirty() || need_layout_) {
			if(need_layout_) {
				screen = make_neutral_surface(restorer);
			}
			draw(screen);
		}

		// delay until it's our frame see display.ccp code for how to do that
		SDL_Delay(10);
		flip();
	}

	// restore area
	if(restore) {
		rect = get_rect();
		SDL_BlitSurface(restorer, 0, video_.getSurface(), &rect);
		update_rect(get_rect());
		flip();
	}

	return retval_;
}

void twindow::layout(const SDL_Rect position)
{
	need_layout_ = false;

	DBG_G << "Window: layout area " << position.x
		<< ',' << position.y << " x " << position.w 
		<< ',' << position.h << ".\n";

	set_client_size(position); 
	need_layout_ = false;
}

//! Inherited from tpanel.
void twindow::draw(surface& surface)
{
	const bool draw_foreground = need_layout_;
	if(need_layout_) {
		DBG_G << "Window: layout client area.\n";
		layout(get_client_rect());

		canvas(0).draw();
		blit_surface(canvas(0).surf(), 0, surface, 0);
	}
	
	for(tgrid::iterator itor = begin(); itor != end(); ++itor) {
		if(! *itor || !itor->dirty()) {
			continue;
		}

		log_scope2(gui_draw, "Window: draw child.");

		itor->draw(surface);
	}
	if(draw_foreground) {
		canvas(1).draw();
		blit_surface(canvas(1).surf(), 0, surface, 0);
	}
	if(tooltip_.dirty()) {
		tooltip_.draw(surface);
	}
	if(help_popup_.dirty()) {
		help_popup_.draw(surface);
	}

	SDL_Rect rect = get_rect();
	SDL_BlitSurface(surface, 0, video_.getSurface(), &rect);
	update_rect(get_rect());
	set_dirty(false);
}

void twindow::flip()
{
	// fixme we need to add the option to either call
	// video_.flip() or display.flip()
	
	const surface frameBuffer = get_video_surface();
	
	cursor::draw(frameBuffer);
	video_.flip();
	cursor::undraw(frameBuffer);
}

void twindow::window_resize(tevent_handler&, 
		const unsigned new_width, const unsigned new_height)
{
	settings::screen_width = new_width;
	settings::screen_height = new_height;
	need_layout_ = true;
}

SDL_Rect twindow::get_client_rect() const
{
	const twindow_definition::tresolution* conf = dynamic_cast<const twindow_definition::tresolution*>(config());
	assert(conf);

	SDL_Rect result = get_rect();
	result.x = conf->left_border;
	result.y = conf->top_border;
	result.w -= conf->left_border + conf->right_border;
	result.h -= conf->top_border + conf->bottom_border;

	// FIXME validate for an available client area.
	
	return result;

}

void twindow::recalculate_size()
{
	if(automatic_placement_) {
		
		tpoint size = get_best_size();
		size.x = size.x < static_cast<int>(settings::screen_width) ? size.x : static_cast<int>(settings::screen_width);
		size.y = size.y < static_cast<int>(settings::screen_height) ? size.y : static_cast<int>(settings::screen_height);

		tpoint position(0, 0);
		switch(horizontal_placement_) {
			case tgrid::HORIZONTAL_ALIGN_LEFT :
				// Do nothing
				break;
			case tgrid::HORIZONTAL_ALIGN_CENTER :
				position.x = (settings::screen_width - size.x) / 2;
				break;
			case tgrid::HORIZONTAL_ALIGN_RIGHT :
				position.x = settings::screen_width - size.x;
				break;
			default :
				assert(false);
		}
		switch(vertical_placement_) {
			case tgrid::VERTICAL_ALIGN_TOP :
				// Do nothing
				break;
			case tgrid::VERTICAL_ALIGN_CENTER :
				position.y = (settings::screen_height - size.y) / 2;
				break;
			case tgrid::VERTICAL_ALIGN_BOTTOM :
				position.y = settings::screen_height - size.y;
				break;
			default :
				assert(false);
		}

		set_size(create_rect(position, size));
	}
}

void twindow::do_show_tooltip(const tpoint& location, const t_string& tooltip)
{
	DBG_G << "Showing tooltip message: '" << tooltip << "'.\n";

	assert(!tooltip.empty());

	twidget* widget = find_widget(location, true);
	assert(widget);
	
	const SDL_Rect widget_rect = widget->get_rect();
	const SDL_Rect client_rect = get_client_rect();

	tooltip_.set_label(tooltip);
	const tpoint size = tooltip_.get_best_size();

	SDL_Rect tooltip_rect = {0, 0, size.x, size.y};

	// Find the best position to place the widget
	if(widget_rect.y - size.y > 0) {
		// put above
		tooltip_rect.y = widget_rect.y - size.y;
	} else {
		//put below no test
		tooltip_rect.y = widget_rect.y + widget_rect.h;
	}

	if(widget_rect.x + size.x < client_rect.w) {
		// Directly above the mouse
		tooltip_rect.x = widget_rect.x;
	} else {
		// shift left, no test
		tooltip_rect.x = client_rect.w - size.x;
	}

	tooltip_.set_size(tooltip_rect);
	tooltip_.set_visible();
}

void twindow::do_show_help_popup(const tpoint& location, const t_string& help_popup)
{
	// Note copy past of twindow::do_show_tooltip except that the help may be empty.
	DBG_G << "Showing help message: '" << help_popup << "'.\n";

	if(help_popup.empty()) {
		return;
	}
	twidget* widget = find_widget(location, true);
	assert(widget);
	
	const SDL_Rect widget_rect = widget->get_rect();
	const SDL_Rect client_rect = get_client_rect();

	help_popup_.set_label(help_popup);
	const tpoint size = help_popup_.get_best_size();

	SDL_Rect help_popup_rect = {0, 0, size.x, size.y};

	// Find the best position to place the widget
	if(widget_rect.y - size.y > 0) {
		// put above
		help_popup_rect.y = widget_rect.y - size.y;
	} else {
		//put below no test
		help_popup_rect.y = widget_rect.y + widget_rect.h;
	}

	if(widget_rect.x + size.x < client_rect.w) {
		// Directly above the mouse
		help_popup_rect.x = widget_rect.x;
	} else {
		// shift left, no test
		help_popup_rect.x = client_rect.w - size.x;
	}

	help_popup_.set_size(help_popup_rect);
	help_popup_.set_visible();
}

} // namespace gui2

