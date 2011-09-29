/* $Id$ */
/*
   Copyright (C) 2003 - 2011 by David White <dave@whitevine.net>
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


#include <boost/unordered_map.hpp>
#include "unit_animation.hpp"
#include "portrait.hpp"
#include "race.hpp"
#include "resources.hpp"

class gamemap;
class unit;
class unit_ability_list;
class unit_map;
class unit_type_data;

//and how much damage it does.
class attack_type
{
public:

	attack_type(const config& cfg);
	/** Default implementation, but defined out-of-line for efficiency reasons. */
	~attack_type();
	const t_string& name() const { return description_; }
	const config::t_token& id() const { return id_; }
	const config::t_token& type() const { return type_; }
	const config::t_token& icon() const { return icon_; }
	const config::t_token& range() const { return range_; }
	config::t_token accuracy_parry_description() const;
	int accuracy() const { return accuracy_; }
	int parry() const { return parry_; }
	int damage() const { return damage_; }
	int num_attacks() const { return num_attacks_; }
	double attack_weight() const { return attack_weight_; }
	double defense_weight() const { return defense_weight_; }

	bool get_special_bool(const config::t_token& special,bool force=false) const;
	unit_ability_list get_specials(const config::t_token& special) const;
	std::vector<t_string> special_tooltips(bool force=false) const;
	config::t_token weapon_specials(bool force=false) const;
	void set_specials_context(const map_location& aloc,const map_location& dloc,
		const unit_map &unitmap, bool attacker, const attack_type *other_attack) const;
	void set_specials_context(const map_location& loc,const map_location& dloc, const unit& un, bool attacker =true) const;

	bool matches_filter(const config& cfg,bool self=false) const;
	std::pair<bool, config::t_token> apply_modification(const config& cfg);
	std::pair<bool, config::t_token> describe_modification(const config& cfg);

	int movement_used() const {
		static const config::t_token & z_movement_used( generate_safe_static_const_t_interned(n_token::t_token("movement_used")) );
		return cfg_[z_movement_used].to_int(100000); }

	config& get_cfg() { return cfg_; }
	const config& get_cfg() const { return cfg_; }
	mutable map_location aloc_,dloc_;
	mutable bool attacker_;
	mutable const unit_map* unitmap_;
	mutable const attack_type* other_attack_;
	/*
	 * cfg: a weapon special WML structure
	 */
	bool special_active(gamemap const & game_map, unit_map const & units,
						  t_teams const & teams, LuaKernel & lua_kernel,
						  tod_manager const & tod_manager,
						const config& cfg, bool self) const;
	inline bool special_active(const config& cfg, bool self) const {
		return special_active(*resources::game_map, *resources::units,*resources::teams,
							  *resources::lua_kernel, *resources::tod_manager, cfg, self);}
	bool special_affects_opponent(const config& cfg) const;
	bool special_affects_self(const config& cfg) const;

	//this function returns a random animation out of the possible
	//animations for this attack. It will not return the same attack
	//each time.
	const unit_animation* animation(const game_display& disp, const map_location& loc,const unit* my_unit,const unit_animation::hit_type hit,const attack_type* secondary_attack,int swing_num,int damage) const;
private:
	config cfg_;
	t_string description_;
	config::t_token id_;
	config::t_token type_;
	config::t_token icon_;
	config::t_token range_;
	int damage_;
	int num_attacks_;
	double attack_weight_;
	double defense_weight_;

	int accuracy_;
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

typedef boost::unordered_map<t_translation::t_terrain, defense_range> defense_cache;

const defense_range &defense_range_modifier_internal(defense_cache &defense_mods,
	const config &cfg, const unit_movement_type *parent,
	const gamemap &map, t_translation::t_terrain terrain, int recurse_count = 0);

int defense_modifier_internal(defense_cache &defense_mods,
	const config &cfg, const unit_movement_type *parent,
	const gamemap &map, t_translation::t_terrain terrain, int recurse_count = 0);

typedef boost::unordered_map<t_translation::t_terrain, int> t_move_cost_cache;
int movement_cost_internal( t_move_cost_cache &move_costs,
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

	config::t_token name() const;
	int movement_cost(const gamemap &map, t_translation::t_terrain terrain) const
	{ return movement_cost_internal(moveCosts_, cfg_, parent_, map, terrain); }
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
	mutable t_move_cost_cache moveCosts_;
	mutable defense_cache defenseMods_;

	const unit_movement_type* parent_;

	config cfg_;
};

typedef boost::unordered_map<config::t_token, unit_movement_type> movement_type_map;

class unit_type
{
public:
	friend class unit;
	friend class unit_type_data;

	/**
	 * Creates a unit type for the given config, but delays its build
	 * till later.
	 * @note @a cfg is not copied, so it has to point to some permanent
	 *       storage, that is, a child of unit_type_data::unit_cfg.
	 */
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
	boost::unordered_set<config::t_token> advancement_tree() const;

	const std::vector<config::t_token>& advances_to() const { return advances_to_; }
	const std::vector<config::t_token> advances_from() const;

