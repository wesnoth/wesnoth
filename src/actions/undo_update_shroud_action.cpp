/*
   Copyright (C) 2017-2018 the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "actions/undo_update_shroud_action.hpp"
#include "replay.hpp"

namespace actions
{
namespace undo
{


/**
 * Writes this into the provided config.
 */
void auto_shroud_action::write(config & cfg) const
{
	undo_action_base::write(cfg);
	cfg["active"] = active;
}

/**
 * Writes this into the provided config.
 */
void update_shroud_action::write(config & cfg) const
{
	undo_action_base::write(cfg);
}


}
}
