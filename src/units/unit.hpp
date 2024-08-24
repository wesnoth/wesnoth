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

#include "movetype.hpp"
#include "units/unit_alignments.hpp"
#include "units/id.hpp"
#include "units/ptr.hpp"
#include "units/attack_type.hpp"
#include "units/race.hpp"
#include "utils/optional_fwd.hpp"
#include "utils/variant.hpp"

#include <bitset>
#include "utils/optional_fwd.hpp"

class team;
class unit_animation_component;
class unit_formula_manager;
class vconfig;
struct color_t;

/** Data typedef for unit_ability_list. */
struct unit_ability
{
	unit_ability(const config* ability_cfg, map_location student_loc, map_location teacher_loc)
		: student_loc(student_loc)
		, teacher_loc(teacher_loc)
		, ability_cfg(ability_cfg)
	{
	}

	/**
	 * Used by the formula in the ability.
	 * The REAL location of the student (not the 'we are assuming the student is at this position' location)
	 * once unit_ability_list can contain abilities from different 'students', as it contains abilities from
	 * a unit aswell from its opponents (abilities with apply_to= opponent)
	 */
	map_location student_loc;
	/**
	 * The location of the teacher, that is the unit who owns the ability tags
	 * (different from student because of [affect_adjacent])
	 */
	map_location teacher_loc;
	/** The contents of the ability tag, never nullptr. */
	const config* ability_cfg;
};

class unit_ability_list
{
public:
	unit_ability_list(const map_location& loc = map_location()) : cfgs_() , loc_(loc) {}

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
	typedef std::vector<unit_ability>::iterator       iterator;
	typedef std::vector<unit_ability>::const_iterator const_iterator;

	iterator       begin()        { return cfgs_.begin(); }
	const_iterator begin() const  { return cfgs_.begin(); }
	iterator       end()          { return cfgs_.end();   }
	const_iterator end()   const  { return cfgs_.end();   }

	// Vector access
	bool                empty() const  { return cfgs_.empty(); }
	unit_ability&       front()        { return cfgs_.front(); }
	const unit_ability& front() const  { return cfgs_.front(); }
	unit_ability&       back()         { return cfgs_.back();  }
	const unit_ability& back()  const  { return cfgs_.back();  }
	std::size_t         size()         { return cfgs_.size();  }

	iterator erase(const iterator& erase_it)  { return cfgs_.erase(erase_it); }
	iterator erase(const iterator& first, const iterator& last)  { return cfgs_.erase(first, last); }

	template<typename... T>
	void emplace_back(T&&... args) { cfgs_.emplace_back(args...); }

	const map_location& loc() const { return loc_; }

	/** Appends the abilities from @a other to @a this, ignores other.loc() */
	void append(const unit_ability_list& other)
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
	void append_if(const unit_ability_list& other, const Predicate& predicate)
	{
		std::copy_if(other.begin(), other.end(), std::back_inserter(cfgs_ ), predicate);
	}

private:
	// Data
	std::vector<unit_ability> cfgs_;
	map_location loc_;
};

/**
 * This class represents a *single* unit of a specific type.
 */
class unit : public std::enable_shared_from_this<unit>
{
public:
	/**
	 * Clear this unit status cache for all units. Currently only the hidden
	 * status of units is cached this way.
	 */
	static void clear_status_caches();

	/** The path to the leader crown overlay. */
	static const std::string& leader_crown();

private:
	void init(const config& cfg, bool use_traits = false, const vconfig* vcfg = nullptr);

	void init(const unit_type& t, int side, bool real_unit, unit_race::GENDER gender = unit_race::NUM_GENDERS, const std::string& variation = "");

	// Copy constructor
	unit(const unit& u);

	struct unit_ctor_t {};

public:
	//private default ctor, butusing constructor to allow calling make_shared<unit> in create().
	unit(unit_ctor_t);
	unit() = delete;

private:
	enum UNIT_ATTRIBUTE
	{
		UA_MAX_HP,
		UA_MAX_MP,
		UA_MAX_AP,
		UA_MAX_XP,
		UA_IS_HEALTHY,
		UA_IS_FEARLESS,
		UA_LEVEL,
		UA_MOVEMENT_TYPE,
		UA_ZOC,
		UA_ADVANCE_TO,
		UA_ADVANCEMENTS,
		UA_ALIGNMENT,
		UA_UNDEAD_VARIATION,
		//note that UA_ATTACKS only tracks added/deleted attacks, not modified attacks.
		UA_ATTACKS,
		UA_NOTES,
		UA_PROFILE,
		UA_SMALL_PROFILE,
		UA_ABILITIES,
		UA_UPKEEP,
		UA_COUNT
	};

	void set_attr_changed(UNIT_ATTRIBUTE attr)
	{
		changed_attributes_[int(attr)] = true;
	}

	bool get_attacks_changed() const;

	bool get_attr_changed(UNIT_ATTRIBUTE attr) const
	{
		return changed_attributes_[int(attr)];
	}

	void clear_changed_attributes();

public:
	/** Initializes a unit from a config */
	static unit_ptr create(const config& cfg, bool use_traits = false, const vconfig* vcfg = nullptr)
	{
		unit_ptr res = std::make_shared<unit>(unit_ctor_t());
		res->init(cfg, use_traits, vcfg);
		return res;
	}

	/**
	 * Initializes a unit from a unit type.
	 *
	 * Only real_unit-s should have random traits, name and gender (to prevent OOS caused by RNG calls)
	 */
	static unit_ptr create(const unit_type& t, int side, bool real_unit, unit_race::GENDER gender = unit_race::NUM_GENDERS, const std::string& variation = "")
	{
		unit_ptr res = std::make_shared<unit>(unit_ctor_t());
		res->init(t, side, real_unit, gender, variation);

		return res;
	}

	unit_ptr clone() const
	{
		return std::shared_ptr<unit>(new unit(*this));
	}

	virtual ~unit();

	unit& operator=(const unit&) = delete;

	/**
	 * @defgroup unit_advance Advancement functions
	 * @{
	 */

	/** Advances this unit to another type */
	void advance_to(const unit_type& t, bool use_traits = false);

	using advances_to_t = std::vector<std::string>;
	/**
	 * Gets the possible types this unit can advance to on level-up.
	 *
	 * @returns                   A list of type IDs this unit may advance to.
	 */
	const advances_to_t& advances_to() const
	{
		return advances_to_;
	}

	/**
	 * Gets the names of the possible types this unit can advance to on level-up.
	 *
	 * @returns                   A list of the names of the types this unit may advance to.
	 */
	const std::vector<std::string> advances_to_translated() const;

	/**
	 * Sets this unit's advancement options.
	 *
	 * @param advances_to         A list of new type IDs this unit may advance to.
	 */
	void set_advances_to(const std::vector<std::string>& advances_to);

	/**
	 * Checks whether this unit has any options to advance to.
	 *
	 * This considers both whether it has types to advance to OR whether any modifications
	 * specify non-type advancement options.
	 *
	 * Note this does not consider unit experience at all, it only checks option availability.
	 * See @ref advances if an experience check is necessary.
	 */
	bool can_advance() const
	{
		return !advances_to_.empty() || !get_modification_advances().empty();
	}

