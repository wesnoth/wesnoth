/* $Id$ */
/*
 Copyright (C) 2010 by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
 Part of the Battle for Wesnoth Project http://www.wesnoth.org

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 2
 or at your option any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY.

 See the COPYING file for more details.
 */

/**
 * @file
 */

#ifndef WB_MANAGER_HPP_
#define WB_MANAGER_HPP_

#include "typedefs.hpp"

#include "map_location.hpp"

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

#include <vector>

class CKey;

namespace pathfind {
	struct marked_route;
}

namespace wb {

class mapbuilder_visitor;
class highlight_visitor;

/**
 * This class holds and manages all of the whiteboard's planned actions.
 */
class manager : private boost::noncopyable
{
public:

	manager();
	~manager();

	void print_help_once();

	///Determine whether the whiteboard is activated.
	bool is_active() const { return active_; }
	///Activates/Deactivates the whiteboard
	void set_active(bool active);
	///Called by the key that temporarily toggles the activated state when held
	void set_invert_behavior(bool invert);

	///Used to ask the whiteboard if its general purpose hotkeys can be called now
	bool can_execute_hotkey() const;
	///Used to ask the whiteboard if its action reordering hotkeys can be called now
	bool can_reorder_action() const;
	///Used to ask permission to the wb to move a leader, to avoid invalidating planned recruits
	bool allow_leader_to_move(unit const& leader) const;

	/**
	 * The on_* methods below inform the whiteboard of specific events
	 */
	void on_init_side(bool is_replay);
	void on_finish_side_turn();
	void on_mouseover_change(const map_location& hex);
	void on_select_hex(){}
	void on_deselect_hex(){ erase_temp_move();}
	void on_gamestate_change();

	/// Whether the current side has actions in its planned actions queue
	static bool current_side_has_actions();

	/// Validates all actions of the current viewing side
	void validate_viewer_actions();

	/// Transforms the unit map so that it now reflects the future state of things,
	/// i.e. when all planned actions will have been executed
	/// @param for_pathfinding Whether to include recruits and recalls when building the future map
	void set_planned_unit_map(bool for_pathfinding = false);
	/// Restore the regular unit map
	void set_real_unit_map();
	/// Whether the planned unit map is currently applied
	bool has_planned_unit_map() const { return planned_unit_map_active_; }
	/// Whether the planned unit map currently applied (if any) is
	/// the "pathfinding" version, i.e. one that includes future recruits and recalls
	bool is_map_for_pathfinding() const { return is_map_for_pathfinding_; }

	///Applies the future unit map and @return a pointer to the unit at hex,
	///NULL if none is visible to the specified viewer side
	static unit* future_visible_unit(map_location hex, int viewer_side = viewer_side());
	///Applies the future unit map and @return a pointer to the unit at hex,
	///NULL if none is visible to the specified viewer side
	/// @param on_side Only search for units of this side.
	static unit* future_visible_unit(int on_side, map_location hex, int viewer_side = viewer_side());

	/**
	 * Callback from the display when drawing hexes, to allow the whiteboard to
	 * add visual elements. Some visual elements such as arrows and fake units
	 * are not handled through this function, but separately registered with the display.
	 */
	void draw_hex(const map_location& hex);

	/// Creates a temporary visual arrow, that follows the cursor, for move creation purposes
	void create_temp_move();
	/// Informs whether an arrow is being displayed for move creation purposes
	bool has_temp_move() const { return route_ && fake_unit_ && move_arrow_; }
	/// Erase the temporary arrow
	void erase_temp_move();
	/// Creates a move action for the current side, and erases the temp move.
	/// The move action is inserted at the end of the queue, to be executed last.
	void save_temp_move();

	/// Creates an attack or attack-move action for the current side
	void save_temp_attack(const map_location& attack_from, const map_location& target_hex);

	/// Creates a recruit action for the current side
	/// @return true if manager has saved a planned recruit
	bool save_recruit(const std::string& name, int side_num, const map_location& recruit_hex);

	/// Creates a recall action for the current side
	/// @return true if manager has saved a planned recall
	bool save_recall(const unit& unit, int side_num, const map_location& recall_hex);

	/** Executes first action in the queue for current side */
	void contextual_execute();
	/** Deletes last action in the queue for current side */
	void contextual_delete();
	/** Moves the action determined by the UI toward the beginning of the queue  */
	void contextual_bump_up_action();
	/** Moves the action determined by the UI toward the beginning of the queue  */
	void contextual_bump_down_action();
	/** Deletes all planned actions for all teams */
	void erase_all_actions();

	/// Get the highlight visitor instance in use by the manager
	boost::weak_ptr<highlight_visitor> get_highlighter() { return highlighter_; }

	/** Checks whether the specified unit has at least one planned action */
	bool unit_has_actions(unit const* unit) const;

	/// Used to track gold spending per-side when building the planned unit map
	/// Is referenced by the top bar gold display
	int get_spent_gold_for(int side);

	/// Utility struct used to automatize memory management for the whiteboard's fake/ghosted units
	struct fake_unit_deleter {
	    void operator() (unit*& ptr);
	};

private:
	void validate_actions_if_needed();

	/// returns resources::screen->viewing_team()
	static size_t viewer_team();
	/// returns resources::screen->viewing_side()
	static int viewer_side();
	static side_actions_ptr viewer_actions();
	static side_actions_ptr current_side_actions();

	///Tracks whether the whiteboard is active.
	bool active_;
	bool inverted_behavior_;
	bool self_activate_once_;
	bool print_help_once_;
	bool wait_for_side_init_;
	bool planned_unit_map_active_;
	bool is_map_for_pathfinding_;
	/** Track whenever we're modifying actions, to avoid dual execution etc. */
	bool executing_actions_;
	/** Track whether the gamestate changed and we need to validate actions. */
	bool gamestate_mutated_;

	boost::scoped_ptr<mapbuilder_visitor> mapbuilder_;
	boost::shared_ptr<highlight_visitor> highlighter_;

	boost::scoped_ptr<pathfind::marked_route> route_;

	arrow_ptr move_arrow_;
	fake_unit_ptr fake_unit_;

	boost::scoped_ptr<CKey> key_poller_;

	map_location hidden_unit_hex_;
};

/** Applies the planned unit map for the duration of the struct's life.
 *  Reverts to real unit map on exit, no matter what was the status when the struct was created. */
struct scoped_planned_unit_map
{
	scoped_planned_unit_map();
	~scoped_planned_unit_map();
	bool has_planned_unit_map_;
	bool is_map_for_pathfinding_;
};

/** Ensures that the real unit map is active for the duration of the struct's life,
 * and reverts to planned unit map if it was active when the struct was created. */
struct scoped_real_unit_map
{
	scoped_real_unit_map();
	~scoped_real_unit_map();
	bool has_planned_unit_map_;
	bool is_map_for_pathfinding_;
};

/** A variant on the regular planned unit map, that includes units only useful for pathfinding,
 * such as those from planned recruits and recalls.
 * It replaces any the regular planned unit map, and rebuilds it afterwards if needed.
 *  */
struct scoped_planned_pathfind_map
{
	scoped_planned_pathfind_map();
	~scoped_planned_pathfind_map();
	bool has_planned_unit_map_;
	bool is_map_for_pathfinding_;
};

/// Predicate that compares the id() of two units. Useful for searches in unit vectors with std::find_if()
struct unit_comparator_predicate {
	unit_comparator_predicate(unit const& unit) : unit_(unit) {}
	bool operator()(unit const& unit);
private:
	unit const& unit_;
};

} // end namespace wb

#endif /* WB_MANAGER_HPP_ */
