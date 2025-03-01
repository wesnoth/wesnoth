/*
	Copyright (C) 2008 - 2025
	by Jörg Hinrichs <joerg.hinrichs@alice-dsl.de>
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

#include "gui/dialogs/game_save.hpp"

#include "gettext.hpp"

namespace gui2::dialogs
{

REGISTER_DIALOG(game_save)

game_save::game_save(std::string& filename, const std::string& title)
	: modal_dialog(window_id())
{
	register_text("txtFilename", false, filename, true);
	register_label("lblTitle", true, title);
}

REGISTER_DIALOG(game_save_message)

game_save_message::game_save_message(std::string& filename,
									   const std::string& title,
									   const std::string& message)
	: modal_dialog(window_id())
{
	register_label("lblTitle", true, title);
	register_text("txtFilename", false, filename, true);
	register_label("lblMessage", true, message);
}

REGISTER_DIALOG(game_save_oos)

game_save_oos::game_save_oos(bool& ignore_all,
							   std::string& filename,
							   const std::string& title,
							   const std::string& message)
	: modal_dialog(window_id())
{
	register_label("lblTitle", true, title);
	register_text("txtFilename", false, filename, true);
	register_label("lblMessage", true, message);
	register_bool("ignore_all", true, ignore_all);

	/* Always need the ignore_all flag. */
	set_always_save_fields(true);
}

} // namespace dialogs
