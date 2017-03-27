/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
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
 * Movement.
 */

#include "actions/move.hpp"

#include "actions/undo.hpp"
#include "actions/vision.hpp"

#include "config_assign.hpp"
#include "game_events/manager.hpp"
#include "game_events/pump.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "hotkey/hotkey_item.hpp"
#include "hotkey/hotkey_command.hpp"
#include "log.hpp"
#include "map/map.hpp"
#include "mouse_handler_base.hpp"
#include "pathfind/pathfind.hpp"
#include "pathfind/teleport.hpp"
#include "replay.hpp"
#include "replay_helper.hpp"
#include "synced_context.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "units/udisplay.hpp"
#include "font/standard_colors.hpp"
#include "formula/string_utils.hpp"
#include "team.hpp"
#include "units/unit.hpp"
#include "units/animation_component.hpp"
#include "whiteboard/manager.hpp"

#include <deque>
#include <map>
#include <set>

static lg::log_domain log_engine("engine");
#define DBG_NG LOG_STREAM(debug, log_engine)


namespace actions {


void move_unit_spectator::add_seen_friend(const unit_map::const_iterator &u)
{
	seen_friends_.push_back(u);
}


void move_unit_spectator::add_seen_enemy(const unit_map::const_iterator &u)
{
	seen_enemies_.push_back(u);
}


const unit_map::const_iterator& move_unit_spectator::get_ambusher() const
{
	return ambusher_;
}


const unit_map::const_iterator& move_unit_spectator::get_failed_teleport() const
{
	return failed_teleport_;
}


const std::vector<unit_map::const_iterator>& move_unit_spectator::get_seen_enemies() const
{
	return seen_enemies_;
}


const std::vector<unit_map::const_iterator>& move_unit_spectator::get_seen_friends() const
{
	return seen_friends_;
}


const unit_map::const_iterator& move_unit_spectator::get_unit() const
{
	return unit_;
}


move_unit_spectator::move_unit_spectator(const unit_map &units)
	: ambusher_(units.end()),failed_teleport_(units.end()),seen_enemies_(),seen_friends_(),unit_(units.end())
{
}


move_unit_spectator::~move_unit_spectator()
{
}

void move_unit_spectator::reset(const unit_map &units)
{
	ambusher_ = units.end();
	failed_teleport_ = units.end();
	seen_enemies_.clear();
	seen_friends_.clear();
	unit_ = units.end();
}


void move_unit_spectator::set_ambusher(const unit_map::const_iterator &u)
{
	ambusher_ = u;
}


void move_unit_spectator::set_failed_teleport(const unit_map::const_iterator &u)
{
	failed_teleport_ = u;
}


void move_unit_spectator::set_unit(const unit_map::const_iterator &u)
{
	unit_ = u;
}


bool get_village(const map_location& loc, int side, bool *action_timebonus, bool fire_event)
{
	std::vector<team> &teams = resources::gameboard->teams();
	team *t = unsigned(side - 1) < teams.size() ? &teams[side - 1] : nullptr;
	if (t && t->owns_village(loc)) {
		return false;
	}

	bool not_defeated = t && !resources::gameboard->team_is_defeated(*t);

	bool grants_timebonus = false;

	int old_owner_side = 0;
	// We strip the village off all other sides, unless it is held by an ally
	// and our side is already defeated (and thus we can't occupy it)
	for(std::vector<team>::iterator i = teams.begin(); i != teams.end(); ++i) {
		int i_side = i - teams.begin() + 1;
		if (!t || not_defeated || t->is_enemy(i_side)) {
			if(i->owns_village(loc)) {
				old_owner_side = i_side;
				i->lose_village(loc);
			}
			if (side != i_side && action_timebonus) {
				grants_timebonus = true;
			}
		}
	}

	if (!t) return false;

	if(grants_timebonus) {
		t->set_action_bonus_count(1 + t->action_bonus_count());
		*action_timebonus = true;
	}

	if(not_defeated) {
		if (resources::screen != nullptr) {
			resources::screen->invalidate(loc);
		}
		return t->get_village(loc, old_owner_side, fire_event ? resources::gamedata : nullptr);
	}

	return false;
}


namespace { // Private helpers for move_unit()

	/// Helper class for move_unit().
	class unit_mover {
		typedef std::vector<map_location>::const_iterator route_iterator;

	public:
		unit_mover(const unit_mover&) = delete;
		unit_mover& operator=(const unit_mover&) = delete;

		unit_mover(const std::vector<map_location> & route,
		           move_unit_spectator *move_spectator,
		           bool skip_sightings, bool skip_ally_sightings);
		~unit_mover();

		/// Determines how far along the route the unit can expect to move this turn.
		bool check_expected_movement();
		/// Attempts to move the unit along the expected path.
		void try_actual_movement(bool show);
		/// Does some bookkeeping and event firing, for use after movement.
		void post_move(undo_list *undo_stack);
		/// Shows the various on-screen messages, for use after movement.
		void feedback() const;

		/// After checking expected movement, this is the expected path.
		std::vector<map_location> expected_path() const
		{ return std::vector<map_location>(begin_, expected_end_); }
		/// After moving, this is the final hex reached.
		const map_location & final_hex() const
		{ return *move_loc_; }
		/// The number of hexes actually entered.
		size_t steps_travelled() const
		{ return move_loc_ - begin_; }
		/// After moving, use this to detect if movement was less than expected.
		bool stopped_early() const  { return expected_end_ != real_end_; }
		/// After moving, use this to detect if something happened that would
		/// interrupt movement (even if movement ended for a different reason).
		bool interrupted(bool include_end_of_move_events=true) const
		{
			return ambushed_ || blocked() || sighted_ || teleport_failed_ ||
			       (include_end_of_move_events ? event_mutated_ : event_mutated_mid_move_ ) ||
			       !move_it_.valid();
		}

