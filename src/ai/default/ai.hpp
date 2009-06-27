/* $Id$ */
/*
   Copyright (C) 2003 - 2009 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file ai/default/ai.hpp */

#ifndef AI_DEFAULT_AI_HPP_INCLUDED
#define AI_DEFAULT_AI_HPP_INCLUDED

#include "../../global.hpp"

#include "contexts.hpp"

#include "../actions.hpp"
#include "../interface.hpp"
#include "../contexts.hpp"

#include "../../formula_callable.hpp"

#ifdef _MSC_VER
#pragma warning(push)
//silence "inherits via dominance" warnings
#pragma warning(disable:4250)
#endif

class formula_ai;

namespace ai {

/** A trivial ai that sits around doing absolutely nothing. */
class idle_ai : public readwrite_context_proxy, public interface {
public:
	idle_ai(readwrite_context &context);
	void play_turn();
	void new_turn();
	virtual std::string describe_self();
	void switch_side(side_number side);
	int get_recursion_count() const;
private:
	recursion_counter recursion_counter_;
};

class ai_default : public virtual default_ai_context_proxy, public interface, public game_logic::formula_callable {
public:
	typedef map_location location;//will get rid of this later

	ai_default(default_ai_context &context);
	virtual ~ai_default();

	virtual void play_turn();
	virtual void new_turn();
	virtual std::string describe_self();
	void switch_side(side_number side);

	struct target {
		enum TYPE { VILLAGE, LEADER, EXPLICIT, THREAT, BATTLE_AID, MASS, SUPPORT };

		target(const location& pos, double val, TYPE target_type=VILLAGE) : loc(pos), value(val), type(target_type)
		{}
		location loc;
		double value;

		TYPE type;
	};

	virtual variant get_value(const std::string& key) const;
	virtual void get_inputs(std::vector<game_logic::formula_input>* inputs) const;

	/** get the recursion counter */
	int get_recursion_count() const;
private:
	recursion_counter recursion_counter_;

protected:

	virtual void do_move();

	virtual bool do_combat(std::map<map_location,paths>& possible_moves,
			const move_map& srcdst, const move_map& dstsrc,
			const move_map& enemy_srcdst, const move_map& enemy_dstsrc);
	virtual bool get_villages(std::map<map_location,paths>& possible_moves,
			const move_map& dstsrc, const move_map& enemy_dstsrc,
			unit_map::iterator &leader);
	virtual bool get_healing(std::map<map_location,paths>& possible_moves,
			const move_map& srcdst, const move_map& enemy_dstsrc);
	virtual bool retreat_units(std::map<map_location,paths>& possible_moves,
			const move_map& srcdst, const move_map& dstsrc,
			const move_map& enemy_dstsrc, unit_map::const_iterator leader);
	virtual bool move_to_targets(std::map<map_location,paths>& possible_moves,
			move_map& srcdst, move_map& dstsrc, const move_map& enemy_dstsrc,
			unit_map::const_iterator leader);

	virtual bool should_retreat(const map_location& loc,
			const unit_map::const_iterator un, const move_map& srcdst,
			const move_map& dstsrc, const move_map& enemy_dstsrc, double caution);

	virtual bool do_recruitment();

	virtual void move_leader_to_keep(const move_map& enemy_dstsrc);
	virtual void move_leader_after_recruit(const move_map& srcdst,
			const move_map& dstsrc, const move_map& enemy_dstsrc);

	virtual void move_leader_to_goals(const move_map& enemy_dstsrc);


	virtual bool recruit_usage(const std::string& usage);

	virtual bool desperate_attack(const map_location &loc);

	void remove_unit_from_moves(const map_location& u, move_map& srcdst, move_map& dstsrc);

	/** Find enemy units that threaten our valuable assets. */
	void find_threats();

	bool threats_found_;

	/**
	 * Our own version of 'move_unit'. It is like the version in readwrite_context
	 * however if it is the leader moving, it will first attempt recruitment.
	 */
	location move_unit(location from, location to, std::map<location,paths>& possible_moves);

