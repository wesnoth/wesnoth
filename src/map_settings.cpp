/*
	Copyright (C) 2007 - 2025
	by Mark de Wever <koraq@xs4all.nl>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

/**
 *  @file
 *  General settings and defaults for scenarios.
 */

#include "lexical_cast.hpp"
#include "map_settings.hpp"

#include "formula/string_utils.hpp"
#include "preferences/preferences.hpp"
#include "saved_game.hpp"
#include "serialization/string_utils.hpp"

namespace settings {

int get_turns(const std::string& value)
{
	// Special case, -1 is also allowed, which means unlimited turns
	int val = lexical_cast_default<int>(value);

	if(val == -1) {
		return turns_max;
	}

	return std::clamp<int>(lexical_cast_default<int>(value, turns_default), turns_min, turns_max);
}

int get_village_gold(const std::string& value, const game_classification* classification)
{
	return lexical_cast_default<int>(value, ((classification && !classification->is_normal_mp_game()) ? 1 : 2));
}

int get_village_support(const std::string& value)
{
	return lexical_cast_default<int>(value, 1);
}

int get_xp_modifier(const std::string& value)
{
	return lexical_cast_default<int>(value, 70);
}

void set_default_values(ng::create_engine& create)
{
	mp_game_settings& params = create.get_state().mp_settings();

	params.use_map_settings = use_map_settings_default(create);
	params.name = game_name_default();
	params.num_turns = num_turns_default(create);
	params.village_gold = village_gold_default(create);
	params.village_support = village_support_default(create);
	params.xp_modifier = xp_modifier_default(create);
	params.mp_countdown_init_time = mp_countdown_init_time_default();
	params.mp_countdown_reservoir_time = mp_countdown_reservoir_time_default();
	params.mp_countdown_action_bonus = mp_countdown_action_bonus_default();
	params.mp_countdown = mp_countdown_default();
	params.random_start_time = random_start_time_default(create);
	params.fog_game = fog_game_default(create);
	params.shroud_game = shroud_game_default(create);
	params.mode = random_faction_mode_default();
}

bool force_lock_settings(ng::create_engine& create)
{
	return create.current_level().data()["force_lock_settings"].to_bool(!create.get_state().classification().is_normal_mp_game());
}

std::string game_name_default()
{
	utils::string_map i18n_symbols;
	i18n_symbols["login"] = prefs::get().login();
	return VGETTEXT("$login|’s game", i18n_symbols);
}

int num_turns_default(ng::create_engine& create)
{
	return create.get_state().mp_settings().use_map_settings ? settings::get_turns(create.current_level().data()["turns"]) : prefs::get().mp_turns();
}

int village_gold_default(ng::create_engine& create)
{
	return create.get_state().mp_settings().use_map_settings
		? settings::get_village_gold(create.current_level().data()["mp_village_gold"], &create.get_state().classification())
		: prefs::get().village_gold();
}

int village_support_default(ng::create_engine& create)
{
	return create.get_state().mp_settings().use_map_settings
		? settings::get_village_support(create.current_level().data()["mp_village_support"])
		: prefs::get().village_support();
}

int xp_modifier_default(ng::create_engine& create)
{
	return create.get_state().mp_settings().use_map_settings
		? settings::get_xp_modifier(create.current_level().data()["experience_modifier"])
		: prefs::get().xp_modifier();
}

std::chrono::seconds mp_countdown_init_time_default()
{
	return prefs::get().countdown_init_time();
}

std::chrono::seconds mp_countdown_reservoir_time_default()
{
	return prefs::get().countdown_reservoir_time();
}

std::chrono::seconds mp_countdown_turn_bonus_default()
{
	return prefs::get().countdown_turn_bonus();
}

std::chrono::seconds mp_countdown_action_bonus_default()
{
	return prefs::get().countdown_action_bonus();
}

bool mp_countdown_default()
{
	return prefs::get().mp_countdown();
}

bool use_map_settings_default(ng::create_engine& create)
{
	return force_lock_settings(create) || prefs::get().mp_use_map_settings();
}

bool random_start_time_default(ng::create_engine& create)
{
	return create.get_state().mp_settings().use_map_settings
		? create.current_level().data()["random_start_time"].to_bool(false)
		: prefs::get().mp_random_start_time();
}

bool fog_game_default(ng::create_engine& create)
{
	return create.get_state().mp_settings().use_map_settings
		? create.current_level().data()["mp_fog"].to_bool(create.get_state().classification().is_normal_mp_game())
		: prefs::get().mp_fog();
}

bool shroud_game_default(ng::create_engine& create)
{
	return create.get_state().mp_settings().use_map_settings
		? create.current_level().data()["mp_shroud"].to_bool(false)
		: prefs::get().mp_shroud();
}

bool allow_observers_default()
{
	return prefs::get().allow_observers();
}

bool shuffle_sides_default()
{
	return prefs::get().shuffle_sides();
}

random_faction_mode::type random_faction_mode_default()
{
	return random_faction_mode::get_enum(prefs::get().random_faction_mode()).value_or(random_faction_mode::type::independent);
}
} // end namespace settings
