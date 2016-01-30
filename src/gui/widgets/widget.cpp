/*
   Copyright (C) 2007 - 2016 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "gui/auxiliary/event/message.hpp"
#include "gui/auxiliary/log.hpp"
#include "sdl/rect.hpp"

namespace gui2
{

/***** ***** ***** Constructor and destructor. ***** ***** *****/

twidget::twidget()
	: id_("")
	, parent_(NULL)
	, x_(-1)
	, y_(-1)
	, width_(0)
	, height_(0)
	, layout_size_(tpoint(0, 0))
#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
	, last_best_size_(tpoint(0, 0))
#endif
	, linked_group_()
	, is_dirty_(true)
	, visible_(tvisible::visible)
	, redraw_action_(tredraw_action::full)
	, clipping_rectangle_()
#ifndef LOW_MEM
	, debug_border_mode_(0)
	, debug_border_colour_(0)
#endif
{
	DBG_GUI_LF << "widget create: " << static_cast<void*>(this) << "\n";
}

twidget::twidget(const tbuilder_widget& builder)
	: id_(builder.id)
	, parent_(NULL)
	, x_(-1)
	, y_(-1)
	, width_(0)
	, height_(0)
	, layout_size_(tpoint(0, 0))
#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
	, last_best_size_(tpoint(0, 0))
#endif
	, linked_group_(builder.linked_group)
	, is_dirty_(true)
	, visible_(tvisible::visible)
	, redraw_action_(tredraw_action::full)
	, clipping_rectangle_()
#ifndef LOW_MEM
	, debug_border_mode_(builder.debug_border_mode)
	, debug_border_colour_(builder.debug_border_color)
#endif
{
	DBG_GUI_LF << "widget create: " << static_cast<void*>(this) << "\n";
}

twidget::~twidget()
{
	DBG_GUI_LF
	<< "widget destroy: " << static_cast<void*>(this) << " (id: " << id_
	<< ")\n";

	twidget* p = parent();
	while(p) {
		fire(event::NOTIFY_REMOVAL, *p, NULL);
		p = p->parent();
	}

	if(!linked_group_.empty()) {
		if(twindow* window = get_window()) {
			window->remove_linked_widget(linked_group_, this);
		}
	}
}

/***** ***** ***** ***** ID functions. ***** ***** ***** *****/

void twidget::set_id(const std::string& id)
{
	tcontrol* this_ctrl = dynamic_cast<tcontrol*>(this);

	DBG_GUI_LF
	<< "set id of " << static_cast<void*>(this) << " to '" << id << "' "
	<< "(was '" << id_ << "'). Widget type: "
	<< (this_ctrl ? this_ctrl->get_control_type() : typeid(twidget).name()) << "\n";

	id_ = id;
}

const std::string& twidget::id() const
{
	return id_;
}

/***** ***** ***** ***** Parent functions ***** ***** ***** *****/

twindow* twidget::get_window()
{
	// Go up into the parent tree until we find the top level
	// parent, we can also be the toplevel so start with
	// ourselves instead of our parent.
	twidget* result = this;
	while(result->parent_) {
		result = result->parent_;
	}

	// on error dynamic_cast returns NULL which is what we want.
	return dynamic_cast<twindow*>(result);
}

const twindow* twidget::get_window() const
{
	// Go up into the parent tree until we find the top level
	// parent, we can also be the toplevel so start with
	// ourselves instead of our parent.
	const twidget* result = this;
	while(result->parent_) {
		result = result->parent_;
	}

	// on error dynamic_cast returns NULL which is what we want.
	return dynamic_cast<const twindow*>(result);
}

tdialog* twidget::dialog()
{
	twindow* window = get_window();
	return window ? window->dialog() : NULL;
}

void twidget::set_parent(twidget* parent)
{
	parent_ = parent;
}

twidget* twidget::parent()
{
	return parent_;
}

/***** ***** ***** ***** Size and layout functions. ***** ***** ***** *****/

void twidget::layout_initialise(const bool /*full_initialisation*/)
{
	assert(visible_ != tvisible::invisible);
	assert(get_window());

	layout_size_ = tpoint(0, 0);
	if(!linked_group_.empty()) {
		get_window()->add_linked_widget(linked_group_, this);
	}
}

void twidget::demand_reduce_width(const unsigned /*maximum_width*/)
{
	/* DO NOTHING */
}

void twidget::request_reduce_height(const unsigned /*maximum_height*/)
{
	/* DO NOTHING */
}

void twidget::demand_reduce_height(const unsigned /*maximum_height*/)
{
	/* DO NOTHING */
}

tpoint twidget::get_best_size() const
{
	assert(visible_ != tvisible::invisible);

	tpoint result = layout_size_;
	if(result == tpoint(0, 0)) {
		result = calculate_best_size();
		//Adjust to linked widget size if linked widget size was already calculated.
		if(!get_window()->get_need_layout() && !linked_group_.empty())
		{
			tpoint linked_size = get_window()->get_linked_size(linked_group_);
			result.x = std::max(result.x, linked_size.x);
			result.y = std::max(result.y, linked_size.y);
		}
	}

#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
	last_best_size_ = result;
#endif

	return result;
}

