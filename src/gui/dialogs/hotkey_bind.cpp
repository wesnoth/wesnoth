/*
   Copyright (C) 2016 - 2017 by the Battle for Wesnoth Project http://www.wesnoth.org/

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
	window.connect_signal<event::SDL_RAW_EVENT>(
			std::bind(&hotkey_bind::sdl_event_callback, this, std::ref(window), _5),
			event::dispatcher::front_child);
}

void hotkey_bind::sdl_event_callback(window& win, const SDL_Event &event)
{
	if (hotkey::is_hotkeyable_event(event)) {
		new_binding_ = hotkey::create_hotkey(hotkey_id_, event);
	}
	if(event.type == SDL_KEYUP) {
		win.set_retval(window::OK);
	}
}


} // namespace dialogs
} // namespace gui2
