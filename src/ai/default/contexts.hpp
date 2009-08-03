/* $Id$ */
/*
   Copyright (C) 2009 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file ai/default/contexts.hpp
 * Default AI contexts
 */

#ifndef AI_DEFAULT_CONTEXTS_HPP_INCLUDED
#define AI_DEFAULT_CONTEXTS_HPP_INCLUDED

#include "../../global.hpp"

#include "../contexts.hpp"
#include <vector>

#ifdef _MSC_VER
#pragma warning(push)
//silence "inherits via dominance" warnings
#pragma warning(disable:4250)
#endif

//============================================================================
namespace ai {

class attack_analysis : public game_logic::formula_callable
{
public:
	attack_analysis() :
		game_logic::formula_callable(),
		target(),
		movements(),
		target_value(0.0),
		avg_losses(0.0),
		chance_to_kill(0.0),
		avg_damage_inflicted(0.0),
		target_starting_damage(0),
		avg_damage_taken(0.0),
		resources_used(0.0),
		terrain_quality(0.0),
		alternative_terrain_quality(0.0),
		vulnerability(0.0),
		support(0.0),
		leader_threat(false),
		uses_leader(false),
		is_surrounded(false)
	{
	}

	void analyze(const gamemap& map, unit_map& units,
				 class readonly_context& ai_obj,
				 const move_map& dstsrc, const move_map& srcdst,
				 const move_map& enemy_dstsrc, double aggression);

	double rating(double aggression, class readonly_context& ai_obj) const;
	variant get_value(const std::string& key) const;
	void get_inputs(std::vector<game_logic::formula_input>* inputs) const;

	bool attack_close(const map_location& loc) const;

	map_location target;
	std::vector<std::pair<map_location,map_location> > movements;

	/** The value of the unit being targeted. */
	double target_value;

	/** The value on average, of units lost in the combat. */
	double avg_losses;

	/** Estimated % chance to kill the unit. */
	double chance_to_kill;

	/** The average hitpoints damage inflicted. */
	double avg_damage_inflicted;

	int target_starting_damage;

	/** The average hitpoints damage taken. */
	double avg_damage_taken;

	/** The sum of the values of units used in the attack. */
	double resources_used;

	/** The weighted average of the % chance to hit each attacking unit. */
	double terrain_quality;

	/**
	 * The weighted average of the % defense of the best possible terrain
	 * that the attacking units could reach this turn, without attacking
	 * (good for comparison to see just how good/bad 'terrain_quality' is).
	 */
	double alternative_terrain_quality;

	/**
	 * The vulnerability is the power projection of enemy units onto the hex
	 * we're standing on. support is the power projection of friendly units.
	 */
	double vulnerability, support;

	/** Is true if the unit is a threat to our leader. */
	bool leader_threat;

	/** Is true if this attack sequence makes use of the leader. */
	bool uses_leader;

	/** Is true if the units involved in this attack sequence are surrounded. */
	bool is_surrounded;

};


class default_ai_context;
class default_ai_context : public virtual readwrite_context{
public:

	/** Return a vector of all possible attack analysisis */
	virtual std::vector<attack_analysis> analyze_targets(
		const move_map& srcdst, const move_map& dstsrc,
		const move_map& enemy_srcdst, const move_map& enemy_dstsrc) = 0;


	virtual int count_free_hexes_in_castle(const map_location& loc, std::set<map_location> &checked_hexes) = 0;


	/** Constructor */
	default_ai_context();


	/** Destructor */
	virtual ~default_ai_context();


	/** Analyze possibility of attacking target on 'loc'. */
	virtual void do_attack_analysis(
		const map_location& loc, const move_map& srcdst, const move_map& dstsrc,
		const move_map& fullmove_srcdst, const move_map& fullmove_dstsrc,
		const move_map& enemy_srcdst, const move_map& enemy_dstsrc,
		const map_location* tiles, bool* used_locations,
		std::vector<map_location>& units,
		std::vector<attack_analysis>& result, attack_analysis& cur_analysis) = 0;


	virtual default_ai_context& get_default_ai_context() = 0;


	virtual bool multistep_move_possible(const map_location& from,
		const map_location& to, const map_location& via,
		const moves_map& possible_moves) const = 0;


