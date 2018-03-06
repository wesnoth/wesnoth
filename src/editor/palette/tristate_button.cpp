/*
   Copyright (C) 2013 - 2018 by Fabian Mueller <fabianmueller5@gmx.de>
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

#include "editor/palette/tristate_button.hpp"

#include "font/constants.hpp"
#include "font/sdl_ttf.hpp"
#include "font/standard_colors.hpp"
#include "game_config.hpp"
#include "image.hpp"
#include "log.hpp"
#include "font/marked-up_text.hpp"
#include "sdl/rect.hpp"
#include "sound.hpp"
#include "video.hpp"

static lg::log_domain log_display("display");
#define ERR_DP LOG_STREAM(err, log_display)

namespace gui {

tristate_button::tristate_button(CVideo& video,
		editor::tristate_palette* palette,
		std::string button_image_name,
		const bool auto_join) :
				widget(video, auto_join),
				baseImage_(nullptr), touchedBaseImage_(nullptr), activeBaseImage_(nullptr),
				itemImage_(nullptr),
				pressedDownImage_(nullptr), pressedUpImage_(nullptr), pressedBothImage_(nullptr),
				pressedBothActiveImage_(nullptr), pressedDownActiveImage_(nullptr), pressedUpActiveImage_(nullptr),
				touchedDownImage_(nullptr), touchedUpImage_(nullptr), touchedBothImage_(nullptr),
				textRect_(),
				state_(NORMAL), pressed_(false),
				base_height_(0), base_width_(0),
				palette_(palette), item_id_()
{

	if (button_image_name.empty()) {
		button_image_name = "buttons/button_selectable/button_selectable_38_";
	}

	baseImage_.assign(
			image::get_image(button_image_name + "base.png"));
	activeBaseImage_.assign(
			image::get_image(button_image_name + "base-active.png"));
	touchedBaseImage_.assign(
			image::get_image(button_image_name + "base-touched.png"));

	touchedBothImage_.assign(
			image::get_image(button_image_name + "border-touched-both.png"));
	touchedUpImage_.assign(
			image::get_image(button_image_name + "border-touched-up.png"));
	touchedDownImage_.assign(
			image::get_image(button_image_name + "border-touched-down.png"));

	pressedUpImage_.assign(
			image::get_image(button_image_name + "border-pressed-up.png"));
	pressedDownImage_.assign(
			image::get_image(button_image_name + "border-pressed-down.png"));
	pressedBothImage_.assign(
			image::get_image(button_image_name + "border-pressed-both.png"));

	pressedUpActiveImage_.assign(
			image::get_image(button_image_name + "border-active-pressed-up.png"));
	pressedDownActiveImage_.assign(
			image::get_image(button_image_name + "border-active-pressed-down.png"));
	pressedBothActiveImage_.assign(
			image::get_image(button_image_name + "border-active-pressed-both.png"));

	//TODO
//	if (button_image.null()) {
//		ERR_DP<< "error initializing button!" << std::endl;
//		throw error();
//	}

	base_height_ = baseImage_->h;
	base_width_  = baseImage_->w;

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

void tristate_button::draw_contents() {

	surface image(nullptr);

	surface overlay(nullptr);
	surface base = baseImage_;

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

	image = base;

	const SDL_Rect& loc = location();

	surface scalled_item;
	scalled_item.assign(scale_surface(itemImage_,
			36, 36));

	surface nitem = make_neutral_surface(scalled_item);
	surface nbase = make_neutral_surface(base);

	//TODO avoid magic numbers
	SDL_Rect r {1, 1, 0, 0};
	sdl_blit(nitem, nullptr, nbase, &r);

	if (!overlay.null()) {
		surface noverlay = make_neutral_surface(overlay);
		sdl_blit(noverlay, nullptr, nbase, nullptr);
	}

	bg_restore();

	image = nbase;
	video().blit_surface(loc.x, loc.y, image);
}

//TODO move to widget
bool tristate_button::hit(int x, int y) const {
	return sdl::point_in_rect(x, y, location());
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

	if (event.button == SDL_BUTTON_RIGHT) {
		if (state_ == ACTIVE)
			state_ = TOUCHED_RIGHT;
		if (state_ == PRESSED_ACTIVE_LEFT)
			state_ = TOUCHED_BOTH_LEFT;
	}

	if (event.button == SDL_BUTTON_LEFT) {
		if (state_ == ACTIVE)
			state_ = TOUCHED_LEFT;
		if (state_ == PRESSED_ACTIVE_RIGHT)
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