	private: // functions
		/// Returns whether or not movement was blocked by a non-ambushing enemy.
		bool blocked() const { return blocked_loc_ != map_location::null_location(); }
		/// Checks the expected route for hidden units.
		void cache_hidden_units(const route_iterator & start,
		                        const route_iterator & stop);
		/// Fires the enter_hex or exit_hex event and updates our data as needed.
		bool fire_hex_event(const std::string & event_name,
		                    const route_iterator & current,
	                        const route_iterator & other);
		/// AI moves are supposed to not change the "goto" order.
		bool is_ai_move() const	{ return spectator_ != nullptr; }
		/// Checks how far it appears we can move this turn.
		route_iterator plot_turn(const route_iterator & start,
		                         const route_iterator & stop);
		/// Updates our stored info after a WML event might have changed something.
		bool post_wml(const route_iterator & step);
		bool post_wml() { return post_wml(full_end_); }
		/// Fires the sighted events that were raised earlier.
		void pump_sighted(const route_iterator & from);
		/// Returns the ambush alert (if any) for the given unit.
		static std::string read_ambush_string(const unit & ambusher);
		/// Reveals the unit at the indicated location.
		void reveal_ambusher(const map_location & hex, bool update_alert=true);

		/// Returns whether or not undoing this move should be blocked.
		bool undo_blocked() const
		{ return ambushed_ || blocked() || event_mutated_ || fog_changed_ ||
		         teleport_failed_; }

		// The remaining private functions are suggested to be inlined because
		// each is used in only one place. (They are separate functions to ease
		// code reading.)

		/// Checks for ambushers around @a hex, setting flags as appropriate.
		inline void check_for_ambushers(const map_location & hex);
		/// Makes sure the path is not obstructed by a unit.
		inline bool check_for_obstructing_unit(const map_location & hex,
		                                       const map_location & prev_hex);
		/// Moves the unit the next step.
		inline bool do_move(const route_iterator & step_from,
		                    const route_iterator & step_to,
		                    unit_display::unit_mover & animator);
		/// Clears fog/shroud and handles units being sighted.
		inline void handle_fog(const map_location & hex, bool new_animation);
		inline bool is_reasonable_stop(const map_location & hex) const;
		/// Reveals the units stored in ambushers_ (and blocked_loc_).
		inline void reveal_ambushers();
		/// Makes sure the units in ambushers_ still exist.
		inline void validate_ambushers();

	private: // data
		// (The order of the fields is somewhat important for the constructor.)

		// Movement parameters (these decrease the number of parameters needed
		// for individual functions).
		move_unit_spectator * const spectator_;
		const bool skip_sighting_;
		const bool skip_ally_sighting_;
		const bool playing_team_is_viewing_;
		// Needed to interface with unit_display::unit_mover.
		const std::vector<map_location> & route_;

		// The route to traverse.
		const route_iterator begin_;
		const route_iterator full_end_;	// The end of the plotted route.
		route_iterator expected_end_;	// The end of this turn's portion of the plotted route.
		route_iterator ambush_limit_;	// How far we can go before encountering hidden units, ignoring allied units.
		route_iterator obstructed_;	// Points to either full_end_ or a hex we cannot enter. This is used so that exit_hex can fire before we decide we cannot enter this hex.
		route_iterator real_end_;	// How far we actually can move this turn.

		// The unit that is moving.
		unit_map::iterator move_it_;

		// This data stores the state from before the move started.
		const int orig_side_;
		const int orig_moves_;
		const map_location::DIRECTION orig_dir_;
		const map_location goto_;

		// This data tracks the current state as the move is in progress.
		int current_side_;
		team * current_team_;	// Will default to the original team if the moving unit becomes invalid.
		bool current_uses_fog_;
		route_iterator move_loc_; // Will point to the last moved-to location (in case the moving unit disappears).
		size_t do_move_track_;	// Tracks whether or not do_move() needs to update the displayed (fake) unit. Should only be touched by do_move() and the constructor.

		// Data accumulated while making the move.
		map_location zoc_stop_;
		map_location ambush_stop_; // Could be inaccurate if ambushed_ is false.
		map_location blocked_loc_; // Location of a blocking, enemy, non-ambusher unit.
		bool ambushed_;
		bool show_ambush_alert_;
		bool event_mutated_;
		bool event_mutated_mid_move_; // Cache of event_mutated_ from just before the end-of-move handling.
		bool fog_changed_;
		bool sighted_;	// Records if sightings were made that could interrupt movement.
		bool sighted_stop_;	// Records if sightings were made that did interrupt movement (the same as sighted_ unless movement ended for another reason).
		bool teleport_failed_;
		size_t enemy_count_;
		size_t friend_count_;
		std::string ambush_string_;
		std::vector<map_location> ambushers_;
		std::deque<int> moves_left_;	// The front value is what the moving unit's remaining moves should be set to after the next step through the route.

		shroud_clearer clearer_;
	};


