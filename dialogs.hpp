/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef DIALOGS_H_INCLUDED
#define DIALOGS_H_INCLUDED

#include "actions.hpp"
#include "display.hpp"
#include "menu.hpp"

namespace dialogs
{
void advance_unit(const game_data& info,
                  std::map<gamemap::location,unit>& units,
				  const gamemap::location& loc,
				  display& gui, bool random_choice=false);
}

#endif