	config::const_child_itors modification_advancements() const {
		static const config::t_token & z_advancement( generate_safe_static_const_t_interned(n_token::t_token("advancement")) );
		return cfg_.child_range(z_advancement); }

	const unit_type& get_gender_unit_type(config::t_token const & gender) const;
	//const unit_type& get_gender_unit_type(std::string gender) const;
	const unit_type& get_gender_unit_type(unit_race::GENDER gender) const;
	const unit_type& get_variation(const config::t_token& name) const;
	/** Info on the type of unit that the unit reanimates as. */
	const config::t_token& undead_variation() const { return undead_variation_; }

	unsigned int num_traits() const { return num_traits_; }

	/** The name of the unit in the current language setting. */
	const t_string& type_name() const { return type_name_; }

	const config::t_token& id() const { return id_; }
	// NOTE: this used to be a const object reference, but it messed up with the
	// translation engine upon changing the language in the same session.
	const t_string unit_description() const;
	int hitpoints() const { return hitpoints_; }
	int level() const { return level_; }
	int movement() const { return movement_; }
	int max_attacks() const { return max_attacks_; }
	int cost() const { return cost_; }
	const config::t_token& usage() const { return usage_; }
	const config::t_token& image() const { return image_; }
	const config::t_token &small_profile() const { return small_profile_; }
	const config::t_token &big_profile() const { return big_profile_; }

	const std::vector<unit_animation>& animations() const;

	const config::t_token& flag_rgb() const { return flag_rgb_; }

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

    bool not_living() const;

	bool has_zoc() const { return zoc_; }

	bool has_ability_by_id(const config::t_token& ability) const;
	std::vector<config::t_token> get_ability_list() const;

	config::const_child_itors possible_traits() const {
		static const config::t_token & z_trait( generate_safe_static_const_t_interned(n_token::t_token("trait")) );
		return possibleTraits_.child_range(z_trait); }
	bool has_random_traits() const;

	const std::vector<unit_race::GENDER>& genders() const { return genders_; }

	const config::t_token& race() const { return race_->id(); }
	bool hide_help() const;

    enum BUILD_STATUS {NOT_BUILT, CREATED, HELP_INDEX, WITHOUT_ANIMATIONS, FULL};

    BUILD_STATUS build_status() const { return build_status_; }

	const std::vector<tportrait>& portraits() const { return portraits_; }

	const config &get_cfg() const { return cfg_; }

private:
	void operator=(const unit_type& o);

	config &cfg_;

	config::t_token id_;
    t_string type_name_;
    t_string description_;
    int hitpoints_;
    int level_;
    int movement_;
    int max_attacks_;
    int cost_;
    config::t_token usage_;
	config::t_token undead_variation_;

	config::t_token image_;
	config::t_token small_profile_;
	config::t_token big_profile_;
	config::t_token flag_rgb_;

    unsigned int num_traits_;

	unit_type* gender_types_[2];

	typedef boost::unordered_map<config::t_token, unit_type*> variations_map;
	variations_map variations_;

	const unit_race* race_;

	fixed_t alpha_;

	std::vector<t_string> abilities_, adv_abilities_;
	std::vector<t_string> ability_tooltips_, adv_ability_tooltips_;

	bool zoc_, hide_help_;

	std::vector<config::t_token> advances_to_;
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

	typedef boost::unordered_map<config::t_token, unit_type> unit_type_map;

	const unit_type_map &types() const { return types_; }
	const race_map &races() const { return races_; }
	const config::const_child_itors traits() const { return unit_cfg_->child_range("trait"); }
	void set_config(config &cfg);

	const unit_type *find(const config::t_token &key, unit_type::BUILD_STATUS status = unit_type::FULL) const;
	const unit_type *find(const std::string &key, unit_type::BUILD_STATUS status = unit_type::FULL) const {
		return find(config::t_token(key), status); }
	void check_types(const std::vector<config::t_token>& types) const;
	const unit_race *find_race(const config::t_token &) const;

	void build_all(unit_type::BUILD_STATUS status);

	/** Checks if the [hide_help] tag contains these IDs. */
	bool hide_help(const config::t_token &type_id, const config::t_token &race_id) const;

private:
	unit_type_data(const unit_type_data &);

	/** Parses the [hide_help] tag. */
	void read_hide_help(const config &cfg);

	void set_unit_config(const config& unit_cfg) { unit_cfg_ = &unit_cfg; }

	const config &find_config(const config::t_token &key) const;
	std::pair<unit_type_map::iterator, bool> insert(const std::pair<config::t_token, unit_type> &utype) { return types_.insert(utype); }
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
	std::vector< boost::unordered_set<config::t_token> > hide_help_type_;
	std::vector< boost::unordered_set<config::t_token> > hide_help_race_;

	const config *unit_cfg_;
	unit_type::BUILD_STATUS build_status_;
};

extern unit_type_data unit_types;

std::pair<config::t_token, config::t_token> adjust_profile(config::t_token const &small, config::t_token const &big, config::t_token const &def);

#endif
