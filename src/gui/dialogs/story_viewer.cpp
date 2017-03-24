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

	// Special callback to make Enter advance to the next part.
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
	// TODO: handle all layers
	const std::string bg = current_part_->get_background_layers()[0].file();

   	window.get_canvas(0).set_variable("background_image", variant(bg));

	find_widget<label>(&window, "title", false).set_label(current_part_->title());
	find_widget<label>(&window, "part_text", false).set_label(current_part_->text());

	// Needed to make the background redraw correctly.
	window.set_is_dirty(true);
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

	// TODO: disable buttons

	update_current_part_ptr();

	display_part(window);
}

void story_viewer::key_press_callback(window& window, const SDL_Keycode key)
{
	if(key == SDLK_RETURN) {
		nav_button_callback(window, DIR_FORWARD);
	}
}

} // namespace dialogs
} // namespace gui2
