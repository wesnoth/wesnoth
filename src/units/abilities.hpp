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

	enum class active_on_t { offense, defense, both };
	enum class apply_to_t { self, opponent, attacker, defender, both };

	enum class affects_t { SELF = 1, OTHER = 2, EITHER = 3 };

	unit_ability_t(std::string tag, config cfg, bool inside_attack);

	static ability_ptr create(std::string tag, config cfg, bool inside_attack) {
		return std::make_shared<unit_ability_t>(tag, cfg, inside_attack);
	}

	static void do_compat_fixes(config& cfg, bool inside_attack);

	const std::string& tag() const { return tag_; };
	const std::string& id() const { return id_; };
	bool in_specials_tag() const { return in_specials_tag_; };
	const config& cfg() const { return cfg_; };

	active_on_t active_on() const { return active_on_; };
	apply_to_t apply_to() const { return apply_to_; };

	void write(config& abilities_cfg);


	static void parse_vector(const config& abilities_cfg, ability_vector& res, bool inside_attack);
	static config vector_to_cfg(const ability_vector& abilities);
	static ability_vector cfg_to_vector(const config& abilities_cfg, bool inside_attack);


	static ability_vector filter_tag(const ability_vector& vec, const std::string& tag);
	static ability_vector clone(const ability_vector& vec);

private:
	std::string tag_;
	std::string id_;
	// abilities/specials inside [specials] tag follow a differnt syntax than in [abilities] tags, in paricular [filter_self] inside [specials] is equivalent to [filter_student] in abilities.
	bool in_specials_tag_;
	active_on_t active_on_;
	apply_to_t apply_to_;
	config cfg_;
};


/** Data typedef for active_ability_list. */
struct active_ability
{
	active_ability(const ability_ptr& p_ability, map_location student_loc, map_location teacher_loc)
		: student_loc(student_loc)
		, teacher_loc(teacher_loc)
		, p_ability_(p_ability)
	{
	}

	/**
	 * Used by the formula in the ability.
	 * The REAL location of the student (not the 'we are assuming the student is at this position' location)
	 * once active_ability_list can contain abilities from different 'students', as it contains abilities from
	 * a unit aswell from its opponents (abilities with apply_to= opponent)
	 */
	map_location student_loc;
	/**
	 * The location of the teacher, that is the unit who owns the ability tags
	 * (different from student because of [affect_adjacent])
	 */
	map_location teacher_loc;

	const config& ability_cfg() const { return p_ability_->cfg(); }
	const unit_ability_t& ability() const { return *p_ability_; }
private:
	/** The contents of the ability tag, never nullptr. */
	const_ability_ptr p_ability_;
};

class active_ability_list
{
public:
	active_ability_list(const map_location& loc = map_location()) : cfgs_(), loc_(loc) {}

	// Implemented in unit_abilities.cpp
	std::pair<int, map_location> highest(const std::string& key, int def = 0) const
	{
		return get_extremum(key, def, std::less<int>());
	}
	std::pair<int, map_location> lowest(const std::string& key, int def = 0) const
	{
		return get_extremum(key, def, std::greater<int>());
	}

	template<typename TComp>
	std::pair<int, map_location> get_extremum(const std::string& key, int def, const TComp& comp) const;

	// The following make this class usable with standard library algorithms and such
	typedef std::vector<active_ability>::iterator       iterator;
	typedef std::vector<active_ability>::const_iterator const_iterator;

	iterator       begin() { return cfgs_.begin(); }
	const_iterator begin() const { return cfgs_.begin(); }
	iterator       end() { return cfgs_.end(); }
	const_iterator end()   const { return cfgs_.end(); }

	// Vector access
	bool                empty() const { return cfgs_.empty(); }
	active_ability& front() { return cfgs_.front(); }
	const active_ability& front() const { return cfgs_.front(); }
	active_ability& back() { return cfgs_.back(); }
	const active_ability& back()  const { return cfgs_.back(); }
	std::size_t         size() { return cfgs_.size(); }

	iterator erase(const iterator& erase_it) { return cfgs_.erase(erase_it); }
	iterator erase(const iterator& first, const iterator& last) { return cfgs_.erase(first, last); }

	template<typename... T>
	void emplace_back(T&&... args) { cfgs_.emplace_back(args...); }

	const map_location& loc() const { return loc_; }

	/** Appends the abilities from @a other to @a this, ignores other.loc() */
	void append(const active_ability_list& other)
	{
		std::copy(other.begin(), other.end(), std::back_inserter(cfgs_));
	}

	/**
	 * Appends any abilities from @a other for which the given condition returns true to @a this, ignores other.loc().
	 *
	 * @param other where to copy the elements from
	 * @param predicate a single-argument function that takes a reference to an element and returns a bool
	 */
	template<typename Predicate>
	void append_if(const active_ability_list& other, const Predicate& predicate)
	{
		std::copy_if(other.begin(), other.end(), std::back_inserter(cfgs_), predicate);
	}

private:
	// Data
	std::vector<active_ability> cfgs_;
	map_location loc_;
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
