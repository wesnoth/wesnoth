/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
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
 * @file
 * Undoing, redoing.
 */

#include "undo.hpp"

#include "create.hpp"
#include "move.hpp"

#include "../game_display.hpp"
#include "../log.hpp"
#include "../play_controller.hpp"
#include "../replay.hpp"
#include "../replay_helper.hpp"
#include "../resources.hpp"
#include "../team.hpp"
#include "../unit.hpp"
#include "../unit_display.hpp"
#include "../unit_map.hpp"
#include "../whiteboard/manager.hpp"
#include "../synced_context.hpp"

#include <boost/foreach.hpp>
#include <cassert>

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)


namespace actions {


/**
 * Virtual destructor
 */
undo_list::undo_action::~undo_action()
{
	delete view_info;
}


struct undo_list::dismiss_action : undo_list::undo_action {
	unit dismissed_unit;


	explicit dismiss_action(const unit& dismissed) : undo_action(),
		dismissed_unit(dismissed)
	{}
	explicit dismiss_action(const config & unit_cfg) : undo_action(),
		dismissed_unit(unit_cfg)
	{}
	virtual ~dismiss_action();

	/// Writes this into the provided config.
	virtual void write(config & cfg) const;

	/// Undoes this action.
	virtual bool undo(int side, undo_list & undos);
	/// Redoes this action.
	virtual bool redo(int side);
};
undo_list::dismiss_action::~dismiss_action()
{}

struct undo_list::move_action : undo_list::undo_action {
	int starting_moves;
	int original_village_owner;
	int countdown_time_bonus;
	map_location::DIRECTION starting_dir;
	map_location goto_hex;


	move_action(const unit& moved,
	            const std::vector<map_location>::const_iterator & begin,
	            const std::vector<map_location>::const_iterator & end,
	            int sm, int timebonus, int orig, const map_location::DIRECTION dir) :
		undo_action(moved, begin, end),
		starting_moves(sm),
		original_village_owner(orig),
		countdown_time_bonus(timebonus),
		starting_dir(dir == map_location::NDIRECTIONS ? moved.facing() : dir),
		goto_hex(moved.get_goto())
	{}
	move_action(const config & unit_cfg, const config & route_cfg,
	            int sm, int timebonus, int orig, const map_location::DIRECTION dir) :
		undo_action(unit_cfg),
		starting_moves(sm),
		original_village_owner(orig),
		countdown_time_bonus(timebonus),
		starting_dir(dir),
		goto_hex(unit_cfg["goto_x"].to_int(-999) - 1,
		         unit_cfg["goto_y"].to_int(-999) - 1)
	{
		read_locations(route_cfg, route);
	}
	virtual ~move_action();

	/// Writes this into the provided config.
	virtual void write(config & cfg) const;

	/// Undoes this action.
	virtual bool undo(int side, undo_list & undos);
	/// Redoes this action.
	virtual bool redo(int side);
};
undo_list::move_action::~move_action()
{}

struct undo_list::recall_action : undo_list::undo_action {
	std::string id;
	map_location recall_from;


	recall_action(const unit& recalled, const map_location& loc,
	              const map_location& from) :
		undo_action(recalled, loc),
		id(recalled.id()),
		recall_from(from)
	{}
	recall_action(const config & unit_cfg, const map_location & loc,
		          const map_location & from) :
		undo_action(unit_cfg, loc),
		id(unit_cfg["id"]),
		recall_from(from)
	{}
	virtual ~recall_action();

	/// Writes this into the provided config.
	virtual void write(config & cfg) const;

	/// Undoes this action.
	virtual bool undo(int side, undo_list & undos);
	/// Redoes this action.
	virtual bool redo(int side);
};
undo_list::recall_action::~recall_action()
{}

struct undo_list::recruit_action : undo_list::undo_action {
	const unit_type & u_type;
	map_location recruit_from;


	recruit_action(const unit& recruited, const map_location& loc,
	               const map_location& from) :
		undo_action(recruited, loc),
		u_type(recruited.type()),
		recruit_from(from)
	{}
	recruit_action(const config & unit_cfg, const unit_type & type,
	               const map_location& loc, const map_location& from) :
		undo_action(unit_cfg, loc),
		u_type(type),
		recruit_from(from)
	{}
	virtual ~recruit_action();