	/// This constructor assumes @a route is not empty, and it will assert() that
	/// there is a unit at route.front().
	/// Iterators into @a route must remain valid for the life of this object.
	/// It is assumed that move_spectator is only supplied for AI moves (only
	/// affects whether or not gotos are changed).
	unit_mover::unit_mover(const std::vector<map_location> & route,
	                       move_unit_spectator *move_spectator,
	                       bool skip_sightings, bool skip_ally_sightings) :
		spectator_(move_spectator),
		skip_sighting_(skip_sightings),
		skip_ally_sighting_(skip_ally_sightings),
		playing_team_is_viewing_(resources::screen->playing_team() ==
		                         resources::screen->viewing_team()
		                         ||  resources::screen->show_everything()),
		route_(route),
		begin_(route.begin()),
		full_end_(route.end()),
		expected_end_(begin_),
		ambush_limit_(begin_),
		obstructed_(full_end_),
		real_end_(begin_),
		// Unit information:
		move_it_(resources::gameboard->units().find(*begin_)),
		orig_side_(( assert(move_it_ != resources::gameboard->units().end()),
		             move_it_->side() )),
		orig_moves_(move_it_->movement_left()),
		orig_dir_(move_it_->facing()),
		goto_( is_ai_move() ? move_it_->get_goto() : route.back() ),
		current_side_(orig_side_),
		current_team_(&resources::gameboard->teams()[current_side_-1]),
		current_uses_fog_(current_team_->fog_or_shroud()  &&
		                  current_team_->auto_shroud_updates()),
		move_loc_(begin_),
		do_move_track_(resources::game_events->pump().wml_tracking()),
		// The remaining fields are set to some sort of "zero state".
		zoc_stop_(map_location::null_location()),
		ambush_stop_(map_location::null_location()),
		blocked_loc_(map_location::null_location()),
		ambushed_(false),
		show_ambush_alert_(false),
		event_mutated_(false),
		event_mutated_mid_move_(false),
		fog_changed_(false),
		sighted_(false),
		sighted_stop_(false),
		teleport_failed_(false),
		enemy_count_(0),
		friend_count_(0),
		ambush_string_(),
		ambushers_(),
		moves_left_(),
		clearer_()
	{
		if ( !is_ai_move() )
			// Clear the "goto" instruction during movement.
			// (It will be reset in the destructor if needed.)
			move_it_->set_goto(map_location::null_location());
	}


	unit_mover::~unit_mover()
	{
		// Set the "goto" order? (Not if WML set it.)
		if ( !is_ai_move()  &&  move_it_.valid()  &&
		     move_it_->get_goto() == map_location::null_location() )
		{
			// Only set the goto if movement was not complete and was not
			// interrupted.
			if ( real_end_ != full_end_  &&  !interrupted(false) ) // End-of-move-events do not cancel a goto. (Use case: tutorial S2.)
				move_it_->set_goto(goto_);
		}
	}


	// Private inlines:

	/**
	 * Checks for ambushers around @a hex, setting flags as appropriate.
	 */
	inline void unit_mover::check_for_ambushers(const map_location & hex)
	{
		const unit_map &units = resources::gameboard->units();

		// Need to check each adjacent hex for hidden enemies.
		map_location adjacent[6];
		get_adjacent_tiles(hex, adjacent);
		for ( int i = 0; i != 6; ++i )
		{
			const unit_map::const_iterator neighbor_it = units.find(adjacent[i]);

			if ( neighbor_it != units.end()  &&
			     current_team_->is_enemy(neighbor_it->side())  &&
			     neighbor_it->invisible(adjacent[i], *resources::gameboard) )
			{
				// Ambushed!
				ambushed_ = true;
				ambush_stop_ = hex;
				ambushers_.push_back(adjacent[i]);
			}
		}
	}


	/**
	 * Makes sure the path is not obstructed by a unit.
	 * @param  hex       The hex to check.
	 * @param  prev_hex  The previous hex in the route (used to detect a teleport).
	 * @return true if @a hex is obstructed.
	 */
	inline bool unit_mover::check_for_obstructing_unit(const map_location & hex,
	                                                   const map_location & prev_hex)
	{
		const unit_map::const_iterator blocking_unit = resources::gameboard->units().find(hex);

		// If no unit, then the path is not obstructed.
		if ( blocking_unit == resources::gameboard->units().end() )
			return false;

		// Check for units blocking a teleport exit. This can now only happen
		// if these units are not visible to the current side, as otherwise no
		// valid path is found.
		if ( !tiles_adjacent(hex, prev_hex) ) {
			if ( current_team_->is_enemy(blocking_unit->side()) ) {
				// Enemy units always block the tunnel.
				teleport_failed_ = true;
				return true;
			} else {
				// By contrast, allied units (of a side which does not share vision) only
				// block the tunnel if pass_allied_units=true. Whether the teleport is possible
				// is checked by getting the teleport map with the see_all flag set to true.
				const pathfind::teleport_map teleports = pathfind::get_teleport_locations(*move_it_, *current_team_, true, false, false);
				std::set<map_location> allowed_teleports;
				teleports.get_adjacents(allowed_teleports, prev_hex);

				bool found_valid_teleport = false;
				std::set<map_location>::const_iterator it = allowed_teleports.begin();
				while( it!=allowed_teleports.end() && !found_valid_teleport ) {
					if ( *it == hex ) {
						found_valid_teleport = true;
					}
					++it;
				}

				if (!found_valid_teleport) {
					teleport_failed_ = true;
					return true;
				}
			}
		}

		if ( current_team_->is_enemy(blocking_unit->side()) ) {
			// Trying to go through an enemy.
			blocked_loc_ = hex;
			return true;
		}

		// If we get here, the unit does not interfere with movement.
		return false;
	}


