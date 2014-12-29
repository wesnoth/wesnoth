
//the 'attack type' is the type of attack, how many times it strikes,
/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef UNIT_ATTACK_TYPE_H_INCLUDED
#define UNIT_ATTACK_TYPE_H_INCLUDED

#include "map_location.hpp"
#include "util.hpp"
#include "tstring.hpp"
#include "config.hpp"
#include <string>
#include <vector>

class unit_ability_list;

//the 'attack type' is the type of attack, how many times it strikes,
//and how much damage it does.
class attack_type
{
public:

	explicit attack_type(const config& cfg);
	/** Default implementation, but defined out-of-line for efficiency reasons. */
	~attack_type();
	const t_string& name() const { return description_; }
	const std::string& id() const { return id_; }
	const std::string& type() const { return type_; }
	const std::string& icon() const { return icon_; }
	const std::string& range() const { return range_; }
	int min_range() const { return min_range_; }
	int max_range() const { return max_range_; }
	std::string accuracy_parry_description() const;
	int accuracy() const { return accuracy_; }
	int parry() const { return parry_; }
	int damage() const { return damage_; }
	int num_attacks() const { return num_attacks_; }
	double attack_weight() const { return attack_weight_; }
	double defense_weight() const { return defense_weight_; }

	// In unit_abilities.cpp:

	bool get_special_bool(const std::string& special, bool simple_check=false) const;
	unit_ability_list get_specials(const std::string& special) const;
	std::vector<std::pair<t_string, t_string> > special_tooltips(std::vector<bool> *active_list=NULL) const;
	std::string weapon_specials(bool only_active=false, bool is_backstab=false) const;
	void set_specials_context(const map_location& unit_loc, const map_location& other_loc,
	                          bool attacking, const attack_type *other_attack) const;
	void set_specials_context(const map_location& loc, bool attacking = true) const;

	/// Calculates the number of attacks this weapon has, considering specials.
	void modified_attacks(bool is_backstab, unsigned & min_attacks,
	                      unsigned & max_attacks) const;
	/// Returns the damage per attack of this weapon, considering specials.
	int modified_damage(bool is_backstab) const;

	// In unit_types.cpp:

	bool matches_filter(const config& filter) const;
	bool apply_modification(const config& cfg,std::string* description);
	bool describe_modification(const config& cfg,std::string* description);

	int movement_used() const { return movement_used_; }

	void write(config& cfg) const;
	inline config to_config() const { config c; write(c); return c; }

private:
	// In unit_abilities.cpp:

	// Configured as a bit field, in case that is useful.
	enum AFFECTS { AFFECT_SELF=1, AFFECT_OTHER=2, AFFECT_EITHER=3 };
	bool special_active(const config& special, AFFECTS whom,
	                    bool include_backstab=true) const;

	// Used via set_specials_context() to control which specials are
	// considered active.
	mutable map_location self_loc_, other_loc_;
	mutable bool is_attacker_;
	mutable const attack_type* other_attack_;

	t_string description_;
	std::string id_;
	std::string type_;
	std::string icon_;
	std::string range_;
	int min_range_, max_range_;
	int damage_;
	int num_attacks_;
	double attack_weight_;
	double defense_weight_;

	int accuracy_;
	int movement_used_;
	int parry_;
	config specials_;
};
#endif
