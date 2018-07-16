/*
   Copyright (C) 2009 - 2018 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
 * candidate action evaluator
 */

#pragma once

#include "ai/composite/rca.hpp"
#include "ai/composite/stage.hpp"

namespace ai {

namespace ai_default_rca {

class candidate_action_evaluation_loop: public virtual stage, public virtual rca_context {
public:
	candidate_action_evaluation_loop( ai_context &context, const config &cfg );

	~candidate_action_evaluation_loop();

	bool do_play_stage();

	void on_create();

	config to_config() const;

	rca_context& get_rca_context();

	void remove_completed_cas();

private:
	std::vector<candidate_action_ptr> candidate_actions_;

	const config &cfg_;
};


} // of namespace testing_ai_default

} // of namespace ai
