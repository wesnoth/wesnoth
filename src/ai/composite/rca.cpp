/*
   Copyright (C) 2009 - 2017 by Yurii Chernyi <terraninfo@terraninfo.net>
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
 * Candidate actions framework
 * @file
 */

#include "ai/composite/ai.hpp"
#include "ai/composite/engine.hpp"
#include "ai/composite/rca.hpp"
#include "log.hpp"

namespace ai {

static lg::log_domain log_ai_stage_rca("ai/stage/rca");
#define DBG_AI_STAGE_RCA LOG_STREAM(debug, log_ai_stage_rca)
#define LOG_AI_STAGE_RCA LOG_STREAM(info, log_ai_stage_rca)
#define ERR_AI_STAGE_RCA LOG_STREAM(err, log_ai_stage_rca)

const double candidate_action::BAD_SCORE = 0;
const double candidate_action::HIGH_SCORE = 10000000;

candidate_action::candidate_action(rca_context &context, const config &cfg):
	recursion_counter_(context.get_recursion_count()),
	enabled_(cfg["enabled"].to_bool(true)), engine_(cfg["engine"]),
	score_(cfg["score"].to_double(BAD_SCORE)),
	max_score_(cfg["max_score"].to_double(HIGH_SCORE)),
	id_(cfg["id"]), name_(cfg["name"]), type_(cfg["type"]), to_be_removed_(false)
{
	init_rca_context_proxy(context);
}

candidate_action::~candidate_action()
{
}


bool candidate_action::is_enabled() const
{
	return enabled_;
}


void candidate_action::enable()
{
	enabled_ = true;
}

int candidate_action::get_recursion_count() const
{
	return recursion_counter_.get_count();
}

void candidate_action::disable()
{
	enabled_ = false;
}


double candidate_action::get_score() const
{
	return score_;
}


double candidate_action::get_max_score() const
{
	return max_score_;
}

const std::string& candidate_action::get_type() const
{
	return type_;
}

config candidate_action::to_config() const
{
	config cfg;
	cfg["enabled"] = enabled_;
	cfg["engine"] = engine_;
	cfg["id"] = id_;
	cfg["name"] = name_;
	cfg["score"] = score_;
	cfg["max_score"] = max_score_;
	cfg["type"] = type_;
	return cfg;
}

void candidate_action::set_to_be_removed()
{
	to_be_removed_ = true;
}

bool candidate_action::to_be_removed()
{
	return to_be_removed_;
}

// This is defined in the source file so that it can easily access the logger
bool candidate_action_factory::is_duplicate(const std::string& name)
{
	if (get_list().find(name) != get_list().end()) {
		ERR_AI_STAGE_RCA << "Error: Attempt to double-register candidate action " << name << std::endl;
		return true;
	}
	return false;
}

//============================================================================

std::ostream &operator<<(std::ostream &s, ai::candidate_action const &ca) {
	s << "candidate action with name ["<< ca.get_name() <<"]";
	return s;
}

} // of namespace ai
