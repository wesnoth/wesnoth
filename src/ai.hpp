/* $Id$ */
/*
   Copyright (C) 2003 - 2007 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file ai.hpp
//!

#ifndef AI_HPP_INCLUDED
#define AI_HPP_INCLUDED

#include "global.hpp"

#include "actions.hpp"
#include "ai_interface.hpp"
#include "formula_callable.hpp"

class ai : public ai_interface {
public:

	ai(ai_interface::info& info);
	virtual ~ai() {}

	virtual void play_turn();

	struct target {
		enum TYPE { VILLAGE, LEADER, EXPLICIT, THREAT, BATTLE_AID, MASS, SUPPORT };

		target(const location& pos, double val, TYPE target_type=VILLAGE) : loc(pos), value(val), type(target_type)
		{}
		location loc;
		double value;

		TYPE type;
	};

	struct defensive_position {
		defensive_position() :
			loc(),
			chance_to_hit(0),
			vulnerability(0.0),
			support(0.0)
			{}

		location loc;
		int chance_to_hit;
		double vulnerability, support;
	};

	defensive_position const& best_defensive_position(const location& unit,
			const move_map& dstsrc, const move_map& srcdst, const move_map& enemy_dstsrc);

	void invalidate_defensive_position_cache() { defensive_position_cache_.clear(); }

	bool leader_can_reach_keep();

	//! Return true iff there has been another attack this turn 'close' to this one.
	bool attack_close(const location& loc) const;

protected:

	std::map<location,defensive_position> defensive_position_cache_;

	virtual void do_move();

	virtual bool do_combat(std::map<gamemap::location,paths>& possible_moves,
			const move_map& srcdst, const move_map& dstsrc,
			const move_map& enemy_srcdst, const move_map& enemy_dstsrc);
	virtual bool get_villages(std::map<gamemap::location,paths>& possible_moves,
			const move_map& dstsrc, const move_map& enemy_dstsrc,
			unit_map::iterator &leader);
	virtual bool get_healing(std::map<gamemap::location,paths>& possible_moves,
			const move_map& srcdst, const move_map& enemy_dstsrc);
	virtual bool retreat_units(std::map<gamemap::location,paths>& possible_moves,
			const move_map& srcdst, const move_map& dstsrc,
			const move_map& enemy_dstsrc, unit_map::const_iterator leader);
	virtual bool move_to_targets(std::map<gamemap::location,paths>& possible_moves,
			move_map& srcdst, move_map& dstsrc, const move_map& enemy_dstsrc,
			unit_map::const_iterator leader);

	virtual bool should_retreat(const gamemap::location& loc,
			const unit_map::const_iterator un, const move_map& srcdst,
			const move_map& dstsrc, const move_map& enemy_dstsrc, double caution);

	virtual void do_recruitment();

	virtual void move_leader_to_keep(const move_map& enemy_dstsrc);
	virtual void move_leader_after_recruit(const move_map& srcdst,
			const move_map& dstsrc, const move_map& enemy_dstsrc);

	virtual void move_leader_to_goals(const move_map& enemy_dstsrc);


	virtual bool recruit_usage(const std::string& usage);

	virtual bool desperate_attack(const gamemap::location &loc);

	void remove_unit_from_moves(const gamemap::location& u, move_map& srcdst, move_map& dstsrc);

	//! Find enemy units that threaten our valuable assets.
	void find_threats();

	bool threats_found_;

	//! Our own version of 'move_unit'. It is like the version in ai_interface,
	//! however if it is the leader moving, it will first attempt recruitment.
	location move_unit(location from, location to, std::map<location,paths>& possible_moves);

	//! Our own version of 'attack_enemy'. We record all attacks to support group attacking.
	void attack_enemy(const location& attacking_unit, const location& target,
			int att_weapon, int def_weapon);

	std::set<location> attacks_;

	//! Sees if it's possible for a unit to move 'from' -> 'via' -> 'to' all in one turn.
	bool multistep_move_possible(const location& from, 
		const location& to, const location& via,
		const std::map<location,paths>& possible_moves) const;

	struct attack_analysis : public game_logic::formula_callable
	{
		void analyze(const gamemap& map, unit_map& units,
					 const std::vector<team>& teams,
					 const gamestatus& status, const game_data& gamedata,
					 class ai& ai_obj,
					 const move_map& dstsrc, const move_map& srcdst,
					 const move_map& enemy_dstsrc, double aggression);

		double rating(double aggression, class ai& ai_obj) const;
		variant get_value(const std::string& key) const;
		void get_inputs(std::vector<game_logic::formula_input>* inputs) const;

		gamemap::location target;
		std::vector<std::pair<gamemap::location,gamemap::location> > movements;

		//! The value of the unit being targeted.
		double target_value;

		//! The value on average, of units lost in the combat
		double avg_losses;

		//! Estimated % chance to kill the unit
		double chance_to_kill;

		//! The average hitpoints damage inflicted
		double avg_damage_inflicted;

		int target_starting_damage;

		//! The average hitpoints damage taken
		double avg_damage_taken;

		//! The sum of the values of units used in the attack
		double resources_used;

		//! The weighted average of the % chance to hit each attacking unit
		double terrain_quality;

		//! The weighted average of the % defense of the best possible terrain
		//! that the attacking units could reach this turn, without attacking
		//! (good for comparison to see just how good/bad 'terrain_quality' is).
		double alternative_terrain_quality;

		//! The vulnerability is the power projection of enemy units onto the hex
		//! we're standing on. support is the power projection of friendly units.
		double vulnerability, support;

		//! Is true if the unit is a threat to our leader.
		bool leader_threat;

		//! Is true if this attack sequence makes use of the leader.
		bool uses_leader;

		//! Is true if the units involved in this attack sequence are surrounded.
		bool is_surrounded;
	};

	virtual void do_attack_analysis(
	                 const location& loc,
	                 const move_map& srcdst, const move_map& dstsrc,
					 const move_map& fullmove_srcdst, const move_map& fullmove_dstsrc,
	                 const move_map& enemy_srcdst, const move_map& enemy_dstsrc,
					 const location* tiles, bool* used_locations,
	                 std::vector<location>& units,
	                 std::vector<attack_analysis>& result,
					 attack_analysis& cur_analysis
	                );


	//! Function which finds how much 'power' a side can attack a certain location with.
	//! This is basically the maximum hp of damage that can be inflicted upon a unit on loc
	//! by full-health units, multiplied by the defense these units will have.
	//! (if 'use_terrain' is false, then it will be multiplied by 0.5)
	//
	// Example: 'loc' can be reached by two units, one of whom has a 10-3 attack
	// and has 48/48 hp, and can defend at 40% on the adjacent grassland.
	// The other has a 8-2 attack, and has 30/40 hp, and can defend at 60% on the adjacent mountain.
	// The rating will be 10*3*1.0*0.4 + 8*2*0.75*0.6 = 19.2
	virtual double power_projection(const gamemap::location& loc, const move_map& dstsrc,
			bool use_terrain=true) const;

	virtual std::vector<attack_analysis> analyze_targets(
	             const move_map& srcdst, const move_map& dstsrc,
	             const move_map& enemy_srcdst, const move_map& enemy_dstsrc
            );

	bool is_accessible(const location& loc, const move_map& dstsrc) const;

	virtual std::vector<target> find_targets(unit_map::const_iterator leader,
			const move_map& enemy_dstsrc);

	//! Function to form a group of units suitable for moving along the route, 'route'.
	//! Returns the location which the group may reach this turn.
	//! Stores the locations of the units in the group in 'units'
	virtual location form_group(const std::vector<location>& route,
			const move_map& dstsrc, std::set<location>& units);

	//! Return the group of enemies that threaten a certain path.
	virtual void enemies_along_path(const std::vector<location>& route,
			const move_map& dstsrc, std::set<location>& units);

	virtual bool move_group(const location& dst, const std::vector<location>& route,
			const std::set<location>& units);

	virtual double rate_group(const std::set<location>& group,
			const std::vector<location>& battlefield) const;

	virtual double compare_groups(const std::set<location>& our_group,
			const std::set<location>& enemy_groups,
			const std::vector<location>& battlefield) const;

	virtual std::pair<location,location> choose_move(std::vector<target>& targets,
			const move_map& srcdst, const move_map& dstsrc, const move_map& enemy_dstsrc);

	//! Rates the value of moving onto certain terrain for a unit.
	virtual int rate_terrain(const unit& u, const location& loc);

	game_display& disp_;
	const gamemap& map_;
	const game_data& gameinfo_;
	unit_map& units_;
	std::vector<team>& teams_;
	unsigned int team_num_;
	const gamestatus& state_;
	bool consider_combat_;
	std::vector<target> additional_targets_;

	void add_target(const target& tgt) { additional_targets_.push_back(tgt); }

	//! Analyze all the units that this side can recruit
	//! and rate their movement types.
	//! Ratings will be placed in 'unit_movement_scores_',
	//! with lower scores being better,
	//! and the lowest possible rating being '10'.
	virtual void analyze_potential_recruit_movements();

	std::map<std::string,int> unit_movement_scores_;
	std::set<std::string> not_recommended_units_;

	//! Analyze all the units that this side can recruit
	//! and rate their fighting suitability against enemy units.
	//! Ratings will be placed in 'unit_combat_scores_',
	//! with a '0' rating indicating that the unit is 'average' against enemy units,
	//! negative ratings meaning they are poorly suited,
	//! and positive ratings meaning they are well suited.
	virtual void analyze_potential_recruit_combat();

	std::map<std::string,int> unit_combat_scores_;

	//! Rates two unit types for their suitability against each other.
	// Returns 0 if the units are equally matched,
	// a positive number if a is suited against b,
	// and a negative number if b is suited against a.
	virtual int compare_unit_types(const unit_type& a, const unit_type& b) const;

	//! calculates the average resistance unit type a has
	//! against the attacks of unit type b.
	virtual int average_resistance_against(const unit_type& a, const unit_type& b) const;

	//! Functions to deal with keeps.
	const std::set<location>& keeps();
	const location& nearest_keep(const location& loc);

	std::set<location> keeps_;

	//! Function which, given a unit position,
	//! and a position the unit wants to get to in two turns,
	//! will return all possible positions the unit can move to,
	//! that will make the destination position accessible next turn.
	void access_points(const move_map& srcdst, const location& u,
			const location& dst, std::vector<location>& out);

	//! Function which gets the areas of the map
	//! that this AI has been instructed to avoid.
	const std::set<location>& avoided_locations();

	std::set<location> avoid_;

	//! Weapon choice cache, to speed simulations.
	std::map<std::pair<location,const unit_type *>,
		std::pair<battle_context::unit_stats,battle_context::unit_stats> > unit_stats_cache_;

	int attack_depth();
	int attack_depth_;
	friend struct attack_analysis;

private:
	void find_villages(/*std::vector<unit_map::const_iterator>& our_units, 
		std::vector<std::vector<gamemap::location> >& reachable_villages, */
		std::map<gamemap::location /*unit location*/, std::vector<gamemap::location /* villages we can reach*/> >& reachmap,
		std::vector<std::pair<gamemap::location,gamemap::location> >& moves,
		const std::multimap<gamemap::location,gamemap::location>& dstsrc,
		const std::map<gamemap::location,paths>& possible_moves,
		const std::multimap<gamemap::location,gamemap::location>& enemy_dstsrc) const;
};

#endif
