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

#include "font.hpp"
#include "foreach.hpp"
#include "gui/widgets/window.hpp"
#include "log.hpp"
#include "marked-up_text.hpp"
#include "util.hpp"

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

tcontrol::tcontrol(const unsigned canvas_count) :
	visible_(true),
	multiline_label_(false),
	wrapped_label_(),
	label_(),
	tooltip_(),
	help_message_(),
	canvas_(canvas_count),
	restorer_(),
	config_(0)
{
}

void tcontrol::mouse_hover(tevent_handler& event)
{
	DBG_G_E << "Control: mouse hover.\n"; 
	event.show_tooltip(tooltip_, 5000); //FIXME timout should be a parameter
}

void tcontrol::help_key(tevent_handler& event)
{
	DBG_G_E << "Control: help key.\n"; 
	event.show_help_popup(help_message_, 0); //FIXME timout should be a parameter
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

	// clear the cache.
	wrapped_label_.clear();
	
	// inherited
	twidget::set_size(rect);
}

void tcontrol::set_label(const t_string& label)
{
	if(label == label_) {
		return;
	}

	label_ = label;
	wrapped_label_.clear();
	set_canvas_text();
	set_dirty();
}

tpoint tcontrol::get_minimum_size() const
{
	assert(config_);
	const tpoint min_size(config_->min_width, config_->min_height);
	if(label_.empty()) {
		return min_size;
	}

	if(!multiline_label_) {
		return get_single_line_best_size(min_size);
	} else {
		return get_multi_line_best_size(min_size);
	}
}

tpoint tcontrol::get_best_size() const
{
	assert(config_);

	// Return default on an empty label.
	const tpoint default_size(config_->default_width, config_->default_height);
	if(label_.empty()) {
		return default_size;
	}

	if(!multiline_label_) {
		return get_single_line_best_size(default_size);
	} else {
		return get_multi_line_best_size(default_size);
	}
}

tpoint tcontrol::get_maximum_size() const
{
	assert(config_);
	return tpoint(config_->max_width, config_->max_height);
}

tpoint tcontrol::get_single_line_best_size(const tpoint& config_size) const
{
	assert(!label_.empty());

	// Get the best size depending on the label.
	SDL_Rect rect = font::line_size(label_, config_->text_font_size, config_->text_font_style);
	const tpoint text_size(rect.w + config_->text_extra_width, rect.h + config_->text_extra_height);

	// Get the best size if default has a 0 value always use the size of the text.
	tpoint size(0, 0);
	if(config_size == size) { // config_size == 0,0
		size = text_size;
	} else if(!config_size.x) {
		size = tpoint(text_size.x, maximum(config_size.y, text_size.y));
	} else if(!config_size.y) {
		size = tpoint(maximum(config_size.x, text_size.x), text_size.y);
	} else {
		size = maximum(config_size, text_size);
	}

	// Honour the maximum.
	const tpoint maximum_size(config_->max_width, config_->max_height);
	if(maximum_size.x && size.x > maximum_size.x) {
		size.x = maximum_size.x;
	}
	if(maximum_size.y && size.y > maximum_size.y) {
		size.y = maximum_size.y;
	}
	return size;
}

tpoint tcontrol::get_multi_line_best_size(const tpoint& config_size) const
{
	assert(!label_.empty());

	// In multiline mode we only expect a fixed width and no
	// fixed height so we ignore the height ;-)
	const tpoint maximum_size(config_->max_width, config_->max_height);
	if(config_size.y || maximum_size.y) {
		WRN_G << "Control: Multiline items don't respect the wanted height.\n";
	}
	unsigned width = 0;
	if(!config_size.x && !maximum_size.x) {
		// FIMXE implement
/*		const twindow* window = get_window();
		if(window) {
			const SDL_Rect rect = window->get_client_rect();
			LOG_G << "Control: Multiline items want a width, falling back to window size.\n";
			width = rect.w;
		} else {
*/			ERR_G << "Control: Multiline items want a width, no window setting hardcoded.\n";
			width = 100;
//		}
	} else {

		if(!config_size.x) {
			width = maximum_size.x;
		} else if(!maximum_size.x) {
			width = config_size.x;
		} else {
			width = minimum(config_size.x, maximum_size.x);
		}
	}

	static const SDL_Color col = {0, 0, 0, 0};
	const std::string& wrapped_message = font::word_wrap_text(label_, config_->text_font_size, width);
	surface surf = font::get_rendered_text(wrapped_message, config_->text_font_size, col);
	assert(surf);

	std::cerr << "Multiline size for width: " << width << " surface size: "  << surf->w << ',' << surf->h << ".\n";
	return tpoint(surf->w + config_->text_extra_width, surf->h + config_->text_extra_height);
}

