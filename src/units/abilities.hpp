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


#include <vector>
class config;

class active_ability_list;


using ability_vector = std::vector<ability_ptr>;

class unit_ability_t
{
public:
	unit_ability_t(std::string tag, config cfg, bool inside_attack);

	static ability_ptr create(std::string tag, config cfg, bool inside_attack) {
		return std::make_shared<unit_ability_t>(tag, cfg, inside_attack);
	}

	static void do_compat_fixes(config& cfg, bool inside_attack);

	const std::string& tag() const { return tag_; };
	const std::string& id() const { return id_; };
	const config& cfg() const { return cfg_; };
	void write(config& abilities_cfg);


	static void parse_vector(const config& abilities_cfg, ability_vector& res, bool inside_attack);
	static config vector_to_cfg(const ability_vector& abilities);
	static ability_vector cfg_to_vector(const config& abilities_cfg, bool inside_attack);


	static ability_vector filter_tag(const ability_vector& vec, const std::string& tag);
	static ability_vector clone(const ability_vector& vec);

	/*
	static auto get_view(const ability_vector& vec) {
		return vec
			| boost::adaptors::transformed([](const ability_ptr& p)->const config& { return *x; });
	}

	static auto get_view(const ability_vector& vec, const std::string& tag) {
		return vec
			| boost::adaptors::transformed([](const ability_ptr& p)->const config& { return *x; });
	}
*/
private:
	std::string tag_;
	std::string id_;
	config cfg_;
};


namespace unit_abilities
{
bool filter_base_matches(const config& cfg, int def);

enum value_modifier {NOT_USED,SET,ADD,MUL,DIV};

enum EFFECTS { EFFECT_DEFAULT=1, EFFECT_CUMULABLE=2, EFFECT_WITHOUT_CLAMP_MIN_MAX=3 };

/**
 * Substitute gettext variables in name and description of abilities and specials
 * @param str                  The string in which the substitution is to be done
 * @param ab                   The special (for example  [plague][/plague] etc.)
 *
 * @return The string `str` with all gettext variables substitutes with corresponding special properties
 */
std::string substitute_variables(const std::string& str, const unit_ability_t& ab);

struct individual_effect
{
	individual_effect() : type(NOT_USED), value(0), ability(nullptr),
		loc(map_location::null_location()) {}
	void set(value_modifier t, int val, const config& abil,const map_location &l);
	value_modifier type;
	int value;
	const config *ability;
	map_location loc;
};

class effect
{
	public:
		effect(const active_ability_list& list, int def, const const_attack_ptr& attacker = const_attack_ptr(), EFFECTS wham = EFFECT_DEFAULT);
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
