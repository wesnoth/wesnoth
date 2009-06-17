/* $Id$ */
/*
   Copyright (C) 2003 - 2009 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 *  @file unit_display.hpp
 *  Display units performing various actions: moving, attacking, and dying.
 */

#ifndef UNIT_DISPLAY_HPP_INCLUDED
#define UNIT_DISPLAY_HPP_INCLUDED

#include "map_location.hpp"
#include "unit_map.hpp"
#include "variable.hpp"
#include "gamestatus.hpp"

class attack_type;
class team;
class unit;

/**
 *  Contains a number of free functions which display units
 *
 *  performing various on-screen actions - moving, attacking, and dying.
 */
namespace unit_display
{
bool unit_visible_on_path( const std::vector<map_location>& path, const unit& u, const unit_map& units, const std::vector<team>& teams);

/** Display a unit moving along a given path. */
void move_unit(const std::vector<map_location>& path, unit& u, const std::vector<team>& teams);

/**
 * Show a unit fading out.
 *
 * Note: this only shows the effect, it doesn't actually kill the unit.
 */
void unit_die( const map_location& loc, unit& u, const attack_type* attack=NULL, const attack_type*secondary_attack=NULL,const map_location& winner_loc = map_location::null_location, unit * winner=NULL);

/**
 *  Make the unit on tile 'a' attack the unit on tile 'b'.
 *
 *  The 'damage' will be subtracted from the unit's hitpoints,
 *  and a die effect will be displayed if the unit dies.
 *
 *  @retval	true                  if the defending unit is dead, should be
 *                                removed from the playing field.
 */
void unit_attack(
                 const map_location& a, const map_location& b, int damage,
                 const attack_type& attack, const attack_type* secondary_attack,
		 int swing,std::string hit_text,bool drain,std::string att_text);


void unit_recruited(const map_location& loc,const map_location& leader_loc=map_location::null_location);

/**
 *  Set healer_loc to an invalid location if there are no healers.
 *
 *  This will use a poisoning anim if healing<0.
 */
void unit_healing(unit& healed,map_location& healed_loc, std::vector<unit_map::iterator> healers, int healing);


/**
 * parse a standard WML for animations and play the corresponding animation, returns once animation is played
 * this is used for the animate_unit action, but can easily be generalized if other wml-decribed animations are needed
 *
 */
void wml_animation(const vconfig &cfg,unit_map & units, const gamemap& map, const gamestatus& game_status, const tod_manager& tod_mng, const map_location& default_location  = map_location::null_location);

}

#endif
