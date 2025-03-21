/*
	Copyright (C) 2012 - 2025
	by Iris Morelle <shadowm2006@gmail.com>
	Copyright (C) 2008 - 2018 by Jörg Hinrichs <joerg.hinrichs@alice-dsl.de>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "gui/dialogs/multiplayer/mp_host_game_prompt.hpp"

#include "preferences/preferences.hpp"

namespace gui2::dialogs
{

REGISTER_DIALOG(mp_host_game_prompt)

/**
 * Helper for @ref prefs::get().ask_delete_saves.
 */
static bool get_do_not_show_again()
{
	return prefs::get().mp_server_warning_disabled() != 1;
}

/**
 * Helper for @ref prefs::get().set_ask_delete_saves.
 */
static void set_do_not_show_again(const bool do_not_show_again)
{
	prefs::get().set_mp_server_warning_disabled(do_not_show_again ? 2 : 1);
}

mp_host_game_prompt::mp_host_game_prompt()
	: modal_dialog(window_id())
{
	register_bool("do_not_show_again",
				  true,
				  &get_do_not_show_again,
				  &set_do_not_show_again);
}

} // namespace dialogs
