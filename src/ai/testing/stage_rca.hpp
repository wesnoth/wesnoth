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
 * @file ai/testing/stage_rca.hpp
 * candidate action evaluator
 */

#ifndef AI_TESTING_STAGE_RCA_HPP_INCLUDED
#define AI_TESTING_STAGE_RCA_HPP_INCLUDED

#include "../../global.hpp"

#include "../composite/contexts.hpp"
#include "../composite/component.hpp"
#include "../composite/rca.hpp"
#include "../composite/stage.hpp"
#include "../../config.hpp"

#include <boost/shared_ptr.hpp>
#include <vector>

#ifdef _MSC_VER
#pragma warning(push)
//silence "inherits via dominance" warnings
#pragma warning(disable:4250)
#endif

namespace ai {

namespace testing_ai_default {

class candidate_action_evaluation_loop: public virtual stage, public virtual rca_context {
public:
	candidate_action_evaluation_loop( ai_context &context, const config &cfg );

	~candidate_action_evaluation_loop();

	bool do_play_stage();

	void on_create();

	config to_config() const;

	rca_context& get_rca_context();

	virtual bool add_child(const path_element &child, const config &cfg);

	virtual bool change_child(const path_element &child, const config &cfg);

	virtual bool delete_child(const path_element &child);


	void create_candidate_action(std::vector<candidate_action_ptr> &candidate_actions, const config &cfg);

private:
	std::vector<candidate_action_ptr> candidate_actions_;

	const config &cfg_;
};


} // of namespace testing_ai_default

} // of namespace ai

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
