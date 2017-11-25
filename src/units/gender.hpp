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

#include <string>

class t_string;
class unit_gender;

class gender_list {
public:
	class const_iterator {
	public:
		const unit_gender& operator*() const;
		const_iterator& operator++();
		bool operator==(const const_iterator& other) const;
		bool operator!=(const const_iterator& other) const;
	private:
		const_iterator(int ix);
		int ix_;
		friend gender_list;
	};
	const_iterator begin();
	const_iterator end();
};

class unit_gender {
public:
	static const unit_gender& male();
	static const unit_gender& female();
	static constexpr int num_genders(){
		return 2;
	}

	static const unit_gender* from_string(const std::string& str_gender);
	static const unit_gender& from_string(const std::string& str_gender, const unit_gender& fallback);
	static gender_list genders();

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
