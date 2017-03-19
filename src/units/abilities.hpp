/*
   Copyright (C) 2006 - 2017 by Dominic Bolin <dominic.bolin@exong.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file */

#ifndef UNIT_ABILITIES_HPP_INCLUDED
#define UNIT_ABILITIES_HPP_INCLUDED

#include "map/location.hpp"

class unit_ability_list;

namespace unit_abilities
{
bool filter_base_matches(const config& cfg, int def);

enum value_modifier {NOT_USED,SET,ADD,MUL,DIV};

struct individual_effect
{
	individual_effect() : type(NOT_USED), value(0), ability(nullptr),
		loc(map_location::null_location()) {}
	void set(value_modifier t, int val, const config *abil,const map_location &l);
	value_modifier type;
	int value;
	const config *ability;
	map_location loc;
};

class effect
{
	public:
		effect(const unit_ability_list& list, int def, bool backstab);

		// Provide read-only access to the effect list:
		typedef std::vector<individual_effect>::const_iterator iterator;
		typedef std::vector<individual_effect>::const_iterator const_iterator;

		int get_composite_value() const
		{ return composite_value_; }
		const_iterator begin() const
		{ return effect_list_.begin(); }
		const_iterator end() const
		{ return effect_list_.end(); }
	private:
		std::vector<individual_effect> effect_list_;
		int composite_value_;
};


}


#endif