	/**
	 * Checks whether this unit is eligible for level-up.
	 *
	 * @retval true              This unit has sufficient experience to level up and has advancement
	 *                           options available.
	 */
	bool advances() const
	{
		return experience_ >= max_experience() && can_advance();
	}

	/**
	 * Gets and image path and and associated description for each advancement option.
	 *
	 * Covers both type and modification-based advancements.
	 *
	 * @returns                  A data map, in image/description format. If the option is a unit type,
	 *                           advancement, the key is the type's image and the value the type ID.
	 *
	 *                           If the option is a modification, the key and value are set from config data
	 *                           (see @ref get_modification_advances).
	 */
	std::map<std::string, std::string> advancement_icons() const;

	/**
	 * Gets any non-typed advanced options set by modifications.
	 *
	 * These are usually used to give a unit special advancement options that don't invole transforming to a
	 * new type.
	 *
	 * Note this is not the raw option data. Parsing is performed to ensure each option appears only once.
	 * Use @ref modification_advancements is the raw data is needed.
	 *
	 * @returns                  A config list of options data. Each option is unique.
	 */
	std::vector<config> get_modification_advances() const;

	/**
	 * Gets the image and description data for modification advancements.
	 *
	 * @returns                  A list of pairs of the image paths(first) and descriptions (second) for
	 *                           each advancement option.
	 */
	std::vector<std::pair<std::string, std::string>> amla_icons() const;

	/** The raw, unparsed data for modification advancements. */
	const std::vector<config>& modification_advancements() const
	{
		return advancements_;
	}

	/** Sets the raw modification advancement option data */
	void set_advancements(std::vector<config> advancements);

	/**
	 * @}
	 * @defgroup unit_access Basic data setters and getters
	 * @{
	 **/

public:
	/**
	 * The side this unit belongs to.
	 *
	 * Note that side numbers starts from 1, not 0, so be sure to subtract 1 if using as a container index.
	 */
	int side() const
	{
		return side_;
	}

	/** Sets the side this unit belongs to. */
	void set_side(unsigned int new_side)
	{
		side_ = new_side;
	}

	/** This unit's type, accounting for gender and variation. */
	const unit_type& type() const
	{
		return *type_;
	}

	/**
	 * The id of this unit's type.
	 *
	 * If you are dealing with creating units (e.g. recruitment), this is not what you want, as a
	 * variation can change this; use type().parent_id() instead.
	 */
	const std::string& type_id() const;

	/** Gets the translatable name of this unit's type. */
	const t_string& type_name() const
	{
		return type_name_;
	}

	/**
	 * Gets this unit's id.
	 *
	 * This is a unique string usually set by WML. It should *not* be used for internal tracking in
	 * the unit_map. Use @ref underlying_id for that.
	 */
	const std::string& id() const
	{
		return id_;
	}

	/** Sets this unit's string ID. */
	void set_id(const std::string& id)
	{
		id_ = id;
	}

	/** This unit's unique internal ID. This should *not* be used for user-facing operations. */
	std::size_t underlying_id() const
	{
		return underlying_id_.value;
	}

private:
	/** Sets the internal ID. */
	void set_underlying_id(n_unit::id_manager& id_manager);

public:
	/** Gets this unit's translatable display name. */
	const t_string& name() const
	{
		return name_;
	}

	/**
	 * Sets this unit's translatable display name.
	 *
	 * This should only be used internally since it ignores the 'unrenamable' flag.
	 */
	void set_name(const t_string& name)
	{
		name_ = name;
	}

	/**
	 * Attempts to rename this unit's translatable display name, taking the 'unrenamable' flag into account.
	 *
	 * If a direct rename is desired, use @ref set_name.
	 * @todo should this also take a t_string?
	 */
	void rename(const std::string& name)
	{
		if(!unrenamable_) {
			name_ = name;
		}
	}

	/**
	 * Whether this unit can be renamed.
	 *
	 * This flag is considered by @ref rename, but not @ref set_name.
	 */
	bool unrenamable() const
	{
		return unrenamable_;
	}

	/**
	 * Sets the 'unrenamable' flag. Usually used for scenario-specific units which should not be renamed.
	 */
	void set_unrenamable(bool unrenamable)
	{
		unrenamable_ = unrenamable;
	}

	/** A detailed description of this unit. */
	t_string unit_description() const
	{
		return description_;
	}

	/** A detailed description of this unit. */
	void set_unit_description(const t_string& new_desc)
	{
		description_ = new_desc;
	}

	/** The unit's special notes. */
	std::vector<t_string> unit_special_notes() const;

	/** The gender of this unit. */
	unit_race::GENDER gender() const
	{
		return gender_;
	}

	/**
	 * The alignment of this unit.
	 *
	 * This affects the time of day during which this unit's attacks do the most damage.
	 */
	unit_alignments::type alignment() const
	{
		return alignment_;
	}

	/** Sets the alignment of this unit. */
	void set_alignment(unit_alignments::type alignment)
	{
		set_attr_changed(UA_ALIGNMENT);
		alignment_ = alignment;
	}

	/**
	 * Gets this unit's race.
	 *
	 * @returns                  A pointer to a unit_race object - never nullptr, but it may point
	 *                           to the null race.
	 */
	const unit_race* race() const
	{
		return race_;
	}

	/** The current number of hitpoints this unit has. */
	int hitpoints() const
	{
		return hit_points_;
	}

	/** The max number of hitpoints this unit can have. */
	int max_hitpoints() const
	{
		return max_hit_points_;
	}

	void set_max_hitpoints(int value)
	{
		set_attr_changed(UA_MAX_HP);
		max_hit_points_ = value;
	}

	/** Sets the current hitpoint amount. */
	void set_hitpoints(int hp)
	{
		hit_points_ = hp;
	}

	/** The current number of experience points this unit has. */
	int experience() const
	{
		return experience_;
	}

	/** The max number of experience points this unit can have. */
	int max_experience() const
	{
		return max_experience_;
	}

	void set_max_experience(int value)
	{
		set_attr_changed(UA_MAX_XP);
		max_experience_ = value;
	}

	/** The number of experience points this unit needs to level up, or 0 if current XP > max XP. */
	unsigned int experience_to_advance() const
	{
		return std::max(0, max_experience_ - experience_);
	}

	/** The number of experience points over max this unit has, or 0 if current XP < max XP. */
	unsigned int experience_overflow() const
	{
		return std::max(0, experience_ - max_experience_);
	}

	/** Sets the current experience point amount. */
	void set_experience(int xp)
	{
		experience_ = xp;
	}

	/** The current level of this unit. */
	int level() const
	{
		return level_;
	}

	/** Sets the current level of this unit. */
	void set_level(int level)
	{
		set_attr_changed(UA_LEVEL);
		level_ = level;
	}

	/** The ID of the variation of this unit's type. */
	const std::string& variation() const
	{
		return variation_;
	}

	/** The ID of the undead variation (ie, dwarf, swimmer) of this unit. */
	void set_undead_variation(const std::string& value)
	{
		set_attr_changed(UA_UNDEAD_VARIATION);
		undead_variation_ = value;
	}
	const std::string& undead_variation() const
	{
		return undead_variation_;
	}

