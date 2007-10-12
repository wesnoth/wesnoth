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

//! @file unit.hpp
//!

#ifndef UNIT_H_INCLUDED
#define UNIT_H_INCLUDED

#include "config.hpp"
#include "map.hpp"
#include "race.hpp"
#include "team.hpp"
#include "unit_types.hpp"
#include "unit_map.hpp"

class game_display;
class gamestatus;
class config_writer;

#include <set>
#include <string>
#include <vector>

class unit_ability_list
{
public:

	bool empty() const;

	std::pair<int,gamemap::location> highest(const std::string& key, int def=0) const;
	std::pair<int,gamemap::location> lowest(const std::string& key, int def=100) const;

	std::vector<std::pair<config*,gamemap::location> > cfgs;
};


class unit
{
public:
	//! @todo Unclear comment:
	// clear the status caches for esch unit, this is should be called it
	// the status of a unit changes
	static void clear_status_caches();

	friend struct unit_movement_resetter;
	// Copy constructor
	unit(const unit& u);
	// Initilizes a unit from a config
	unit(const game_data& gamedata, const config& cfg, bool use_traits=false);
	unit(const game_data* gamedata, unit_map* unitmap, const gamemap* map, const gamestatus* game_status, const std::vector<team>* teams, const config& cfg, bool use_traits=false);
	// Initilizes a unit from a unit type
	unit(const unit_type* t, int side, bool use_traits=false, bool dummy_unit=false, unit_race::GENDER gender=unit_race::MALE);
	unit(const game_data* gamedata, unit_map* unitmap, const gamemap* map, const gamestatus* game_status, const std::vector<team>* teams, const unit_type* t, int side, bool use_traits=false, bool dummy_unit=false, unit_race::GENDER gender=unit_race::MALE);
	virtual ~unit();
	unit& operator=(const unit&);

	void set_game_context(const game_data* gamedata, unit_map* unitmap, const gamemap* map, const gamestatus* game_status, const std::vector<team>* teams);

	//! Advances this unit to another type
	void advance_to(const unit_type* t, bool use_traits=false);
	const std::vector<std::string> advances_to() const { return advances_to_; }

	//! The current type id
	const std::string& id() const { return id_; }
	const unit_type* type() const;
	//! The actual name of the unit
	const std::string& name() const { if (description_.empty()) return language_name(); else return description_; }
	void rename(const std::string& name) { if (!unrenamable_) custom_unit_description_ = name; }
	//! The unit type name
	const std::string& description() const { return (custom_unit_description_); }
	const std::string& underlying_description() const { return underlying_description_; }
	const t_string& language_name() const { return language_name_; }
	const std::string& undead_variation() const { return undead_variation_; }
	//! The unit's profile
	const std::string& profile() const;
	//! Information about the unit -- a detailed description of it
	const std::string& unit_description() const { return cfg_["unit_description"]; }

	int hitpoints() const { return hit_points_; }
	int max_hitpoints() const { return max_hit_points_; }
	int experience() const { return experience_; }
	int max_experience() const { return maximum<int>(1,(max_experience_*unit_type::experience_accelerator::get_acceleration() + 50) / 100); }
	int level() const { return level_; }
	//! Adds 'xp' points to the units experience; returns true if advancement should occur
	bool get_experience(int xp) { experience_ += xp; return advances(); }
	SDL_Colour hp_color() const;
	SDL_Colour xp_color() const;
	//! Set to true for some scenario-specific units which should not be renamed
	bool unrenamable() const { return unrenamable_; }
	unsigned int side() const { return side_; }
	Uint32 team_rgb() const { return(team::get_side_rgb(side())); }
	const std::string& team_color() const { return flag_rgb_; }
	unit_race::GENDER gender() const { return gender_; }
	void set_side(unsigned int new_side) { side_ = new_side; }
	fixed_t alpha() const { return alpha_; }

