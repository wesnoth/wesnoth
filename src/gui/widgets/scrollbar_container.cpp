/*
   Copyright (C) 2008 - 2014 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/widgets/scrollbar_container_private.hpp"

#include "gui/auxiliary/find_widget.tpp"
#include "gui/auxiliary/log.hpp"
#include "gui/auxiliary/layout_exception.hpp"
#include "gui/widgets/clickable.hpp"
#include "gui/widgets/spacer.hpp"
#include "gui/widgets/window.hpp"
#include "utils/foreach.tpp"

#include <boost/bind.hpp>

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

namespace
{

static const std::string button_up_names[]
		= { "_begin", "_line_up", "_half_page_up", "_page_up" };

static const std::string button_down_names[]
		= { "_end", "_line_down", "_half_page_down", "_page_down" };

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

} // namespace

tscrollbar_container::tscrollbar_container(const unsigned canvas_count)
	: tcontainer_(canvas_count)
	, state_(ENABLED)
	, vertical_scrollbar_mode_(auto_visible_first_run)
	, horizontal_scrollbar_mode_(auto_visible_first_run)
	, vertical_scrollbar_grid_(NULL)
	, horizontal_scrollbar_grid_(NULL)
	, vertical_scrollbar_(NULL)
	, horizontal_scrollbar_(NULL)
	, content_grid_(NULL)
	, content_(NULL)
	, content_visible_area_()
{
	connect_signal<event::SDL_KEY_DOWN>(
			boost::bind(&tscrollbar_container::signal_handler_sdl_key_down,
						this,
						_2,
						_3,
						_5,
						_6));


	connect_signal<event::SDL_WHEEL_UP>(
			boost::bind(&tscrollbar_container::signal_handler_sdl_wheel_up,
						this,
						_2,
						_3),
			event::tdispatcher::back_post_child);

	connect_signal<event::SDL_WHEEL_DOWN>(
			boost::bind(&tscrollbar_container::signal_handler_sdl_wheel_down,
						this,
						_2,
						_3),
			event::tdispatcher::back_post_child);

	connect_signal<event::SDL_WHEEL_LEFT>(
			boost::bind(&tscrollbar_container::signal_handler_sdl_wheel_left,
						this,
						_2,
						_3),
			event::tdispatcher::back_post_child);

	connect_signal<event::SDL_WHEEL_RIGHT>(
			boost::bind(&tscrollbar_container::signal_handler_sdl_wheel_right,
						this,
						_2,
						_3),
			event::tdispatcher::back_post_child);
}

void tscrollbar_container::layout_initialise(const bool full_initialisation)
{
	// Inherited.
	tcontainer_::layout_initialise(full_initialisation);

	if(full_initialisation) {

		assert(vertical_scrollbar_grid_);
		switch(vertical_scrollbar_mode_) {
			case always_visible:
				vertical_scrollbar_grid_->set_visible(
						twidget::tvisible::visible);
				break;

			case auto_visible:
				vertical_scrollbar_grid_->set_visible(
						twidget::tvisible::hidden);
				break;

			default:
				vertical_scrollbar_grid_->set_visible(
						twidget::tvisible::invisible);
		}

		assert(horizontal_scrollbar_grid_);
		switch(horizontal_scrollbar_mode_) {
			case always_visible:
				horizontal_scrollbar_grid_->set_visible(
						twidget::tvisible::visible);
				break;

			case auto_visible:
				horizontal_scrollbar_grid_->set_visible(
						twidget::tvisible::hidden);
				break;

			default:
				horizontal_scrollbar_grid_->set_visible(
						twidget::tvisible::invisible);
		}
	}

	assert(content_grid_);
	content_grid_->layout_initialise(full_initialisation);
}

void tscrollbar_container::request_reduce_height(const unsigned maximum_height)
{
	DBG_GUI_L << LOG_HEADER << " requested height " << maximum_height << ".\n";
	/*
	 * First ask the content to reduce it's height. This seems to work for now,
	 * but maybe some sizing hints will be required later.
	 */
	/** @todo Evaluate whether sizing hints are required. */
	assert(content_grid_);
	const unsigned offset
			= horizontal_scrollbar_grid_
					  && horizontal_scrollbar_grid_->get_visible()
						 != twidget::tvisible::invisible
					  ? horizontal_scrollbar_grid_->get_best_size().y
					  : 0;

	content_grid_->request_reduce_height(maximum_height - offset);

	// Did we manage to achieve the wanted size?
	tpoint size = get_best_size();
	if(static_cast<unsigned>(size.y) <= maximum_height) {
		DBG_GUI_L << LOG_HEADER << " child honored request, height " << size.y
				  << ".\n";
		return;
	}

	if(vertical_scrollbar_mode_ == always_invisible) {
		DBG_GUI_L << LOG_HEADER << " request failed due to scrollbar mode.\n";
		return;
	}

	assert(vertical_scrollbar_grid_);
	const bool resized = vertical_scrollbar_grid_->get_visible()
						 == twidget::tvisible::invisible;

	// Always set the bar visible, is a nop is already visible.
	vertical_scrollbar_grid_->set_visible(twidget::tvisible::visible);

	const tpoint scrollbar_size = vertical_scrollbar_grid_->get_best_size();

	// If showing the scrollbar increased the height, hide and abort.
	if(resized && scrollbar_size.y > size.y) {
		vertical_scrollbar_grid_->set_visible(twidget::tvisible::invisible);
		DBG_GUI_L << LOG_HEADER << " request failed, showing the scrollbar"
				  << " increased the height to " << scrollbar_size.y << ".\n";
		return;
	}

	if(maximum_height > static_cast<unsigned>(scrollbar_size.y)) {
		size.y = maximum_height;
	} else {
		size.y = scrollbar_size.y;
	}

	// FIXME adjust for the step size of the scrollbar

	set_layout_size(size);
	DBG_GUI_L << LOG_HEADER << " resize resulted in " << size.y << ".\n";

	if(resized) {
		DBG_GUI_L << LOG_HEADER
				  << " resize modified the width, throw notification.\n";

		throw tlayout_exception_width_modified();
	}
}

