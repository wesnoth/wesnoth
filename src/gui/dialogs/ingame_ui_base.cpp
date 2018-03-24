/*
   Copyright (C) 2018 by Charles Dang <exodia339@gmail.com>
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

#include "gui/dialogs/ingame_ui_base.hpp"

#include "display.hpp"
#include "game_config_manager.hpp"
#include "gettext.hpp"

namespace gui2
{
namespace dialogs
{
ingame_ui_base::ingame_ui_base()
	: disp_(display::get_singleton())
	, game_config_(game_config_manager::get()->game_config())
	, scenario_(game_config_.child_or_empty("scenario")) // FIXME: invalid in MP
{
}

} // namespace dialogs
} // namespace gui2
