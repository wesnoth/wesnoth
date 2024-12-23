/*
	Copyright (C) 2003 - 2024
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

#pragma once

#include "fake_unit_ptr.hpp"
#include "map/location.hpp"
#include "units/animation.hpp"

class attack_type;
class game_board;
class game_display;
class unit;

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
class unit_mover {
public:
	unit_mover(const unit_mover&) = delete;
	unit_mover& operator=(const unit_mover&) = delete;

	explicit unit_mover(const std::vector<map_location>& path, bool animate=true, bool force_scroll=false);
	~unit_mover();

	void start(const unit_ptr& u);
	void proceed_to(const unit_ptr& u, std::size_t path_index, bool update=false, bool wait=true);
	void wait_for_anims();
	void finish(const unit_ptr& u, map_location::direction dir = map_location::direction::indeterminate);

private: // functions
	void replace_temporary(const unit_ptr& u);
	void update_shown_unit();

private: // data
	game_display * const disp_;
	const bool can_draw_;
	const bool animate_;
	const bool force_scroll_;
	unit_animator animator_;
	/** The animation potential to wait until. milliseconds::min for no wait; milliseconds::max to wait for end. */
	std::chrono::milliseconds wait_until_;
	/** The unit to be (re-)shown after an animation finishes. */
	unit_ptr shown_unit_;
	const std::vector<map_location>& path_;
	std::size_t current_;
	fake_unit_ptr temp_unit_ptr_;
	bool was_hidden_;
	bool is_enemy_;
};


/**
 * Display a unit moving along a given path.
 */
void move_unit(const std::vector<map_location>& path, const unit_ptr& u,
	bool animate=true,
	map_location::direction dir=map_location::direction::indeterminate,
	bool force_scroll=false);

/**
 * Play a pre-fight animation
 * First unit is the attacker, second unit the defender
 */
void unit_draw_weapon( const map_location& loc, unit& u, const const_attack_ptr& attack=nullptr, const const_attack_ptr& secondary_attack=nullptr,const map_location& defender_loc = map_location::null_location(), const unit_ptr& defender=unit_ptr());

/**
 * Play a post-fight animation
 * Both unit can be set to null, only valid units will play their animation
 */
void unit_sheath_weapon( const map_location& loc, const unit_ptr& u=unit_ptr(), const const_attack_ptr& attack=nullptr, const const_attack_ptr& secondary_attack=nullptr,const map_location& defender_loc = map_location::null_location(), const unit_ptr& defender=unit_ptr());

/**
 * Show a unit fading out.
 *
 * Note: this only shows the effect, it doesn't actually kill the unit.
 */
 void unit_die( const map_location& loc, unit& u,
	const const_attack_ptr& attack=nullptr, const const_attack_ptr& secondary_attack=nullptr,
	const map_location& winner_loc=map_location::null_location(),
	const unit_ptr& winner = unit_ptr());


/**
 *  Make the unit on tile 'a' attack the unit on tile 'b'.
 *
 *  The 'damage' will be subtracted from the unit's hitpoints,
 *  and a die effect will be displayed if the unit dies.
 */
void unit_attack(display * disp, game_board & board, //TODO: Would be nice if this could be purely a display function and defer damage dealing to its caller
	const map_location& a, const map_location& b, int damage,
	const attack_type& attack, const const_attack_ptr& secondary_attack,
	int swing, const std::string& hit_text, int drain_amount, const std::string& att_text, const std::vector<std::string>* extra_hit_sounds=nullptr,
	bool attacking=true);


void unit_recruited(const map_location& loc,
	const map_location& leader_loc=map_location::null_location());

/**
 * This will use a poisoning anim if healing<0.
 */
void unit_healing(unit &healed, const std::vector<unit *> &healers, int healing,
                  const std::string & extra_text="");

}
