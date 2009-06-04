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
 * Default AI (Testing)
 * @file ai/testing/ca.cpp
 */
#include "../../global.hpp"

#include "ca.hpp"
#include "../composite/engine.hpp"
#include "../composite/rca.hpp"
#include "../../log.hpp"

static lg::log_domain log_ai_testing_ai_default("ai/testing/ai_default");
#define DBG_AI_TESTING_AI_DEFAULT LOG_STREAM(debug, log_ai_testing_ai_default)
#define LOG_AI_TESTING_AI_DEFAULT LOG_STREAM(info, log_ai_testing_ai_default)
#define ERR_AI_TESTING_AI_DEFAULT LOG_STREAM(err, log_ai_testing_ai_default)


namespace testing_ai_default {

goto_phase::goto_phase( ai::composite_ai::rca_context &context, const config &cfg )
	: candidate_action(context,"testing_ai_default::goto_phase",cfg["type"])
{
}

goto_phase::~goto_phase()
{
}

double goto_phase::evaluate()
{
	ERR_AI_TESTING_AI_DEFAULT << get_name() << ": evaluate - not yet implemented" << std::endl;
	return BAD_SCORE;
}

bool goto_phase::execute()
{
	ERR_AI_TESTING_AI_DEFAULT << get_name() << ": execute - not yet implemented" << std::endl;
	return true;
}

recruitment_phase::recruitment_phase( ai::composite_ai::rca_context &context, const config &cfg )
	: candidate_action(context,"testing_ai_default::recruitment_phase",cfg["type"])
{
}

recruitment_phase::~recruitment_phase()
{
}

double recruitment_phase::evaluate()
{
	ERR_AI_TESTING_AI_DEFAULT << get_name() << ": evaluate - not yet implemented!" << std::endl;
	return BAD_SCORE;
}

bool recruitment_phase::execute()
{
	ERR_AI_TESTING_AI_DEFAULT << get_name() << ": execute - not yet implemented!" << std::endl;
	return true;
}


}
