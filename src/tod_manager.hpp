/*
   Copyright (C) 2009 - 2010 by Eugen Jiresch
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
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

#include <set>
#include <string>
#include <vector>

class game_state;
class gamemap;
class unit_map;

//time of day and turn functionality
class tod_manager : public savegame::savegame_config
{
	public:
		tod_manager(const config& time_cfg, int num_turns, game_state* state=NULL);
		~tod_manager() {}
		tod_manager& operator=(const tod_manager& manager);

		config to_config() const;

		/** Returns time of day object for current turn. */
		const time_of_day& get_time_of_day() const;
		const time_of_day& get_previous_time_of_day() const;
		/**
		 * Returns time of day object in the turn at a location.
		 * If nturn = 0 use current turn
		 */
		const time_of_day& get_time_of_day(const map_location& loc, int n_turn = 0) const;
		/**
		 * Sets global time of day in this turn.
		 */
		void set_time_of_day(int newTime);

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

		const std::vector<time_of_day> times() const {return times_;}

		//consider tod modifying units (e.g. illuminates)
		time_of_day time_of_day_at(const map_location& loc) const;

		//turn functions
		size_t turn() const {return turn_;}
		int number_of_turns() const {return num_turns_;}
		void modify_turns(const std::string& mod);
		void set_number_of_turns(int num);

		/** Dynamically change the current turn number. */
		void set_turn(unsigned int num);

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
		void set_start_ToD(config&, int current_turn);

		/**
		 * Returns time of day object in the turn.
		 *
		 * Correct time is calculated from current time.
		 */
		const time_of_day& get_time_of_day_turn(int nturn) const;

		struct area_time_of_day {
			area_time_of_day() :
				xsrc(),
				ysrc(),
				id(),
				times(),
				hexes()
			{}

			std::string xsrc, ysrc;
			std::string id;
			std::vector<time_of_day> times;
			std::set<map_location> hexes;
		};

		int currentTime_;
		std::vector<time_of_day> times_;
		std::vector<area_time_of_day> areas_;

		size_t turn_;
		int num_turns_;
};
#endif
