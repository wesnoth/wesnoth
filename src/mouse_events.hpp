/*
	Copyright (C) 2006 - 2024
	by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
	Copyright (C) 2003 by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "game_display.hpp"             // for game_display -> display conversion.
#include "map/location.hpp"             // for map_location
#include "mouse_handler_base.hpp"       // for mouse_handler_base
#include "pathfind/pathfind.hpp"        // for marked_route, paths
#include "units/map.hpp"                 // for unit_map, etc

#include <set>                          // for set
#include <vector>                       // for vector
#include <SDL2/SDL_events.h>                 // for SDL_MouseButtonEvent

class battle_context;  // lines 23-23
class play_controller;
class team;
class unit;

namespace events{

class mouse_handler : public mouse_handler_base {
public:
	mouse_handler(game_display* gui, play_controller & pc);
	~mouse_handler();
	static mouse_handler* get_singleton() { return singleton_ ;}
	void set_side(int side_number);
	void cycle_units(const bool browse, const bool reverse = false);
	void cycle_back_units(const bool browse) { cycle_units(browse, true); }

	int get_path_turns() const { return path_turns_; }

	/**
	 * @param loc the location occupied by the enemy
	 * @returns the location from which the selected unit can attack the enemy
	 */
	map_location current_unit_attacks_from(const map_location& loc) const;
	const pathfind::paths& current_paths() const { return current_paths_; }

	const map_location& get_last_hex() const { return last_hex_; }
	map_location get_selected_hex() const { return selected_hex_; }
	void set_path_turns(const int path_turns) { path_turns_ = path_turns; }
	void set_current_paths(const pathfind::paths & new_paths);
	void deselect_hex();
	void invalidate_reachmap() { reachmap_invalid_ = true; }

	void set_gui(game_display* gui) { gui_ = gui; }

	unit_map::iterator selected_unit();

	pathfind::marked_route get_route(const unit* un, map_location go_to, const team &team) const;

	const pathfind::marked_route& get_current_route() const { return current_route_; }

	//get visible adjacent enemies of 1-based side around location loc
	std::set<map_location> get_adj_enemies(const map_location& loc, int side) const;

	// show the attack dialog and return the choice made
	// which can be invalid if 'cancel' was used
	int show_attack_dialog(const map_location& attacker_loc, const map_location& defender_loc);
	// wrapper to catch bad_alloc so this should be called
	void attack_enemy(const map_location& attacker_loc, const map_location& defender_loc, int choice);

	/** Moves a unit across the board for a player. */
	std::size_t move_unit_along_route(const std::vector<map_location> & steps, bool & interrupted);

	void select_hex(const map_location& hex, const bool browse,
		const bool highlight = true,
		const bool fire_event = true,
		const bool force_unhighlight = false);

	void move_action(bool browse) override;
	void teleport_action();

	void touch_action(const map_location hex, bool browse) override;

	void select_or_action(bool browse);
	void select_teleport();

	/**
	 * Uses SDL and @ref game_display::hex_clicked_on
	 * to fetch the hex the mouse is hovering, if applicable.
	 */
	const map_location hovered_hex() const;

	/** Unit exists on the hex, no matter if friend or foe. */
	bool hex_hosts_unit(const map_location& hex) const;

	/**
	 * Use this to disable hovering an unit from highlighting its movement
	 * range.
	 *
	 * @see enable_units_highlight()
	 */
	void disable_units_highlight();

	/**
	 * When unit highlighting is disabled, call this when the mouse no
	 * longer hovers any unit to enable highlighting again.
	 *
	 * @see disable_units_highlight()
	 */
	void enable_units_highlight();

protected:
	/**
	 * Due to the way this class is constructed we can assume that the
	 * display* gui_ member actually points to a game_display (derived class)
	 */
	game_display& gui() override { return *gui_; }
	/** Const version */
	const game_display& gui() const override { return *gui_; }

	int drag_threshold() const override;
	/**
	 * Use update to force an update of the mouse state.
	 */
	void mouse_motion(int x, int y, const bool browse, bool update=false, map_location loc = map_location::null_location()) override;
	bool mouse_button_event(const SDL_MouseButtonEvent& event, uint8_t button, map_location loc, bool click = false) override;
	bool right_click_show_menu(int x, int y, const bool browse) override;
//	bool left_click(int x, int y, const bool browse);
	bool move_unit_along_current_route();

	void touch_motion(int x, int y, const bool browse, bool update=false, map_location loc = map_location::null_location()) override;

	void save_whiteboard_attack(const map_location& attacker_loc, const map_location& defender_loc, int weapon_choice);

	// fill weapon choices into bc_vector
	// return the best weapon choice
	int fill_weapon_choices(std::vector<battle_context>& bc_vector, const unit_map::iterator& attacker, const unit_map::iterator& defender);
	// the real function but can throw bad_alloc
	// choice is the attack chosen in the attack dialog
	void attack_enemy_(const map_location& attacker_loc
			, const map_location& defender_loc
			, int choice);

	void show_attack_options(const unit_map::const_iterator &u);
	unit_map::const_iterator find_unit(const map_location& hex) const;
	unit_map::iterator find_unit(const map_location& hex);
	/*
	 * These return raw pointers instead of smart pointers.
	 * Useful if you don't want to increase the unit reference count.
	 */
	unit* find_unit_nonowning(const map_location& hex);
	const unit* find_unit_nonowning(const map_location& hex) const;
	bool unit_in_cycle(const unit_map::const_iterator& it);
private:
	team &current_team();

	// Some common code from mouse_motion and touch_motion.
	/**
	 * Highlight the hexes that a unit can move to.
	 *
	 * Based on the currently selected hex, selected unit and what's being moused-over,
	 * conditionally draw any planned moves for the unit passed as an argument.
	 */
	void show_reach_for_unit(const unit_ptr& un);

	game_display* gui_;
	play_controller & pc_;

	// previous highlighted hexes
	// the hex of the selected unit and empty hex are "free"
	map_location previous_hex_;
	map_location previous_free_hex_;
	map_location selected_hex_;
	map_location next_unit_;
	pathfind::marked_route current_route_;
	/**
	 * If non-empty, current_paths_.destinations contains a cache of highlighted
	 * hexes, likely the movement range or attack range of a unit.
	 */
	pathfind::paths current_paths_;
	bool unselected_paths_;
	bool unselected_reach_;
	int path_turns_;
	int side_num_;

	bool over_route_;
	bool reachmap_invalid_;
	bool show_partial_move_;
	bool teleport_selected_;

	static mouse_handler * singleton_;

	bool preventing_units_highlight_;
};

}
