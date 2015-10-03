/* $Id$ */
/*
   Copyright (C) 2008 - 2011 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "control.hpp"

#include "font.hpp"
#include "foreach.hpp"
#include "gui/auxiliary/log.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "marked-up_text.hpp"

#include <iomanip>

#define LOG_SCOPE_HEADER "tcontrol(" + get_control_type() + ") [" \
		+ id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2 {

tcontrol::tcontrol(const unsigned canvas_count)
	: label_()
	, use_markup_(false)
	, use_tooltip_on_label_overflow_(true)
	, tooltip_()
	, help_message_()
	, canvas_(canvas_count)
	, config_(0)
	, renderer_()
	, text_maximum_width_(0)
	, shrunken_(false)
{
}

void tcontrol::set_members(const string_map& data)
{
	/** @todo document this feature on the wiki. */
	/** @todo do we need to add the debug colours here as well? */
	string_map::const_iterator itor = data.find("id");
	if(itor != data.end()) {
		set_id(itor->second);
	}

	itor = data.find("linked_group");
	if(itor != data.end()) {
		set_linked_group(itor->second);
	}

	itor = data.find("label");
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

	itor = data.find("use_markup");
	if(itor != data.end()) {
		set_use_markup(utils::string_bool(itor->second));
	}
}

bool tcontrol::disable_click_dismiss() const
{
	return get_visible() == twidget::VISIBLE && get_active();
}

tpoint tcontrol::get_config_minimum_size() const
{
	assert(config_);

	tpoint result(config_->min_width, config_->min_height);

	DBG_GUI_L << LOG_HEADER << " result " << result << ".\n";
	return result;
}

tpoint tcontrol::get_config_default_size() const
{
	assert(config_);

	tpoint result(config_->default_width, config_->default_height);

	DBG_GUI_L << LOG_HEADER << " result " << result << ".\n";
	return result;
}

tpoint tcontrol::get_config_maximum_size() const
{
	assert(config_);

	tpoint result(config_->max_width, config_->max_height);

	DBG_GUI_L << LOG_HEADER << " result " << result << ".\n";
	return result;
}

void tcontrol::layout_init(const bool full_initialization)
{
	// Inherited.
	twidget::layout_init(full_initialization);

	if(full_initialization) {
		shrunken_ = false;
	}
}

void tcontrol::request_reduce_width(const unsigned maximum_width)
{
	assert(config_);

	if(!label_.empty() && can_wrap()) {

		tpoint size = get_best_text_size(
				tpoint(0,0),
				tpoint(maximum_width - config_->text_extra_width, 0));

		size.x += config_->text_extra_width;
		size.y += config_->text_extra_height;

		set_layout_size(size);

		DBG_GUI_L << LOG_HEADER
				<< " label '" << debug_truncate(label_)
				<< "' maximum_width " << maximum_width
				<< " result " << size
				<< ".\n";

	} else {
		DBG_GUI_L << LOG_HEADER
				<< " label '" << debug_truncate(label_)
				<< "' failed; either no label or wrapping not allowed.\n";
	}
}

tpoint tcontrol::calculate_best_size() const
{
	assert(config_);
	tpoint result(config_->default_width, config_->default_height);
	if(! label_.empty()) {
		// If no label text set we use the predefined value.

		/**
		 * @todo The value send should subtract the border size
		 * and readd it after calculation to get the proper result.
		 */
		result = get_best_text_size(result);
	}

	DBG_GUI_L << LOG_HEADER
			<< " label '" << debug_truncate(label_)
			<< "' result " << result
			<< ".\n";
	return result;
}

void tcontrol::place(const tpoint& origin, const tpoint& size)
{
	// resize canvasses
	BOOST_FOREACH(tcanvas& canvas, canvas_) {
		canvas.set_width(size.x);
		canvas.set_height(size.y);
	}

	// Note we assume that the best size has been queried but otherwise it
	// should return false.
	if(renderer_.is_truncated()
			&& use_tooltip_on_label_overflow_ && tooltip_.empty()) {

		 set_tooltip(label_);
	}

	// inherited
	twidget::place(origin, size);

	// update the state of the canvas after the sizes have been set.
	update_canvas();
}

