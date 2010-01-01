/* $Id$ */
/*
   Copyright (C) 2003 - 2010 by David White <dave@whitevine.net>
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

/**
 * Display a unit moving along a given path.
 *
 * Note: Hide the unit in its current location,
 * but don't actually remove it until the move is done,
 * so that while the unit is moving status etc.
 * will still display the correct number of units.
 *
 * @param path
 * @param u
 * @param teams
 * @param animate If set to false, only side-effects of move
 *        are applied (correct unit facing, path hexes redrawing).
 * @param dir Unit will be set facing this direction after move.
 *        If nothing passed, direction will be set based on path.
 */
void move_unit(const std::vector<map_location>& path, unit& u,
	const std::vector<team>& teams, bool animate=true,
	map_location::DIRECTION dir=map_location::NDIRECTIONS);

/**
 * Play a pre-fight animation
 * First unit is the attacker, second unit the defender
 */
void unit_draw_weapon( const map_location& loc, unit& u, const attack_type* attack=NULL, const attack_type*secondary_attack=NULL,const map_location& defender_loc = map_location::null_location, unit * defender=NULL);

/**
 * Play a post-fight animation
 * Both unit can be set to null, only valid units will play their animation
 */
void unit_sheath_weapon( const map_location& loc, unit* u=NULL, const attack_type* attack=NULL, const attack_type*secondary_attack=NULL,const map_location& defender_loc = map_location::null_location, unit * defender=NULL);

/**
 * Show a unit fading out.
 *
 * Note: this only shows the effect, it doesn't actually kill the unit.
 */
 void unit_die( const map_location& loc, unit& u,
 	const attack_type* attack=NULL, const attack_type* secondary_attack=NULL,
 	const map_location& winner_loc=map_location::null_location,
 	unit* winner=NULL);


/**
 *  Make the unit on tile 'a' attack the unit on tile 'b'.
 *
 *  The 'damage' will be subtracted from the unit's hitpoints,
 *  and a die effect will be displayed if the unit dies.
 *
 *  @retval	true                  if the defending unit is dead, should be
 *                                removed from the playing field.
 */
void unit_attack(const map_location& a, const map_location& b, int damage,
	const attack_type& attack, const attack_type* secondary_attack,
	int swing, std::string hit_text, bool drain, std::string att_text);


void unit_recruited(const map_location& loc,
	const map_location& leader_loc=map_location::null_location);

/**
 * Set healer_loc to an invalid location if there are no healers.
 *
 * This will use a poisoning anim if healing<0.
 */
void unit_healing(unit& healed, map_location& healed_loc,
	std::vector<unit_map::iterator> healers, int healing);


/**
 * Parse a standard WML for animations and play the corresponding animation.
 * Returns once animation is played.
 *
 * This is used for the animate_unit action, but can easily be generalized if
 * other wml-decribed animations are needed.
 */
void wml_animation(const vconfig &cfg,
	const map_location& default_location=map_location::null_location);

}

#endif
