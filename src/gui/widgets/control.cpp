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

#include "gui/widgets/control.hpp"

#include "foreach.hpp"
#include "gui/widgets/window.hpp"
#include "log.hpp"
#include "../../text.hpp"

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
	label_(),
	multiline_label_(false),
	wrapped_label_(),
	tooltip_(),
	help_message_(),
	canvas_(canvas_count),
	restorer_(),
	config_(0)
{
}

void tcontrol::set_members(const std::map<
		std::string /* member id */, t_string /* member value */>& data)
{
	std::map<std::string, t_string>::const_iterator itor = data.find("label");
	if(itor != data.end()) {
		set_label(itor->second);
	}

	itor = data.find("tooltip");
	if(itor != data.end()) {
		set_tooltip(itor->second);
	}

	itor = data.find("help");
	if(itor != data.end()) {
		set_help_message(itor->second);
	}
}

void tcontrol::mouse_hover(tevent_handler& event)
{
	DBG_G_E << "Control: mouse hover.\n"; 
	event.show_tooltip(tooltip_, settings::popup_show_time);
}

void tcontrol::help_key(tevent_handler& event)
{
	DBG_G_E << "Control: help key.\n"; 
	event.show_help_popup(help_message_, settings::help_show_time);
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

tpoint tcontrol::get_minimum_size() const
{
	assert(config_);
	const tpoint min_size(config_->min_width, config_->min_height);
	if(label_.empty()) {
		return min_size;
	}

	return get_best_text_size(min_size, multiline_label_);
}

tpoint tcontrol::get_best_size() const
{
	assert(config_);

	// Return default on an empty label.
	const tpoint default_size(config_->default_width, config_->default_height);
	if(label_.empty()) {
		return default_size;
	}

	return get_best_text_size(default_size, multiline_label_);
}

tpoint tcontrol::get_maximum_size() const
{
	assert(config_);
	return tpoint(config_->max_width, config_->max_height);
}

void tcontrol::draw(surface& surface, const bool force, 
		const bool invalidate_background)
{
	assert(config_);

	if(!is_dirty() && !force && !invalidate_background) {
		return;
	}

	if(invalidate_background) {
		restorer_ = 0;
	}

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
	} else if(needs_full_redraw()) {
		restore_background(surface);
	}

	canvas(get_state()).draw(true);
	blit_surface(canvas(get_state()).surf(), 0, surface, &rect);
}

void tcontrol::set_definition(const std::string& definition)
{
	assert(!config());
	twidget::set_definition(definition);
	load_config();
	assert(config());
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

bool tcontrol::needs_full_redraw() const
{
	assert(config());
	return config()->state[get_state()].full_redraw;
}

void tcontrol::set_canvas_text()
{
	// set label in canvases
	foreach(tcanvas& canvas, canvas_) {
		canvas.set_variable("text", variant(label_));
	}
}

void tcontrol::save_background(const surface& src)
{
	assert(!restorer_);

	restorer_ = gui2::save_background(src, get_rect());
}

void tcontrol::restore_background(surface& dst)
{
	gui2::restore_background(restorer_, dst, get_rect());
}

tpoint tcontrol::get_best_text_size(const tpoint& minimum_size) const
{
	assert(!label_.empty());

	const tpoint border(config_->text_extra_width, config_->text_extra_height);
	tpoint size = minimum_size - border;

	font::ttext text;
	text.set_text(label_, false);
	text.set_font_size(config_->text_font_size);

	// Try with the minimum wanted size.
	text.set_maximum_width(size.x);
	text.set_maximum_height(size.y);

	// If doesn't fit try the maximum.
	if(text.is_truncated()) {
		const tpoint maximum_size(config_->max_width, config_->max_height);
		text.set_maximum_width(maximum_size.x ? maximum_size.x - border.x : -1);
		text.set_maximum_height(maximum_size.y ? maximum_size.y - border.y : -1);
	}

	size = text.get_size() + border;

	if(size.x < minimum_size.x) {
		size.x = minimum_size.x;
	}

	if(size.y < minimum_size.y) {
		size.y = minimum_size.y;
	}

	return size;
}

} // namespace gui2

