/* $Id$ */
/*
   Copyright (C) 2009 - 2010 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file ai/composite/rca.hpp
 * candidate action framework
 */

#ifndef AI_COMPOSITE_RCA_HPP_INCLUDED
#define AI_COMPOSITE_RCA_HPP_INCLUDED

#include "../../global.hpp"

#include "../../config.hpp"

#include "component.hpp"
#include "contexts.hpp"
#include "../contexts.hpp"
#include <boost/shared_ptr.hpp>
#include <vector>
#include <string>

#ifdef _MSC_VER
#pragma warning(push)
//silence "inherits via dominance" warnings
#pragma warning(disable:4250)
#endif

//============================================================================
namespace ai {

class candidate_action : public virtual rca_context_proxy, public component {
public:
	//this is a score guaranteed to be <=0, thus candidate action with this score will not be selected for execution
	static const double BAD_SCORE;

	//this is a score guaranteed to be very high, higher than any 'normal' candidate action score
	static const double HIGH_SCORE;

	/**
	 * Constructor
	 * @param ai context of the candidate action
	 * @param name name of the action (for debug purposes)
	 * @param type type of the action (for debug purposes)
	 */
	candidate_action( rca_context &context, const config &cfg );

	/**
	 * Destructor
	 */
	virtual ~candidate_action();


	/**
	 * Evaluate the candidate action, resetting the internal state of the action
	 * @return the score
	 * @retval >0 if the action is good
	 * @retval <=0 if the action is not good
	 */
	virtual double evaluate() = 0;

	/**
	 * Execute the candidate action
	 */
	virtual void execute() = 0;

	/**
	 * Is this candidate action enabled ?
	 */
	bool is_enabled() const;

	/**
	 * Enable the candidate action
	 */
	void enable();

	/**
	 * Disable the candidate action
	 */
	void disable();

	/**
	 * Get the usual score of the candidate action without re-evaluation
	 */
	double get_score() const;


	/**
	 * Get the upper bound of the score of the candidate action without re-evaluation
	 */
	double get_max_score() const;

	/**
	 * Get the name of the candidate action (useful for debug purposes)
	 */
	virtual std::string get_name() const
	{ return name_; }

	/**
	 * Get the type of the candidate action (useful for debug purposes)
	 */
	const std::string& get_type() const;

	virtual std::string get_id() const
	{ return id_; }

	virtual std::string get_engine() const
	{ return engine_; }

	int get_recursion_count() const;


	/**
	 * serialize
	 */
	virtual config to_config() const;

private:

	recursion_counter recursion_counter_;

	bool enabled_;


	std::string engine_;


	double score_;


	double max_score_;


	std::string id_;


	std::string name_;


	std::string type_;

};

typedef boost::shared_ptr<candidate_action> candidate_action_ptr;

class candidate_action_factory;

class candidate_action_factory{
public:
	typedef boost::shared_ptr< candidate_action_factory > factory_ptr;
	typedef std::map<std::string, factory_ptr> factory_map;
	typedef std::pair<const std::string, factory_ptr> factory_map_pair;

	static factory_map& get_list() {
		static factory_map *candidate_action_factories;
		if (candidate_action_factories==NULL) {
			candidate_action_factories = new factory_map;
		}
		return *candidate_action_factories;
	}

	virtual candidate_action_ptr get_new_instance( rca_context &context, const config &cfg ) = 0;

	candidate_action_factory( const std::string &name )
	{
		factory_ptr ptr_to_this(this);
		get_list().insert(make_pair(name,ptr_to_this));
	}

	virtual ~candidate_action_factory() {}
};


template<class CANDIDATE_ACTION>
class register_candidate_action_factory : public candidate_action_factory {
public:
	register_candidate_action_factory( const std::string &name )
		: candidate_action_factory( name )
	{
	}

	virtual candidate_action_ptr get_new_instance( rca_context &ai, const config &cfg ){
		return candidate_action_ptr(new CANDIDATE_ACTION(ai,cfg));
	}
};


//============================================================================

} //end of namespace ai

std::ostream &operator<<(std::ostream &s, ai::candidate_action const &ca);

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