	/**
	 * Moves the unit the next step.
	 * @a step_to is the hex being moved to.
	 * @a step_from is the hex before that in the route.
	 * (The unit is actually at *move_loc_.)
	 * @a animator is the unit_display::unit_mover being used.
	 * @return whether or not we started a new animation.
	 */
	inline bool unit_mover::do_move(const route_iterator & step_from,
	                                const route_iterator & step_to,
	                                unit_display::unit_mover & animator)
	{
		game_display &disp = *resources::screen;

		// Adjust the movement even if we cannot move yet.
		// We will eventually be able to move if nothing unexpected
		// happens, and if something does happen, this movement is the
		// cost to discover it.
		move_it_->set_movement(moves_left_.front(), true);
		moves_left_.pop_front();

		// Invalidate before moving so we invalidate neighbor hexes if needed.
		move_it_->anim_comp().invalidate(disp);

		// Attempt actually moving.
		// (Fails if *step_to is occupied).
		std::pair<unit_map::iterator, bool> move_result =
			resources::gameboard->units().move(*move_loc_, *step_to);
		if ( move_result.second )
		{
			// Update the moving unit.
			move_it_ = move_result.first;
			move_it_->set_facing(step_from->get_relative_dir(*step_to));
			// Disable bars. The expectation here is that the animation
			// unit_mover::finish() will clean after us at a later point. Ugly,
			// but it works.
			move_it_->anim_comp().set_standing(false);
			disp.invalidate_unit_after_move(*move_loc_, *step_to);
			disp.invalidate(*step_to);
			move_loc_ = step_to;

			// Show this move.
			const size_t current_tracking = resources::game_events->pump().wml_tracking();
			animator.proceed_to(move_it_.get_shared_ptr(), step_to - begin_,
			                    current_tracking != do_move_track_, false);
			do_move_track_ = current_tracking;
			disp.redraw_minimap();
		}

		return move_result.second;
	}


	/**
	 * Clears fog/shroud and raises events for units being sighted.
	 * Only call this if the current team uses fog or shroud.
	 * @a hex is both the center of fog clearing and the filtered location of
	 * the moving unit when the sighted events will be fired.
	 */
	inline void unit_mover::handle_fog(const map_location & hex,
	                                   bool new_animation)
	{
		// Clear the fog.
		if ( clearer_.clear_unit(hex, *move_it_, *current_team_, nullptr,
		                         &enemy_count_, &friend_count_, spectator_,
		                         !new_animation) )
		{
			clearer_.invalidate_after_clear();
			fog_changed_ = true;
		}

		// Check for sighted units?
		if ( !skip_sighting_ ) {
			sighted_ = enemy_count_ != 0 ;
		}
		if( !skip_sighting_ && !skip_ally_sighting_ )
		{
			sighted_ |= (friend_count_ != 0);
		}
	}


	/**
	 * @return true if an unscheduled stop at @a hex is not likely to negatively
	 * impact the player's plans.
	 * (E.g. it would not kill movement by making an unintended village capture.)
	 */
	inline bool unit_mover::is_reasonable_stop(const map_location & hex) const
	{
		// We cannot reasonably stop if move_it_ could not be moved to this
		// hex (the hex was occupied by someone else).
		if ( *move_loc_ != hex )
			return false;

		// We can reasonably stop if the hex is not an unowned village.
		return !resources::gameboard->map().is_village(hex) ||
		       current_team_->owns_village(hex);
	}


	/**
	 * Reveals the units stored in ambushers_ (and blocked_loc_).
	 * Also sets ambush_string_.
	 * May fire "sighted" events.
	 * Only call this if appropriate; this function does not itself check
	 * ambushed_ or blocked().
	 */
	inline void unit_mover::reveal_ambushers()
	{
		// Reveal the blocking unit.
		if ( blocked() )
			reveal_ambusher(blocked_loc_, false);

		// Reveal ambushers.
		for(const map_location & reveal : ambushers_) {
			reveal_ambusher(reveal, true);
		}

		// Default "Ambushed!" message?
		if ( ambush_string_.empty() )
			ambush_string_ = _("Ambushed!");

		// Update the display.
		resources::screen->draw();
	}


	/**
	 * Makes sure the units in ambushers_ still exist.
	 */
	inline void unit_mover::validate_ambushers()
	{
		const unit_map &units = resources::gameboard->units();

		// Loop through the previously-detected ambushers.
		size_t i = 0;
		while ( i != ambushers_.size() ) {
			if ( units.count(ambushers_[i]) == 0 )
				// Ambusher is gone.
				ambushers_.erase(ambushers_.begin() + i);
			else {
				// Proceed to the next ambusher.
				++i;
			}
		}
	}


	// Private utilities:

