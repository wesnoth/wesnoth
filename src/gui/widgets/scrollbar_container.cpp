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

#ifdef NEW_DRAW

#include "gui/widgets/scrollbar_container.hpp"

#include "foreach.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/scrollbar.hpp"
#include "gui/widgets/spacer.hpp"
#include "gui/widgets/window.hpp"

namespace gui2 {

namespace {

static const std::string button_up_names[] = {
	"_begin", "_line_up", "_half_page_up", "_page_up" };

static const std::string button_down_names[] = {
	"_end", "_line_down", "_half_page_down", "_page_down" };

/**
 * Returns a map with the names of all buttons and the scrollbar jump they're
 * supposed to execute.
 */
const std::map<std::string, tscrollbar_::tscroll>& scroll_lookup()
{
	static std::map<std::string, tscrollbar_::tscroll> lookup;
	if(lookup.empty()) {
		lookup["_begin"] = tscrollbar_::BEGIN;
		lookup["_line_up"] = tscrollbar_::ITEM_BACKWARDS;
		lookup["_half_page_up"] = tscrollbar_::HALF_JUMP_BACKWARDS;
		lookup["_page_up"] = tscrollbar_::JUMP_BACKWARDS;

		lookup["_end"] = tscrollbar_::END;
		lookup["_line_down"] = tscrollbar_::ITEM_FORWARD;
		lookup["_half_page_down"] = tscrollbar_::HALF_JUMP_FORWARD;
		lookup["_page_down"] = tscrollbar_::JUMP_FORWARD;
	}

	return lookup;
}

void callback_vertical_scrollbar_button(twidget* caller)
{
	gui2::get_parent<gui2::tscrollbar_container>
		(caller)->vertical_scrollbar_click(caller);
}	

void callback_horizontal_scrollbar_button(twidget* caller)
{
	gui2::get_parent<gui2::tscrollbar_container>
		(caller)->horizontal_scrollbar_click(caller);
}	

void callback_vertical_scrollbar(twidget* caller)
{
	gui2::get_parent<gui2::tscrollbar_container>
		(caller)->vertical_scrollbar_moved(caller);
}

void callback_horizontal_scrollbar(twidget* caller)
{
	gui2::get_parent<gui2::tscrollbar_container>
		(caller)->horizontal_scrollbar_moved(caller);
}

} // namespace

tscrollbar_container::tscrollbar_container(const unsigned canvas_count)
	: tcontainer_(canvas_count)
	, state_(ENABLED)
	, vertical_scrollbar_mode_(SHOW_WHEN_NEEDED)
	, horizontal_scrollbar_mode_(SHOW_WHEN_NEEDED)
	, vertical_scrollbar_grid_(NULL)
	, horizontal_scrollbar_grid_(NULL)
	, vertical_scrollbar_(NULL)
	, horizontal_scrollbar_(NULL)
	, content_grid_(NULL)
	, content_(NULL)
{
}

void tscrollbar_container::layout_init()
{
	// Inherited.
	tcontainer_::layout_init();

	assert(content_grid_);
	content_grid_->layout_init();
}

void tscrollbar_container::layout_wrap(const unsigned maximum_width)
{
	// Inherited.
	twidget::layout_wrap(maximum_width);

	assert(content_grid_ && vertical_scrollbar_grid_);
	const unsigned offset = vertical_scrollbar_mode_ == HIDE
			? 0
			: vertical_scrollbar_grid_->get_best_size().x;

	content_grid_->layout_wrap(maximum_width - offset);
}

void tscrollbar_container::
		layout_use_vertical_scrollbar(const unsigned maximum_height)
{
	// Inherited.
	twidget::layout_use_vertical_scrollbar(maximum_height);
	
	tpoint size = get_best_size();

	size.y = maximum_height;

	// FIXME adjust for the step size of the scrollbar

	set_layout_size(size);
}

void tscrollbar_container::
		layout_use_horizontal_scrollbar(const unsigned maximum_width)
{
	// Inherited.
	twidget::layout_use_horizontal_scrollbar(maximum_width);
	
	tpoint size = get_best_size();

	size.x = maximum_width;

	set_layout_size(size);
}

tpoint tscrollbar_container::calculate_best_size() const
{
	log_scope2(gui_layout, 
		std::string("tscrollbar_container ") + __func__);

	/***** get vertical scrollbar size *****/
	const tpoint vertical_scrollbar = 
			vertical_scrollbar_mode_ == HIDE 
			? tpoint(0, 0)
			: vertical_scrollbar_grid_->get_best_size();

	/***** get horizontal scrollbar size *****/
	const tpoint horizontal_scrollbar = 
			horizontal_scrollbar_mode_ == HIDE 
			? tpoint(0, 0)
			: horizontal_scrollbar_grid_->get_best_size();

	/***** get content size *****/
	assert(content_grid_);
	const tpoint content = content_grid_->get_best_size();

	const tpoint result(
			vertical_scrollbar.x + 
				std::max(horizontal_scrollbar.x, content.x),
			horizontal_scrollbar.y + 
				std::max(vertical_scrollbar.y,  content.y));

	DBG_G_L << "tscrollbar_container" 
		<< " vertical_scrollbar " << vertical_scrollbar
		<< " horizontal_scrollbar " << horizontal_scrollbar
		<< " content " << content
		<< " result " << result
		<< ".\n";

	return result;
}

void tscrollbar_container::
		set_size(const tpoint& origin, const tpoint& size)
{
	// Inherited.
	tcontainer_::set_size(origin, size);

	// Set content size
	assert(content_ && content_grid_);

	const tpoint content_origin = tpoint(
			content_->get_screen_x(),
			content_->get_screen_y());

	const tpoint best_size = content_grid_->get_best_size();
	const tpoint content_size(content_->get_width(), content_->get_height());

	const tpoint content_grid_size(
			std::max(best_size.x, content_size.x),
			std::max(best_size.y, content_size.y));

	/*
	 * For set_size to work properly, we need to disable the parent
	 * temporary. Without a parent the screen coordinates won't be
	 * remapped, which is wanted in this case. For event handling the
	 * parent is needed. 
	 */
	twidget* parent = content_grid_->parent();
	content_grid_->set_parent(NULL);
	content_grid_->set_size(content_origin, content_grid_size);
	content_grid_->set_parent(parent);


	// Set vertical scrollbar
	assert(vertical_scrollbar_);
	if(vertical_scrollbar_mode_ != HIDE) {
		vertical_scrollbar_->set_item_count(content_grid_size.y);
		vertical_scrollbar_->set_visible_items(content_->get_height());
	}

	// Set horizontal scrollbar
	assert(horizontal_scrollbar_);
	if(horizontal_scrollbar_mode_ != HIDE) {
		horizontal_scrollbar_->set_item_count(content_grid_size.x);
		horizontal_scrollbar_->set_visible_items(content_->get_width());
	}
}

void tscrollbar_container:: set_origin(const tpoint& origin)
{
	// Inherited.
	tcontainer_::set_origin(origin);

	// Set content size
	assert(content_ && content_grid_);

	const tpoint content_origin = tpoint(
			content_->get_screen_x(),
			content_->get_screen_y());
	
	content_grid_->set_origin(content_origin);
}

void tscrollbar_container::draw_background(surface& frame_buffer)
{
	// Inherited.
	tcontainer_::draw_background(frame_buffer);

	/***** **** Draw content ***** *****/
	assert(content_ && content_grid_);

	// Update the location depending on the scrollbars.
 	if(vertical_scrollbar_mode_ != HIDE 
			|| horizontal_scrollbar_mode_ != HIDE) {

		assert(vertical_scrollbar_ && horizontal_scrollbar_);
		const int x_offset = horizontal_scrollbar_mode_ == HIDE
				? 0
				: horizontal_scrollbar_->get_item_position() *
				  horizontal_scrollbar_->get_step_size();

		const int y_offset = vertical_scrollbar_mode_ == HIDE
				? 0
				: vertical_scrollbar_->get_item_position() *
				  vertical_scrollbar_->get_step_size();


		const tpoint content_size = content_grid_->get_best_size();

		const tpoint content_origin = tpoint(
				content_->get_screen_x() - x_offset,
				content_->get_screen_y() - y_offset);

		content_grid_->set_origin(content_origin);
	}

	// Make sure the content can't draw outside its canvas.
	clip_rect_setter clip_rect(frame_buffer, ::create_rect(
		content_->get_screen_x(),
		content_->get_screen_y(),
		content_->get_width(),
		content_->get_height()));

	// Draw.
	content_grid_->draw_children(frame_buffer);
}

void tscrollbar_container::draw_foreground(surface& frame_buffer)
{
	// Make sure the content can't draw outside its canvas.
	clip_rect_setter clip_rect(frame_buffer, ::create_rect(
		content_->get_screen_x(),
		content_->get_screen_y(),
		content_->get_width(),
		content_->get_height()));

	// Inherited.
	tcontainer_::draw_foreground(frame_buffer);
}
	
twidget* tscrollbar_container::find_widget(
		const tpoint& coordinate, const bool must_be_active)
{
	assert(content_);

	twidget* result = tcontainer_::find_widget(coordinate, must_be_active);

	if(result != content_) {
		return result;
	}

	return content_->find_widget(coordinate, must_be_active);
}

const twidget* tscrollbar_container::find_widget(const tpoint& coordinate, 
		const bool must_be_active) const
{
	assert(content_);

	const twidget* result = 
			tcontainer_::find_widget(coordinate, must_be_active);

	if(result != content_) {
		return result;
	}

	return content_->find_widget(coordinate, must_be_active);
}

void tscrollbar_container::vertical_scrollbar_click(twidget* caller)
{
	/** @todo Hack to capture the keyboard focus. */
	get_window()->keyboard_capture(this);

	const std::map<std::string, tscrollbar_::tscroll>::const_iterator 
		itor = scroll_lookup().find(caller->id());

	assert(itor != scroll_lookup().end());
	vertical_scrollbar_->scroll(itor->second);

	set_scrollbar_button_status();
	set_dirty();
}

void tscrollbar_container::horizontal_scrollbar_click(twidget* caller)
{
	/** @todo Hack to capture the keyboard focus. */
	get_window()->keyboard_capture(this);

	const std::map<std::string, tscrollbar_::tscroll>::const_iterator 
		itor = scroll_lookup().find(caller->id());

	assert(itor != scroll_lookup().end());
	horizontal_scrollbar_->scroll(itor->second);

	set_scrollbar_button_status();
	set_dirty();
}

void tscrollbar_container::finalize_setup()
{
	/***** Setup vertical scrollbar *****/

	vertical_scrollbar_grid_ = 
		find_widget<tgrid>("_vertical_scrollbar_grid", false, true);

	vertical_scrollbar_ = vertical_scrollbar_grid_->
		find_widget<tscrollbar_>("_vertical_scrollbar", false, true);

	vertical_scrollbar_->
		set_callback_positioner_move(callback_vertical_scrollbar);

	/***** Setup horizontal scrollbar *****/
	horizontal_scrollbar_grid_ = 
		find_widget<tgrid>("_horizontal_scrollbar_grid", false, true);

	horizontal_scrollbar_ = horizontal_scrollbar_grid_->
		find_widget<tscrollbar_>("_horizontal_scrollbar", false, true);

	horizontal_scrollbar_->
		set_callback_positioner_move(callback_horizontal_scrollbar);

	/***** Setup the scrollbar buttons *****/
	typedef std::pair<std::string, tscrollbar_::tscroll> hack;
	foreach(const hack& item, scroll_lookup()) {

		// Vertical.
		tbutton* button = vertical_scrollbar_grid_->
				find_widget<tbutton>(item.first, false, false);

		if(button) {
			button->set_callback_mouse_left_click(
					callback_vertical_scrollbar_button);
		}

		// Horizontal.
		button = horizontal_scrollbar_grid_->
				find_widget<tbutton>(item.first, false, false);

		if(button) {
			button->set_callback_mouse_left_click(
					callback_horizontal_scrollbar_button);
		}
	}

	/***** Setup the content *****/
	content_ = new tspacer();
	content_->set_definition("default");

	content_grid_ = dynamic_cast<tgrid*>(
			grid().swap_child("_content_grid", content_, true));
	assert(content_grid_);

	content_grid_->set_parent(this);

	/***** Set the easy close status. *****/
	/** @todo needs more testing. */
	set_block_easy_close(get_visible() 
			&& get_active() && does_block_easy_close());

	/***** Let our subclasses initialize themselves. *****/
	finalize_subclass();
}

void tscrollbar_container::
		set_vertical_scrollbar_mode(const tscrollbar_mode scrollbar_mode)
{ 
	if(vertical_scrollbar_mode_ != scrollbar_mode) {
		vertical_scrollbar_mode_ = scrollbar_mode;
		show_vertical_scrollbar(vertical_scrollbar_mode_ != HIDE);
	}
}

void tscrollbar_container::
		set_horizontal_scrollbar_mode(const tscrollbar_mode scrollbar_mode)
{ 
	if(horizontal_scrollbar_mode_ != scrollbar_mode) {
		horizontal_scrollbar_mode_ = scrollbar_mode;
		show_horizontal_scrollbar(horizontal_scrollbar_mode_ != HIDE);
	}
}

void tscrollbar_container::show_vertical_scrollbar(const bool /*show*/)
{
	/** @todo implement the new visibility. */
//	vertical_scrollbar_grid_->set_visible(show);
}

void tscrollbar_container::show_horizontal_scrollbar(const bool /*show*/)
{
	/** @todo implement the new visibility. */
//	horizontal_scrollbar_grid_->set_visible(show);
}

void tscrollbar_container::set_scrollbar_button_status()
{
	if(true) { /** @todo scrollbar visibility. */
		/***** set scroll up button status *****/
		foreach(const std::string& name, button_up_names) {
			tbutton* button = vertical_scrollbar_grid_->
					find_widget<tbutton>(name, false, false);

			if(button) {
				button->set_active(!vertical_scrollbar_->at_begin());
			}
		}

		/***** set scroll down status *****/
		foreach(const std::string& name, button_down_names) {
			tbutton* button = vertical_scrollbar_grid_->
					find_widget<tbutton>(name, false, false);

			if(button) {
				button->set_active(vertical_scrollbar_->at_end());
			}
		}

		/***** Set the status if the scrollbars *****/
		vertical_scrollbar_->set_active( !(vertical_scrollbar_->at_begin() 
				&& vertical_scrollbar_->at_end()));
	}

	if(true) { /** @todo scrollbar visibility. */
		/***** Set scroll left button status *****/
		foreach(const std::string& name, button_up_names) {
			tbutton* button = horizontal_scrollbar_grid_->
					find_widget<tbutton>(name, false, false);

			if(button) {
				button->set_active(!horizontal_scrollbar_->at_begin());
			}
		}

		/***** Set scroll right button status *****/
		foreach(const std::string& name, button_down_names) {
			tbutton* button = horizontal_scrollbar_grid_->
					find_widget<tbutton>(name, false, false);

			if(button) {
				button->set_active(horizontal_scrollbar_->at_end());
			}
		}

		/***** Set the status if the scrollbars *****/
		horizontal_scrollbar_->set_active( !(horizontal_scrollbar_->at_begin() 
				&& horizontal_scrollbar_->at_end()));
	}
}

} // namespace gui2

#endif


