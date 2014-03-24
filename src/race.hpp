/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef RACE_HPP_INCLUDED
#define RACE_HPP_INCLUDED

#include "config.hpp"
#include "serialization/unicode.hpp"



typedef std::map<ucs4::string, ucs4::string > markov_prefix_map;

class unit_race
{
public:
	enum GENDER { MALE, FEMALE, NUM_GENDERS };
	static const std::string s_female;
	static const std::string s_male;


	explicit unit_race(const config& cfg);

	const config& get_cfg() const { return cfg_; }
	const std::string& id() const { return id_; }
	const t_string& name(GENDER gender=MALE) const { return name_[gender]; }
	const t_string& plural_name() const { return plural_name_; }
	const t_string& description() const { return description_; }

	std::string generate_name(GENDER gender) const;

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
	t_string name_[NUM_GENDERS];
	t_string plural_name_;
	t_string description_;
	unsigned int ntraits_;
	markov_prefix_map next_[NUM_GENDERS];
	int chain_size_;

	config::const_child_itors traits_;
	config::const_child_itors topics_;
	bool global_traits_;
	std::string undead_variation_;
};

unit_race::GENDER string_gender(const std::string& str,unit_race::GENDER def=unit_race::MALE);
std::string const& gender_string(unit_race::GENDER gender);

typedef std::map<std::string,unit_race> race_map;

#endif
