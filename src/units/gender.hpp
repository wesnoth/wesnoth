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

enum class unit_gender {
	MALE, FEMALE, NUM_GENDERS
};

std::ostream& operator<<(std::ostream& os, unit_gender gender);

unit_gender string_gender(const std::string& str, unit_gender def=unit_gender::MALE);
const std::string& gender_string(unit_gender gender);
