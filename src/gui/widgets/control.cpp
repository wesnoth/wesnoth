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

#include "control.hpp"

#include "foreach.hpp"
#include "gui/widgets/window.hpp"

namespace gui2 {

tcontrol::tcontrol(const unsigned canvas_count)
	: visible_(true)
	, label_()
	, use_tooltip_on_label_overflow_(true)
	, tooltip_()
	, help_message_()
	, canvas_(canvas_count)
	, restorer_()
	, config_(0)
	, renderer_()
	, text_maximum_width_(0)
	, shrunken_(false)
{
}

void tcontrol::set_members(const string_map& data)
{
	string_map::const_iterator itor = data.find("label");
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

void tcontrol::set_block_easy_close(const bool block)
{
	twindow* window = get_window();
	if(!window) {
		/*
		 * This can happen in a listbox when the row data is manipulated before
		 * the listbox is finalized. In that case that widget should do set the
		 * state in its finalizer.
		 */
		DBG_GUI << "tcontrol(" + get_control_type() + ") " + __func__ + ": "
			"No window set, this might be a bug.\n";
		return;
	}

	if(block) {
		if(id().empty()) {
			set_id(get_uid());
		}
		window->add_easy_close_blocker(id());
	} else if(!id().empty()) {
		// It might never have been enabled so the id might be empty.
		window->remove_easy_close_blocker(id());
	}
}

tpoint tcontrol::get_config_minimum_size() const
{
	assert(config_);

	tpoint result(config_->min_width, config_->min_height);

	DBG_G_L << "tcontrol(" + get_control_type() + ") " + __func__ + ":"
		<< " result " << result
		<< ".\n";
	return result;
}

tpoint tcontrol::get_config_default_size() const
{
	assert(config_);

	tpoint result(config_->default_width, config_->default_height);

	DBG_G_L << "tcontrol(" + get_control_type() + ") " + __func__ + ":"
		<< " result " << result
		<< ".\n";
	return result;
}

tpoint tcontrol::get_config_maximum_size() const
{
	assert(config_);

	tpoint result(config_->max_width, config_->max_height);

	DBG_G_L << "tcontrol(" + get_control_type() + ") " + __func__ + ":"
		<< " result " << result
		<< ".\n";
	return result;
}

void tcontrol::layout_init()
{ 
	twidget::layout_init();
	shrunken_ = false; 
}

void tcontrol::layout_wrap(const unsigned maximum_width)
{
	// Inherited.
	twidget::layout_wrap(maximum_width);

	assert(config_);

	if(label_.empty()) {
		// FIXME see what to do on an empty label later.
		return;
	} else {

		tpoint size = get_best_text_size(
				tpoint(0,0), 
				tpoint(maximum_width - config_->text_extra_width, 0));

		size.x += config_->text_extra_width;
		size.y += config_->text_extra_height;

		set_layout_size(size);

		DBG_G_L << "tcontrol(" + get_control_type() + ") " + __func__ + ":"
			<< " result " << size
			<< ".\n";

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

	DBG_G_L << "tcontrol(" + get_control_type() + ") " + __func__ + ":"
		<< " empty label " << label_.empty()
		<< " result " << result
		<< ".\n";
	return result;
}

void tcontrol::set_size(const tpoint& origin, const tpoint& size)
{
	// resize canvasses
	foreach(tcanvas& canvas, canvas_) {
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
	twidget::set_size(origin, size);

	// update the state of the canvas after the sizes have been set.
	update_canvas();
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

		update_canvas();

		load_config_extra();
	}
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

void tcontrol::set_visible(const bool visible)
{ 
	if(visible_ != visible) { 
		visible_ = visible;
		set_block_easy_close(visible_ && does_block_easy_close());
		set_dirty();
	} 
}

void tcontrol::set_label(const t_string& label)
{
	if(label == label_) {
		return;
	}

	label_ = label;
	update_canvas();
	set_dirty();
}

bool tcontrol::needs_full_redraw() const
{
	assert(config());
	return config()->state[get_state()].full_redraw;
}

void tcontrol::update_canvas()
{
	const int max_width = get_text_maximum_width();
	const int max_height = get_text_maximum_height();

	// set label in canvases
	foreach(tcanvas& canvas, canvas_) {
		canvas.set_variable("text", variant(label_));
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

void tcontrol::save_background(const surface& src)
{
	assert(!restorer_);

	restorer_ = gui2::save_background(src, get_rect());
}

void tcontrol::restore_background(surface& dst)
{
	gui2::restore_background(restorer_, dst, get_rect());
}

tpoint tcontrol::get_best_text_size(const tpoint& minimum_size, const tpoint& maximum_size) const
{
	log_scope2(gui_layout, "tcontrol(" + get_control_type() + ") " + __func__);

	assert(!label_.empty());

	const tpoint border(config_->text_extra_width, config_->text_extra_height);
	tpoint size = minimum_size - border;

	renderer_.set_text(label_, false);
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

	DBG_G_L << "tcontrol(" + get_control_type() + ") status:\n";
	DBG_G_L << "minimum_size " << minimum_size
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
		renderer_.set_maximum_width(maximum_size.x ? maximum_size.x - border.x : -1);
	}

	size = renderer_.get_size() + border;

	if(size.x < minimum_size.x) {
		size.x = minimum_size.x;
	}

	if(size.y < minimum_size.y) {
		size.y = minimum_size.y;
	}
	
	DBG_G_L << "tcontrol(" + get_control_type() + ") result " << size << ".\n";
	return size;
}

} // namespace gui2

