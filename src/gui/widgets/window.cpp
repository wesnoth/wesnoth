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
#include "gui/widgets/settings.hpp"
#include "log.hpp"
#include "serialization/parser.hpp"
#include "variable.hpp"

#include <cassert>

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


namespace gui2{

twindow::twindow(CVideo& video, 
		const int x, const int y, const int w, const int h) :
	tpanel(),
	events::handler(false), // don't join we haven't created a context yet
	video_(video),
	status_(NEW),
	event_info_(),
	event_context_(),
	need_layout_(true),
	restorer_(),
	canvas_background_(),
	canvas_foreground_()
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
	log_scope2(gui_draw, "Window: show.");	

	// Sanity
	if(status_ != NEW) {
		// FIXME throw an exception

	}
	
	// We cut a piec of the screen and use that, that way all coordinates
	// are relative to the window.
	SDL_Rect rect = get_rect();
	restorer_ = get_surface_portion(video_.getSurface(), rect);
	surface screen;

	// Start our loop drawing will happen here as well.
	for(status_ = SHOWING; status_ != REQUEST_CLOSE; ) {
		events::pump();

		// fixme manual destroy
		if(status_ == REQUEST_CLOSE) {
			break;
		}

		if(dirty() || need_layout_) {
			const bool draw_foreground = need_layout_;
			if(need_layout_) {
				DBG_G << "Window: layout client area.\n";
				resolve_definition();
				layout(get_client_rect());

				screen = create_optimized_surface(restorer_);

				canvas_background_.draw();
				SDL_Rect blit = {0, 0, screen->w, screen->h};
				SDL_BlitSurface(canvas_background_.surf(), 0, screen, &blit);
			}
#if 0			
			// Darkening for debugging redraw.
			SDL_Rect temp_rect = {0, 0, screen->w, screen->h};
			fill_rect_alpha(temp_rect, 0, 1, screen);
#endif

			for(tgrid::iterator itor = begin(); itor != end(); ++itor) {

				if(! *itor || !itor->dirty()) {
					continue;
				}

				log_scope2(gui_draw, "Window: draw child.");

				itor->draw(screen);
			}
			if(draw_foreground) {
				canvas_foreground_.draw();
				SDL_Rect blit = {0, 0, screen->w, screen->h};
				SDL_BlitSurface(canvas_foreground_.surf(), 0, screen, &blit);
			}

			rect = get_rect();
			SDL_BlitSurface(screen, 0, video_.getSurface(), &rect);
			update_rect(get_rect());
			set_dirty(false);
		}

		// delay until it's our frame see display.ccp code for how to do that
		SDL_Delay(10);
		flip();
	}

	// restore area
	if(restore) {
		rect = get_rect();
		SDL_BlitSurface(restorer_, 0, video_.getSurface(), &rect);
		update_rect(get_rect());
		flip();
	}
}

void twindow::layout(const SDL_Rect position)
{
	need_layout_ = false;

	tpoint best_size = get_best_size();

	if(best_size.x < position.w && best_size.y < position.h) {
		set_best_size(tpoint(position.x, position.y));
		return;
	}

	DBG_G << "Window: layout can't be set to best size, try minimum.\n";

	// Implement the code.
	assert(false);

	// Failed at best size try minumum.
	
	// Failed at minimum log error and try to do the best possible thing.
}

void twindow::set_width(const unsigned width)
{
	canvas_background_.set_width(width);
	canvas_foreground_.set_width(width);
	need_layout_ = true;

	// inherited
	tcontrol::set_width(width);
}

void twindow::set_height(const unsigned height)
{
	canvas_background_.set_height(height);
	canvas_foreground_.set_height(height);
	need_layout_ = true;

	// inherited
	tcontrol::set_height(height);
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
	if(event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
		event_info_.handle_event(event, get_widget(tpoint(event.button.x - get_x(), event.button.y - get_y())));
	} else if (event.type == SDL_MOUSEMOTION) {
		event_info_.handle_event(event, get_widget(tpoint(event.motion.x - get_x(), event.motion.y - get_y())));
	} else {
		event_info_.handle_event(event, 0);
	}
}

void twindow::window_resize(tevent_info&, 
		const unsigned new_width, const unsigned new_height)
{
	screen_width = new_width;
	screen_height = new_height;
	need_layout_ = true;
}

void twindow::resolve_definition()
{
	if(definition_ == std::vector<twindow_definition::tresolution>::const_iterator()) {
		definition_ = get_window(definition());

		canvas_background_ = definition_->background.canvas;
		canvas_background_.set_width(get_width());
		canvas_background_.set_height(get_height());

		canvas_foreground_ = definition_->foreground.canvas;
		canvas_foreground_.set_width(get_width());
		canvas_foreground_.set_height(get_height());
	}

}
SDL_Rect twindow::get_client_rect()
{
	assert(definition_ != std::vector<twindow_definition::tresolution>::const_iterator());

	SDL_Rect result = get_rect();
	result.x = definition_->left_border;
	result.y = definition_->top_border;
	result.w -= definition_->left_border + definition_->right_border;
	result.h -= definition_->top_border + definition_->bottom_border;

	// FIXME validate for an available client area.
	
	return result;

}

} // namespace gui2

