/*
	Copyright (C) 2003 - 2024
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
#include <cassert>

#include <boost/iterator/indirect_iterator.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/dynamic_bitset_fwd.hpp>

#include "units/ptr.hpp" // for attack_ptr

class unit_ability_list;
class unit_type;
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
	int accuracy() const { return accuracy_; }
	int parry() const { return parry_; }
	int damage() const { return damage_; }
	int num_attacks() const { return num_attacks_; }
	double attack_weight() const { return attack_weight_; }
	double defense_weight() const { return defense_weight_; }
	const config &specials() const { return specials_; }

	void set_name(const t_string& value) { description_  = value; set_changed(true); }
	void set_id(const std::string& value) { id_ = value; set_changed(true); }
	void set_type(const std::string& value) { type_ = value; set_changed(true); }
	void set_icon(const std::string& value) { icon_ = value; set_changed(true); }
	void set_range(const std::string& value) { range_ = value; set_changed(true); }
	void set_accuracy(int value) { accuracy_ = value; set_changed(true); }
	void set_parry(int value) { parry_ = value; set_changed(true); }
	void set_damage(int value) { damage_ = value; set_changed(true); }
	void set_num_attacks(int value) { num_attacks_ = value; set_changed(true); }
	void set_attack_weight(double value) { attack_weight_ = value; set_changed(true); }
	void set_defense_weight(double value) { defense_weight_ = value; set_changed(true); }
	void set_specials(config value) { specials_ = value; set_changed(true); }


	// In unit_abilities.cpp:

	/**
	 * @return True iff the special @a special is active.
	 * @param special The special being checked.
	 * @param simple_check If true, check whether the unit has the special. Else, check whether the special is currently active.
	 * @param special_id If true, match @a special against the @c id of special tags.
	 * @param special_tags If true, match @a special against the tag name of special tags.
	 */
	bool has_special(const std::string& special, bool simple_check=false, bool special_id=true, bool special_tags=true) const;
	unit_ability_list get_specials(const std::string& special) const;
	std::vector<std::pair<t_string, t_string>> special_tooltips(boost::dynamic_bitset<>* active_list = nullptr) const;
	std::string weapon_specials() const;
	std::string weapon_specials_value(const std::set<std::string> checking_tags) const;

	/** Calculates the number of attacks this weapon has, considering specials. */
	void modified_attacks(unsigned & min_attacks,
	                      unsigned & max_attacks) const;

	/** return a modified damage type and/or add a secondary_type for hybrid use if special is active. */
	std::pair<std::string, std::string> damage_type() const;

	/** Returns the damage per attack of this weapon, considering specials. */
	int modified_damage() const;

	/** Return the special weapon value, considering specials.
	 * @param abil_list The list of special checked.
	 * @param base_value The value modified or not by function.
	 */
	int composite_value(const unit_ability_list& abil_list, int base_value) const;
	/** Returns list for weapon like abilities for each ability type. */
	unit_ability_list get_weapon_ability(const std::string& ability) const;
	/**
	 * @param special the tag name to check for
	 * @return list which contains get_weapon_ability and get_specials list for each ability type, with overwritten items removed
	 */
	unit_ability_list get_specials_and_abilities(const std::string& special) const;
	/** used for abilities used like weapon
	 * @return True if the ability @a special is active.
	 * @param special The special being checked.
	 * @param special_id If true, match @a special against the @c id of special tags.
	 * @param special_tags If true, match @a special against the tag name of special tags.
	 */
	bool has_weapon_ability(const std::string& special, bool special_id=true, bool special_tags=true) const;
	/** used for abilities used like weapon and true specials
	 * @return True if the ability @a special is active.
	 * @param special The special being checked.
	 * @param special_id If true, match @a special against the @c id of special tags.
	 * @param special_tags If true, match @a special against the tag name of special tags.
	 */
	bool has_special_or_ability(const std::string& special, bool special_id=true, bool special_tags=true) const;
	/**
	 * Returns true if this is a dummy attack_type, for example the placeholder that the unit_attack dialog
	 * uses when a defender has no weapon for a given range.
	 */
	bool attack_empty() const {return (id().empty() && name().empty() && type().empty() && range().empty());}

	// In unit_types.cpp:

	bool matches_filter(const config& filter, const std::string& check_if_recursion = "") const;
	bool apply_modification(const config& cfg);
	bool describe_modification(const config& cfg,std::string* description);

	int movement_used() const { return movement_used_; }
	void set_movement_used(int value) { movement_used_ = value; }
	int attacks_used() const { return attacks_used_; }
	void set_attacks_used(int value) { attacks_used_ = value; }

	void write(config& cfg) const;
	inline config to_config() const { config c; write(c); return c; }

	void add_formula_context(wfl::map_formula_callable&) const;

	/**
	 * Helper similar to std::unique_lock for detecting when calculations such as has_special
	 * have entered infinite recursion.
	 *
	 * This assumes that there's only a single thread accessing the attack_type, it's a lightweight
	 * increment/decrement counter rather than a mutex.
	 */
	class recursion_guard {
		friend class attack_type;
		/**
		 * Only expected to be called in update_variables_recursion(), which handles some of the checks.
		 */
		explicit recursion_guard(const attack_type& weapon);
	public:
		/**
		 * Construct an empty instance, only useful for extending the lifetime of a
		 * recursion_guard returned from weapon.update_variables_recursion() by
		 * std::moving it to an instance declared in a larger scope.
		 */
		explicit recursion_guard();

		/**
		 * Returns true if a level of recursion was available at the time when update_variables_recursion()
		 * created this object.
		 */
		operator bool() const;

		recursion_guard(recursion_guard&& other);
		recursion_guard(const recursion_guard& other) = delete;
		recursion_guard& operator=(recursion_guard&&);
		recursion_guard& operator=(const recursion_guard&) = delete;
		~recursion_guard();
	private:
		std::shared_ptr<const attack_type> parent;
	};

	/**
	 * Tests which might otherwise cause infinite recursion should call this, check that the
	 * returned object evaluates to true, and then keep the object returned as long as the
	 * recursion might occur, similar to a reentrant mutex that's limited to a small number of
	 * reentrances.
	 *
	 * This is a cheap function, so no reason to optimise by doing some filters before calling it.
	 * However, it only expects to be called in a single thread, but the whole of attack_type makes
	 * that assumption, for example its mutable members are assumed to be set up by the current
	 * caller (or caller's caller, probably several layers up).
	 */
	recursion_guard update_variables_recursion() const;

