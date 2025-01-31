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
 * @file
 * Various functions related to moving units.
 */

#pragma once

struct map_location;

#include "units/map.hpp"
#include "game_events/fwd.hpp"
#include "synced_commands.hpp"

#include <vector>


namespace actions {
	class undo_list;


class move_unit_spectator : public action_spectator {
public:

	/** ingerited from action_spectator */
	void error(const std::string& message) override;
	/** add a location of a seen friend */
	void add_seen_friend(const unit_map::const_iterator &u);


	/** add the location of new seen enemy */
	void add_seen_enemy(const unit_map::const_iterator &u);


	/** get the location of an ambusher */
	const unit_map::const_iterator& get_ambusher() const;


	/** get the location of a failed teleport */
	const unit_map::const_iterator& get_failed_teleport() const;


	/** get the locations of seen enemies */
	const std::vector<unit_map::const_iterator>& get_seen_enemies() const;


	/** get the locations of seen friends */
	const std::vector<unit_map::const_iterator>& get_seen_friends() const;


	/** get new location of moved unit */
	const unit_map::const_iterator& get_unit() const;


	/** constructor */
	move_unit_spectator(const unit_map& units);

	move_unit_spectator();


	/** destructor */
	virtual ~move_unit_spectator();

	/** reset all locations to empty values*/
	void reset(const unit_map &units);


	/** set the location of an ambusher */
	void set_ambusher(const unit_map::const_iterator &u);


	/** set the location of a failed teleport */
	void set_failed_teleport(const unit_map::const_iterator &u);


	/** set the iterator to moved unit*/
	void set_unit(const unit_map::const_iterator &u);

	void set_ai_move(bool ai_move = true) { is_ai_move_ = ai_move; }
	bool is_ai_move() const { return is_ai_move_; }

	void set_interrupted(bool interrupted) { interrupted_ = interrupted; }
	bool get_interrupted() const { return interrupted_; }

	void set_tiles_entered(std::size_t tiles_entered) { tiles_entered_ = tiles_entered; }
	std::size_t get_tiles_entered() const { return tiles_entered_; }

private:
	unit_map::const_iterator ambusher_;
	unit_map::const_iterator failed_teleport_;
	std::vector<unit_map::const_iterator> seen_enemies_;
	std::vector<unit_map::const_iterator> seen_friends_;
	unit_map::const_iterator unit_;
	std::size_t tiles_entered_;
	bool interrupted_;
	bool is_ai_move_ = false;
};

/**
 * Makes it so the village at the given location is owned by the given side.
 * Returns true if getting the village triggered a mutating event.
 * side can be 0 to make the village uncaptured.
 */
game_events::pump_result_t get_village(const map_location& loc, int side, bool *time_bonus = nullptr, bool fire_event = true);

/**
 * Teleports a unit across the board and enters the synced context.
 */
void teleport_unit_and_record(const map_location& teleport_from,
	const map_location& teleport_to,
	move_unit_spectator* move_spectator = nullptr);

/**
 * Teleports a unit across the board.
 * To be called from replay when we are already in the synced context.
 */
void teleport_unit_from_replay(
	const std::vector<map_location> &steps,
	bool continued_move,
	bool skip_ally_sighted,
	bool show_move);

/**
 * Moves a unit across the board.
 * And enters the synced context.
 */
void move_unit_and_record(
	const std::vector<map_location> &steps,
	bool continued_move,
	move_unit_spectator& move_spectator);

std::size_t move_unit_and_record(const std::vector<map_location>& steps,
	bool continued_move = false,
	bool* interrupted = nullptr);

/**
 * Moves a unit across the board.
 * to be called from replay when we are already in the synced context.
 */
void execute_move_unit(
	const std::vector<map_location> &steps,
	bool continued_move,
	bool skip_ally_sighted,
	move_unit_spectator* move_spectator);

}//namespace actions
