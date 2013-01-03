/* $Id$ */
/*
   Copyright (C) 2003 - 2013 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef UNIT_TYPES_H_INCLUDED
#define UNIT_TYPES_H_INCLUDED

#include "unit_animation.hpp"
#include "portrait.hpp"
#include "race.hpp"

class gamemap;
class unit;
class unit_ability_list;
class unit_map;
class unit_type_data;


//the 'attack type' is the type of attack, how many times it strikes,
//and how much damage it does.
class attack_type
{
public:

	attack_type(const config& cfg);
	/** Default implementation, but defined out-of-line for efficiency reasons. */
	~attack_type();
	const t_string& name() const { return description_; }
	const std::string& id() const { return id_; }
	const std::string& type() const { return type_; }
	const std::string& icon() const { return icon_; }
	const std::string& range() const { return range_; }
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

	const config& get_cfg() const { return cfg_; }

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

	config cfg_;
	t_string description_;
	std::string id_;
	std::string type_;
	std::string icon_;
	std::string range_;
	int damage_;
	int num_attacks_;
	double attack_weight_;
	double defense_weight_;

	int accuracy_;
	int movement_used_;
	int parry_;
};

class unit_movement_type;

/**
 * Possible range of the defense. When a single value is needed, #max_
 * (maximum defense) is selected, unless #min_ is bigger.
 */
struct defense_range
{
	int min_, max_;
};

typedef std::map<t_translation::t_terrain, defense_range> defense_cache;

const defense_range &defense_range_modifier_internal(defense_cache &defense_mods,
	const config &cfg, const unit_movement_type *parent,
	const gamemap &map, t_translation::t_terrain terrain, int recurse_count = 0);

int defense_modifier_internal(defense_cache &defense_mods,
	const config &cfg, const unit_movement_type *parent,
	const gamemap &map, t_translation::t_terrain terrain, int recurse_count = 0);

int movement_cost_internal(std::map<t_translation::t_terrain, int> &move_costs,
	const config &cfg, const unit_movement_type *parent,
	const gamemap &map, t_translation::t_terrain terrain, int recurse_count = 0);

//the 'unit movement type' is the basic size of the unit - flying, small land,
//large land, etc etc.
class unit_movement_type
{
public:
	//this move distance means a hex is unreachable
	//if there is an UNREACHABLE macro declared in the data tree
	//it should match this value.
	static const int UNREACHABLE = 99;

	//this class assumes that the passed in reference will remain valid
	//for at least as long as the class instance
	unit_movement_type(const config& cfg, const unit_movement_type* parent=NULL);
	unit_movement_type();

	std::string name() const;
	int movement_cost(const gamemap &map, t_translation::t_terrain terrain) const
	{ return movement_cost_internal(moveCosts_, cfg_.child("movement_costs"), parent_, map, terrain); }
	int vision_cost(const gamemap &map, t_translation::t_terrain terrain) const
	{ return movement_cost_internal(visionCosts_, cfg_.child("vision_costs"), parent_, map, terrain); }
	int jamming_cost(const gamemap &map, t_translation::t_terrain terrain) const
	{ return movement_cost_internal(jammingCosts_, cfg_.child("jamming_costs"), parent_, map, terrain); }
	int defense_modifier(const gamemap &map, t_translation::t_terrain terrain) const
	{ return defense_modifier_internal(defenseMods_, cfg_, parent_, map, terrain); }
	const defense_range &defense_range_modifier(const gamemap &map, t_translation::t_terrain terrain) const
	{ return defense_range_modifier_internal(defenseMods_, cfg_, parent_, map, terrain); }
	int damage_against(const attack_type& attack) const { return resistance_against(attack); }
	int resistance_against(const attack_type& attack) const;

	utils::string_map damage_table() const;

	void set_parent(const unit_movement_type* parent) { parent_ = parent; }

	bool is_flying() const;

	const config& get_cfg() const { return cfg_; }
	const unit_movement_type* get_parent() const { return parent_; }
private:
	mutable std::map<t_translation::t_terrain, int> moveCosts_;
	mutable std::map<t_translation::t_terrain, int> visionCosts_;
	mutable std::map<t_translation::t_terrain, int> jammingCosts_;