void tscrollbar_container::request_reduce_width(const unsigned maximum_width)
{
	DBG_GUI_L << LOG_HEADER << " requested width " << maximum_width << ".\n";

	// First ask our content, it might be able to wrap which looks better as
	// a scrollbar.
	assert(content_grid_);
	const unsigned offset
			= vertical_scrollbar_grid_
					  && vertical_scrollbar_grid_->get_visible()
						 != twidget::tvisible::invisible
					  ? vertical_scrollbar_grid_->get_best_size().x
					  : 0;

	content_grid_->request_reduce_width(maximum_width - offset);

	// Did we manage to achieve the wanted size?
	tpoint size = get_best_size();
	if(static_cast<unsigned>(size.x) <= maximum_width) {
		DBG_GUI_L << LOG_HEADER << " child honored request, width " << size.x
				  << ".\n";
		return;
	}

	if(horizontal_scrollbar_mode_ == always_invisible) {
		DBG_GUI_L << LOG_HEADER << " request failed due to scrollbar mode.\n";
		return;
	}

	// Always set the bar visible, is a nop when it's already visible.
	assert(horizontal_scrollbar_grid_);
	horizontal_scrollbar_grid_->set_visible(twidget::tvisible::visible);
	size = get_best_size();

	const tpoint scrollbar_size = horizontal_scrollbar_grid_->get_best_size();

	// If showing the scrollbar increased the width, hide and abort.
	if(horizontal_scrollbar_mode_ == auto_visible_first_run && scrollbar_size.x
															   > size.x) {

		horizontal_scrollbar_grid_->set_visible(twidget::tvisible::invisible);
		DBG_GUI_L << LOG_HEADER << " request failed, showing the scrollbar"
				  << " increased the width to " << scrollbar_size.x << ".\n";
		return;
	}

	if(maximum_width > static_cast<unsigned>(scrollbar_size.x)) {
		size.x = maximum_width;
	} else {
		size.x = scrollbar_size.x;
	}

	// FIXME adjust for the step size of the scrollbar

	set_layout_size(size);
	DBG_GUI_L << LOG_HEADER << " resize resulted in " << size.x << ".\n";
}

bool tscrollbar_container::can_wrap() const
{
	return content_grid_ ? content_grid_->can_wrap() : false;
}