bool twidget::can_wrap() const
{
	return false;
}

void twidget::set_origin(const tpoint& origin)
{
	x_ = origin.x;
	y_ = origin.y;
}

void twidget::set_size(const tpoint& size)
{
	assert(size.x >= 0);
	assert(size.y >= 0);

	width_ = size.x;
	height_ = size.y;

	set_is_dirty(true);
}

void twidget::place(const tpoint& origin, const tpoint& size)
{
	assert(size.x >= 0);
	assert(size.y >= 0);

	x_ = origin.x;
	y_ = origin.y;
	width_ = size.x;
	height_ = size.y;

#if 0
	std::cerr
			<< "Id " << id()
			<< " rect " << get_rectangle()
			<< " parent "
			<< (parent ? parent->get_x() : 0)
			<< ','
			<< (parent ? parent->get_y() : 0)
			<< " screen origin " << x_ << ',' << y_
			<< ".\n";
#endif

	set_is_dirty(true);
}

void twidget::move(const int x_offset, const int y_offset)
{
	x_ += x_offset;
	y_ += y_offset;
}

void twidget::layout_children()
{
	/* DO NOTHING */
}

tpoint twidget::get_origin() const
{
	return tpoint(x_, y_);
}

tpoint twidget::get_size() const
{
	return tpoint(width_, height_);
}

SDL_Rect twidget::get_rectangle() const
{
	return create_rect(get_origin(), get_size());
}

int twidget::get_x() const
{
	return x_;
}

int twidget::get_y() const
{
	return y_;
}

unsigned twidget::get_width() const
{
	return width_;
}

unsigned twidget::get_height() const
{
	return height_;
}

void twidget::set_layout_size(const tpoint& size)
{
	layout_size_ = size;
}

const tpoint& twidget::layout_size() const
{
	return layout_size_;
}

void twidget::set_linked_group(const std::string& linked_group)
{
	linked_group_ = linked_group;
}

/***** ***** ***** ***** Drawing functions. ***** ***** ***** *****/

SDL_Rect twidget::calculate_blitting_rectangle(const int x_offset,
											   const int y_offset)
{
	SDL_Rect result = get_rectangle();
	result.x += x_offset;
	result.y += y_offset;
	return result;
}

SDL_Rect twidget::calculate_clipping_rectangle(const int x_offset,
											   const int y_offset)
{
	SDL_Rect result = clipping_rectangle_;
	result.x += x_offset;
	result.y += y_offset;
	return result;
}

void twidget::draw_background(surface& frame_buffer, int x_offset, int y_offset)
{
	assert(visible_ == tvisible::visible);

	if(redraw_action_ == tredraw_action::partly) {
		const SDL_Rect clipping_rectangle
				= calculate_clipping_rectangle(x_offset, y_offset);

		clip_rect_setter clip(frame_buffer, &clipping_rectangle);
		draw_debug_border(frame_buffer, x_offset, y_offset);
		impl_draw_background(frame_buffer, x_offset, y_offset);
	} else {
		draw_debug_border(frame_buffer, x_offset, y_offset);
		impl_draw_background(frame_buffer, x_offset, y_offset);
	}
}

void twidget::draw_children(surface& frame_buffer, int x_offset, int y_offset)
{
	assert(visible_ == tvisible::visible);

	if(redraw_action_ == tredraw_action::partly) {
		const SDL_Rect clipping_rectangle
				= calculate_clipping_rectangle(x_offset, y_offset);

		clip_rect_setter clip(frame_buffer, &clipping_rectangle);
		impl_draw_children(frame_buffer, x_offset, y_offset);
	} else {
		impl_draw_children(frame_buffer, x_offset, y_offset);
	}
}

void twidget::draw_foreground(surface& frame_buffer, int x_offset, int y_offset)
{
	assert(visible_ == tvisible::visible);

	if(redraw_action_ == tredraw_action::partly) {
		const SDL_Rect clipping_rectangle
				= calculate_clipping_rectangle(x_offset, y_offset);

		clip_rect_setter clip(frame_buffer, &clipping_rectangle);
		impl_draw_foreground(frame_buffer, x_offset, y_offset);
	} else {
		impl_draw_foreground(frame_buffer, x_offset, y_offset);
	}
}

void twidget::populate_dirty_list(twindow& caller,
								  std::vector<twidget*>& call_stack)
{
	assert(call_stack.empty() || call_stack.back() != this);

	if(visible_ != tvisible::visible) {
		return;
	}

	if(get_drawing_action() == tredraw_action::none) {
		return;
	}

	call_stack.push_back(this);
	if(is_dirty_) {
		caller.add_to_dirty_list(call_stack);
	} else {
		// virtual function which only does something for container items.
		child_populate_dirty_list(caller, call_stack);
	}
}

