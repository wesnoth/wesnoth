/* $Id$ */
/*
   Copyright (C) 2008 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "editor_mode.hpp"

namespace editor2 {

editor_mode::editor_mode() 
: foreground_terrain_(t_translation::MOUNTAIN)
, background_terrain_(t_translation::DEEP_WATER)
, brush_(NULL)
, mouse_action_(NULL)
{
}


} //end namespace editor2