	/**
	 * Checks the expected route for hidden units.
	 * This basically handles all the checks for surprises that can be done
	 * without visibly notifying a player. Thus this can be called at the
	 * start of movement and immediately after events, rather than tie up
	 * CPU time in the middle of animating a move.
	 *
	 * @param[in]  start           The beginning of the path to check.
	 * @param[in]  stop            The end of the path to check.
	 */
	void unit_mover::cache_hidden_units(const route_iterator & start,
	                                    const route_iterator & stop)
	{
		// Clear the old cache.
		obstructed_ = full_end_;
		blocked_loc_ = map_location::null_location();
		teleport_failed_ = false;
		// The ambush cache needs special treatment since we cannot re-detect
		// an ambush if we are already at the ambushed location.
		ambushed_ =  ambushed_  &&  ambush_stop_ == *start;
		if ( ambushed_ ) {
			validate_ambushers();
			ambushed_ = !ambushers_.empty();
		}
		if ( !ambushed_ ) {
			ambush_stop_ = map_location::null_location();
			ambushers_.clear();
		}

		// Update the shroud clearer.
		clearer_.cache_units(current_uses_fog_ ? current_team_ : nullptr);


		// Abort for null routes.
		if ( start == stop ) {
			ambush_limit_ = start;
			return;
		}

		// This loop will end with ambush_limit_ pointing one element beyond
		// where the unit would be forced to stop by a hidden unit.
		for ( ambush_limit_ = start+1; ambush_limit_ != stop; ++ambush_limit_ ) {
			// Check if we need to stop in the previous hex.
			if ( ambushed_ ) {
					break;
			}
			// Check for being unable to enter this hex.
			if ( check_for_obstructing_unit(*ambush_limit_, *(ambush_limit_-1)) ) {
				// No replay check here? Makes some sense, I guess.
				obstructed_ = ambush_limit_++; // The limit needs to be after obstructed_ in order for the latter to do anything.
				break;
			}

			// We can enter this hex.
			// See if we are stopped in this hex.
			check_for_ambushers(*ambush_limit_);
		}
	}


	/**
	 * Fires the enter_hex or exit_hex event and updates our data as needed.
	 *
	 * @param[in]  event_name  The name of the event ("enter_hex" or "exit_hex").
	 * @param[in]  current     The currently occupied hex.
	 * @param[in]  other       The secondary hex to provide to the event.
	 *
	 * @return true if this event should interrupt movement.
	 * (This is also stored in event_mutated_.)
	 */
	bool unit_mover::fire_hex_event(const std::string & event_name,
		                            const route_iterator & current,
	                                const route_iterator & other)
	{
		const size_t track = resources::game_events->pump().wml_tracking();
		bool valid = true;

		const game_events::entity_location mover(*move_it_, *current);
		const bool event = resources::game_events->pump().fire(event_name, mover, *other);

		if ( track != resources::game_events->pump().wml_tracking() )
			// Some WML fired, so update our status.
			valid = post_wml(current);

		if ( event || !valid )
			event_mutated_ = true;

		return event || !valid;
	}


	/**
	 * Checks how far it appears we can move this turn.
	 *
	 * @param[in] start         The beginning of the plotted path.
	 * @param[in] stop          The end of the plotted path.
	 *
	 * @return An end iterator for the path that can be traversed this turn.
	 */
	unit_mover::route_iterator unit_mover::plot_turn(const route_iterator & start,
	                                                 const route_iterator & stop)
	{
		const gamemap &map = resources::gameboard->map();

		// Handle null routes.
		if ( start == stop )
			return start;


		int remaining_moves = move_it_->movement_left();
		zoc_stop_ = map_location::null_location();
		moves_left_.clear();

		if ( start != begin_ ) {
			// Check for being unable to leave the current hex.
			if ( !move_it_->get_ability_bool("skirmisher", *start, *resources::gameboard) &&
			      pathfind::enemy_zoc(*current_team_, *start, *current_team_) )
				zoc_stop_ = *start;
		}

		// This loop will end with end pointing one element beyond where the
		// unit thinks it will stop (the usual notion of "end" for iterators).
		route_iterator end = start + 1;
		for ( ; end != stop; ++end )
		{
			// Break out of the loop if we cannot leave the previous hex.
			if ( zoc_stop_ != map_location::null_location() ) {
				break;
			}
			remaining_moves -= move_it_->movement_cost(map[*end]);
			if ( remaining_moves < 0 ) {
					break;
			}

			// We can enter this hex. Record the cost.
			moves_left_.push_back(remaining_moves);

			// Check for being unable to leave this hex.
			if ( !move_it_->get_ability_bool("skirmisher", *end, *resources::gameboard) &&
			      pathfind::enemy_zoc(*current_team_, *end, *current_team_) )
				zoc_stop_ = *end;
		}

		if ( true ) {
			// Avoiding stopping on a (known) unit.
			route_iterator min_end =  start == begin_ ? start : start + 1;
			while ( end != min_end  &&  resources::gameboard->has_visible_unit(*(end-1), *current_team_) )
				// Backtrack.
				--end;
		}

		return end;
	}


	/**
	 * Updates our stored info after a WML event might have changed something.
	 *
	 * @param step      Indicates the position in the path where we might need to start recalculating movement.
	 *                  Set this to full_end_ (or do not supply it) to skip such recalculations (because movement has finished).
	 *
	 * @returns false if continuing is impossible (i.e. we lost the moving unit).
	 */
	bool unit_mover::post_wml(const route_iterator & step)
	{
		// Re-find the moving unit.
		move_it_ = resources::gameboard->units().find(*move_loc_);
		const bool found = move_it_ != resources::gameboard->units().end();

		// Update the current unit data.
		current_side_ = found ? move_it_->side() : orig_side_;
		current_team_ = &resources::gameboard->teams()[current_side_-1];
		current_uses_fog_ = current_team_->fog_or_shroud()  &&
		                    ( current_side_ != orig_side_  ||
		                      current_team_->auto_shroud_updates() );

		// Update the path.
		if ( found  &&  step != full_end_ ) {
			const route_iterator new_limit = plot_turn(step, expected_end_);
			cache_hidden_units(step, new_limit);
			// Just in case: length 0 paths become length 1 paths.
			if ( ambush_limit_ == step )
				++ambush_limit_;
		}

		return found;
	}


