/*
   Copyright (C) 2013 by Andrius Silinskas <silinskas.andrius@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef FLG_MANAGER_HPP_INCLUDED
#define FLG_MANAGER_HPP_INCLUDED

#include "widgets/combo.hpp"

#include <string>
#include <vector>

class config;

namespace mp {

class flg_manager
{
public:
	flg_manager(const std::vector<const config*>& era_factions,
		const config& side, const bool map_settings, const bool saved_game,
		const bool first_scenario, const int color);
	~flg_manager();

	void set_current_faction(const unsigned index);
	void set_current_faction(const config* faction);
	void set_current_leader(const unsigned index);
	void set_current_gender(const unsigned index);

	// Update the status of combo: items, selection and whether
	// it should be enabled or not.
	void reset_leader_combo(gui::combo& combo_leader);
	void reset_gender_combo(gui::combo& combo_gender);

	const std::vector<const config*> available_factions() const
		{ return available_factions_; }
	const std::vector<const config*> choosable_factions() const
		{ return choosable_factions_; }
	const std::string& current_leader() const
		{ return current_leader_; }
	const std::string& current_gender() const
		{ return current_gender_; }

private:
	flg_manager(const flg_manager&);
	void operator=(const flg_manager&);

	void init_available_factions();
	void init_choosable_factions();
	void update_choosable_leaders();
	void update_choosable_genders();

	// Picks the first faction with the greater amount of data
	// matching the criteria.
	int find_suitable_faction() const;

	// Append leaders from a given faction
	// to a choosable factions.
	void append_leaders_from_faction(const config* faction);

	int current_leader_index() const;
	int current_gender_index() const;

	const std::vector<const config*>& era_factions_;

	const config& side_;

	const bool map_settings_;
	const bool saved_game_;
	const bool first_scenario_;

	const int color_;

	std::vector<const config*> available_factions_;
	std::vector<const config*> choosable_factions_;
	std::vector<std::string> choosable_leaders_;
	std::vector<std::string> choosable_genders_;

	const config* current_faction_;
	std::string current_leader_;
	std::string current_gender_;
};

} // end namespace mp

#endif
