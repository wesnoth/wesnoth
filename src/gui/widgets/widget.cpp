/* $Id$ */
/*
   copyright (C) 2007 - 2008 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/widgets/window.hpp"

namespace gui2 {

tpoint twidget::get_best_size() const
{
	assert(!is_invisible());

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
	x_ = origin.x;
	y_ = origin.y;
	w_ = size.x;
	h_ = size.y;

	screen_x_ = x_;
	screen_y_ = y_;
	twidget* parent = get_window();
	if(parent && parent != this) {
		screen_x_ += parent->get_screen_x();
		screen_y_ += parent->get_screen_y();
	}
#if 0
	std::cerr << "Id " << id()
		<< " rect " << get_rect()
		<< " parent "
			<< (parent ? parent->get_screen_x() : 0)
			<< ','
			<< (parent ? parent->get_screen_y() : 0)
		<< " screen origin " << screen_x_ << ',' << screen_y_
		<< ".\n";
#endif

	set_dirty();
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
	if(!is_visible()) {
		return;
	}
	
	if(drawing_action_ == NOT_DRAWN) {
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
	if(visible_ != INVISIBLE || visible == INVISIBLE) {
		twindow *window = get_window();
		if(window) {
			window->invalidate_layout();
		}
	}

	visible_ = visible;
}

void twidget::set_visible_area(const SDL_Rect& area)
{
	clip_rect_ = get_rect_union(area, get_screen_rect());

	if(clip_rect_ == get_screen_rect()) {
		drawing_action_ = DRAWN;
	} else if(clip_rect_ == empty_rect) {
		drawing_action_ = NOT_DRAWN;
	} else {
		drawing_action_ = PARTLY_DRAWN;
	}
}


} // namespace gui2