	/**
	 * Fires the sighted events that were raised earlier.
	 *
	 * @param[in]  from  Points to the hex the sighting unit currently occupies.
	 *
	 * @return sets event_mutated_ to true if this event should interrupt movement.
	 */
	void unit_mover::pump_sighted(const route_iterator & from)
	{
		const size_t track = resources::game_events->pump().wml_tracking();
		bool valid = true;

		const bool event = clearer_.fire_events();

		if ( track != resources::game_events->pump().wml_tracking() )
			// Some WML fired, so update our status.
			valid = post_wml(from);

		if ( event || !valid )
			event_mutated_ = true;
	}


	/**
	 * Returns the ambush alert (if any) for the given unit.
	 */
	std::string unit_mover::read_ambush_string(const unit & ambusher)
	{
		for(const unit_ability &hide : ambusher.get_abilities("hides"))
		{
			const std::string & ambush_string = (*hide.first)["alert"].str();
			if ( !ambush_string.empty() )
				return ambush_string;
		}

		// No string found.
		return std::string();
	}


	/**
	 * Reveals the unit at the indicated location.
	 * Can also update the current ambushed alert.
	 * May fire "sighted" events.
	 */
	void unit_mover::reveal_ambusher(const map_location & hex, bool update_alert)
	{
		// Convenient alias:
		unit_map &units = resources::gameboard->units();
		game_display &disp = *resources::screen;

		// Find the unit at the indicated location.
		unit_map::iterator ambusher = units.find(hex);
		if ( ambusher != units.end() ) {
			// Prepare for sighted events.
			std::vector<int> sight_cache(get_sides_not_seeing(*ambusher));
			// Make sure the unit is visible (during sighted events, and in case
			// we had to backtrack).
			ambusher->set_state(unit::STATE_UNCOVERED, true);

			// Record this in the move spectator.
			if ( spectator_ )
				spectator_->set_ambusher(ambusher);

			// Override the default ambushed messge?
			if ( update_alert ) {
				// Observers don't get extra information.
				if ( playing_team_is_viewing_ || !disp.fogged(hex) ) {
					show_ambush_alert_ = true;
					// We only support one custom ambush message; use the first one.
					if ( ambush_string_.empty() )
						ambush_string_ = read_ambush_string(*ambusher);
				}
			}

			// Make sure this hex is drawn correctly.
			disp.invalidate(hex);
			// Fire sighted events.
			event_mutated_ |= actor_sighted(*ambusher, &sight_cache);
			post_wml();
		}
	}


	// Public interface:

	/**
	 * Determines how far along the route the unit can expect to move this turn.
	 * This is based solely on data known to the player, and will not plot a move
	 * that ends on another (known) unit.
	 * (For example, this prevents a player from plotting a multi-turn move that
	 * has this turn's movement ending on a (slower) unit, and thereby clearing
	 * fog as if the moving unit actually made it on top of that other unit.)
	 *
	 * @returns false if the expectation is to not move at all.
	 */
	bool unit_mover::check_expected_movement()
	{
		expected_end_ = plot_turn(begin_, full_end_);
		return expected_end_ != begin_;
	}


	/**
	 * Attempts to move the unit along the expected path.
	 * (This will do nothing unless check_expected_movement() was called first.)
	 *
	 * @param[in]  show            Set to false to suppress animations.
	 */
	void unit_mover::try_actual_movement(bool show)
	{
		static const std::string enter_hex_str("enter hex");
		static const std::string exit_hex_str("exit hex");


		bool obstructed_stop = false;


		// Check for hidden units along the expected path before we start
		// animating and firing events.
		cache_hidden_units(begin_, expected_end_);

		if ( begin_ != ambush_limit_ ) {
			// Cache the moving unit's visibility.
			std::vector<int> not_seeing = get_sides_not_seeing(*move_it_);

			// Prepare to animate.
			unit_display::unit_mover animator(route_, show);
			animator.start(move_it_.get_shared_ptr());

			// Traverse the route to the hex where we need to stop.
			// Each iteration performs the move from real_end_-1 to real_end_.
			for ( real_end_ = begin_+1; real_end_ != ambush_limit_; ++real_end_ ) {
				const route_iterator step_from = real_end_ - 1;

				// See if we can leave *step_from.
				// Already accounted for: ambusher
				if ( event_mutated_ )
				{
					break;
				}
				if ( sighted_ && is_reasonable_stop(*step_from) )
				{
					sighted_stop_ = true;
					break;
				}
				// Already accounted for: ZoC
				// Already accounted for: movement cost
				if ( fire_hex_event(exit_hex_str, step_from, real_end_) ) {
					break;
				}
				if ( real_end_ == obstructed_ ) {
					// We did not check for being a replay when checking for an
					// obstructed hex, so we do not check can_break here.
					obstructed_stop = true;
					break;
				}

				// We can leave *step_from. Make the move to *real_end_.
				bool new_animation = do_move(step_from, real_end_, animator);
				// Update the fog.
				if ( current_uses_fog_ )
					handle_fog(*real_end_, new_animation);
				animator.wait_for_anims();

				// Fire the events for this step.
				// (These return values are not checked since real_end_ still
				// needs to be incremented. The event_mutated_ check will break
				// us out of the loop if needed.)
				fire_hex_event(enter_hex_str, real_end_, step_from);
				// Sighted events only fire if we could stop due to sighting.
				if (is_reasonable_stop(*real_end_)) {
					pump_sighted(real_end_);
				}
			}//for
			// Make sure any remaining sighted events get fired.
			pump_sighted(real_end_-1);

			if ( move_it_.valid() ) {
				// Finish animating.
				animator.finish(move_it_.get_shared_ptr());
				// Check for the moving unit being seen.
				event_mutated_ |= actor_sighted(*move_it_, &not_seeing);
			}
		}//if

		// Some flags were set to indicate why we might stop.
		// Update those to reflect whether or not we got to them.
		ambushed_ = ambushed_ && real_end_ == ambush_limit_;
		if ( !obstructed_stop )
			blocked_loc_ = map_location::null_location();
		teleport_failed_ = teleport_failed_ && obstructed_stop;
		// event_mutated_ does not get unset, regardless of other reasons
		// for stopping, but we do save its current value.
		event_mutated_mid_move_ = event_mutated_;
	}


