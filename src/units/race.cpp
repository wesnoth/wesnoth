/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
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

#include "filesystem.hpp"
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
	name_[MALE] = "";
	name_[FEMALE] = "";
	name_generator_[MALE].reset(new name_generator());
	name_generator_[FEMALE].reset(new name_generator());
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
	name_[MALE] = cfg["male_name"];
	if(name_[MALE].empty()) {
		name_[MALE] = (cfg["name"]);
	}
	name_[FEMALE] = cfg["female_name"];
	if(name_[FEMALE].empty()) {
		name_[FEMALE] = (cfg["name"]);
	}

	name_generator_factory generator_factory = name_generator_factory(cfg, {"male", "female"});

	for(int i=MALE; i<NUM_GENDERS; i++) {
		GENDER gender = static_cast<GENDER>(i);
		name_generator_[i] = generator_factory.get_name_generator(gender_string(gender));
	}
}

std::string unit_race::generate_name(unit_race::GENDER gender) const
{
    return name_generator_[gender]->generate();
}

const name_generator& unit_race::generator(unit_race::GENDER gender) const
{
	return *name_generator_[gender];
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


const std::string& gender_string(unit_race::GENDER gender) {
	switch(gender) {
	case unit_race::FEMALE:
		return unit_race::s_female;
	default:
	case unit_race::MALE:
		return unit_race::s_male;
	}
}

unit_race::GENDER string_gender(const std::string& str, unit_race::GENDER def) {
	if ( str == unit_race::s_male ) {
		return unit_race::MALE;
	} else if ( str == unit_race::s_female ) {
		return unit_race::FEMALE;
	}
	return def;
}

std::string unit_race::get_icon_path_stem() const
{
	if(!icon_.empty()) {
		return icon_;
	}

	std::string path = "icons/unit-groups/race_" + id_;

	// FIXME: hardcoded '30' is bad...
	if(!filesystem::file_exists(filesystem::get_binary_file_location("images", path + "_30.png"))) {
		path = "icons/unit-groups/race_custom";
	}

	return path;
}
