/*
   Copyright (C) 2017 by Charles Dang <exodia339@gmail.com>
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

#include "gui/dialogs/story_viewer.hpp"

#include "formula/variant.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/scroll_label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"

namespace gui2
{
namespace dialogs
{

REGISTER_DIALOG(story_viewer)

story_viewer::story_viewer(storyscreen::controller& controller)
	: controller_(controller)
	, part_index_(0)
	, current_part_()
{
	update_current_part_ptr();
}

void story_viewer::pre_show(window& window)
{
	window.set_enter_disabled(true);

	// Special callback handle key presses
	connect_signal_pre_key_press(window, std::bind(&story_viewer::key_press_callback, this, std::ref(window), _5));

	connect_signal_mouse_left_click(find_widget<button>(&window, "next", false),
		std::bind(&story_viewer::nav_button_callback, this, std::ref(window), DIR_FORWARD));

	connect_signal_mouse_left_click(find_widget<button>(&window, "back", false),
		std::bind(&story_viewer::nav_button_callback, this, std::ref(window), DIR_BACKWARDS));

	display_part(window);
}

void story_viewer::update_current_part_ptr()
{
	current_part_ = controller_.get_part(part_index_);
}

void story_viewer::display_part(window& window)
{
	// Update Back button state. Doing this here so it gets called in pre_show too.
	find_widget<button>(&window, "back", false).set_active(part_index_ != 0);

	config cfg, shape;

	for(const auto& layer : current_part_->get_background_layers()) {
		//SDL_Rect image_rect;

		shape["x"] = "(max(pos, 0) where pos = (width  / 2 - image_width  / 2))";
		shape["y"] = "(max(pos, 0) where pos = (height / 2 - image_height / 2))";
		shape["w"] = "(image_original_width * height / image_original_height)";
		shape["h"] = "(height)";
		shape["name"] = layer.file();

		cfg.add_child("image", shape);
	}

	canvas& window_canvas = window.get_canvas(0);

	window_canvas.set_cfg(cfg);

	// Needed to make the background redraw correctly.
	window_canvas.set_is_dirty(true);
	window.set_is_dirty(true);

	// FIXME: seems if the label doesn't have *something* in it each time its set, the label will never show up.
	const std::string title = current_part_->title().empty() ? " " : current_part_->title();
	find_widget<label>(&window, "title", false).set_label(title);

	find_widget<label>(&window, "part_text", false).set_label(current_part_->text());
}

void story_viewer::nav_button_callback(window& window, NAV_DIRECTION direction)
{
	part_index_ = (direction == DIR_FORWARD ? part_index_ + 1 : part_index_ -1);

	// If we've viewed all the parts, close the dialog.
	if(part_index_ == controller_.max_parts()) {
		window.close();
		return;
	}

	if(part_index_ < 0) {
		part_index_ = 0;
	}

	update_current_part_ptr();

	display_part(window);
}

void story_viewer::key_press_callback(window& window, const SDL_Keycode key)
{
	const bool next_keydown =
		   key == SDLK_SPACE
		|| key == SDLK_RETURN
		|| key == SDLK_KP_ENTER
		|| key == SDLK_RIGHT;

	const bool back_keydown =
		   key == SDLK_BACKSPACE
		|| key == SDLK_LEFT;

	if(next_keydown) {
		nav_button_callback(window, DIR_FORWARD);
	} else if(back_keydown) {
		nav_button_callback(window, DIR_BACKWARDS);
	}
}

} // namespace dialogs
} // namespace gui2
