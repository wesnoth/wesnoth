/*
	Copyright (C) 2003 - 2025
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "draw.hpp"
#include "draw_manager.hpp"
#include "widgets/widget.hpp"
#include "sdl/rect.hpp"
#include "tooltips.hpp"

#include <cassert>
namespace {
	const SDL_Rect EmptyRect {-1234,-1234,0,0};
}

namespace gui {

bool widget::mouse_lock_ = false;

widget::widget(const bool auto_join)
	: events::sdl_handler(auto_join), focus_(true), rect_(EmptyRect),
	  state_(UNINIT), enabled_(true), clip_(false),
	  clip_rect_(EmptyRect), mouse_lock_local_(false)
{
}

widget::~widget()
{
	if (!hidden()) {
		queue_redraw();
	}
	free_mouse_lock();
}

void widget::aquire_mouse_lock()
{
	assert(!mouse_lock_);
	mouse_lock_ = true;
	mouse_lock_local_ = true;
}

void widget::free_mouse_lock()
{
	if (mouse_lock_local_)
	{
		mouse_lock_local_ = false;
		mouse_lock_ = false;
	}
}

bool widget::mouse_locked() const
{
	return mouse_lock_ && !mouse_lock_local_;
}

void widget::set_location(const SDL_Rect& rect)
{
	if(rect_ == rect) {
		return;
	}

	// If we were shown somewhere else, queue it to be cleared.
	if(state_ == DIRTY || state_ == DRAWN) {
		queue_redraw();
	}

	if(state_ == UNINIT && rect.x != -1234 && rect.y != -1234)
		state_ = DRAWN;

	rect_ = rect;
	queue_redraw();
	update_location(rect);
}

void widget::layout()
{
	// this basically happens in set_location, so there's nothing to do here.
}

void widget::set_location(int x, int y)
{
	set_location({x, y, rect_.w, rect_.h});
}

void widget::set_width(int w)
{
	set_location({rect_.x, rect_.y, w, rect_.h});
}

void widget::set_height(int h)
{
	set_location({rect_.x, rect_.y, rect_.w, h});
}

void widget::set_measurements(int w, int h)
{
	set_location({rect_.x, rect_.y, w, h});
}

int widget::width() const
{
	return rect_.w;
}

int widget::height() const
{
	return rect_.h;
}

const rect& widget::location() const
{
	return rect_;
}

void widget::set_focus(bool focus)
{
	if (focus)
		events::focus_handler(this);
	focus_ = focus;
	queue_redraw();
}

bool widget::focus(const SDL_Event* event)
{
	return events::has_focus(this, event) && focus_;
}

void widget::hide(bool value)
{
	if (value) {
		if (state_ == DIRTY || state_ == DRAWN) {
			queue_redraw();
		}
		state_ = HIDDEN;
	} else if (state_ == HIDDEN) {
		state_ = DRAWN;
		queue_redraw();
	}
}

void widget::set_clip_rect(const SDL_Rect& rect)
{
	clip_rect_ = rect;
	clip_ = true;
	queue_redraw();
}

bool widget::hidden() const
{
	return (state_ == HIDDEN || state_ == UNINIT
		|| (clip_ && !rect_.overlaps(clip_rect_)));
}

void widget::enable(bool new_val)
{
	if (enabled_ != new_val) {
		enabled_ = new_val;
		queue_redraw();
	}
}

bool widget::enabled() const
{
	return enabled_;
}

void widget::set_dirty(bool dirty)
{
	if ((dirty && state_ != DRAWN) || (!dirty && state_ != DIRTY)) {
		return;
	}

	state_ = dirty ? DIRTY : DRAWN;

	if (dirty) {
		queue_redraw();
	}
}

bool widget::dirty() const
{
	return state_ == DIRTY;
}

const std::string& widget::id() const
{
	return id_;
}

void widget::set_id(const std::string& id)
{
	if (id_.empty()){
		id_ = id;
	}
}

void widget::queue_redraw(const rect& r)
{
	draw_manager::invalidate_region(r);
}

void widget::queue_redraw()
{
	queue_redraw(location());
}

bool widget::expose(const rect& region)
{
	if (hidden()) { return false; }
	if (!rect_.overlaps(region)) { return false; }
	if (clip_ && !clip_rect_.overlaps(region)) { return false; }

	draw();
	return true;
}

void widget::draw()
{
	if (hidden()) {
		return;
	}

	if (clip_) {
		auto clipper = draw::reduce_clip(clip_rect_);
		draw_contents();
	} else {
		draw_contents();
	}

	set_dirty(false);
}

void widget::set_tooltip_string(const std::string& str)
{
	tooltip_text_ = str;
}

void widget::process_tooltip_string(int mousex, int mousey)
{
	if (!hidden() && rect_.contains(mousex, mousey)) {
		if (!tooltip_text_.empty())
			tooltips::add_tooltip(rect_, tooltip_text_ );
	}
}

}
