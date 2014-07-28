/*
   Copyright (C) 2009 - 2014 by Yurii Chernyi <terraninfo@terraninfo.net>
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
 * All known c++ AI parts. Workaround to a linker feature of not including all symbols from files, and including only actually referenced stuff. this is not good for 'static registry' pattern. (other workarounds such as --whole-archive for ld are possible, but require messing with all buildsystems)
 * @file
 */

#include "registry.hpp"
#include "global.hpp"

#include "../config.hpp"             // for config, operator<<
#include "../terrain_filter.hpp"  // for terrain_filter
#include "ai/composite/engine.hpp"      // for register_engine_factory
#include "ai/composite/stage.hpp"       // for ministage, idle_stage, etc
#include "ai/composite/rca.hpp"
#include "ai/game_info.hpp"             // for attacks_vector
#include "ai/interface.hpp"  // for register_ai_factory
#include "akihara/recruitment.hpp"      // for recruitment
#include "composite/ai.hpp"             // for ai_composite
#include "composite/aspect.hpp"         // for composite_aspect, etc
#include "composite/engine_default.hpp"  // for engine_cpp
#include "composite/engine_fai.hpp"     // for engine_fai
#include "composite/engine_lua.hpp"     // for engine_lua
#include "composite/goal.hpp"           // for register_goal_factory, etc
#include "default/ai.hpp"
#include "lua/unit_advancements_aspect.hpp"
#include "recruitment/recruitment.hpp"  // for recruitment
#include "testing/aspect_attacks.hpp"   // for aspect_attacks
#include "testing/ca.hpp"               // for leader_shares_keep_phase, etc
#include "testing/ca_global_fallback.hpp"  // for global_fallback_phase
#include "testing/ca_testing_move_to_targets.hpp"
#include "testing/ca_testing_recruitment.hpp"
#include "testing/stage_sf_with_rca.hpp"
#include "testing/stage_fallback.hpp"   // for fallback_to_other_ai
#include "testing/stage_rca.hpp"

#include <boost/shared_ptr.hpp>         // for shared_ptr, etc
#include <string>                       // for string
#include <vector>                       // for vector


