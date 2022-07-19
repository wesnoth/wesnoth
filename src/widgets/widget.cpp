/*
	Copyright (C) 2003 - 2022
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
#include "floating_label.hpp"
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
	: events::sdl_handler(auto_join), focus_(true), rect_(EmptyRect), needs_restore_(false),
	  state_(UNINIT), hidden_override_(false), enabled_(true), clip_(false),
	  clip_rect_(EmptyRect), has_help_(false), mouse_lock_local_(false)
{
}

widget::~widget()
{
	if (!hidden()) {
		queue_redraw();
	}
	bg_cancel();
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

// TODO: draw_manager - kill surface restorers
void widget::bg_cancel()
{
	/*for(std::vector< surface_restorer >::iterator i = restorer_.begin(),
	    i_end = restorer_.end(); i != i_end; ++i)
		i->cancel();*/
	restorer_.clear();
}

void widget::set_location(const SDL_Rect& rect)
{
	if(rect_.x == rect.x && rect_.y == rect.y && rect_.w == rect.w && rect_.h == rect.h)
		return;
	if(state_ == UNINIT && rect.x != -1234 && rect.y != -1234)
		state_ = DRAWN;

	// TODO: draw_manager - overhaul
	bg_restore();
	bg_cancel();
	rect_ = rect;
	set_dirty(true);
	update_location(rect);
}

void widget::update_location(const SDL_Rect& rect)
{
	bg_register(rect);
}

void widget::layout()
{
	// this basically happens in set_location, so there's nothing to do here.
}

const SDL_Rect* widget::clip_rect() const
{
	return clip_ ? &clip_rect_ : nullptr;
}

void widget::bg_register(const SDL_Rect& rect)
{
	restorer_.emplace_back(rect);
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
	set_dirty(true);
}

bool widget::focus(const SDL_Event* event)
{
	return events::has_focus(this, event) && focus_;
}

void widget::hide(bool value)
{
	if (value) {
		if ((state_ == DIRTY || state_ == DRAWN) && !hidden_override_)
			bg_restore();
		state_ = HIDDEN;
	} else if (state_ == HIDDEN) {
		state_ = DRAWN;
		if (!hidden_override_) {
			bg_update();
			set_dirty(true);
		}
	}
}

void widget::hide_override(bool value) {
	if (hidden_override_ != value) {
		hidden_override_ = value;
		if (state_ == DIRTY || state_ == DRAWN) {
			if (value) {
				bg_restore();
			} else {
				bg_update();
				set_dirty(true);
			}
		}
	}
}

void widget::set_clip_rect(const SDL_Rect& rect)
{
	clip_rect_ = rect;
	clip_ = true;
	set_dirty(true);
}

bool widget::hidden() const
{
	return (state_ == HIDDEN || hidden_override_ || state_ == UNINIT
		|| (clip_ && !rect_.overlaps(clip_rect_)));
}

void widget::enable(bool new_val)
{
	if (enabled_ != new_val) {
		enabled_ = new_val;
		set_dirty();
	}
}

bool widget::enabled() const
{
	return enabled_;
}

// TODO: draw_manager - this needs to die
void widget::set_dirty(bool dirty)
{
	if ((dirty && (hidden_override_ || state_ != DRAWN)) || (!dirty && state_ != DIRTY))
		return;

	state_ = dirty ? DIRTY : DRAWN;
	if (dirty) {
		queue_redraw();
	}
	//if (!dirty)
	//	needs_restore_ = true;
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

void widget::bg_update()
{
	/*for(std::vector< surface_restorer >::iterator i = restorer_.begin(),
	    i_end = restorer_.end(); i != i_end; ++i)
		i->update();*/
}

void widget::bg_restore() const
{
	if (needs_restore_) {
		for(const rect& r : restorer_) {
			if(clip_) {
				draw_manager::invalidate_region(r.intersect(clip_rect_));
			} else {
				draw_manager::invalidate_region(r);
			}
		}
		/*for(std::vector< surface_restorer >::const_iterator i = restorer_.begin(),
		    i_end = restorer_.end(); i != i_end; ++i)
			i->restore();*/
		needs_restore_ = false;
	}
}

void widget::bg_restore(const SDL_Rect& where) const
{
	rect c = clip_ ? clip_rect_ : where;
	c.clip(where);

	for(const rect& r : restorer_) {
		draw_manager::invalidate_region(r.intersect(c));
	}
	/*for(std::vector< surface_restorer >::const_iterator i = restorer_.begin(),
	    i_end = restorer_.end(); i != i_end; ++i)
		i->restore(rect);*/
}

void widget::queue_redraw(const rect& r)
{
	draw_manager::invalidate_region(r);
}

void widget::queue_redraw()
{
	queue_redraw(location());
}

bool widget::expose(const SDL_Rect& region)
{
	// TODO: draw_manager - draw always? or only when dirty?
	//if (!dirty()) {
	//	return false;
	//}
	(void)region;
	draw();
	return true;
}

void widget::draw()
{
	if (hidden())
		return;

	//bg_restore();

	if (clip_) {
		auto clipper = draw::reduce_clip(clip_rect_);
		draw_contents();
	} else {
		draw_contents();
	}

	set_dirty(false);
}

void widget::set_help_string(const std::string& str)
{
	help_text_ = str;
}

void widget::set_tooltip_string(const std::string& str)
{
	tooltip_text_ = str;
}

void widget::process_help_string(int mousex, int mousey)
{
	if (!hidden() && rect_.contains(mousex, mousey)) {
		if(!has_help_ && !help_text_.empty()) {
			//PLAIN_LOG << "setting help string to '" << help_text_ << "'";
			font::set_help_string(help_text_);
			has_help_ = true;
		}
	} else if(has_help_) {
		font::clear_help_string();
		has_help_ = false;
	}
}

void widget::process_tooltip_string(int mousex, int mousey)
{
	if (!hidden() && rect_.contains(mousex, mousey)) {
		if (!tooltip_text_.empty())
			tooltips::add_tooltip(rect_, tooltip_text_ );
	}
}

}
