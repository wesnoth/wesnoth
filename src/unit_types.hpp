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
#ifndef UNIT_TYPES_H_INCLUDED
#define UNIT_TYPES_H_INCLUDED

#include "map_location.hpp"
#include "movetype.hpp"
#include "race.hpp"
#include "util.hpp"

#include <boost/noncopyable.hpp>

struct tportrait;
class unit_ability_list;
class unit_animation;


typedef std::map<std::string, movetype> movement_type_map;


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
	int min_range_, max_range_;
	int damage_;
	int num_attacks_;
	double attack_weight_;
	double defense_weight_;

	int accuracy_;
	int movement_used_;
	int parry_;
};

class unit_type
{
public:
	/**
	 * Creates a unit type for the given config, but delays its build
	 * till later.
	 * @note @a cfg is not copied, so it has to point to some permanent
	 *       storage, that is, a child of unit_type_data::unit_cfg.
	 */
	explicit unit_type(const config &cfg, const std::string & parent_id="");
	unit_type(const unit_type& o);

	~unit_type();

	/// Records the status of the lazy building of unit types.
	/// These are in order of increasing levels of being built.
	/// HELP_INDEX is already defined in a windows header under some conditions.
	enum BUILD_STATUS {NOT_BUILT, CREATED, VARIATIONS, HELP_INDEXED , WITHOUT_ANIMATIONS, FULL};
private: // These will be called by build().
	/// Load data into an empty unit_type (build to FULL).
	void build_full(const movement_type_map &movement_types,
		const race_map &races, const config::const_child_itors &traits);
	/// Partially load data into an empty unit_type (build to HELP_INDEXED).
	void build_help_index(const movement_type_map &movement_types,
		const race_map &races, const config::const_child_itors &traits);
	/// Load the most needed data into an empty unit_type (build to CREATE).
	void build_created(const movement_type_map &movement_types,
		const race_map &races, const config::const_child_itors &traits);
public:
	/// Performs a build of this to the indicated stage.
	void build(BUILD_STATUS status, const movement_type_map &movement_types,
	           const race_map &races, const config::const_child_itors &traits);
	/// Performs a build of this to the indicated stage.
	/// (This does not logically change the unit type, so allow const access.)
	void build(BUILD_STATUS status, const movement_type_map &movement_types,
	           const race_map &races, const config::const_child_itors &traits) const
	{ const_cast<unit_type *>(this)->build(status, movement_types, races, traits); }


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
	const unit_type& get_variation(const std::string& id) const;
	/** Info on the type of unit that the unit reanimates as. */
	const std::string& undead_variation() const { return undead_variation_; }

	unsigned int num_traits() const { return num_traits_; }

	/** The name of the unit in the current language setting. */
	const t_string& type_name() const { return type_name_; }

	/// The id for this unit_type.
	const std::string& id() const { return id_; }
	/// A variant on id() that is more descriptive, for use with message logging.
	const std::string log_id() const { return id_ + debug_id_; }
	/// The id of the original type from which this (variation) descended.
	const std::string& base_id() const { return base_id_; }
	// NOTE: this used to be a const object reference, but it messed up with the
	// translation engine upon changing the language in the same session.
	t_string unit_description() const;
	int hitpoints() const { return hitpoints_; }
	double hp_bar_scaling() const { return hp_bar_scaling_; }
	double xp_bar_scaling() const { return xp_bar_scaling_; }
	int level() const { return level_; }
	int recall_cost() const { return recall_cost_;}
	int movement() const { return movement_; }
	int vision() const { return vision_ < 0 ? movement() : vision_; }
	/// If @a base_value is set to true, do not fall back to movement().
	int vision(bool base_value) const { return base_value ? vision_ : vision(); }
	int jamming() const {return jamming_; }
	int max_attacks() const { return max_attacks_; }
	int cost() const { return cost_; }
	const std::string& default_variation() const { return default_variation_; }
	const std::string& variation_name() const { return variation_name_; }
	const std::string& usage() const { return usage_; }
	const std::string& image() const { return image_; }
	const std::string& icon() const { return icon_; }
	const std::string &small_profile() const { return small_profile_; }
	const std::string &big_profile() const { return big_profile_; }

	const std::vector<unit_animation>& animations() const;

	const std::string& flag_rgb() const { return flag_rgb_; }

	std::vector<attack_type> attacks() const;
	const movetype & movement_type() const { return movement_type_; }

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

