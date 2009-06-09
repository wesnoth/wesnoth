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
 * Stage: fallback to other AI
 * @file ai/testing/rca_default.cpp
 */

#include "stage_fallback.hpp"

#include "../manager.hpp"
#include "../composite/ai.hpp"
#include "../../foreach.hpp"
#include "../../log.hpp"

namespace ai {

namespace testing_ai_default {

static lg::log_domain log_ai_testing_stage_fallback("ai/testing/stage_fallback");
#define DBG_AI_TESTING_STAGE_FALLBACK LOG_STREAM(debug, log_ai_testing_stage_fallback)
#define LOG_AI_TESTING_STAGE_FALLBACK LOG_STREAM(info, log_ai_testing_stage_fallback)
#define ERR_AI_TESTING_STAGE_FALLBACK LOG_STREAM(err, log_ai_testing_stage_fallback)

fallback_to_other_ai::fallback_to_other_ai( composite_ai::composite_ai_context &context, const config &cfg )
	: stage(context,cfg), cfg_(cfg), fallback_ai_()
{
}

void fallback_to_other_ai::on_create()
{
	fallback_ai_ = manager::create_transient_ai(cfg_["fallback"], this);
}

void fallback_to_other_ai::do_play_stage()
{
	if (fallback_ai_) {
		LOG_AI_TESTING_STAGE_FALLBACK << "side "<<get_side()<<" : falling back to "<<fallback_ai_->describe_self()<<std::endl;
		fallback_ai_->play_turn();
	} else {
		ERR_AI_TESTING_STAGE_FALLBACK << "side "<<get_side()<<" : UNABLE TO FALLBACK, fallback ai is NULL"<<std::endl;
	}
}

fallback_to_other_ai::~fallback_to_other_ai()
{
}

} // end of namespace testing_ai_default

} // end of namespace ai