tpoint tscrollbar_container::calculate_best_size() const
{
	log_scope2(log_gui_layout, LOG_SCOPE_HEADER);

	/***** get vertical scrollbar size *****/
	const tpoint vertical_scrollbar
			= vertical_scrollbar_grid_->get_visible()
					  == twidget::tvisible::invisible
					  ? tpoint(0, 0)
					  : vertical_scrollbar_grid_->get_best_size();

	/***** get horizontal scrollbar size *****/
	const tpoint horizontal_scrollbar
			= horizontal_scrollbar_grid_->get_visible()
					  == twidget::tvisible::invisible
					  ? tpoint(0, 0)
					  : horizontal_scrollbar_grid_->get_best_size();

	/***** get content size *****/
	assert(content_grid_);
	const tpoint content = content_grid_->get_best_size();

	const tpoint result(
			vertical_scrollbar.x + std::max(horizontal_scrollbar.x, content.x),
			horizontal_scrollbar.y + std::max(vertical_scrollbar.y, content.y));

	DBG_GUI_L << LOG_HEADER << " vertical_scrollbar " << vertical_scrollbar
			  << " horizontal_scrollbar " << horizontal_scrollbar << " content "
			  << content << " result " << result << ".\n";

	return result;
}

static void
set_scrollbar_mode(tgrid* scrollbar_grid,
				   tscrollbar_* scrollbar,
				   tscrollbar_container::tscrollbar_mode& scrollbar_mode,
				   const unsigned items,
				   const unsigned visible_items)
{
	assert(scrollbar_grid && scrollbar);

	if(scrollbar_mode == tscrollbar_container::always_invisible) {
		scrollbar_grid->set_visible(twidget::tvisible::invisible);
		return;
	}

	scrollbar->set_item_count(items);
	scrollbar->set_item_position(0);
	scrollbar->set_visible_items(visible_items);

	if(scrollbar_mode == tscrollbar_container::auto_visible) {

		const bool scrollbar_needed = items > visible_items;

		scrollbar_grid->set_visible(scrollbar_needed
											? twidget::tvisible::visible
											: twidget::tvisible::hidden);
	}
}

void tscrollbar_container::place(const tpoint& origin, const tpoint& size)
{
	// Inherited.
	tcontainer_::place(origin, size);

	// Set content size
	assert(content_ && content_grid_);

	const tpoint content_origin = content_->get_origin();

	const tpoint best_size = content_grid_->get_best_size();
	const tpoint content_size(content_->get_width(), content_->get_height());

	const tpoint content_grid_size(std::max(best_size.x, content_size.x),
								   std::max(best_size.y, content_size.y));

	set_content_size(content_origin, content_grid_size);

	// Set vertical scrollbar
	set_scrollbar_mode(vertical_scrollbar_grid_,
					   vertical_scrollbar_,
					   vertical_scrollbar_mode_,
					   content_grid_->get_height(),
					   content_->get_height());

	// Set horizontal scrollbar
	set_scrollbar_mode(horizontal_scrollbar_grid_,
					   horizontal_scrollbar_,
					   horizontal_scrollbar_mode_,
					   content_grid_->get_width(),
					   content_->get_width());

	// Update the buttons.
	set_scrollbar_button_status();

	// Now set the visible part of the content.
	content_visible_area_ = content_->get_rectangle();
	content_grid_->set_visible_rectangle(content_visible_area_);
}

void tscrollbar_container::set_origin(const tpoint& origin)
{
	// Inherited.
	tcontainer_::set_origin(origin);

	// Set content size
	assert(content_ && content_grid_);

	const tpoint content_origin = content_->get_origin();

	content_grid_->set_origin(content_origin);

	// Changing the origin also invalidates the visible area.
	content_grid_->set_visible_rectangle(content_visible_area_);
}

void tscrollbar_container::set_visible_rectangle(const SDL_Rect& rectangle)
{
	// Inherited.
	tcontainer_::set_visible_rectangle(rectangle);

	// Now get the visible part of the content.
	content_visible_area_
			= intersect_rects(rectangle, content_->get_rectangle());

	content_grid_->set_visible_rectangle(content_visible_area_);
}

bool tscrollbar_container::get_active() const
{
	return state_ != DISABLED;
}

unsigned tscrollbar_container::get_state() const
{
	return state_;
}

twidget* tscrollbar_container::find_at(const tpoint& coordinate,
									   const bool must_be_active)
{
	return tscrollbar_container_implementation::find_at<twidget>(
			*this, coordinate, must_be_active);
}

const twidget* tscrollbar_container::find_at(const tpoint& coordinate,
											 const bool must_be_active) const
{
	return tscrollbar_container_implementation::find_at<const twidget>(
			*this, coordinate, must_be_active);
}