	bool can_recruit() const { return utils::string_bool(cfg_["canrecruit"]); }
	bool incapacitated() const { return utils::string_bool(get_state("stoned"),false); }
	const std::vector<std::string>& recruits() const { return recruits_; }
	int total_movement() const { return max_movement_; }
	int movement_left() const { return incapacitated() ? 0 : movement_; }
	void set_hold_position(bool value) { hold_position_ = value; }
	bool hold_position() const { return hold_position_; }
	void set_user_end_turn(bool value=true) { end_turn_ = value; }
	bool user_end_turn() const { return end_turn_; }
	int attacks_left() const { return incapacitated() ? 0 : attacks_left_; }
	void set_movement(int moves);
	void set_attacks(int left) { attacks_left_ = maximum<int>(0,minimum<int>(left,max_attacks_)); }
	void unit_hold_position() { hold_position_ = end_turn_ = true; }
	void new_turn();
	void end_turn();
	void new_level();
	//! Called on every draw
	void refresh(const game_display& disp,const gamemap::location& loc) {
		if (state_ == STATE_IDLING && anim_ && anim_->animation_finished()) {
			set_standing(disp, loc);
			return;
		}
		if (state_ != STATE_STANDING || incapacitated() || (get_current_animation_tick() < next_idling_)) return;
		if (get_current_animation_tick() > next_idling_ + 1000) { // prevent all units animating at the same
			set_standing(disp,loc);
		} else {
			set_idling(disp, loc);
		}
	}

	bool take_hit(int damage) { hit_points_ -= damage; return hit_points_ <= 0; }
	void heal(int amount);
	void heal_all() { hit_points_ = max_hitpoints(); }
	bool resting() const { return resting_; }
	void set_resting(bool rest) { resting_ = rest; }

	const std::string get_state(const std::string& state) const;
	void set_state(const std::string& state, const std::string& value);

	bool has_moved() const { return movement_left() != total_movement(); }
	bool has_goto() const { return get_goto().valid(); }
	int emits_zoc() const { return (incapacitated()) ? false : emit_zoc_; }
	/* cfg: standard unit filter */
	bool matches_filter(const vconfig& cfg,const gamemap::location& loc,bool use_flat_tod=false) const;
	void add_overlay(const std::string& overlay) { overlays_.push_back(overlay); }
	void remove_overlay(const std::string& overlay) { overlays_.erase(std::remove(overlays_.begin(),overlays_.end(),overlay),overlays_.end()); }
	const std::vector<std::string>& overlays() const { return overlays_; }

	//! Initialize this unit from a cfg object.
	void read(const config& cfg, bool use_traits=true);
	void write(config& cfg) const;
	void write(config_writer& out) const;

	void assign_role(const std::string& role) { role_ = role; }
	void assign_ai_special(const std::string& s) { ai_special_ = s;}
            std::string get_ai_special() const { return(ai_special_); }
	const std::vector<attack_type>& attacks() const { return attacks_; }
	std::vector<attack_type>& attacks() { return attacks_; }

	int damage_from(const attack_type& attack,bool attacker,const gamemap::location& loc) const { return resistance_against(attack,attacker,loc); }

	//! A SDL surface, ready for display for place where we need a still-image of the unit.
	const surface still_image(bool scaled = false) const;
	void redraw_unit(game_display& disp, const gamemap::location& loc);
	//! Clear unit_halo_ and unit_anim_halo_
	void clear_haloes();


