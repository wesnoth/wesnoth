/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "gettext.hpp"
#include "utils/make_enum.hpp"
#include "map/location.hpp"
#include "movetype.hpp"
#include "units/race.hpp"
#include "units/attack_type.hpp"
#include "game_errors.hpp"

#include <array>
#include <map>
#include <set>
#include <string>
#include <vector>

class unit_ability_list;
class unit_animation;


typedef std::map<std::string, movetype> movement_type_map;


/**
 * A single unit type that the player may recruit.
 * Individual units are defined by the unit class.
 */
class unit_type
{
public:
	class error : public game::game_error
	{
	public:
		error(const std::string& msg)
			: game::game_error(msg)
		{
		}
	};
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
	enum BUILD_STATUS {NOT_BUILT, CREATED, VARIATIONS, HELP_INDEXED, FULL};

	/**
	 * Validate the id argument.
	 * Replaces invalid characters in the reference with underscores.
	 * @param id The proposed id for a unit_type.
	 * @throw error if id starts with a space.
	 */
	static void check_id(std::string& id);

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

	typedef std::map<std::string,unit_type> variations_map;
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
	 *  @return A set of ids of all unit_type objects that this unit_type can
	 *  directly or indirectly advance to.
	 */
	std::set<std::string> advancement_tree() const;

	/// A vector of unit_type ids that this unit_type can advance to.
	const std::vector<std::string>& advances_to() const { return advances_to_; }
	/// A vector of unit_type ids that can advance to this unit_type.
	const std::vector<std::string> advances_from() const;

	/// Returns two iterators pointing to a range of AMLA configs.
	config::const_child_itors modification_advancements() const
	{ return cfg_.child_range("advancement"); }

	/**
	 * Returns a gendered variant of this unit_type.
	 * @param gender "male" or "female".
	 */
	const unit_type& get_gender_unit_type(std::string gender) const;
	/// Returns a gendered variant of this unit_type based on the given parameter.
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
	const std::string &big_profile() const { return profile_; }
	std::string halo() const { return cfg_["halo"]; }
	std::string ellipse() const { return cfg_["ellipse"]; }
	bool generate_name() const { return cfg_["generate_name"].to_bool(true); }
	const std::vector<unit_animation>& animations() const;

	const std::string& flag_rgb() const;

	const_attack_itors attacks() const;
	const movetype & movement_type() const { return movement_type_; }

	int experience_needed(bool with_acceleration=true) const;

	MAKE_ENUM (ALIGNMENT,
		(LAWFUL, N_("lawful"))
		(NEUTRAL, N_("neutral"))
		(CHAOTIC, N_("chaotic"))
		(LIMINAL, N_("liminal"))
	)

	ALIGNMENT alignment() const { return alignment_; }
	static std::string alignment_description(ALIGNMENT align, unit_race::GENDER gender = unit_race::MALE);

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
	{ return possible_traits_.child_range("trait"); }

	const config& abilities_cfg() const
	{ return cfg_.child_or_empty("abilities"); }

	config::const_child_itors advancements() const
	{ return cfg_.child_range("advancement"); }

	config::const_child_itors events() const
	{ return cfg_.child_range("event"); }

	bool has_random_traits() const;

	/// The returned vector will not be empty, provided this has been built
	/// to the HELP_INDEXED status.
	const std::vector<unit_race::GENDER>& genders() const { return genders_; }
	bool has_gender_variation(const unit_race::GENDER gender) const
	{
		return std::find(genders_.begin(), genders_.end(), gender) != genders_.end();
	}

	std::vector<std::string> variations() const;
	const variations_map& variation_types() const {return variations_; }

	/**
	 * @param variation_id		The id of the variation we search for.
	 * @return					Whether one of the type's variations' (or the
	 *                          siblings' if the unit_type is a variation
	 *                          itself) id matches @a variation_id.
	 */
	bool has_variation(const std::string& variation_id) const;

	/**
	 * Whether the unit type has at least one help-visible variation.
	 */
	bool show_variations_in_help() const;

	/// Returns the ID of this type's race without the need to build the type.
	std::string race_id() const { return cfg_["race"]; } //race_->id(); }
	/// Never returns nullptr, but may point to the null race.
	/// Requires building to the HELP_INDEXED status to get the correct race.
	const unit_race* race() const { return race_; }
	bool hide_help() const;
	bool do_not_list() const { return do_not_list_; }

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
	mutable attack_list attacks_cache_;

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
	std::string profile_;
	std::string flag_rgb_;

	unsigned int num_traits_;

	std::array<std::unique_ptr<unit_type>, 2> gender_types_;

	variations_map variations_;
	std::string default_variation_;
	std::string variation_name_;

	const unit_race* race_;	/// Never nullptr, but may point to the null race.

	std::vector<t_string> abilities_, adv_abilities_;
	std::vector<t_string> ability_tooltips_, adv_ability_tooltips_;

	bool zoc_, hide_help_, do_not_list_;

	std::vector<std::string> advances_to_;
	int experience_needed_;
	bool in_advancefrom_;


	ALIGNMENT alignment_;

	movetype movement_type_;

	config possible_traits_;

	std::vector<unit_race::GENDER> genders_;

	// animations are loaded only after the first animations() call
	mutable std::vector<unit_animation> animations_;

	BUILD_STATUS build_status_;
};

class unit_type_data
{
public:
	unit_type_data(const unit_type_data&) = delete;
	unit_type_data& operator=(const unit_type_data&) = delete;

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

void adjust_profile(std::string& profile);

struct unit_experience_accelerator {
	unit_experience_accelerator(int modifier);
	~unit_experience_accelerator();
	static int get_acceleration();
private:
	int old_value_;
};
