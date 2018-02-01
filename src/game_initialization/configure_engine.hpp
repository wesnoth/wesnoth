/*
   Copyright (C) 2014 - 2018 by Nathan Walker <nathan.b.walker@vanderbilt.edu>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "gettext.hpp"
#include "mp_game_settings.hpp"
#include "saved_game.hpp"

namespace ng
{
/**
 * configure_engine
 *
 * this class wraps the parameters relevent to mp_configure,
 * as well as providing defaults for these parameters.
 */
class configure_engine
{
public:
	configure_engine(saved_game& state, const config* initial = nullptr);

	/** Set all parameters to their default values. */
	void set_default_values();

	/** Checks force_lock_settings in config. */
	bool force_lock_settings() const;

	//
	// Getter methods
	//

	std::string game_name() const
	{
		return parameters_.name;
	}

	int num_turns() const
	{
		return parameters_.num_turns;
	}

	int village_gold() const
	{
		return parameters_.village_gold;
	}

	int village_support() const
	{
		return parameters_.village_support;
	}

	int xp_modifier() const
	{
		return parameters_.xp_modifier;
	}

	int mp_countdown_init_time() const
	{
		return parameters_.mp_countdown_init_time;
	}

	int mp_countdown_reservoir_time() const
	{
		return parameters_.mp_countdown_reservoir_time;
	}

	int mp_countdown_turn_bonus() const
	{
		return parameters_.mp_countdown_turn_bonus;
	}

	int mp_countdown_action_bonus() const
	{
		return parameters_.mp_countdown_action_bonus;
	}

	bool mp_countdown() const
	{
		return parameters_.mp_countdown;
	}

	bool use_map_settings() const
	{
		return parameters_.use_map_settings;
	}

	bool random_start_time() const
	{
		return parameters_.random_start_time;
	}

	bool fog_game() const
	{
		return parameters_.fog_game;
	}

	bool shroud_game() const
	{
		return parameters_.shroud_game;
	}

	bool allow_observers() const
	{
		return parameters_.allow_observers;
	}

	bool registered_users_only() const
	{
		return parameters_.registered_users_only;
	}

	bool shuffle_sides() const
	{
		return parameters_.shuffle_sides;
	}

	mp_game_settings::RANDOM_FACTION_MODE random_faction_mode() const
	{
		return parameters_.random_faction_mode;
	}

	const config& options() const
	{
		return parameters_.options;
	}

	//
	// Setter methods
	//

	void set_game_name(std::string name)
	{
		parameters_.name = name;
	}

	void set_game_password(std::string name)
	{
		parameters_.password = name;
	}

	void set_num_turns(int val)
	{
		parameters_.num_turns = val;
	}

	void set_village_gold(int val)
	{
		parameters_.village_gold = val;
	}

	void set_village_support(int val)
	{
		parameters_.village_support = val;
	}

	void set_xp_modifier(int val)
	{
		parameters_.xp_modifier = val;
	}

	void set_mp_countdown_init_time(int val)
	{
		parameters_.mp_countdown_init_time = val;
	}

	void set_mp_countdown_reservoir_time(int val)
	{
		parameters_.mp_countdown_reservoir_time = val;
	}

	void set_mp_countdown_turn_bonus(int val)
	{
		parameters_.mp_countdown_turn_bonus = val;
	}

	void set_mp_countdown_action_bonus(int val)
	{
		parameters_.mp_countdown_action_bonus = val;
	}

	void set_mp_countdown(bool val)
	{
		parameters_.mp_countdown = val;
	}

	void set_use_map_settings(bool val)
	{
		parameters_.use_map_settings = val;
	}

	void set_random_start_time(bool val)
	{
		parameters_.random_start_time = val;
	}

	void set_fog_game(bool val)
	{
		parameters_.fog_game = val;
	}

	void set_shroud_game(bool val)
	{
		parameters_.shroud_game = val;
	}

	void set_allow_observers(bool val)
	{
		parameters_.allow_observers = val;
	}

	void set_registered_users_only(bool val)
	{
		parameters_.registered_users_only = val;
	}

	void set_oos_debug(bool val)
	{
		state_.classification().oos_debug = val;
	}

	void set_shuffle_sides(bool val)
	{
		parameters_.shuffle_sides = val;
	}

	void set_random_faction_mode(mp_game_settings::RANDOM_FACTION_MODE val)
	{
		parameters_.random_faction_mode = val;
	}

	void set_options(const config& cfg);

	//
	// Parameter defaults
	//

	static std::string game_name_default();
	int num_turns_default() const;
	int village_gold_default() const;
	int village_support_default() const;
	int xp_modifier_default() const;
	int mp_countdown_init_time_default() const;
	int mp_countdown_reservoir_time_default() const;
	int mp_countdown_turn_bonus_default() const;
	int mp_countdown_action_bonus_default() const;
	bool mp_countdown_default() const;
	bool use_map_settings_default() const;
	bool random_start_time_default() const;
	bool fog_game_default() const;
	bool shroud_game_default() const;
	bool allow_observers_default() const;
	bool registered_users_only_default() const;
	bool shuffle_sides_default() const;
	mp_game_settings::RANDOM_FACTION_MODE random_faction_mode_default() const;
	const config& options_default() const;

	const mp_game_settings& get_parameters() const
	{
		return parameters_;
	}

	void write_parameters();

	void update_initial_cfg(const config& cfg)
	{
		initial_ = &cfg;
	}

private:
	saved_game& state_;

	mp_game_settings& parameters_;

	/// Never nullptr.
	const config* initial_;

	/**
	 * Village gold, village support, fog, and shroud are per player but always show the
	 * player 1's values.
	 *
	 * @todo This might not be 100% correct, but at the moment it is not possible to show
	 * fog and shroud per player. This might change in the future.
	 *
	 * @todo: Is the above even still true?
	 * -- vultraz, 2017-10-05
	 *
	 * NOTE when 'load game' is selected there are no sides.
	 */
	const config& side_cfg() const
	{
		return initial_->child_or_empty("side");
	}

	const config& initial_cfg() const
	{
		return *initial_;
	}
};

} // end namespace ng
