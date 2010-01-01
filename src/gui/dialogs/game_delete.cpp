/* $Id$ */
/*
   Copyright (C) 2008 - 2010 by Jörg Hinrichs <joerg.hinrichs@alice-dsl.de>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "gui/dialogs/field.hpp"
#include "gui/dialogs/game_delete.hpp"
#include "gui/widgets/settings.hpp"

namespace gui2 {

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_game_delete
 *
 * == Delete a savegame ==
 *
 * This shows the dialog to confirm deleting a savegame file.
 *
 * @start_table = container
 *     (dont_ask_again) (boolean_selector) ()
 *                     A checkbox to not show this dialog again.
 * @end_table
 */

tgame_delete::tgame_delete()
	: chk_dont_ask_again_(register_bool("dont_ask_again"))
	, dont_ask_again_(false)
{}

twindow* tgame_delete::build_window(CVideo& video)
{
	return build(video, get_id(GAME_DELETE));
}

void tgame_delete::post_show(twindow& window)
{
	dont_ask_again_ = chk_dont_ask_again_->get_widget_value(window);
}

} // namespace gui2

