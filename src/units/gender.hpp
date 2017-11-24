/*
   Copyright (C) 2014 - 2017 by Chris Beck <render787@gmail.com>
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

#include <iosfwd>

class t_string;
class unit_gender;

class unit_gender {
public:
	static const unit_gender& male();
	static const unit_gender& female();
	static constexpr int num_genders(){
		return 2;
	}

	static const unit_gender* from_string(const std::string&);
	static const unit_gender& from_string(const std::string&, const unit_gender& fallback);
	static const unit_gender* from_int(int);

	bool operator==(const unit_gender& other) const {
		return this == &other;
	}

	explicit operator int() const { return index_; }
	explicit operator const std::string&() const { return this->str(); }
	const std::string& str() const { return name_; }

	const char* gender_string(const char* male_string, const char* female_string) const;
	const std::string& gender_string(const std::string& male_string, const std::string& female_string) const;
	const t_string& gender_string(const t_string& male_string, const t_string& female_string) const;
private:
	unit_gender(int index, const std::string& name);
	unit_gender(unit_gender&) = delete;
	const int index_;
	const std::string& name_;
};

// Shows underlying integer type
std::ostream& operator<<(std::ostream&, unit_gender);
