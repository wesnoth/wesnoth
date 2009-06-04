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
 * @file ai/testing/ca.hpp
 */

#ifndef AI_TESTING_CA_HPP_INCLUDED
#define AI_TESTING_CA_HPP_INCLUDED

#include "../../global.hpp"

#include "../composite/rca.hpp"
#include "../composite/engine_default.hpp"

namespace testing_ai_default {

//============================================================================

class goto_phase : public ai::composite_ai::candidate_action {
public:

	goto_phase( ai::composite_ai::rca_context &context, const config &cfg );

	virtual ~goto_phase();

	virtual double evaluate();

	virtual bool execute();

};

//============================================================================

class recruitment_phase : public ai::composite_ai::candidate_action {
public:

	recruitment_phase( ai::composite_ai::rca_context &context, const config &cfg );

	virtual ~recruitment_phase();

	virtual double evaluate();

	virtual bool execute();

};
/*
//============================================================================

class combat_phase : public ai_candidate_action {
public:

	combat_phase( ai_readonly_context& ai, const std::string& name, const std::string& type );

	virtual ~combat_phase();

	virtual double evaluate();

	virtual bool execute(ai_readwrite_context& context);

};

//============================================================================

class move_leader_to_goals_phase : public ai_candidate_action {
public:

	move_leader_to_goals_phase( ai_readonly_context& ai, const std::string& name, const std::string& type );

	virtual ~move_leader_to_goals_phase();

	virtual double evaluate();

	virtual bool execute(ai_readwrite_context& ai);

};

//============================================================================

class get_villages_phase : public ai_candidate_action {
public:

	get_villages_phase( ai_readonly_context& ai, const std::string& name, const std::string& type );

	virtual ~get_villages_phase();

	virtual double evaluate();

	virtual bool execute(ai_readwrite_context& ai);

};

//============================================================================

class get_healing_phase : public ai_candidate_action {
public:

	get_healing_phase( ai_readonly_context& ai, const std::string& name, const std::string& type );

	virtual ~get_healing_phase();

	virtual double evaluate();

	virtual bool execute(ai_readwrite_context& ai);

};

//============================================================================

class retreat_phase : public ai_candidate_action {
public:

	retreat_phase( ai_readonly_context& ai, const std::string& name, const std::string& type );

	virtual ~retreat_phase();

	virtual double evaluate();

	virtual bool execute(ai_readwrite_context& ai);

};

//============================================================================

class move_and_targeting_phase : public ai_candidate_action {
public:

	move_and_targeting_phase( ai_readonly_context& ai, const std::string& name, const std::string& type );

	virtual ~move_and_targeting_phase();

	virtual double evaluate();

	virtual bool execute(ai_readwrite_context& ai);

};

//============================================================================

class leader_control_phase : public ai_candidate_action {
public:

	leader_control_phase( ai_readonly_context& ai, const std::string& name, const std::string& type );

	virtual ~leader_control_phase();

	virtual double evaluate();

	virtual bool execute(ai_readwrite_context& ai);

};


//============================================================================

*/

}

#endif
