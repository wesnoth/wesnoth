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
 * All known AI parts. Workaround to a linker feature of not includning all symbols from files, and including only actually referenced stuff. this is not good for 'static registry' pattern. (other workarounds such as --whole-archive for ld are possible, but require messing with all buildsystems)
 * @file ai/registry.cpp
 */

#include "ai.hpp"
#include "ai2.hpp"
#include "ai_dfool.hpp"
#include "composite/ai.hpp"
#include "composite/engine_default.hpp"
#include "formula_ai.hpp"
#include "registry.hpp"
#include "testing/ca.hpp"
#include "testing/stage_rca.hpp"
#include "testing/stage_fallback.hpp"

// =======================================================================
// AIs
// =======================================================================
/*
static ai_factory<ai> ai_factory("");
static ai_factory<ai> default_ai_ai_factory("default_ai");
static ai_factory<ai2> ai2_ai_factory("ai2");
static ai_factory<dfool_ai> dfool_ai_ai_factory("dfool_ai");
static ai_factory<formula_ai> formula_ai_ai_factory("formula_ai");
static ai_factory<composite_ai> composite_ai_ai_factory("composite_ai");
*/

// =======================================================================
// Engines
// =======================================================================

static ai::composite_ai::register_engine_factory<ai::composite_ai::engine_cpp>
	composite_ai_factory("cpp");

// =======================================================================
// Stages
// =======================================================================
static ai::composite_ai::register_stage_factory<testing_ai_default::candidate_action_evaluation_loop>
	candidate_action_evaluation_loop_factory("testing_ai_default::candidate_action_evaluation_loop");

static ai::composite_ai::register_stage_factory<testing_ai_default::fallback_to_other_ai>
	fallback_to_other_ai_factory("testing_ai_default::fallback");

// =======================================================================
// Candidate actions
// =======================================================================

static ai::composite_ai::register_candidate_action_factory<testing_ai_default::goto_phase>
	goto_phase_factory("testing_ai_default::goto_phase");

static ai::composite_ai::register_candidate_action_factory<testing_ai_default::recruitment_phase>
	recruitment_phase_factory("testing_ai_default::recruitment_phase");


void ai_registry::init()
{
}

ai_registry::ai_registry()
{
}
