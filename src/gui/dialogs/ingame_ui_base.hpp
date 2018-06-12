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

#pragma once

#include "gui/dialogs/modeless_dialog.hpp"

class config;
class display;

namespace gui2
{
namespace dialogs
{
/**
 * Base dialog class intended for in-game UIs for the main game and editor.
 */
class ingame_ui_base : public modeless_dialog
{
public:
	ingame_ui_base();

protected:
	::display* disp_; // TODO: needed?

	/** Reference to the entire master game config object. */
	const config& game_config_;

	/** Reference to the current scenario's config. */
	const config& scenario_;
};

} // namespace dialogs
} // namespace gui2
