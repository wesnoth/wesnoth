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
 * Candidate actions framework
 * @file ai/composite/rca.cpp
 */

#include "ai.hpp"
#include "engine.hpp"
#include "rca.hpp"
#include "../../foreach.hpp"
#include "../../log.hpp"

namespace ai {

static lg::log_domain log_ai_composite_rca("ai/composite/rca");
#define DBG_AI_COMPOSITE_RCA LOG_STREAM(debug, log_ai_composite_rca)
#define LOG_AI_COMPOSITE_RCA LOG_STREAM(info, log_ai_composite_rca)
#define ERR_AI_COMPOSITE_RCA LOG_STREAM(err, log_ai_composite_rca)

const double candidate_action::BAD_SCORE = 0;
const double candidate_action::HIGH_SCORE = 100000;

candidate_action::candidate_action(rca_context &context, const config &cfg)
	: recursion_counter_(context.get_recursion_count()), enabled_(utils::string_bool(cfg["enabled"],true)), engine_(cfg["engine"]), score_(lexical_cast_default<double>(cfg["score"],BAD_SCORE)),max_score_(lexical_cast_default<double>(cfg["max_score"],HIGH_SCORE)),id_(cfg["id"]),name_(cfg["name"]),type_(cfg["type"])
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


const std::string& candidate_action::get_name() const
{
	return name_;
}


const std::string& candidate_action::get_type() const
{
	return type_;
}


const std::string& candidate_action::get_id() const
{
	return id_;
}


const std::string& candidate_action::get_engine() const
{
	return engine_;
}


config candidate_action::to_config() const
{
	config cfg;
	cfg["enabled"] = lexical_cast<std::string>(enabled_);
	cfg["engine"] = engine_;
	cfg["name"] = name_;
	cfg["score"] = lexical_cast<std::string>(score_);
	cfg["max_score"] = lexical_cast<std::string>(max_score_);
	cfg["type"] = type_;
	return cfg;
}

//============================================================================

} // of namespace ai


std::ostream &operator<<(std::ostream &s, ai::candidate_action const &ca) {
	s << "candidate action with name ["<< ca.get_name() <<"]";
	return s;
}

