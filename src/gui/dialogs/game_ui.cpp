/*
   Copyright (C) 2017-2018 by Charles Dang <exodia339@gmail.com>
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

#include "gui/dialogs/game_ui.hpp"

#include "gui/auxiliary/find_widget.hpp"
#include "gui/dialogs/modal_dialog.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/minimap.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"

#include "display.hpp"
#include "formula/variant.hpp"
#include "game_config_manager.hpp"
#include "gettext.hpp"

namespace gui2
{
namespace dialogs
{
REGISTER_DIALOG(game_ui)

game_ui::game_ui()
	: ingame_ui_base()
{
}

void game_ui::pre_show(window& window)
{
	minimap& mmap = find_widget<minimap>(&window, "minimap", false);

	mmap.set_config(&game_config_);
	mmap.set_map_data(scenario_["map_data"].str());
}

} // namespace dialogs
} // namespace gui2