private:
	// In unit_abilities.cpp:

	// Configured as a bit field, in case that is useful.
	enum AFFECTS { AFFECT_SELF=1, AFFECT_OTHER=2, AFFECT_EITHER=3 };
	/**
	 * Filter a list of abilities or weapon specials, removing any entries that don't own
	 * the overwrite_specials attributes.
	 *
	 * @param overwriters list that may have overwrite_specials attributes.
	 * @param tag_name type of abilitie/special checked.
	 */
	unit_ability_list overwrite_special_overwriter(unit_ability_list overwriters, const std::string& tag_name) const;
	/**
	 * Check whether @a cfg would be overwritten by any element of @a overwriters.
	 *
	 * @return True if element checked is overwritable.
	 * @param overwriters list used for check if element is overwritable.
	 * @param cfg element checked.
	 * @param tag_name type of abilitie/special checked.
	 */
	bool overwrite_special_checking(unit_ability_list& overwriters, const config& cfg, const std::string& tag_name) const;
	/** check_self_abilities : return an boolean value for checking of activities of abilities used like weapon
	 * @return True if the special @a special is active.
	 * @param cfg the config to one special ability checked.
	 * @param special The special ability type who is being checked.
	 */
	bool check_self_abilities(const config& cfg, const std::string& special) const;
	/** check_adj_abilities : return an boolean value for checking of activities of abilities used like weapon
	 * @return True if the special @a special is active.
	 * @param cfg the config to one special ability checked.
	 * @param special The special ability type who is being checked.
	 * @param dir direction to research a unit adjacent to self_.
	 * @param from unit adjacent to self_ is checked.
	 */
	bool check_adj_abilities(const config& cfg, const std::string& special, int dir, const unit& from) const;
	bool special_active(const config& special, AFFECTS whom, const std::string& tag_name,
	                    const std::string& filter_self ="filter_self") const;

