/*
	Copyright (C) 2008 - 2021
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "gui/dialogs/game_delete.hpp"

#include "preferences/game.hpp"
#include "gui/widgets/settings.hpp"

namespace gui2::dialogs
{

REGISTER_DIALOG(game_delete)

/**
 * Helper to invert @ref preferences::ask_delete_saves.
 *
 * The value stored and the way shown is inverted.
 */
static bool get_dont_ask_again()
{
	return !preferences::ask_delete_saves();
}

/**
 * Helper to invert @ref preferences::set_ask_delete_saves.
 *
 * The value stored and the way shown is inverted.
 */
static void set_dont_ask_again(const bool ask_again)
{
	preferences::set_ask_delete_saves(!ask_again);
}

game_delete::game_delete()
{
	set_restore(true);

	register_bool(
			"dont_ask_again", true, &get_dont_ask_again, &set_dont_ask_again);
}

} // namespace dialogs
