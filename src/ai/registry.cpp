/*
   Copyright (C) 2009 - 2018 by Yurii Chernyi <terraninfo@terraninfo.net>
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

#include "ai/registry.hpp"

#include "config.hpp"             // for config, operator<<
#include "terrain/filter.hpp"  // for terrain_filter
#include "ai/composite/engine.hpp"      // for register_engine_factory
#include "ai/composite/stage.hpp"       // for ministage, idle_stage, etc
#include "ai/composite/rca.hpp"
#include "ai/game_info.hpp"             // for attacks_vector
#include "ai/composite/ai.hpp"             // for ai_composite
#include "ai/composite/aspect.hpp"         // for composite_aspect, etc
#include "ai/default/engine_cpp.hpp"  // for engine_cpp
#include "ai/formula/engine_fai.hpp"     // for engine_fai
#include "ai/lua/engine_lua.hpp"     // for engine_lua
#include "ai/composite/goal.hpp"           // for register_goal_factory, etc
#include "ai/lua/aspect_advancements.hpp"
#include "ai/default/recruitment.hpp"  // for recruitment
#include "ai/default/aspect_attacks.hpp"   // for aspect_attacks
#include "ai/default/ca.hpp"               // for leader_shares_keep_phase, etc
#include "ai/default/ca_move_to_targets.hpp"
#include "ai/default/stage_rca.hpp"

#include <string>                       // for string
#include <vector>                       // for vector


namespace ai {
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

static register_stage_factory<ai_default_rca::candidate_action_evaluation_loop>
	candidate_action_evaluation_loop_factory("ai_default_rca::candidate_action_evaluation_loop");

static register_stage_factory<idle_stage>
	ai_idle_stage_factory("empty");

// === Also keep the old syntax ===
static register_stage_factory<ai_default_rca::candidate_action_evaluation_loop>
	old_candidate_action_evaluation_loop_factory("testing_ai_default::candidate_action_evaluation_loop");

// =======================================================================
// Candidate actions
// =======================================================================

static register_candidate_action_factory<ai_default_rca::goto_phase>
	goto_phase_factory("ai_default_rca::goto_phase");

static register_candidate_action_factory<ai_default_rca::combat_phase>
	combat_phase_factory("ai_default_rca::combat_phase");

static register_candidate_action_factory<ai_default_rca::move_leader_to_goals_phase>
	move_leader_to_goals_phase_factory("ai_default_rca::move_leader_to_goals_phase");

static register_candidate_action_factory<ai_default_rca::move_leader_to_keep_phase>
	move_leader_to_keep_phase_factory("ai_default_rca::move_leader_to_keep_phase");

static register_candidate_action_factory<ai_default_rca::get_villages_phase>
	get_villages_phase_factory("ai_default_rca::get_villages_phase");

static register_candidate_action_factory<ai_default_rca::get_healing_phase>
	get_healing_phase_factory("ai_default_rca::get_healing_phase");

static register_candidate_action_factory<ai_default_rca::retreat_phase>
	retreat_phase_factory("ai_default_rca::retreat_phase");

static register_candidate_action_factory<ai_default_rca::move_to_targets_phase>
	default_move_to_targets_phase_factory("ai_default_rca::move_to_targets_phase");

static register_candidate_action_factory<ai_default_rca::leader_control_phase>
	leader_control_phase_factory("ai_default_rca::leader_control_phase");

static register_candidate_action_factory<ai_default_rca::leader_shares_keep_phase>
	leader_shares_keep_phase_factory("ai_default_rca::leader_shares_keep_phase");

//Also keep passive_leader_shares_keep_phase for backward compatibility
static register_candidate_action_factory<ai_default_rca::leader_shares_keep_phase>
	passive_leader_shares_keep_phase_factory("ai_default_rca::passive_leader_shares_keep_phase");

static register_candidate_action_factory<default_recruitment::recruitment>
	default_recruitment_factory("default_recruitment::recruitment");

// === Also keep the old syntax ===
static register_candidate_action_factory<ai_default_rca::goto_phase>
	old_goto_phase_factory("testing_ai_default::goto_phase");

static register_candidate_action_factory<ai_default_rca::combat_phase>
	old_combat_phase_factory("testing_ai_default::combat_phase");

static register_candidate_action_factory<ai_default_rca::move_leader_to_goals_phase>
	old_move_leader_to_goals_phase_factory("testing_ai_default::move_leader_to_goals_phase");

static register_candidate_action_factory<ai_default_rca::move_leader_to_keep_phase>
	old_move_leader_to_keep_phase_factory("testing_ai_default::move_leader_to_keep_phase");

static register_candidate_action_factory<ai_default_rca::get_villages_phase>
	old_get_villages_phase_factory("testing_ai_default::get_villages_phase");

static register_candidate_action_factory<ai_default_rca::get_healing_phase>
	old_get_healing_phase_factory("testing_ai_default::get_healing_phase");

static register_candidate_action_factory<ai_default_rca::retreat_phase>
	old_retreat_phase_factory("testing_ai_default::retreat_phase");

static register_candidate_action_factory<ai_default_rca::move_to_targets_phase>
	old_default_move_to_targets_phase_factory("testing_ai_default::default_move_to_targets_phase");

static register_candidate_action_factory<ai_default_rca::move_to_targets_phase>
	old_testing_move_to_targets_phase_factory("testing_ai_default::testing_move_to_targets_phase");

static register_candidate_action_factory<ai_default_rca::leader_control_phase>
	old_leader_control_phase_factory("testing_ai_default::leader_control_phase");

static register_candidate_action_factory<ai_default_rca::leader_shares_keep_phase>
	old_passive_leader_shares_keep_phase_factory("testing_ai_default::passive_leader_shares_keep_phase");

// =======================================================================
// Goals
// =======================================================================

static register_goal_factory<target_unit_goal>
	goal_factory("");


static register_goal_factory<target_unit_goal>
	goal_factory_target("target");


static register_goal_factory<target_unit_goal>
	goal_factory_target_unit("target_unit");


static register_goal_factory<target_location_goal>
	goal_factory_target_location("target_location");


static register_goal_factory<protect_location_goal>
	goal_factory_protect_location("protect_location");


static register_goal_factory<protect_unit_goal>
	goal_factory_protect_unit("protect_unit");

static register_goal_factory<lua_goal>
	goal_factory_lua_goal("lua_goal");


// =======================================================================
// Aspects
// =======================================================================

//name=composite_aspect

static register_aspect_factory< composite_aspect< unit_advancements_aspect >>
	advancements__composite_aspect_factory("advancements*composite_aspect");

static register_aspect_factory< composite_aspect<double>>
	aggression__composite_aspect_factory("aggression*composite_aspect");

static register_aspect_factory< composite_aspect<int>>
	attack_depth__composite_aspect_factory("attack_depth*composite_aspect");

static register_aspect_factory< composite_aspect< attacks_vector >>
	attacks__composite_aspect_factory("attacks*composite_aspect");

static register_aspect_factory< composite_aspect< terrain_filter >>
	avoid__composite_aspect_factory("avoid*composite_aspect");

static register_aspect_factory< composite_aspect<double>>
	caution__composite_aspect_factory("caution*composite_aspect");

static register_aspect_factory< composite_aspect<std::string>>
	grouping__composite_aspect_factory("grouping*composite_aspect");

static register_aspect_factory< composite_aspect<double>>
	leader_aggression__composite_aspect_factory("leader_aggression*composite_aspect");

static register_aspect_factory< composite_aspect<config>>
	leader_goal__composite_aspect_factory("leader_goal*composite_aspect");

static register_aspect_factory< composite_aspect<bool>>
	leader_igores_keep__composite_aspect_factory("leader_ignores_keep*composite_aspect");

static register_aspect_factory< composite_aspect<double>>
	leader_value__composite_aspect_factory("leader_value*composite_aspect");

static register_aspect_factory< composite_aspect<bool>>
	passive_leader__composite_aspect_factory("passive_leader*composite_aspect");

static register_aspect_factory< composite_aspect<bool>>
	passive_leader_shares_keep__composite_aspect_factory("passive_leader_shares_keep*composite_aspect");

static register_aspect_factory< composite_aspect<double>>
	recruitment_diversity__composite_aspect_factory("recruitment_diversity*composite_aspect");

static register_aspect_factory< composite_aspect<config>>
	recruitment_instructions__composite_aspect_factory("recruitment_instructions*composite_aspect");

static register_aspect_factory< composite_aspect< std::vector<std::string>> >
	recruitment_more__composite_aspect_factory("recruitment_more*composite_aspect");

static register_aspect_factory< composite_aspect< std::vector<std::string>> >
	recruitment_pattern__composite_aspect_factory("recruitment_pattern*composite_aspect");

static register_aspect_factory< composite_aspect<int>>
	recruitment_randomness__composite_aspect_factory("recruitment_randomness*composite_aspect");

static register_aspect_factory< composite_aspect<config>>
	recruitment_save_gold__composite_aspect_factory("recruitment_save_gold*composite_aspect");

static register_aspect_factory< composite_aspect<double>>
	scout_village_targeting__composite_aspect_factory("scout_village_targeting*composite_aspect");

static register_aspect_factory< composite_aspect<bool>>
	simple_targeting__composite_aspect_factory("simple_targeting*composite_aspect");

static register_aspect_factory< composite_aspect<bool>>
	support_villages__composite_aspect_factory("support_villages*composite_aspect");

static register_aspect_factory< composite_aspect<double>>
	village_value__composite_aspect_factory("village_value*composite_aspect");

static register_aspect_factory< composite_aspect<int>>
	villages_per_scout__composite_aspect_factory("villages_per_scout*composite_aspect");


//name=standard_aspect
static register_aspect_factory< standard_aspect< unit_advancements_aspect >>
	advancements__standard_aspect_factory("advancements*standard_aspect");

static register_aspect_factory< standard_aspect<double>>
	aggression__standard_aspect_factory("aggression*standard_aspect");

static register_aspect_factory< standard_aspect<int>>
	attack_depth__standard_aspect_factory("attack_depth*standard_aspect");

static register_aspect_factory< ai_default_rca::aspect_attacks >
	attacks__testing_ai_default_aspect_attacks_factory("attacks*ai_default_rca::aspect_attacks");

static register_aspect_factory< standard_aspect< terrain_filter >>
	avoid__standard_aspect_factory("avoid*standard_aspect");

static register_aspect_factory< standard_aspect<double>>
	caution__standard_aspect_factory("caution*standard_aspect");

static register_aspect_factory< standard_aspect<std::string>>
	grouping__standard_aspect_factory("grouping*standard_aspect");

static register_aspect_factory< standard_aspect<double>>
	leader_aggression__standard_aspect_factory("leader_aggression*standard_aspect");

static register_aspect_factory< standard_aspect<config>>
	leader_goal__standard_aspect_factory("leader_goal*standard_aspect");

static register_aspect_factory< standard_aspect<bool>>
	leader_ignores_keep__standard_aspect_factory("leader_ignores_keep*standard_aspect");

static register_aspect_factory< standard_aspect<double>>
	leader_value__standard_aspect_factory("leader_value*standard_aspect");

static register_aspect_factory< standard_aspect<bool>>
	passive_leader__standard_aspect_factory("passive_leader*standard_aspect");

static register_aspect_factory< standard_aspect<bool>>
	passive_leader_shares_keep__standard_aspect_factory("passive_leader_shares_keep*standard_aspect");

static register_aspect_factory< standard_aspect<double>>
	recruitment_diversity__standard_aspect_factory("recruitment_diversity*standard_aspect");

static register_aspect_factory< default_recruitment::recruitment_aspect >
	recruitment_instructions__standard_aspect_factory("recruitment_instructions*standard_aspect");

static register_aspect_factory< standard_aspect< std::vector<std::string>> >
	recruitment_more__standard_aspect_factory("recruitment_more*standard_aspect");

static register_aspect_factory< standard_aspect< std::vector<std::string>> >
	recruitment_pattern__standard_aspect_factory("recruitment_pattern*standard_aspect");

static register_aspect_factory< standard_aspect<int>>
	recruitment_randomness__standard_aspect_factory("recruitment_randomness*standard_aspect");

static register_aspect_factory< standard_aspect<config>>
	recruitment_save_gold__standard_aspect_factory("recruitment_save_gold*standard_aspect");

static register_aspect_factory< standard_aspect<double>>
	scout_village_targeting__standard_aspect_factory("scout_village_targeting*standard_aspect");

static register_aspect_factory< standard_aspect<bool>>
	simple_targeting__standard_aspect_factory("simple_targeting*standard_aspect");

static register_aspect_factory< standard_aspect<bool>>
	support_villages__standard_aspect_factory("support_villages*standard_aspect");

static register_aspect_factory< standard_aspect<double>>
	village_value__standard_aspect_factory("village_value*standard_aspect");

static register_aspect_factory< standard_aspect<int>>
	villages_per_scout__standard_aspect_factory("villages_per_scout*standard_aspect");


// Also keep the old syntax
static register_aspect_factory< ai_default_rca::aspect_attacks >
	old_attacks__testing_ai_default_aspect_attacks_factory("attacks*testing_ai_default::aspect_attacks");

//name = default
static register_aspect_factory< standard_aspect< unit_advancements_aspect >>
	advancements__standard_aspect_factory2("advancements*");

static register_aspect_factory< standard_aspect<double>>
	aggression__standard_aspect_factory2("aggression*");

static register_aspect_factory< standard_aspect<int>>
	attack_depth__standard_aspect_factory2("attack_depth*");

static register_aspect_factory< ai_default_rca::aspect_attacks >
	attacks__testing_ai_default_aspect_attacks_factory2("attacks*");

static register_aspect_factory< standard_aspect< terrain_filter >>
	avoid__standard_aspect_factory2("avoid*");

static register_aspect_factory< standard_aspect<double>>
	caution__standard_aspect_factory2("caution*");

static register_aspect_factory< standard_aspect<std::string>>
	grouping__standard_aspect_factory2("grouping*");

static register_aspect_factory< standard_aspect<double>>
	leader_aggression__standard_aspect_factory2("leader_aggression*");

static register_aspect_factory< standard_aspect<config>>
	leader_goal__standard_aspect_factory2("leader_goal*");

static register_aspect_factory< standard_aspect<bool>>
	leader_ignores_keep__standard_aspect_factory2("leader_ignores_keep*");

static register_aspect_factory< standard_aspect<double>>
	leader_value__standard_aspect_factory2("leader_value*");

static register_aspect_factory< standard_aspect<bool>>
	passive_leader__standard_aspect_factory2("passive_leader*");

static register_aspect_factory< standard_aspect<bool>>
	passive_leader_shares_keep__standard_aspect_factory2("passive_leader_shares_keep*");

static register_aspect_factory< standard_aspect<double>>
	recruitment_diversity__standard_aspect_factory2("recruitment_diversity*");

static register_aspect_factory< default_recruitment::recruitment_aspect >
	recruitment_instructions__standard_aspect_factory2("recruitment_instructions*");

static register_aspect_factory< standard_aspect< std::vector<std::string>> >
	recruitment_more__standard_aspect_factory2("recruitment_more*");

static register_aspect_factory< standard_aspect< std::vector<std::string>> >
	recruitment_pattern__standard_aspect_factory2("recruitment_pattern*");

static register_aspect_factory< standard_aspect<int>>
	recruitment_randomness__standard_aspect_factory2("recruitment_randomness*");

static register_aspect_factory< standard_aspect<config>>
	recruitment_save_gold__standard_aspect_factory2("recruitment_save_gold*");

static register_aspect_factory< standard_aspect<double>>
	scout_village_targeting__standard_aspect_factory2("scout_village_targeting*");

static register_aspect_factory< standard_aspect<bool>>
	simple_targeting__standard_aspect_factory2("simple_targeting*");

static register_aspect_factory< standard_aspect<bool>>
	support_villages__standard_aspect_factory2("support_villages*");

static register_aspect_factory< standard_aspect<double>>
	village_value__standard_aspect_factory2("village_value*");

static register_aspect_factory< standard_aspect<int>>
	villages_per_scout__standard_aspect_factory2("villages_per_scout*");


//name = lua
static register_lua_aspect_factory< lua_aspect< unit_advancements_aspect >>
	advancements__lua_aspect_factory("advancements*lua_aspect");

static register_lua_aspect_factory< lua_aspect<double>>
	aggression__lua_aspect_factory("aggression*lua_aspect");

static register_lua_aspect_factory< lua_aspect<int>>
	attack_depth__lua_aspect_factory("attack_depth*lua_aspect");

static register_lua_aspect_factory< aspect_attacks_lua >
	attacks__lua_aspect_factory("attacks*lua_aspect");

static register_lua_aspect_factory< lua_aspect<terrain_filter>>
	avoid__lua_aspect_factory("avoid*lua_aspect");

static register_lua_aspect_factory< lua_aspect<double>>
	caution__lua_aspect_factory("caution*lua_aspect");

static register_lua_aspect_factory< lua_aspect<std::string>>
	grouping__lua_aspect_factory("grouping*lua_aspect");

static register_lua_aspect_factory< lua_aspect<double>>
	leader_aggression__lua_aspect_factory("leader_aggression*lua_aspect");

static register_lua_aspect_factory< lua_aspect<config>>
	leader_goal__lua_aspect_factory("leader_goal*lua_aspect");

static register_lua_aspect_factory< lua_aspect<bool>>
	leader_ignores_keep__lua_aspect_factory("leader_ignores_keep*lua_aspect");

static register_lua_aspect_factory< lua_aspect<double>>
	leader_value__lua_aspect_factory("leader_value*lua_aspect");

static register_lua_aspect_factory< lua_aspect<bool>>
	passive_leader__lua_aspect_factory("passive_leader*lua_aspect");

static register_lua_aspect_factory< lua_aspect<bool>>
	passive_leader_shares_keep__lua_aspect_factory("passive_leader_shares_keep*lua_aspect");

static register_lua_aspect_factory< lua_aspect<double>>
	scout_village_targeting__lua_aspect_factory("scout_village_targeting*lua_aspect");

static register_lua_aspect_factory< lua_aspect<bool>>
	simple_targeting__lua_aspect_factory("simple_targeting*lua_aspect");

static register_lua_aspect_factory< lua_aspect<bool>>
	support_villages__lua_aspect_factory("support_villages*lua_aspect");

static register_lua_aspect_factory< lua_aspect<double>>
	village_value__lua_aspect_factory("village_value*lua_aspect");

static register_lua_aspect_factory< lua_aspect<int>>
	villages_per_scout__lua_aspect_factory("villages_per_scout*lua_aspect");

static register_lua_aspect_factory< lua_aspect< std::vector<std::string>> >
	recruitment_pattern__lua_aspect_factory("recruitment_pattern*lua_aspect");


// Some compatibility - recruitment is a removed aspect, but its syntax
// is compatible with recruitment_instructions
static register_aspect_factory< composite_aspect<config>>
	recruitments__composite_aspect_factory("recruitment*composite_aspect");

static register_aspect_factory< default_recruitment::recruitment_aspect >
	recruitment__standard_aspect_factory("recruitment*standard_aspect");

static register_aspect_factory< default_recruitment::recruitment_aspect >
	recruitment__standard_aspect_factory2("recruitment*");

void registry::init()
{
}

} //end of namespace ai
