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

#include "gui/dialogs/surrender_quit.hpp"
#include "../widgets/settings.hpp"


namespace gui2
{
namespace dialogs
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_surrender_quit
 *
 * == Surrender and/or quit a game ==
 *
 * This shows the dialog to confirm surrender and/or quitting the game
 *
 */

REGISTER_DIALOG(surrender_quit)


surrender_quit::surrender_quit()
{
	set_restore(true);
}

} // namespace dialogs
} // namespace gui2
