/*
   Copyright (C) 2014 - 2017 by Nathan Walker <nathan.b.walker@vanderbilt.edu>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef MULTIPLAYER_CONFIGURE_ENGINE_INCLUDED
#define MULTIPLAYER_CONFIGURE_ENGINE_INCLUDED

#include "gettext.hpp"
#include "game_preferences.hpp"
#include "mp_game_settings.hpp"
#include "saved_game.hpp"

namespace ng {

/**
 * configure_engine
 *
 * this class wraps the parameters relevent to mp_configure,
 * as well as providing defaults for these parameters.
 */
class configure_engine
{
public:
	configure_engine(saved_game& state, const config* intial = nullptr);

	// Set all parameters to their default values
	void set_default_values();

	// check force_lock_settings in config
	bool force_lock_settings() const;

	// getter methods
	std::string game_name() const;
	int num_turns() const;
	int village_gold() const;
	int village_support() const;
	int xp_modifier() const;
	int mp_countdown_init_time() const;
	int mp_countdown_reservoir_time() const;
	int mp_countdown_turn_bonus() const;
	int mp_countdown_action_bonus() const;
	bool mp_countdown() const;
	bool use_map_settings() const;
	bool random_start_time() const;
	bool fog_game() const;
	bool shroud_game() const;
	bool allow_observers() const;
	bool registered_users_only() const;
	bool shuffle_sides() const;
	mp_game_settings::RANDOM_FACTION_MODE random_faction_mode() const;
	const config& options() const;

	// setter methods
	void set_game_name(std::string name);
	void set_game_password(std::string name);
	void set_num_turns(int val);
	void set_village_gold(int val);
	void set_village_support(int val);
	void set_xp_modifier(int val);
	void set_mp_countdown_init_time(int val);
	void set_mp_countdown_reservoir_time(int val);
	void set_mp_countdown_turn_bonus(int val);
	void set_mp_countdown_action_bonus(int val);
	void set_mp_countdown(bool val);
	void set_use_map_settings(bool val);
	void set_random_start_time(bool val);
	void set_fog_game(bool val);
	void set_shroud_game(bool val);
	void set_allow_observers(bool val);
	void set_registered_users_only(bool val);
	void set_oos_debug(bool val);
	void set_shuffle_sides(bool val);
	void set_random_faction_mode(mp_game_settings::RANDOM_FACTION_MODE val);
	void set_options(const config& cfg);

	// parameter defaults
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

	// parameters_ accessor
	const mp_game_settings& get_parameters() const;

	const std::vector<std::string>& entry_point_titles() const;
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
	// village gold, village support, fog, and shroud are per player, always show values of player 1.
	/**
	 * @todo This might not be 100% correct, but at the moment
	 * it is not possible to show the fog and shroud per player.
	 * This might change in the future.
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
#endif

