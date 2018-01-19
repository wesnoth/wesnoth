/*
 Copyright (C) 2010 - 2018 by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
 Part of the Battle for Wesnoth Project http://www.wesnoth.org

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
 */

#pragma once

#include "side_actions.hpp"

#include "units/map.hpp"

#include <boost/dynamic_bitset.hpp>

class CKey;
class team;

namespace pathfind {
	struct marked_route;
}

namespace wb {

class mapbuilder;
class highlighter;

/**
 * This class is the frontend of the whiteboard framework for the rest of the Wesnoth code.
 */
class manager
{
	friend struct future_map;
	friend struct future_map_if_active;
	friend struct real_map;

public:
	manager(const manager&) = delete;
	manager& operator=(const manager&) = delete;

	manager();
	~manager();

	void print_help_once();

	/** Determine whether the game is initialized and the current side has control of the game
	 *  i.e. the whiteboard can take over
	 */
	bool can_modify_game_state() const;
	/** Determine whether the whiteboard can be activated safely */
	bool can_activate() const;
	/** Determine whether the whiteboard is activated. */
	bool is_active() const { return active_; }
	/** Activates/Deactivates the whiteboard*/
	void set_active(bool active);
	/** Called by the key that temporarily toggles the activated state when held */
	void set_invert_behavior(bool invert);
	/** Prevents the whiteboard from changing its activation state, as long as the returned reference is held */
	whiteboard_lock get_activation_state_lock() { return activation_state_lock_; }

	/** Is the whiteboard in the process of executing an action? */
	bool is_executing_actions() const { return executing_actions_; }

	/** Used to ask the whiteboard if its action execution hotkeys should be available to the user */
	bool can_enable_execution_hotkeys() const;
	/** Used to ask the whiteboard if hotkeys affecting the action queue should be available to the user */
	bool can_enable_modifier_hotkeys() const;
	/** Used to ask the whiteboard if its action reordering hotkeys should be available to the user */
	bool can_enable_reorder_hotkeys() const;
	/** Used to ask permission to the wb to move a leader, to avoid invalidating planned recruits */
	bool allow_leader_to_move(unit const& leader) const;
	/** @ return true if the whiteboard is ready to end turn. Triggers the execution of remaining planned actions. */
	bool allow_end_turn();
	/**
	 * The on_* methods below inform the whiteboard of specific events
	 */
	void on_init_side();
	void on_finish_side_turn(int side);
	void on_mouseover_change(const map_location& hex);
	void on_deselect_hex(){ erase_temp_move();}
	void on_gamestate_change();
	void on_viewer_change(size_t team_index);
	void on_change_controller(int side, const team& t);
	void on_kill_unit();
	/** Handles various cleanup right before removing an action from the queue */
	void pre_delete_action(action_ptr action);
	/** Handles various cleanup right after removing an action from the queue */
	void post_delete_action(action_ptr action);

	/** Called by replay_network_sender to add whiteboard data to the outgoing network packets */
	void send_network_data();
	/** Called by turn_info::process_network_data() when network data needs to be processed */
	void process_network_data(config const&);
	/** Adds a side_actions::net_cmd to net_buffer_[team_index], whereupon it will (later) be sent to all allies */
	void queue_net_cmd(size_t team_index, side_actions::net_cmd const&);

	/** Whether the current side has actions in the first turn of its planned actions queue */
	static bool current_side_has_actions();

	/** Validates all actions of the current viewing side */
	void validate_viewer_actions();

	/** Whether the planned unit map is currently applied */
	bool has_planned_unit_map() const { return planned_unit_map_active_; }


	/**
	 * Called from the display before drawing.
	 */
	void pre_draw();
	/**
	 * Called from the display after drawing.
	 */
	void post_draw();
	/**
	 * Called from the display when drawing hexes, to allow the whiteboard to
	 * add visual elements. Some visual elements such as arrows and fake units
	 * are not handled through this function, but separately registered with the display.
	 */
	void draw_hex(const map_location& hex);

	/** Creates a temporary visual arrow, that follows the cursor, for move creation purposes */
	void create_temp_move();
	/** Informs whether an arrow is being displayed for move creation purposes */
	bool has_temp_move() const { return route_ && !fake_units_.empty() && !move_arrows_.empty(); }
	/** Erase the temporary arrow */
	void erase_temp_move();
	/** Creates a move action for the current side, and erases the temp move.
	 *  The move action is inserted at the end of the queue, to be executed last. */
	void save_temp_move();
	/** @return an iterator to the unit that owns the temp move, resources::gameboard->units().end() if there's none. */
	unit_map::iterator get_temp_move_unit() const;

	/** Creates an attack or attack-move action for the current side */
	void save_temp_attack(const map_location& attacker_loc, const map_location& defender_loc, int weapon_choice);

	/** Creates a recruit action for the current side
	 *  @return true if manager has saved a planned recruit */
	bool save_recruit(const std::string& name, int side_num, const map_location& recruit_hex);