	void set_standing(const game_display& disp,const gamemap::location& loc, bool with_bars = true);
	void set_defending(const game_display &disp,const gamemap::location& loc, int damage,const attack_type* attack,const attack_type* secondary_attack,int swing_num);
	void set_leading(const game_display& disp,const gamemap::location& loc);
	void set_healing(const game_display& disp,const gamemap::location& loc,int damage);
	void set_leveling_in(const game_display& disp,const gamemap::location& loc);
	void set_leveling_out(const game_display& disp,const gamemap::location& loc);
	void set_teleporting (const game_display& disp,const gamemap::location& loc);
	void set_extra_anim(const game_display& disp,const gamemap::location& loc, std::string flag);
	void set_dying(const game_display &disp,const gamemap::location& loc,const attack_type* attack,const attack_type* secondary_attack);
	void set_walking(const game_display& disp,const gamemap::location& loc);
	void set_attacking( const game_display &disp,const gamemap::location& loc,int damage,const attack_type& type,const attack_type* secondary_attack,int swing_num);
	void set_recruited(const game_display& disp,const gamemap::location& loc);
	void set_healed(const game_display& disp,const gamemap::location& loc,int healing);
	void set_victorious(const game_display &disp,const gamemap::location& loc,const attack_type* attack,const attack_type* secondary_attack);
	void set_poisoned(const game_display& disp,const gamemap::location& loc,int damage);
	void set_idling(const game_display& disp,const gamemap::location& loc);
	void restart_animation(const game_display& disp,int start_time);
	const unit_animation* get_animation() const {  return anim_;};
	void set_offset(double offset){offset_ = offset;}
	void set_facing(gamemap::location::DIRECTION dir);
	gamemap::location::DIRECTION facing() const { return facing_; }

	std::set<gamemap::location> overlaps(const gamemap::location &loc) const;
	const t_string& traits_description() const { return traits_description_; }

	int value() const { return unit_value_; }
	int cost () const { return unit_value_; }

	const gamemap::location& get_goto() const { return goto_; }
	void set_goto(const gamemap::location& new_goto) { goto_ = new_goto; }

	int upkeep() const;

	void set_hidden(bool state);
	bool get_hidden() { return hidden_; };
	bool is_flying() const { return flying_; }
	bool is_fearless() const { return is_fearless_; }
	bool is_healthy() const { return is_healthy_; }
	int movement_cost(const t_translation::t_letter terrain) const;
	int defense_modifier(t_translation::t_letter terrain, int recurse_count=0) const;
	int resistance_against(const std::string& damage_name,bool attacker,const gamemap::location& loc) const;
	int resistance_against(const attack_type& damage_type,bool attacker,const gamemap::location& loc) const
		{return resistance_against(damage_type.type(), attacker, loc);};

	//return resistances without any abililities applied
	string_map get_base_resistances() const;
//		std::map<terrain_type::TERRAIN,int> movement_type() const;

	bool can_advance() const { return advances_to_.empty()==false || get_modification_advances().empty() == false; }
	bool advances() const { return experience_ >= max_experience() && can_advance(); }

    std::map<std::string,std::string> advancement_icons() const;
    std::vector<std::pair<std::string,std::string> > amla_icons() const;

	config::child_list get_modification_advances() const;
	const config::child_list& modification_advancements() const { return cfg_.get_children("advancement"); }

	size_t modification_count(const std::string& type, const std::string& id) const;

	void add_modification(const std::string& type, const config& modification,
	                  bool no_add=false);

	const t_string& modification_description(const std::string& type) const;

	bool move_interrupted() const { return movement_left() > 0 && interrupted_move_.x >= 0 && interrupted_move_.y >= 0; }
	const gamemap::location& get_interrupted_move() const { return interrupted_move_; }
	void set_interrupted_move(const gamemap::location& interrupted_move) { interrupted_move_ = interrupted_move; }

	//! States for animation.
	enum STATE { STATE_STANDING, STATE_ATTACKING, STATE_DEFENDING,
				STATE_LEADING, STATE_HEALING, STATE_WALKING,
				STATE_LEVELIN, STATE_LEVELOUT,
				STATE_DYING, STATE_EXTRA, STATE_TELEPORT,
				STATE_RECRUITED, STATE_HEALED, STATE_POISONED,
				STATE_IDLEIN, STATE_IDLING, STATE_VICTORIOUS};
	void start_animation(const game_display &disp, const gamemap::location &loc,const unit_animation* animation, bool with_bars,bool cycles=false);

