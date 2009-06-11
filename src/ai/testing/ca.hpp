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

#ifdef _MSC_VER
#pragma warning(push)
//silence "inherits via dominance" warnings
#pragma warning(disable:4250)
#endif

namespace ai {

namespace testing_ai_default {

using composite_ai::rca_context;
using composite_ai::candidate_action;

//============================================================================

class goto_phase : public candidate_action {
public:

	goto_phase( rca_context &context, const config &cfg );

	virtual ~goto_phase();

	virtual double evaluate();

	virtual bool execute();

};

//============================================================================

class recruitment_phase : public candidate_action {
public:

	recruitment_phase( rca_context &context, const config &cfg );

	virtual ~recruitment_phase();

	virtual double evaluate();

	virtual bool execute();

};

//============================================================================

class combat_phase : public candidate_action {
public:

	combat_phase( rca_context &context, const config &cfg );

	virtual ~combat_phase();

	virtual double evaluate();

	virtual bool execute();

};

//============================================================================

class move_leader_to_goals_phase : public candidate_action {
public:

	move_leader_to_goals_phase( rca_context &context, const config &cfg );

	virtual ~move_leader_to_goals_phase();

	virtual double evaluate();

	virtual bool execute();

};

//============================================================================

class get_villages_phase : public candidate_action {
public:

	get_villages_phase( rca_context &context, const config& cfg );

	virtual ~get_villages_phase();

	virtual double evaluate();

	virtual bool execute();

};

//============================================================================

class get_healing_phase : public candidate_action {
public:

	get_healing_phase( rca_context &context, const config& cfg );

	virtual ~get_healing_phase();

	virtual double evaluate();

	virtual bool execute();

};

//============================================================================

class retreat_phase : public candidate_action {
public:

	retreat_phase( rca_context &context, const config &cfg );

	virtual ~retreat_phase();

	virtual double evaluate();

	virtual bool execute();

};

//============================================================================

class move_and_targeting_phase : public candidate_action {
public:

	move_and_targeting_phase( rca_context &context, const config &cfg );

	virtual ~move_and_targeting_phase();

	virtual double evaluate();

	virtual bool execute();

};

//============================================================================

class leader_control_phase : public candidate_action {
public:

	leader_control_phase( rca_context &context, const config &cfg );

	virtual ~leader_control_phase();

	virtual double evaluate();

	virtual bool execute();

};


//============================================================================

} // end of namespace testing_ai_default

} // end of namespace ai

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
