/* $Id$ */
/*
   Copyright (C) 2007 - 2010 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/widgets/window.hpp"
#include "gui/auxiliary/log.hpp"

namespace gui2 {

twidget::twidget()
	: id_("")
	, definition_("default")
	, parent_(0)
	, x_(-1)
	, y_(-1)
	, w_(0)
	, h_(0)
	, dirty_(true)
	, visible_(VISIBLE)
	, drawing_action_(DRAWN)
	, clip_rect_()
	, layout_size_(tpoint(0,0))
	, linked_group_()
#ifndef LOW_MEM
	, debug_border_mode_(0)
	, debug_border_colour_(0)
#endif
#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
	, last_best_size_(tpoint(0,0))
#endif
{
	DBG_GUI_LF << "widget create: " << (void*)this << "\n";
}

twidget::~twidget()
{
	DBG_GUI_LF << "widget destroy: " << (void*)this
		<< " (id: " << id_ << ", definition: " << definition_ << ")\n";

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

void twidget::set_id(const std::string& id)
{
	DBG_GUI_LF << "set id of " << (void*)this << " to '" << id << "' "
		<< "(was '" << id_ << "'). Widget type: " <<
		(dynamic_cast<tcontrol*>(this) ?
			dynamic_cast<tcontrol*>(this)->get_control_type()
			: typeid(twidget).name())
		<< "\n";
	id_ = id;
}


void twidget::layout_init(const bool /*full_initialization*/)
{
	assert(visible_ != INVISIBLE);
	assert(get_window());

	layout_size_ = tpoint(0,0);
	if(!linked_group_.empty()) {
		get_window()->add_linked_widget(linked_group_, this);
	}
}

tpoint twidget::get_best_size() const
{
	assert(visible_ != INVISIBLE);

	tpoint result = layout_size_;
	if(result == tpoint(0, 0)) {
		result = calculate_best_size();
	}

#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
	last_best_size_ = result;
#endif
	return result;
}

void twidget::set_size(const tpoint& origin, const tpoint& size)
{
	assert(size.x >= 0);
	assert(size.y >= 0);

	x_ = origin.x;
	y_ = origin.y;
	w_ = size.x;
	h_ = size.y;

#if 0
	std::cerr << "Id " << id()
		<< " rect " << get_rect()
		<< " parent "
			<< (parent ? parent->get_x() : 0)
			<< ','
			<< (parent ? parent->get_y() : 0)
		<< " screen origin " << x_ << ',' << y_
		<< ".\n";
#endif

	set_dirty();
}

twidget* twidget::find_at(const tpoint& coordinate,
		const bool must_be_active)
{
	return is_at(coordinate, must_be_active) ? this : 0;
}

const twidget* twidget::find_at(const tpoint& coordinate,
		const bool must_be_active) const
{
	return is_at(coordinate, must_be_active) ? this : 0;
}

SDL_Rect twidget::get_dirty_rect() const
{
	return drawing_action_ == DRAWN
			? get_rect()
			: clip_rect_;
}

twindow* twidget::get_window()
{
	// Go up into the parent tree until we find the top level
	// parent, we can also be the toplevel so start with
	// ourselves instead of our parent.
	twidget* result = this;
	while(result->parent_) {
		result = result->parent_;
	}

	// on error dynamic_cast return 0 which is what we want.
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

	// on error dynamic_cast return 0 which is what we want.
	return dynamic_cast<const twindow*>(result);
}

tdialog* twidget::dialog()
{
	twindow* window = get_window();
	return window ? window->dialog() : 0;
}

void twidget::populate_dirty_list(twindow& caller,
		std::vector<twidget*>& call_stack)
{
	assert(call_stack.empty() || call_stack.back() != this);

	if(visible_ != VISIBLE) {
		return;
	}

	if(get_drawing_action() == NOT_DRAWN) {
		return;
	}

	call_stack.push_back(this);
	if(dirty_) {
		caller.add_to_dirty_list(call_stack);
	} else {
		// virtual function which only does something for container items.
		child_populate_dirty_list(caller, call_stack);
	}
}

void twidget::set_visible(const tvisible visible)
{
	if(visible == visible_) {
		return;
	}

	// Switching to or from invisible should invalidate the layout.
	if(visible_ == INVISIBLE || visible == INVISIBLE) {
		twindow *window = get_window();
		if(window) {
			window->invalidate_layout();
		}
	} else {
		set_dirty();
	}

	visible_ = visible;
}

twidget::tdrawing_action twidget::get_drawing_action() const
{
	return (w_ == 0 || h_ == 0)
		? NOT_DRAWN
		: drawing_action_;
}

void twidget::set_visible_area(const SDL_Rect& area)
{
	clip_rect_ = get_rect_union(area, get_rect());

	if(clip_rect_ == get_rect()) {
		drawing_action_ = DRAWN;
	} else if(clip_rect_ == empty_rect) {
		drawing_action_ = NOT_DRAWN;
	} else {
		drawing_action_ = PARTLY_DRAWN;
	}
}

void twidget::draw_background(surface& frame_buffer)
{
	assert(visible_ == VISIBLE);

	if(drawing_action_ == PARTLY_DRAWN) {
		clip_rect_setter clip(frame_buffer, clip_rect_);
		draw_debug_border(frame_buffer);
		impl_draw_background(frame_buffer);
	} else {
		draw_debug_border(frame_buffer);
		impl_draw_background(frame_buffer);
	}
}

void twidget::draw_children(surface& frame_buffer)
{
	assert(visible_ == VISIBLE);

	if(drawing_action_ == PARTLY_DRAWN) {
		clip_rect_setter clip(frame_buffer, clip_rect_);
		impl_draw_children(frame_buffer);
	} else {
		impl_draw_children(frame_buffer);
	}
}

void twidget::draw_foreground(surface& frame_buffer)
{
	assert(visible_ == VISIBLE);

	if(drawing_action_ == PARTLY_DRAWN) {
		clip_rect_setter clip(frame_buffer, clip_rect_);
		impl_draw_foreground(frame_buffer);
	} else {
		impl_draw_foreground(frame_buffer);
	}
}

#ifndef LOW_MEM
void twidget::draw_debug_border(surface& frame_buffer)
{
	SDL_Rect r = drawing_action_ == PARTLY_DRAWN
		? clip_rect_
		: get_rect();
	switch(debug_border_mode_) {
		case 0:
			/* DO NOTHING */
			break;
		case 1:
			draw_rectangle(r.x, r.y, r.w, r.h
					, debug_border_colour_, frame_buffer);
			break;
		case 2:
			SDL_FillRect(frame_buffer, &r, debug_border_colour_);
			break;
		default:
			assert(false);
	}
}
#endif

bool twidget::is_at(const tpoint& coordinate, const bool must_be_active) const
{
	if(visible_ == INVISIBLE
			|| (visible_ == HIDDEN && must_be_active)) {
		return false;
	}

	return coordinate.x >= x_
			&& coordinate.x < (x_ + static_cast<int>(w_))
			&& coordinate.y >= y_
			&& coordinate.y < (y_ + static_cast<int>(h_)) ? true : false;
}
} // namespace gui2
