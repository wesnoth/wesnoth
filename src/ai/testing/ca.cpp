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


namespace ai {

namespace testing_ai_default {

//==============================================================

goto_phase::goto_phase( rca_context &context, const config &cfg )
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

//==============================================================

recruitment_phase::recruitment_phase( rca_context &context, const config &cfg )
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

//==============================================================

combat_phase::combat_phase( rca_context &context, const config &cfg )
	: candidate_action(context,"testing_ai_default::combat_phase",cfg["type"])
{
}

combat_phase::~combat_phase()
{
}

double combat_phase::evaluate()
{
	ERR_AI_TESTING_AI_DEFAULT << get_name() << ": evaluate - not yet implemented!" << std::endl;
	return BAD_SCORE;
}

bool combat_phase::execute()
{
	ERR_AI_TESTING_AI_DEFAULT << get_name() << ": execute - not yet implemented!" << std::endl;
	return true;
}

//==============================================================

move_leader_to_goals_phase::move_leader_to_goals_phase( rca_context &context, const config &cfg )
	: candidate_action(context,"testing_ai_default::move_leader_to_goals_phase",cfg["type"])
{
}

move_leader_to_goals_phase::~move_leader_to_goals_phase()
{
}

double move_leader_to_goals_phase::evaluate()
{
	ERR_AI_TESTING_AI_DEFAULT << get_name() << ": evaluate - not yet implemented!" << std::endl;
	return BAD_SCORE;
}

bool move_leader_to_goals_phase::execute()
{
	ERR_AI_TESTING_AI_DEFAULT << get_name() << ": execute - not yet implemented!" << std::endl;
	return true;
}

//==============================================================

get_villages_phase::get_villages_phase( rca_context &context, const config &cfg )
	: candidate_action(context,"testing_ai_default::get_villages_phase",cfg["type"])
{
}

get_villages_phase::~get_villages_phase()
{
}

double get_villages_phase::evaluate()
{
	ERR_AI_TESTING_AI_DEFAULT << get_name() << ": evaluate - not yet implemented!" << std::endl;
	return BAD_SCORE;
}

bool get_villages_phase::execute()
{
	ERR_AI_TESTING_AI_DEFAULT << get_name() << ": execute - not yet implemented!" << std::endl;
	return true;
}

//==============================================================

get_healing_phase::get_healing_phase( rca_context &context, const config &cfg )
	: candidate_action(context,"testing_ai_default::get_healing_phase",cfg["type"])
{
}

get_healing_phase::~get_healing_phase()
{
}

double get_healing_phase::evaluate()
{
	ERR_AI_TESTING_AI_DEFAULT << get_name() << ": evaluate - not yet implemented!" << std::endl;
	return BAD_SCORE;
}

bool get_healing_phase::execute()
{
	ERR_AI_TESTING_AI_DEFAULT << get_name() << ": execute - not yet implemented!" << std::endl;
	return true;
}

//==============================================================

retreat_phase::retreat_phase( rca_context &context, const config &cfg )
	: candidate_action(context,"testing_ai_default::retreat_phase",cfg["type"])
{
}

retreat_phase::~retreat_phase()
{
}

double retreat_phase::evaluate()
{
	ERR_AI_TESTING_AI_DEFAULT << get_name() << ": evaluate - not yet implemented!" << std::endl;
	return BAD_SCORE;
}

bool retreat_phase::execute()
{
	ERR_AI_TESTING_AI_DEFAULT << get_name() << ": execute - not yet implemented!" << std::endl;
	return true;
}

//==============================================================

move_and_targeting_phase::move_and_targeting_phase( rca_context &context, const config &cfg )
	: candidate_action(context,"testing_ai_default::move_and_targeting_phase",cfg["type"])
{
}

move_and_targeting_phase::~move_and_targeting_phase()
{
}

double move_and_targeting_phase::evaluate()
{
	ERR_AI_TESTING_AI_DEFAULT << get_name() << ": evaluate - not yet implemented!" << std::endl;
	return BAD_SCORE;
}

bool move_and_targeting_phase::execute()
{
	ERR_AI_TESTING_AI_DEFAULT << get_name() << ": execute - not yet implemented!" << std::endl;
	return true;
}

//==============================================================

leader_control_phase::leader_control_phase( rca_context &context, const config &cfg )
	: candidate_action(context,"testing_ai_default::leader_control_phase",cfg["type"])
{
}

leader_control_phase::~leader_control_phase()
{
}

double leader_control_phase::evaluate()
{
	ERR_AI_TESTING_AI_DEFAULT << get_name() << ": evaluate - not yet implemented!" << std::endl;
	return BAD_SCORE;
}

bool leader_control_phase::execute()
{
	ERR_AI_TESTING_AI_DEFAULT << get_name() << ": execute - not yet implemented!" << std::endl;
	return true;
}

//==============================================================

} //end of namespace testing_ai_default

} //end of namespace ai
