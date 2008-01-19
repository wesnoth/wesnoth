/* $Id$ */
/*
   Copyright (C) 2003 - 2007 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef UNIT_TYPES_H_INCLUDED
#define UNIT_TYPES_H_INCLUDED

#include "unit_animation.hpp"
#include "config.hpp"
#include "map.hpp"
#include "race.hpp"
#include "util.hpp"

#include <string>
#include <vector>


class unit_type;
class unit;
class unit_ability_list;
class game_data;
class gamestatus;
class team;


//and how much damage it does.
class attack_type
{
public:

	attack_type(const config& cfg);
	const t_string& name() const { return description_; }
	const std::string& id() const { return id_; }
	const std::string& type() const { return type_; }
	const std::string& icon() const { return icon_; }
	const std::string& range() const { return range_; }
	int damage() const { return damage_; }
	int num_attacks() const { return num_attacks_; }
	double attack_weight() const { return attack_weight_; }
	double defense_weight() const { return defense_weight_; }

	bool get_special_bool(const std::string& special,bool force=false) const;
	unit_ability_list get_specials(const std::string& special) const;
	std::vector<std::string> special_tooltips(bool force=false) const;
	std::string weapon_specials(bool force=false) const;
	void set_specials_context(const gamemap::location& aloc,const gamemap::location& dloc,
                              const game_data* gamedata, const unit_map* unitmap,
							  const gamemap* map, const gamestatus* game_status,
							  const std::vector<team>* teams,bool attacker,const attack_type* other_attack) const;
	void set_specials_context(const gamemap::location& loc,const gamemap::location& dloc, const unit& un, bool attacker =true) const;

	bool has_special_by_id(const std::string& special) const;
	//this function returns a random animation out of the possible
	//animations for this attack. It will not return the same attack
	//each time.
	bool matches_filter(const config& cfg,bool self=false) const;
	bool apply_modification(const config& cfg,std::string* description);
	bool describe_modification(const config& cfg,std::string* description);

	int movement_used() const { return cfg_["movement_used"] == "" ? 100000 : lexical_cast_default<int>(cfg_["movement_used"]); }

	config& get_cfg() { return cfg_; }
	const config& get_cfg() const { return cfg_; }
	mutable gamemap::location aloc_,dloc_;
	mutable bool attacker_;
	mutable const game_data* gamedata_;
	mutable const unit_map* unitmap_;
	mutable const gamemap* map_;
	mutable const gamestatus* game_status_;
	mutable const std::vector<team>* teams_;
	mutable const attack_type* other_attack_;
	/*
	 * cfg: a weapon special WML structure
	 */
	bool special_active(const config& cfg,bool self,bool report=false) const;
	bool special_affects_opponent(const config& cfg) const;
	bool special_affects_self(const config& cfg) const;

	config cfg_;
	const unit_animation* animation(const game_display& disp, const gamemap::location& loc,const unit* my_unit,const unit_animation::hit_type hit,const attack_type* secondary_attack,int swing_num,int damage) const;
	// made public to ease backward compatibility for WML syntax
	// to be removed (with all corresponding code once 1.3.6 is reached
	std::vector<unit_animation> animation_;
private:
	t_string description_;
	std::string id_;
	std::string type_;
	std::string icon_;
	std::string range_;
	int damage_;
	int num_attacks_;
	double attack_weight_;
	double defense_weight_;

};

class unit_movement_type;

//the 'unit movement type' is the basic size of the unit - flying, small land,
//large land, etc etc.
class unit_movement_type
{
public:
	//this class assumes that the passed in reference will remain valid
	//for at least as long as the class instance
	unit_movement_type(const config& cfg, const unit_movement_type* parent=NULL);
	unit_movement_type();

	const t_string& name() const;
	int movement_cost(const gamemap& map, t_translation::t_letter terrain, int recurse_count=0) const;
	int defense_modifier(const gamemap& map, t_translation::t_letter terrain, int recurse_count=0) const;
	int damage_against(const attack_type& attack) const { return resistance_against(attack); }
	int resistance_against(const attack_type& attack) const;

	string_map damage_table() const;

	void set_parent(const unit_movement_type* parent) { parent_ = parent; }

	bool is_flying() const;
	const std::map<t_translation::t_letter, int>& movement_costs() const { return moveCosts_; }
	const std::map<t_translation::t_letter, int>& defense_mods() const { return defenseMods_; }

	const config& get_cfg() const { return cfg_; }
	const unit_movement_type* get_parent() const { return parent_; }
private:
	mutable std::map<t_translation::t_letter, int> moveCosts_;
	mutable std::map<t_translation::t_letter, int> defenseMods_;

	const unit_movement_type* parent_;

	config cfg_;
};

typedef std::map<std::string,unit_movement_type> movement_type_map;