	/**
	 * An optional profile image to display in Help.
	 *
	 * @returns                   The specified image, this unit's type's sprite image if empty
	 *                            or 'unit_image' was set.
	 */
	std::string small_profile() const;

	void set_small_profile(const std::string& value)
	{
		set_attr_changed(UA_SMALL_PROFILE);
		small_profile_ = value;
	}
	/**
	 * An optional profile image displays when this unit is 'speaking' via [message].
	 *
	 * @returns                   The specified image, this unit's type's sprite image if empty
	 *                            or 'unit_image' was set.
	 */
	std::string big_profile() const;

	void set_big_profile(const std::string& value);

	/** Whether this unit can recruit other units - ie, are they a leader unit. */
	bool can_recruit() const
	{
		return canrecruit_;
	}

	/** Sets whether this unit can recruit other units. */
	void set_can_recruit(bool canrecruit)
	{
		canrecruit_ = canrecruit;
	}

	/** The type IDs of the other units this unit may recruit, if possible. */
	const std::vector<std::string>& recruits() const
	{
		return recruit_list_;
	}

	/** Sets the recruit list. */
	void set_recruits(const std::vector<std::string>& recruits);

	/** How much gold is required to recruit this unit. */
	int cost() const
	{
		return unit_value_;
	}

	/** How much gold it costs to recall this unit, or -1 if the side's default
	 * recall cost is used. */
	int recall_cost() const
	{
		return recall_cost_;
	}

	/** Sets the cost of recalling this unit. */
	void set_recall_cost(int recall_cost)
	{
		recall_cost_ = recall_cost;
	}

	/** Gets the filter constraints upon which units this unit may recall, if able. */
	const config& recall_filter() const
	{
		return filter_recall_;
	}

	/** Sets the filter constraints upon which units this unit may recall, if able. */
	void set_recall_filter(const config& filter)
	{
		filter_recall_ = filter;
	}

	/**
	 * Gets this unit's role.
	 *
	 * A role is a special string flag usually used to represent a unit's purpose in a scenario.
	 * It can be filtered on.
	 */
	const std::string& get_role() const
	{
		return role_;
	}

	/** Sets a unit's role */
	void set_role(const std::string& role)
	{
		role_ = role;
	}

	/**
	 * Gets this unit's usage. This is relevant to the AI.
	 *
	 * Usage refers to how the AI may consider utilizing this unit in combat.
	 * @todo document further
	 */
	std::string usage() const
	{
		return usage_.value_or("");
	}

	/** Sets this unit's usage. */
	void set_usage(const std::string& usage)
	{
		usage_ = usage;
	}

	/**
	 * Gets any user-defined variables this unit 'owns'.
	 *
	 * These are accessible via WML if the unit's data is serialized to a variable. They're strictly
	 * user-facing; internal engine calculations shouldn't use this.
	 */
	config& variables()
	{
		return variables_;
	}

	/** Const overload of @ref variables. */
	const config& variables() const
	{
		return variables_;
	}

	/**
	 * Gets whether this unit is currently hidden on the map.
	 *
	 * Hidden units are not drawn on the main map or the minimap. They are
	 * an implementation detail. For the [hides] ability, see invisible().
	 */
	bool get_hidden() const
	{
		return hidden_;
	}

	/** Sets whether the unit is hidden on the map. */
	void set_hidden(bool state) const;

	/**
	 * The factor by which the HP bar should be scaled.
	 * @todo: document further
	 */
	double hp_bar_scaling() const
	{
		return hp_bar_scaling_;
	}

	/**
	 * The factor by which the XP bar should be scaled.
	 * @todo: document further
	 */
	double xp_bar_scaling() const
	{
		return xp_bar_scaling_;
	}

	/**
	 * Whether the unit has been instructed to hold its position.
	 * This excludes it from the unit cycling function.
	 * @return true if it is holding position
	 */
	bool hold_position() const
	{
		return hold_position_;
	}

	/**
	 * Toggle the unit's hold position status.
	 */
	void toggle_hold_position()
	{
		hold_position_ = !hold_position_;
		if(hold_position_) {
			end_turn_ = true;
		}
	}

	/**
	 * Set whether the user ended their turn
	 * @todo Verify meaning and explain better
	 */
	void set_user_end_turn(bool value = true)
	{
		end_turn_ = value;
	}

	/**
	 * Toggle whether the user ended their turn
	 * @todo Verify meaning and explain better
	 */
	void toggle_user_end_turn()
	{
		end_turn_ = !end_turn_;
		if(!end_turn_) {
			hold_position_ = false;
		}
	}

	/**
	 * Check whether the user ended their turn
	 * @todo Verify meaning and explain better
	 */
	bool user_end_turn() const
	{
		return end_turn_;
	}

	/**
	 * Refresh unit for the beginning of a turn
	 */
	void new_turn();

	/**
	 * Refresh unit for the end of a turn
	 */
	void end_turn();

	/**
	 * Refresh unit for the beginning of a new scenario
	 */
	void new_scenario();

	/**
	 * Damage the unit.
	 * @returns true if the unit dies as a result
	 */
	bool take_hit(int damage)
	{
		hit_points_ -= damage;
		return hit_points_ <= 0;
	}

	/**
	 * Heal the unit
	 * @param amount The number of hitpoints to gain
	 */
	void heal(int amount);

	/**
	 * Fully heal the unit, restoring it to max hitpoints
	 */
	void heal_fully()
	{
		hit_points_ = max_hitpoints();
	}

	/**
	 * Get the status effects currently affecting the unit.
	 * @return A set of status keys
	 */
	const std::set<std::string> get_states() const;

	/**
	 * Check if the unit is affected by a status effect
	 * @param state The status effect to check
	 * @returns true if the unit is affected by the status effect
	 */
	bool get_state(const std::string& state) const;

	/**
	 * Set whether the unit is affected by a status effect
	 * @param state The status effect to change
	 * @param value Whether the unit should be affected by the status
	 */
	void set_state(const std::string& state, bool value);

	/**
	 * Built-in status effects known to the engine
	 */
	enum state_t {
		STATE_SLOWED = 0,   /** The unit is slowed - it moves slower and does less damage */
		STATE_POISONED,     /** The unit is poisoned - it loses health each turn */
		STATE_PETRIFIED,    /** The unit is petrified - it cannot move or be attacked */
		STATE_UNCOVERED,    /** The unit is uncovered - it was hiding but has been spotted */
		STATE_NOT_MOVED,    /** The unit has not moved @todo Explain better */
		STATE_UNHEALABLE,   /** The unit cannot be healed */
		STATE_GUARDIAN,     /** The unit is a guardian - it won't move unless a target is sighted */
		STATE_INVULNERABLE, /** The unit is invulnerable - it cannot be hit by any attack */
		NUMBER_OF_STATES,   /** To set the size of known_boolean_states_ */
		STATE_UNKNOWN = -1  /** A status effect not known to the engine */
	};

	/**
	 * Set whether the unit is affected by a status effect
	 * @param state The status effect to change
	 * @param value Whether the unit should be affected by the status
	 */
	void set_state(state_t state, bool value);

	/**
	 * Check if the unit is affected by a status effect
	 * @param state The status effect to check
	 * @returns true if the unit is affected by the status effect
	 */
	bool get_state(state_t state) const;