/** weapon_specials_impl_self and weapon_specials_impl_adj : check if special name can be added.
	 * @param[in,out] temp_string the string modified and returned
	 * @param[in] self the unit checked.
	 * @param[in] self_attack the attack used by unit checked in this function.
	 * @param[in] other_attack the attack used by opponent to unit checked.
	 * @param[in] self_loc location of the unit checked.
	 * @param[in] whom determine if unit affected or not by special ability.
	 * @param[in,out] checking_name the reference for checking if a name is already added
	 * @param[in] checking_tags the reference for checking if special ability type can be used
	 * @param[in] leader_bool If true, [leadership] abilities are checked.
	 */
	static void weapon_specials_impl_self(
		std::string& temp_string,
		unit_const_ptr self,
		const_attack_ptr self_attack,
		const_attack_ptr other_attack,
		const map_location& self_loc,
		AFFECTS whom,
		std::set<std::string>& checking_name,
		const std::set<std::string>& checking_tags={},
		bool leader_bool=false
	);

	static void weapon_specials_impl_adj(
		std::string& temp_string,
		unit_const_ptr self,
		const_attack_ptr self_attack,
		const_attack_ptr other_attack,
		const map_location& self_loc,
		AFFECTS whom,
		std::set<std::string>& checking_name,
		const std::set<std::string>& checking_tags={},
		const std::string& affect_adjacents="",
		bool leader_bool=false
	);
	/** check_self_abilities_impl : return an boolean value for checking of activities of abilities used like weapon
	 * @return True if the special @a tag_name is active.
	 * @param self_attack the attack used by unit checked in this function.
	 * @param other_attack the attack used by opponent to unit checked.
	 * @param special the config to one special ability checked.
	 * @param u the unit checked.
	 * @param loc location of the unit checked.
	 * @param whom determine if unit affected or not by special ability.
	 * @param tag_name The special ability type who is being checked.
	 * @param leader_bool If true, [leadership] abilities are checked.
	 */
	static bool check_self_abilities_impl(
		const_attack_ptr self_attack,
		const_attack_ptr other_attack,
		const config& special,
		unit_const_ptr u,
		const map_location& loc,
		AFFECTS whom,
		const std::string& tag_name,
		bool leader_bool=false
	);


	/** check_adj_abilities_impl : return an boolean value for checking of activities of abilities used like weapon in unit adjacent to fighter
	 * @return True if the special @a tag_name is active.
	 * @param self_attack the attack used by unit who fight.
	 * @param other_attack the attack used by opponent.
	 * @param special the config to one special ability checked.
	 * @param u the unit who is or not affected by an abilities owned by @a from.
	 * @param from unit adjacent to @a u is checked.
	 * @param dir direction to research a unit adjacent to @a u.
	 * @param loc location of the unit checked.
	 * @param whom determine if unit affected or not by special ability.
	 * @param tag_name The special ability type who is being checked.
	 * @param leader_bool If true, [leadership] abilities are checked.
	 */
	static bool check_adj_abilities_impl(
		const_attack_ptr self_attack,
		const_attack_ptr other_attack,
		const config& special,
		unit_const_ptr u,
		const unit& from,
		int dir,
		const map_location& loc,
		AFFECTS whom,
		const std::string& tag_name,
		bool leader_bool=false
	);

	static bool special_active_impl(
		const_attack_ptr self_attack,
		const_attack_ptr other_attack,
		const config& special,
		AFFECTS whom,
		const std::string& tag_name,
		const std::string& filter_self ="filter_self"
	);

	// Used via specials_context() to control which specials are
	// considered active.
	friend class specials_context_t;
	mutable map_location self_loc_, other_loc_;
	mutable unit_const_ptr self_;
	mutable unit_const_ptr other_;
	mutable bool is_attacker_;
	mutable const_attack_ptr other_attack_;
	mutable bool is_for_listing_ = false;
public:
	class specials_context_t {
		std::shared_ptr<const attack_type> parent;
		friend class attack_type;
		/** Initialize weapon specials context for listing */
		explicit specials_context_t(const attack_type& weapon, bool attacking);
		/** Initialize weapon specials context for a unit type */
		specials_context_t(const attack_type& weapon, const unit_type& self_type, const map_location& loc, bool attacking = true);
		/** Initialize weapon specials context for a single unit */
		specials_context_t(const attack_type& weapon, const_attack_ptr other_weapon,
			unit_const_ptr self, unit_const_ptr other,
			const map_location& self_loc, const map_location& other_loc,
			bool attacking);
		/** Initialize weapon specials context for a pair of units */
		specials_context_t(const attack_type& weapon, unit_const_ptr self, const map_location& loc, bool attacking);
		specials_context_t(const specials_context_t&) = delete;
		bool was_moved = false;
	public:
		// Destructor at least needs to be public for all this to work.
		~specials_context_t();
		specials_context_t(specials_context_t&&);
	};
	// Set up a specials context.
	// Usage: auto ctx = weapon.specials_context(...);
	specials_context_t specials_context(unit_const_ptr self, unit_const_ptr other,
		const map_location& unit_loc, const map_location& other_loc,
		bool attacking, const_attack_ptr other_attack) const {
		return specials_context_t(*this, other_attack, self, other, unit_loc, other_loc, attacking);
	}
	specials_context_t specials_context(unit_const_ptr self, const map_location& loc, bool attacking = true) const {
		return specials_context_t(*this, self, loc, attacking);
	}
	specials_context_t specials_context(const unit_type& self_type, const map_location& loc, bool attacking = true) const {
		return specials_context_t(*this, self_type, loc, attacking);
	}
	specials_context_t specials_context_for_listing(bool attacking = true) const {
		return specials_context_t(*this, attacking);
	}
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
	int damage_;
	int num_attacks_;
	double attack_weight_;
	double defense_weight_;

	int accuracy_;
	int movement_used_;
	int attacks_used_;
	int parry_;
	config specials_;
	bool changed_;
	/** Number of instances of recursion_guard that are currently allocated permission to recurse */
	mutable unsigned int num_recursion_ = 0;
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
