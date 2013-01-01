/*
   Copyright (C) 2009 - 2013 by Eugen Jiresch
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
 */
#ifndef TOD_MANAGER_HPP_INCLUDED
#define TOD_MANAGER_HPP_INCLUDED

#include "map_location.hpp"
#include "config.hpp"
#include "time_of_day.hpp"
#include "savegame_config.hpp"

class game_state;
class gamemap;
class unit_map;

//time of day and turn functionality
class tod_manager : public savegame::savegame_config
{
	public:
	explicit tod_manager(const config& scenario_cfg = config(), const int num_turns = -1);
		~tod_manager() {}
		tod_manager& operator=(const tod_manager& manager);

		config to_config() const;

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

		/**
		 * Returns time of day object for the passed turn at a location.
		 * tod areas matter, for_turn = 0 means current turn
		 * taking account of illumination caused by units
		 */
		const time_of_day get_illuminated_time_of_day(const map_location& loc,
				int for_turn = 0) const;

		const time_of_day& get_previous_time_of_day() const;

		static bool is_start_ToD(const std::string&);

		/**
		 * Replace the time of day schedule
		 */
		void replace_schedule(const config& time_cfg);

		/**
		 * Adds a new local time area from config, making it follow its own
		 * time-of-day sequence.
		 *
		 * @param cfg                 Config object containing x,y range/list of
		 *                            locations and desired [time] information.
		 */
		void add_time_area(const config& cfg);

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

		bool has_time_area() const {return !areas_.empty();};

		const std::vector<time_of_day> times() const {return times_;}

		//turn functions
		int turn() const { return turn_; }
		int number_of_turns() const {return num_turns_;}
		void modify_turns(const std::string& mod);
		void set_number_of_turns(int num);

		/** Dynamically change the current turn number. */
		void set_turn(const int num, const bool increase_limit_if_needed = true);

		/**
		 * Function to move to the next turn.
		 *
		 * @returns                   True if time has not expired.
		 */
		bool next_turn();

		/**
		 * Function to check the end of turns.
		 *
		 * @returns                   True if time has not expired.
		 */
		bool is_time_left();
	private:
		int get_start_ToD(const config& level) const;

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

		//index of the times vector of the main time where we're currently at
		int currentTime_;
		std::vector<time_of_day> times_;
		std::vector<area_time_of_day> areas_;

		// current turn
		int turn_;
		//turn limit
		int num_turns_;
};
#endif