	/**
	 * Convert a string status effect ID to a built-in status effect ID
	 * @returns the state_t representing the status, or STATE_UNKNOWN if it's not built-in
	 */
	static state_t get_known_boolean_state_id(const std::string& state);

	/**
	 * Check if the unit has been poisoned
	 * @returns true if it's poisoned
	 */
	bool poisoned() const
	{
		return get_state(STATE_POISONED);
	}

	/**
	 * Check if the unit has been petrified
	 * @returns true if it's petrified
	 */
	bool incapacitated() const
	{
		return get_state(STATE_PETRIFIED);
	}

	/**
	 * Check if the unit has been slowed
	 * @returns true if it's slowed
	 */
	bool slowed() const
	{
		return get_state(STATE_SLOWED);
	}

	/**
	 * @}
	 * @defgroup unit_atk Attack and resistance functions
	 * @{
	 */

public:
	/** Gets an iterator over this unit's attacks. */
	attack_itors attacks()
	{
		return make_attack_itors(attacks_);
	}

	/** Const overload of @ref attacks. */
	const_attack_itors attacks() const
	{
		return make_attack_itors(attacks_);
	}

	/**
	 * Adds a new attack to the unit.
	 * @param position An iterator pointing to the attack before which to insert the new one.
	 * @param args The arguments for constructing the attack
	 */
	template<typename... Args>
	attack_ptr add_attack(attack_itors::iterator position, Args&&... args)
	{
		set_attr_changed(UA_ATTACKS);
		return *attacks_.emplace(position.base(), new attack_type(std::forward<Args>(args)...));
	}

	/**
	 * Remove an attack from the unit
	 * @param atk A pointer to the attack to remove
	 * @return true if the attack was removed, false if it didn't exist on the unit
	 */
	bool remove_attack(attack_ptr atk);

	/**
	 * Set the unit to have no attacks left for this turn.
	 */
	void remove_attacks_ai();

	/**
	 * Calculates the damage this unit would take from a certain attack.
	 *
	 * @param attack              The attack to consider.
	 * @param attacker            Whether this unit should be considered the attacker.
	 * @param loc                 The unit's location (to resolve [resistance] abilities)
	 * @param weapon              The weapon to check for any abilities or weapon specials
	 *
	 * @returns                   The expected damage.
	 */
	int damage_from(const attack_type& attack, bool attacker, const map_location& loc, const_attack_ptr weapon = nullptr) const
	{
		return resistance_against(attack, attacker, loc, weapon);
	}

	/** The maximum number of attacks this unit may perform per turn, usually 1. */
	int max_attacks() const
	{
		return max_attacks_;
	}

	void set_max_attacks(int value)
	{
		set_attr_changed(UA_MAX_AP);
		max_attacks_ = value;
	}

	/**
	 * Gets the remaining number of attacks this unit can perform this turn.
	 *
	 * If the 'incapacitated' status is set, this will always be 0.
	 */
	int attacks_left() const
	{
		return (attacks_left_ == 0 || incapacitated()) ? 0 : attacks_left_;
	}

	/**
	 * Gets the remaining number of attacks this unit can perform this turn.
	 *
	 * @param base_value          If false, consider the `incapacitated` flag.
	 *
	 * @returns                   If @a base_value is true, the raw value is returned.
	 */
	int attacks_left(bool base_value) const
	{
		return base_value ? attacks_left_ : attacks_left();
	}

	/**
	 * Sets the number of attacks this unit has left this turn.
	 * @param left The number of attacks left
	 */
	void set_attacks(int left)
	{
		attacks_left_ = std::max<int>(0, left);
	}

	/**
	 * The unit's defense on a given terrain
	 * @param terrain The terrain to check
	 */
	int defense_modifier(const t_translation::terrain_code& terrain) const;

	/**
	 * For the provided list of resistance abilities, determine the damage resistance based on which are active and any max_value that's present.
	 *
	 * @param resistance_list A list of resistance abilities that the unit has.
	 * @param damage_name The name of the damage type, for example "blade".
	 * @return The resistance value for a unit with the provided resistance abilities to the provided damage type.
	 */
	int resistance_value(unit_ability_list resistance_list, const std::string& damage_name) const;

	/**
	 * The unit's resistance against a given damage type
	 * @param damage_name The damage type
	 * @param attacker True if this unit is on the offensive (to resolve [resistance] abilities)
	 * @param loc The unit's location (to resolve [resistance] abilities)
	 * @param weapon The weapon to check for any abilities or weapon specials
	 * @param opp_weapon The opponent's weapon to check for any abilities or weapon specials
	 */
	int resistance_against(const std::string& damage_name, bool attacker, const map_location& loc, const_attack_ptr weapon = nullptr, const_attack_ptr opp_weapon = nullptr) const;

	/**
	 * The unit's resistance against a given attack
	 * @param atk The attack
	 * @param attacker True if this unit is on the offensive (to resolve [resistance] abilities)
	 * @param loc The unit's location (to resolve [resistance] abilities)
	 * @param weapon The weapon to check for any abilities or weapon specials
	 */
	int resistance_against(const attack_type& atk, bool attacker, const map_location& loc, const_attack_ptr weapon = nullptr) const
	{
		return resistance_against(atk.type(), attacker, loc , weapon, atk.shared_from_this());
	}

	/** Gets resistances without any abilities applied. */
	utils::string_map_res get_base_resistances() const
	{
		return movement_type_.damage_table();
	}

private:
	bool resistance_filter_matches(const config& cfg, const std::string& damage_name, int res) const;

	/**
	 * @}
	 * @defgroup unit_trait Trait and upkeep functions
	 * @{
	 */
public:
	/**
	 * Applies mandatory traits (e.g. undead, mechanical) to a unit and then fills in the remaining traits
	 * traits until no more are available (leaders have a restricted set of available traits) or the unit has
	 * its maximum number of traits.
	 *
	 * This routine does not apply the effects of added traits to a unit; that must be done by the caller.
	 *
	 * Note that random numbers used in config files don't work in multiplayer, so leaders should be barred
	 * from all random traits until that is fixed. Later the restrictions will be based on play balance.
	 *
	 * @param must_have_only      Whether random or optional traits should be included or not. If false only
	 *                            mandatory traits will be used.
	 */
	void generate_traits(bool must_have_only = false);

	/**
	 * Gets the names of the currently registered traits.
	 *
	 * @returns                   A list of translatable trait names.
	 */
	const std::vector<t_string>& trait_names() const
	{
		return trait_names_;
	}

	/**
	 * Gets the descriptions of the currently registered traits.
	 *
	 * @returns                   A list of translatable trait descriptions.
	 */
	const std::vector<t_string>& trait_descriptions() const
	{
		return trait_descriptions_;
	}

	/**
	 * Gets the ids of the traits corresponding to those returned by trait_names() and
	 * trait_descriptions(). Omits hidden traits, which are those with an empty name.
	 *
	 * @returns                   A list of trait IDs.
	 */
	std::vector<std::string> trait_nonhidden_ids() const
	{
		return trait_nonhidden_ids_;
	}

	/** Gets a list of the modification this unit currently has.
	 * @param mod_type type of modification.
	 * @returns                   A list of modification IDs.
	 */
	std::vector<std::string> get_modifications_list(const std::string& mod_type) const;

