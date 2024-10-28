/*
	Copyright (C) 2004 - 2024
	by Guillaume Melquiond <guillaume.melquiond@gmail.com>
	Copyright (C) 2003 by David White <dave@whitevine.net>
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
#include "log.hpp"
#include "widgets/scrollbar.hpp"
#include "picture.hpp"
#include "sdl/input.hpp" // get_mouse_state
#include "sdl/rect.hpp"
#include "sdl/texture.hpp"
#include "sdl/utils.hpp"

namespace {

const std::string scrollbar_top = "buttons/scrollbars/scrolltop.png";
const std::string scrollbar_bottom = "buttons/scrollbars/scrollbottom.png";
const std::string scrollbar_mid = "buttons/scrollbars/scrollmid.png";

const std::string scrollbar_top_hl = "buttons/scrollbars/scrolltop-active.png";
const std::string scrollbar_bottom_hl = "buttons/scrollbars/scrollbottom-active.png";
const std::string scrollbar_mid_hl = "buttons/scrollbars/scrollmid-active.png";

const std::string scrollbar_top_pressed = "buttons/scrollbars/scrolltop-pressed.png";
const std::string scrollbar_bottom_pressed = "buttons/scrollbars/scrollbottom-pressed.png";
const std::string scrollbar_mid_pressed = "buttons/scrollbars/scrollmid-pressed.png";

}

namespace gui {

scrollbar::scrollbar()
	: widget()
	, state_(NORMAL)
	, minimum_grip_height_(0)
	, mousey_on_grip_(0)
	, grip_position_(0)
	, grip_height_(0)
	, full_height_(0)
	, scroll_rate_(1)
{
	const point img_size(image::get_size(scrollbar_mid));

	if (img_size.x && img_size.y) {
		set_width(img_size.x);
		// this is a bit rough maybe
		minimum_grip_height_ = 2 * img_size.y;
	}
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
	queue_redraw();
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
	queue_redraw();
}

void scrollbar::set_full_size(unsigned h)
{
	if (h == full_height_)
		return;
	bool at_bottom = get_position() == get_max_position() && get_max_position() > 0;
	full_height_ = h;
	if (at_bottom)
		grip_position_ = get_max_position();
	set_shown_size(grip_height_);
	set_position(grip_position_);
	queue_redraw();
}

void scrollbar::set_scroll_rate(unsigned r)
{
	scroll_rate_ = r;
}

void scrollbar::scroll_down()
{
	move_position(scroll_rate_);
}

void scrollbar::scroll_up()
{
	move_position(-scroll_rate_);
}

SDL_Rect scrollbar::grip_area() const
{
	const SDL_Rect& loc = location();
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
	texture mid_img;
	texture bot_img;
	texture top_img;

	switch (state_) {

	case NORMAL:
		top_img = image::get_texture(scrollbar_top);
		mid_img = image::get_texture(scrollbar_mid);
		bot_img = image::get_texture(scrollbar_bottom);
		break;

	case ACTIVE:
		top_img = image::get_texture(scrollbar_top_hl);
		mid_img = image::get_texture(scrollbar_mid_hl);
		bot_img = image::get_texture(scrollbar_bottom_hl);
		break;

	case DRAGGED:
		top_img = image::get_texture(scrollbar_top_pressed);
		mid_img = image::get_texture(scrollbar_mid_pressed);
		bot_img = image::get_texture(scrollbar_bottom_pressed);
		break;

	case UNINIT:
	default:
		break;
	}

	SDL_Rect grip = grip_area();

	int mid_height = grip.h - top_img.h() - bot_img.h();
	if (mid_height <= 0) {
		// For now, minimum size of the middle piece is 1.
		// This should never really be encountered, and if it is,
		// it's a symptom of a larger problem, I think.
		mid_height = 1;
	}

	SDL_Rect groove = location();

	if (grip.h > groove.h) {
		PLAIN_LOG << "abort draw scrollbar: grip too large";
		return;
	}

	// Draw scrollbar "groove"
	const color_t c{0, 0, 0, uint8_t(255 * 0.35)};
	draw::fill(groove, c);

	// Draw scrollbar "grip"
	SDL_Rect dest{grip.x, grip.y, top_img.w(), top_img.h()};
	draw::blit(top_img, dest);

	dest = {dest.x, dest.y + top_img.h(), mid_img.w(), mid_height};
	draw::blit(mid_img, dest);

	dest = {dest.x, dest.y + mid_height, bot_img.w(), bot_img.h()};
	draw::blit(bot_img, dest);
}

void scrollbar::handle_event(const SDL_Event& event)
{
	gui::widget::handle_event(event);

	if (mouse_locked() || hidden())
		return;

	STATE new_state = state_;
	const rect grip = grip_area();
	const rect& groove = location();


	switch (event.type) {
	case SDL_MOUSEBUTTONUP:
	{
		const SDL_MouseButtonEvent& e = event.button;
		bool on_grip = grip.contains(e.x, e.y);
		new_state = on_grip ? ACTIVE : NORMAL;
		break;
	}
	case SDL_MOUSEBUTTONDOWN:
	{
		const SDL_MouseButtonEvent& e = event.button;
		bool on_grip = grip.contains(e.x, e.y);
		bool on_groove = groove.contains(e.x, e.y);
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
		const SDL_MouseMotionEvent& e = event.motion;
		if (state_ == NORMAL || state_ == ACTIVE) {
			bool on_grip = grip.contains(e.x, e.y);
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
		bool on_groove = groove.contains(sdl::get_mouse_location());
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
		state_ = new_state;
		queue_redraw();
	}
}

} // end namespace gui