	/// The returned vector will not be empty, provided this has been built
	/// to the HELP_INDEXED status.
	const std::vector<unit_race::GENDER>& genders() const { return genders_; }
	std::vector<std::string> variations() const;
	
	/**
	 * @param variation_id		The id of the variation we search for.
	 * @return					Iff one of the type's variations' (or the sibling's if the unit_type is a variation itself) id matches @variation_id.
	 */
	bool has_variation(const std::string& variation_id) const;

	/// Returns the ID of this type's race without the need to build the type.
	std::string race_id() const { return cfg_["race"]; } //race_->id(); }
	/// Never returns NULL, but may point to the null race.
	/// Requires building to the HELP_INDEXED status to get the correct race.
	const unit_race* race() const { return race_; }
	bool hide_help() const;
	bool do_not_list() const { return do_not_list_; }

	const std::vector<tportrait>& portraits() const { return portraits_; }

	const config &get_cfg() const { return cfg_; }
	/// Returns a trimmed config suitable for use with units.
	const config & get_cfg_for_units() const
	{ return built_unit_cfg_ ? unit_cfg_ : build_unit_cfg(); }

	/// Gets resistance while considering custom WML abilities.
	/// Attention: Filters in resistance-abilities will be ignored.
	int resistance_against(const std::string& damage_name, bool attacker) const;

private:
	/// Generates (and returns) a trimmed config suitable for use with units.
	const config & build_unit_cfg() const;

	/// Identical to unit::resistance_filter_matches.
	bool resistance_filter_matches(const config& cfg,bool attacker,const std::string& damage_name, int res) const;

private:
	void operator=(const unit_type& o);

	const config &cfg_;
	mutable config unit_cfg_;  /// Generated as needed via get_cfg_for_units().
	mutable bool built_unit_cfg_;

	std::string id_;
	std::string debug_id_;  /// A suffix for id_, used when logging messages.
	std::string base_id_;   /// The id of the top ancestor of this unit_type.
	t_string type_name_;
	t_string description_;
	int hitpoints_;
	double hp_bar_scaling_, xp_bar_scaling_;
	int level_;
	int recall_cost_;
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
	std::string default_variation_;
	std::string variation_name_;

	const unit_race* race_;	/// Never NULL, but may point to the null race.

	fixed_t alpha_;

	std::vector<t_string> abilities_, adv_abilities_;
	std::vector<t_string> ability_tooltips_, adv_ability_tooltips_;

	bool zoc_, hide_help_, do_not_list_;

	std::vector<std::string> advances_to_;
	int experience_needed_;
	bool in_advancefrom_;


	ALIGNMENT alignment_;

	movetype movement_type_;

	config possibleTraits_;

	std::vector<unit_race::GENDER> genders_;

	// animations are loaded only after the first animations() call
	mutable std::vector<unit_animation> animations_;

	BUILD_STATUS build_status_;

	/** List with the portraits available for the unit. */
	std::vector<tportrait> portraits_;
};

class unit_type_data
	: private boost::noncopyable
{
public:
	unit_type_data();

	typedef std::map<std::string,unit_type> unit_type_map;

	const unit_type_map &types() const { return types_; }
	const race_map &races() const { return races_; }
	const config::const_child_itors traits() const { return unit_cfg_->child_range("trait"); }
	void set_config(config &cfg);

	/// Finds a unit_type by its id() and makes sure it is built to the specified level.
	const unit_type *find(const std::string &key, unit_type::BUILD_STATUS status = unit_type::FULL) const;
	void check_types(const std::vector<std::string>& types) const;
	const unit_race *find_race(const std::string &) const;

	/// Makes sure the all unit_types are built to the specified level.
	void build_all(unit_type::BUILD_STATUS status);
	/// Makes sure the provided unit_type is built to the specified level.
	void build_unit_type(const unit_type & ut, unit_type::BUILD_STATUS status) const
	{ ut.build(status, movement_types_, races_, unit_cfg_->child_range("trait")); }

	/** Checks if the [hide_help] tag contains these IDs. */
	bool hide_help(const std::string &type_id, const std::string &race_id) const;

private:
	/** Parses the [hide_help] tag. */
	void read_hide_help(const config &cfg);

	std::pair<unit_type_map::iterator, bool> insert(const std::pair<std::string, unit_type> &utype) { return types_.insert(utype); }
	void clear();

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

namespace legacy {
	/// Strips the name of an ability/special from its description.
	t_string ability_description(const t_string & description);
}


#endif