	/**
	 * Gets a list of the traits this unit currently has, including hidden traits.
	 *
	 * @returns                   A list of trait IDs.
	 */
	std::vector<std::string> get_traits_list() const
	{
		return get_modifications_list("trait");
	}

	std::vector<std::string> get_objects_list() const
	{
		return get_modifications_list("object");
	}

	std::vector<std::string> get_advancements_list() const
	{
		return get_modifications_list("advancement");
	}

	/**
	 * Register a trait's name and its description for the UI's use.
	 *
	 * The resulting data can be fetched with @ref trait_names and @ref trait_descriptions.
	 *
	 * @param trait               A config containing the trait's attributes.
	 * @param description         The translatable description of the trait.
	 */
	void add_trait_description(const config& trait, const t_string& description);

	/**
	 * Gets the amount of gold this unit costs a side per turn.
	 *
	 * This fetches an actual numeric gold value:
	 * - If can_recruit is true, no upkeep is paid (0 is returned).
	 * - If a special upkeep flag is set, the associated gold amount is returned (see @ref upkeep_value_visitor).
	 * - If a numeric value is already set, it is returned directly.
	 *
	 * @returns                   A gold value, evaluated based on the state of @ref upkeep_raw.
	 */
	int upkeep() const;

	struct upkeep_full
	{
		static std::string type() { static std::string v = "full"; return v; }
	};

	struct upkeep_loyal
	{
		static std::string type() { static std::string v = "loyal"; return v; }
	};

	using upkeep_t = utils::variant<upkeep_full, upkeep_loyal, int>;

	/** Visitor helper class to fetch the appropriate upkeep value. */
	class upkeep_value_visitor
#ifdef USING_BOOST_VARIANT
		: public boost::static_visitor<int>
#endif
	{
	public:
		explicit upkeep_value_visitor(const unit& unit) : u_(unit) {}

		/** Full upkeep equals the unit's level. */
		int operator()(const upkeep_full&) const
		{
			return u_.level();
		}

		/** Loyal units cost no upkeep. */
		int operator()(const upkeep_loyal&) const
		{
			return 0;
		}

		int operator()(int v) const
		{
			return v;
		}

	private:
		const unit& u_;
	};

	/** Visitor helper struct to fetch the upkeep type flag if applicable, or the the value otherwise. */
	struct upkeep_type_visitor
#ifdef USING_BOOST_VARIANT
		: public boost::static_visitor<std::string>
#endif
	{
		template<typename T>
		std::enable_if_t<!std::is_same_v<int, T>, std::string>
		operator()(T&) const
		{
			// Any special upkeep type should have an associated @ref type getter in its helper struct.
			return T::type();
		}

		std::string operator()(int v) const
		{
			return std::to_string(v);
		}
	};

	/** Visitor helper class to parse the upkeep value from a config. */
	class upkeep_parser_visitor
#ifdef USING_BOOST_VARIANT
		: public boost::static_visitor<upkeep_t>
#endif
	{
	public:
		template<typename N>
		std::enable_if_t<std::is_arithmetic_v<N>, upkeep_t>
		operator()(N n) const
		{
			if(n == 0) return upkeep_loyal();
			if(n < 0) throw std::invalid_argument(std::to_string(n));
			return static_cast<int>(n);
		}

		template<typename B>
		std::enable_if_t<std::is_convertible_v<B, bool> && !std::is_arithmetic_v<B>, upkeep_t>
		operator()(B b) const
		{
			throw std::invalid_argument(b.str());
		}

		upkeep_t operator()(utils::monostate) const
		{
			return upkeep_full();
		}

		upkeep_t operator()(const std::string& s) const
		{
			if(s == "loyal" || s == "free")
				return upkeep_loyal();
			if(s == "full")
				return upkeep_full();
			throw std::invalid_argument(s);
		}
	};

	/**
	 * Gets the raw variant controlling the upkeep value.
	 *
	 * This should not usually be called directly. To get an actual numeric value of upkeep use @ref upkeep.
	 */
	upkeep_t upkeep_raw() const
	{
		return upkeep_;
	}

	/** Sets the upkeep value to a specific value value. Does not necessarily need to be numeric */
	void set_upkeep(upkeep_t v)
	{
		upkeep_ = v;
	}

	/** Gets whether this unit is loyal - ie, it costs no upkeep. */
	bool loyal() const;

	void set_loyal(bool loyal);

	/** Gets whether this unit is fearless - ie, unaffected by time of day. */
	bool is_fearless() const
	{
		return is_fearless_;
	}

	/** Gets whether this unit is healthy - ie, always rest heals. */
	bool is_healthy() const
	{
		return is_healthy_;
	}

	/**
	 * @}
	 * @defgroup unit_mvmt Movement and location functions
	 * @{
	 */

public:
	/** The maximum moves this unit has. */
	int total_movement() const
	{
		return max_movement_;
	}

	void set_total_movement(int value)
	{
		set_attr_changed(UA_MAX_MP);
		max_movement_ = value;
	}

	/**
	 * Gets how far a unit can move, considering the `incapacitated` flag.
	 *
	 * @returns                   The remaining movement, or zero if incapacitated.
	 */
	int movement_left() const
	{
		return (movement_ == 0 || incapacitated()) ? 0 : movement_;
	}

	/**
	 * Gets how far a unit can move.
	 *
	 * @param base_value          If false, consider the `incapacitated` flag.
	 *
	 * @returns                   If @a base_value is true, the raw value is returned.
	 */
	int movement_left(bool base_value) const
	{
		return base_value ? movement_ : movement_left();
	}

	/**
	 * Set this unit's remaining movement to @a moves.
	 *
	 * This does not affect maximum movement.
	 *
	 * @param moves               The new number of moves
	 * @param unit_action         If to true, the "end turn" and "hold position" flags will be cleared
	 *                            (as they should be if a unit acts, as opposed to the movement being set
	 *                            by the engine for other reasons).
	 */
	void set_movement(int moves, bool unit_action = false);

	/** Checks if this unit has moved. */
	bool has_moved() const
	{
		return movement_left() != total_movement();
	}

	/** Sets the unit to have no moves left for this turn. */
	void remove_movement_ai();

	/**
	 * Checks whether this unit is 'resting'.
	 *
	 * Resting refers to whether this unit has not moved yet this turn. Note that this can be true even
	 * if @ref movement_left is not equal to @ref total_movement.
	 */
	bool resting() const
	{
		return resting_;
	}

	/** Sets this unit's resting status. */
	void set_resting(bool rest)
	{
		resting_ = rest;
	}

	/** Tests whether the unit has a zone-of-control, considering @ref incapacitated. */
	bool emits_zoc() const
	{
		return emit_zoc_  && !incapacitated();
	}

	/** Gets the raw zone-of-control flag, disregarding @ref incapacitated. */
	bool get_emit_zoc() const
	{
		return emit_zoc_;
	}

	/** Sets the raw zone-of-control flag. */
	void set_emit_zoc(bool val)
	{
		set_attr_changed(UA_ZOC);
		emit_zoc_ = val;
	}

	/** The current map location this unit is at. */
	const map_location& get_location() const
	{
		return loc_;
	}

