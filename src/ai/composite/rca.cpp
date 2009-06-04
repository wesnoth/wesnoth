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

namespace composite_ai {

static lg::log_domain log_ai_composite_rca("ai/composite/rca");
#define DBG_AI_COMPOSITE_RCA LOG_STREAM(debug, log_ai_composite_rca)
#define LOG_AI_COMPOSITE_RCA LOG_STREAM(info, log_ai_composite_rca)
#define ERR_AI_COMPOSITE_RCA LOG_STREAM(err, log_ai_composite_rca)


candidate_action::candidate_action(rca_context &context, const std::string &name, const std::string &type)
	: recursion_counter_(context.get_recursion_count()), enabled_(true), score_(BAD_SCORE),name_(name),type_(type)
{
	init_rca_context_proxy(context);
	//LOG_AI_COMPOSITE_RCA << "side "<< get_side() << " : "<<" created "<<*this<<std::endl;
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


const std::string& candidate_action::get_name() const
{
	return name_;
}


const std::string& candidate_action::get_type() const
{
	return type_;
}

candidate_action_evaluation_exception::candidate_action_evaluation_exception(const std::string &message)
	: message_(message)
{
}

candidate_action_execution_exception::candidate_action_execution_exception(const std::string &message)
	: message_(message)
{
}

candidate_action_evaluation_exception::~candidate_action_evaluation_exception()
{
}

candidate_action_execution_exception::~candidate_action_execution_exception()
{
}

const std::string& candidate_action_evaluation_exception::get_message() const
{
	return message_;
}

const std::string& candidate_action_execution_exception::get_message() const
{
	return message_;
}


//============================================================================
} //end of namespace composite_ai

} // of namespace ai


std::ostream &operator<<(std::ostream &s, ai::composite_ai::candidate_action_evaluation_exception const &caee) {
	s << "candidate action evaluation exception :"<< caee.get_message();
	return s;
}


std::ostream &operator<<(std::ostream &s, ai::composite_ai::candidate_action_execution_exception const &caee) {
	s << "candidate action execution exception :"<< caee.get_message();
	return s;
}


std::ostream &operator<<(std::ostream &s, ai::composite_ai::candidate_action const &ca) {
	s << "candidate action with name ["<< ca.get_name() <<"]";
	return s;
}

