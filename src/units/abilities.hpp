/*
	Copyright (C) 2006 - 2025
	by Dominic Bolin <dominic.bolin@exong.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "map/location.hpp"
#include "units/ptr.hpp"

class unit_ability_list;
namespace unit_abilities
{
bool filter_base_matches(const config& cfg, int def);

enum value_modifier {NOT_USED,SET,ADD,MUL,DIV};

enum EFFECTS { EFFECT_DEFAULT=1, EFFECT_CUMULABLE=2, EFFECT_WITHOUT_CLAMP_MIN_MAX=3 };

/**
 * Substitute gettext variables in name and description of abilities and specials
 * @param str                  The string in which the substitution is to be done
 * @param tag_name             Tag name of the special (plague, leadership, chance_to_hit etc.)
 * @param ability_or_special   The config for the special (for example, contents inside [plague][/plague] etc.)
 *
 * @return The string `str` with all gettext variables substitutes with corresponding special properties
 */
std::string substitute_variables(const std::string& str, const std::string& tag_name, const config& ability_or_special);

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
		effect(const unit_ability_list& list, int def, const const_attack_ptr& attacker = const_attack_ptr(), EFFECTS wham = EFFECT_DEFAULT);
		// Provide read-only access to the effect list:
		typedef std::vector<individual_effect>::const_iterator iterator;
		typedef std::vector<individual_effect>::const_iterator const_iterator;

		int get_composite_value() const
		{ return composite_value_; }
		double get_composite_double_value() const
		{ return composite_double_value_; }
		const_iterator begin() const
		{ return effect_list_.begin(); }
		const_iterator end() const
		{ return effect_list_.end(); }
	private:
		std::vector<individual_effect> effect_list_;
		int composite_value_;
		double composite_double_value_;
};


}