	/**
	 * Sets this unit's map location.
	 *
	 * Note this should only be called by unit_map or for temporary units.
	 */
	void set_location(const map_location& loc)
	{
		loc_ = loc;
	}

	/** The current direction this unit is facing within its hex. */
	map_location::DIRECTION facing() const
	{
		return facing_;
	}

	/** The this unit's facing. */
	void set_facing(map_location::DIRECTION dir) const;

	/** Gets whether this unit has a multi-turn destination set. */
	bool has_goto() const
	{
		return get_goto().valid();
	}

	/** The map location to which this unit is moving over multiple turns, if any. */
	const map_location& get_goto() const
	{
		return goto_;
	}

	/** Sets this unit's long term destination. */
	void set_goto(const map_location& new_goto)
	{
		goto_ = new_goto;
	}

	/** Gets the unit's vision points. */
	int vision() const
	{
		return vision_ < 0 ? max_movement_ : vision_;
	}

	/** Gets the unit's jamming points. */
	int jamming() const
	{
		return jamming_;
	}

	/** Check whether the unit's move has been interrupted. */
	bool move_interrupted() const
	{
		return movement_left() > 0 && interrupted_move_.x >= 0 && interrupted_move_.y >= 0;
	}

	/** Get the target location of the unit's interrupted move. */
	const map_location& get_interrupted_move() const
	{
		return interrupted_move_;
	}

	/** Set the target location of the unit's interrupted move. */
	void set_interrupted_move(const map_location& interrupted_move)
	{
		interrupted_move_ = interrupted_move;
	}

	/** Get the unit's movement type. */
	const movetype& movement_type() const
	{
		return movement_type_;
	}

	/**
	 * Get the unit's movement cost on a particular terrain
	 * @param terrain The terrain to check
	 * @returns the number of movement points to enter that terrain
	 */
	int movement_cost(const t_translation::terrain_code& terrain) const
	{
		return movement_type_.movement_cost(terrain, get_state(STATE_SLOWED));
	}

	/**
	 * Get the unit's vision cost on a particular terrain
	 * @param terrain The terrain to check
	 * @returns the number of vision points to see into that terrain
	 */
	int vision_cost(const t_translation::terrain_code& terrain) const
	{
		return movement_type_.vision_cost(terrain, get_state(STATE_SLOWED));
	}

	/**
	 * Get the unit's jamming cost on a particular terrain
	 * @param terrain The terrain to check
	 * @returns the number of jamming points to jam that terrain
	 */
	int jamming_cost(const t_translation::terrain_code& terrain) const
	{
		return movement_type_.jamming_cost(terrain, get_state(STATE_SLOWED));
	}

	/** Check if the unit is a flying unit. */
	bool is_flying() const
	{
		return movement_type_.is_flying();
	}

	/**
	 * @}
	 * @defgroup unit_mod Modification functions
	 * @{
	 */

public:
	/** Get the raw modifications. */
	config& get_modifications()
	{
		return modifications_;
	}

	/** Set the raw modifications. */
	const config& get_modifications() const
	{
		return modifications_;
	}

	/**
	 * Count modifications of a particular type.
	 * @param type The type of modification to count.
	 *             Valid values are "advancement", "trait", "object"
	 * @param id The ID of the modification to count
	 * @return The total number of modifications of that type and ID.
	 */
	std::size_t modification_count(const std::string& type, const std::string& id) const;

	/**
	 * Count modifications of a particular type.
	 * @param type The type of modification to count.
	 *             Valid values are "advancement", "trait", "object"
	 * @return The total number of modifications of that type.
	 */
	std::size_t modification_count(const std::string& type) const;

	std::size_t traits_count() const
	{
		return modification_count("trait");
	}

	std::size_t objects_count() const
	{
		return modification_count("object");
	}

	std::size_t advancements_count() const
	{
		return modification_count("advancement");
	}

	/**
	 * Add a new modification to the unit.
	 * @param type The type of modification to add.
	 *             Valid values are "advancement", "trait", "object"
	 * @param modification The details of the modification
	 * @param no_add If true, apply the modification but don't save it for unit rebuild time.
	 *               Defaults to false.
	 */
	void add_modification(const std::string& type, const config& modification, bool no_add = false);

	/**
	 * Clears those modifications whose duration has expired.
	 *
	 * @param duration            If empty, all temporary modifications (those not lasting forever) expire.
	 *                            Otherwise, modifications whose duration equals @a duration expire.
	 */
	void expire_modifications(const std::string& duration);

	static const std::set<std::string> builtin_effects;

	/**
	 * Apply a builtin effect to the unit.
	 * @param type The effect to apply. Must be one of the effects in @ref builtin_effects.
	 * @param effect The details of the effect
	 */
	void apply_builtin_effect(std::string type, const config& effect);

	/**
	 * Construct a string describing a built-in effect.
	 * @param type The effect to describe. Must be one of the effects in @ref builtin_effects.
	 * @param effect The details of the effect
	 */
	std::string describe_builtin_effect(std::string type, const config& effect);

	/** Re-apply all saved modifications. */
	void apply_modifications();

	/**
	 * @}
	 * @defgroup unit_img Image and animations functions
	 * @{
	 */

public:
	/** @todo Document this */
	unit_animation_component& anim_comp() const
	{
		return *anim_comp_;
	}

	/** The name of the file to game_display (used in menus). */
	std::string absolute_image() const;

	/** The default image to use for animation frames with no defined image. */
	std::string default_anim_image() const;

	/** Get the unit's halo image. */
	std::string image_halo() const
	{
		return halo_.value_or("");
	}

	const std::vector<std::string> halo_or_icon_abilities(const std::string& image_type) const;

	/** Get the [halo] abilities halo image(s). */
	const std::vector<std::string> halo_abilities() const
	{
		return halo_or_icon_abilities("halo");
	}

	/** Set the unit's halo image. */
	void set_image_halo(const std::string& halo);

	/** Get the unit's ellipse image. */
	std::string image_ellipse() const
	{
		return ellipse_.value_or("");
	}

	/** Set the unit's ellipse image. */
	void set_image_ellipse(const std::string& ellipse)
	{
		appearance_changed_ = true;
		ellipse_ = ellipse;
	}

	/**
	 * Get the source color palette to use when recoloring the unit's image.
	 */
	const std::string& flag_rgb() const;

	/** Constructs a recolor (RC) IPF string for this unit's team color. */
	std::string TC_image_mods() const;

	/** Gets any IPF image mods applied by effects. */
	const std::string& effect_image_mods() const
	{
		return image_mods_;
	}

	/**
	 * Gets an IPF string containing all IPF image mods.
	 *
	 * @returns                 An amalgamation of @ref effect_image_mods followed by @ref TC_image_mods.
	 */
	std::string image_mods() const;

	/** Get the unit's overlay images. */
	const std::vector<std::string>& overlays() const
	{
		return overlays_;
	}

	/** Get the [overlay] ability overlay images. */
	const std::vector<std::string> overlays_abilities() const
	{
		return halo_or_icon_abilities("overlay");
	}
	/**
	 * Color for this unit's *current* hitpoints.
	 *
	 * @returns                   A color between green and red representing how wounded this unit is.
	 *                            The maximum_hitpoints are considered as base.
	 */
	color_t hp_color() const;
	static color_t hp_color_max();

