/*
	Copyright (C) 2003 - 2024
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

#include "widgets/button.hpp"

#include "draw.hpp"
#include "filesystem.hpp"
#include "game_config.hpp"
#include "picture.hpp"
#include "log.hpp"
#include "font/sdl_ttf_compat.hpp"
#include "font/standard_colors.hpp"
#include "sdl/rect.hpp"
#include "serialization/string_utils.hpp"
#include "sound.hpp"
#include "wml_separators.hpp"

#include <boost/algorithm/string/predicate.hpp>
#include <utility>

static lg::log_domain log_display("display");
#define ERR_DP LOG_STREAM(err, log_display)

namespace gui {

const int default_font_size = font::SIZE_BUTTON;

button::button(const std::string& label, button::TYPE type,
               const std::string& button_image_name, SPACE_CONSUMPTION spacing,
               const bool auto_join, std::string overlay_image, int font_size)
	: widget(auto_join), type_(type),
	  label_text_(label),
	  image_(nullptr), pressedImage_(nullptr), activeImage_(nullptr), pressedActiveImage_(nullptr),
	  disabledImage_(nullptr), pressedDisabledImage_(nullptr),
	  overlayImage_(nullptr), overlayPressedImage_(nullptr), overlayActiveImage_(nullptr),
	  state_(NORMAL), pressed_(false),
	  spacing_(spacing), base_height_(0), base_width_(0),
	  button_image_name_(), button_overlay_image_name_(std::move(overlay_image)),
	  button_image_path_suffix_(),
	  font_size_(font_size <= 0 ? (type != TYPE_CHECK && type != TYPE_RADIO ? default_font_size : font::SIZE_SMALL) : font_size),
	  horizontal_padding_(font_size_),
	  checkbox_horizontal_padding_(font_size_ / 2),
	  vertical_padding_(font_size_ / 2)
{
	if (button_image_name.empty()) {

		switch (type_) {
		case TYPE_PRESS:
			button_image_name_ = "buttons/button_normal/button_H22";
			break;
		case TYPE_TURBO:
			button_image_name_ = "buttons/button_menu/menu_button_copper_H20";
			break;
		case TYPE_CHECK:
			button_image_name_ = "buttons/checkbox";
			break;
		case TYPE_RADIO:
			button_image_name_ = "buttons/radiobox";
			break;
		default:
			break;
		}
	} else {
		button_image_name_ = "buttons/" + button_image_name;
	}

	load_images();
}

// This function is a mess and i can only hope it someday dies a horrible death
void button::load_images() {

	std::string size_postfix;

	switch (location().h) {
		case 25:
			size_postfix = "_25";
			break;
		case 30:
			size_postfix = "_30";
			break;
		case 60:
			size_postfix = "_60";
			break;
		default:
			break;
	}

	image_ = image::get_texture(
		button_image_name_ + ".png" + button_image_path_suffix_);
	pressedImage_ = image::get_texture(
		button_image_name_ + "-pressed.png" + button_image_path_suffix_);
	activeImage_ = image::get_texture(
		button_image_name_ + "-active.png" + button_image_path_suffix_);
	if (filesystem::file_exists(game_config::path + "/images/" + button_image_name_ + "-disabled.png")) {
		disabledImage_ = image::get_texture(
			button_image_name_ + "-disabled.png" + button_image_path_suffix_);
	} else {
		disabledImage_.reset();
	}

	if (!button_overlay_image_name_.empty()) {

		if (button_overlay_image_name_.length() > size_postfix.length() &&
				boost::algorithm::ends_with(button_overlay_image_name_, size_postfix)) {
			button_overlay_image_name_.resize(button_overlay_image_name_.length() - size_postfix.length());
		}

		overlayImage_ = image::get_texture(button_overlay_image_name_ + size_postfix + ".png"+ button_image_path_suffix_);
		overlayPressedImage_ = image::get_texture(button_overlay_image_name_ + size_postfix + "-pressed.png"+ button_image_path_suffix_);

		if (filesystem::file_exists(game_config::path + "/images/" + button_overlay_image_name_ + size_postfix + "-active.png"))
			overlayActiveImage_ = image::get_texture(button_overlay_image_name_ + size_postfix + "-active.png"+ button_image_path_suffix_);

		if (filesystem::file_exists(game_config::path + "/images/" + button_overlay_image_name_ + size_postfix + "-disabled.png"))
			overlayDisabledImage_ = image::get_texture(button_overlay_image_name_ + size_postfix + "-disabled.png"+ button_image_path_suffix_);
		if (!overlayDisabledImage_)
			overlayDisabledImage_ = image::get_texture(button_overlay_image_name_ + size_postfix + ".png~GS()" + button_image_path_suffix_);

		if (filesystem::file_exists(game_config::path + "/images/" + button_overlay_image_name_ + size_postfix + "-disabled-pressed.png"))
			overlayPressedDisabledImage_ = image::get_texture(button_overlay_image_name_ + size_postfix + "-disabled-pressed.png"+ button_image_path_suffix_);
		if (!overlayPressedDisabledImage_)
			overlayPressedDisabledImage_ = image::get_texture(button_overlay_image_name_ + size_postfix + "-pressed.png~GS()"+ button_image_path_suffix_);
	} else {
		overlayImage_.reset();
	}

	if (!disabledImage_) {
		disabledImage_ = image::get_texture(
			button_image_name_ + ".png~GS()" + button_image_path_suffix_);
	}

	if (!pressedImage_) {
		pressedImage_ = image_;
	}

	if (!activeImage_) {
		activeImage_ = image_;
	}

	if (type_ == TYPE_CHECK || type_ == TYPE_RADIO) {
		touchedImage_ = image::get_texture(
			button_image_name_ + "-touched.png"+ button_image_path_suffix_);
		if (!touchedImage_) {
			touchedImage_ = pressedImage_;
		}

		pressedActiveImage_ = image::get_texture(
			button_image_name_ + "-active-pressed.png"+ button_image_path_suffix_);
		if (!pressedActiveImage_) {
			pressedActiveImage_ = pressedImage_;
		}

		if (filesystem::file_exists(game_config::path + "/images/" + button_image_name_ + size_postfix + "-disabled-pressed.png")) {
			pressedDisabledImage_ = image::get_texture(
				button_image_name_ + "-disabled-pressed.png"+ button_image_path_suffix_);
		}
		if (!pressedDisabledImage_) {
			pressedDisabledImage_ = image::get_texture(
				button_image_name_ + "-pressed.png~GS()"+ button_image_path_suffix_);
		}
	}

	if (!image_) {
		std::string err_msg = "error initializing button images! file name: ";
		err_msg += button_image_name_;
		err_msg += ".png";
		ERR_DP << err_msg;
		throw game::error(err_msg);
	}

	base_height_ = image_.h();
	base_width_ = image_.w();

	if (type_ != TYPE_IMAGE) {
		set_label(label_text_);
	}

	if (type_ == TYPE_IMAGE){
		calculate_size();
	}
}

button::~button()
{
}

void button::calculate_size()
{
	if (type_ == TYPE_IMAGE){
		SDL_Rect loc_image = location();
		loc_image.h = image_.h();
		loc_image.w = image_.w();
		set_location(loc_image);
		return;
	}

	if (type_ != TYPE_IMAGE){
		textRect_ = font::pango_draw_text(false, sdl::empty_rect, font_size_, font::BUTTON_COLOR, label_text_, 0, 0);
	}

	// TODO: There's a weird text clipping bug, allowing the code below to run fixes it.
	// The proper fix should possibly be in the draw_contents() function.
#if 0
	if (!change_size)
		return;
#endif

	set_height(std::max(textRect_.h+vertical_padding_,base_height_));
	if(type_ == TYPE_PRESS || type_ == TYPE_TURBO) {
		if(spacing_ == MINIMUM_SPACE) {
			set_width(textRect_.w + horizontal_padding_);
		} else {
			set_width(std::max(textRect_.w+horizontal_padding_,base_width_));
		}
	} else {
		if(label_text_.empty()) {
			set_width(base_width_);
		} else {
			set_width(checkbox_horizontal_padding_ + textRect_.w + base_width_);
		}
	}
}

void button::set_check(bool check)
{
	if (type_ != TYPE_CHECK && type_ != TYPE_RADIO && type_ != TYPE_IMAGE)
		return;
	STATE new_state;

	if (check) {
		new_state = (state_ == ACTIVE || state_ == PRESSED_ACTIVE)? PRESSED_ACTIVE : PRESSED;
	} else {
		new_state = (state_ == ACTIVE || state_ == PRESSED_ACTIVE)? ACTIVE : NORMAL;
	}

	if (state_ != new_state) {
		state_ = new_state;
		queue_redraw();
	}
}

void button::set_active(bool active)
{
	if ((state_ == NORMAL) && active) {
		state_ = ACTIVE;
		queue_redraw();
	} else if ((state_ == ACTIVE) && !active) {
		state_ = NORMAL;
		queue_redraw();
	}
}

bool button::checked() const
{
	return state_ == PRESSED || state_ == PRESSED_ACTIVE || state_ == TOUCHED_PRESSED;
}

void button::enable(bool new_val)
{
	if(new_val != enabled())
	{
		pressed_ = false;
		// check buttons should keep their state
		if(type_ != TYPE_CHECK) {
			state_ = NORMAL;
		}
		widget::enable(new_val);
	}
}

// I can only assume this is working because nobody has complained it isn't.
void button::draw_contents()
{
	texture image = image_;

	int offset = 0;
	switch(state_) {
	case ACTIVE:
		image = activeImage_;
		break;
	case PRESSED:
		image = pressedImage_;
		if (type_ == TYPE_PRESS)
			offset = 1;
		break;
	case PRESSED_ACTIVE:
		image = pressedActiveImage_;
		break;
	case TOUCHED_NORMAL:
	case TOUCHED_PRESSED:
		image = touchedImage_;
		break;
	default:
		break;
	}

	SDL_Rect loc = location();
	SDL_Rect clipArea = loc;
	const int texty = loc.y + loc.h / 2 - textRect_.h / 2 + offset;
	int textx;

	if (type_ != TYPE_CHECK && type_ != TYPE_RADIO && type_ != TYPE_IMAGE) {
		textx = loc.x + image.w() / 2 - textRect_.w / 2 + offset;
	} else {
		clipArea.w += image.w() + checkbox_horizontal_padding_;
		textx = loc.x + image.w() + checkbox_horizontal_padding_ / 2;
	}

	color_t button_color = font::BUTTON_COLOR;

	if (!enabled()) {
		if (state_ == PRESSED || state_ == PRESSED_ACTIVE)
			image = pressedDisabledImage_;
		else image = disabledImage_;

		button_color = font::GRAY_COLOR;
	}

	SDL_Rect dest = loc;
	if(type_ != TYPE_PRESS && type_ != TYPE_TURBO) {
		// Scale other button types to match the base image?
		dest.w = image_.w();
		dest.h = image_.h();
	}

	draw::blit(image, dest);

	if (overlayImage_) {
		texture overlay = enabled() ? overlayImage_ : overlayDisabledImage_;

		if (overlayPressedImage_) {
			switch (state_) {
			case ACTIVE:
				if (overlayActiveImage_)
					overlay = overlayActiveImage_;
				break;
			case PRESSED:
			case PRESSED_ACTIVE:
			case TOUCHED_NORMAL:
			case TOUCHED_PRESSED:
				overlay = enabled() ? overlayPressedImage_ : overlayPressedDisabledImage_;
				break;
			default:
				break;
			}
		}

		dest.w = overlay.w();
		dest.h = overlay.h();
		draw::blit(overlay, dest);
	}

	if (type_ != TYPE_IMAGE){
		clipArea.x += offset;
		clipArea.y += offset;
		clipArea.w -= 2*offset;
		clipArea.h -= 2*offset;
		font::pango_draw_text(true, clipArea, font_size_, button_color, label_text_, textx, texty);
	}
}

bool button::hit(int x, int y) const
{
	return location().contains(x, y);
}

static bool is_valid_image(const std::string& str) { return !str.empty() && str[0] != IMAGE_PREFIX; }

void button::set_image(const std::string& image_file)
{
	if(!is_valid_image(image_file)) {
		return;
	}

	button_image_name_ = "buttons/" + image_file;
	load_images();
	queue_redraw();
}

void button::set_overlay(const std::string& image_file)
{
	// We allow empty paths for overlays
	if(image_file[0] == IMAGE_PREFIX) {
		return;
	}

	button_overlay_image_name_ = image_file;
	load_images();
	queue_redraw();
}

void button::set_label(const std::string& val)
{
	label_text_ = val;

	//if we have a list of items, use the first one that isn't an image
	if (std::find(label_text_.begin(), label_text_.end(), COLUMN_SEPARATOR) != label_text_.end()) {
		const std::vector<std::string>& items = utils::split(label_text_, COLUMN_SEPARATOR);
		const std::vector<std::string>::const_iterator i = std::find_if(items.begin(),items.end(),is_valid_image);
		if(i != items.end()) {
			label_text_ = *i;
		}
	}

	calculate_size();
	queue_redraw();
}

void button::mouse_motion(const SDL_MouseMotionEvent& event)
{
	if (hit(event.x, event.y)) {
		// the cursor is over the widget
		if (state_ == NORMAL)
			state_ = ACTIVE;
		else if (state_ == PRESSED && (type_ == TYPE_CHECK || type_ == TYPE_RADIO))
			state_ = PRESSED_ACTIVE;
	} else {
		// the cursor is not over the widget

		if (type_ == TYPE_CHECK || type_ == TYPE_RADIO) {

			switch (state_) {
				case TOUCHED_NORMAL:
					state_ = NORMAL;
					break;
				case TOUCHED_PRESSED:
					state_ = PRESSED;
					break;
				case PRESSED_ACTIVE:
					state_ = PRESSED;
					break;
				case ACTIVE:
					state_ = NORMAL;
					break;
				default:
					break;
			}
		} else if ((type_ != TYPE_IMAGE) || state_ != PRESSED)
				state_ = NORMAL;
	}
}

void button::mouse_down(const SDL_MouseButtonEvent& event)
{
	if (hit(event.x, event.y) && event.button == SDL_BUTTON_LEFT) {

		switch (type_) {
			case TYPE_RADIO:
			case TYPE_CHECK:
				if (state_ == PRESSED_ACTIVE)
					state_ = TOUCHED_PRESSED;
				else if (state_ == ACTIVE)
					state_ = TOUCHED_NORMAL;
				break;
			case TYPE_TURBO:
				sound::play_UI_sound(game_config::sounds::button_press);
				state_ = PRESSED;
				break;
			default:
				state_ = PRESSED;
				break;
		}
	}
}

void button::release(){
	state_ = NORMAL;
	draw_contents();
}

void button::mouse_up(const SDL_MouseButtonEvent& event)
{
	if (!(hit(event.x, event.y) && event.button == SDL_BUTTON_LEFT))
		return;

	// the user has stopped pressing the mouse left button while on the widget
	switch (type_) {
	case TYPE_CHECK:

		switch (state_) {
			case TOUCHED_NORMAL:
				state_ = PRESSED_ACTIVE;
				pressed_ = true;
				break;
			case TOUCHED_PRESSED:
				state_ = ACTIVE;
				pressed_ = true;
				break;
			default:
				break;
		}
		if (pressed_) sound::play_UI_sound(game_config::sounds::checkbox_release);
		break;
	case TYPE_RADIO:
		if (state_ == TOUCHED_NORMAL || state_ == TOUCHED_PRESSED) {
			state_ = PRESSED_ACTIVE;
			pressed_ = true;
		//	exit(0);
			sound::play_UI_sound(game_config::sounds::checkbox_release);
		}
		//} else if (state_ == TOUCHED_PRESSED) {
		//	state_ = PRESSED_ACTIVE;
		//}
		break;
	case TYPE_PRESS:
		if (state_ == PRESSED) {
			state_ = ACTIVE;
			pressed_ = true;
			sound::play_UI_sound(game_config::sounds::button_press);
		}
		break;
	case TYPE_TURBO:
		state_ = ACTIVE;
		break;
	case TYPE_IMAGE:
		pressed_ = true;
		sound::play_UI_sound(game_config::sounds::button_press);
		break;
	}
}

void button::handle_event(const SDL_Event& event)
{
	gui::widget::handle_event(event);

	if (hidden() || !enabled())
		return;

	STATE start_state = state_;

	if (!mouse_locked())
	{
		switch(event.type) {
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

	if (start_state != state_) {
		queue_redraw();
	}
}

bool button::pressed()
{
	if (type_ != TYPE_TURBO) {
		const bool res = pressed_;
		pressed_ = false;
		return res;
	} else
		return state_ == PRESSED || state_ == PRESSED_ACTIVE || state_ == TOUCHED_PRESSED;
}

}