	//! The name of the file to game_display (used in menus).
	const std::string& absolute_image() const { return cfg_["image"]; }
	const std::string& image_halo() const { return cfg_["halo"]; }
	const std::string& image_fighting(attack_type::RANGE range) const;

	const std::string& get_hit_sound() const { return cfg_["get_hit_sound"]; }
	const std::string& die_sound() const { return cfg_["die_sound"]; }
	const std::string& image_ellipse() const { return cfg_["ellipse"]; }

	const std::string& usage() const { return cfg_["usage"]; }
	unit_type::ALIGNMENT alignment() const { return alignment_; }
	const std::string& race() const { return race_->id(); }

	const unit_animation* choose_animation(const game_display& disp, const gamemap::location& loc,const std::string& event,const int damage=0,const unit_animation::hit_type hit_type = unit_animation::INVALID,const attack_type* attack=NULL,const attack_type* second_attack = NULL, int swing_num =0) const;

	bool get_ability_bool(const std::string& ability, const gamemap::location& loc) const;
	unit_ability_list get_abilities(const std::string& ability, const gamemap::location& loc) const;
	std::vector<std::string> ability_tooltips(const gamemap::location& loc) const;
	std::vector<std::string> unit_ability_tooltips() const;
	bool has_ability_type(const std::string& ability) const;

	void reset_modifications();
	void backup_state();
	void apply_modifications();
	void remove_temporary_modifications();
	void add_trait(std::string trait);
	void generate_traits(bool musthaveonly=false);
	void generate_traits_description();
	std::string generate_description() const { return race_->generate_name(string_gender(cfg_["gender"])); }

	// Only see_all=true use caching
	bool invisible(const gamemap::location& loc,
		const unit_map& units,const std::vector<team>& teams, bool see_all=true) const;

	unit_race::GENDER generate_gender(const unit_type& type, bool gen);
	std::string image_mods() const;

private:

	bool internal_matches_filter(const vconfig& cfg,const gamemap::location& loc,
		bool use_flat_tod) const;
	/*
	 * cfg: an ability WML structure
	 */
	bool ability_active(const std::string& ability,const config& cfg,const gamemap::location& loc) const;
	bool ability_affects_adjacent(const std::string& ability,const config& cfg,int dir,const gamemap::location& loc) const;
	bool ability_affects_self(const std::string& ability,const config& cfg,const gamemap::location& loc) const;
	bool resistance_filter_matches(const config& cfg,bool attacker,const std::string& damage_name) const;
	bool resistance_filter_matches(const config& cfg,bool attacker,const attack_type& damage_type) const
	{return resistance_filter_matches(cfg, attacker, damage_type.type()); };

	int movement_cost_internal(t_translation::t_letter terrain, int recurse_count=0) const;
	bool has_ability_by_id(const std::string& ability) const;
	void remove_ability_by_id(const std::string& ability);

	config cfg_;
	config movement_b_;
	config defense_b_;
	config resistance_b_;
		config abilities_b_;

	std::vector<std::string> advances_to_;
	std::string id_;
	const unit_race* race_;
	std::string name_;
	std::string description_;
	std::string custom_unit_description_;
	std::string underlying_description_;
	t_string language_name_;
	std::string undead_variation_;
	std::string variation_;

	int hit_points_;
	int max_hit_points_, max_hit_points_b_;
	int experience_;
	int max_experience_, max_experience_b_;
	int level_;
	unit_type::ALIGNMENT alignment_;
	std::string flag_rgb_;
	std::string image_mods_;

	bool unrenamable_;
	unsigned int side_;
	unit_race::GENDER gender_;

	fixed_t alpha_;

	std::vector<std::string> recruits_;

	int movement_;
	int max_movement_, max_movement_b_;
	mutable std::map<t_translation::t_letter, int> movement_costs_; // movement cost cache
	bool hold_position_;
	bool end_turn_;
	bool resting_;
	int attacks_left_;
	int max_attacks_;