twidget* tscrollbar_container::find(const std::string& id,
									const bool must_be_active)
{
	return tscrollbar_container_implementation::find<twidget>(
			*this, id, must_be_active);
}

const twidget* tscrollbar_container::find(const std::string& id,
										  const bool must_be_active) const
{
	return tscrollbar_container_implementation::find<const twidget>(
			*this, id, must_be_active);
}

bool tscrollbar_container::disable_click_dismiss() const
{
	assert(content_grid_);
	return tcontainer_::disable_click_dismiss()
		   || content_grid_->disable_click_dismiss();
}

bool tscrollbar_container::content_resize_request(const bool force_sizing)
{
	/**
	 * @todo Try to handle auto_visible_first_run here as well.
	 *
	 * Handling it here makes the code a bit more complex but allows to not
	 * reserve space for scrollbars, which will look nicer in the MP lobby.
	 * But the extra complexity is no 1.8 material.
	 */

	assert(content_ && content_grid_);

	tpoint best_size = content_grid_->recalculate_best_size();
	tpoint size = content_->get_size();

	DBG_GUI_L << LOG_HEADER << " wanted size " << best_size
			  << " available size " << size << ".\n";

	if(size == tpoint(0, 0)) {
		DBG_GUI_L << LOG_HEADER << " initial setup not done, bailing out.\n";
		return false;
	}

	if(best_size.x <= size.x && best_size.y <= size.y) {
		const tpoint content_size = content_grid_->get_size();
		if(content_size.x > size.x || content_size.y > size.y) {
			DBG_GUI_L << LOG_HEADER << " will fit, only needs a resize.\n";
			goto resize;
		}
		if(force_sizing) {
			DBG_GUI_L << LOG_HEADER << " fits, but resize forced.\n";
			goto resize;
		}
		DBG_GUI_L << LOG_HEADER << " fits, nothing to do.\n";
		return true;
	}

	if(best_size.x > size.x) {
		DBG_GUI_L << LOG_HEADER << " content too wide.\n";
		if(horizontal_scrollbar_mode_ == always_invisible
		   || (horizontal_scrollbar_mode_ == auto_visible_first_run
			   && horizontal_scrollbar_grid_->get_visible()
				  == twidget::tvisible::invisible)) {

			DBG_GUI_L << LOG_HEADER
					  << " can't use horizontal scrollbar, ask window.\n";
			twindow* window = get_window();
			assert(window);
			window->invalidate_layout();
			return false;
		}
	}

	if(best_size.y > size.y) {
		DBG_GUI_L << LOG_HEADER << " content too high.\n";
		if(vertical_scrollbar_mode_ == always_invisible
		   || (vertical_scrollbar_mode_ == auto_visible_first_run
			   && vertical_scrollbar_grid_->get_visible()
				  == twidget::tvisible::invisible)) {

			DBG_GUI_L << LOG_HEADER
					  << " can't use vertical scrollbar, ask window.\n";
			twindow* window = get_window();
			assert(window);
			window->invalidate_layout();
			return false;
		}
	}

resize:
	DBG_GUI_L << LOG_HEADER << " handle resizing.\n";
	place(get_origin(), get_size());
	return true;
}

bool tscrollbar_container::content_resize_request(const int width_modification,
												  const int height_modification)
{
	DBG_GUI_L << LOG_HEADER << " wanted width modification "
			  << width_modification << " wanted height modification "
			  << height_modification << ".\n";

	if(get_size() == tpoint(0, 0)) {
		DBG_GUI_L << LOG_HEADER << " initial setup not done, bailing out.\n";
		return false;
	}

	twindow* window = get_window();
	assert(window);
	if(window->get_need_layout()) {
		DBG_GUI_L << LOG_HEADER
				  << " window already needs a layout phase, bailing out.\n";
		return false;
	}

	assert(content_ && content_grid_);

	const bool result = content_resize_width(width_modification)
						&& content_resize_height(height_modification);

	if(result) {
		/*
		 * The subroutines set the new size of the scrollbar but don't
		 * update the button status.
		 */
		set_scrollbar_button_status();
	}

	DBG_GUI_L << LOG_HEADER << " result " << result << ".\n";
	return result;
}

