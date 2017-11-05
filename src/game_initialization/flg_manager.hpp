/*
   Copyright (C) 2013 - 2017 by Andrius Silinskas <silinskas.andrius@gmail.com>
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

#include <string>
#include <vector>

class config;
namespace randomness { class mt_rng; }

namespace ng {

const std::string random_enemy_picture("units/random-dice.png");

/// FLG stands for faction, leader and gender.
class flg_manager
{
public:
	flg_manager(const std::vector<const config*>& era_factions,
		const config& side, const bool faction_lock, const bool leader_lock, const bool saved_game);

	void set_current_faction(const unsigned index);
	void set_current_faction(const std::string& id);
	void set_current_faction(const config& faction)
		{ set_current_faction(faction_index(faction)); }

	void set_current_leader(const unsigned index);
	void set_current_leader(const std::string& leader);

	void set_current_gender(const unsigned index);
	void set_current_gender(const std::string& gender);

	bool is_random_faction();

	// Second Argument is a list of faction ids we don't want to match, used to implement random faction modes.
	// If it is not possible to resolve then we just proceed anyways rather than give an error.
	void resolve_random(randomness::mt_rng & rng, const std::vector<std::string> & avoid);

	bool is_saved_game() const
	{
		return saved_game_;
	}

	// Picks the first faction with the greater amount of data
	// matching the criteria.
	int find_suitable_faction() const;

	const std::vector<const config*>& choosable_factions() const
		{ return choosable_factions_; }
	const std::vector<std::string>& choosable_leaders() const
		{ return choosable_leaders_; }
	const std::vector<std::string>& choosable_genders() const
		{ return choosable_genders_; }
	const config& current_faction() const
		{ return *current_faction_; }
	const std::string& current_leader() const
		{ return current_leader_; }
	const std::string& current_gender() const
		{ return current_gender_; }

	const config* default_leader_cfg() const
		{ return default_leader_cfg_; }

	int current_faction_index() const;

	int current_leader_index() const
		{ return leader_index(current_leader_); }
	int current_gender_index() const
		{ return gender_index(current_gender_); }

private:
	flg_manager(const flg_manager&) = delete;
	void operator=(const flg_manager&) = delete;

	void update_available_factions();
	void update_available_leaders();
	void update_available_genders();
	void update_choosable_factions();
	void update_choosable_leaders();
	void update_choosable_genders();

	// Append leaders from a given faction to a choosable factions.
	void append_leaders_from_faction(const config* faction);

	void select_default_faction();

	int faction_index(const config& faction) const;
	/// returns -1 if no leader with that name was found
	int leader_index(const std::string& leader) const;
	/// returns -1 if no gender with that name was found
	int gender_index(const std::string& gender) const;

	const std::vector<const config*>& era_factions_;

	const config& side_;

	const bool saved_game_;
	const bool has_no_recruits_;

	const bool faction_lock_;
	const bool leader_lock_;

	// All factions which could be played by a side (including Random).
	std::vector<const config*> available_factions_;
	std::vector<std::string> available_leaders_;
	std::vector<std::string> available_genders_;

	std::vector<const config*> choosable_factions_;
	std::vector<std::string> choosable_leaders_;
	std::vector<std::string> choosable_genders_;

	const config* current_faction_;
	std::string current_leader_;
	std::string current_gender_;

	std::string default_leader_type_;
	std::string default_leader_gender_;
	const config* default_leader_cfg_;

	static std::vector<std::string> get_original_recruits(const config& cfg);
	static const config& get_default_faction(const config& cfg);
};

} // end namespace ng