	/**
	 * Color for this unit's hitpoints.
	 *
	 * @param hitpoints           The number of hitpoints the color represents.
	 * @returns                   The color considering the current hitpoints as base.
	 */
	color_t hp_color(int hitpoints) const;

	/**
	 * Color for this unit's XP. See also @ref hp_color
	 */
	color_t xp_color() const;
	static color_t xp_color(int xp_to_advance, bool can_advance, bool has_amla);

	/**
	 * @}
	 * @defgroup unit_abil Ability functions
	 * @{
	 */

public:
	/**
	 * Checks whether this unit currently possesses or is affected by a given ability.
	 *
	 * This means that the ability could be owned by this unit itself or by an adjacent unit, should
	 * the ability affect an AoE in which this unit happens to be.
	 *
	 * @param tag_name            The name of the ability to check for.
	 * @param loc                 The location around which to check for affected units. This may or
	 *                            may not be the location of this unit.
	 */
	bool get_ability_bool(const std::string& tag_name, const map_location& loc) const;

	/**
	 * Checks whether this unit currently possesses or is affected by a given ability.
	 *
	 * This means that the ability could be owned by this unit itself or by an adjacent unit, should
	 * the ability affect an AoE in which this unit happens to be.
	 *
	 * This overload uses the location of this unit for calculations.
	 *
	 * @param tag_name            The name of the ability to check for.
	 */
	bool get_ability_bool(const std::string& tag_name) const
	{
		return get_ability_bool(tag_name, loc_);
	}

	/** Checks whether this unit currently possesses a given ability used like weapon
	 * @return True if the ability @a tag_name is active.
	 * @param special the const config to one of abilities @a tag_name checked.
	 * @param tag_name name of ability type checked.
	 * @param loc location of the unit checked.
	 */
	bool get_self_ability_bool(const config& special, const std::string& tag_name, const map_location& loc) const;
	/** Checks whether this unit currently possesses a given ability of leadership type
	 * @return True if the ability @a tag_name is active.
	 * @param special the const config to one of abilities @a tag_name checked.
	 * @param tag_name name of ability type checked.
	 * @param loc location of the unit checked.
	 * @param weapon the attack used by unit checked in this function.
	 * @param opp_weapon the attack used by opponent to unit checked.
	 */
	bool get_self_ability_bool_weapon(const config& special, const std::string& tag_name, const map_location& loc, const_attack_ptr weapon = nullptr, const_attack_ptr opp_weapon = nullptr) const;
	/** Checks whether this unit is affected by a given ability  used like weapon
	 * @return True if the ability @a tag_name is active.
	 * @param special the const config to one of abilities @a tag_name checked.
	 * @param tag_name name of ability type checked.
	 * @param loc location of the unit checked.
	 * @param from unit adjacent to @a this is checked in case of [affect_adjacent] abilities.
	 * @param dir direction to research a unit adjacent to @a this.
	 */
	bool get_adj_ability_bool(const config& special, const std::string& tag_name, int dir, const map_location& loc, const unit& from) const;
	/** Checks whether this unit is affected by a given ability of leadership type
	 * @return True if the ability @a tag_name is active.
	 * @param special the const config to one of abilities @a tag_name checked.
	 * @param tag_name name of ability type checked.
	 * @param loc location of the unit checked.
	 * @param from unit adjacent to @a this is checked in case of [affect_adjacent] abilities.
	 * @param dir direction to research a unit adjacent to @a this.
	 * @param weapon the attack used by unit checked in this function.
	 * @param opp_weapon the attack used by opponent to unit checked.
	 */
	bool get_adj_ability_bool_weapon(const config& special, const std::string& tag_name, int dir, const map_location& loc, const unit& from, const_attack_ptr weapon=nullptr, const_attack_ptr opp_weapon = nullptr) const;

	/**
	 * Gets the unit's active abilities of a particular type if it were on a specified location.
	 * @param tag_name The type of ability to check for
	 * @param loc The location to use for resolving abilities
	 * @return A list of active abilities, paired with the location they are active on
	 */
	unit_ability_list get_abilities(const std::string& tag_name, const map_location& loc) const;

	/**
	 * Gets the unit's active abilities of a particular type.
	 * @param tag_name The type of ability to check for
	 * @return A list of active abilities, paired with the location they are active on
	 */
	unit_ability_list get_abilities(const std::string& tag_name) const
	{
		return get_abilities(tag_name, loc_);
	}

	unit_ability_list get_abilities_weapons(const std::string& tag_name, const map_location& loc, const_attack_ptr weapon = nullptr, const_attack_ptr opp_weapon = nullptr) const;

	unit_ability_list get_abilities_weapons(const std::string& tag_name, const_attack_ptr weapon = nullptr, const_attack_ptr opp_weapon = nullptr) const
	{
		return get_abilities_weapons(tag_name, loc_, weapon, opp_weapon);
	}

	const config &abilities() const { return abilities_; }

	const std::set<std::string>& checking_tags() const { return checking_tags_; };

	/**
	 * Gets the names and descriptions of this unit's abilities. Location-independent variant
	 * with all abilities shown as active.
	 *
	 * @returns                   A list of quadruples consisting of (in order) id, base name,
	 *                            male or female name as appropriate for the unit, and description.
	 */
	std::vector<std::tuple<std::string, t_string, t_string, t_string>>
	ability_tooltips() const;

	/**
	 * Gets the names and descriptions of this unit's abilities.
	 *
	 * @param active_list         This vector will be the same length as the returned one and will
	 *                            indicate whether or not the corresponding ability is active.
	 *
	 * @param loc                 The location on which to resolve the ability.
	 *
	 * @returns                   A list of quadruples consisting of (in order) id, base name,
	 *                            male or female name as appropriate for the unit, and description.
	 */
	std::vector<std::tuple<std::string, t_string, t_string, t_string>>
	ability_tooltips(boost::dynamic_bitset<>& active_list, const map_location& loc) const;

	/** Get a list of all abilities by ID. */
	std::vector<std::string> get_ability_list() const;

	/**
	 * Check if the unit has an ability of a specific type.
	 * @param ability The type of ability (tag name) to check for.
	 * @returns true if the ability is present
	 */
	bool has_ability_type(const std::string& ability) const;

	/**
	 * Check if the unit has an ability of a specific ID.
	 * @param ability The ID of ability to check for.
	 * @returns true if the ability is present
	 */
	bool has_ability_by_id(const std::string& ability) const;

	/**
	 * Removes a unit's abilities with a specific ID.
	 * @param ability The type of ability (tag name) to remove.
	 */
	void remove_ability_by_id(const std::string& ability);

	/**
	 * Removes a unit's abilities with a specific ID or other attribute.
	 * @param filter the config of ability to remove.
	 */
	void remove_ability_by_attribute(const config& filter);

	/**
	 * Verify what abilities attributes match with filter.
	 * @param cfg the config of ability to check.
	 * @param tag_name the tag name of ability to check.
	 * @param filter the filter used for checking.
	 */
	bool ability_matches_filter(const config & cfg, const std::string& tag_name, const config & filter) const;


private:

