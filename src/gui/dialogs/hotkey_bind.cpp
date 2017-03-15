/*
   Copyright (C) 2016 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/hotkey_bind.hpp"

#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"

#include <SDL.h>

namespace gui2
{
namespace dialogs
{

REGISTER_DIALOG(hotkey_bind)

hotkey_bind::hotkey_bind(const std::string& hotkey_id)
	: hotkey_id_(hotkey_id)
	, new_binding_()
{
	set_restore(true);
}

void hotkey_bind::pre_show(window& window)
{
	connect_signal_pre_key_press(window, std::bind(&hotkey_bind::key_press_callback, this, std::ref(window), _5));

	window.connect_signal<event::SDL_LEFT_BUTTON_DOWN>(
		std::bind(&hotkey_bind::mouse_button_callback, this, std::ref(window), SDL_BUTTON_LEFT), event::dispatcher::front_child);
	window.connect_signal<event::SDL_MIDDLE_BUTTON_DOWN>(
		std::bind(&hotkey_bind::mouse_button_callback, this, std::ref(window), SDL_BUTTON_MIDDLE), event::dispatcher::front_child);
	window.connect_signal<event::SDL_RIGHT_BUTTON_DOWN>(
		std::bind(&hotkey_bind::mouse_button_callback, this, std::ref(window), SDL_BUTTON_RIGHT), event::dispatcher::front_child);
}

void hotkey_bind::key_press_callback(window& window, const SDL_Keycode key)
{
	/* HACK: SDL_KEYDOWN and SDL_TEXTINPUT events forward to the same GUI2 event (SDL_KEY_DOWN), meaning
	 *       this even gets fired twice, causing problems since 'key' will be 0 in the latter case. SDLK_UNKNOWN
	 *       is the key value used by SDL_TEXTINPUT handling, so exit here if that's detected.
	 */
	if(key == SDLK_UNKNOWN) {
		return;
	}

	new_binding_ = hotkey::create_hotkey(hotkey_id_, SDL_GetScancodeFromKey(key));

	window.set_retval(window::OK);
}

void hotkey_bind::mouse_button_callback(window& window, Uint8 button)
{
	new_binding_ = hotkey::create_hotkey(hotkey_id_, button);

	window.set_retval(window::OK);
}

} // namespace dialogs
} // namespace gui2
