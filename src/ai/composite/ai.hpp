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
 * @file ai/composite/ai.hpp
 * Composite AI with turn sequence which is a vector of stages
 */

#ifndef AI_COMPOSITE_AI_HPP_INCLUDED
#define AI_COMPOSITE_AI_HPP_INCLUDED

#include "../../global.hpp"

#include "contexts.hpp"
#include "engine.hpp"
#include "stage.hpp"
#include "../contexts.hpp"
#include "../default/contexts.hpp"
#include "../ai_interface.hpp"

#include <vector>

//============================================================================
namespace ai {

namespace composite_ai {

class ai_composite : public composite_ai_context, public virtual default_ai_context_proxy, public interface {
public:


	/**
	 * Constructor
	 */
	ai_composite( default_ai_context &context );


	/**
	 * Destructor
	 */
	virtual ~ai_composite();


	/**
	 * Play the turn
	 */
	void play_turn();


	/**
	 * get engine by cfg, creating it if it is not created yet but known
	 */
	virtual engine_ptr get_engine(const config& cfg);


	std::string describe_self();


	int get_recursion_count() const;


	void switch_side(side_number side);


	void on_create();

	/**
	 * unwrap
	 */
	virtual composite_ai_context& get_composite_ai_context();

protected:

	/**
	 * Stages of the composite AI
	 */
	std::vector< stage_ptr > stages_;

	/**
	 * Engines of the composite AI
	 */
	std::vector< engine_ptr > engines_;


	/**
	 * Recursion counter
	 */
	recursion_counter recursion_counter_;
};

} //end of namespace composite_ai

} //end of namespace ai

#endif