bool tscrollbar_container::content_resize_width(const int width_modification)
{
	if(width_modification == 0) {
		return true;
	}

	const int new_width = content_grid_->get_width() + width_modification;
	DBG_GUI_L << LOG_HEADER << " current width " << content_grid_->get_width()
			  << " wanted width " << new_width;

	assert(new_width > 0);

	if(static_cast<unsigned>(new_width) <= content_->get_width()) {
		DBG_GUI_L << " width fits in container, test height.\n";
		set_scrollbar_mode(horizontal_scrollbar_grid_,
						   horizontal_scrollbar_,
						   horizontal_scrollbar_mode_,
						   new_width,
						   content_->get_width());
		return true;
	}

	assert(horizontal_scrollbar_ && horizontal_scrollbar_grid_);
	if(horizontal_scrollbar_mode_ == always_invisible
	   || (horizontal_scrollbar_mode_ == auto_visible_first_run
		   && horizontal_scrollbar_grid_->get_visible()
			  == twidget::tvisible::invisible)) {

		DBG_GUI_L << " can't use horizontal scrollbar, ask window.\n";
		twindow* window = get_window();
		assert(window);
		window->invalidate_layout();
		return false;
	}

	DBG_GUI_L << " use the horizontal scrollbar, test height.\n";
	set_scrollbar_mode(horizontal_scrollbar_grid_,
					   horizontal_scrollbar_,
					   horizontal_scrollbar_mode_,
					   new_width,
					   content_->get_width());

	return true;
}

bool tscrollbar_container::content_resize_height(const int height_modification)
{
	if(height_modification == 0) {
		return true;
	}

	const int new_height = content_grid_->get_height() + height_modification;

	DBG_GUI_L << LOG_HEADER << " current height " << content_grid_->get_height()
			  << " wanted height " << new_height;

	assert(new_height > 0);

	if(static_cast<unsigned>(new_height) <= content_->get_height()) {
		DBG_GUI_L << " height in container, resize allowed.\n";
		set_scrollbar_mode(vertical_scrollbar_grid_,
						   vertical_scrollbar_,
						   vertical_scrollbar_mode_,
						   new_height,
						   content_->get_height());
		return true;
	}

	assert(vertical_scrollbar_ && vertical_scrollbar_grid_);
	if(vertical_scrollbar_mode_ == always_invisible
	   || (vertical_scrollbar_mode_ == auto_visible_first_run
		   && vertical_scrollbar_grid_->get_visible()
			  == twidget::tvisible::invisible)) {

		DBG_GUI_L << " can't use vertical scrollbar, ask window.\n";
		twindow* window = get_window();
		assert(window);
		window->invalidate_layout();
		return false;
	}

	DBG_GUI_L << " use the vertical scrollbar, resize allowed.\n";
	set_scrollbar_mode(vertical_scrollbar_grid_,
					   vertical_scrollbar_,
					   vertical_scrollbar_mode_,
					   new_height,
					   content_->get_height());

	return true;
}

void tscrollbar_container::finalize_setup()
{
	/***** Setup vertical scrollbar *****/

	vertical_scrollbar_grid_
			= find_widget<tgrid>(this, "_vertical_scrollbar_grid", false, true);

	vertical_scrollbar_ = find_widget<tscrollbar_>(
			vertical_scrollbar_grid_, "_vertical_scrollbar", false, true);

	connect_signal_notify_modified(
			*vertical_scrollbar_,
			boost::bind(&tscrollbar_container::vertical_scrollbar_moved, this));

	/***** Setup horizontal scrollbar *****/
	horizontal_scrollbar_grid_ = find_widget<tgrid>(
			this, "_horizontal_scrollbar_grid", false, true);

	horizontal_scrollbar_ = find_widget<tscrollbar_>(
			horizontal_scrollbar_grid_, "_horizontal_scrollbar", false, true);

	connect_signal_notify_modified(
			*horizontal_scrollbar_,
			boost::bind(&tscrollbar_container::horizontal_scrollbar_moved,
						this));

	/***** Setup the scrollbar buttons *****/
	FOREACH(const AUTO & item, scroll_lookup())
	{

		// Vertical.
		tclickable_* button = find_widget<tclickable_>(
				vertical_scrollbar_grid_, item.first, false, false);

		if(button) {
			button->connect_click_handler(boost::bind(
					&tscrollbar_container::scroll_vertical_scrollbar,
					this,
					item.second));
		}

		// Horizontal.
		button = find_widget<tclickable_>(
				horizontal_scrollbar_grid_, item.first, false, false);

		if(button) {
			button->connect_click_handler(boost::bind(
					&tscrollbar_container::scroll_horizontal_scrollbar,
					this,
					item.second));
		}
	}

	/***** Setup the content *****/
	content_ = new tspacer();
	content_->set_definition("default");

	content_grid_ = dynamic_cast<tgrid*>(
			grid().swap_child("_content_grid", content_, true));
	assert(content_grid_);

	content_grid_->set_parent(this);

	/***** Let our subclasses initialize themselves. *****/
	finalize_subclass();
}

