/*
   Copyright (C) 2009 - 2015 by Yurii Chernyi <terraninfo@terraninfo.net>
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
 * Stage: fallback to other AI
 * @file
 */

#include "stage_fallback.hpp"

#include "../configuration.hpp"
#include "../manager.hpp"
#include "../composite/ai.hpp"
#include "../../log.hpp"

namespace ai {

namespace testing_ai_default {

static lg::log_domain log_ai_testing_stage_fallback("ai/stage/fallback");
#define DBG_AI_TESTING_STAGE_FALLBACK LOG_STREAM(debug, log_ai_testing_stage_fallback)
#define LOG_AI_TESTING_STAGE_FALLBACK LOG_STREAM(info, log_ai_testing_stage_fallback)
#define ERR_AI_TESTING_STAGE_FALLBACK LOG_STREAM(err, log_ai_testing_stage_fallback)

fallback_to_other_ai::fallback_to_other_ai( ai_context &context, const config &cfg )
	: stage(context,cfg), cfg_(cfg), fallback_ai_()
{
}

void fallback_to_other_ai::on_create()
{
	config ai_cfg = cfg_.child_or_empty("ai");
	///@deprecated 1.9.3 backward-compatibility hack - try to update the old default ai config.
	std::string ai_algorithm = ai_cfg["ai_algorithm"];
	if ((ai_algorithm.empty()) || (ai_algorithm=="default_ai")) {
		if (configuration::parse_side_config(get_side(),cfg_,ai_cfg)) {
				fallback_ai_ = manager::create_transient_ai("", ai_cfg, this);
		}
	}

}


config fallback_to_other_ai::to_config() const
{
	config cfg = stage::to_config();

	if (fallback_ai_) {
		cfg.add_child("ai",fallback_ai_->to_config());
	}
	return cfg;
}

bool fallback_to_other_ai::do_play_stage()
{
	if (fallback_ai_) {
		LOG_AI_TESTING_STAGE_FALLBACK << "side "<<get_side()<<" : falling back to "<<fallback_ai_->describe_self()<<std::endl;
		fallback_ai_->new_turn();
		fallback_ai_->play_turn();
	} else {
		ERR_AI_TESTING_STAGE_FALLBACK << "side "<<get_side()<<" : UNABLE TO FALLBACK, fallback ai is NULL"<<std::endl;
	}
	return false;
}

fallback_to_other_ai::~fallback_to_other_ai()
{
}

} // end of namespace testing_ai_default

} // end of namespace ai
