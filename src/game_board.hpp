/*
   Copyright (C) 2014 - 2017 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "display_context.hpp"
#include "team.hpp"
#include "terrain/type_data.hpp"
#include "units/map.hpp"
#include "units/id.hpp"

#include <boost/optional.hpp>
#include <set>
#include <vector>

class config;
class gamemap;

namespace events {
	class mouse_handler;
	class menu_handler;
}

/**
 *
 * Game board class.
 *
 * The purpose of this class is to encapsulate some of the core game logic, including the unit map,
 * the list of teams, and the game map.
 *
 * This should eventually become part of the game state object IMO, which should be a child of play_controller.
 *
 * I also intend to move the pathfinding module to be housed within this class -- this way, we can implement a
 * sound pathfinding data structure to speed up path computations for AI, without having to make "update event"
 * code at all points in the engine which modify the relevant data.
 *
 **/

class game_board : public display_context
{

	std::vector<team> teams_;
	std::vector<std::string> labels_;

	std::unique_ptr<gamemap> map_;
	n_unit::id_manager unit_id_manager_;
	unit_map units_;

	//TODO: Remove these when we have refactored enough to make it possible.
	friend class play_controller;
	friend class events::mouse_handler;
	friend class events::menu_handler;
	friend class game_state;
	friend class game_lua_kernel;

	/**
	 * Temporary unit move structs:
	 *
	 * Probably don't remove these friends, this is actually fairly useful. These structs are used by:
	 *  - AI
	 *  - Whiteboard
	 *  - I think certain wml actions
	 * For AI, the ai wants to move two units next to eachother so it can ask for attack calculations. This should not trigger
	 * pathfinding modifications, so the version that directly changes the unit map is probably preferable, although it should be
	 * refactored.
	 * For whiteboard and wml actions, we generally do want pathfinding to be updated, so use the game_board constructors which I
	 * have added to these structs instead.
	 *
	 **/
	friend struct temporary_unit_placer;
	friend struct temporary_unit_mover;
	friend struct temporary_unit_remover;

public:
	n_unit::id_manager& unit_id_manager() { return unit_id_manager_; }
	// Constructors, trivial dtor, and const accessors

	game_board(const ter_data_cache & tdata, const config & level);
	virtual ~game_board();

	virtual const std::vector<team> & teams() const { return teams_; }
	using display_context::get_team; // so as not to hide the const version
	team& get_team(int i) {return teams_[i - 1];}
	virtual std::vector<team> & teams() { return teams_; }
	virtual const gamemap & map() const { return *map_; }
	virtual const unit_map & units() const { return units_; }
	unit_map & units() { return units_; }
	virtual const std::vector<std::string> & hidden_label_categories() const { return labels_; }

	// Copy and swap idiom, because we have a scoped pointer.

	game_board(const game_board & other);
	game_board& operator=(const game_board& other) = delete;

	friend void swap(game_board & one, game_board & other);

	// Saving

	void write_config(config & cfg) const;

	// Manipulators from play_controller

	void new_turn(int pnum);
	void end_turn(int pnum);
	void set_all_units_user_end_turn();

	void heal_all_survivors();

	void check_victory(bool &, bool &, bool &, bool &, std::set<unsigned> &, bool);

	// Manipulator from playturn

	void side_drop_to (int side_num, team::CONTROLLER ctrl, team::PROXY_CONTROLLER proxy = team::PROXY_CONTROLLER::PROXY_HUMAN);
	void side_change_controller (int side_num, bool is_local, const std::string& pname = "");

	// Manipulator from actionwml

	bool try_add_unit_to_recall_list(const map_location& loc, const unit_ptr u);
	boost::optional<std::string> replace_map (const gamemap & r);
	void overlay_map (const gamemap & o, const config & cfg, map_location loc);

	bool change_terrain(const map_location &loc, const std::string &t,
	                    const std::string & mode, bool replace_if_failed); //used only by lua

	// Global accessor from unit.hpp

	unit_map::iterator find_visible_unit(const map_location &loc, const team& current_team, bool see_all = false);
	unit_map::iterator find_visible_unit(const map_location & loc, size_t team, bool see_all = false) { return find_visible_unit(loc, teams_[team], see_all); }
	bool has_visible_unit (const map_location & loc, const team & team, bool see_all = false) const;
	bool has_visible_unit (const map_location & loc, size_t team, bool see_all = false) const { return has_visible_unit(loc, teams_[team], see_all); }

	unit* get_visible_unit(const map_location &loc, const team &current_team, bool see_all = false); //TODO: can this not return a pointer?

	// Wrapped functions from unit_map. These should ultimately provide notification to observers, pathfinding.

	unit_map::iterator find_unit(const map_location & loc) { return units_.find(loc); }
	/// Calculates whether a team is defeated
	bool team_is_defeated(const team& t) const;
};

void swap(game_board & one, game_board & other);


/**
 * This object is used to temporary place a unit in the unit map, swapping out
 * any unit that is already there.  On destruction, it restores the unit map to
 * its original.
 */
struct temporary_unit_placer
{
	temporary_unit_placer(unit_map& m, const map_location& loc, unit& u);
	temporary_unit_placer(game_board& m, const map_location& loc, unit& u);
	virtual  ~temporary_unit_placer();

private:
	unit_map& m_;
	const map_location loc_;
	unit_ptr temp_;
};

// Begin Temporary Unit Move Structs
// TODO: Fix up the implementations which use game_board

/**
 * This object is used to temporary remove a unit from the unit map.
 * On destruction, it restores the unit map to its original.
 * unit_map iterators to this unit must not be accessed while the unit is temporarily
 * removed, otherwise a collision will happen when trying to reinsert the unit.
 */
struct temporary_unit_remover
{
	temporary_unit_remover(unit_map& m, const map_location& loc);
	temporary_unit_remover(game_board& m, const map_location& loc);
	virtual  ~temporary_unit_remover();

private:
	unit_map& m_;
	const map_location loc_;
	unit_ptr temp_;
};


/**
 * This object is used to temporary move a unit in the unit map, swapping out
 * any unit that is already there.  On destruction, it restores the unit map to
 * its original.
 */
struct temporary_unit_mover
{
	temporary_unit_mover(unit_map& m, const map_location& src,
	                     const map_location& dst, int new_moves);
	temporary_unit_mover(unit_map& m, const map_location& src,
	                     const map_location& dst);
	temporary_unit_mover(game_board& b, const map_location& src,
	                     const map_location& dst, int new_moves);
	temporary_unit_mover(game_board& b, const map_location& src,
	                     const map_location& dst);
	virtual  ~temporary_unit_mover();

private:
	unit_map& m_;
	const map_location src_;
	const map_location dst_;
	int old_moves_;
	unit_ptr temp_;
};