	/// Writes this into the provided config.
	virtual void write(config & cfg) const;

	/// Undoes this action.
	virtual bool undo(int side, undo_list & undos);
	/// Redoes this action.
	virtual bool redo(int side);
};
undo_list::recruit_action::~recruit_action()
{}

struct undo_list::auto_shroud_action : undo_list::undo_action {
	bool active;


	explicit auto_shroud_action(bool turned_on) :
		undo_action(),
		active(turned_on)
	{}
	virtual ~auto_shroud_action();

	/// Writes this into the provided config.
	virtual void write(config & cfg) const;

	/// Undoes this action.
	virtual bool undo(int side, undo_list & undos);
	/// Redoes this action.
	virtual bool redo(int side);
};
undo_list::auto_shroud_action::~auto_shroud_action()
{}

struct undo_list::update_shroud_action : undo_list::undo_action {
	// No additional data.

	update_shroud_action() : undo_action() {}
	virtual ~update_shroud_action();

	/// Writes this into the provided config.
	virtual void write(config & cfg) const;

	/// Undoes this action.
	virtual bool undo(int side, undo_list & undos);
	/// Redoes this action.
	virtual bool redo(int side);
};
undo_list::update_shroud_action::~update_shroud_action()
{}


/**
 * Creates an undo_action based on a config.
 * @param  tag  is the tag of this config, which is used for error reporting.
 *              It should be enclosed in square brackets.
 * @return a pointer that must be deleted, or NULL if the @a cfg could not be parsed.
 */
undo_list::undo_action *
undo_list::undo_action::create(const config & cfg, const std::string & tag)
{
	const std::string str = cfg["type"];

	// The general division of labor in this function is that the various
	// constructors will parse the "unit" child config, while this function
	// parses everything else.

	if ( str == "move" )
		return new move_action(cfg.child("unit", tag), cfg,
		                       cfg["starting_moves"],
		                       cfg["time_bonus"],
		                       cfg["village_owner"],
		                       map_location::parse_direction(cfg["starting_direction"]));

	if ( str == "recruit" ) {
		// Validate the unit type.
		const config & child = cfg.child("unit", tag);
		const unit_type * u_type = unit_types.find(child["type"]);

		if ( !u_type ) {
			// Bad data.
			ERR_NG << "Invalid recruit found in " << tag << "; unit type '"
			       << child["type"] << "' was not found.\n";
			return NULL;
		}
		return new recruit_action(child, *u_type,
		                          map_location(cfg, NULL),
		                          map_location(cfg.child_or_empty("leader"), NULL));
	}

	if ( str == "recall" )
		return new recall_action(cfg.child("unit", tag),
		                         map_location(cfg, NULL),
		                         map_location(cfg.child_or_empty("leader"), NULL));

	if ( str == "dismiss" )
		return new dismiss_action(cfg.child("unit", tag));

	if ( str == "auto_shroud" )
		return new auto_shroud_action(cfg["active"].to_bool());

	if ( str == "update_shroud" )
		return new update_shroud_action;

	// Unrecognized type.
	ERR_NG << "Unrecognized undo action type: " << str << ".\n";
	return NULL;
}


/**
 * Writes this into the provided config.
 */
void undo_list::dismiss_action::write(config & cfg) const
{
	cfg["type"] = "dismiss";
	dismissed_unit.write(cfg.add_child("unit"));
}

/**
 * Writes this into the provided config.
 */
void undo_list::recall_action::write(config & cfg) const
{
	cfg["type"] = "recall";
	route.front().write(cfg);
	recall_from.write(cfg.add_child("leader"));

	config & child = cfg.add_child("unit");
	view_info->write(child);
	child["id"] = id;
}

/**
 * Writes this into the provided config.
 */
void undo_list::recruit_action::write(config & cfg) const
{
	cfg["type"] = "recruit";
	route.front().write(cfg);
	recruit_from.write(cfg.add_child("leader"));

	config & child = cfg.add_child("unit");
	view_info->write(child);
	child["type"] = u_type.base_id();
}

/**
 * Writes this into the provided config.
 */
void undo_list::move_action::write(config & cfg) const
{
	cfg["type"] = "move";
	cfg["starting_direction"] = map_location::write_direction(starting_dir);
	cfg["starting_moves"] = starting_moves;
	cfg["time_bonus"] = countdown_time_bonus;
	cfg["village_owner"] = original_village_owner;
	write_locations(route, cfg);

	config & child = cfg.add_child("unit");
	view_info->write(child);
	child["goto_x"] = goto_hex.x + 1;
	child["goto_y"] = goto_hex.y + 1;
}

/**
 * Writes this into the provided config.
 */
void undo_list::auto_shroud_action::write(config & cfg) const
{
	cfg["type"] = "auto_shroud";
	cfg["active"] = active;
}

/**
 * Writes this into the provided config.
 */
void undo_list::update_shroud_action::write(config & cfg) const
{
	cfg["type"] = "update_shroud";
}



/**
 * Constructor.
 * The config is allowed to be invalid.
 */
undo_list::undo_list(const config & cfg) :
	undos_(), redos_(), side_(1), committed_actions_(false)
{
	if ( cfg )
		read(cfg);
}

/**
 * Destructor.
 */
undo_list::~undo_list()
{
	// Default destructor, but defined out-of-line to localize the templating.
	// (Might make compiles faster.)
}


/**
 * Adds an auto-shroud toggle to the undo stack.
 */
void undo_list::add_auto_shroud(bool turned_on)
{
	/// @todo: Consecutive shroud actions can be collapsed into one.

	// Do not call add(), as this should not clear the redo stack.
	undos_.push_back(new auto_shroud_action(turned_on));
}

/**
 * Adds a dismissal to the undo stack.
 */
void undo_list::add_dismissal(const unit & u)
{
	add(new dismiss_action(u));
}

/**
 * Adds a move to the undo stack.
 */
void undo_list::add_move(const unit& u,
                         const std::vector<map_location>::const_iterator & begin,
                         const std::vector<map_location>::const_iterator & end,
                         int start_moves, int timebonus, int village_owner,
                         const map_location::DIRECTION dir)
{
	add(new move_action(u, begin, end, start_moves, timebonus, village_owner, dir));
}

/**
 * Adds a recall to the undo stack.
 */
void undo_list::add_recall(const unit& u, const map_location& loc,
                           const map_location& from)
{
	add(new recall_action(u, loc, from));
}

/**
 * Adds a recruit to the undo stack.
 */
void undo_list::add_recruit(const unit& u, const map_location& loc,
                            const map_location& from)
{
	add(new recruit_action(u, loc, from));
}

/**
 * Adds a shroud update to the undo stack.
 * This is called from within commit_vision(), so there should be no need
 * for this to be publicly visible.
 */
void undo_list::add_update_shroud()
{
	/// @todo: Consecutive shroud actions can be collapsed into one.

	// Do not call add(), as this should not clear the redo stack.
	undos_.push_back(new update_shroud_action());
}


/**
 * Clears the stack of undoable (and redoable) actions.
 * (Also handles updating fog/shroud if needed.)
 * Call this if an action alters the game state, but add that action to the
 * stack before calling this (if the action is a kind that can be undone).
 * This may fire events and change the game state.
 */
void undo_list::clear()
{
	// The fact that this function was called indicates that something was done.
	// (Some actions, such as attacks, are never put on the stack.)
	committed_actions_ = true;

	// We can save some overhead by not calling apply_shroud_changes() for an
	// empty stack.
	if ( !undos_.empty() ) {
		apply_shroud_changes();
		undos_.clear();
	}
	// No special handling for redos, so just clear that stack.
	redos_.clear();
}


/**
 * Updates fog/shroud based on the undo stack, then updates stack as needed.
 * Call this when "updating shroud now".
 * This may fire events and change the game state.
 * @param[in]  is_replay  Set to true when this is called during a replay.
 */
void undo_list::commit_vision(bool /*is_replay*/)
{
	// Update fog/shroud.
	size_t erase_to = apply_shroud_changes();

	if ( erase_to != 0 ) {
		// The actions that led to information being revealed can no longer
		// be undone.
		undos_.erase(undos_.begin(), undos_.begin() + erase_to);
		committed_actions_ = true;
	}

	// Record that vision was updated.
	add_update_shroud();
}


/**
 * Performs some initializations and error checks when starting a new side-turn.
 * @param[in]  side  The side whose turn is about to start.
 */
void undo_list::new_side_turn(int side)
{
	// Error checks.
	if ( !undos_.empty() ) {
		ERR_NG << "Undo stack not empty in new_side_turn().\n";
		// At worst, someone missed some sighted events, so try to recover.
		undos_.clear();
		redos_.clear();
	}
	else if ( !redos_.empty() ) {
		ERR_NG << "Redo stack not empty in new_side_turn().\n";
		// Sloppy tracking somewhere, but not critically so.
		redos_.clear();
	}

	// Reset the side.
	side_ = side;
	committed_actions_ = false;
}


/**
 * Read the undo_list from the provided config.
 * Currently, this is only used when the undo_list is empty, but in theory
 * it could be used to append the config to the current data.
 */
void undo_list::read(const config & cfg)
{
	// Merge header data.
	side_ = cfg["side"].to_int(side_);
	committed_actions_ = committed_actions_ || cfg["committed"].to_bool();

	// Build the undo stack.
	BOOST_FOREACH( const config & child, cfg.child_range("undo") ) {
		undo_action * action = undo_action::create(child, "[undo]");
		if ( action )
			undos_.push_back(action);
	}

	// Build the redo stack.
	BOOST_FOREACH( const config & child, cfg.child_range("redo") ) {
		undo_action * action = undo_action::create(child, "[redo]");
		if ( action )
			redos_.push_back(action);
	}
}


/**
 * Write the undo_list into the provided config.
 */
void undo_list::write(config & cfg) const
{
	cfg["side"] = side_;
	cfg["committed"] = committed_actions_;

	for ( action_list::const_iterator it = undos_.begin(); it != undos_.end(); ++it )
		it->write(cfg.add_child("undo"));

	for ( action_list::const_iterator it = redos_.begin(); it != redos_.end(); ++it )
		it->write(cfg.add_child("redo"));
}


/**
 * Undoes the top action on the undo stack.
 */
void undo_list::undo()
{
	if ( undos_.empty() )
		return;

	const events::command_disabler disable_commands;

	game_display & gui = *resources::screen;

	// Get the action to undo. (This will be placed on the redo stack, but
	// only if the undo is successful.)
	action_list::auto_type action = undos_.pop_back();

	if ( !action->undo(side_, *this) )
		return;

	// Bookkeeping.
	recorder.undo_cut(action->get_replay_data());
	redos_.push_back(action.release());
	resources::whiteboard->on_gamestate_change();

	// Screen updates.
	gui.invalidate_unit();
	gui.invalidate_game_status();
	gui.redraw_minimap();
	gui.draw();
}

/**
 * Undoes this action.
 * @return true on success; false on an error.
 */
bool undo_list::dismiss_action::undo(int side, undo_list & /*undos*/)
{
	team &current_team = (*resources::teams)[side-1];

	if ( !current_team.persistent() ) {
		ERR_NG << "Trying to undo a dismissal for side " << side
			<< ", which has no recall list!\n";
		return false;
	}

	current_team.recall_list().push_back(dismissed_unit);
	return true;
}

/**
 * Undoes this action.
 * @return true on success; false on an error.
 */
bool undo_list::recall_action::undo(int side, undo_list & /*undos*/)
{
	game_display & gui = *resources::screen;
	unit_map &   units = *resources::units;
	team &current_team = (*resources::teams)[side-1];

	if ( !current_team.persistent() ) {
		ERR_NG << "Trying to undo a recall for side " << side
			<< ", which has no recall list!\n";
		return false;
	}

	const map_location & recall_loc = route.front();
	unit_map::iterator un_it = units.find(recall_loc);
	if ( un_it == units.end() ) {
		return false;
	}

	const unit &un = *un_it;
	statistics::un_recall_unit(un);
	int cost = statistics::un_recall_unit_cost(un);
	if (cost < 0) {
		current_team.spend_gold(-current_team.recall_cost());
	}
	else {
		current_team.spend_gold(-cost);
	}

	current_team.recall_list().push_back(un);
	// invalidate before erasing allow us
	// to also do the overlapped hexes
	gui.invalidate(recall_loc);
	units.erase(recall_loc);
	return true;
}

/**
 * Undoes this action.
 * @return true on success; false on an error.
 */
bool undo_list::recruit_action::undo(int side, undo_list & /*undos*/)
{
	game_display & gui = *resources::screen;
	unit_map &   units = *resources::units;
	team &current_team = (*resources::teams)[side-1];

	const map_location & recruit_loc = route.front();
	unit_map::iterator un_it = units.find(recruit_loc);
	if ( un_it == units.end() ) {
		return false;
	}

	const unit &un = *un_it;
	statistics::un_recruit_unit(un);
	current_team.spend_gold(-un.type().cost());

	//MP_COUNTDOWN take away recruit bonus
	current_team.set_action_bonus_count(current_team.action_bonus_count() - 1);

	// invalidate before erasing allow us
	// to also do the overlapped hexes
	gui.invalidate(recruit_loc);
	units.erase(recruit_loc);
	return true;
}

/**
 * Undoes this action.
 * @return true on success; false on an error.
 */
bool undo_list::move_action::undo(int side, undo_list & /*undos*/)
{
	game_display & gui = *resources::screen;
	unit_map &   units = *resources::units;
	team &current_team = (*resources::teams)[side-1];

	// Copy some of our stored data.
	const int saved_moves = starting_moves;
	std::vector<map_location> rev_route = route;
	std::reverse(rev_route.begin(), rev_route.end());

	// Check units.
	unit_map::iterator u = units.find(rev_route.front());
	const unit_map::iterator u_end = units.find(rev_route.back());
	if ( u == units.end()  ||  u_end != units.end() ) {
		//this can actually happen if the scenario designer has abused the [allow_undo] command
		ERR_NG << "Illegal 'undo' found. Possible abuse of [allow_undo]?\n";
		return false;
	}

	if ( resources::game_map->is_village(rev_route.front()) ) {
		get_village(rev_route.front(), original_village_owner + 1);
		//MP_COUNTDOWN take away capture bonus
		if ( countdown_time_bonus )
		{
			current_team.set_action_bonus_count(current_team.action_bonus_count() - 1);
		}
	}

	// Record the unit's current state so it can be redone.
	starting_moves = u->movement_left();
	goto_hex = u->get_goto();

	// Move the unit.
	unit_display::move_unit(rev_route, *u, true, starting_dir);
	units.move(u->get_location(), rev_route.back());
	unit::clear_status_caches();

	// Restore the unit's old state.
	u = units.find(rev_route.back());
	u->set_goto(map_location());
	u->set_movement(saved_moves, true);
	u->set_standing();

	gui.invalidate_unit_after_move(rev_route.front(), rev_route.back());
	return true;
}

/**
 * Undoes this action.
 * @return true on success; false on an error.
 */
bool undo_list::auto_shroud_action::undo(int /*side*/, undo_list & undos)
{
	// This does not count as an undoable action, so undo the next
	// action instead.
	recorder.undo();
	undos.undo();
	// Now keep the auto-shroud toggle at the top of the undo stack.
	recorder.add_synced_command("auto_shroud", replay_helper::get_auto_shroud(active));
	undos.add_auto_shroud(active);
	// Shroud actions never get moved to the redo stack, so claim an error.
	return false;
}

/**
 * Undoes this action.
 * @return true on success; false on an error.
 */
bool undo_list::update_shroud_action::undo(int /*side*/, undo_list & undos)
{
	// This does not count as an undoable action, so undo the next
	// action instead.
	recorder.undo();
	undos.undo();
	// Now keep the shroud update at the top of the undo stack.
	recorder.add_synced_command("update_shroud", replay_helper::get_update_shroud());
	
	undos.add_update_shroud();
	// Shroud actions never get moved to the redo stack, so claim an error.
	return false;
}


/**
 * Redoes the top action on the redo stack.
 */
void undo_list::redo()
{
	if ( redos_.empty() )
		return;

	const events::command_disabler disable_commands;

	game_display & gui = *resources::screen;

	// Get the action to redo. (This will be placed on the undo stack, but
	// only if the redo is successful.)
	action_list::auto_type action = redos_.pop_back();

	if ( !action->redo(side_) )
		return;

	// Bookkeeping.
	undos_.push_back(action.release());
	resources::whiteboard->on_gamestate_change();

	// Screen updates.
	gui.invalidate_unit();
	gui.invalidate_game_status();
	gui.redraw_minimap();
	gui.draw();
}

/**
 * Redoes this action.
 * @return true on success; false on an error.
 */
bool undo_list::dismiss_action::redo(int side)
{
	team &current_team = (*resources::teams)[side-1];

	if ( !current_team.persistent() ) {
		ERR_NG << "Trying to redo a dismissal for side " << side
			<< ", which has no recall list!\n";
		return false;
	}
	recorder.redo(replay_data);
	replay_data.clear();
	std::vector<unit>::iterator unit_it =
		find_if_matches_id(current_team.recall_list(), dismissed_unit.id());
	current_team.recall_list().erase(unit_it);
	return true;
}

/**
 * Redoes this action.
 * @return true on success; false on an error.
 */
bool undo_list::recall_action::redo(int side)
{
	game_display & gui = *resources::screen;
	team &current_team = (*resources::teams)[side-1];

	if ( !current_team.persistent() ) {
		ERR_NG << "Trying to redo a recall for side " << side
			<< ", which has no recall list!\n";
		return false;
	}

	map_location loc = route.front();
	map_location from = recall_from;

	const std::vector<unit> & recalls = current_team.recall_list();
	std::vector<unit>::const_iterator unit_it = find_if_matches_id(recalls, id);
	if ( unit_it == recalls.end() ) {
		ERR_NG << "Trying to redo a recall of '" << id
		       << "', but that unit is not in the recall list.";
		return false;
	}

	const std::string &msg = find_recall_location(side, loc, from, *unit_it);
	if ( msg.empty() ) {
		recorder.redo(replay_data);
		replay_data.clear();
		set_scontext_synced sco;
		recall_unit(id, current_team, loc, from, true, false);

		// Quick error check. (Abuse of [allow_undo]?)
		if ( loc != route.front() ) {
			ERR_NG << "When redoing a recall at " << route.front()
			       << ", the location was moved to " << loc << ".\n";
			// Not really fatal, I suppose. Just update the action so
			// undoing this works.
			route.front() = loc;
		}
	} else {
		gui::dialog(gui, "", msg, gui::OK_ONLY).show();
		return false;
	}

	return true;
}

/**
 * Redoes this action.
 * @return true on success; false on an error.
 */
bool undo_list::recruit_action::redo(int side)
{
	game_display & gui = *resources::screen;
	team &current_team = (*resources::teams)[side-1];

	map_location loc = route.front();
	map_location from = recruit_from;
	const std::string & name = u_type.base_id();

	//search for the unit to be recruited in recruits
	if ( !util::contains(get_recruits(side, loc), name) ) {
		ERR_NG << "Trying to redo a recruit for side " << side
			<< ", which does not recruit type \"" << name << "\"\n";
		assert(false);
		return false;
	}

	current_team.last_recruit(name);
	const std::string &msg = find_recruit_location(side, loc, from, name);
	if ( msg.empty() ) {
		//MP_COUNTDOWN: restore recruitment bonus
		current_team.set_action_bonus_count(1 + current_team.action_bonus_count());
		recorder.redo(replay_data);
		replay_data.clear();
		set_scontext_synced sco;
		recruit_unit(u_type, side, loc, from, true, false);

		// Quick error check. (Abuse of [allow_undo]?)
		if ( loc != route.front() ) {
			ERR_NG << "When redoing a recruit at " << route.front()
			       << ", the location was moved to " << loc << ".\n";
			// Not really fatal, I suppose. Just update the action so
			// undoing this works.
			route.front() = loc;
		}
	} else {
		gui::dialog(gui, "", msg, gui::OK_ONLY).show();
		return false;
	}

	return true;
}

/**
 * Redoes this action.
 * @return true on success; false on an error.
 */
bool undo_list::move_action::redo(int side)
{
	game_display & gui = *resources::screen;
	unit_map &   units = *resources::units;
	team &current_team = (*resources::teams)[side-1];

	// Check units.
	unit_map::iterator u = units.find(route.front());
	if ( u == units.end() ) {
		ERR_NG << "Illegal movement 'redo'.\n";
		assert(false);
		return false;
	}

	// Adjust starting moves.
	const int saved_moves = starting_moves;
	starting_moves = u->movement_left();

	// Move the unit.
	unit_display::move_unit(route, *u);
	units.move(u->get_location(), route.back());
	u = units.find(route.back());
	unit::clear_status_caches();

	// Set the unit's state.
	u->set_goto(goto_hex);
	u->set_movement(saved_moves, true);
	u->set_standing();

	if ( resources::game_map->is_village(route.back()) ) {
		get_village(route.back(), u->side());
		//MP_COUNTDOWN restore capture bonus
		if ( countdown_time_bonus )
		{
			current_team.set_action_bonus_count(1 + current_team.action_bonus_count());
		}
	}

	gui.invalidate_unit_after_move(route.front(), route.back());
	recorder.redo(replay_data);
	replay_data.clear();
	return true;
}

/**
 * Redoes this action.
 * @return true on success; false on an error.
 */
bool undo_list::auto_shroud_action::redo(int /*side*/)
{
	// This should never happen.
	ERR_NG << "Attempt to redo an auto shroud toggle.\n";
	assert(false);
	return false;
}

/**
 * Redoes this action.
 * @return true on success; false on an error.
 */
bool undo_list::update_shroud_action::redo(int /*side*/)
{
	// This should never happen.
	ERR_NG << "Attempt to redo a shroud update.\n";
	assert(false);
	return false;
}


/**
 * Applies the pending fog/shroud changes from the undo stack.
 * Does nothing if the the current side does not use fog or shroud.
 * @returns  an index (into undos_) pointing to the first undoable action
 *           that can be kept (or undos_.size() if none can be kept).
 */
size_t undo_list::apply_shroud_changes() const
{
	game_display &disp = *resources::screen;
	team &tm = (*resources::teams)[side_ - 1];
	// No need to do clearing if fog/shroud has been kept up-to-date.
	if ( tm.auto_shroud_updates()  ||  !tm.fog_or_shroud() )
		return 0;


	shroud_clearer clearer;
	bool cleared_shroud = false;  // for optimization
	size_t erase_to = 0;
	size_t list_size = undos_.size();


	// Loop through the list of undo_actions.
	for( size_t i = 0; i != list_size; ++i ) {
		const undo_action & action = undos_[i];
		// Only actions with vision data are relevant.
		if ( !action.view_info )
			continue;
		LOG_NG << "Turning an undo...\n";

		// Clear the hexes this unit can see from each hex occupied during
		// the action.
		std::vector<map_location>::const_iterator step;
		for (step = action.route.begin(); step != action.route.end(); ++step) {
			// Clear the shroud, collecting new sighted events.
			// (This can be made gradual by changing "true" to "false".)
			if ( clearer.clear_unit(*step, tm, *action.view_info, true) ) {
				cleared_shroud = true;
				erase_to = i + 1;
			}
		}
	}

	// Optimization: if nothing was cleared, then there is nothing to redraw.
	if ( cleared_shroud ) {
		// Update the display before pumping events.
		clearer.invalidate_after_clear();
		disp.draw();
	}

	// Fire sighted events
	if ( clearer.fire_events() ) {
		// Fix up the display in case WML changed stuff.
		clear_shroud(side_);
		disp.invalidate_unit();
		disp.draw();
		// The entire stack needs to be cleared in order to preserve replays.
		// (The events that fired might depend on current unit positions.)
		erase_to = list_size;
	}

	return erase_to;
}


}//namespace actions

