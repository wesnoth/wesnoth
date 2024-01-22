/*
	Copyright (C) 2012 - 2024
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

#include "preferences/game.hpp"
#include "gui/widgets/settings.hpp"

namespace gui2::dialogs
{

REGISTER_DIALOG(mp_host_game_prompt)

/**
 * Helper for @ref preferences::ask_delete_saves.
 */
static bool get_do_not_show_again()
{
	return preferences::mp_server_warning_disabled() != 1;
}

/**
 * Helper for @ref preferences::set_ask_delete_saves.
 */
static void set_do_not_show_again(const bool do_not_show_again)
{
	preferences::set_mp_server_warning_disabled(do_not_show_again ? 2 : 1);
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
