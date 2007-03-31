/* $Id$ */
/*
   Copyright (C) 2003-5 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef UNIT_DISPLAY_HPP_INCLUDED
#define UNIT_DISPLAY_HPP_INCLUDED

#include "map.hpp"

class attack_type;
class display;
class team;
class unit;
class unit_map;

///the unit_display namespace contains a number of free functions
///which display units performing various on-screen actions - moving,
///attacking, and dying
namespace unit_display
{
bool unit_visible_on_path(display& disp, const std::vector<gamemap::location>& path, const unit& u, const unit_map& units, const std::vector<team>& teams);

///a function to display a unit moving along a given path
void move_unit(display& disp, const gamemap& map, const std::vector<gamemap::location>& path, unit& u, const unit_map& units, const std::vector<team>& teams);

///a function to show a unit fading out. Note that this only shows the effect, it doesn't
///actually kill the unit.
void unit_die(display& disp, const gamemap::location& loc, unit& u, const attack_type* attack=NULL, const attack_type*secondary_attack=NULL, unit * winner=NULL);

///a function to make the unit on tile 'a' attack the unit on tile 'b'.
///the 'damage' will be subtracted from the unit's hitpoints, and a die effect will be
///displayed if the unit dies.
///true is returned if the defending unit is dead, and should be removed from the
///playing field.
void unit_attack(display& disp, unit_map& units,
                 const gamemap::location& a, const gamemap::location& b, int damage,
                 const attack_type& attack, const attack_type* secondary_attack,
		 bool update_display, int swing);

}

#endif
