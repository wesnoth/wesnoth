/*
   Copyright (C) 2003 - 2016 by David White <dave@whitevine.net>
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

#include "global.hpp"

#include "units/race.hpp"

#include "log.hpp"
#include "serialization/string_utils.hpp"
#include "serialization/unicode_cast.hpp"
#include "utils/markov_generator.hpp"
#include "utils/context_free_grammar_generator.hpp"

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
}

unit_race::unit_race(const config& cfg) :
		cfg_(cfg),
		id_(cfg["id"]),
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

	config::attribute_value male_generator = cfg["male_name_generator"];
	config::attribute_value female_generator = cfg["female_name_generator"];
	if(male_generator.blank()) {
		male_generator = cfg["name_generator"];
	}
	if(female_generator.blank()) {
		female_generator = cfg["name_generator"];
	}

	if(!male_generator.blank()) {
		name_generator_[MALE].reset(new context_free_grammar_generator(male_generator));
		if(!name_generator_[MALE]->is_valid()) {
			name_generator_[MALE].reset();
		}
	}
	if(!female_generator.blank()) {
		name_generator_[FEMALE].reset(new context_free_grammar_generator(female_generator));
		if(!name_generator_[FEMALE]->is_valid()) {
			name_generator_[FEMALE].reset();
		}
	}

	int chain_size = cfg["markov_chain_size"].to_int(2);
	if(!name_generator_[MALE]) {
		name_generator_[MALE].reset(new markov_generator(utils::split(cfg["male_names"]), chain_size, 12));
	}
	if(!name_generator_[FEMALE]) {
		name_generator_[FEMALE].reset(new markov_generator(utils::split(cfg["female_names"]), chain_size, 12));
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


std::string const& gender_string(unit_race::GENDER gender) {
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
