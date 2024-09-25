/*
	Copyright (C) 2016 - 2024
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

#include "gui/dialogs/hotkey_bind.hpp"

#include "gui/widgets/window.hpp"


namespace gui2::dialogs
{

REGISTER_DIALOG(hotkey_bind)

hotkey_bind::hotkey_bind(const std::string& hotkey_id)
	: modal_dialog(window_id())
	, hotkey_id_(hotkey_id)
	, new_binding_()
{
}

void hotkey_bind::pre_show()
{
	connect_signal<event::SDL_RAW_EVENT>(
			std::bind(&hotkey_bind::sdl_event_callback, this, std::placeholders::_5),
			event::dispatcher::front_child);
}

void hotkey_bind::sdl_event_callback(const SDL_Event &event)
{
	if (hotkey::is_hotkeyable_event(event)) {
		new_binding_ = hotkey::create_hotkey(hotkey_id_, event);
	}
	if(new_binding_) {
		set_retval(retval::OK);
	}
}


} // namespace dialogs