void tcontrol::load_config()
{
	if(!config()) {
		set_config(get_control(get_control_type(), definition()));

		assert(canvas().size() == config()->state.size());
		for(size_t i = 0; i < canvas().size(); ++i) {
			canvas(i) = config()->state[i].canvas;
		}

		update_canvas();

		load_config_extra();
	}
}

void tcontrol::set_definition(const std::string& definition)
{
	assert(!config());
	twidget::set_definition(definition);
	load_config();
	assert(config());
}

void tcontrol::set_label(const t_string& label)
{
	if(label == label_) {
		return;
	}

	label_ = label;
	set_layout_size(tpoint(0, 0));
	update_canvas();
	set_dirty();
}

void tcontrol::set_use_markup(bool use_markup)
{
	if(use_markup == use_markup_) {
		return;
	}

	use_markup_ = use_markup;
	update_canvas();
	set_dirty();
}

void tcontrol::update_canvas()
{
	const int max_width = get_text_maximum_width();
	const int max_height = get_text_maximum_height();

	// set label in canvases
	BOOST_FOREACH(tcanvas& canvas, canvas_) {
		canvas.set_variable("text", variant(label_));
		canvas.set_variable("text_markup", variant(use_markup_));
		canvas.set_variable("text_maximum_width", variant(max_width));
		canvas.set_variable("text_maximum_height", variant(max_height));
		canvas.set_variable("text_wrap_mode", variant(can_wrap()
			? PANGO_ELLIPSIZE_NONE : PANGO_ELLIPSIZE_END));
	}
}

int tcontrol::get_text_maximum_width() const
{
	assert(config_);

	return text_maximum_width_ != 0
		? text_maximum_width_
		: get_width() - config_->text_extra_width;
}

int tcontrol::get_text_maximum_height() const
{
	assert(config_);

	return get_height() - config_->text_extra_height;
}

void tcontrol::impl_draw_background(surface& frame_buffer)
{
	DBG_GUI_D << LOG_HEADER
			<< " label '" << debug_truncate(label_)
			<< " size " << get_rect()
			<< ".\n";

	canvas(get_state()).blit(frame_buffer, get_rect());
}

tpoint tcontrol::get_best_text_size(const tpoint& minimum_size, const tpoint& maximum_size) const
{
	log_scope2(log_gui_layout, LOG_SCOPE_HEADER);

	assert(!label_.empty());

	const tpoint border(config_->text_extra_width, config_->text_extra_height);
	tpoint size = minimum_size - border;

	renderer_.set_text(label_, use_markup_);

	renderer_.set_font_size(config_->text_font_size);
	renderer_.set_font_style(config_->text_font_style);

	// Try with the minimum wanted size.
	const int maximum_width =  text_maximum_width_ != 0
		? text_maximum_width_
		: maximum_size.x;

	renderer_.set_maximum_width(maximum_width);

	if(can_wrap()) {
		renderer_.set_ellipse_mode(PANGO_ELLIPSIZE_NONE);
	}

	DBG_GUI_L << LOG_HEADER
			<< " label '" << debug_truncate(label_)
			<< "' status: "
			<< " minimum_size " << minimum_size
			<< " maximum_size " << maximum_size
			<< " text_maximum_width_ " << text_maximum_width_
			<< " can_wrap " << can_wrap()
			<< " truncated " << renderer_.is_truncated()
			<< " renderer size " << renderer_.get_size()
			<< ".\n";

	// If doesn't fit try the maximum.
	if(renderer_.is_truncated() && !can_wrap()) {
		// FIXME if maximum size is defined we should look at that
		// but also we don't adjust for the extra text space yet!!!
		const tpoint maximum_size(config_->max_width, config_->max_height);
		renderer_.set_maximum_width(maximum_size.x
				? maximum_size.x - border.x
				: -1);
	}

	size = renderer_.get_size() + border;

	if(size.x < minimum_size.x) {
		size.x = minimum_size.x;
	}

	if(size.y < minimum_size.y) {
		size.y = minimum_size.y;
	}

	DBG_GUI_L << LOG_HEADER
			<< " label '" << debug_truncate(label_)
			<< "' result " << size
			<< ".\n";
	return size;
}

} // namespace gui2