namespace ai {
// =======================================================================
// AIs
// =======================================================================

static register_ai_factory<ai_composite> ai_factory_default("");
static register_ai_factory<ai_composite> ai_default_ai_factory("default_ai");
static register_ai_factory<idle_ai> ai_idle_ai_factory("idle_ai");
static register_ai_factory<ai_composite> ai_composite_ai_factory("composite_ai");


// =======================================================================
// Engines
// =======================================================================

static register_engine_factory<engine_cpp>
	composite_ai_factory_cpp("cpp");

static register_engine_factory<engine_fai>
	composite_ai_factory_fai("fai");

static register_engine_factory<engine_lua>
	composite_ai_factory_lua("lua");

// =======================================================================
// Stages
// =======================================================================

static register_stage_factory<testing_ai_default::candidate_action_evaluation_loop>
	candidate_action_evaluation_loop_factory("ai_default_rca::candidate_action_evaluation_loop");

static register_stage_factory<testing_ai_default::strategy_formulation_with_rca>
	strategy_formulation_with_rca_factory("testing_ai_default::strategy_formulation_with_rca");

static register_stage_factory<testing_ai_default::fallback_to_other_ai>
	fallback_to_other_ai_factory("testing_ai_default::fallback");

static register_stage_factory<ai_default_recruitment_stage>
	ai_default_recruitment_stage_factory("ai_default::recruitment");

static register_stage_factory<idle_stage>
	ai_idle_stage_factory("empty");

// === Also keep the old syntax ===
static register_stage_factory<testing_ai_default::candidate_action_evaluation_loop>
	old_candidate_action_evaluation_loop_factory("testing_ai_default::candidate_action_evaluation_loop");

// =======================================================================
// Candidate actions
// =======================================================================

static register_candidate_action_factory<testing_ai_default::goto_phase>
	goto_phase_factory("ai_default_rca::goto_phase");

static register_candidate_action_factory<testing_ai_default::aspect_recruitment_phase>
	aspect_recruitment_phase_factory("ai_default_rca::aspect_recruitment_phase");

static register_candidate_action_factory<testing_ai_default::recruitment_phase>
	recruitment_phase_factory("ai_default_rca::recruitment_phase");

static register_candidate_action_factory<testing_ai_default::combat_phase>
	combat_phase_factory("ai_default_rca::combat_phase");

static register_candidate_action_factory<testing_ai_default::move_leader_to_goals_phase>
	move_leader_to_goals_phase_factory("ai_default_rca::move_leader_to_goals_phase");

static register_candidate_action_factory<testing_ai_default::move_leader_to_keep_phase>
	move_leader_to_keep_phase_factory("ai_default_rca::move_leader_to_keep_phase");

static register_candidate_action_factory<testing_ai_default::get_villages_phase>
	get_villages_phase_factory("ai_default_rca::get_villages_phase");

static register_candidate_action_factory<testing_ai_default::get_healing_phase>
	get_healing_phase_factory("ai_default_rca::get_healing_phase");

static register_candidate_action_factory<testing_ai_default::retreat_phase>
	retreat_phase_factory("ai_default_rca::retreat_phase");

static register_candidate_action_factory<testing_ai_default::simple_move_and_targeting_phase>
	simple_move_and_targeting_phase_factory("ai_default_rca::simple_move_and_targeting_phase");

static register_candidate_action_factory<testing_ai_default::testing_move_to_targets_phase>
	default_move_to_targets_phase_factory("ai_default_rca::move_to_targets_phase");

static register_candidate_action_factory<testing_ai_default::leader_control_phase>
	leader_control_phase_factory("ai_default_rca::leader_control_phase");

static register_candidate_action_factory<testing_ai_default::testing_recruitment_phase>
	testing_recruitment_phase_factory("ai_default_rca::testing_recruitment_phase");

static register_candidate_action_factory<testing_ai_default::leader_shares_keep_phase>
	leader_shares_keep_phase_factory("ai_default_rca::leader_shares_keep_phase");

//Also keep passive_leader_shares_keep_phase for backward compatibility
static register_candidate_action_factory<testing_ai_default::leader_shares_keep_phase>
	passive_leader_shares_keep_phase_factory("ai_default_rca::passive_leader_shares_keep_phase");

static register_candidate_action_factory<testing_ai_default::global_fallback_phase>
	global_fallback_phase_factory("ai_default_rca::global_fallback_phase");

static register_candidate_action_factory<akihara_recruitment::recruitment>
	recruitment_factory("akihara_recruitment::recruitment");

static register_candidate_action_factory<default_recruitment::recruitment>
	default_recruitment_factory("default_recruitment::recruitment");

// === Also keep the old syntax ===
static register_candidate_action_factory<testing_ai_default::goto_phase>
	old_goto_phase_factory("testing_ai_default::goto_phase");

static register_candidate_action_factory<testing_ai_default::aspect_recruitment_phase>
	old_aspect_recruitment_phase_factory("testing_ai_default::aspect_recruitment_phase");

static register_candidate_action_factory<testing_ai_default::recruitment_phase>
	old_recruitment_phase_factory("testing_ai_default::recruitment_phase");

static register_candidate_action_factory<testing_ai_default::combat_phase>
	old_combat_phase_factory("testing_ai_default::combat_phase");

static register_candidate_action_factory<testing_ai_default::move_leader_to_goals_phase>
	old_move_leader_to_goals_phase_factory("testing_ai_default::move_leader_to_goals_phase");

static register_candidate_action_factory<testing_ai_default::move_leader_to_keep_phase>
	old_move_leader_to_keep_phase_factory("testing_ai_default::move_leader_to_keep_phase");

static register_candidate_action_factory<testing_ai_default::get_villages_phase>
	old_get_villages_phase_factory("testing_ai_default::get_villages_phase");

static register_candidate_action_factory<testing_ai_default::get_healing_phase>
	old_get_healing_phase_factory("testing_ai_default::get_healing_phase");

static register_candidate_action_factory<testing_ai_default::retreat_phase>
	old_retreat_phase_factory("testing_ai_default::retreat_phase");

static register_candidate_action_factory<testing_ai_default::simple_move_and_targeting_phase>
	old_simple_move_and_targeting_phase_factory("testing_ai_default::simple_move_and_targeting_phase");

static register_candidate_action_factory<testing_ai_default::testing_move_to_targets_phase>
	old_default_move_to_targets_phase_factory("testing_ai_default::default_move_to_targets_phase");

static register_candidate_action_factory<testing_ai_default::testing_move_to_targets_phase>
	old_testing_move_to_targets_phase_factory("testing_ai_default::testing_move_to_targets_phase");

static register_candidate_action_factory<testing_ai_default::leader_control_phase>
	old_leader_control_phase_factory("testing_ai_default::leader_control_phase");

static register_candidate_action_factory<testing_ai_default::testing_recruitment_phase>
	old_testing_recruitment_phase_factory("testing_ai_default::testing_recruitment_phase");

static register_candidate_action_factory<testing_ai_default::leader_shares_keep_phase>
	old_passive_leader_shares_keep_phase_factory("testing_ai_default::passive_leader_shares_keep_phase");

static register_candidate_action_factory<testing_ai_default::global_fallback_phase>
	old_global_fallback_phase_factory("testing_ai_default::global_fallback_phase");

// =======================================================================
// Goals
// =======================================================================

static register_goal_factory<target_unit_goal>
	goal_factory("");


static register_goal_factory<target_unit_goal>
	goal_factory_target("target");


static register_goal_factory<target_location_goal>
	goal_factory_target_location("target_location");


static register_goal_factory<protect_location_goal>
	goal_factory_protect("protect");


static register_goal_factory<protect_location_goal>
	goal_factory_protect_location("protect_location");


static register_goal_factory<protect_my_unit_goal>
	goal_factory_protect_my_unit("protect_my_unit");


static register_goal_factory<protect_unit_goal>
	goal_factory_protect_unit("protect_unit");

static register_goal_factory<lua_goal>
	goal_factory_lua_goal("lua_goal");


// =======================================================================
// Aspects
// =======================================================================

//name=composite_aspect

static register_aspect_factory< composite_aspect< unit_advancements_aspect > >
	advancements__composite_aspect_factory("advancements*composite_aspect");

static register_aspect_factory< composite_aspect<double> >
	aggression__composite_aspect_factory("aggression*composite_aspect");

static register_aspect_factory< composite_aspect<int> >
	attack_depth__composite_aspect_factory("attack_depth*composite_aspect");

static register_aspect_factory< composite_aspect< attacks_vector > >
	attacks__composite_aspect_factory("attacks*composite_aspect");

static register_aspect_factory< composite_aspect< terrain_filter > >
	avoid__composite_aspect_factory("avoid*composite_aspect");

static register_aspect_factory< composite_aspect<double> >
	caution__composite_aspect_factory("caution*composite_aspect");

static register_aspect_factory< composite_aspect<std::string> >
	grouping__composite_aspect_factory("grouping*composite_aspect");

static register_aspect_factory< composite_aspect<double> >
	leader_aggression__composite_aspect_factory("leader_aggression*composite_aspect");

static register_aspect_factory< composite_aspect<config> >
	leader_goal__composite_aspect_factory("leader_goal*composite_aspect");

static register_aspect_factory< composite_aspect<bool> >
	leader_igores_keep__composite_aspect_factory("leader_ignores_keep*composite_aspect");

static register_aspect_factory< composite_aspect<double> >
	leader_value__composite_aspect_factory("leader_value*composite_aspect");

static register_aspect_factory< composite_aspect<double> >
	number_of_possible_recruits_to_force_recruit__composite_aspect_factory("number_of_possible_recruits_to_force_recruit*composite_aspect");

static register_aspect_factory< composite_aspect<bool> >
	passive_leader__composite_aspect_factory("passive_leader*composite_aspect");

static register_aspect_factory< composite_aspect<bool> >
	passive_leader_shares_keep__composite_aspect_factory("passive_leader_shares_keep*composite_aspect");

static register_aspect_factory< composite_aspect<ministage> >
	recruitment__composite_aspect_factory("recruitment*composite_aspect");

static register_aspect_factory< composite_aspect<double> >
	recruitment_diversity__composite_aspect_factory("recruitment_diversity*composite_aspect");

static register_aspect_factory< composite_aspect<bool> >
	recruitment_ignore_bad_combat__composite_aspect_factory("recruitment_ignore_bad_combat*composite_aspect");

static register_aspect_factory< composite_aspect<bool> >
	recruitment_ignore_bad_movement__composite_aspect_factory("recruitment_ignore_bad_movement*composite_aspect");

static register_aspect_factory< composite_aspect<config> >
	recruitment_instructions__composite_aspect_factory("recruitment_instructions*composite_aspect");

static register_aspect_factory< composite_aspect< std::vector<std::string> > >
	recruitment_more__composite_aspect_factory("recruitment_more*composite_aspect");

static register_aspect_factory< composite_aspect< std::vector<std::string> > >
	recruitment_pattern__composite_aspect_factory("recruitment_pattern*composite_aspect");

static register_aspect_factory< composite_aspect<int> >
	recruitment_randomness__composite_aspect_factory("recruitment_randomness*composite_aspect");

static register_aspect_factory< composite_aspect<config> >
	recruitment_save_gold__composite_aspect_factory("recruitment_save_gold*composite_aspect");

static register_aspect_factory< composite_aspect<double> >
	scout_village_targeting__composite_aspect_factory("scout_village_targeting*composite_aspect");

static register_aspect_factory< composite_aspect<bool> >
	simple_targeting__composite_aspect_factory("simple_targeting*composite_aspect");

static register_aspect_factory< composite_aspect<bool> >
	support_villages__composite_aspect_factory("support_villages*composite_aspect");

static register_aspect_factory< composite_aspect<double> >
	village_value__composite_aspect_factory("village_value*composite_aspect");

static register_aspect_factory< composite_aspect<int> >
	villages_per_scout__composite_aspect_factory("villages_per_scout*composite_aspect");


//name=standard_aspect
static register_aspect_factory< standard_aspect< unit_advancements_aspect > >
	advancements__standard_aspect_factory("advancements*standard_aspect");

static register_aspect_factory< standard_aspect<double> >
	aggression__standard_aspect_factory("aggression*standard_aspect");

static register_aspect_factory< standard_aspect<int> >
	attack_depth__standard_aspect_factory("attack_depth*standard_aspect");

static register_aspect_factory< testing_ai_default::aspect_attacks >
	attacks__testing_ai_default_aspect_attacks_factory("attacks*ai_default_rca::aspect_attacks");

static register_aspect_factory< standard_aspect< terrain_filter > >
	avoid__standard_aspect_factory("avoid*standard_aspect");

static register_aspect_factory< standard_aspect<double> >
	caution__standard_aspect_factory("caution*standard_aspect");

static register_aspect_factory< standard_aspect<std::string> >
	grouping__standard_aspect_factory("grouping*standard_aspect");

static register_aspect_factory< standard_aspect<double> >
	leader_aggression__standard_aspect_factory("leader_aggression*standard_aspect");

static register_aspect_factory< standard_aspect<config> >
	leader_goal__standard_aspect_factory("leader_goal*standard_aspect");

static register_aspect_factory< standard_aspect<bool> >
	leader_ignores_keep__standard_aspect_factory("leader_ignores_keep*standard_aspect");

static register_aspect_factory< standard_aspect<double> >
	leader_value__standard_aspect_factory("leader_value*standard_aspect");

static register_aspect_factory< standard_aspect<double> >
	number_of_possible_recruits_to_force_recruit__standard_aspect_factory("number_of_possible_recruits_to_force_recruit*standard_aspect");

static register_aspect_factory< standard_aspect<bool> >
	passive_leader__standard_aspect_factory("passive_leader*standard_aspect");

static register_aspect_factory< standard_aspect<bool> >
	passive_leader_shares_keep__standard_aspect_factory("passive_leader_shares_keep*standard_aspect");

static register_aspect_factory< standard_aspect<ministage> >
	recruitment__standard_aspect_factory("recruitment*standard_aspect");

static register_aspect_factory< standard_aspect<double> >
	recruitment_diversity__standard_aspect_factory("recruitment_diversity*standard_aspect");

static register_aspect_factory< standard_aspect<bool> >
	recruitment_ignore_bad_combat__standard_aspect_factory("recruitment_ignore_bad_combat*standard_aspect");

static register_aspect_factory< standard_aspect<bool> >
	recruitment_ignore_bad_movement__standard_aspect_factory("recruitment_ignore_bad_movement*standard_aspect");

static register_aspect_factory< standard_aspect<config> >
	recruitment_instructions__standard_aspect_factory("recruitment_instructions*standard_aspect");

static register_aspect_factory< standard_aspect< std::vector<std::string> > >
	recruitment_more__standard_aspect_factory("recruitment_more*standard_aspect");

static register_aspect_factory< standard_aspect< std::vector<std::string> > >
	recruitment_pattern__standard_aspect_factory("recruitment_pattern*standard_aspect");

static register_aspect_factory< standard_aspect<int> >
	recruitment_randomness__standard_aspect_factory("recruitment_randomness*standard_aspect");

static register_aspect_factory< standard_aspect<config> >
	recruitment_save_gold__standard_aspect_factory("recruitment_save_gold*standard_aspect");

static register_aspect_factory< standard_aspect<double> >
	scout_village_targeting__standard_aspect_factory("scout_village_targeting*standard_aspect");

static register_aspect_factory< standard_aspect<bool> >
	simple_targeting__standard_aspect_factory("simple_targeting*standard_aspect");

static register_aspect_factory< standard_aspect<bool> >
	support_villages__standard_aspect_factory("support_villages*standard_aspect");

static register_aspect_factory< standard_aspect<double> >
	village_value__standard_aspect_factory("village_value*standard_aspect");

static register_aspect_factory< standard_aspect<int> >
	villages_per_scout__standard_aspect_factory("villages_per_scout*standard_aspect");


// Also keep the old syntax
static register_aspect_factory< testing_ai_default::aspect_attacks >
	old_attacks__testing_ai_default_aspect_attacks_factory("attacks*testing_ai_default::aspect_attacks");

//name = default
static register_aspect_factory< standard_aspect< unit_advancements_aspect > >
	advancements__standard_aspect_factory2("advancements*");

static register_aspect_factory< standard_aspect<double> >
	aggression__standard_aspect_factory2("aggression*");

static register_aspect_factory< standard_aspect<int> >
	attack_depth__standard_aspect_factory2("attack_depth*");

static register_aspect_factory< testing_ai_default::aspect_attacks >
	attacks__testing_ai_default_aspect_attacks_factory2("attacks*");

static register_aspect_factory< standard_aspect< terrain_filter > >
	avoid__standard_aspect_factory2("avoid*");

static register_aspect_factory< standard_aspect<double> >
	caution__standard_aspect_factory2("caution*");

static register_aspect_factory< standard_aspect<std::string> >
	grouping__standard_aspect_factory2("grouping*");

static register_aspect_factory< standard_aspect<double> >
	leader_aggression__standard_aspect_factory2("leader_aggression*");

static register_aspect_factory< standard_aspect<config> >
	leader_goal__standard_aspect_factory2("leader_goal*");

static register_aspect_factory< standard_aspect<bool> >
	leader_ignores_keep__standard_aspect_factory2("leader_ignores_keep*");

static register_aspect_factory< standard_aspect<double> >
	leader_value__standard_aspect_factory2("leader_value*");

static register_aspect_factory< standard_aspect<double> >
	number_of_possible_recruits_to_force_recruit__standard_aspect_factory2("number_of_possible_recruits_to_force_recruit*");

static register_aspect_factory< standard_aspect<bool> >
	passive_leader__standard_aspect_factory2("passive_leader*");

static register_aspect_factory< standard_aspect<bool> >
	passive_leader_shares_keep__standard_aspect_factory2("passive_leader_shares_keep*");

static register_aspect_factory< standard_aspect<ministage> >
	recruitment__standard_aspect_factory2("recruitment*");

static register_aspect_factory< standard_aspect<double> >
	recruitment_diversity__standard_aspect_factory2("recruitment_diversity*");

static register_aspect_factory< standard_aspect<bool> >
	recruitment_ignore_bad_combat__standard_aspect_factory2("recruitment_ignore_bad_combat*");

static register_aspect_factory< standard_aspect<bool> >
	recruitment_ignore_bad_movement__standard_aspect_factory2("recruitment_ignore_bad_movement*");

static register_aspect_factory< standard_aspect<config> >
	recruitment_instructions__standard_aspect_factory2("recruitment_instructions*");

static register_aspect_factory< standard_aspect< std::vector<std::string> > >
	recruitment_more__standard_aspect_factory2("recruitment_more*");

static register_aspect_factory< standard_aspect< std::vector<std::string> > >
	recruitment_pattern__standard_aspect_factory2("recruitment_pattern*");

static register_aspect_factory< standard_aspect<int> >
	recruitment_randomness__standard_aspect_factory2("recruitment_randomness*");

static register_aspect_factory< standard_aspect<config> >
	recruitment_save_gold__standard_aspect_factory2("recruitment_save_gold*");

static register_aspect_factory< standard_aspect<double> >
	scout_village_targeting__standard_aspect_factory2("scout_village_targeting*");

static register_aspect_factory< standard_aspect<bool> >
	simple_targeting__standard_aspect_factory2("simple_targeting*");

static register_aspect_factory< standard_aspect<bool> >
	support_villages__standard_aspect_factory2("support_villages*");

static register_aspect_factory< standard_aspect<double> >
	village_value__standard_aspect_factory2("village_value*");

static register_aspect_factory< standard_aspect<int> >
	villages_per_scout__standard_aspect_factory2("villages_per_scout*");


//name = lua
static register_lua_aspect_factory< lua_aspect< unit_advancements_aspect > >
	advancements__lua_aspect_factory("advancements*lua_aspect");

static register_lua_aspect_factory< lua_aspect<double> >
	aggression__lua_aspect_factory("aggression*lua_aspect");

static register_lua_aspect_factory< lua_aspect<int> >
	attack_depth__lua_aspect_factory("attack_depth*lua_aspect");

static register_lua_aspect_factory< lua_aspect<terrain_filter> >
	avoid__lua_aspect_factory("avoid*lua_aspect");

static register_lua_aspect_factory< lua_aspect<double> >
	caution__lua_aspect_factory("caution*lua_aspect");

static register_lua_aspect_factory< lua_aspect<std::string> >
	grouping__lua_aspect_factory("grouping*lua_aspect");

static register_lua_aspect_factory< lua_aspect<double> >
	leader_aggression__lua_aspect_factory("leader_aggression*lua_aspect");

static register_lua_aspect_factory< lua_aspect<config> >
	leader_goal__lua_aspect_factory("leader_goal*lua_aspect");

static register_lua_aspect_factory< lua_aspect<bool> >
	leader_ignores_keep__lua_aspect_factory("leader_ignores_keep*lua_aspect");

static register_lua_aspect_factory< lua_aspect<double> >
	leader_value__lua_aspect_factory("leader_value*lua_aspect");

static register_lua_aspect_factory< lua_aspect<double> >
	number_of_possible_recruits_to_force_recruit__lua_aspect_factory("number_of_possible_recruits_to_force_recruit*lua_aspect");

static register_lua_aspect_factory< lua_aspect<bool> >
	passive_leader__lua_aspect_factory("passive_leader*lua_aspect");

static register_lua_aspect_factory< lua_aspect<bool> >
	passive_leader_shares_keep__lua_aspect_factory("passive_leader_shares_keep*lua_aspect");

static register_lua_aspect_factory< lua_aspect<bool> >
	recruitment_ignore_bad_combat__lua_aspect_factory("recruitment_ignore_bad_combat*lua_aspect");

static register_lua_aspect_factory< lua_aspect<bool> >
	recruitment_ignore_bad_movement__lua_aspect_factory("recruitment_ignore_bad_movement*lua_aspect");

static register_lua_aspect_factory< lua_aspect<double> >
	scout_village_targeting__lua_aspect_factory("scout_village_targeting*lua_aspect");

static register_lua_aspect_factory< lua_aspect<bool> >
	simple_targeting__lua_aspect_factory("simple_targeting*lua_aspect");

static register_lua_aspect_factory< lua_aspect<bool> >
	support_villages__lua_aspect_factory("support_villages*lua_aspect");

static register_lua_aspect_factory< lua_aspect<double> >
	village_value__lua_aspect_factory("village_value*lua_aspect");

static register_lua_aspect_factory< lua_aspect<int> >
	villages_per_scout__lua_aspect_factory("villages_per_scout*lua_aspect");

static register_lua_aspect_factory< lua_aspect< std::vector<std::string> > >
	recruitment_pattern__lua_aspect_factory("recruitment_pattern*lua_aspect");

void registry::init()
{
}

} //end of namespace ai