	/**
	 * Our own version of 'attack_enemy'. We record all attacks to support
	 * group attacking.
	 */
	void attack_enemy(const location& attacking_unit, const location& target,
			int att_weapon, int def_weapon);


	/**
	 * Sees if it's possible for a unit to move 'from' -> 'via' -> 'to' all in
	 * one turn.
	 */
	bool multistep_move_possible(const location& from,
		const location& to, const location& via,
		const std::map<location,paths>& possible_moves) const;

public:

protected:

	bool is_accessible(const location& loc, const move_map& dstsrc) const;

	virtual std::vector<target> find_targets(unit_map::const_iterator leader,
			const move_map& enemy_dstsrc);

	/**
	 * Function to form a group of units suitable for moving along the route, 'route'.
	 *
	 * @Returns                   The location which the group may reach this
	 *                            turn. Stores the locations of the units in
	 *                            the group in 'units'.
	 */
	virtual location form_group(const std::vector<location>& route,
			const move_map& dstsrc, std::set<location>& units);

	/** Return the group of enemies that threaten a certain path. */
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

	game_display& disp_;
	gamemap& map_;
	unit_map& units_;
	std::vector<team>& teams_;
	tod_manager& tod_manager_;
	bool consider_combat_;
	std::vector<target> additional_targets_;

	void add_target(const target& tgt) { additional_targets_.push_back(tgt); }

	/**
	 * Analyze all the units that this side can recruit
	 * and rate their movement types.
	 * Ratings will be placed in 'unit_movement_scores_',
	 * with lower scores being better,
	 * and the lowest possible rating being '10'.
	 */
	virtual void analyze_potential_recruit_movements();

	std::map<std::string,int> unit_movement_scores_;
	std::set<std::string> not_recommended_units_;

	/**
	 * Analyze all the units that this side can recruit
	 * and rate their fighting suitability against enemy units.
	 * Ratings will be placed in 'unit_combat_scores_',
	 * with a '0' rating indicating that the unit is 'average' against enemy units,
	 * negative ratings meaning they are poorly suited,
	 * and positive ratings meaning they are well suited.
	 */
	virtual void analyze_potential_recruit_combat();

	std::map<std::string,int> unit_combat_scores_;

	/**
	 * Rates two unit types for their suitability against each other.
	 * Returns 0 if the units are equally matched,
	 * a positive number if a is suited against b,
	 * and a negative number if b is suited against a.
	 */
	virtual int compare_unit_types(const unit_type& a, const unit_type& b) const;

	/**
	 * calculates the average resistance unit type a has against the attacks of
	 * unit type b.
	 */
	virtual int average_resistance_against(const unit_type& a, const unit_type& b) const;

	/** Functions to deal with keeps. */
	void evaluate_recruiting_value(const map_location &leader_loc);

	/**
	 * Function which, given a unit position, and a position the unit wants to
	 * get to in two turns, will return all possible positions the unit can
	 * move to, that will make the destination position accessible next turn.
	 */
	void access_points(const move_map& srcdst, const location& u,
			const location& dst, std::vector<location>& out);


	friend struct attack_analysis;

private:
	void find_villages(/*std::vector<unit_map::const_iterator>& our_units,
		std::vector<std::vector<map_location> >& reachable_villages, */
		std::map<map_location /*unit location*/, std::vector<map_location /* villages we can reach*/> >& reachmap,
		std::vector<std::pair<map_location,map_location> >& moves,
		const std::multimap<map_location,map_location>& dstsrc,
		const std::map<map_location,paths>& possible_moves,
		const std::multimap<map_location,map_location>& enemy_dstsrc) const;

	int recruiting_preferred_;
	static const int min_recruiting_value_to_force_recruit = 28;
protected:
	formula_ai *formula_ai_;
	ai_ptr formula_ai_ptr_;
};

} //end of namespace ai

#ifdef _MSC_VER
#pragma warning(pop)
#endif


#endif
