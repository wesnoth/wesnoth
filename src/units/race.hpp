/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
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

#include "config.hpp"
#include "units/gender.hpp"
#include "utils/name_generator.hpp"
#include <memory>

class unit_race
{
public:
	static const std::string s_female;
	static const std::string s_male;


	explicit unit_race(const config& cfg);

	const config& get_cfg() const { return cfg_; }
	const std::string& id() const { return id_; }
	const std::string& editor_icon() const { return icon_; }
	const t_string& name(unit_gender gender=unit_gender::MALE) const { return name_[static_cast<int>(gender)]; }
	const t_string& plural_name() const { return plural_name_; }
	const t_string& description() const { return description_; }

	std::string generate_name(unit_gender gender) const;
	const name_generator& generator(unit_gender gender) const;

	bool uses_global_traits() const;

	const config::const_child_itors &additional_traits() const;
	const config::const_child_itors &additional_topics() const;
	unsigned int num_traits() const;
	const std::string& undead_variation() const { return undead_variation_; }

	/// Dummy race used when a race is not yet known.
	static const unit_race null_race;

private:
	/// Only used to construct null_race.
	unit_race();

	const config cfg_;

	std::string id_;
	std::string icon_;
	t_string name_[static_cast<int>(unit_gender::NUM_GENDERS)];
	t_string plural_name_;
	t_string description_;
	unsigned int ntraits_;
	std::shared_ptr<name_generator> name_generator_[static_cast<int>(unit_gender::NUM_GENDERS)];

	config::const_child_itors traits_;
	config::const_child_itors topics_;
	bool global_traits_;
	std::string undead_variation_;
};

typedef std::map<std::string,unit_race> race_map;