	virtual int rate_terrain(const unit& u, const map_location& loc) const = 0;

	/** get most suitable keep for leader - nearest free that can be reached in 1 turn, if none - return nearest occupied that can be reached in 1 turn, if none - return nearest keep, if none - return null_location */
	virtual const map_location& suitable_keep( const map_location& leader_location, const paths& leader_paths ) = 0;

};


// proxies
class default_ai_context_proxy : public virtual default_ai_context, public virtual readwrite_context_proxy {
public:

	virtual std::vector<attack_analysis> analyze_targets(
		const move_map& srcdst, const move_map& dstsrc,
		const move_map& enemy_srcdst, const move_map& enemy_dstsrc)
	{
		return target_->analyze_targets(srcdst,dstsrc,enemy_srcdst,enemy_dstsrc);
	}


	int count_free_hexes_in_castle(const map_location& loc, std::set<map_location> &checked_hexes)
	{
		return target_->count_free_hexes_in_castle(loc, checked_hexes);
	}


	default_ai_context_proxy()
		: target_(NULL)
	{
	}


	virtual	~default_ai_context_proxy();



	virtual void do_attack_analysis(
		const map_location& loc, const move_map& srcdst, const move_map& dstsrc,
		const move_map& fullmove_srcdst, const move_map& fullmove_dstsrc,
		const move_map& enemy_srcdst, const move_map& enemy_dstsrc,
		const map_location* tiles, bool* used_locations,
		std::vector<map_location>& units,
		std::vector<attack_analysis>& result, attack_analysis& cur_analysis)
	{
		target_->do_attack_analysis(loc,srcdst,dstsrc,fullmove_srcdst,fullmove_dstsrc,enemy_srcdst,enemy_dstsrc,tiles,used_locations,units,result,cur_analysis);
	}


	virtual default_ai_context& get_default_ai_context()
	{
		return target_->get_default_ai_context();
	}


	void init_default_ai_context_proxy(default_ai_context &target);


	virtual bool multistep_move_possible(const map_location& from,
		const map_location& to, const map_location& via,
		const moves_map& possible_moves) const
	{
		return target_->multistep_move_possible(from,to,via,possible_moves);
	}


	virtual int rate_terrain(const unit& u, const map_location& loc) const
	{
		return target_->rate_terrain(u,loc);
	}


	virtual const map_location& suitable_keep( const map_location& leader_location, const paths& leader_paths )
	{
		return target_->suitable_keep(leader_location,leader_paths);
	}


private:
	default_ai_context *target_;
};


class default_ai_context_impl : public virtual readwrite_context_proxy, public default_ai_context {
public:

	virtual std::vector<attack_analysis> analyze_targets(
		const move_map& srcdst, const move_map& dstsrc,
		const move_map& enemy_srcdst, const move_map& enemy_dstsrc);


	int count_free_hexes_in_castle(const map_location& loc, std::set<map_location> &checked_hexes);


	default_ai_context_impl(readwrite_context &context)
		: recursion_counter_(context.get_recursion_count())
	{
		init_readwrite_context_proxy(context);
	}


	virtual ~default_ai_context_impl();


	virtual void do_attack_analysis(
		const map_location& loc, const move_map& srcdst, const move_map& dstsrc,
		const move_map& fullmove_srcdst, const move_map& fullmove_dstsrc,
		const move_map& enemy_srcdst, const move_map& enemy_dstsrc,
		const map_location* tiles, bool* used_locations,
		std::vector<map_location>& units,
		std::vector<attack_analysis>& result, attack_analysis& cur_analysis);


	virtual default_ai_context& get_default_ai_context();


	int get_recursion_count() const
	{
		return recursion_counter_.get_count();
	}


	virtual bool multistep_move_possible(const map_location& from,
		const map_location& to, const map_location& via,
		const moves_map& possible_moves) const;


	virtual int rate_terrain(const unit& u, const map_location& loc) const;


	virtual const map_location& suitable_keep( const map_location& leader_location, const paths& leader_paths );


private:
	recursion_counter recursion_counter_;


};

} //end of namespace ai

#ifdef _MSC_VER
#pragma warning(pop)
#endif


#endif