void tscrollbar_container::set_vertical_scrollbar_mode(
		const tscrollbar_mode scrollbar_mode)
{
	if(vertical_scrollbar_mode_ != scrollbar_mode) {
		vertical_scrollbar_mode_ = scrollbar_mode;
	}
}

void tscrollbar_container::set_horizontal_scrollbar_mode(
		const tscrollbar_mode scrollbar_mode)
{
	if(horizontal_scrollbar_mode_ != scrollbar_mode) {
		horizontal_scrollbar_mode_ = scrollbar_mode;
	}
}

void tscrollbar_container::impl_draw_children(surface& frame_buffer)
{
	assert(get_visible() == twidget::tvisible::visible
		   && content_grid_->get_visible() == twidget::tvisible::visible);

	// Inherited.
	tcontainer_::impl_draw_children(frame_buffer);

	content_grid_->draw_children(frame_buffer);
}

void tscrollbar_container::impl_draw_children(surface& frame_buffer,
											  int x_offset,
											  int y_offset)
{
	assert(get_visible() == twidget::tvisible::visible
		   && content_grid_->get_visible() == twidget::tvisible::visible);

	// Inherited.
	tcontainer_::impl_draw_children(frame_buffer, x_offset, y_offset);

	content_grid_->draw_children(frame_buffer, x_offset, y_offset);
}

void tscrollbar_container::layout_children()
{
	// Inherited.
	tcontainer_::layout_children();

	assert(content_grid_);
	content_grid_->layout_children();
}

void tscrollbar_container::child_populate_dirty_list(
		twindow& caller, const std::vector<twidget*>& call_stack)
{
	// Inherited.
	tcontainer_::child_populate_dirty_list(caller, call_stack);

	assert(content_grid_);
	std::vector<twidget*> child_call_stack(call_stack);
	content_grid_->populate_dirty_list(caller, child_call_stack);
}

void tscrollbar_container::set_content_size(const tpoint& origin,
											const tpoint& size)
{
	content_grid_->place(origin, size);
}

void tscrollbar_container::show_content_rect(const SDL_Rect& rect)
{
	assert(content_);
	assert(horizontal_scrollbar_ && vertical_scrollbar_);

	// Set the bottom right location first if it doesn't fit the top left
	// will look good. First calculate the left and top position depending on
	// the current position.

	const int left_position = horizontal_scrollbar_->get_item_position()
							  + (rect.x - content_->get_x());
	const int top_position = vertical_scrollbar_->get_item_position()
							 + (rect.y - content_->get_y());

	// bottom.
	const int wanted_bottom = rect.y + rect.h;
	const int current_bottom = content_->get_y() + content_->get_height();
	int distance = wanted_bottom - current_bottom;
	if(distance > 0) {
		vertical_scrollbar_->set_item_position(
				vertical_scrollbar_->get_item_position() + distance);
	}

	// right.
	const int wanted_right = rect.x + rect.w;
	const int current_right = content_->get_x() + content_->get_width();
	distance = wanted_right - current_right;
	if(distance > 0) {
		horizontal_scrollbar_->set_item_position(
				horizontal_scrollbar_->get_item_position() + distance);
	}

	// top.
	if(top_position
	   < static_cast<int>(vertical_scrollbar_->get_item_position())) {

		vertical_scrollbar_->set_item_position(top_position);
	}

	// left.
	if(left_position
	   < static_cast<int>(horizontal_scrollbar_->get_item_position())) {

		horizontal_scrollbar_->set_item_position(left_position);
	}

	// Update.
	scrollbar_moved();
}

