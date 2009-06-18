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

struct defensive_position {
	defensive_position() :
		loc(),
		chance_to_hit(0),
		vulnerability(0.0),
		support(0.0)
		{}

	map_location loc;
	int chance_to_hit;
	double vulnerability, support;
};


class default_ai_context;
class default_ai_context : public virtual readwrite_context{
public:

	virtual void add_recent_attack(map_location loc) = 0;


	/** Return true if there has been another attack this turn 'close' to this one. */
	virtual bool attack_close(const map_location& loc) const = 0;


	/** Constructor */
	default_ai_context();


	/** Destructor */
	virtual ~default_ai_context();


	virtual defensive_position const& best_defensive_position(const map_location& unit,
			const move_map& dstsrc, const move_map& srcdst, const move_map& enemy_dstsrc) = 0;


	virtual std::map<map_location,defensive_position>& defensive_position_cache() = 0;


	virtual default_ai_context& get_default_ai_context() = 0;


	virtual void invalidate_defensive_position_cache() = 0;


	virtual void invalidate_keeps_cache() = 0;


	virtual void invalidate_recent_attacks_list() = 0;


	virtual const std::set<map_location>& keeps() = 0;


	virtual bool leader_can_reach_keep() = 0;


	virtual const map_location& nearest_keep(const map_location& loc) = 0;


	/**
	 * Function which finds how much 'power' a side can attack a certain location with.
	 * This is basically the maximum hp of damage that can be inflicted upon a unit on loc
	 * by full-health units, multiplied by the defense these units will have.
	 * (if 'use_terrain' is false, then it will be multiplied by 0.5)
	 *
	 * Example: 'loc' can be reached by two units, one of whom has a 10-3 attack
	 * and has 48/48 hp, and can defend at 40% on the adjacent grassland.
	 * The other has a 8-2 attack, and has 30/40 hp, and can defend at 60% on the adjacent mountain.
	 * The rating will be 10*3*1.0*0.4 + 8*2*0.75*0.6 = 19.2
	 */
	virtual double power_projection(const map_location& loc, const move_map& dstsrc) const = 0;


	/** get most suitable keep for leader - nearest free that can be reached in 1 turn, if none - return nearest occupied that can be reached in 1 turn, if none - return nearest keep, if none - return null_location */
	virtual const map_location& suitable_keep( const map_location& leader_location, const paths& leader_paths ) = 0;


	/** Weapon choice cache, to speed simulations. */
	virtual std::map<std::pair<map_location,const unit_type *>,
		std::pair<battle_context::unit_stats,battle_context::unit_stats> >& unit_stats_cache() = 0;

};


// proxies
class default_ai_context_proxy : public virtual default_ai_context, public virtual readwrite_context_proxy {
public:

	virtual void add_recent_attack(map_location loc)
	{
		return target_->add_recent_attack(loc);
	}



	bool attack_close(const map_location& loc) const
	{
		return target_->attack_close(loc);
	}


	defensive_position const& best_defensive_position(const map_location& unit,
			const move_map& dstsrc, const move_map& srcdst, const move_map& enemy_dstsrc)
	{
		return target_->best_defensive_position(unit,dstsrc,srcdst,enemy_dstsrc);
	}


	default_ai_context_proxy()
		: target_(NULL)
	{
	}


	virtual	~default_ai_context_proxy();


	virtual std::map<map_location,defensive_position>& defensive_position_cache()
	{
		return target_->defensive_position_cache();
	}


	virtual default_ai_context& get_default_ai_context()
	{
		return target_->get_default_ai_context();
	}


	void init_default_ai_context_proxy(default_ai_context &target);


	void invalidate_defensive_position_cache()
	{
		return target_->invalidate_defensive_position_cache();
	}


	virtual void invalidate_keeps_cache()
	{
		return target_->invalidate_keeps_cache();
	}


	virtual void invalidate_recent_attacks_list()
	{
		return target_->invalidate_recent_attacks_list();
	}


	virtual bool leader_can_reach_keep()
	{
		return target_->leader_can_reach_keep();
	}


	virtual const std::set<map_location>& keeps()
	{
		return target_->keeps();
	}


	virtual const map_location& nearest_keep( const map_location& loc )
	{
		return target_->nearest_keep(loc);
	}


	virtual double power_projection(const map_location& loc, const move_map& dstsrc) const
	{
		return target_->power_projection(loc,dstsrc);
	}


	virtual const map_location& suitable_keep( const map_location& leader_location, const paths& leader_paths )
	{
		return target_->suitable_keep(leader_location,leader_paths);
	}


	/** Weapon choice cache, to speed simulations. */
	virtual std::map<std::pair<map_location,const unit_type *>,
		std::pair<battle_context::unit_stats,battle_context::unit_stats> >& unit_stats_cache()
	{
		return target_->unit_stats_cache();
	}


private:
	default_ai_context *target_;
};


class default_ai_context_impl : public virtual readwrite_context_proxy, public default_ai_context {
public:

	virtual void add_recent_attack(map_location loc);


	bool attack_close(const map_location& loc) const;


	defensive_position const& best_defensive_position(const map_location& unit,
			const move_map& dstsrc, const move_map& srcdst, const move_map& enemy_dstsrc);
	virtual ~default_ai_context_impl();


	virtual default_ai_context& get_default_ai_context();


	int get_recursion_count() const
	{
		return recursion_counter_.get_count();
	}

	virtual std::map<std::pair<map_location,const unit_type *>,
		std::pair<battle_context::unit_stats,battle_context::unit_stats> >& unit_stats_cache();



	default_ai_context_impl(readwrite_context &context)
		: recursion_counter_(context.get_recursion_count()),unit_stats_cache_(),
			defensive_position_cache_(),attacks_(),keeps_()
	{
		init_readwrite_context_proxy(context);
	}


	virtual std::map<map_location,defensive_position>& defensive_position_cache();


	virtual void invalidate_defensive_position_cache();


	virtual void invalidate_keeps_cache();


	virtual void invalidate_recent_attacks_list();


	virtual bool leader_can_reach_keep();


	virtual const std::set<map_location>& keeps();


	virtual const map_location& nearest_keep( const map_location& loc );


	virtual double power_projection(const map_location& loc, const move_map& dstsrc) const;


	virtual const map_location& suitable_keep( const map_location& leader_location, const paths& leader_paths );


private:
	recursion_counter recursion_counter_;

	std::map<std::pair<map_location,const unit_type *>,
		std::pair<battle_context::unit_stats,battle_context::unit_stats> > unit_stats_cache_;

	std::map<map_location,defensive_position> defensive_position_cache_;

	std::set<map_location> attacks_;

	std::set<map_location> keeps_;

};

} //end of namespace ai

#ifdef _MSC_VER
#pragma warning(pop)
#endif


#endif
