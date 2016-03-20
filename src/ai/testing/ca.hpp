
/*
   Copyright (C) 2009 - 2016 by Yurii Chernyi <terraninfo@terraninfo.net>
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
 * Default AI (Testing)
 * @file
 */

#ifndef AI_TESTING_CA_HPP_INCLUDED
#define AI_TESTING_CA_HPP_INCLUDED

#include "units/map.hpp"

#include "ai/composite/rca.hpp"


#ifdef _MSC_VER
#pragma warning(push)
//silence "inherits via dominance" warnings
#pragma warning(disable:4250)
#endif

namespace ai {

namespace testing_ai_default {

//============================================================================

class goto_phase : public candidate_action {
public:

	goto_phase( rca_context &context, const config &cfg );

	virtual ~goto_phase();

	virtual double evaluate();

	virtual void execute();
private:
	move_result_ptr move_;
};


//============================================================================
class aspect_recruitment_phase : public candidate_action {
public:

	aspect_recruitment_phase( rca_context &context, const config &cfg );

	virtual ~aspect_recruitment_phase();

	virtual double evaluate();

	virtual void execute();
};

//============================================================================

class recruitment_phase : public candidate_action {
public:

	recruitment_phase( rca_context &context, const config &cfg );

	virtual ~recruitment_phase();

	virtual double evaluate();

	virtual void execute();

private:

	bool recruit_usage(const std::string& usage);

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
	void analyze_potential_recruit_combat();

	std::map<std::string,int> unit_combat_scores_;

	/**
	 * Rates two unit types for their suitability against each other.
	 * Returns 0 if the units are equally matched,
	 * a positive number if a is suited against b,
	 * and a negative number if b is suited against a.
	 */
	int compare_unit_types(const unit_type& a, const unit_type& b) const;

	/**
	 * calculates the average resistance unit type a has against the attacks of
	 * unit type b.
	 */
	int average_resistance_against(const unit_type& a, const unit_type& b) const;
};

//============================================================================

class combat_phase : public candidate_action {
public:

	combat_phase( rca_context &context, const config &cfg );

	virtual ~combat_phase();

	virtual double evaluate();

	virtual void execute();
private:
	attack_analysis best_analysis_;
	double choice_rating_;

};

//============================================================================

class move_leader_to_goals_phase : public candidate_action {
public:

	move_leader_to_goals_phase( rca_context &context, const config &cfg );

	virtual ~move_leader_to_goals_phase();

	virtual double evaluate();

	virtual void execute();
private:

	void remove_goal(const std::string &id);

	bool auto_remove_;
	map_location dst_;
	std::string id_;
	move_result_ptr move_;
};

//============================================================================

class move_leader_to_keep_phase : public candidate_action {
public:

	move_leader_to_keep_phase( rca_context &context, const config &cfg );

	virtual ~move_leader_to_keep_phase();

	virtual double evaluate();

	virtual void execute();

private:
	move_result_ptr move_;
};

//============================================================================

class get_villages_phase : public candidate_action {
public:

	get_villages_phase( rca_context &context, const config& cfg );

	virtual ~get_villages_phase();

	virtual double evaluate();

	virtual void execute();
private:
	/** Location of the keep the closest to our leader. */
	map_location keep_loc_;

	/** Locaton of our leader. */
	map_location leader_loc_;

	/** The best possible location for our leader if it can't reach a village. */
	map_location best_leader_loc_;

	/** debug log level for AI enabled? */
	bool debug_;

	typedef std::map<map_location /* unit location */,
		std::vector<map_location /* villages we can reach */> > treachmap;

	typedef std::vector<std::pair<map_location /* destination */,
		map_location /* start */ > > tmoves;


	// The list of moves we want to make
	tmoves moves_;


	/** Dispatches all units to their best location. */
	void dispatch(treachmap& reachmap, tmoves& moves);


	/**
	 * Dispatches all units who can reach one village.
	 * Returns true if it modified reachmap isn't empty.
	 */
	bool dispatch_unit_simple(treachmap& reachmap, tmoves& moves);


	/*
	 * Dispatches units to villages which can only be reached by one unit.
	 * Returns true if modified reachmap and reachmap isn't empty.
	 */
	bool dispatch_village_simple(
		treachmap& reachmap, tmoves& moves, size_t& village_count);


	/** Removes a village for all units, returns true if anything is deleted. */
	bool remove_village(
		treachmap& reachmap, tmoves& moves, const map_location& village);


	/** Removes a unit which can't reach any village anymore. */
	treachmap::iterator remove_unit(
		treachmap& reachmap, tmoves& moves, treachmap::iterator unit);


	/** Dispatches the units to a village after the simple dispatching failed. */
	void dispatch_complex(
		treachmap& reachmap, tmoves& moves, const size_t village_count);


	/** Dispatches all units to a village, every unit can reach every village. */
	void full_dispatch(treachmap& reachmap, tmoves& moves);


	/** Shows which villages every unit can reach (debug function). */
	void dump_reachmap(treachmap& reachmap);


	void get_villages(
		const move_map &dstsrc, const move_map &enemy_dstsrc,
		unit_map::const_iterator &leader);


	void find_villages(
		treachmap& reachmap,
		tmoves& moves,
		const std::multimap<map_location,map_location>& dstsrc,
		const std::multimap<map_location,map_location>& enemy_dstsrc);

};

//============================================================================

class get_healing_phase : public candidate_action {
public:

	get_healing_phase( rca_context &context, const config& cfg );

	virtual ~get_healing_phase();

	virtual double evaluate();

	virtual void execute();
private:

	move_result_ptr move_;
};

//============================================================================

class retreat_phase : public candidate_action {
public:

	retreat_phase( rca_context &context, const config &cfg );

	virtual ~retreat_phase();

	virtual double evaluate();

	virtual void execute();
private:

	bool should_retreat(const map_location& loc, const unit_map::const_iterator& un, const move_map &srcdst, const move_map &dstsrc, double caution);

	move_result_ptr move_;

};

//============================================================================

class simple_move_and_targeting_phase : public candidate_action {
public:

	simple_move_and_targeting_phase( rca_context &context, const config &cfg );

	virtual ~simple_move_and_targeting_phase();

	virtual double evaluate();

	virtual void execute();

private:

	move_result_ptr move_;
};


//============================================================================

class leader_control_phase : public candidate_action {
public:

	leader_control_phase( rca_context &context, const config &cfg );

	virtual ~leader_control_phase();

	virtual double evaluate();

	virtual void execute();
};


//============================================================================
class leader_shares_keep_phase : public candidate_action {
public:

	leader_shares_keep_phase( rca_context &context, const config &cfg );

	virtual ~leader_shares_keep_phase();

	virtual double evaluate();

	virtual void execute();
};


//============================================================================

} // end of namespace testing_ai_default

} // end of namespace ai

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