	std::map<std::string,std::string> states_;
	config variables_;
	int emit_zoc_;
	STATE state_;

	std::vector<std::string> overlays_;

	std::string role_;
	std::string ai_special_;
	std::vector<attack_type> attacks_, attacks_b_;
	gamemap::location::DIRECTION facing_;

	t_string traits_description_;
	int unit_value_;
	gamemap::location goto_, interrupted_move_;
	bool flying_, is_fearless_, is_healthy_;

//		std::map<terrain_type::TERRAIN,int> movement_costs_, movement_costs_b_;
//		std::map<terrain_type::TERRAIN,int> defense_mods_, defense_mods_b_;

	string_map modification_descriptions_;
	// Animations:
	std::vector<unit_animation> animations_;

	unit_animation *anim_;
	int next_idling_;
	int frame_begin_time_;


	double offset_;
	int unit_halo_;
	int unit_anim_halo_;
	bool getsHit_;
	bool refreshing_; // avoid infinite recursion
	bool hidden_;
	bool draw_bars_;

	config modifications_;

	friend void attack_type::set_specials_context(const gamemap::location& loc,const unit& un) const;
	const game_data* gamedata_;
	const unit_map* units_;
	const gamemap* map_;
	const gamestatus* gamestatus_;
	const std::vector<team>* teams_;

	//! Hold the visibility status cache for a unit, mutable since it's a cache.
	mutable std::map<gamemap::location, bool> invisibility_cache_;

	//! Clears the cache.
	// Since we don't change the state of the object we're marked const
	// (also required since the objects in the cache need to be marked const).
	void clear_visibility_cache() const { invisibility_cache_.clear(); }
};

//! Object which temporarily resets a unit's movement
struct unit_movement_resetter
{
	unit_movement_resetter(unit& u, bool operate=true) : u_(u), moves_(u.movement_)
	{
		if(operate) {
			u.movement_ = u.total_movement();
		}
	}

	~unit_movement_resetter()
	{
		u_.movement_ = moves_;
	}

private:
	unit& u_;
	int moves_;
};

void sort_units(std::vector< unit > &);

int team_units(const unit_map& units, unsigned int team_num);
int team_upkeep(const unit_map& units, unsigned int team_num);
unit_map::const_iterator team_leader(unsigned int side, const unit_map& units);
unit_map::iterator find_visible_unit(unit_map& units,
		const gamemap::location loc,
		const gamemap& map,
		const std::vector<team>& teams, const team& current_team,
		bool see_all=false);
unit_map::const_iterator find_visible_unit(const unit_map& units,
		const gamemap::location loc,
		const gamemap& map,
		const std::vector<team>& teams, const team& current_team,
		bool see_all=false);

struct team_data
{
	int units, upkeep, villages, expenses, net_income, gold;
	std::string teamname;
};

team_data calculate_team_data(const class team& tm, int side, const unit_map& units);

const std::set<gamemap::location> vacant_villages(const std::set<gamemap::location>& villages, const unit_map& units);

// This object is used to temporary place a unit in the unit map,
// swapping out any unit that is already there.
// On destruction, it restores the unit map to its original .
struct temporary_unit_placer
{
	temporary_unit_placer(unit_map& m, const gamemap::location& loc, const unit& u);
	~temporary_unit_placer();

private:
	unit_map& m_;
	const gamemap::location& loc_;
	std::pair<gamemap::location,unit> *temp_;
};

//! Gets a checksum for a unit.
//! In MP games the descriptions are locally generated and might differ,
//! so it should be possible to discard them.
//! Not sure whether replays suffer the same problem.
//!
//! @param u                    the unit
//! @param discard_description  discards the descriptions for the checksum
//!
//! @returns                    the checksum for a unit
//!
std::string get_checksum(const unit& u, const bool discard_description = false);

#endif
