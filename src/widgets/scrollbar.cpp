/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
                 2004 - 2015 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file */

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "widgets/scrollbar.hpp"
#include "image.hpp"
#include "sdl/rect.hpp"
#include "video.hpp"

#include <iostream>

namespace {
	const std::string scrollbar_top = "buttons/scrollbars_large/scrolltop.png";
	const std::string scrollbar_bottom = "buttons/scrollbars_large/scrollbottom.png";
	const std::string scrollbar_mid = "buttons/scrollbars_large/scrollmid.png";

	const std::string scrollbar_top_hl = "buttons/scrollbars_large/scrolltop-active.png";
	const std::string scrollbar_bottom_hl = "buttons/scrollbars_large/scrollbottom-active.png";
	const std::string scrollbar_mid_hl = "buttons/scrollbars_large/scrollmid-active.png";

	const std::string scrollbar_top_pressed = "buttons/scrollbars_large/scrolltop-pressed.png";
	const std::string scrollbar_bottom_pressed = "buttons/scrollbars_large/scrollbottom-pressed.png";
	const std::string scrollbar_mid_pressed = "buttons/scrollbars_large/scrollmid-pressed.png";

	const std::string groove_top = "buttons/scrollbars_large/scrollgroove-top.png";
	const std::string groove_mid = "buttons/scrollbars_large/scrollgroove-mid.png";
	const std::string groove_bottom = "buttons/scrollbars_large/scrollgroove-bottom.png";

}