	/**
	 * Does some bookkeeping and event firing, for use after movement.
	 * This includes village capturing and the undo stack.
	 */
	void unit_mover::post_move(undo_list *undo_stack)
	{
		const map_location & final_loc = final_hex();

		int orig_village_owner = 0;
		bool action_time_bonus = false;

		// Reveal ambushers?
		if ( ambushed_ || blocked() )
			reveal_ambushers();
		else if ( teleport_failed_ && spectator_ )
			spectator_->set_failed_teleport(resources::gameboard->units().find(*obstructed_));
		unit::clear_status_caches();

		if ( move_it_.valid() ) {
			// Update the moving unit.
			move_it_->set_interrupted_move(
				sighted_stop_ && !resources::whiteboard->is_executing_actions() ?
					*(full_end_-1) :
					map_location::null_location());
			if ( ambushed_ || final_loc == zoc_stop_ )
				move_it_->set_movement(0, true);

			// Village capturing.
			if ( resources::gameboard->map().is_village(final_loc) ) {
				// Is this a capture?
				orig_village_owner = resources::gameboard->village_owner(final_loc) + 1;
				if ( orig_village_owner != current_side_) {
					// Captured. Zap movement and take over the village.
					move_it_->set_movement(0, true);
					event_mutated_ |= get_village(final_loc, current_side_, &action_time_bonus);
					post_wml();
				}
			}
		}

		// Finally, the moveto event.
		event_mutated_ |= resources::game_events->pump().fire("moveto", final_loc, *begin_);
		post_wml();

		// Record keeping.
		if ( spectator_ )
			spectator_->set_unit(move_it_);
		if ( undo_stack ) {
			const bool mover_valid = move_it_.valid();

			if ( mover_valid ) {
				// MP_COUNTDOWN: added param
				undo_stack->add_move(
					move_it_.get_shared_ptr(), begin_, real_end_, orig_moves_,
					action_time_bonus, orig_village_owner, orig_dir_);
			}

			if ( !mover_valid  ||  undo_blocked()  ||
				(resources::whiteboard->is_active() && resources::whiteboard->should_clear_undo()) || !synced_context::can_undo())
			{
				undo_stack->clear();
			}
		}

		// Update the screen.
		resources::screen->redraw_minimap();
		resources::screen->draw();
	}