void
twidget::child_populate_dirty_list(twindow& /*caller*/
								   ,
								   const std::vector<twidget*>& /*call_stack*/)
{
	/* DO NOTHING */
}

SDL_Rect twidget::get_dirty_rectangle() const
{
	return redraw_action_ == tredraw_action::full ? get_rectangle()
												  : clipping_rectangle_;
}

void twidget::set_visible_rectangle(const SDL_Rect& rectangle)
{
	clipping_rectangle_ = sdl::intersect_rects(rectangle, get_rectangle());

	if(clipping_rectangle_ == get_rectangle()) {
		redraw_action_ = tredraw_action::full;
	} else if(clipping_rectangle_ == sdl::empty_rect) {
		redraw_action_ = tredraw_action::none;
	} else {
		redraw_action_ = tredraw_action::partly;
	}
}

void twidget::set_is_dirty(const bool is_dirty)
{
	is_dirty_ = is_dirty;
}

bool twidget::get_is_dirty() const
{
	return is_dirty_;
}

void twidget::set_visible(const tvisible::scoped_enum visible)
{
	if(visible == visible_) {
		return;
	}

	// Switching to or from invisible should invalidate the layout.
	const bool need_resize = visible_ == tvisible::invisible
							 || visible == tvisible::invisible;
	visible_ = visible;

	if(need_resize) {
		if(new_widgets) {
			event::tmessage message;
			fire(event::REQUEST_PLACEMENT, *this, message);
		} else {
			twindow* window = get_window();
			if(window) {
				window->invalidate_layout();
			}
		}
	} else {
		set_is_dirty(true);
	}
}

twidget::tvisible::scoped_enum twidget::get_visible() const
{
	return visible_;
}

twidget::tredraw_action::scoped_enum twidget::get_drawing_action() const
{
	return (width_ == 0 || height_ == 0) ? tredraw_action::none
										 : redraw_action_;
}

#ifndef LOW_MEM

void twidget::set_debug_border_mode(const unsigned debug_border_mode)
{
	debug_border_mode_ = debug_border_mode;
}

void twidget::set_debug_border_colour(const unsigned debug_border_colour)
{
	debug_border_colour_ = debug_border_colour;
}

void twidget::draw_debug_border(surface& frame_buffer)
{
	SDL_Rect r = redraw_action_ == tredraw_action::partly ? clipping_rectangle_
														  : get_rectangle();

	switch(debug_border_mode_) {
		case 0:
			/* DO NOTHING */
			break;
		case 1:
			sdl::draw_rectangle(
					r.x, r.y, r.w, r.h, debug_border_colour_, frame_buffer);
			break;

		case 2:
			sdl::fill_rect(frame_buffer, &r, debug_border_colour_);
			break;

		default:
			assert(false);
	}
}

void
twidget::draw_debug_border(surface& frame_buffer, int x_offset, int y_offset)
{
	SDL_Rect r = redraw_action_ == tredraw_action::partly
						 ? calculate_clipping_rectangle(x_offset, y_offset)
						 : calculate_blitting_rectangle(x_offset, y_offset);

	switch(debug_border_mode_) {
		case 0:
			/* DO NOTHING */
			break;

		case 1:
			sdl::draw_rectangle(
					r.x, r.y, r.w, r.h, debug_border_colour_, frame_buffer);
			break;

		case 2:
			sdl::fill_rect(frame_buffer, &r, debug_border_colour_);
			break;

		default:
			assert(false);
	}
}

#endif

/***** ***** ***** ***** Query functions ***** ***** ***** *****/

twidget* twidget::find_at(const tpoint& coordinate, const bool must_be_active)
{
	return is_at(coordinate, must_be_active) ? this : NULL;
}

const twidget* twidget::find_at(const tpoint& coordinate,
								const bool must_be_active) const
{
	return is_at(coordinate, must_be_active) ? this : NULL;
}

twidget* twidget::find(const std::string& id, const bool /*must_be_active*/)
{
	return id_ == id ? this : NULL;
}

const twidget* twidget::find(const std::string& id,
							 const bool /*must_be_active*/) const
{
	return id_ == id ? this : NULL;
}

bool twidget::has_widget(const twidget& widget) const
{
	return &widget == this;
}

bool twidget::is_at(const tpoint& coordinate) const
{
	return is_at(coordinate, true);
}

bool twidget::recursive_is_visible(const twidget* widget, const bool must_be_active) const
{
	while(widget) {
		if(widget->visible_ == tvisible::invisible
		   || (widget->visible_ == tvisible::hidden && must_be_active)) {
			return false;
		}

		widget = widget->parent_;
	}

	return true;
}

bool twidget::is_at(const tpoint& coordinate, const bool must_be_active) const
{
	if(!recursive_is_visible(this, must_be_active)) {
		return false;
	}

	return coordinate.x >= x_ && coordinate.x < (x_ + static_cast<int>(width_))
		   && coordinate.y >= y_
		   && coordinate.y < (y_ + static_cast<int>(height_));
}

} // namespace gui2
