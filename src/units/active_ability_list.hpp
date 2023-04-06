/*
	Copyright (C) 2006 - 2022
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
#include "units/ability.hpp"
#include "units/ptr.hpp"
#include "units/ptr.hpp"


/** Data typedef for active_ability_list. */
struct active_ability
{
	active_ability(ability_ptr ability, map_location student_loc, map_location teacher_loc)
		: student_loc(student_loc)
		, teacher_loc(teacher_loc)
		, ability(std::move(ability))
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
	/** The contents of the ability tag, never nullptr. */
	ability_ptr ability;

	const config& ability_cfg() const
	{
		return ability->cfg();
	}
};

class active_ability_list
{
public:
	active_ability_list(const map_location& loc = map_location()) : cfgs_() , loc_(loc) {}

	// Implemented in unit_abilities.cpp
	std::pair<int, map_location> highest(const std::string& key, int def=0) const
	{
		return get_extremum(key, def, std::less<int>());
	}
	std::pair<int, map_location> lowest(const std::string& key, int def=0) const
	{
		return get_extremum(key, def, std::greater<int>());
	}

	template<typename TComp>
	std::pair<int, map_location> get_extremum(const std::string& key, int def, const TComp& comp) const;

	// The following make this class usable with standard library algorithms and such
	typedef std::vector<active_ability>::iterator       iterator;
	typedef std::vector<active_ability>::const_iterator const_iterator;

	iterator       begin()        { return cfgs_.begin(); }
	const_iterator begin() const  { return cfgs_.begin(); }
	iterator       end()          { return cfgs_.end();   }
	const_iterator end()   const  { return cfgs_.end();   }

	// Vector access
	bool                empty() const  { return cfgs_.empty(); }
	active_ability&       front()        { return cfgs_.front(); }
	const active_ability& front() const  { return cfgs_.front(); }
	active_ability&       back()         { return cfgs_.back();  }
	const active_ability& back()  const  { return cfgs_.back();  }

	iterator erase(const iterator& erase_it)  { return cfgs_.erase(erase_it); }
	iterator erase(const iterator& first, const iterator& last)  { return cfgs_.erase(first, last); }

	template<typename... T>
	void emplace_back(T&&... args) { cfgs_.emplace_back(args...); }

	const map_location& loc() const { return loc_; }

	/** Appends the abilities from @a other to @a this, ignores other.loc() */
	void append(const active_ability_list& other)
	{
		std::copy(other.begin(), other.end(), std::back_inserter(cfgs_ ));
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
		std::copy_if(other.begin(), other.end(), std::back_inserter(cfgs_ ), predicate);
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
		effect(const active_ability_list& list, int def, const_attack_ptr attacker = const_attack_ptr(), bool is_cumulable = false);
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