class unit_type
{
public:
	friend class unit;
	friend class game_data;

	unit_type();
	unit_type(const config& cfg, const movement_type_map& movement_types,
	          const race_map& races, const std::vector<config*>& traits);
	unit_type(const unit_type& o);

	~unit_type();

	//! Load data into an empty unit_type
	void build(const config& cfg, const movement_type_map& movement_types,
	          const race_map& races, const std::vector<config*>& traits);

	//! Adds an additional advancement path to a unit type.
	//! This is used to implement the [advancefrom] tag.
	void add_advancement(const unit_type &advance_to,int experience);

	//! Adds units that this unit advances from, for help file purposes.
	void add_advancesfrom(const unit_type &advance_from);

	const unit_type& get_gender_unit_type(unit_race::GENDER gender) const;
	const unit_type& get_variation(const std::string& name) const;
	//! Info on the type of unit that the unit reanimates as.
	const std::string& undead_variation() const { return cfg_["undead_variation"]; }

	unsigned int num_traits() const { return (cfg_["num_traits"].size() ? atoi(cfg_["num_traits"].c_str()) : race_->num_traits()); }

	std::string generate_description() const { return race_->generate_name(string_gender(cfg_["gender"])); }

	//! The name of the unit in the current language setting.
	const t_string& language_name() const { return cfg_["name"]; }

	const std::string& id() const;
	//Disabling this one for consistency with other similar structures,
	//where name() is always the user-visible, translated, name.
	//language_name should eventually be renamed name()
	// const std::string& name() const;

	const std::string& image() const { return cfg_["image"]; }
	const std::string& image_profile() const;
	const t_string& unit_description() const;

	const std::vector<unit_animation>& animations() const;

	const std::string& flag_rgb() const { return flag_rgb_; }

	int hitpoints() const { return atoi(cfg_["hitpoints"].c_str()); }
	std::vector<attack_type> attacks() const;
	const unit_movement_type& movement_type() const { return movementType_; }

	int experience_needed(bool with_acceleration=true) const;
	std::vector<std::string> advances_to() const { return advances_to_; }
	std::vector<std::string> advances_from() const { return advances_from_; }
	const config::child_list& modification_advancements() const { return cfg_.get_children("advancement"); }
	const std::string& usage() const { return cfg_["usage"]; }

	struct experience_accelerator {
		experience_accelerator(int modifier);
		~experience_accelerator();
		static int get_acceleration();
	private:
		int old_value_;
	};

	int level() const { return atoi(cfg_["level"].c_str()); }
	int movement() const { return atoi(cfg_["movement"].c_str()); }
	int cost() const { return atoi(cfg_["cost"].c_str()); }

	enum ALIGNMENT { LAWFUL, NEUTRAL, CHAOTIC };

	ALIGNMENT alignment() const { return alignment_; }
	static const char* alignment_description(ALIGNMENT align);
	static const char* alignment_id(ALIGNMENT align);

	fixed_t alpha() const { return alpha_; }

	const std::vector<std::string>& abilities() const { return abilities_; }
	const std::vector<std::string>& ability_tooltips() const { return ability_tooltips_; }

	bool can_advance() const { return !advances_to_.empty(); }

        bool not_living() const;

	bool has_zoc() const { return zoc_; }

	bool has_ability(const std::string& ability) const;
	bool has_ability_by_id(const std::string& ability) const;

	const std::vector<config*> possible_traits() const { return possibleTraits_.get_children("trait"); }
	bool has_random_traits() const { return (num_traits() > 0 && possible_traits().size() > 1); }

	const std::vector<unit_race::GENDER>& genders() const { return genders_; }

	const std::string& race() const;
	bool hide_help() const { return hide_help_; }

private:
	void operator=(const unit_type& o);

	unit_type* gender_types_[2];

	typedef std::map<std::string,unit_type*> variations_map;
	variations_map variations_;

	config cfg_;

	const unit_race* race_;

	fixed_t alpha_;

	std::vector<std::string> abilities_;
	std::vector<std::string> ability_tooltips_;

	mutable std::string id_;

	bool zoc_, hide_help_;

	std::vector<std::string> advances_to_;
	std::vector<std::string> advances_from_;
	int experience_needed_;


	ALIGNMENT alignment_;

	unit_movement_type movementType_;

	config possibleTraits_;

	std::vector<unit_race::GENDER> genders_;

	// animations are loaded only after the first animations() call
	mutable std::vector<unit_animation> animations_;

	std::string flag_rgb_;
};

class game_data
{
public:
	game_data();
	game_data(const config& cfg);
	void set_config(const config& cfg);
	void clear();

	movement_type_map movement_types;
	typedef std::map<std::string,unit_type> unit_type_map;
	unit_type_map unit_types;
	config merged_units;
	race_map races;
};

#endif
