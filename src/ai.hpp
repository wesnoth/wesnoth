/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef AI_HPP_INCLUDED
#define AI_HPP_INCLUDED

#include "actions.hpp"
#include "ai_move.hpp"
#include "display.hpp"
#include "map.hpp"
#include "unit.hpp"
#include "unit_types.hpp"

#include <map>

class ai {
public:

	typedef gamemap::location location;
	typedef std::multimap<location,location> move_map;

	ai(display& disp, const gamemap& map, const game_data& gameinfo,
	   std::map<gamemap::location,unit>& units,
	   std::vector<team>& teams, int team_num, const gamestatus& state);

	void do_move();

	team& current_team();
	const team& current_team() const;

private:
	void do_attack(const location& u, const location& target, int weapon);

	bool do_combat(std::map<gamemap::location,paths>& possible_moves, const move_map& srcdst, const move_map& dstsrc, const move_map& enemy_srcdst, const move_map& enemy_dstsrc);
	bool get_villages(std::map<gamemap::location,paths>& possible_moves, const move_map& srcdst, const move_map& dstsrc, const move_map& enemy_srcdst, const move_map& enemy_dstsrc, unit_map::const_iterator leader);
	bool get_healing(std::map<gamemap::location,paths>& possible_moves, const move_map& srcdst, const move_map& dstsrc, const move_map& enemy_srcdst, const move_map& enemy_dstsrc);
	bool retreat_units(std::map<gamemap::location,paths>& possible_moves, const move_map& srcdst, const move_map& dstsrc, const move_map& enemy_srcdst, const move_map& enemy_dstsrc, unit_map::const_iterator leader);
	void move_to_targets(std::map<gamemap::location,paths>& possible_moves, move_map& srcdst, move_map& dstsrc, const move_map& enemy_srcdst, const move_map& enemy_dstsrc, unit_map::const_iterator leader);

	bool should_retreat(const gamemap::location& loc, const move_map& srcdst, const move_map& dstsrc, const move_map& enemy_srcdst, const move_map& enemy_dstsrc) const;

	void do_recruitment();

	void move_leader_to_keep(const move_map& enemy_dstsrc);
	void move_leader_after_recruit(const move_map& enemy_dstsrc);
	void leader_attack();

	bool recruit(const std::string& usage);
	void move_unit(const location& from, const location& to, std::map<location,paths>& possible_moves);

	void calculate_possible_moves(std::map<location,paths>& moves, move_map& srcdst, move_map& dstsrc, bool enemy, bool assume_full_movement=false);

	struct attack_analysis
	{
		void analyze(const gamemap& map, std::map<location,unit>& units,
		             const gamestatus& status, const game_data& info, int sims,
					 class ai& ai_obj);

		double rating(double aggression) const;

		gamemap::location target;
		std::vector<std::pair<gamemap::location,gamemap::location> > movements;
		std::vector<int> weapons;

		//the value of the unit being targeted
		double target_value;

		//the value on average, of units lost in the combat
		double avg_losses;

		//estimated % chance to kill the unit
		double chance_to_kill;

		//the average hitpoints damage inflicted
		double avg_damage_inflicted;

		int target_starting_damage;

		//the average hitpoints damage taken
		double avg_damage_taken;

		//the sum of the values of units used in the attack
		double resources_used;

		//the weighted average of the % chance to hit each attacking unit
		double terrain_quality;

		//the ratio of the attacks the unit being attacked will get to
		//the strength of its most powerful attack
		double counter_strength_ratio;

		//the vulnerability is the power projection of enemy units onto the hex
		//we're standing on. support is the power projection of friendly units.
		double vulnerability, support;

		//is true if the unit is a threat to our leader
		bool leader_threat;
	};

	void do_attack_analysis(
	                 const location& loc,
	                 const move_map& srcdst, const move_map& dstsrc,
	                 const move_map& enemy_srcdst, const move_map& enemy_dstsrc,
					 const location* tiles, bool* used_locations,
	                 std::vector<location>& units,
	                 std::vector<attack_analysis>& result,
					 attack_analysis& cur_analysis
	                );


	double power_projection(const gamemap::location& loc, const move_map& srcdst, const move_map& dstsrc, bool use_terrain=true) const;

public:
	int choose_weapon(const location& att, const location& def,
					  battle_stats& cur_stats, gamemap::TERRAIN terrain);

private:
	std::vector<attack_analysis> analyze_targets(
	             const move_map& srcdst, const move_map& dstsrc,
	             const move_map& enemy_srcdst, const move_map& enemy_dstsrc
            );


	struct target {
		target(const location& pos, double val) : loc(pos), value(val)
		{}
		location loc;
		double value;
	};

	std::vector<target> find_targets(unit_map::const_iterator leader, const move_map& enemy_srcdst, const move_map& enemy_dstsrc);

	std::pair<location,location> choose_move(std::vector<target>& targets,const move_map& dstsrc, const move_map& enemy_srcdst, const move_map& enemy_dstsrc);

	display& disp_;
	const gamemap& map_;
	const game_data& gameinfo_;
	unit_map& units_;
	std::vector<team>& teams_;
	int team_num_;
	const gamestatus& state_;
	bool consider_combat_;
	std::vector<target> additional_targets_;
};

#endif