	const std::set<std::string> checking_tags_{"disable", "attacks", "damage", "chance_to_hit", "berserk", "swarm", "drains", "heal_on_hit", "plague", "slow", "petrifies", "firststrike", "poison", "damage_type"};
	/**
	 * Check if an ability is active.
	 * @param ability The type (tag name) of the ability
	 * @param cfg an ability WML structure
	 * @param loc The location on which to resolve the ability
	 * @returns true if it is active
	 */
	bool ability_active(const std::string& ability, const config& cfg, const map_location& loc) const;

	/**
	 * Check if an ability affects adjacent units.
	 * @param ability The type (tag name) of the ability
	 * @param cfg an ability WML structure
	 * @param loc The location on which to resolve the ability
	 * @param from The "other unit" for filter matching
	 * @param dir The direction the unit is facing
	 */
	bool ability_affects_adjacent(const std::string& ability, const config& cfg, int dir, const map_location& loc, const unit& from) const;

	/**
	 * Check if an ability affects the owning unit.
	 * @param ability The type (tag name) of the ability
	 * @param cfg an ability WML structure
	 * @param loc The location on which to resolve the ability
	 */
	bool ability_affects_self(const std::string& ability, const config& cfg, const map_location& loc) const;

	/**
	 * filters the weapons that condition the use of abilities for combat ([resistance],[leadership] or abilities used like specials
	 * (deprecated in two last cases)
	 */
	bool ability_affects_weapon(const config& cfg, const_attack_ptr weapon, bool is_opp) const;

public:
	/** Get the unit formula manager. */
	unit_formula_manager& formula_manager() const
	{
		return *formula_man_;
	}

	/** Generates a random race-appropriate name if one has not already been provided. */
	void generate_name();

	// Only see_all = true use caching
	bool invisible(const map_location& loc, bool see_all = true) const;

	bool is_visible_to_team(const team& team, bool const see_all = true) const;
	/** Return true if the unit would be visible to team if its location were loc. */
	bool is_visible_to_team(const map_location& loc, const team& team, bool const see_all = true) const;

	/**
	 * Serializes the current unit metadata values.
	 *
	 * @param cfg                 The config to write to.
	 * @param write_all           set this to false to not write unchanged attributes.
	 */
	void write(config& cfg, bool write_all = true) const;

	/**
	 * Mark this unit as clone so it can be inserted to unit_map.
	 *
	 * @returns                   self (for convenience)
	 */
	unit& mark_clone(bool is_temporary);


	void set_appearance_changed(bool value) { appearance_changed_ = value; }
	bool appearance_changed() const { return appearance_changed_; }

protected:

private:
	map_location loc_;

	std::vector<std::string> advances_to_;

	/** Never nullptr. Adjusted for gender and variation. */
	const unit_type* type_;

	/** The displayed name of this unit type. */
	t_string type_name_;

	/** Never nullptr, but may point to the null race. */
	const unit_race* race_;

	std::string id_;
	t_string name_;
	n_unit::unit_id underlying_id_;

	std::string undead_variation_;
	std::string variation_;

	int hit_points_;
	int max_hit_points_;
	int experience_;
	int max_experience_;

	int level_;

	int recall_cost_;
	bool canrecruit_;
	std::vector<std::string> recruit_list_;
	unit_alignments::type alignment_;

	std::string flag_rgb_;
	std::string image_mods_;

	bool unrenamable_;

	int side_;

	unit_race::GENDER gender_;

	std::unique_ptr<unit_formula_manager> formula_man_;

	int movement_;
	int max_movement_;
	int vision_;
	int jamming_;

	movetype movement_type_;

	bool hold_position_;
	bool end_turn_;
	bool resting_;

	int attacks_left_;
	int max_attacks_;

	std::set<std::string> states_;

	static const std::size_t num_bool_states = state_t::NUMBER_OF_STATES;

	std::bitset<num_bool_states> known_boolean_states_;
	static std::map<std::string, state_t> known_boolean_state_names_;

	config variables_;
	config events_;
	config filter_recall_;

	bool emit_zoc_;

	std::vector<std::string> overlays_;

	std::string role_;
	attack_list attacks_;

protected:
	// TODO: I think we actually consider this to be part of the gamestate, so it might be better if it's not mutable,
	// but it's not easy to separate this guy from the animation code right now.
	mutable map_location::DIRECTION facing_;

private:
	std::vector<t_string> trait_names_;
	std::vector<t_string> trait_descriptions_;
	std::vector<std::string> trait_nonhidden_ids_;

	int unit_value_;
	map_location goto_, interrupted_move_;

	bool is_fearless_, is_healthy_;

	utils::string_map modification_descriptions_;

	// Animations:
	friend class unit_animation_component;

	std::unique_ptr<unit_animation_component> anim_comp_;

	mutable bool hidden_;
	double hp_bar_scaling_, xp_bar_scaling_;

	config modifications_;
	config abilities_;

	std::vector<config> advancements_;

	t_string description_;
	std::vector<t_string> special_notes_;

	utils::optional<std::string> usage_;
	utils::optional<std::string> halo_;
	utils::optional<std::string> ellipse_;

	bool random_traits_;
	bool generate_name_;

	upkeep_t upkeep_;

	std::string profile_;
	std::string small_profile_;

	//Used to check whether the moving units during a move needs to be updated
	mutable bool appearance_changed_ = true;
	std::bitset<UA_COUNT> changed_attributes_;

	void parse_upkeep(const config::attribute_value& upkeep);
	void write_upkeep(config::attribute_value& upkeep) const;

	/**
	 * Hold the visibility status cache for a unit, when not uncovered.
	 * This is mutable since it is a cache.
	 */
	mutable std::map<map_location, bool> invisibility_cache_;

	/**
	 * Clears the cache.
	 *
	 * Since we don't change the state of the object we're marked const (also
	 * required since the objects in the cache need to be marked const).
	 */
	void clear_visibility_cache() const
	{
		invisibility_cache_.clear();
	}
};

/**
 * Object which temporarily resets a unit's movement.
 *
 * @warning A unit whose movement is reset may not be deleted while held in a
 * @ref unit_movement_resetter object, so it's best to use thus only in a small scope.
 */
struct unit_movement_resetter
{
	unit_movement_resetter(const unit_movement_resetter&) = delete;
	unit_movement_resetter& operator=(const unit_movement_resetter&) = delete;

	unit_movement_resetter(const unit& u, bool operate = true);
	~unit_movement_resetter();

private:
	unit& u_;
	int moves_;
};

namespace backwards_compatibility
{
/**
 * Optional parameter for get_checksum to use the algorithm of an older version of Wesnoth,
 * thus preventing spurious OOS warnings while watching old replays.
 */
enum class unit_checksum_version {
	current,
	version_1_16_or_older /**< Included some of the flavortext from weapon specials. */
};

} // namespace backwards_compatibility

/**
 * Gets a checksum for a unit.
 *
 * In MP games the descriptions are locally generated and might differ, so it
 * should be possible to discard them.  Not sure whether replays suffer the
 * same problem.
 *
 *  @param u                    this unit
 *  @param version              allows the checksum expected in older replays to be used
 *
 *  @returns                    the checksum for a unit
 */
std::string get_checksum(const unit& u,
	backwards_compatibility::unit_checksum_version version = backwards_compatibility::unit_checksum_version::current);