void tscrollbar_container::set_scrollbar_button_status()
{
	if(true) { /** @todo scrollbar visibility. */
		/***** set scroll up button status *****/
		FOREACH(const AUTO & name, button_up_names)
		{
			tcontrol* button = find_widget<tcontrol>(
					vertical_scrollbar_grid_, name, false, false);

			if(button) {
				button->set_active(!vertical_scrollbar_->at_begin());
			}
		}

		/***** set scroll down status *****/
		FOREACH(const AUTO & name, button_down_names)
		{
			tcontrol* button = find_widget<tcontrol>(
					vertical_scrollbar_grid_, name, false, false);

			if(button) {
				button->set_active(!vertical_scrollbar_->at_end());
			}
		}

		/***** Set the status if the scrollbars *****/
		vertical_scrollbar_->set_active(
				!vertical_scrollbar_->all_items_visible());
	}

	if(true) { /** @todo scrollbar visibility. */
		/***** Set scroll left button status *****/
		FOREACH(const AUTO & name, button_up_names)
		{
			tcontrol* button = find_widget<tcontrol>(
					horizontal_scrollbar_grid_, name, false, false);

			if(button) {
				button->set_active(!horizontal_scrollbar_->at_begin());
			}
		}

		/***** Set scroll right button status *****/
		FOREACH(const AUTO & name, button_down_names)
		{
			tcontrol* button = find_widget<tcontrol>(
					horizontal_scrollbar_grid_, name, false, false);

			if(button) {
				button->set_active(!horizontal_scrollbar_->at_end());
			}
		}

		/***** Set the status if the scrollbars *****/
		horizontal_scrollbar_->set_active(
				!horizontal_scrollbar_->all_items_visible());
	}
}

void tscrollbar_container::scroll_vertical_scrollbar(
		const tscrollbar_::tscroll scroll)
{
	assert(vertical_scrollbar_);

	vertical_scrollbar_->scroll(scroll);
	scrollbar_moved();
}

void tscrollbar_container::scroll_horizontal_scrollbar(
		const tscrollbar_::tscroll scroll)
{
	assert(horizontal_scrollbar_);

	horizontal_scrollbar_->scroll(scroll);
	scrollbar_moved();
}

void tscrollbar_container::handle_key_home(SDLMod /*modifier*/, bool& handled)
{
	assert(vertical_scrollbar_ && horizontal_scrollbar_);

	vertical_scrollbar_->scroll(tscrollbar_::BEGIN);
	horizontal_scrollbar_->scroll(tscrollbar_::BEGIN);
	scrollbar_moved();

	handled = true;
}

void tscrollbar_container::handle_key_end(SDLMod /*modifier*/, bool& handled)
{
	assert(vertical_scrollbar_);

	vertical_scrollbar_->scroll(tscrollbar_::END);
	scrollbar_moved();

	handled = true;
}

void tscrollbar_container::handle_key_page_up(SDLMod /*modifier*/,
											  bool& handled)
{
	assert(vertical_scrollbar_);

	vertical_scrollbar_->scroll(tscrollbar_::JUMP_BACKWARDS);
	scrollbar_moved();

	handled = true;
}

void tscrollbar_container::handle_key_page_down(SDLMod /*modifier*/,
												bool& handled)

{
	assert(vertical_scrollbar_);

	vertical_scrollbar_->scroll(tscrollbar_::JUMP_FORWARD);
	scrollbar_moved();

	handled = true;
}

void tscrollbar_container::handle_key_up_arrow(SDLMod /*modifier*/,
											   bool& handled)
{
	assert(vertical_scrollbar_);

	vertical_scrollbar_->scroll(tscrollbar_::ITEM_BACKWARDS);
	scrollbar_moved();

	handled = true;
}

void tscrollbar_container::handle_key_down_arrow(SDLMod /*modifier*/,
												 bool& handled)
{
	assert(vertical_scrollbar_);

	vertical_scrollbar_->scroll(tscrollbar_::ITEM_FORWARD);
	scrollbar_moved();

	handled = true;
}

void tscrollbar_container::handle_key_left_arrow(SDLMod /*modifier*/,
												 bool& handled)
{
	assert(horizontal_scrollbar_);

	horizontal_scrollbar_->scroll(tscrollbar_::ITEM_BACKWARDS);
	scrollbar_moved();

	handled = true;
}

void tscrollbar_container::handle_key_right_arrow(SDLMod /*modifier*/,
												  bool& handled)
{
	assert(horizontal_scrollbar_);

	horizontal_scrollbar_->scroll(tscrollbar_::ITEM_FORWARD);
	scrollbar_moved();

	handled = true;
}

