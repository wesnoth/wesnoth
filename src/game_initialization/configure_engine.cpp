/*
   Copyright (C) 2013 - 2017 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "game_initialization/configure_engine.hpp"
#include "formula/string_utils.hpp"
#include "game_config_manager.hpp"
#include "mp_game_settings.hpp"
#include "settings.hpp"
#include "tod_manager.hpp"
#include "preferences/credentials.hpp"

#include <cassert>
#include <sstream>
#include <iostream>

namespace ng {

configure_engine::configure_engine(saved_game& state, const config* intial)
	: state_(state)
	, parameters_(state_.mp_settings())
	, initial_(intial ? intial : &state_.get_starting_pos())
{
	set_use_map_settings(use_map_settings_default());
}

void configure_engine::set_default_values() {
	set_use_map_settings(use_map_settings_default());
	set_game_name(game_name_default());
	set_num_turns(num_turns_default());
	set_village_gold(village_gold_default());
	set_village_support(village_support_default());
	set_xp_modifier(xp_modifier_default());
	set_mp_countdown_init_time(mp_countdown_init_time_default());
	set_mp_countdown_reservoir_time(mp_countdown_reservoir_time_default());
	set_mp_countdown_action_bonus(mp_countdown_action_bonus_default());
	set_mp_countdown(mp_countdown_default());
	set_random_start_time(random_start_time_default());
	set_fog_game(fog_game_default());
	set_shroud_game(shroud_game_default());
	set_random_faction_mode(random_faction_mode_default());
}

bool configure_engine::force_lock_settings() const {
	return initial_cfg()["force_lock_settings"].to_bool(!state_.classification().is_normal_mp_game());
}

std::string configure_engine::game_name() const { return parameters_.name; }
int configure_engine::num_turns() const { return parameters_.num_turns; }
int configure_engine::village_gold() const { return parameters_.village_gold; }
int configure_engine::village_support() const { return parameters_.village_support; }
int configure_engine::xp_modifier() const { return parameters_.xp_modifier; }
int configure_engine::mp_countdown_init_time() const { return parameters_.mp_countdown_init_time; }
int configure_engine::mp_countdown_reservoir_time() const { return parameters_.mp_countdown_reservoir_time; }
int configure_engine::mp_countdown_turn_bonus() const { return parameters_.mp_countdown_turn_bonus; }
int configure_engine::mp_countdown_action_bonus() const { return parameters_.mp_countdown_action_bonus; }
bool configure_engine::mp_countdown() const { return parameters_.mp_countdown; }
bool configure_engine::use_map_settings() const { return parameters_.use_map_settings; }
bool configure_engine::random_start_time() const { return parameters_.random_start_time; }
bool configure_engine::fog_game() const { return parameters_.fog_game; }
bool configure_engine::shroud_game() const { return parameters_.shroud_game; }
bool configure_engine::allow_observers() const { return parameters_.allow_observers; }
bool configure_engine::registered_users_only() const { return parameters_.registered_users_only; }
bool configure_engine::shuffle_sides() const { return parameters_.shuffle_sides; }
mp_game_settings::RANDOM_FACTION_MODE configure_engine::random_faction_mode() const { return parameters_.random_faction_mode; }
const config& configure_engine::options() const { return parameters_.options; }

void configure_engine::set_game_name(std::string val) { parameters_.name = val; }
void configure_engine::set_game_password(std::string val) { parameters_.password = val; }
void configure_engine::set_num_turns(int val) { parameters_.num_turns = val; }
void configure_engine::set_village_gold(int val) { parameters_.village_gold = val; }
void configure_engine::set_village_support(int val) { parameters_.village_support = val; }
void configure_engine::set_xp_modifier(int val) { parameters_.xp_modifier = val; }
void configure_engine::set_mp_countdown_init_time(int val) { parameters_.mp_countdown_init_time = val; }
void configure_engine::set_mp_countdown_reservoir_time(int val) { parameters_.mp_countdown_reservoir_time = val; }
void configure_engine::set_mp_countdown_turn_bonus(int val) { parameters_.mp_countdown_turn_bonus = val; }
void configure_engine::set_mp_countdown_action_bonus(int val) { parameters_.mp_countdown_action_bonus = val; }
void configure_engine::set_mp_countdown(bool val) { parameters_.mp_countdown = val; }
void configure_engine::set_use_map_settings(bool val) { parameters_.use_map_settings = val; }
void configure_engine::set_random_start_time(bool val) { parameters_.random_start_time = val; }
void configure_engine::set_fog_game(bool val) { parameters_.fog_game = val; }
void configure_engine::set_shroud_game(bool val) { parameters_.shroud_game = val; }
void configure_engine::set_allow_observers(bool val) { parameters_.allow_observers = val; }
void configure_engine::set_registered_users_only(bool val) { parameters_.registered_users_only = val; }
void configure_engine::set_oos_debug(bool val) { state_.classification().oos_debug = val; }
void configure_engine::set_shuffle_sides(bool val) { parameters_.shuffle_sides = val; }
void configure_engine::set_random_faction_mode(mp_game_settings::RANDOM_FACTION_MODE val) { parameters_.random_faction_mode = val;}
void configure_engine::set_options(const config& cfg) { parameters_.options = cfg; }

std::string configure_engine::game_name_default() {
	utils::string_map i18n_symbols;
	i18n_symbols["login"] = preferences::login();
	return vgettext("$login|’s game", i18n_symbols);
}
int configure_engine::num_turns_default() const {
	return use_map_settings() ?
		settings::get_turns(initial_cfg()["turns"]) :
		preferences::turns();
}
int configure_engine::village_gold_default() const {
	return use_map_settings() ?
		settings::get_village_gold(initial_cfg()["mp_village_gold"], &state_.classification()) :
		preferences::village_gold();
}
int configure_engine::village_support_default() const {
	return use_map_settings() ?
		settings::get_village_support(initial_cfg()["mp_village_support"]) :
		preferences::village_support();
}
int configure_engine::xp_modifier_default() const {
	return use_map_settings() ?
		settings::get_xp_modifier(initial_cfg()["experience_modifier"]) :
		preferences::xp_modifier();
}
int configure_engine::mp_countdown_init_time_default() const {
	return preferences::countdown_init_time();
}
int configure_engine::mp_countdown_reservoir_time_default() const {
	return preferences::countdown_reservoir_time();
}
int configure_engine::mp_countdown_turn_bonus_default() const {
	return preferences::countdown_turn_bonus();
}
int configure_engine::mp_countdown_action_bonus_default() const {
	return preferences::countdown_action_bonus();
}
bool configure_engine::mp_countdown_default() const {
	return preferences::countdown();
}
bool configure_engine::use_map_settings_default() const {
	return force_lock_settings() || preferences::use_map_settings();
}
bool configure_engine::random_start_time_default() const {
	return use_map_settings() ?
		initial_cfg()["random_start_time"].to_bool(false) :
		preferences::random_start_time();
}
bool configure_engine::fog_game_default() const {
	return use_map_settings() ?
		initial_cfg()["mp_fog"].to_bool(state_.classification().is_normal_mp_game()) :
		preferences::fog();
}
bool configure_engine::shroud_game_default() const {
	return use_map_settings() ?
		initial_cfg()["mp_shroud"].to_bool(false) :
		preferences::shroud();
}
bool configure_engine::allow_observers_default() const {
	return preferences::allow_observers();
}
bool configure_engine::registered_users_only_default() const
{
	return preferences::registered_users_only();
}
bool configure_engine::shuffle_sides_default() const {
	return preferences::shuffle_sides();
}
mp_game_settings::RANDOM_FACTION_MODE configure_engine::random_faction_mode_default() const {
	return mp_game_settings::RANDOM_FACTION_MODE::string_to_enum(preferences::random_faction_mode(), mp_game_settings::RANDOM_FACTION_MODE::DEFAULT);
}

const config& configure_engine::options_default() const {
	return preferences::options();
}

const mp_game_settings& configure_engine::get_parameters() const {
	return parameters_;
}

void configure_engine::write_parameters()
{
	config& scenario = this->state_.get_starting_pos();
	const mp_game_settings& params = this->state_.mp_settings();

	if (params.random_start_time) {
		if (!tod_manager::is_start_ToD(scenario["random_start_time"])) {
			scenario["random_start_time"] = true;
		}
	}
	else {
		scenario["random_start_time"] = false;
	}
	scenario["experience_modifier"] = params.xp_modifier;
	scenario["turns"] = params.num_turns;

	for (config& side : scenario.child_range("side"))
	{
		if (!params.use_map_settings) {
			side["fog"] = params.fog_game;
			side["shroud"] = params.shroud_game;
			side["village_gold"] = params.village_gold;
			side["village_support"] = params.village_support;
		} 
		else {
			if (side["fog"].empty()) {
				side["fog"] = params.fog_game;
			}
			if (side["shroud"].empty()) {
				side["shroud"] = params.shroud_game;
			}
			if (side["village_gold"].empty()) {
				side["village_gold"] = params.village_gold;
			}
			if (side["village_support"].empty()) {
				side["village_support"] = params.village_support;
			}
		}
	}
}

} //end namespace ng