namespace gui {

scrollbar::scrollbar(CVideo &video)
	: widget(video)
	, mid_scaled_(nullptr)
	, groove_scaled_(nullptr)
	, uparrow_(video, "", button::TYPE_TURBO, "button_square/button_square_25"
			, gui::button::DEFAULT_SPACE, true,"icons/arrows/arrows_ornate_up_25")
	, downarrow_(video, "", button::TYPE_TURBO, "button_square/button_square_25"
			, gui::button::DEFAULT_SPACE, true,"icons/arrows/arrows_ornate_down_25")
	, state_(NORMAL)
	, minimum_grip_height_(0)
	, mousey_on_grip_(0)
	, grip_position_(0)
	, grip_height_(0)
	, full_height_(0)
	, scroll_rate_(1)
{
	uparrow_.enable(false);
	downarrow_.enable(false);

	static const surface img(image::get_image(scrollbar_mid));

	if (img != nullptr) {
		set_width(img->w);
		// this is a bit rough maybe
		minimum_grip_height_ = 2 * img->h;
	}
}

sdl_handler_vector scrollbar::handler_members()
{
	sdl_handler_vector h;
	h.push_back(&uparrow_);
	h.push_back(&downarrow_);
	return h;
}

void scrollbar::update_location(SDL_Rect const &rect)
{
	int uh = uparrow_.height(), dh = downarrow_.height();
	uparrow_.set_location(rect.x, rect.y);
	downarrow_.set_location(rect.x, rect.y + rect.h - dh);
	SDL_Rect r = rect;
	r.y += uh;
	r.h -= uh + dh;

	widget::update_location(r);
	//TODO comment or remove
	//bg_register(r);
}

void scrollbar::hide(bool value)
{
	widget::hide(value);
	uparrow_.hide(value);
	downarrow_.hide(value);
}

unsigned scrollbar::get_position() const
{
	return grip_position_;
}

unsigned scrollbar::get_max_position() const
{
	return full_height_ - grip_height_;
}

void scrollbar::set_position(unsigned pos)
{
	if (pos > full_height_ - grip_height_)
		pos = full_height_ - grip_height_;
	if (pos == grip_position_)
		return;
	grip_position_ = pos;
	uparrow_.enable(grip_position_ != 0);
	downarrow_.enable(grip_position_ < full_height_ - grip_height_);
	set_dirty();
}

void scrollbar::adjust_position(unsigned pos)
{
	if (pos < grip_position_)
		set_position(pos);
	else if (pos >= grip_position_ + grip_height_)
		set_position(pos - (grip_height_ - 1));
}

void scrollbar::move_position(int dep)
{
	int pos = grip_position_ + dep;
	if (pos > 0)
		set_position(pos);
	else
		set_position(0);
}

void scrollbar::set_shown_size(unsigned h)
{
	if (h > full_height_)
		h = full_height_;
	if (h == grip_height_)
		return;
	bool at_bottom = get_position() == get_max_position() && get_max_position() > 0;
	grip_height_ = h;
	if (at_bottom)
		grip_position_ = get_max_position();
	set_position(grip_position_);
	set_dirty(true);
}

void scrollbar::set_full_size(unsigned h)
{
	if (h == full_height_)
		return;
	bool at_bottom = get_position() == get_max_position() && get_max_position() > 0;
	full_height_ = h;
	if (at_bottom)
		grip_position_ = get_max_position();
	downarrow_.enable(grip_position_ < full_height_ - grip_height_);
	set_shown_size(grip_height_);
	set_position(grip_position_);
	set_dirty(true);
}

void scrollbar::set_scroll_rate(unsigned r)
{
	scroll_rate_ = r;
}

bool scrollbar::is_valid_height(int height) const
{
	int uh = uparrow_.height();
	int dh = downarrow_.height();
	if(uh + dh >= height) {
		return false;
	} else {
		return true;
	}
}

void scrollbar::scroll_down()
{
	move_position(scroll_rate_);
}

void scrollbar::scroll_up()
{
	move_position(-scroll_rate_);
}

void scrollbar::process_event()
{
	if (uparrow_.pressed())
		scroll_up();

	if (downarrow_.pressed())
		scroll_down();
}

SDL_Rect scrollbar::groove_area() const
{
	SDL_Rect loc = location();
	int uh = uparrow_.height();
	int dh = downarrow_.height();
	if(uh + dh >= loc.h) {
		loc.h = 0;
	} else {
		loc.y += uh;
		loc.h -= uh + dh;
	}
	return loc;
}

SDL_Rect scrollbar::grip_area() const
{
	SDL_Rect const &loc = groove_area();
	if (full_height_ == grip_height_)
		return loc;
	int h = static_cast<int>(loc.h) * grip_height_ / full_height_;
	if (h < minimum_grip_height_)
		h = minimum_grip_height_;
	int y = loc.y + (static_cast<int>(loc.h) - h) * grip_position_ / (full_height_ - grip_height_);
	return {loc.x, y, loc.w, h};
}

void scrollbar::draw_contents()
{
	surface mid_img;
	surface bottom_img;
	surface top_img;

	switch (state_) {

	case NORMAL:
		top_img.assign(image::get_image(scrollbar_top));
		mid_img.assign(image::get_image(scrollbar_mid));
		bottom_img.assign(image::get_image(scrollbar_bottom));
		break;

	case ACTIVE:
		top_img.assign(image::get_image(scrollbar_top_hl));
		mid_img.assign(image::get_image(scrollbar_mid_hl));
		bottom_img.assign(image::get_image(scrollbar_bottom_hl));
		break;

	case DRAGGED:
		top_img.assign(image::get_image(scrollbar_top_pressed));
		mid_img.assign(image::get_image(scrollbar_mid_pressed));
		bottom_img.assign(image::get_image(scrollbar_bottom_pressed));
		break;

	case UNINIT:
	default:
		break;
	}

	const surface top_grv(image::get_image(groove_top));
	const surface mid_grv(image::get_image(groove_mid));
	const surface bottom_grv(image::get_image(groove_bottom));

	if (mid_img == nullptr || bottom_img == nullptr || top_img == nullptr
	 || top_grv == nullptr || bottom_grv == nullptr || mid_grv == nullptr) {
		std::cerr << "Failure to load scrollbar image.\n";
		return;
	}

	SDL_Rect grip = grip_area();
	int mid_height = grip.h - top_img->h - bottom_img->h;
	if (mid_height <= 0) {
		// For now, minimum size of the middle piece is 1.
		// This should never really be encountered, and if it is,
		// it's a symptom of a larger problem, I think.
		mid_height = 1;
	}

	if(mid_scaled_.null() || mid_scaled_->h != mid_height) {
		mid_scaled_.assign(scale_surface(mid_img, mid_img->w, mid_height));
	}

	SDL_Rect groove = groove_area();
	int groove_height = groove.h - top_grv->h - bottom_grv->h;
	if (groove_height <= 0) {
		groove_height = 1;
	}

	if (groove_scaled_.null() || groove_scaled_->h != groove_height) {
		groove_scaled_.assign(scale_surface(mid_grv, mid_grv->w, groove_height));
	}

	if (mid_scaled_.null() || groove_scaled_.null()) {
		std::cerr << "Failure during scrollbar image scale.\n";
		return;
	}

	if (grip.h > groove.h) {
		std::cerr << "abort draw scrollbar: grip too large\n";
		return;
	}

	// Draw scrollbar "groove"
	video().blit_surface(groove.x, groove.y, top_grv);
	video().blit_surface(groove.x, groove.y + top_grv->h, groove_scaled_);
	video().blit_surface(groove.x, groove.y + top_grv->h + groove_height, bottom_grv);

	// Draw scrollbar "grip"
	video().blit_surface(grip.x, grip.y, top_img);
	video().blit_surface(grip.x, grip.y + top_img->h, mid_scaled_);
	video().blit_surface(grip.x, grip.y + top_img->h + mid_height, bottom_img);
}

void scrollbar::handle_event(const SDL_Event& event)
{
	gui::widget::handle_event(event);

	if (mouse_locked() || hidden())
		return;

	STATE new_state = state_;
	SDL_Rect const &grip = grip_area();
	SDL_Rect const &groove = groove_area();


	switch (event.type) {
	case SDL_MOUSEBUTTONUP:
	{
		SDL_MouseButtonEvent const &e = event.button;
		bool on_grip = sdl::point_in_rect(e.x, e.y, grip);
		new_state = on_grip ? ACTIVE : NORMAL;
		break;
	}
	case SDL_MOUSEBUTTONDOWN:
	{
		SDL_MouseButtonEvent const &e = event.button;
		bool on_grip = sdl::point_in_rect(e.x, e.y, grip);
		bool on_groove = sdl::point_in_rect(e.x, e.y, groove);
		if (on_grip && e.button == SDL_BUTTON_LEFT) {
			mousey_on_grip_ = e.y - grip.y;
			new_state = DRAGGED;
		} else if (on_groove && e.button == SDL_BUTTON_LEFT && groove.h != grip.h) {
			if (e.y < grip.y)
				move_position(-static_cast<int>(grip_height_));
			else
				move_position(grip_height_);
		} else if (on_groove && e.button == SDL_BUTTON_MIDDLE && groove.h != grip.h) {
			int y_dep = e.y - grip.y - grip.h/2;
			int dep = y_dep * int(full_height_ - grip_height_)/ (groove.h - grip.h);
			move_position(dep);
		}
		break;
	}
	case SDL_MOUSEMOTION:
	{
		SDL_MouseMotionEvent const &e = event.motion;
		if (state_ == NORMAL || state_ == ACTIVE) {
			bool on_grip = sdl::point_in_rect(e.x, e.y, grip);
			new_state = on_grip ? ACTIVE : NORMAL;
		} else if (state_ == DRAGGED && groove.h != grip.h) {
			int y_dep = e.y - grip.y - mousey_on_grip_;
			int dep = y_dep * static_cast<int>(full_height_ - grip_height_) / (groove.h - grip.h);
			move_position(dep);
		}
		break;
	}
	case SDL_MOUSEWHEEL:
	{
		const SDL_MouseWheelEvent& e = event.wheel;
		int x, y;
		SDL_GetMouseState(&x, &y);
		bool on_groove = sdl::point_in_rect(x, y, groove);
		if (on_groove && e.y < 0) {
			move_position(scroll_rate_);
		} else if (on_groove && e.y > 0) {
			move_position(-scroll_rate_);
		}
		break;
	}
	default:
		break;
	}


	if (new_state != state_) {
		set_dirty();
		mid_scaled_.assign(nullptr);
		state_ = new_state;
	}
}

} // end namespace gui