	/**
	 * Shows the various on-screen messages, for use after movement.
	 */
	void unit_mover::feedback() const
	{
		// Alias some resources.
		game_display &disp = *resources::screen;

		bool redraw = false;

		// Multiple messages may be displayed simultaneously
		// this variable is used to keep them from overlapping
		std::string message_prefix = "";

		// Ambush feedback?
		if ( ambushed_  &&  show_ambush_alert_ ) {
			disp.announce(message_prefix + ambush_string_, font::BAD_COLOR);
			message_prefix += " \n";
			redraw = true;
		}

		// Failed teleport feedback?
		if ( playing_team_is_viewing_  &&  teleport_failed_ ) {
			std::string teleport_string = _("Failed teleport! Exit not empty");
			disp.announce(message_prefix + teleport_string, font::BAD_COLOR);
			message_prefix += " \n";
			redraw = true;
		}

		// Sighted units feedback?
		if ( playing_team_is_viewing_  &&  (enemy_count_ != 0 || friend_count_ != 0) ) {
			// Create the message to display (depends on whether friends,
			// enemies, or both were sighted, and on how many of each).
			utils::string_map symbols;
			symbols["enemies"] = std::to_string(enemy_count_);
			symbols["friends"] = std::to_string(friend_count_);
			std::string message;
			color_t msg_color;
			if ( friend_count_ != 0  &&  enemy_count_ != 0 ) {
				// Both friends and enemies sighted -- neutral message.
				symbols["friendphrase"] = vngettext("Part of 'Units sighted! (...)' sentence^1 friendly", "$friends friendly", friend_count_, symbols);
				symbols["enemyphrase"] = vngettext("Part of 'Units sighted! (...)' sentence^1 enemy", "$enemies enemy", enemy_count_, symbols);
				message = vgettext("Units sighted! ($friendphrase, $enemyphrase)", symbols);
				msg_color = font::NORMAL_COLOR;
			} else if ( enemy_count_ != 0 ) {
				// Only enemies sighted -- bad message.
				message = vngettext("Enemy unit sighted!", "$enemies enemy units sighted!", enemy_count_, symbols);
				msg_color = font::BAD_COLOR;
			} else if ( friend_count_ != 0 ) {
				// Only friends sighted -- good message.
				message = vngettext("Friendly unit sighted", "$friends friendly units sighted", friend_count_, symbols);
				msg_color = font::GOOD_COLOR;
			}

			disp.announce(message_prefix + message, msg_color);
			message_prefix += " \n";
			redraw = true;
		}

		// Suggest "continue move"?
		if ( playing_team_is_viewing_ && sighted_stop_ && !resources::whiteboard->is_executing_actions() ) {
			// See if the "Continue Move" action has an associated hotkey
			std::string name = hotkey::get_names(hotkey::hotkey_command::get_command_by_command(hotkey::HOTKEY_CONTINUE_MOVE).command);
			if ( !name.empty() ) {
				utils::string_map symbols;
				symbols["hotkey"] = name;
				std::string message = vgettext("(press $hotkey to keep moving)", symbols);
				disp.announce(message_prefix + message, font::NORMAL_COLOR);
				message_prefix += " \n";
				redraw = true;
			}
		}

		// Update the screen.
		if ( redraw )
			disp.draw();
	}

}//end anonymous namespace


static size_t move_unit_internal(undo_list* undo_stack,
                 bool show_move,
                 bool* interrupted,
				 unit_mover& mover)
{
	const events::command_disabler disable_commands;
	// Default return value.
	if ( interrupted )
		*interrupted = false;

	// Attempt moving.
	mover.try_actual_movement(show_move);

	config co;
	config cn = config_of
		("stopped_early", mover.stopped_early())
		("final_hex_x", mover.final_hex().wml_x())
		("final_hex_y", mover.final_hex().wml_y());
	bool matches_replay = checkup_instance->local_checkup(cn,co);
	if(!matches_replay)
	{
		replay::process_error("calculated movement destination (x="+ cn["final_hex_x"].str() +  " y=" + cn["final_hex_y"].str() +
			") didn't match the original destination(x="+ co["final_hex_x"].str() +  " y=" + co["final_hex_y"].str() + ")\n");

		//TODO: move the unit by force to the desired destination with something like mover.reset_final_hex(co["x"], co["y"]);
	}

	// Bookkeeping, etc.
	// also fires the moveto event
	mover.post_move(undo_stack);
	if ( show_move )
		mover.feedback();

	// Set return value.
	if ( interrupted )
		*interrupted = mover.interrupted();

	return mover.steps_travelled();
}

/**
 * Moves a unit across the board.
 *
 * This function handles actual movement, checking terrain costs as well as
 * things that might interrupt movement (e.g. ambushes). If the full path
 * cannot be reached this turn, the remainder is stored as the unit's "goto"
 * instruction. (The unit itself is whatever unit is at the beginning of the
 * supplied path.)
 *
 * @param[in]  steps                The route to be traveled. The unit to be moved is at the beginning of this route.
 * @param      undo_stack           If supplied, then either this movement will be added to the stack or the stack will be cleared.
 * @param[in]  continued_move       If set to true, this is a continuation of an earlier move (movement is not interrupted should units be spotted).
 * @param[in]  show_move            Controls whether or not the movement is animated for the player.
 * @param[out] interrupted          If supplied, then this is set to true if information was uncovered that warrants interrupting a chain of actions (and set to false otherwise).
 * @param[out] move_spectator       If supplied, this will be given the information uncovered by the move (and the unit's "goto" instruction will be preserved).
 *
 * @returns The number of hexes entered. This can safely be used as an index
 *          into @a steps to get the location where movement ended, provided
 *          @a steps is not empty (the return value is guaranteed to be less
 *          than steps.size() ).
 */
size_t move_unit_and_record(const std::vector<map_location> &steps,
                 undo_list* undo_stack,
                 bool continued_move, bool show_move,
                 bool* interrupted,
                 move_unit_spectator* move_spectator)
{

	// Avoid some silliness.
	if ( steps.size() < 2  ||  (steps.size() == 2 && steps.front() == steps.back()) ) {
		DBG_NG << "Ignoring a unit trying to jump on its hex at " <<
		          ( steps.empty() ? map_location::null_location() : steps.front() ) << ".\n";
		return 0;
	}
	//if we have no fog activated then we always skip sighted
	if(resources::gameboard->units().find(steps.front()) != resources::gameboard->units().end())
	{
		const team &current_team = resources::gameboard->teams()[
			resources::gameboard->units().find(steps.front())->side() - 1];
		continued_move |= !current_team.fog_or_shroud();
	}
	const bool skip_ally_sighted = !preferences::interrupt_when_ally_sighted();

	// Evaluate this move.
	unit_mover mover(steps, move_spectator, continued_move, skip_ally_sighted);
	if ( !mover.check_expected_movement() )
		return 0;
	if(synced_context::get_synced_state() != synced_context::SYNCED)
	{
		/*
			enter the synced mode and do the actual movement.
		*/
		resources::recorder->add_synced_command("move",replay_helper::get_movement(steps, continued_move, skip_ally_sighted));
		set_scontext_synced sync;
		size_t r =  move_unit_internal(undo_stack, show_move, interrupted, mover);
		resources::controller->check_victory();
		resources::controller->maybe_throw_return_to_play_side();
		sync.do_final_checkup();
		return r;
	}
	else
	{
		//we are already in synced mode and don't need to reenter it again.
		return  move_unit_internal(undo_stack, show_move, interrupted, mover);
	}
}

size_t move_unit_from_replay(const std::vector<map_location> &steps,
                 undo_list* undo_stack,
                 bool continued_move,bool skip_ally_sighted, bool show_move)
{
	// Evaluate this move.
	unit_mover mover(steps, nullptr, continued_move,skip_ally_sighted);
	if ( !mover.check_expected_movement() )
	{
		replay::process_error("found corrupt movement in replay.");
		return 0;
	}

	return move_unit_internal(undo_stack, show_move, nullptr, mover);
}


}//namespace actions
