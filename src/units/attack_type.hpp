/*
	Copyright (C) 2003 - 2025
	by David White <dave@whitevine.net>
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
#include "tstring.hpp"
#include "config.hpp"
#include <string>
#include <vector>

#include <boost/iterator/indirect_iterator.hpp>
#include <boost/dynamic_bitset_fwd.hpp>

#include "units/abilities.hpp"
#include "units/ptr.hpp" // for attack_ptr
#include "units/unit_alignments.hpp"

class active_ability_list;
class unit_type;
class specials_context_t;

namespace wfl {
	class map_formula_callable;
}

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
	std::string accuracy_parry_tooltip() const;
	int accuracy() const { return accuracy_; }
	int parry() const { return parry_; }
	int damage() const { return damage_; }
	int num_attacks() const { return num_attacks_; }
	double attack_weight() const { return attack_weight_; }
	double defense_weight() const { return defense_weight_; }
	const ability_vector& specials() const { return specials_; }

	config specials_cfg() const {
		return unit_ability_t::vector_to_cfg(specials_);
	}

	void set_name(const t_string& value) { description_  = value; set_changed(true); }
	void set_id(const std::string& value) { id_ = value; set_changed(true); }
	void set_type(const std::string& value) { type_ = value; set_changed(true); }
	void set_icon(const std::string& value) { icon_ = value; set_changed(true); }
	void set_range(const std::string& value) { range_ = value; set_changed(true); }
	void set_min_range(int value) { min_range_ = value; set_changed(true); }
	void set_max_range(int value) { max_range_ = value; set_changed(true); }
	void set_attack_alignment(const std::string& value) { alignment_ = unit_alignments::get_enum(value); set_changed(true); }
	void set_accuracy(int value) { accuracy_ = value; set_changed(true); }
	void set_parry(int value) { parry_ = value; set_changed(true); }
	void set_damage(int value) { damage_ = value; set_changed(true); }
	void set_num_attacks(int value) { num_attacks_ = value; set_changed(true); }
	void set_attack_weight(double value) { attack_weight_ = value; set_changed(true); }
	void set_defense_weight(double value) { defense_weight_ = value; set_changed(true); }
	void set_specials_cfg(const config& value) {
		specials_ = unit_ability_t::cfg_to_vector(value, true);  set_changed(true);
	}

	std::vector<unit_ability_t::tooltip_info> special_tooltips() const;

	/** Returns alignment specified by alignment_ variable.
	 */
	utils::optional<unit_alignments::type> alignment() const { return alignment_; }
	/** Returns alignment specified by alignment() for filtering when exist.
	 */
	std::string alignment_str() const { return alignment_ ? unit_alignments::get_string(*alignment_) : ""; }

	/** Calculates the number of attacks this weapon has, considering specials. */
	void modified_attacks(unsigned & min_attacks,
	                      unsigned & max_attacks) const;

	/** @return A type()/replacement_type and a list of alternative_types that should be displayed in the selected unit's report. */
	std::pair<std::string, std::set<std::string>> damage_types() const;
	/** @return The type of attack used and the resistance value that does the most damage. */
	std::pair<std::string, int> effective_damage_type() const;

	/** Returns the damage per attack of this weapon, considering specials. */
	double modified_damage() const;
	/** Return the defense value, considering specials.
	 * @param cth The chance_to_hit value modified or not by function.
	 */
	int modified_chance_to_hit(int cth) const;

	/** Return the special weapon value, considering specials.
	 * @param abil_list The list of special checked.
	 * @param base_value The value modified or not by function.
	 */
	int composite_value(const active_ability_list& abil_list, int base_value) const;
	/**
	 * @param special the tag name to check for
	 * @return list which contains get_weapon_ability and get_specials list for each ability type, with overwritten items removed
	 */
	active_ability_list get_specials_and_abilities(const std::string& special) const;
	/** used for abilities used like weapon and true specials
	 * @return True if the ability @a special is active.
	 * @param special The special being checked.
	 */
	bool has_special_or_ability(const std::string& special) const;
	/**
	 * @param special id to check.
	 */
	bool has_active_special_or_ability_id(const std::string& special) const;
	/** check if special matche
	 * handles the special_(id/type) attributes in weapon filters.
	 * @return True if a speical matching the filter was found.
	 * @param filter contains attributes special_id, special_type, special
	 */
	bool has_filter_special_or_ability(const config& filter) const;
	/**
	 * Returns true if this is a dummy attack_type, for example the placeholder that the unit_attack dialog
	 * uses when a defender has no weapon for a given range.
	 */
	bool attack_empty() const {return (id().empty() && name().empty() && type().empty() && range().empty());}
	/** remove special if matche condition
	 * @param filter if special check with filter, it will be removed.
	 */
	void remove_special_by_filter(const config& filter);
	/** check if special matche
	 * @return True if special matche with filter(if 'active' filter is true, check if special active).
	 * @param filter if special check with filter, return true.
	 */
	bool has_special_or_ability_with_filter(const config & filter) const;

	// In unit_types.cpp:

	bool matches_filter(const config& filter, const std::string& check_if_recursion = "") const;

	/** Applies effect modifications described by @a cfg. */
	void apply_effect(const config& cfg);

	/**
	 * Generates a description of the effect specified by @a cfg, if applicable.
	 * This covers a subset of the effects which can be applied via @ref apply_effect.
	 */
	static std::string describe_effect(const config& cfg);

	int movement_used() const { return movement_used_; }
	void set_movement_used(int value) { movement_used_ = value; }
	int attacks_used() const { return attacks_used_; }
	void set_attacks_used(int value) { attacks_used_ = value; }

	void write(config& cfg) const;
	inline config to_config() const { config c; write(c); return c; }

	void add_formula_context(wfl::map_formula_callable&) const;

	// In unit_abilities.cpp:

	// Configured as a bit field, in case that is useful.
	using AFFECTS = unit_ability_t::affects_t;
	/**
	 * Select best damage type based on frequency count for replacement_type.
	 *
	 * @param damage_type_list list of [damage_type] to check.
	 */
	std::string select_replacement_type(const active_ability_list& damage_type_list) const;
	/**
	 * Select best damage type based on highest damage for alternative_type.
	 *
	 * @param damage_type_list list of [damage_type] to check.
	 * @param resistance_list list of "resistance" abilities to check for each type of damage checked.
	 */
	std::pair<std::string, int> select_alternative_type(const active_ability_list& damage_type_list, const active_ability_list& resistance_list) const;
	/**
	 * Filter a list of abilities or weapon specials, removing any entries that don't own
	 * the overwrite_specials attributes.
	 *
	 * @param overwriters list that may have overwrite_specials attributes.
	 */
	active_ability_list overwrite_special_overwriter(active_ability_list overwriters) const;
	/**
	 * Check whether @a cfg would be overwritten by any element of @a overwriters.
	 *
	 * @return True if element checked is overwritable.
	 * @param overwriters list used for check if element is overwritable.
	 * @param i the ability/special checked
	 */
	bool overwrite_special_checking(active_ability_list& overwriters, const active_ability& i) const;

	bool special_active(const unit_ability_t& ab, AFFECTS whom) const;

	// make more functions proivate after refactoring finished.

	// Used via specials_context() to control which specials are
	// considered active.
	friend class specials_context_t;
	mutable specials_context_t* context_;

	std::unique_ptr<specials_context_t> fallback_context(const unit_ptr& self = nullptr) const;

	void set_changed(bool value)
	{
		changed_ = value;
	}
	bool get_changed() const
	{
		return changed_;
	}
private:

	t_string description_;
	std::string id_;
	std::string type_;
	std::string icon_;
	std::string range_;
	int min_range_, max_range_;
	utils::optional<unit_alignments::type> alignment_;
	int damage_;
	int num_attacks_;
	double attack_weight_;
	double defense_weight_;

	int accuracy_;
	int movement_used_;
	int attacks_used_;
	int parry_;
	ability_vector specials_;
	bool changed_;
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
