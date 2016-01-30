/*
   Copyright (C) 2003 - 2016 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 *  @file
 *  Display units performing various actions: moving, attacking, and dying.
 */

#ifndef UNIT_DISPLAY_HPP_INCLUDED
#define UNIT_DISPLAY_HPP_INCLUDED

#include "fake_unit_ptr.hpp"
#include "map_location.hpp"
#include "unit_animation.hpp"

class attack_type;
class fake_unit_manager;
class game_board;
class game_display;
class unit;
class vconfig;

/**
 *  Contains a number of free functions which display units
 *
 *  performing various on-screen actions - moving, attacking, and dying.
 */
namespace unit_display
{

/**
 * A class to encapsulate the steps of drawing a unit's move.
 * If control over how far the unit moves is not needed, move_unit() may
 * be a more convenient interface.
 */
class unit_mover : public boost::noncopyable {
public:
	explicit unit_mover(const std::vector<map_location>& path, bool animate=true, bool force_scroll=false);
	~unit_mover();

	void start(unit_ptr u);
	void proceed_to(unit_ptr u, size_t path_index, bool update=false, bool wait=true);
	void wait_for_anims();
	void finish(unit_ptr u, map_location::DIRECTION dir = map_location::NDIRECTIONS);

private: // functions
	void replace_temporary(unit_ptr u);
	void update_shown_unit();

private: // data
	game_display * const disp_;
	const bool can_draw_;
	const bool animate_;
	const bool force_scroll_;
	unit_animator animator_;
	int wait_until_;	/// The animation potential to wait until. INT_MIN for no wait; INT_MAX to wait for end.
	unit_ptr shown_unit_;	/// The unit to be (re-)shown after an animation finishes.
	const std::vector<map_location>& path_;
	size_t current_;
	fake_unit_ptr temp_unit_ptr_;
	bool was_hidden_;
	bool is_enemy_;
};


/**
 * Display a unit moving along a given path.
 */
void move_unit(const std::vector<map_location>& path, unit_ptr u,
	bool animate=true,
	map_location::DIRECTION dir=map_location::NDIRECTIONS,
	bool force_scroll=false);

/**
 * Play a pre-fight animation
 * First unit is the attacker, second unit the defender
 */
void unit_draw_weapon( const map_location& loc, unit& u, const attack_type* attack=NULL, const attack_type*secondary_attack=NULL,const map_location& defender_loc = map_location::null_location(), unit * defender=NULL);

/**
 * Play a post-fight animation
 * Both unit can be set to null, only valid units will play their animation
 */
void unit_sheath_weapon( const map_location& loc, unit* u=NULL, const attack_type* attack=NULL, const attack_type*secondary_attack=NULL,const map_location& defender_loc = map_location::null_location(), unit * defender=NULL);

/**
 * Show a unit fading out.
 *
 * Note: this only shows the effect, it doesn't actually kill the unit.
 */
 void unit_die( const map_location& loc, unit& u,
	const attack_type* attack=NULL, const attack_type* secondary_attack=NULL,
	const map_location& winner_loc=map_location::null_location(),
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
void unit_attack(display * disp, game_board & board, //TODO: Would be nice if this could be purely a display function and defer damage dealing to its caller
	const map_location& a, const map_location& b, int damage,
	const attack_type& attack, const attack_type* secondary_attack,
	int swing, std::string hit_text, int drain_amount, std::string att_text, const std::vector<std::string>* extra_hit_sounds=NULL);


void unit_recruited(const map_location& loc,
	const map_location& leader_loc=map_location::null_location());

/**
 * This will use a poisoning anim if healing<0.
 */
void unit_healing(unit &healed, const std::vector<unit *> &healers, int healing,
                  const std::string & extra_text="");


/**
 * Parse a standard WML for animations and play the corresponding animation.
 * Returns once animation is played.
 *
 * This is used for the animate_unit action, but can easily be generalized if
 * other wml-described animations are needed.
 */
void wml_animation(const vconfig &cfg,
	const map_location& default_location=map_location::null_location());

}

#endif
