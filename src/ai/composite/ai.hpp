/*
   Copyright (C) 2009 - 2018 by Yurii Chernyi <terraninfo@terraninfo.net>
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
 * @file
 * Composite AI with turn sequence which is a vector of stages
 */

#pragma once

#include "ai/composite/contexts.hpp"
#include "ai/composite/component.hpp"

//============================================================================
namespace ai {

class ai_composite : public ai_context, public virtual default_ai_context_proxy, public component {
public:


	/**
	 * Constructor
	 */
	ai_composite( default_ai_context &context, const config &cfg );


	/**
	 * Destructor
	 */
	virtual ~ai_composite();


	/**
	 * Play the turn
	 */
	void play_turn();


	/**
	 * Evaluate command (using fai)
	 */
	virtual std::string evaluate(const std::string& str);

	/**
	 * On new turn
	 */
	virtual void new_turn();


	std::string describe_self() const;

	/**
	 * serialize
	 */
	virtual config to_config() const;


	int get_recursion_count() const;


	void switch_side(side_number side);


	virtual bool add_goal(const config &cfg);


	virtual bool add_stage(const config &cfg);


	void create_stage(std::vector<stage_ptr> &stages, const config &cfg);


	void create_goal(std::vector<goal_ptr> &goals, const config &cfg);


	void create_engine(std::vector<engine_ptr> &engines, const config &cfg);


	void replace_aspect(std::map<std::string,aspect_ptr> &aspects, const config &cfg, std::string id);


	void on_create();

	/**
	 * unwrap
	 */
	virtual ai_context& get_ai_context();


	virtual std::string get_id() const;
	virtual std::string get_name() const;
	virtual std::string get_engine() const;

	static config preparse_cfg(ai_context& ctx, const config& cfg);

protected:

	/**
	 * Config of the AI
	 */
	const config &cfg_;


	/**
	 * Stages of the composite AI
	 */
	std::vector< stage_ptr > stages_;


	/**
	 * Recursion counter
	 */
	recursion_counter recursion_counter_;
};

} //end of namespace ai