	mutable defense_cache defenseMods_;

	const unit_movement_type* parent_;

	config cfg_;
};

typedef std::map<std::string,unit_movement_type> movement_type_map;

class unit_type
{
public:
	/**
	 * Creates a unit type for the given config, but delays its build
	 * till later.
	 * @note @a cfg is not copied, so it has to point to some permanent
	 *       storage, that is, a child of unit_type_data::unit_cfg.
	 */
	unit_type();
	unit_type(config &cfg);
	unit_type(const unit_type& o);

	~unit_type();

	/** Load data into an empty unit_type */
	void build_full(const movement_type_map &movement_types,
		const race_map &races, const config::const_child_itors &traits);
	/** Partially load data into an empty unit_type */
	void build_help_index(const movement_type_map &movement_types,
		const race_map &races, const config::const_child_itors &traits);
	/** Load the most needed data into an empty unit_type */
	void build_created(const movement_type_map &movement_types,
		const race_map &races, const config::const_child_itors &traits);

	/**
	 * Adds an additional advancement path to a unit type.
	 * This is used to implement the [advancefrom] tag.
	 */
	void add_advancement(const unit_type &advance_to,int experience);

	/** Get the advancement tree
	 *  Build a set of unit type's id of this unit type's advancement tree */
	std::set<std::string> advancement_tree() const;

	const std::vector<std::string>& advances_to() const { return advances_to_; }
	const std::vector<std::string> advances_from() const;

	config::const_child_itors modification_advancements() const
	{ return cfg_.child_range("advancement"); }

	const unit_type& get_gender_unit_type(std::string gender) const;
	const unit_type& get_gender_unit_type(unit_race::GENDER gender) const;
	const unit_type& get_variation(const std::string& name) const;
	/** Info on the type of unit that the unit reanimates as. */
	const std::string& undead_variation() const { return undead_variation_; }

	unsigned int num_traits() const { return num_traits_; }

	/** The name of the unit in the current language setting. */
	const t_string& type_name() const { return type_name_; }

	const std::string& id() const { return id_; }
	// NOTE: this used to be a const object reference, but it messed up with the
	// translation engine upon changing the language in the same session.
	const t_string unit_description() const;
	int hitpoints() const { return hitpoints_; }
	int level() const { return level_; }
	int movement() const { return movement_; }
	int vision() const { return vision_; }
	int jamming() const {return jamming_; }
	int max_attacks() const { return max_attacks_; }
	int cost() const { return cost_; }
	const std::string& usage() const { return usage_; }
	const std::string& image() const { return image_; }
	const std::string& icon() const { return icon_; }
	const std::string &small_profile() const { return small_profile_; }
	const std::string &big_profile() const { return big_profile_; }

	const std::vector<unit_animation>& animations() const;

	const std::string& flag_rgb() const { return flag_rgb_; }

	std::vector<attack_type> attacks() const;
	const unit_movement_type& movement_type() const { return movementType_; }

	int experience_needed(bool with_acceleration=true) const;

	struct experience_accelerator {
		experience_accelerator(int modifier);
		~experience_accelerator();
		static int get_acceleration();
	private:
		int old_value_;
	};

	enum ALIGNMENT { LAWFUL, NEUTRAL, CHAOTIC, LIMINAL };

	ALIGNMENT alignment() const { return alignment_; }
	static const char* alignment_description(ALIGNMENT align, unit_race::GENDER gender = unit_race::MALE);
	static const char* alignment_id(ALIGNMENT align);

	fixed_t alpha() const { return alpha_; }

	const std::vector<t_string>& abilities() const { return abilities_; }
	const std::vector<t_string>& ability_tooltips() const { return ability_tooltips_; }

	// some extra abilities may be gained through AMLA advancements
	const std::vector<t_string>& adv_abilities() const { return adv_abilities_; }
	const std::vector<t_string>& adv_ability_tooltips() const { return adv_ability_tooltips_; }

	bool can_advance() const { return !advances_to_.empty(); }

	bool musthave_status(const std::string& status) const;

	bool has_zoc() const { return zoc_; }

	bool has_ability_by_id(const std::string& ability) const;
	std::vector<std::string> get_ability_list() const;

