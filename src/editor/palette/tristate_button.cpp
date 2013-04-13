/*
   Copyright (C) 2003 - 2013 by David White <dave@whitevine.net>
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

#include "global.hpp"

#include "tristate_button.hpp"

#include "font.hpp"
#include "game_config.hpp"
#include "image.hpp"
#include "log.hpp"
#include "marked-up_text.hpp"
#include "serialization/string_utils.hpp"
#include "sound.hpp"
#include "video.hpp"
#include "wml_separators.hpp"

static lg::log_domain log_display("display");
#define ERR_DP LOG_STREAM(err, log_display)

namespace gui {

const int font_size = font::SIZE_SMALL;
const int horizontal_padding = font::SIZE_SMALL;
const int checkbox_horizontal_padding = font::SIZE_SMALL / 2;
const int vertical_padding = font::SIZE_SMALL / 2;

tristate_button::tristate_button(CVideo& video, const std::string& label,
		editor::common_palette* palette,
		std::string button_image_name, SPACE_CONSUMPTION spacing,
		const bool auto_join) :
				widget(video, auto_join)
				, label_(label),

				baseImage_(NULL), touchedBaseImage_(NULL), activeBaseImage_(NULL),

				itemImage_(NULL),

//				normalImage_(NULL), activeImage_(NULL),
				pressedDownImage_(NULL), pressedUpImage_(NULL), pressedBothImage_(NULL),
				pressedBothActiveImage_(NULL), pressedDownActiveImage_(NULL), pressedUpActiveImage_(NULL),
				touchedDownImage_(NULL), touchedUpImage_(NULL), touchedBothImage_(NULL),
//				disabledImage_(NULL), pressedDownDisabledImage_(NULL),
//				pressedUpDisabledImage_(NULL), pressedBothDisabledImage_(NULL),

				state_(NORMAL), pressed_(false),
				spacing_(spacing), base_height_(0), base_width_(0)
				, palette_(palette), item_id_()
{

	if (button_image_name.empty()) {
		button_image_name = "buttons/editor/palette/";
	}

	baseImage_.assign(
			image::get_image(button_image_name + "base.png"));
	activeBaseImage_.assign(
			image::get_image(button_image_name + "base-active.png"));
	touchedBaseImage_.assign(
			image::get_image(button_image_name + "base-touched.png"));

	touchedBothImage_.assign(
			image::get_image(button_image_name + "touched-both.png"));
	touchedUpImage_.assign(
			image::get_image(button_image_name + "touched-up.png"));
	touchedDownImage_.assign(
			image::get_image(button_image_name + "touched-down.png"));

	pressedUpImage_.assign(
			image::get_image(button_image_name + "pressed-up.png"));
	pressedDownImage_.assign(
			image::get_image(button_image_name + "pressed-down.png"));
	pressedBothImage_.assign(
			image::get_image(button_image_name + "pressed-both.png"));

	pressedUpActiveImage_.assign(
			image::get_image(button_image_name + "active-pressed-up.png"));
	pressedDownActiveImage_.assign(
			image::get_image(button_image_name + "active-pressed-down.png"));
	pressedBothActiveImage_.assign(
			image::get_image(button_image_name + "active-pressed-both.png"));

//	if (button_image.null()) {
//		ERR_DP<< "error initializing button!\n";
//		throw error();
//	}

	base_height_ = baseImage_->h;
	base_width_  = baseImage_->w;

//  calculate_size();

}

tristate_button::~tristate_button() {
}

void tristate_button::calculate_size() {

//  TODO
//	if (type_ == TYPE_IMAGE){
//		SDL_Rect loc_image = location();
//		loc_image.h = normalImage_->h;
//		loc_image.w = normalImage_->w;
//		set_location(loc_image);
//		return;
//	}

	SDL_Rect const &loc = location();
	bool change_size = loc.h == 0 || loc.w == 0;

	if (!change_size) {
		unsigned w = loc.w - (checkbox_horizontal_padding + base_width_);

		int fs = font_size;
		int style = TTF_STYLE_NORMAL;
		std::string::const_iterator i_beg = label_.begin(), i_end =
				label_.end(), i = font::parse_markup(i_beg, i_end, &fs, NULL,
						&style);
		if (i != i_end) {
			std::string tmp(i, i_end);
			label_.erase(i - i_beg, i_end - i_beg);
			label_ += font::make_text_ellipsis(tmp, fs, w, style);
		}
	}

	textRect_ = font::draw_text(NULL, screen_area(), font_size,
			font::BUTTON_COLOR, label_, 0, 0);

	if (!change_size)
		return;

	set_height(std::max(textRect_.h + vertical_padding, base_height_));

	if (label_.empty()) {
		set_width(base_width_);
	} else {
		set_width(checkbox_horizontal_padding + textRect_.w + base_width_);
	}

}

void tristate_button::set_pressed(PRESSED_STATE new_pressed_state) {

	STATE new_state = state_;

	switch (new_pressed_state) {
		case LEFT:
			new_state = PRESSED_LEFT;
			break;
		case RIGHT:
			new_state = PRESSED_RIGHT;
			break;
		case BOTH:
			new_state = PRESSED_BOTH;
			break;
		case NONE:
			new_state = NORMAL;
	}

//	if (check) {
//		new_state =
//				(state_ == ACTIVE || state_ == PRESSED_ACTIVE) ?
//						PRESSED_ACTIVE : PRESSED;
//	} else {
//		new_state =
//				(state_ == ACTIVE || state_ == PRESSED_ACTIVE) ?
//						ACTIVE : NORMAL;
//	}

	if (state_ != new_state) {
		state_ = new_state;
		set_dirty();
	}
}

//void tristate_button::set_active(bool active) {
//	if ((state_ == NORMAL) && active) {
//		state_ = ACTIVE;
//		set_dirty();
//	} else if ((state_ == ACTIVE) && !active) {
//		state_ = NORMAL;
//		set_dirty();
//	}
//}
//
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
			break;
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

	surface image(NULL);

	surface overlay(NULL);
	surface base = baseImage_;

	int offset = 0;
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
		overlay = pressedUpImage_;
		break;
	case PRESSED_RIGHT:
		overlay = pressedDownImage_;
		break;
	case PRESSED_BOTH:
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

	const int image_w = image->w;
	SDL_Rect const &loc = location();
	SDL_Rect clipArea = loc;
	const int texty = loc.y + loc.h / 2 - textRect_.h / 2 + offset;
	int textx;

	clipArea.w += image_w + checkbox_horizontal_padding;
	textx = loc.x + image_w + checkbox_horizontal_padding / 2;

	SDL_Color button_color = font::BUTTON_COLOR;

	// blit_surface want neutral surfaces
	surface nitem = make_neutral_surface(itemImage_);
	surface nbase = make_neutral_surface(base);

	//TODO avoid magic numbers
	SDL_Rect r = create_rect(1, 1, 0, 0);
	blit_surface(nitem, NULL, nbase, &r);

	if (!overlay.null()) {
		surface noverlay = make_neutral_surface(overlay);
		blit_surface(noverlay, NULL, nbase, NULL);
	}

//  TODO for later reference
//	SDL_SetAlpha(nbase, SDL_SRCALPHA, SDL_ALPHA_OPAQUE);
//	SDL_SetAlpha(image, 0, 0);
//
//	TODO might be needed.
	bg_restore();

	image = nbase;
	video().blit_surface(loc.x, loc.y, image);

	clipArea.x += offset;
	clipArea.y += offset;
	clipArea.w -= 2 * offset;
	clipArea.h -= 2 * offset;
	font::draw_text(&video(), clipArea, font_size, button_color, label_, textx,
			texty);

	update_rect(loc);
}

//TODO move to widget
bool tristate_button::hit(int x, int y) const {
	return point_in_rect(x, y, location());
}

static bool not_image(const std::string& str) {
	return !str.empty() && str[0] != IMAGE_PREFIX;
}

void tristate_button::set_label(const std::string& val) {
	label_ = val;

	//if we have a list of items, use the first one that isn't an image
	if (std::find(label_.begin(), label_.end(), COLUMN_SEPARATOR)
	!= label_.end()) {
		const std::vector<std::string>& items = utils::split(label_,
				COLUMN_SEPARATOR);
		const std::vector<std::string>::const_iterator i = std::find_if(
				items.begin(), items.end(), not_image);
		if (i != items.end()) {
			label_ = *i;
		}
	}

	calculate_size();
	set_dirty(true);
}

void tristate_button::mouse_motion(SDL_MouseMotionEvent const &event) {

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

void tristate_button::mouse_down(SDL_MouseButtonEvent const &event) {

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

void tristate_button::mouse_up(SDL_MouseButtonEvent const &event) {

	if (!(hit(event.x, event.y)))
		return;

	// the user has stopped pressing the mouse left button while on the widget
	if (event.button == SDL_BUTTON_LEFT) {

		if (state_ == TOUCHED_LEFT) {
			state_ = PRESSED_ACTIVE_LEFT;
			palette_->select_fg_item(item_id_);
			palette_->draw(true);
			pressed_ = true;
		}
		if (state_ == TOUCHED_BOTH_RIGHT) {
			state_ = PRESSED_ACTIVE_BOTH;
			palette_->select_fg_item(item_id_);
			palette_->select_bg_item(item_id_);
			palette_->draw(true);
			pressed_ = true;
		}
	}

	if (event.button == SDL_BUTTON_RIGHT) {

		if (state_ == TOUCHED_RIGHT) {
			state_ = PRESSED_ACTIVE_RIGHT;
			palette_->select_bg_item(item_id_);
			palette_->draw(true);
			pressed_ = true;
		}
		if (state_ == TOUCHED_BOTH_LEFT) {
			state_ = PRESSED_ACTIVE_BOTH;
			palette_->select_fg_item(item_id_);
			palette_->select_bg_item(item_id_);
			palette_->draw(true);
			pressed_ = true;
		}
	}

	if (pressed_)
		sound::play_UI_sound(game_config::sounds::checkbox_release);
}

void tristate_button::handle_event(const SDL_Event& event) {

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
