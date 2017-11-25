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

#include <vector>

namespace {
	/// Standard string id (not translatable) for FEMALE
	const std::string s_female("female");
	/// Standard string id (not translatable) for MALE
	const std::string s_male("male");
	/// Storage for gender map
	std::vector<const unit_gender*> gender_map = std::vector<const unit_gender*>(unit_gender::num_genders(), nullptr);
	// Populate the map
	auto & m_ = unit_gender::male();
	auto & f_ = unit_gender::female();

	// C++17 as_const
	template<class T>
	T const& constant(T& v){ return v; }
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
	assert(gender_map[index] == nullptr);
	gender_map[index] = this;
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
/*static*/ gender_list unit_gender::genders(){
	return {};
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

gender_list::const_iterator gender_list::begin(){
	return {0};
}
gender_list::const_iterator gender_list::end(){
	return {unit_gender::num_genders()};
}
const unit_gender& gender_list::const_iterator::operator*() const {
	assert(ix_ >= 0);
	assert(ix_ < unit_gender::num_genders());
	return *gender_map[ix_];
}
gender_list::const_iterator& gender_list::const_iterator::operator++() {
	++ix_;
	return *this;
}
bool gender_list::const_iterator::operator==(const const_iterator& other) const {
	return ix_ == other.ix_;
}
bool gender_list::const_iterator::operator!=(const const_iterator& other) const {
	return !(*this == other);
}
gender_list::const_iterator::const_iterator(int ix) : ix_(ix) {
	assert(ix >= 0);
	assert(ix <= unit_gender::num_genders());
}
