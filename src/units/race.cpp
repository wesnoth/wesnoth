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

/**
 *  @file
 *  Generate race-specific unit-names.
 */

#include "units/race.hpp"

#include "log.hpp"
#include "serialization/string_utils.hpp"
#include "serialization/unicode_cast.hpp"
#include "utils/name_generator.hpp"
#include "utils/name_generator_factory.hpp"

/// Dummy race used when a race is not yet known.
const unit_race unit_race::null_race;
/// Standard string id (not translatable) for FEMALE
const std::string unit_race::s_female("female");
/// Standard string id (not translatable) for MALE
const std::string unit_race::s_male("male");


static const config &empty_traits() {
		static config cfg;
		return cfg;
}

static const config &empty_topics() {
  		static config cfg;
		return cfg;
}

unit_race::unit_race() :
		cfg_(),
		id_(),
		plural_name_(),
		description_(),
		ntraits_(0),
		traits_(empty_traits().child_range("trait")),
		topics_(empty_topics().child_range("topic")),
		global_traits_(true),
		undead_variation_()
{
	name_[static_cast<int>(unit_gender::MALE)] = "";
	name_[static_cast<int>(unit_gender::FEMALE)] = "";
	name_generator_[static_cast<int>(unit_gender::MALE)].reset(new name_generator());
	name_generator_[static_cast<int>(unit_gender::FEMALE)].reset(new name_generator());
}

unit_race::unit_race(const config& cfg) :
		cfg_(cfg),
		id_(cfg["id"]),
		icon_(cfg["editor_icon"]),
		plural_name_(cfg["plural_name"].t_str()),
		description_(cfg["description"].t_str()),
		ntraits_(cfg["num_traits"]),
		traits_(cfg.child_range("trait")),
		topics_(cfg.child_range("topic")),
		global_traits_(!cfg["ignore_global_traits"].to_bool()),
		undead_variation_(cfg["undead_variation"])

{
	if (id_.empty()) {
		lg::wml_error() << "[race] '" << cfg["name"] << "' is missing an id field.";
	}
	if (plural_name_.empty()) {
		lg::wml_error() << "[race] '" << cfg["name"] << "' is missing a plural_name field.";
		plural_name_ = (cfg["name"]);
	}

	// use "name" if "male_name" or "female_name" aren't available
	name_[static_cast<int>(unit_gender::MALE)] = cfg["male_name"];
	if(name_[static_cast<int>(unit_gender::MALE)].empty()) {
		name_[static_cast<int>(unit_gender::MALE)] = (cfg["name"]);
	}
	name_[static_cast<int>(unit_gender::FEMALE)] = cfg["female_name"];
	if(name_[static_cast<int>(unit_gender::FEMALE)].empty()) {
		name_[static_cast<int>(unit_gender::FEMALE)] = (cfg["name"]);
	}

	name_generator_factory generator_factory = name_generator_factory(cfg, {"male", "female"});

	for(int i = static_cast<int>(unit_gender::MALE); i < static_cast<int>(unit_gender::NUM_GENDERS); ++i) {
		unit_gender gender = static_cast<unit_gender>(i);
		name_generator_[i] = generator_factory.get_name_generator(gender_string(gender));
	}
}

std::string unit_race::generate_name(unit_gender gender) const
{
    return name_generator_[static_cast<int>(gender)]->generate();
}

const name_generator& unit_race::generator(unit_gender gender) const
{
	return *name_generator_[static_cast<int>(gender)];
}

bool unit_race::uses_global_traits() const
{
	return global_traits_;
}

const config::const_child_itors &unit_race::additional_traits() const
{
	return traits_;
}

const config::const_child_itors &unit_race::additional_topics() const
{
  return topics_;
}

unsigned int unit_race::num_traits() const { return ntraits_; }

