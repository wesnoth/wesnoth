/* $Id$ */
/*
   Copyright (C) 2007 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file gamestatus_editor.cpp
//! Dummy of gamestatus.cpp for the editor. The editor only needs to have
//! game_state::get_random defined, when using gamestatus.cpp there are
//! many more dependencies pulled in.

#include "gamestatus.hpp"

int game_state::get_random()
{
	return rand();
}
