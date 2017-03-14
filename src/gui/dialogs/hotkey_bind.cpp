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
	window.connect_signal<event::SDL_KEY_DOWN>(
		std::bind(&hotkey_bind::key_press_callback, this, _3, _4, _5), event::dispatcher::back_child);
}

void hotkey_bind::key_press_callback(bool&, bool&, const SDL_Keycode key)
{
	UNUSED(key);

	//new_binding_ = hotkey::create_hotkey(hotkey_id_, event);
}

void hotkey_bind::post_show(window& window)
{
	if(window.get_retval() == window::OK) {

	}
}

} // namespace dialogs
} // namespace gui2
