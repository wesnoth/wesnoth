/*
   Copyright (C) 2008 - 2017 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/auxiliary/find_widget.hpp"
#include "gui/core/log.hpp"
#include "gui/core/layout_exception.hpp"
#include "gui/widgets/clickable_item.hpp"
#include "gui/widgets/spacer.hpp"
#include "gui/widgets/window.hpp"
#include "sdl/rect.hpp"

#include "utils/functional.hpp"

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
const std::map<std::string, scrollbar_base::scroll_mode>& scroll_lookup()
{
	static std::map<std::string, scrollbar_base::scroll_mode> lookup;
	if(lookup.empty()) {
		lookup["_begin"] = scrollbar_base::BEGIN;
		lookup["_line_up"] = scrollbar_base::ITEM_BACKWARDS;
		lookup["_half_page_up"] = scrollbar_base::HALF_JUMP_BACKWARDS;
		lookup["_page_up"] = scrollbar_base::JUMP_BACKWARDS;

		lookup["_end"] = scrollbar_base::END;
		lookup["_line_down"] = scrollbar_base::ITEM_FORWARD;
		lookup["_half_page_down"] = scrollbar_base::HALF_JUMP_FORWARD;
		lookup["_page_down"] = scrollbar_base::JUMP_FORWARD;
	}

	return lookup;
}

} // namespace

scrollbar_container::scrollbar_container(const unsigned canvas_count)
	: container_base(canvas_count)
	, state_(ENABLED)
	, vertical_scrollbar_mode_(AUTO_VISIBLE_FIRST_RUN)
	, horizontal_scrollbar_mode_(AUTO_VISIBLE_FIRST_RUN)
	, vertical_scrollbar_grid_(nullptr)
	, horizontal_scrollbar_grid_(nullptr)
	, vertical_scrollbar_(nullptr)
	, horizontal_scrollbar_(nullptr)
	, content_grid_(nullptr)
	, content_(nullptr)
	, content_visible_area_()
{
	connect_signal<event::SDL_KEY_DOWN>(
			std::bind(&scrollbar_container::signal_handler_sdl_key_down,
						this,
						_2,
						_3,
						_5,
						_6));


	connect_signal<event::SDL_WHEEL_UP>(
			std::bind(&scrollbar_container::signal_handler_sdl_wheel_up,
						this,
						_2,
						_3),
			event::dispatcher::back_post_child);

	connect_signal<event::SDL_WHEEL_DOWN>(
			std::bind(&scrollbar_container::signal_handler_sdl_wheel_down,
						this,
						_2,
						_3),
			event::dispatcher::back_post_child);

	connect_signal<event::SDL_WHEEL_LEFT>(
			std::bind(&scrollbar_container::signal_handler_sdl_wheel_left,
						this,
						_2,
						_3),
			event::dispatcher::back_post_child);

	connect_signal<event::SDL_WHEEL_RIGHT>(
			std::bind(&scrollbar_container::signal_handler_sdl_wheel_right,
						this,
						_2,
						_3),
			event::dispatcher::back_post_child);
}

void scrollbar_container::layout_initialise(const bool full_initialisation)
{
	// Inherited.
	container_base::layout_initialise(full_initialisation);

	if(full_initialisation) {

		assert(vertical_scrollbar_grid_);
		switch(vertical_scrollbar_mode_) {
			case ALWAYS_VISIBLE:
				vertical_scrollbar_grid_->set_visible(
						widget::visibility::visible);
				break;

			case AUTO_VISIBLE:
				vertical_scrollbar_grid_->set_visible(
						widget::visibility::hidden);
				break;

			default:
				vertical_scrollbar_grid_->set_visible(
						widget::visibility::invisible);
		}

		assert(horizontal_scrollbar_grid_);
		switch(horizontal_scrollbar_mode_) {
			case ALWAYS_VISIBLE:
				horizontal_scrollbar_grid_->set_visible(
						widget::visibility::visible);
				break;

			case AUTO_VISIBLE:
				horizontal_scrollbar_grid_->set_visible(
						widget::visibility::hidden);
				break;

			default:
				horizontal_scrollbar_grid_->set_visible(
						widget::visibility::invisible);
		}
	}

	assert(content_grid_);
	content_grid_->layout_initialise(full_initialisation);
}

void scrollbar_container::request_reduce_height(const unsigned maximum_height)
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
						 != widget::visibility::invisible
					  ? horizontal_scrollbar_grid_->get_best_size().y
					  : 0;

	content_grid_->request_reduce_height(maximum_height - offset);

	// Did we manage to achieve the wanted size?
	point size = get_best_size();
	if(static_cast<unsigned>(size.y) <= maximum_height) {
		DBG_GUI_L << LOG_HEADER << " child honored request, height " << size.y
				  << ".\n";
		return;
	}

	if(vertical_scrollbar_mode_ == ALWAYS_INVISIBLE) {
		DBG_GUI_L << LOG_HEADER << " request failed due to scrollbar mode.\n";
		return;
	}

	assert(vertical_scrollbar_grid_);
	const bool resized = vertical_scrollbar_grid_->get_visible()
						 == widget::visibility::invisible;

	// Always set the bar visible, is a nop is already visible.
	vertical_scrollbar_grid_->set_visible(widget::visibility::visible);

	const point scrollbar_size = vertical_scrollbar_grid_->get_best_size();

	// If showing the scrollbar increased the height, hide and abort.
	if(resized && scrollbar_size.y > size.y) {
		vertical_scrollbar_grid_->set_visible(widget::visibility::invisible);
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

		throw layout_exception_width_modified();
	}
}

void scrollbar_container::request_reduce_width(const unsigned maximum_width)
{
	DBG_GUI_L << LOG_HEADER << " requested width " << maximum_width << ".\n";

	// First ask our content, it might be able to wrap which looks better as
	// a scrollbar.
	assert(content_grid_);
	const unsigned offset
			= vertical_scrollbar_grid_
					  && vertical_scrollbar_grid_->get_visible()
						 != widget::visibility::invisible
					  ? vertical_scrollbar_grid_->get_best_size().x
					  : 0;

	content_grid_->request_reduce_width(maximum_width - offset);

	// Did we manage to achieve the wanted size?
	point size = get_best_size();
	if(static_cast<unsigned>(size.x) <= maximum_width) {
		DBG_GUI_L << LOG_HEADER << " child honored request, width " << size.x
				  << ".\n";
		return;
	}

	if(horizontal_scrollbar_mode_ == ALWAYS_INVISIBLE) {
		DBG_GUI_L << LOG_HEADER << " request failed due to scrollbar mode.\n";
		return;
	}

	// Always set the bar visible, is a nop when it's already visible.
	assert(horizontal_scrollbar_grid_);
	horizontal_scrollbar_grid_->set_visible(widget::visibility::visible);
	size = get_best_size();

	point scrollbar_size = horizontal_scrollbar_grid_->get_best_size();

	/*
	 * If the vertical bar is not invisible it's size needs to be added to the
	 * minimum size.
	 */
	if(vertical_scrollbar_grid_->get_visible()
	   != widget::visibility::invisible) {

		scrollbar_size.x += vertical_scrollbar_grid_->get_best_size().x;
	}

	// If showing the scrollbar increased the width, hide and abort.
	if(horizontal_scrollbar_mode_ == AUTO_VISIBLE_FIRST_RUN && scrollbar_size.x
															   > size.x) {

		horizontal_scrollbar_grid_->set_visible(widget::visibility::invisible);
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

bool scrollbar_container::can_wrap() const
{
	return content_grid_ ? content_grid_->can_wrap() : false;
}

point scrollbar_container::calculate_best_size() const
{
	log_scope2(log_gui_layout, LOG_SCOPE_HEADER);

	/***** get vertical scrollbar size *****/
	const point vertical_scrollbar
			= vertical_scrollbar_grid_->get_visible()
					  == widget::visibility::invisible
					  ? point()
					  : vertical_scrollbar_grid_->get_best_size();

	/***** get horizontal scrollbar size *****/
	const point horizontal_scrollbar
			= horizontal_scrollbar_grid_->get_visible()
					  == widget::visibility::invisible
					  ? point()
					  : horizontal_scrollbar_grid_->get_best_size();

	/***** get content size *****/
	assert(content_grid_);
	const point content = content_grid_->get_best_size();

	point result(
			vertical_scrollbar.x + std::max(horizontal_scrollbar.x, content.x),
			horizontal_scrollbar.y + std::max(vertical_scrollbar.y, content.y));

	//
	// Workaround for bug #24780. This should probably be moved somewhere more specific to
	// the listbox, but for now it suffices.
	//
	if(const grid* header = find_widget<const grid>(&get_grid(), "_header_grid", false, false)) {
		result.y += header->get_best_size().y;
	}

	if(const grid* footer = find_widget<const grid>(&get_grid(), "_footer_grid", false, false)) {
		result.y += footer->get_best_size().y;
	}

	DBG_GUI_L << LOG_HEADER << " vertical_scrollbar " << vertical_scrollbar
			  << " horizontal_scrollbar " << horizontal_scrollbar << " content "
			  << content << " result " << result << ".\n";

	return result;
}

static void
set_scrollbar_mode(grid* scrollbar_grid,
				   scrollbar_base* scrollbar,
				   scrollbar_container::scrollbar_mode& scrollbar_mode,
				   const unsigned items,
				   const unsigned visible_items)
{
	assert(scrollbar_grid && scrollbar);

	if(scrollbar_mode == scrollbar_container::ALWAYS_INVISIBLE) {
		scrollbar_grid->set_visible(widget::visibility::invisible);
		return;
	}

	scrollbar->set_item_count(items);
	scrollbar->set_item_position(0);
	scrollbar->set_visible_items(visible_items);

	if(scrollbar_mode == scrollbar_container::AUTO_VISIBLE) {

		const bool scrollbar_needed = items > visible_items;

		scrollbar_grid->set_visible(scrollbar_needed
											? widget::visibility::visible
											: widget::visibility::hidden);
	}
}
static bool is_inserted_before(unsigned insertion_pos, unsigned old_item_count, unsigned old_position, unsigned visible_items)
{
	if(old_position == 0)
		return false;
	else if(old_position + visible_items >= old_item_count)
		return true;
	else if(insertion_pos <= old_position)
		return true;
	else
		return false;
}

static void
adjust_scrollbar_mode(grid* scrollbar_grid,
				   scrollbar_base* scrollbar,
				   scrollbar_container::scrollbar_mode& scrollbar_mode,
				   const unsigned items_before,
				   const unsigned items_after,
				   const int insertion_pos,
				   const unsigned visible_items)
{
	assert(scrollbar_grid && scrollbar);
	if(items_before != scrollbar->get_item_count()) {
		return set_scrollbar_mode(scrollbar_grid, scrollbar, scrollbar_mode, items_after, visible_items);
	}
	//TODO: does this also work well in case the items were removed?
	const unsigned previous_item_position = scrollbar->get_item_position();
	//Casts insertion_pos to an unsigned so negative values are interpreted as 'at end'
	const bool inserted_before_visible_area = is_inserted_before(static_cast<unsigned>(insertion_pos), items_before, previous_item_position, visible_items);

	if(scrollbar_mode == scrollbar_container::ALWAYS_INVISIBLE) {
		scrollbar_grid->set_visible(widget::visibility::invisible);
		return;
	}

	scrollbar->set_item_count(items_after);
	scrollbar->set_item_position(inserted_before_visible_area ? previous_item_position + items_after - items_before : previous_item_position);
	//scrollbar->set_item_position(0);
	scrollbar->set_visible_items(visible_items);

	if(scrollbar_mode == scrollbar_container::AUTO_VISIBLE) {

		const bool scrollbar_needed = items_after > visible_items;

		scrollbar_grid->set_visible(scrollbar_needed
											? widget::visibility::visible
											: widget::visibility::hidden);
	}
}

void scrollbar_container::place(const point& origin, const point& size)
{
	// Inherited.
	container_base::place(origin, size);

	// Set content size
	assert(content_ && content_grid_);

	const point content_origin = content_->get_origin();

	const point best_size = content_grid_->get_best_size();
	const point content_size(content_->get_width(), content_->get_height());

	const point content_grid_size(std::max(best_size.x, content_size.x),
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

void scrollbar_container::set_origin(const point& origin)
{
	// Inherited.
	container_base::set_origin(origin);

	// Set content size
	assert(content_ && content_grid_);

	const point content_origin = content_->get_origin();

	content_grid_->set_origin(content_origin);

	// Changing the origin also invalidates the visible area.
	content_grid_->set_visible_rectangle(content_visible_area_);
}

void scrollbar_container::set_visible_rectangle(const SDL_Rect& rectangle)
{
	// Inherited.
	container_base::set_visible_rectangle(rectangle);

	// Now get the visible part of the content.
	content_visible_area_
			= sdl::intersect_rects(rectangle, content_->get_rectangle());

	content_grid_->set_visible_rectangle(content_visible_area_);
}

bool scrollbar_container::get_active() const
{
	return state_ != DISABLED;
}

unsigned scrollbar_container::get_state() const
{
	return state_;
}

widget* scrollbar_container::find_at(const point& coordinate,
									   const bool must_be_active)
{
	return scrollbar_container_implementation::find_at<widget>(
			*this, coordinate, must_be_active);
}

const widget* scrollbar_container::find_at(const point& coordinate,
											 const bool must_be_active) const
{
	return scrollbar_container_implementation::find_at<const widget>(
			*this, coordinate, must_be_active);
}

widget* scrollbar_container::find(const std::string& id,
									const bool must_be_active)
{
	return scrollbar_container_implementation::find<widget>(
			*this, id, must_be_active);
}

const widget* scrollbar_container::find(const std::string& id,
										  const bool must_be_active) const
{
	return scrollbar_container_implementation::find<const widget>(
			*this, id, must_be_active);
}

bool scrollbar_container::disable_click_dismiss() const
{
	assert(content_grid_);
	return container_base::disable_click_dismiss()
		   || content_grid_->disable_click_dismiss();
}

bool scrollbar_container::content_resize_request(const bool force_sizing)
{
	/**
	 * @todo Try to handle AUTO_VISIBLE_FIRST_RUN here as well.
	 *
	 * Handling it here makes the code a bit more complex but allows to not
	 * reserve space for scrollbars, which will look nicer in the MP lobby.
	 * But the extra complexity is no 1.8 material.
	 */

	assert(content_ && content_grid_);

	point best_size = content_grid_->recalculate_best_size();
	point size = content_->get_size();

	DBG_GUI_L << LOG_HEADER << " wanted size " << best_size
			  << " available size " << size << ".\n";

	if(size == point()) {
		DBG_GUI_L << LOG_HEADER << " initial setup not done, bailing out.\n";
		return false;
	}

	if(best_size.x <= size.x && best_size.y <= size.y) {
		const point content_size = content_grid_->get_size();
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
		if(horizontal_scrollbar_mode_ == ALWAYS_INVISIBLE
		   || (horizontal_scrollbar_mode_ == AUTO_VISIBLE_FIRST_RUN
			   && horizontal_scrollbar_grid_->get_visible()
				  == widget::visibility::invisible)) {

			DBG_GUI_L << LOG_HEADER
					  << " can't use horizontal scrollbar, ask grid.\n";
			layout_initialise(true);
			grid* grid = get_parent_grid();
			assert(grid);
			grid->relayout();
			return false;
		}
	}

	if(best_size.y > size.y) {
		DBG_GUI_L << LOG_HEADER << " content too high.\n";
		if(vertical_scrollbar_mode_ == ALWAYS_INVISIBLE
		   || (vertical_scrollbar_mode_ == AUTO_VISIBLE_FIRST_RUN
			   && vertical_scrollbar_grid_->get_visible()
				  == widget::visibility::invisible)) {

			DBG_GUI_L << LOG_HEADER
					  << " can't use vertical scrollbar, ask grid.\n";
			layout_initialise(true);
			grid* grid = get_parent_grid();
			assert(grid);
			grid->relayout();
			return false;
		}
	}

resize:
	DBG_GUI_L << LOG_HEADER << " handle resizing.\n";
	place(get_origin(), get_size());
	return true;
}

bool scrollbar_container::content_resize_request(const int width_modification,
												  const int height_modification,
												  const int width_modification_pos,
												  const int height_modification_pos)
{
	DBG_GUI_L << LOG_HEADER << " wanted width modification "
			  << width_modification << " wanted height modification "
			  << height_modification << ".\n";

	if(get_size() == point()) {
		DBG_GUI_L << LOG_HEADER << " initial setup not done, bailing out.\n";
		return false;
	}

	window* window = get_window();
	assert(window);
	if(window->get_need_layout()) {
		DBG_GUI_L << LOG_HEADER
				  << " window already needs a layout phase, bailing out.\n";
		return false;
	}

	assert(content_ && content_grid_);

	const bool result = content_resize_width(width_modification, width_modification_pos)
						&& content_resize_height(height_modification, height_modification_pos);
	scrollbar_moved();
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

bool scrollbar_container::content_resize_width(const int width_modification, const int width_modification_pos)
{
	if(width_modification == 0) {
		return true;
	}

	const int new_width = content_grid_->get_width() + width_modification;
	DBG_GUI_L << LOG_HEADER << " current width " << content_grid_->get_width()
			  << " wanted width " << new_width;

	if(new_width < 0) {
		return false;
	}

	if(static_cast<unsigned>(new_width) <= content_->get_width()) {
		DBG_GUI_L << " width fits in container, test height.\n";
		adjust_scrollbar_mode(horizontal_scrollbar_grid_,
						   horizontal_scrollbar_,
						   horizontal_scrollbar_mode_,
						   content_grid_->get_width(),
						   content_grid_->get_width() + width_modification,
						   width_modification_pos,
						   content_->get_width());
		return true;
	}

	assert(horizontal_scrollbar_ && horizontal_scrollbar_grid_);
	if(horizontal_scrollbar_mode_ == ALWAYS_INVISIBLE
	   || (horizontal_scrollbar_mode_ == AUTO_VISIBLE_FIRST_RUN
		   && horizontal_scrollbar_grid_->get_visible()
			  == widget::visibility::invisible)) {

		DBG_GUI_L << " can't use horizontal scrollbar, ask window.\n";
		window* window = get_window();
		assert(window);
		window->invalidate_layout();
		return false;
	}

	DBG_GUI_L << " use the horizontal scrollbar, test height.\n";
	adjust_scrollbar_mode(horizontal_scrollbar_grid_,
					   horizontal_scrollbar_,
					   horizontal_scrollbar_mode_,
					   content_grid_->get_width(),
					   content_grid_->get_width() + width_modification,
					   width_modification_pos,
					   content_->get_width());

	return true;
}

bool scrollbar_container::content_resize_height(const int height_modification, const int height_modification_pos)
{
	if(height_modification == 0) {
		return true;
	}

	const int new_height = content_grid_->get_height() + height_modification;

	DBG_GUI_L << LOG_HEADER << " current height " << content_grid_->get_height()
			  << " wanted height " << new_height;

	if(new_height < 0) {
		return false;
	}

	if(static_cast<unsigned>(new_height) <= content_->get_height()) {
		DBG_GUI_L << " height in container, resize allowed.\n";
		adjust_scrollbar_mode(vertical_scrollbar_grid_,
						   vertical_scrollbar_,
						   vertical_scrollbar_mode_,
						   content_grid_->get_height(),
						   new_height,
						   height_modification_pos,
						   content_->get_height());
		return true;
	}

	assert(vertical_scrollbar_ && vertical_scrollbar_grid_);
	if(vertical_scrollbar_mode_ == ALWAYS_INVISIBLE
	   || (vertical_scrollbar_mode_ == AUTO_VISIBLE_FIRST_RUN
		   && vertical_scrollbar_grid_->get_visible()
			  == widget::visibility::invisible)) {

		DBG_GUI_L << " can't use vertical scrollbar, ask window.\n";
		window* window = get_window();
		assert(window);
		window->invalidate_layout();
		return false;
	}

	DBG_GUI_L << " use the vertical scrollbar, resize allowed.\n";
	adjust_scrollbar_mode(vertical_scrollbar_grid_,
					   vertical_scrollbar_,
					   vertical_scrollbar_mode_,
					   content_grid_->get_height(),
					   new_height,
					   height_modification_pos,
					   content_->get_height());

	return true;
}

void scrollbar_container::finalize_setup()
{
	/***** Setup vertical scrollbar *****/

	vertical_scrollbar_grid_
			= find_widget<grid>(this, "_vertical_scrollbar_grid", false, true);

	vertical_scrollbar_ = find_widget<scrollbar_base>(
			vertical_scrollbar_grid_, "_vertical_scrollbar", false, true);

	connect_signal_notify_modified(
			*vertical_scrollbar_,
			std::bind(&scrollbar_container::vertical_scrollbar_moved, this));

	/***** Setup horizontal scrollbar *****/
	horizontal_scrollbar_grid_ = find_widget<grid>(
			this, "_horizontal_scrollbar_grid", false, true);

	horizontal_scrollbar_ = find_widget<scrollbar_base>(
			horizontal_scrollbar_grid_, "_horizontal_scrollbar", false, true);

	connect_signal_notify_modified(
			*horizontal_scrollbar_,
			std::bind(&scrollbar_container::horizontal_scrollbar_moved,
						this));

	/***** Setup the scrollbar buttons *****/
	for(const auto & item : scroll_lookup())
	{

		// Vertical.
		clickable_item* button = find_widget<clickable_item>(
				vertical_scrollbar_grid_, item.first, false, false);

		if(button) {
			button->connect_click_handler(std::bind(
					&scrollbar_container::scroll_vertical_scrollbar,
					this,
					item.second));
		}

		// Horizontal.
		button = find_widget<clickable_item>(
				horizontal_scrollbar_grid_, item.first, false, false);

		if(button) {
			button->connect_click_handler(std::bind(
					&scrollbar_container::scroll_horizontal_scrollbar,
					this,
					item.second));
		}
	}

	/***** Setup the content *****/
	content_ = new spacer();
	content_->set_definition("default");

	content_grid_ = dynamic_cast<grid*>(
			get_grid().swap_child("_content_grid", content_, true));
	assert(content_grid_);

	content_grid_->set_parent(this);

	/***** Let our subclasses initialize themselves. *****/
	finalize_subclass();
}

void scrollbar_container::set_vertical_scrollbar_mode(
		const scrollbar_mode scrollbar_mode)
{
	if(vertical_scrollbar_mode_ != scrollbar_mode) {
		vertical_scrollbar_mode_ = scrollbar_mode;
	}
}

void scrollbar_container::set_horizontal_scrollbar_mode(
		const scrollbar_mode scrollbar_mode)
{
	if(horizontal_scrollbar_mode_ != scrollbar_mode) {
		horizontal_scrollbar_mode_ = scrollbar_mode;
	}
}

void scrollbar_container::impl_draw_children(surface& frame_buffer,
											  int x_offset,
											  int y_offset)
{
	assert(get_visible() == widget::visibility::visible
		   && content_grid_->get_visible() == widget::visibility::visible);

	// Inherited.
	container_base::impl_draw_children(frame_buffer, x_offset, y_offset);

	content_grid_->draw_children(frame_buffer, x_offset, y_offset);
}

void scrollbar_container::layout_children()
{
	// Inherited.
	container_base::layout_children();

	assert(content_grid_);
	content_grid_->layout_children();
}

void scrollbar_container::child_populate_dirty_list(
		window& caller, const std::vector<widget*>& call_stack)
{
	// Inherited.
	container_base::child_populate_dirty_list(caller, call_stack);

	assert(content_grid_);
	std::vector<widget*> child_call_stack(call_stack);
	content_grid_->populate_dirty_list(caller, child_call_stack);
}

void scrollbar_container::set_content_size(const point& origin,
											const point& size)
{
	content_grid_->place(origin, size);
}

void scrollbar_container::show_content_rect(const SDL_Rect& rect)
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

void scrollbar_container::set_scrollbar_button_status()
{
	if(true) { /** @todo scrollbar visibility. */
		/***** set scroll up button status *****/
		for(const auto & name : button_up_names)
		{
			styled_widget* button = find_widget<styled_widget>(
					vertical_scrollbar_grid_, name, false, false);

			if(button) {
				button->set_active(!vertical_scrollbar_->at_begin());
			}
		}

		/***** set scroll down status *****/
		for(const auto & name : button_down_names)
		{
			styled_widget* button = find_widget<styled_widget>(
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
		for(const auto & name : button_up_names)
		{
			styled_widget* button = find_widget<styled_widget>(
					horizontal_scrollbar_grid_, name, false, false);

			if(button) {
				button->set_active(!horizontal_scrollbar_->at_begin());
			}
		}

		/***** Set scroll right button status *****/
		for(const auto & name : button_down_names)
		{
			styled_widget* button = find_widget<styled_widget>(
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

bool scrollbar_container::vertical_scrollbar_at_end()
{
	assert(vertical_scrollbar_);

	return vertical_scrollbar_->at_end();
}

unsigned scrollbar_container::get_vertical_scrollbar_item_position() const
{
	assert(vertical_scrollbar_);

	return vertical_scrollbar_->get_item_position();
}

void scrollbar_container::set_vertical_scrollbar_item_position(
		const unsigned position)
{
	assert(vertical_scrollbar_);

	vertical_scrollbar_->set_item_position(position);
	scrollbar_moved();
}

unsigned scrollbar_container::get_horizontal_scrollbar_item_position() const
{
	assert(horizontal_scrollbar_);

	return horizontal_scrollbar_->get_item_position();
}

void scrollbar_container::set_horizontal_scrollbar_item_position(
	const unsigned position)
{
	assert(horizontal_scrollbar_);

	horizontal_scrollbar_->set_item_position(position);
	scrollbar_moved();
}

void scrollbar_container::scroll_vertical_scrollbar(
		const scrollbar_base::scroll_mode scroll)
{
	assert(vertical_scrollbar_);

	vertical_scrollbar_->scroll(scroll);
	scrollbar_moved();
}

void scrollbar_container::scroll_horizontal_scrollbar(
		const scrollbar_base::scroll_mode scroll)
{
	assert(horizontal_scrollbar_);

	horizontal_scrollbar_->scroll(scroll);
	scrollbar_moved();
}

void scrollbar_container::handle_key_home(SDL_Keymod /*modifier*/, bool& handled)
{
	assert(vertical_scrollbar_ && horizontal_scrollbar_);

	vertical_scrollbar_->scroll(scrollbar_base::BEGIN);
	horizontal_scrollbar_->scroll(scrollbar_base::BEGIN);
	scrollbar_moved();

	handled = true;
}

void scrollbar_container::handle_key_end(SDL_Keymod /*modifier*/, bool& handled)
{
	assert(vertical_scrollbar_);

	vertical_scrollbar_->scroll(scrollbar_base::END);
	scrollbar_moved();

	handled = true;
}

void scrollbar_container::handle_key_page_up(SDL_Keymod /*modifier*/,
											  bool& handled)
{
	assert(vertical_scrollbar_);

	vertical_scrollbar_->scroll(scrollbar_base::JUMP_BACKWARDS);
	scrollbar_moved();

	handled = true;
}

void scrollbar_container::handle_key_page_down(SDL_Keymod /*modifier*/,
												bool& handled)

{
	assert(vertical_scrollbar_);

	vertical_scrollbar_->scroll(scrollbar_base::JUMP_FORWARD);
	scrollbar_moved();

	handled = true;
}

void scrollbar_container::handle_key_up_arrow(SDL_Keymod /*modifier*/,
											   bool& handled)
{
	assert(vertical_scrollbar_);

	vertical_scrollbar_->scroll(scrollbar_base::ITEM_BACKWARDS);
	scrollbar_moved();

	handled = true;
}

void scrollbar_container::handle_key_down_arrow(SDL_Keymod /*modifier*/,
												 bool& handled)
{
	assert(vertical_scrollbar_);

	vertical_scrollbar_->scroll(scrollbar_base::ITEM_FORWARD);
	scrollbar_moved();

	handled = true;
}

void scrollbar_container::handle_key_left_arrow(SDL_Keymod /*modifier*/,
												 bool& handled)
{
	assert(horizontal_scrollbar_);

	horizontal_scrollbar_->scroll(scrollbar_base::ITEM_BACKWARDS);
	scrollbar_moved();

	handled = true;
}

void scrollbar_container::handle_key_right_arrow(SDL_Keymod /*modifier*/,
												  bool& handled)
{
	assert(horizontal_scrollbar_);

	horizontal_scrollbar_->scroll(scrollbar_base::ITEM_FORWARD);
	scrollbar_moved();

	handled = true;
}

void scrollbar_container::scrollbar_moved()
{
	// Init.
	assert(content_ && content_grid_);
	assert(vertical_scrollbar_ && horizontal_scrollbar_);

	/*** Update the content location. ***/
	const int x_offset = horizontal_scrollbar_mode_ == ALWAYS_INVISIBLE
								 ? 0
								 : horizontal_scrollbar_->get_item_position()
								   * horizontal_scrollbar_->get_step_size();

	const int y_offset = vertical_scrollbar_mode_ == ALWAYS_INVISIBLE
								 ? 0
								 : vertical_scrollbar_->get_item_position()
								   * vertical_scrollbar_->get_step_size();

	const point content_origin = {content_->get_x() - x_offset, content_->get_y() - y_offset};

	content_grid_->set_origin(content_origin);
	content_grid_->set_visible_rectangle(content_visible_area_);
	content_grid_->set_is_dirty(true);

	// Update scrollbar.
	set_scrollbar_button_status();
}

const std::string& scrollbar_container::get_control_type() const
{
	static const std::string type = "scrollbar_container";
	return type;
}

void
scrollbar_container::signal_handler_sdl_key_down(const event::ui_event event,
												  bool& handled,
												  const SDL_Keycode key,
												  SDL_Keymod modifier)
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
scrollbar_container::signal_handler_sdl_wheel_up(const event::ui_event event,
												  bool& handled)
{
	DBG_GUI_E << LOG_HEADER << event << ".\n";

	assert(vertical_scrollbar_grid_ && vertical_scrollbar_);

	if(vertical_scrollbar_grid_->get_visible() == widget::visibility::visible) {
		vertical_scrollbar_->scroll(scrollbar_base::HALF_JUMP_BACKWARDS);
		scrollbar_moved();
		handled = true;
	}
}

void
scrollbar_container::signal_handler_sdl_wheel_down(const event::ui_event event,
													bool& handled)
{
	DBG_GUI_E << LOG_HEADER << event << ".\n";

	assert(vertical_scrollbar_grid_ && vertical_scrollbar_);

	if(vertical_scrollbar_grid_->get_visible() == widget::visibility::visible) {
		vertical_scrollbar_->scroll(scrollbar_base::HALF_JUMP_FORWARD);
		scrollbar_moved();
		handled = true;
	}
}

void
scrollbar_container::signal_handler_sdl_wheel_left(const event::ui_event event,
													bool& handled)
{
	DBG_GUI_E << LOG_HEADER << event << ".\n";

	assert(horizontal_scrollbar_grid_ && horizontal_scrollbar_);

	if(horizontal_scrollbar_grid_->get_visible()
	   == widget::visibility::visible) {
		horizontal_scrollbar_->scroll(scrollbar_base::HALF_JUMP_BACKWARDS);
		scrollbar_moved();
		handled = true;
	}
}

void
scrollbar_container::signal_handler_sdl_wheel_right(const event::ui_event event,
													 bool& handled)
{
	DBG_GUI_E << LOG_HEADER << event << ".\n";

	assert(horizontal_scrollbar_grid_ && horizontal_scrollbar_);

	if(horizontal_scrollbar_grid_->get_visible()
	   == widget::visibility::visible) {
		horizontal_scrollbar_->scroll(scrollbar_base::HALF_JUMP_FORWARD);
		scrollbar_moved();
		handled = true;
	}
}

} // namespace gui2
