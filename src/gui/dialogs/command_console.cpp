/*
   Copyright (C) 2017 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/command_console.hpp"

#include "gui/auxiliary/find_widget.hpp"
#include "gui/dialogs/modal_dialog.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/window.hpp"

namespace gui2
{
namespace dialogs
{
REGISTER_DIALOG(command_console)

std::unique_ptr<command_console> command_console::singleton_ = nullptr;

command_console::command_console(const std::string& prompt, callback_t callback)
	: input_(nullptr)
	, prompt_(prompt)
	, command_callback_(callback)
{
}

void command_console::post_build(window& window)
{
	// Allow ESC to dismiss the console.
	connect_signal_pre_key_press(window,
		std::bind(&command_console::window_key_press_callback, this, _5));

	input_ = find_widget<text_box>(&window, "input", false, true);

	// Execute provided callback on ENTER press.
	connect_signal_pre_key_press(*input_,
		std::bind(&command_console::input_key_press_callback, this, _5));
}

void command_console::pre_show(window& window)
{
	find_widget<label>(&window, "prompt", false).set_label(prompt_);

	window.keyboard_capture(input_);
}

void command_console::window_key_press_callback(const SDL_Keycode key)
{
	if(key == SDLK_ESCAPE) {
		close();
	}
}

void command_console::input_key_press_callback(const SDL_Keycode key)
{
	if(key == SDLK_RETURN || key == SDLK_KP_ENTER) {
		// Execute callback.
		if(command_callback_ != nullptr) {
			command_callback_(input_->get_value());
		}

		// Dismiss dialog.
		close();
	} else if(key == SDLK_TAB) {
		// TODO: implement
	}
}

void command_console::close()
{
	hide();
	singleton_.reset();
}

} // namespace dialogs
} // namespace gui2
