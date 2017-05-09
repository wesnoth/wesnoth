/*
   Copyright (C) 2009 - 2017 by Eugen Jiresch
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

#include "map/location.hpp"
#include "config.hpp"
#include "time_of_day.hpp"

class gamemap;
class unit_map;
class game_data;

namespace randomness
{
	class rng;
}

//time of day and turn functionality
class tod_manager
{
	public:
	explicit tod_manager(const config& scenario_cfg = config());
		~tod_manager() {}
		tod_manager& operator=(const tod_manager& manager);

		config to_config() const;
		/**
			handles random_start_time, should be called before the game starts.
		*/
		void resolve_random(randomness::rng& r);
		int get_current_time(const map_location& loc = map_location::null_location()) const;

		void set_current_time(int time);
		void set_current_time(int time, int area_index);
		void set_current_time(int time, const std::string& area_id);
		void set_area_id(int area_index, const std::string& id);

		/**
		 * Returns global time of day for the passed turn.
		 * for_turn = 0 means current turn
		 */
		const time_of_day& get_time_of_day(int for_turn = 0) const {
			return get_time_of_day_turn(times_, for_turn ? for_turn : turn_, currentTime_);
		}

		/**
		 * Returns time of day for the passed turn at a location.
		 * tod areas matter, for_turn = 0 means current turn
		 * ignoring illumination
		 */
		const time_of_day& get_time_of_day(const map_location& loc,
				int for_turn = 0) const;

		int get_current_area_time(int index) const;

		/**
		 * Returns time of day object for the passed turn at a location.
		 * tod areas matter, for_turn = 0 means current turn
		 * taking account of illumination caused by units
		 */
		const time_of_day get_illuminated_time_of_day(const unit_map & units, const gamemap & map, const map_location& loc,
				int for_turn = 0) const;

		const time_of_day& get_previous_time_of_day() const;

		static bool is_start_ToD(const std::string&);

		/**
		 * Replace the time of day schedule
		 */
		void replace_schedule(const config& time_cfg);
		void replace_schedule(const std::vector<time_of_day>& schedule);
		void replace_local_schedule(const std::vector<time_of_day>& schedule, int area_index);

		void replace_area_locations(int index, const std::set<map_location>& locs);

		/**
		 * @returns the [time_area]s' ids.
		 */
		std::vector<std::string> get_area_ids() const;

		/**
		 * @returns the nth area.
		 */
		const std::set<map_location>& get_area_by_index(int index) const;

		/**
		 * @param id The id of the area to return.
		 * @returns The area with id @p id.
		 */
		const std::set<map_location>& get_area_by_id(const std::string& id) const;

		/**
		 * Adds a new local time area from config, making it follow its own
		 * time-of-day sequence.
		 *
		 * @param cfg                 Config object containing x,y range/list of
		 *                            locations and desired [time] information.
		 */
		void add_time_area(const gamemap & map, const config& cfg);

		/**
		 * Adds a new local time area from a set of locations, making those
		 * follow a different time-of-day sequence.
		 *
		 * @param id                  Identifier string to associate this time area
		 *                            with.
		 * @param locs                Set of locations to be affected.
		 * @param time_cfg            Config object containing [time] information.
		 */
		void add_time_area(const std::string& id, const std::set<map_location>& locs,
				const config& time_cfg);

		/**
		 * Removes a time area from config, making it follow the scenario's
		 * normal time-of-day sequence.
		 *
		 * @param id                  Identifier of time_area to remove. Supply an
		 *                            empty one to remove all local time areas.
		 */
		void remove_time_area(const std::string& id);

		void remove_time_area(int index);


		bool has_time_area() const {return !areas_.empty();}

		const std::vector<time_of_day>& times(const map_location& loc = map_location::null_location()) const;

		const std::vector<time_of_day>& times(int index) const {
			assert(index < static_cast<int>(areas_.size()));
			return areas_[index].times;
		}

		//turn functions
		int turn() const { return turn_; }
		int number_of_turns() const {return num_turns_;}
		void modify_turns(const std::string& mod);
		void set_number_of_turns(int num);

		void update_server_information() const;
		void modify_turns_by_wml(const std::string& mod);
		void set_number_of_turns_by_wml(int num);

		/** Dynamically change the current turn number. */
		void set_turn(const int num, game_data* vars = nullptr, const bool increase_limit_if_needed = true);
		/** Dynamically change the current turn number. */
		void set_turn_by_wml(const int num, game_data* vars = nullptr, const bool increase_limit_if_needed = true);

		/**
		 * Function to move to the next turn.
		 *
		 * @returns                   True if time has not expired.
		 */
		bool next_turn(game_data* vars);

		/**
		 * Function to check the end of turns.
		 *
		 * @returns                   True if time has not expired.
		 */
		bool is_time_left() const;
		bool has_turn_event_fired() const
		{ return has_turn_event_fired_; }
		void turn_event_fired()
		{ has_turn_event_fired_ = true; }
		bool has_tod_bonus_changed() const
		{ return has_tod_bonus_changed_; }
	private:

		/**
		 * Returns time of day object in the turn "nturn".
		 *
		 * Correct time is calculated from current time.
		 */
		const time_of_day& get_time_of_day_turn(const std::vector<time_of_day>& times, int nturn, const int current_time) const;

		/**
		 * Computes for the main time or a time area the index of its times where we're currently at.
		 * number_of_times: size of that main time or time area's times vector
		 * for_turn_number: for which current turn
		 * current_time: the main or time area's current time
		 */
		int calculate_current_time(
			const int number_of_times,
			const int for_turn_number,
			const int current_time,
			const bool only_to_allowed_range = false) const;

		/**
		 * For a change of the current turn number, sets the current times of the main time
		 * and all time areas.
		 */
		void set_new_current_times(const int new_current_turn_number);


		struct area_time_of_day {
			area_time_of_day() :
				xsrc(),
				ysrc(),
				id(),
				times(),
				hexes(),
				currentTime(0)
			{}

			std::string xsrc, ysrc;
			std::string id;
			std::vector<time_of_day> times;
			std::set<map_location> hexes;
			int currentTime;
		};

		void set_current_time(int time, area_time_of_day& area);

		//index of the times vector of the main time where we're currently at
		int currentTime_;
		std::vector<time_of_day> times_;
		std::vector<area_time_of_day> areas_;

		// current turn
		int turn_;
		//turn limit
		int num_turns_;
		//Whether the "turn X" and the "new turn" events were already fired this turn.
		bool has_turn_event_fired_;
		bool has_tod_bonus_changed_;
		//
		config::attribute_value random_tod_;
};