void tscrollbar_container::scrollbar_moved()
{
	// Init.
	assert(content_ && content_grid_);
	assert(vertical_scrollbar_ && horizontal_scrollbar_);

	/*** Update the content location. ***/
	const int x_offset = horizontal_scrollbar_mode_ == always_invisible
								 ? 0
								 : horizontal_scrollbar_->get_item_position()
								   * horizontal_scrollbar_->get_step_size();

	const int y_offset = vertical_scrollbar_mode_ == always_invisible
								 ? 0
								 : vertical_scrollbar_->get_item_position()
								   * vertical_scrollbar_->get_step_size();

	const tpoint content_origin = tpoint(content_->get_x() - x_offset,
										 content_->get_y() - y_offset);

	content_grid_->set_origin(content_origin);
	content_grid_->set_visible_rectangle(content_visible_area_);
	content_grid_->set_is_dirty(true);

	// Update scrollbar.
	set_scrollbar_button_status();
}

const std::string& tscrollbar_container::get_control_type() const
{
	static const std::string type = "scrollbar_container";
	return type;
}

void
tscrollbar_container::signal_handler_sdl_key_down(const event::tevent event,
												  bool& handled,
												  const SDLKey key,
												  SDLMod modifier)
{
	DBG_GUI_E << LOG_HEADER << event << ".\n";

	switch(key) {
		case SDLK_HOME:
			handle_key_home(modifier, handled);
			break;

		case SDLK_END:
			handle_key_end(modifier, handled);
			break;


		case SDLK_PAGEUP:
			handle_key_page_up(modifier, handled);
			break;

		case SDLK_PAGEDOWN:
			handle_key_page_down(modifier, handled);
			break;


		case SDLK_UP:
			handle_key_up_arrow(modifier, handled);
			break;

		case SDLK_DOWN:
			handle_key_down_arrow(modifier, handled);
			break;

		case SDLK_LEFT:
			handle_key_left_arrow(modifier, handled);
			break;

		case SDLK_RIGHT:
			handle_key_right_arrow(modifier, handled);
			break;
		default:
			/* ignore */
			break;
	}
}

void
tscrollbar_container::signal_handler_sdl_wheel_up(const event::tevent event,
												  bool& handled)
{
	DBG_GUI_E << LOG_HEADER << event << ".\n";

	assert(vertical_scrollbar_grid_ && vertical_scrollbar_);

	if(vertical_scrollbar_grid_->get_visible() == twidget::tvisible::visible) {
		vertical_scrollbar_->scroll(tscrollbar_::HALF_JUMP_BACKWARDS);
		scrollbar_moved();
		handled = true;
	}
}

void
tscrollbar_container::signal_handler_sdl_wheel_down(const event::tevent event,
													bool& handled)
{
	DBG_GUI_E << LOG_HEADER << event << ".\n";

	assert(vertical_scrollbar_grid_ && vertical_scrollbar_);

	if(vertical_scrollbar_grid_->get_visible() == twidget::tvisible::visible) {
		vertical_scrollbar_->scroll(tscrollbar_::HALF_JUMP_FORWARD);
		scrollbar_moved();
		handled = true;
	}
}

void
tscrollbar_container::signal_handler_sdl_wheel_left(const event::tevent event,
													bool& handled)
{
	DBG_GUI_E << LOG_HEADER << event << ".\n";

	assert(horizontal_scrollbar_grid_ && horizontal_scrollbar_);

	if(horizontal_scrollbar_grid_->get_visible()
	   == twidget::tvisible::visible) {
		horizontal_scrollbar_->scroll(tscrollbar_::HALF_JUMP_BACKWARDS);
		scrollbar_moved();
		handled = true;
	}
}

void
tscrollbar_container::signal_handler_sdl_wheel_right(const event::tevent event,
													 bool& handled)
{
	DBG_GUI_E << LOG_HEADER << event << ".\n";

	assert(horizontal_scrollbar_grid_ && horizontal_scrollbar_);

	if(horizontal_scrollbar_grid_->get_visible()
	   == twidget::tvisible::visible) {
		horizontal_scrollbar_->scroll(tscrollbar_::HALF_JUMP_FORWARD);
		scrollbar_moved();
		handled = true;
	}
}

} // namespace gui2
