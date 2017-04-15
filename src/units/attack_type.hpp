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
#ifndef UNIT_ATTACK_TYPE_H_INCLUDED
#define UNIT_ATTACK_TYPE_H_INCLUDED

#include "map/location.hpp"
#include "tstring.hpp"
#include "config.hpp"
#include <string>
#include <vector>
#include <cassert>

#include <boost/iterator/indirect_iterator.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/dynamic_bitset_fwd.hpp>

#include "units/ptr.hpp" // for attack_ptr

class unit_ability_list;

//the 'attack type' is the type of attack, how many times it strikes,
//and how much damage it does.
class attack_type : public std::enable_shared_from_this<attack_type>
{
public:

	explicit attack_type(const config& cfg);
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
	const config &specials() const { return specials_; }

	void set_name(const t_string& value) { description_  = value; }
	void set_id(const std::string& value) { id_ = value; }
	void set_type(const std::string& value) { type_ = value; }
	void set_icon(const std::string& value) { icon_ = value; }
	void set_range(const std::string& value) { range_ = value; }
	void set_accuracy(int value) { accuracy_ = value; }
	void set_parry(int value) { parry_ = value; }
	void set_damage(int value) { damage_ = value; }
	void set_num_attacks(int value) { num_attacks_ = value; }
	void set_attack_weight(double value) { attack_weight_ = value; }
	void set_defense_weight(double value) { defense_weight_ = value; }
	void set_specials(config value) { specials_ = value; }


	// In unit_abilities.cpp:

	bool get_special_bool(const std::string& special, bool simple_check=false) const;
	unit_ability_list get_specials(const std::string& special) const;
	std::vector<std::pair<t_string, t_string> > special_tooltips(boost::dynamic_bitset<>* active_list = nullptr) const;
	std::string weapon_specials(bool only_active=false, bool is_backstab=false) const;
	void set_specials_context(const map_location& unit_loc, const map_location& other_loc,
	                          bool attacking, const_attack_ptr other_attack) const;
	void set_specials_context(const map_location& loc, bool attacking = true) const;
	void set_specials_context_for_listing() const;

	/// Calculates the number of attacks this weapon has, considering specials.
	void modified_attacks(bool is_backstab, unsigned & min_attacks,
	                      unsigned & max_attacks) const;
	/// Returns the damage per attack of this weapon, considering specials.
	int modified_damage(bool is_backstab) const;

	// In unit_types.cpp:

	bool matches_filter(const config& filter) const;
	bool apply_modification(const config& cfg);
	bool describe_modification(const config& cfg,std::string* description);

	int movement_used() const { return movement_used_; }
	void set_movement_used(int value) { movement_used_ = value; }

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
	mutable const_attack_ptr other_attack_;
	mutable bool is_for_listing_ = false;

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

using attack_list = std::vector<attack_ptr>;
using attack_itors = boost::iterator_range<boost::indirect_iterator<attack_list::iterator>>;
using const_attack_itors = boost::iterator_range<boost::indirect_iterator<attack_list::const_iterator>>;

inline attack_itors make_attack_itors(attack_list& atks) {
	return boost::make_iterator_range(boost::make_indirect_iterator(atks.begin()), boost::make_indirect_iterator(atks.end()));
}

inline const_attack_itors make_attack_itors(const attack_list& atks) {
	return boost::make_iterator_range(boost::make_indirect_iterator(atks.begin()), boost::make_indirect_iterator(atks.end()));
}

#endif
