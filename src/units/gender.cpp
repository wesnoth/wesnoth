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

#include "units/gender.hpp"
#include "units/race.hpp"

#include <ostream>

namespace {
	/// Standard string id (not translatable) for FEMALE
	const std::string s_female("female");
	/// Standard string id (not translatable) for MALE
	const std::string s_male("male");
}

/*static*/ const unit_gender& unit_gender::male(){
	static unit_gender male = {0, s_male};
	return male;
}
/*static*/ const unit_gender& unit_gender::female(){
	static unit_gender female = {1, s_female};
	return female;
}

unit_gender::unit_gender(int index, const std::string& name)
	: index_(index)
	, name_(name)
{
	assert(index < unit_gender::num_genders());
}

std::ostream& operator<<(std::ostream& os, unit_gender gender){
	os << static_cast<int>(gender);
	return os;
}

/*static*/ const unit_gender* unit_gender::from_string(const std::string& str_gender){
	if ( str_gender == s_male ) {
		return &unit_gender::male();
	} else if ( str_gender == s_female ) {
		return &unit_gender::female();
	}
	return nullptr;
}
/*static*/ const unit_gender& unit_gender::from_string(const std::string& str_gender, const unit_gender& fallback){
	const unit_gender* gender = from_string(str_gender);
	return gender ? *gender : fallback;
}
/*static*/ const unit_gender* unit_gender::from_int(int index){
	//FIXME: this is a hack
	switch(index){
	case 0:
		return &unit_gender::male();
	case 1:
		return &unit_gender::female();
	default:
		return nullptr;
	}
}

const char* unit_gender::gender_string(const char* male_string, const char* female_string) const {
	return *this == female() ? female_string : male_string;
}
const std::string& unit_gender::gender_string(const std::string& male_string, const std::string& female_string) const {
	return *this == female() ? female_string : male_string;
}
const t_string& unit_gender::gender_string(const t_string& male_string, const t_string& female_string) const {
	return *this == female() ? female_string : male_string;
}