	/** Creates a recall action for the current side
	 *  @return true if manager has saved a planned recall */
	bool save_recall(const unit& unit, int side_num, const map_location& recall_hex);

	/** Creates a suppose-dead action for the current side */
	void save_suppose_dead(unit& curr_unit, map_location const& loc);

	/** Executes first action in the queue for current side */
	void contextual_execute();
	/** Executes all actions for the current turn in sequence
	 *  @return true if the there are no more actions left for this turn when the method returns */
	bool execute_all_actions();
	/** Deletes last action in the queue for current side */
	void contextual_delete();
	/** Moves the action determined by the UI toward the beginning of the queue  */
	void contextual_bump_up_action();
	/** Moves the action determined by the UI toward the beginning of the queue  */
	void contextual_bump_down_action();

	/** Get the highlight visitor instance in use by the manager */
	std::weak_ptr<highlighter> get_highlighter() { return highlighter_; }

	/** Checks whether the whiteboard has any planned action on any team */
	bool has_actions() const;
	/** Checks whether the specified unit has at least one planned action */
	bool unit_has_actions(unit const* unit) const;

	/** Used to track gold spending per-side when building the planned unit map
	 *  Is referenced by the top bar gold display */
	int get_spent_gold_for(int side);

	/** Determines whether or not the undo_stack should be cleared.
	 *  @todo When there are network allies, only clear the undo stack when we have set a preferences option */
	bool should_clear_undo() const;

	/** Displays the whiteboard options dialog. */
	void options_dlg();

private:
	/** Transforms the unit map so that it now reflects the future state of things,
	 *  i.e. when all planned actions will have been executed */
	void set_planned_unit_map();
	/** Restore the regular unit map */
	void set_real_unit_map();

	void validate_actions_if_needed();
	/** Called by all of the save_***() methods after they have added their action to the queue */
	void update_plan_hiding(size_t viewing_team);
	void update_plan_hiding(); //same as above, but uses wb::viewer_team() as default argument

	/** Tracks whether the whiteboard is active. */
	bool active_;
	bool inverted_behavior_;
	bool self_activate_once_;
#if 0
	bool print_help_once_;
#endif
	bool wait_for_side_init_;
	bool planned_unit_map_active_;
	/** Track whenever we're modifying actions, to avoid dual execution etc. */
	bool executing_actions_;
	/** Track whether we're in the process of executing all actions */
	bool executing_all_actions_;
	/** true if we're in the process of executing all action and should end turn once finished. */
	bool preparing_to_end_turn_;
	/** Track whether the gamestate changed and we need to validate actions. */
	bool gamestate_mutated_;

	/** Reference counted "lock" to allow preventing whiteboard activation state changes */
	whiteboard_lock activation_state_lock_;
	/** Reference counted "lock" to prevent the building of the unit map at certain times */
	whiteboard_lock unit_map_lock_;


	std::unique_ptr<mapbuilder> mapbuilder_;
	std::shared_ptr<highlighter> highlighter_;

	std::unique_ptr<pathfind::marked_route> route_;

	std::vector<arrow_ptr> move_arrows_;
	std::vector<fake_unit_ptr> fake_units_;
	size_t temp_move_unit_underlying_id_;

	const std::unique_ptr<CKey> key_poller_;

	std::vector<map_location> hidden_unit_hexes_;

	///net_buffer_[i] = whiteboard network data to be sent "from" teams[i].
	std::vector<config> net_buffer_;

	///team_plans_hidden_[i] = whether or not to hide actions from teams[i].
	boost::dynamic_bitset<> team_plans_hidden_;

	///used to keep track of units owning planned moves for visual ghosting/unghosting
	std::set<size_t> units_owning_moves_;
};

/** Applies the planned unit map for the duration of the struct's life.
 *  Reverts to real unit map on destruction, unless planned unit map was already applied when the struct was created. */
struct future_map
{
	future_map();
	~future_map();
	bool initial_planned_unit_map_;
};

struct future_map_if
{
	const std::unique_ptr<future_map> future_map_;

	/** @param cond If true, applies the planned unit map for the duration of the struct's life and reverts to real unit map on destruction.
			No effect if cond == false.
	*/
	future_map_if(bool cond)
		: future_map_(cond ? new future_map() : nullptr)
	{}
};

/** ONLY IF whiteboard is currently active, applies the planned unit map for the duration of the struct's life.
 *  Reverts to real unit map on destruction, unless planned unit map was already applied when the struct was created. */
struct future_map_if_active
{
	future_map_if_active();
	~future_map_if_active();
	bool initial_planned_unit_map_;
	bool whiteboard_active_;
};

/** Ensures that the real unit map is active for the duration of the struct's life.
 *  On destruction reverts to planned unit map if it was active when the struct was created. */
struct real_map
{
	real_map();
	~real_map();
	bool initial_planned_unit_map_;
	whiteboard_lock unit_map_lock_;
};

} // end namespace wb
