/* $Id$ */
/*
  Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
  Part of the Battle for Wesnoth Project http://www.wesnoth.org/

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License version 2
  or at your option any later version.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY.

  See the COPYING file for more details.
*/

//! @file editor/editor_dialogs.hpp
//!

#include "../display.hpp"
#include "../config.hpp"
#include "../map.hpp"

#include "map_manip.hpp"

#include <map>

#ifndef EDITOR_DIALOGS_H_INCLUDED
#define EDITOR_DIALOGS_H_INCLUDED

namespace map_editor {

//! Notify the user that the map is changed and ask if the user wants to
//! proceed. Return true if yes is answered.
bool confirm_modification_disposal(display &disp);

//! Show a dialog to create new map. If confirmation_needed is true, the
//! user will be asked if she wants to continue even though the changes
//! to the current map is lost. fill_terrain will be used to fill the
//! map if a new one is created. Return the string representation of the
//! new map, or the empty string if the operation failed or was
//! cancelled.
std::string new_map_dialog(display &disp, const t_translation::t_terrain fill_terrain,
   const bool confirmation_needed, const config &gconfig);

//! Show a dialog where the user may set the preferences used in the
//! editor.
void preferences_dialog(display &disp, config &prefs);

//! Show a dialog asking for the new size of the map. Return the chosen
//! width and height. Checks are made to see that the desired values
//! will result in a feasible map.
bool resize_dialog(display &disp, unsigned& width, unsigned& height,
	int& x_offset, int& y_offset, bool& do_expand);

FLIP_AXIS flip_dialog(display &disp);

}


#endif // DIALOGS_H_INCLUDED
