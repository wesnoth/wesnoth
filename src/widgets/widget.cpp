/* $Id$

   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "widgets/widget.hpp"
#include "video.hpp"

namespace {
	const SDL_Rect EmptyRect = {-1234,-1234,0,0};
}

namespace gui {

widget::widget(const widget &o)
	: events::handler(), focus_(o.focus_), video_(o.video_), restorer_(o.restorer_), rect_(o.rect_),
	   needs_restore_(o.needs_restore_), state_(o.state_), hidden_override_(o.hidden_override_),
	  enabled_(o.enabled_), clip_(o.clip_), clip_rect_(o.clip_rect_), volatile_(o.volatile_),
	  help_text_(o.help_text_), help_string_(o.help_string_), id_(o.id_)
{
}

widget::widget(CVideo& video, const bool auto_join)
	: handler(auto_join), focus_(true), video_(&video), rect_(EmptyRect), needs_restore_(false),
	  state_(UNINIT), hidden_override_(false), enabled_(true), clip_(false),
	  clip_rect_(EmptyRect), volatile_(false), help_string_(0)
{
}

widget::~widget()
{
	bg_cancel();
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
	return clip_ ? &clip_rect_ : NULL;
}

void widget::bg_register(SDL_Rect const &rect)
{
	restorer_.push_back(surface_restorer(&video(), rect));
}

void widget::set_location(int x, int y)
{
	SDL_Rect rect = { x, y, rect_.w, rect_.h };
	set_location(rect);
}

void widget::set_width(unsigned w)
{
	SDL_Rect rect = { rect_.x, rect_.y, w, rect_.h };
	set_location(rect);
}

void widget::set_height(unsigned h)
{
	SDL_Rect rect = { rect_.x, rect_.y, rect_.w, h };
	set_location(rect);
}

void widget::set_measurements(unsigned w, unsigned h)
{
	SDL_Rect rect = { rect_.x, rect_.y, w, h };
	set_location(rect);
}

unsigned widget::width() const
{
	return rect_.w;
}

unsigned widget::height() const
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
		|| (clip_ && !rects_overlap(clip_rect_, rect_)));
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
	util::scoped_ptr<clip_rect_setter> clipper(NULL);
	if (clip_)
		clipper.assign(new clip_rect_setter(video().getSurface(), clip_rect_));

	if (needs_restore_) {
		for(std::vector< surface_restorer >::const_iterator i = restorer_.begin(),
		    i_end = restorer_.end(); i != i_end; ++i)
			i->restore();
		needs_restore_ = false;
	} else {
		//this function should be able to be relied upon to update the rectangle,
		//so do that even if we don't restore
		update_rect(rect_);
	}
}

void widget::bg_restore(SDL_Rect const &rect) const
{
	util::scoped_ptr<clip_rect_setter> clipper(NULL);
	if (clip_)
		clipper.assign(new clip_rect_setter(video().getSurface(), clip_rect_));

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

	util::scoped_ptr<clip_rect_setter> clipper(NULL);
	if (clip_)
		clipper.assign(new clip_rect_setter(video().getSurface(), clip_rect_));

	draw_contents();

	update_rect(rect_);
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

void widget::process_help_string(int mousex, int mousey)
{
	if (!hidden() && point_in_rect(mousex, mousey, rect_)) {
		if(help_string_ == 0 && help_text_ != "") {
			//std::cerr << "setting help string to '" << help_text_ << "'\n";
			help_string_ = video().set_help_string(help_text_);
		}
	} else if(help_string_ > 0) {
		video().clear_help_string(help_string_);
		help_string_ = 0;
	}
}

}