//! Does the widget need to restore the surface before (re)painting?
bool tcontrol::full_redraw() const
{
	assert(config());
	return config()->state[get_state()].full_redraw;
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
	assert(config_);

	set_dirty(false);
	SDL_Rect rect = get_rect();

	if(!visible_) {
		// When not visible we first restore our original surface.
		// Next time when visible we grab the background again.
		if(restorer_) {
			DBG_G_D << "Control: drawing setting invisible.\n";
			restore_background(surface);
			restorer_ = 0;
		}
		return;
	}

	DBG_G_D << "Control: drawing.\n";
	if(!restorer_) {
		save_background(surface);
	} else if(full_redraw()) {
		restore_background(surface);
	}

	if(multiline_label_) {
		// Set the text hardcoded in multiline mode.
		if(wrapped_label_.empty()) {
			wrapped_label_ = font::word_wrap_text(label_, config_->text_font_size, get_width());
		}
		canvas(get_state()).set_variable("text", variant(wrapped_label_));
	}

	canvas(get_state()).draw(true);
	blit_surface(canvas(get_state()).surf(), 0, surface, &rect);
}

//! Saves the portion of the background.
//!
//! We expect an empty restorer and copy the part in get_rect() to the new 
//! surface. We copy the data since we want to put it back 1:1 and not a blit
//! so can't use get_surface_portion.
//!
//! @param src          background to save.
void tcontrol::save_background(const surface& src)
{
	assert(src);
	assert(!restorer_);

	const SDL_Rect rect = get_rect();

	restorer_.assign(SDL_CreateRGBSurface(SDL_SWSURFACE, 
		rect.w, rect.h, 32, 0xFF0000, 0xFF00, 0xFF, 0xFF000000));

	{
		// Extra scoping used for the surface_lock.
		surface_lock src_lock(src);
		surface_lock dst_lock(restorer_);

		Uint32* src_pixels = reinterpret_cast<Uint32*>(src_lock.pixels());
		Uint32* dst_pixels = reinterpret_cast<Uint32*>(dst_lock.pixels());

		unsigned offset = rect.y * src->w + rect.x;
		for(unsigned y = 0; y < rect.h; ++y) {
			for(unsigned x = 0; x < rect.w; ++x) {

				*dst_pixels++ = src_pixels[offset + x];
			
			}
		offset += src->w;
		}
	}
}

//! Restores a portion of the background.
//!
//! See save_background for more info.
//! 
//! @param dst          Background to restore.
void tcontrol::restore_background(surface& dst)
{
	assert(restorer_);
	assert(dst);

	const SDL_Rect rect = get_rect();

	{
		// Extra scoping used for the surface_lock.
		surface_lock src_lock(restorer_);
		surface_lock dst_lock(dst);

		Uint32* src_pixels = reinterpret_cast<Uint32*>(src_lock.pixels());
		Uint32* dst_pixels = reinterpret_cast<Uint32*>(dst_lock.pixels());

		unsigned offset = rect.y * dst->w + rect.x;
		for(unsigned y = 0; y < rect.h; ++y) {
			for(unsigned x = 0; x < rect.w; ++x) {

				dst_pixels[offset + x] = *src_pixels++;

			}
		offset += dst->w;
		}
	}
}

void tcontrol::load_config()
{
	if(!config()) {
		set_config(get_control(get_control_type(), definition()));

		assert(canvas().size() == config()->state.size());
		for(size_t i = 0; i < canvas().size(); ++i) {
			canvas(i) = config()->state[i].canvas;
		}

		set_canvas_text();

		load_config_extra();
	}
}

} // namespace gui2