	config::const_child_itors possible_traits() const
	{ return possibleTraits_.child_range("trait"); }
	bool has_random_traits() const;

	const std::vector<unit_race::GENDER>& genders() const { return genders_; }
	std::vector<std::string> variations() const;

	/// Returns the ID of this type's race without the need to build the type.
	std::string race_id() const { return cfg_["race"]; } //race_->id(); }
	/// Never returns NULL, but may point to the null race.
	/// Requires building to the HELP_INDEX status to get the correct race.
	const unit_race* race() const { return race_; }
	bool hide_help() const;

    enum BUILD_STATUS {NOT_BUILT, CREATED, HELP_INDEX, WITHOUT_ANIMATIONS, FULL};

    BUILD_STATUS build_status() const { return build_status_; }

	const std::vector<tportrait>& portraits() const { return portraits_; }

	const config &get_cfg() const { return cfg_; }

private:
	void operator=(const unit_type& o);

	config &cfg_;

	std::string id_;
    t_string type_name_;
    t_string description_;
    int hitpoints_;
    int level_;
    int movement_;
    int vision_;
    int jamming_;
    int max_attacks_;
    int cost_;
    std::string usage_;
    std::string undead_variation_;

	std::string image_;
	std::string icon_;
	std::string small_profile_;
	std::string big_profile_;
	std::string flag_rgb_;

    unsigned int num_traits_;

	unit_type* gender_types_[2];

	typedef std::map<std::string,unit_type*> variations_map;
	variations_map variations_;

	const unit_race* race_;	/// Never returns NULL, but may point to the null race.

	fixed_t alpha_;

	std::vector<t_string> abilities_, adv_abilities_;
	std::vector<t_string> ability_tooltips_, adv_ability_tooltips_;

	bool zoc_, hide_help_;

	std::vector<std::string> advances_to_;
	int experience_needed_;
	bool in_advancefrom_;


	ALIGNMENT alignment_;

	unit_movement_type movementType_;

	config possibleTraits_;

	std::vector<unit_race::GENDER> genders_;

	// animations are loaded only after the first animations() call
	mutable std::vector<unit_animation> animations_;

	BUILD_STATUS build_status_;

	/** List with the portraits available for the unit. */
	std::vector<tportrait> portraits_;
};

class unit_type_data
{
public:
	unit_type_data();

	typedef std::map<std::string,unit_type> unit_type_map;

	const unit_type_map &types() const { return types_; }
	const race_map &races() const { return races_; }
	const config::const_child_itors traits() const { return unit_cfg_->child_range("trait"); }
	void set_config(config &cfg);

	const unit_type *find(const std::string &key, unit_type::BUILD_STATUS status = unit_type::FULL) const;
	void check_types(const std::vector<std::string>& types) const;
	const unit_race *find_race(const std::string &) const;

	void build_all(unit_type::BUILD_STATUS status);

	/** Checks if the [hide_help] tag contains these IDs. */
	bool hide_help(const std::string &type_id, const std::string &race_id) const;

private:
	unit_type_data(const unit_type_data &);

	/** Parses the [hide_help] tag. */
	void read_hide_help(const config &cfg);

	void set_unit_config(const config& unit_cfg) { unit_cfg_ = &unit_cfg; }

	const config &find_config(const std::string &key) const;
	std::pair<unit_type_map::iterator, bool> insert(const std::pair<std::string, unit_type> &utype) { return types_.insert(utype); }
	void clear();

	unit_type& build_unit_type(const unit_type_map::iterator &ut, unit_type::BUILD_STATUS status) const;
	void add_advancefrom(const config& unit_cfg) const;
	void add_advancement(unit_type& to_unit) const;

	mutable unit_type_map types_;
	movement_type_map movement_types_;
	race_map races_;

	/** True if [hide_help] contains a 'all=yes' at its root. */
	bool hide_help_all_;
	// vectors containing the [hide_help] and its sub-tags [not]
	std::vector< std::set<std::string> > hide_help_type_;
	std::vector< std::set<std::string> > hide_help_race_;

	const config *unit_cfg_;
	unit_type::BUILD_STATUS build_status_;
};

extern unit_type_data unit_types;

void adjust_profile(std::string &small, std::string &big, std::string const &def);

#endif
