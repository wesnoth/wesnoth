/*
   Copyright (C) 2008 - 2017 by Jörg Hinrichs <joerg.hinrichs@alice-dsl.de>
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

#include <boost/multi_index_container.hpp>
// ^ This is apparently unnecessary but we don't compile without it...

#include "gui/dialogs/game_save.hpp"

#include "gettext.hpp"
#include "gui/auxiliary/field.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/settings.hpp"

namespace gui2
{
namespace dialogs
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_game_save
 *
 * == Save a game ==
 *
 * This shows the dialog to create a savegame file.
 *
 * @begin{table}{dialog_widgets}
 *
 * lblTitle & & label & m &
 *         The title of the window. $
 *
 * txtFilename & & text_box & m &
 *         The name of the savefile. $
 *
 * @end{table}
 */

REGISTER_DIALOG(game_save)

game_save::game_save(std::string& filename, const std::string& title)
{
	set_restore(true);

	register_text("txtFilename", false, filename, true);
	register_label("lblTitle", true, title);
}

REGISTER_DIALOG(game_save_message)

game_save_message::game_save_message(std::string& filename,
									   const std::string& title,
									   const std::string& message)
{
	set_restore(true);

	register_label("lblTitle", true, title);
	register_text("txtFilename", false, filename, true);
	register_label("lblMessage", true, message);
}

REGISTER_DIALOG(game_save_oos)

game_save_oos::game_save_oos(bool& ignore_all,
							   std::string& filename,
							   const std::string& title,
							   const std::string& message)
{
	register_label("lblTitle", true, title);
	register_text("txtFilename", false, filename, true);
	register_label("lblMessage", true, message);
	register_bool("ignore_all", true, ignore_all);

	/* Always need the ignore_all flag. */
	set_always_save_fields(true);
}

} // namespace dialogs
} // namespace gui2
