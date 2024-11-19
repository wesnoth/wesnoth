/*
	Copyright (C) 2013 - 2024
	by Fabian Mueller <fabianmueller5@gmx.de>
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

#include "editor/palette/tristate_button.hpp"

#include "draw.hpp"
#include "game_config.hpp"
#include "picture.hpp"
#include "log.hpp"
#include "sdl/rect.hpp"
#include "sound.hpp"

static lg::log_domain log_display("display");
#define ERR_DP LOG_STREAM(err, log_display)

namespace gui {

tristate_button::tristate_button(
		editor::tristate_palette* palette,
		std::string button_image_name,
		const bool auto_join)
	: widget(auto_join)
	, baseImage_()
	, touchedBaseImage_()
	, activeBaseImage_()
	, itemBaseImage_()
	, itemOverlayImage_()
	, pressedDownImage_()
	, pressedUpImage_()
	, pressedBothImage_()
	, pressedBothActiveImage_()
	, pressedDownActiveImage_()
	, pressedUpActiveImage_()
	, touchedDownImage_()
	, touchedUpImage_()
	, touchedBothImage_()
	, textRect_()
	, state_(NORMAL)
	, pressed_(false)
	, base_height_(0)
	, base_width_(0)
	, palette_(palette)
	, item_id_()
{
	// TODO: highdpi - there seem to be higher-quality 74-px versions of these available, but as that's not actually an integer multiple of 38... this is not a straight swap

	if (button_image_name.empty()) {
		button_image_name = "buttons/button_selectable/button_selectable_38_";
	}

	baseImage_ =
		image::get_texture(button_image_name + "base.png");
	activeBaseImage_ =
		image::get_texture(button_image_name + "base-active.png");
	touchedBaseImage_ =
		image::get_texture(button_image_name + "base-touched.png");

	touchedBothImage_ =
		image::get_texture(button_image_name + "border-touched-both.png");
	touchedUpImage_ =
		image::get_texture(button_image_name + "border-touched-up.png");
	touchedDownImage_ =
		image::get_texture(button_image_name + "border-touched-down.png");

	pressedUpImage_ =
		image::get_texture(button_image_name + "border-pressed-up.png");
	pressedDownImage_ =
		image::get_texture(button_image_name + "border-pressed-down.png");
	pressedBothImage_ =
		image::get_texture(button_image_name + "border-pressed-both.png");

	pressedUpActiveImage_ =
		image::get_texture(button_image_name + "border-active-pressed-up.png");
	pressedDownActiveImage_ =
		image::get_texture(button_image_name + "border-active-pressed-down.png");
	pressedBothActiveImage_ =
		image::get_texture(button_image_name + "border-active-pressed-both.png");

	//TODO
//	if (button_image.null()) {
//		ERR_DP<< "error initializing button!";
//		throw error();
//	}

	// TODO: highdpi - set this some better way
	base_height_ = baseImage_.h();
	base_width_  = baseImage_.w();

}

tristate_button::~tristate_button() {}

void tristate_button::set_pressed(PRESSED_STATE new_pressed_state) {

	STATE new_state = state_;

	switch (new_pressed_state) {
		case LEFT:
			new_state = (state_ == PRESSED_ACTIVE_LEFT) ? PRESSED_ACTIVE_LEFT : PRESSED_LEFT;
			break;
		case RIGHT:
			new_state = (state_ == PRESSED_ACTIVE_RIGHT) ? PRESSED_ACTIVE_RIGHT : PRESSED_RIGHT;
			break;
		case BOTH:
			new_state = (state_ == PRESSED_ACTIVE_BOTH) ? PRESSED_ACTIVE_BOTH : PRESSED_BOTH;
			break;
		case NONE:
			new_state = NORMAL;
	}

	if (state_ != new_state) {
		state_ = new_state;
		set_dirty();
	}
}

//TODO
//void tristate_button::set_active(bool active) {
//	if ((state_ == NORMAL) && active) {
//		state_ = ACTIVE;
//		set_dirty();
//	} else if ((state_ == ACTIVE) && !active) {
//		state_ = NORMAL;
//		set_dirty();
//	}
//}

tristate_button::PRESSED_STATE tristate_button::pressed_state() const {

	switch (state_) {
		case PRESSED_LEFT:
		case PRESSED_ACTIVE_LEFT:
		case TOUCHED_BOTH_LEFT:
			return LEFT;
		case PRESSED_RIGHT:
		case PRESSED_ACTIVE_RIGHT:
		case TOUCHED_BOTH_RIGHT:
			return RIGHT;
		case PRESSED_BOTH:
		case PRESSED_ACTIVE_BOTH:
			return BOTH;
		default:
			return NONE;
	}
}

void tristate_button::enable(bool new_val) {
	if (new_val != enabled()) {
		pressed_ = false;
		// check buttons should keep their state
		state_ = NORMAL;

		widget::enable(new_val);
	}
}

void tristate_button::draw_contents()
{
	texture overlay;
	texture base = baseImage_;

	switch (state_) {

	case UNINIT:
		return;

	case NORMAL:
		break;

	case TOUCHED_LEFT:
		overlay = touchedUpImage_;
		base = touchedBaseImage_;
		break;
	case TOUCHED_RIGHT:
		overlay = touchedDownImage_;
		base = touchedBaseImage_;
		break;
	case TOUCHED_BOTH_LEFT:
	case TOUCHED_BOTH_RIGHT:
		overlay = touchedBothImage_;
		base = touchedBaseImage_;
		break;
	case ACTIVE:
		//	overlay = activeImage_;
		base = activeBaseImage_;
		break;
	case PRESSED_LEFT:
		base = activeBaseImage_;
		overlay = pressedUpImage_;
		break;
	case PRESSED_RIGHT:
		base = activeBaseImage_;
		overlay = pressedDownImage_;
		break;
	case PRESSED_BOTH:
		base = activeBaseImage_;
		overlay = pressedBothImage_;
		break;
	case PRESSED_ACTIVE_LEFT:
		overlay = pressedUpActiveImage_;
		base = activeBaseImage_;
		break;
	case PRESSED_ACTIVE_BOTH:
		overlay = pressedBothActiveImage_;
		base = activeBaseImage_;
		break;
	case PRESSED_ACTIVE_RIGHT:
		overlay = pressedDownActiveImage_;
		base = activeBaseImage_;
		break;
	}

	// Draw the button base.
	const SDL_Rect& loc = location();
	draw::blit(base, loc);

	// Draw the item.
	// TODO: don't hardcode an implicit reliance on 38 pixel buttons
	SDL_Rect magic{loc.x + 1, loc.y + 1, 36, 36};
	draw::blit(itemBaseImage_, magic);
	if (itemOverlayImage_) {
		draw::blit(itemOverlayImage_, magic);
	}

	// Draw the button overlay, if any.
	if (overlay) {
		draw::blit(overlay, loc);
	}
}

//TODO move to widget
bool tristate_button::hit(int x, int y) const {
	return location().contains(x, y);
}

void tristate_button::mouse_motion(const SDL_MouseMotionEvent& event) {

	if (hit(event.x, event.y))
	{ // the cursor is over the widget

		switch (state_) {

		case UNINIT:
			return;

		case NORMAL:
			state_ = ACTIVE;
			break;
		case PRESSED_LEFT:
			state_ = PRESSED_ACTIVE_LEFT;
			break;
		case PRESSED_RIGHT:
			state_ = PRESSED_ACTIVE_RIGHT;
			break;
		case PRESSED_BOTH:
			state_ = PRESSED_ACTIVE_BOTH;
			break;
		default:
			//assert(false);
			break;
		}

	} else { // the cursor is not over the widget

		switch (state_) {

		case ACTIVE:
			state_ = NORMAL;
			break;
		case TOUCHED_LEFT:
		case TOUCHED_RIGHT:
			state_ = ACTIVE;
			break;
		case PRESSED_ACTIVE_RIGHT:
		case TOUCHED_BOTH_RIGHT:
			state_ = PRESSED_RIGHT;
			break;
		case PRESSED_ACTIVE_BOTH:
			state_ = PRESSED_BOTH;
			break;
		case PRESSED_ACTIVE_LEFT:
		case TOUCHED_BOTH_LEFT:
			state_ = PRESSED_LEFT;
			break;
		default:
			break;
		}
	}
}

void tristate_button::mouse_down(const SDL_MouseButtonEvent& event) {

	if (!hit(event.x, event.y))
		return;

	//The widget is expected to be in one of the "active" states when the mouse cursor is hovering over it, but that currently doesn't happen if the widget is moved under the cursor by scrolling the palette.
	if (event.button == SDL_BUTTON_RIGHT) {
		if (state_ == ACTIVE || state_ == NORMAL)
			state_ = TOUCHED_RIGHT;
		if (state_ == PRESSED_ACTIVE_LEFT || state_ == PRESSED_LEFT)
			state_ = TOUCHED_BOTH_LEFT;
	}

	if (event.button == SDL_BUTTON_LEFT) {
		if (state_ == ACTIVE || state_ == NORMAL)
			state_ = TOUCHED_LEFT;
		if (state_ == PRESSED_ACTIVE_RIGHT || state_ == PRESSED_RIGHT)
			state_ = TOUCHED_BOTH_RIGHT;
	}

}

void tristate_button::release() {
	state_ = NORMAL;
	draw_contents();
}

void tristate_button::mouse_up(const SDL_MouseButtonEvent& event) {

	if (!(hit(event.x, event.y)))
		return;

	// the user has stopped pressing the mouse left button while on the widget
	if (event.button == SDL_BUTTON_LEFT) {

		if (state_ == TOUCHED_LEFT) {
			state_ = PRESSED_ACTIVE_LEFT;
			palette_->select_fg_item(item_id_);
			//TODO
		//	palette_->draw(true);
			pressed_ = true;
		}
		if (state_ == TOUCHED_BOTH_RIGHT) {
			state_ = PRESSED_ACTIVE_BOTH;
			palette_->select_fg_item(item_id_);
		//	palette_->select_bg_item(item_id_);
		//	palette_->draw(true);
			pressed_ = true;
		}
	}

	if (event.button == SDL_BUTTON_RIGHT) {

		pressed_ = true;
		palette_->select_bg_item(item_id_);

		if (state_ == TOUCHED_RIGHT) {
			state_ = PRESSED_ACTIVE_RIGHT;
		//	palette_->select_bg_item(item_id_);
		//	palette_->draw(true);
		//	pressed_ = true;
		}
		if (state_ == TOUCHED_BOTH_LEFT) {
			state_ = PRESSED_ACTIVE_BOTH;
		//	palette_->select_fg_item(item_id_);
		//	palette_->select_bg_item(item_id_);
		//	palette_->draw(true);
		//	pressed_ = true;
		}
	}

	if (pressed_)
		sound::play_UI_sound(game_config::sounds::checkbox_release);
}

void tristate_button::handle_event(const SDL_Event& event) {

	gui::widget::handle_event(event);

	if (hidden() || !enabled())
		return;

	STATE start_state = state_;

	if (!mouse_locked()) {
		switch (event.type) {
		case SDL_MOUSEBUTTONDOWN:
			mouse_down(event.button);
			break;
		case SDL_MOUSEBUTTONUP:
			mouse_up(event.button);
			break;
		case SDL_MOUSEMOTION:
			mouse_motion(event.motion);
			break;
		default:
			return;
		}
	}

	if (start_state != state_)
		set_dirty(true);
}

bool tristate_button::pressed() {
	const bool res = pressed_;
	pressed_ = false;
	return res;
}

}
