/*

   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
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

#include "widgets/widget.hpp"
#include "video.hpp"
#include "sdl/rect.hpp"
#include "tooltips.hpp"

#include <cassert>

namespace {
	const SDL_Rect EmptyRect {-1234,-1234,0,0};
}

namespace gui {

bool widget::mouse_lock_ = false;

widget::widget(const widget &o)
	: events::sdl_handler(), focus_(o.focus_), video_(o.video_), restorer_(o.restorer_), rect_(o.rect_),
	   needs_restore_(o.needs_restore_), state_(o.state_), hidden_override_(o.hidden_override_),
	  enabled_(o.enabled_), clip_(o.clip_), clip_rect_(o.clip_rect_), volatile_(o.volatile_),
	  help_text_(o.help_text_), tooltip_text_(o.tooltip_text_), help_string_(o.help_string_), id_(o.id_), mouse_lock_local_(o.mouse_lock_local_)
{
}

widget::widget(CVideo& video, const bool auto_join)
	: events::sdl_handler(auto_join), focus_(true), video_(&video), rect_(EmptyRect), needs_restore_(false),
	  state_(UNINIT), hidden_override_(false), enabled_(true), clip_(false),
	  clip_rect_(EmptyRect), volatile_(false), help_string_(0), mouse_lock_local_(false)
{
}

widget::~widget()
{
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

void widget::bg_cancel()
{
	for(std::vector< surface_restorer >::iterator i = restorer_.begin(),
	    i_end = restorer_.end(); i != i_end; ++i)
		i->cancel();
	restorer_.clear();
}

void widget::set_location(SDL_Rect const &rect)
{
	if(rect_.x == rect.x && rect_.y == rect.y && rect_.w == rect.w && rect_.h == rect.h)
		return;
	if(state_ == UNINIT && rect.x != -1234 && rect.y != -1234)
		state_ = DRAWN;

	bg_restore();
	bg_cancel();
	rect_ = rect;
	set_dirty(true);
	update_location(rect);
}

void widget::update_location(SDL_Rect const &rect)
{
	bg_register(rect);
}

const SDL_Rect* widget::clip_rect() const
{
	return clip_ ? &clip_rect_ : nullptr;
}

void widget::bg_register(SDL_Rect const &rect)
{
	restorer_.push_back(surface_restorer(&video(), rect));
}

void widget::set_location(int x, int y)
{
	set_location(sdl::create_rect(x, y, rect_.w, rect_.h));
}

void widget::set_width(int w)
{
	set_location(sdl::create_rect(rect_.x, rect_.y, w, rect_.h));
}

void widget::set_height(int h)
{
	set_location(sdl::create_rect(rect_.x, rect_.y, rect_.w, h));
}

void widget::set_measurements(int w, int h)
{
	set_location(sdl::create_rect(rect_.x, rect_.y, w, h));
}

int widget::width() const
{
	return rect_.w;
}

int widget::height() const
{
	return rect_.h;
}

const SDL_Rect& widget::location() const
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
		|| (clip_ && !sdl::rects_overlap(clip_rect_, rect_)));
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

void widget::set_dirty(bool dirty)
{
	if ((dirty && (volatile_ || hidden_override_ || state_ != DRAWN)) || (!dirty && state_ != DIRTY))
		return;

	state_ = dirty ? DIRTY : DRAWN;
	if (!dirty)
		needs_restore_ = true;
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
	for(std::vector< surface_restorer >::iterator i = restorer_.begin(),
	    i_end = restorer_.end(); i != i_end; ++i)
		i->update();
}

void widget::bg_restore() const
{
	clip_rect_setter clipper(video().getSurface(), &clip_rect_, clip_);

	if (needs_restore_) {
		for(std::vector< surface_restorer >::const_iterator i = restorer_.begin(),
		    i_end = restorer_.end(); i != i_end; ++i)
			i->restore();
		needs_restore_ = false;
	}
}

void widget::bg_restore(SDL_Rect const &rect) const
{
	clip_rect_setter clipper(video().getSurface(), &clip_rect_, clip_);

	for(std::vector< surface_restorer >::const_iterator i = restorer_.begin(),
	    i_end = restorer_.end(); i != i_end; ++i)
		i->restore(rect);
}

void widget::set_volatile(bool val)
{
	volatile_ = val;
	if (volatile_ && state_ == DIRTY)
		state_ = DRAWN;
}

void widget::draw()
{
	if (hidden() || !dirty())
		return;

	bg_restore();

	clip_rect_setter clipper(video().getSurface(), &clip_rect_, clip_);

	draw_contents();

	set_dirty(false);
}

void widget::volatile_draw()
{
	if (!volatile_ || state_ != DRAWN || hidden_override_)
		return;
	state_ = DIRTY;
	bg_update();
	draw();
}

void widget::volatile_undraw()
{
	if (!volatile_)
		return;
	bg_restore();
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
	if (!hidden() && sdl::point_in_rect(mousex, mousey, rect_)) {
		if(help_string_ == 0 && help_text_ != "") {
			//std::cerr << "setting help string to '" << help_text_ << "'\n";
			help_string_ = video().set_help_string(help_text_);
		}
	} else if(help_string_ > 0) {
		video().clear_help_string(help_string_);
		help_string_ = 0;
	}
}

void widget::process_tooltip_string(int mousex, int mousey)
{
	if (!hidden() && sdl::point_in_rect(mousex, mousey, rect_)) {
		if (!tooltip_text_.empty())
			tooltips::add_tooltip(rect_, tooltip_text_ );
	}
}

void widget::handle_event(SDL_Event const &event) {
	if (event.type == DRAW_ALL_EVENT) {
		set_dirty();
		draw();
	}
}

void widget::handle_window_event(SDL_Event const &event) {
	if (event.type == SDL_WINDOWEVENT) {
		switch (event.window.event) {
		case SDL_WINDOWEVENT_RESIZED:
		case SDL_WINDOWEVENT_RESTORED:
		case SDL_WINDOWEVENT_SHOWN:
		case SDL_WINDOWEVENT_EXPOSED:
			set_dirty();
		}
	}
}


}
