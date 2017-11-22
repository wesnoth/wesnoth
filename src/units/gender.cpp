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

std::ostream& operator<<(std::ostream& os, unit_gender gender){
	os << static_cast<int>(gender);
	return os;
}

const std::string& gender_string(unit_gender gender) {
	switch(gender) {
	case unit_gender::FEMALE:
		return unit_race::s_female;
	default:
	case unit_gender::MALE:
		return unit_race::s_male;
	}
}

unit_gender string_gender(const std::string& str, unit_gender def) {
	if ( str == unit_race::s_male ) {
		return unit_gender::MALE;
	} else if ( str == unit_race::s_female ) {
		return unit_gender::FEMALE;
	}
	return def;
}
