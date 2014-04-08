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

/** @file */

#ifndef UNIT_H_INCLUDED
#define UNIT_H_INCLUDED

#include <boost/tuple/tuple.hpp>
#include <boost/scoped_ptr.hpp>

#include "formula_callable.hpp"
#include "portrait.hpp"
#include "resources.hpp"
#include "unit_animation.hpp"
#include "unit_types.hpp"
#include "unit_map.hpp"

class display;
class vconfig;
class team;

/// The things contained within a unit_ability_list.
typedef std::pair<const config *, map_location> unit_ability;

class unit_ability_list
{
public:
	unit_ability_list() :
		cfgs_()
	{
	}

	// Implemented in unit_abilities.cpp:
	std::pair<int,map_location> highest(const std::string& key, int def=0) const;
	std::pair<int,map_location> lowest(const std::string& key, int def=0) const;

	// The following make this class usable with BOOST_FOREACH:
	typedef std::vector<unit_ability>::iterator       iterator;
	typedef std::vector<unit_ability>::const_iterator const_iterator;
	iterator       begin()        { return cfgs_.begin(); }
	const_iterator begin() const  { return cfgs_.begin(); }
	iterator       end()          { return cfgs_.end();   }
	const_iterator end()   const  { return cfgs_.end();   }

	// Vector access:
	bool                 empty() const  { return cfgs_.empty(); }
	unit_ability &       front()        { return cfgs_.front(); }
	const unit_ability & front() const  { return cfgs_.front(); }
	unit_ability &       back()         { return cfgs_.back();  }
	const unit_ability & back()  const  { return cfgs_.back();  }

	iterator erase(const iterator & erase_it)  { return cfgs_.erase(erase_it); }
	void push_back(const unit_ability & ability)  { cfgs_.push_back(ability); }

private:
	// Data:
	std::vector<unit_ability> cfgs_;
};


class unit
{
public:
	/**
	 * Clear the unit status cache for all units. Currently only the hidden
	 * status of units is cached this way.
	 */
	static void clear_status_caches();

	/** The path to the leader crown overlay. */
	static const std::string& leader_crown();

	// Copy constructor
	unit(const unit& u);

	/** Initializes a unit from a config */
	explicit unit(
			  const config& cfg
			, bool use_traits = false
			, const vconfig* vcfg = NULL);

	/**
	  * Initializes a unit from a unit type
	  * only real_unit may have random traits, name and gender
	  * (to prevent OOS caused by RNG calls)
	  */
	unit(const unit_type& t, int side, bool real_unit,
		unit_race::GENDER gender = unit_race::NUM_GENDERS);
	virtual ~unit();
	virtual unit& operator=(const unit&);


	/** Advances this unit to another type */
	void advance_to(const unit_type &t, bool use_traits = false)
	{
		advance_to(cfg_, t, use_traits);
	}
	const std::vector<std::string>& advances_to() const { return advances_to_; }
	const std::vector<std::string> advances_to_translated() const;
	void set_advances_to(const std::vector<std::string>& advances_to);

	/**
	 * The id of the type of the unit.
	 * If you are dealing with creating units (e.g. recruitment), this is not what
	 * you want, as a variation can change this; use type().base_id() instead.
	 */
	const std::string& type_id() const { return type_->id(); }
	/** The type of the unit (accounting for gender and variation). */
	const unit_type& type() const { return *type_; }

	/** id assigned by wml */
	void set_id(const std::string& id) { id_ = id; }
	const std::string& id() const { if (id_.empty()) return type_name(); else return id_; }
	/** The unique internal ID of the unit */
	size_t underlying_id() const { return underlying_id_; }

	/** The unit type name */
	const t_string& type_name() const {return type_name_;}
	const std::string& undead_variation() const {return undead_variation_;}

	/** The unit name for display */
	const t_string &name() const { return name_; }
	void set_name(const t_string &name) { name_ = name; }
	void rename(const std::string& name) {if (!unrenamable_) name_= name;}

	/** The unit's profile */
	std::string small_profile() const;
	std::string big_profile() const;
	/** Information about the unit -- a detailed description of it */
	t_string unit_description() const { return cfg_["description"]; }

	int hitpoints() const { return hit_points_; }
	int max_hitpoints() const { return max_hit_points_; }
	void set_hitpoints(int hp) { hit_points_ = hp; }
	int experience() const { return experience_; }
	int max_experience() const { return max_experience_; }
	void set_experience(int xp) { experience_ = xp; }
	void set_recall_cost(int recall_cost) { recall_cost_ = recall_cost; }
	int level() const { return level_; }
	int recall_cost() const { return recall_cost_; }
	void remove_movement_ai();
	void remove_attacks_ai();

	/** Colors for the unit's *current* hitpoints.
	 * @returns a color between green and red representing
	 * how wounded the unit is.
	 * The maximum_hitpoints are considered as base.
	 */
	SDL_Color hp_color() const;
	/** Colors for the unit's hitpoints.
	 * @param hitpoints the amount of hitpoints the color represents.
	 * @returns the color considering the current hitpoints as base.
	 */
	SDL_Color hp_color(int hitpoints) const;
	/** Colors for the unit's XP. */
	SDL_Color xp_color() const;
	/** Set to true for some scenario-specific units which should not be renamed */
	bool unrenamable() const { return unrenamable_; }
	void set_unrenamable(bool unrenamable) { unrenamable_ = unrenamable; }
	int side() const { return side_; }
	std::string side_id() const;
	const std::string& team_color() const { return flag_rgb_; }
	unit_race::GENDER gender() const { return gender_; }
	void set_side(unsigned int new_side) { side_ = new_side; }
	fixed_t alpha() const { return alpha_; }

	bool can_recruit() const { return canrecruit_; }
	void set_can_recurit(bool canrecruit) { canrecruit_ = canrecruit; }
	const std::vector<std::string>& recruits() const
		{ return recruit_list_; }
	void set_recruits(const std::vector<std::string>& recruits);
	const config& recall_filter() const { return filter_recall_; }

	bool incapacitated() const { return get_state(STATE_PETRIFIED); }
	int total_movement() const { return max_movement_; }
	/// Returns how far a unit can move this turn (zero if incapacitated).
	int movement_left() const { return (movement_ == 0 || incapacitated()) ? 0 : movement_; }
	/// Providing a true parameter to movement_left() causes it to ignore incapacitation.
	int movement_left(bool base_value) const { return base_value ? movement_ : movement_left(); }
	int vision() const { return vision_ < 0 ? max_movement_ : vision_; }
	int jamming() const { return jamming_; }
	void toggle_hold_position() { hold_position_ = !hold_position_; if ( hold_position_ ) end_turn_ = true; }
	bool hold_position() const { return hold_position_; }
	void set_user_end_turn(bool value=true) { end_turn_ = value; }
	void toggle_user_end_turn() { end_turn_ = !end_turn_; if ( !end_turn_ ) hold_position_ = false; }
	bool user_end_turn() const { return end_turn_; }
	int attacks_left() const { return (attacks_left_ == 0 || incapacitated()) ? 0 : attacks_left_; }
	int max_attacks() const { return max_attacks_; }
	void set_movement(int moves, bool unit_action=false);
	void set_attacks(int left) { attacks_left_ = std::max<int>(0, left); }
	void new_turn();
	void end_turn();
	void new_scenario();
	/** Called on every draw */
	void refresh();

	bool take_hit(int damage) { hit_points_ -= damage; return hit_points_ <= 0; }
	void heal(int amount);
	void heal_all() { hit_points_ = max_hitpoints(); }
	bool resting() const { return resting_; }
	void set_resting(bool rest) { resting_ = rest; }

	const std::map<std::string,std::string> get_states() const;
	bool get_state(const std::string& state) const;
	void set_state(const std::string &state, bool value);
	enum state_t { STATE_SLOWED = 0, STATE_POISONED, STATE_PETRIFIED,
		STATE_UNCOVERED, STATE_NOT_MOVED, STATE_UNHEALABLE, STATE_GUARDIAN, STATE_UNKNOWN = -1 };
	void set_state(state_t state, bool value);
	bool get_state(state_t state) const;
	static state_t get_known_boolean_state_id(const std::string &state);
	static std::map<std::string, state_t> get_known_boolean_state_names();

	bool has_moved() const { return movement_left() != total_movement(); }
	bool has_goto() const { return get_goto().valid(); }
	bool emits_zoc() const { return emit_zoc_ && !incapacitated();}
	bool matches_id(const std::string& unit_id) const;
	/* cfg: standard unit filter */
	bool matches_filter(const vconfig& cfg,const map_location& loc,bool use_flat_tod=false) const;
	/// Determine if *this matches @a filter at its current location.
	/// (Only use for units currently on the map; otherwise use the overload
	/// that takes a location, possibly with a null location.)
	bool matches_filter(const vconfig& filter, bool use_flat_tod=false) const
	{ return matches_filter(filter, get_location(), use_flat_tod); }
	const std::vector<std::string>& overlays() const { return overlays_; }

	void write(config& cfg) const;

	void set_role(const std::string& role) { role_ = role; }
	const std::string &get_role() const { return role_; }

	const std::vector<attack_type>& attacks() const { return attacks_; }
	std::vector<attack_type>& attacks() { return attacks_; }

	int damage_from(const attack_type& attack,bool attacker,const map_location& loc) const { return resistance_against(attack,attacker,loc); }

	/** A SDL surface, ready for display for place where we need a still-image of the unit. */
	const surface still_image(bool scaled = false) const;

	/** draw a unit.  */
	void redraw_unit();
	/** Clear unit_halo_  */
	void clear_haloes();

	void set_standing(bool with_bars = true);

	void set_ghosted(bool with_bars = true);
	void set_disabled_ghosted(bool with_bars = true);

	void set_idling();
	void set_selecting();
	unit_animation* get_animation() {  return anim_.get();}
	const unit_animation* get_animation() const {  return anim_.get();}
	void set_facing(map_location::DIRECTION dir);
	map_location::DIRECTION facing() const { return facing_; }

	bool invalidate(const map_location &loc);
	const std::vector<t_string>& trait_names() const { return trait_names_; }
	const std::vector<t_string>& trait_descriptions() const { return trait_descriptions_; }
	std::vector<std::string> get_traits_list() const;

	int cost () const { return unit_value_; }

	const map_location &get_location() const { return loc_; }
	/** To be called by unit_map or for temporary units only. */
	void set_location(const map_location &loc) { loc_ = loc; }

	const map_location& get_goto() const { return goto_; }
	void set_goto(const map_location& new_goto) { goto_ = new_goto; }

	int upkeep() const;
	bool loyal() const;

	void set_hidden(bool state);
	bool get_hidden() const { return hidden_; }
	bool is_flying() const { return movement_type_.is_flying(); }
	bool is_fearless() const { return is_fearless_; }
	bool is_healthy() const { return is_healthy_; }
	int movement_cost(const t_translation::t_terrain & terrain) const
	{ return movement_type_.movement_cost(terrain, get_state(STATE_SLOWED)); }
	int vision_cost(const t_translation::t_terrain & terrain) const
	{ return movement_type_.vision_cost(terrain, get_state(STATE_SLOWED)); }
	int jamming_cost(const t_translation::t_terrain & terrain) const
	{ return movement_type_.jamming_cost(terrain, get_state(STATE_SLOWED)); }
	int defense_modifier(const t_translation::t_terrain & terrain) const;
	int resistance_against(const std::string& damage_name,bool attacker,const map_location& loc) const;
	int resistance_against(const attack_type& damage_type,bool attacker,const map_location& loc) const
	{ return resistance_against(damage_type.type(), attacker, loc); }

	//return resistances without any abilities applied
	utils::string_map get_base_resistances() const { return movement_type_.damage_table(); }
	const movetype & movement_type() const { return movement_type_; }

	bool can_advance() const { return advances_to_.empty()==false || get_modification_advances().empty() == false; }
	bool advances() const { return experience_ >= max_experience() && can_advance(); }

    std::map<std::string,std::string> advancement_icons() const;
    std::vector<std::pair<std::string,std::string> > amla_icons() const;

	std::vector<config> get_modification_advances() const;
	config::const_child_itors modification_advancements() const
	{ return cfg_.child_range("advancement"); }

	size_t modification_count(const std::string& type, const std::string& id) const;

	void add_modification(const std::string& type, const config& modification,
	                      bool no_add=false);
	void expire_modifications(const std::string & duration);

	bool move_interrupted() const { return movement_left() > 0 && interrupted_move_.x >= 0 && interrupted_move_.y >= 0; }
	const map_location& get_interrupted_move() const { return interrupted_move_; }
	void set_interrupted_move(const map_location& interrupted_move) { interrupted_move_ = interrupted_move; }

	/** States for animation. */
	enum STATE {
		STATE_STANDING,   /** anim must fit in a hex */
		STATE_FORGET,     /** animation will be automatically replaced by a standing anim when finished */
		STATE_ANIM};      /** normal anims */
	void start_animation(int start_time, const unit_animation *animation,
		bool with_bars,  const std::string &text = "",
		Uint32 text_color = 0, STATE state = STATE_ANIM);

	/** The name of the file to game_display (used in menus). */
	std::string absolute_image() const;
	std::string image_halo() const { return cfg_["halo"]; }

	std::string image_ellipse() const { return cfg_["ellipse"]; }

	config &variables() { return variables_; }
	const config &variables() const { return variables_; }

	std::string usage() const { return cfg_["usage"]; }
	unit_type::ALIGNMENT alignment() const { return alignment_; }
	/// Never returns NULL, but may point to the null race.
	const unit_race* race() const { return race_; }

	const unit_animation* choose_animation(const display& disp,
		       	const map_location& loc, const std::string& event,
		       	const map_location& second_loc = map_location::null_location,
			const int damage=0,
			const unit_animation::hit_type hit_type = unit_animation::INVALID,
			const attack_type* attack=NULL,const attack_type* second_attack = NULL,
			int swing_num =0) const;

	/**
	 * Returns true if the unit is currently under effect by an ability with this given TAG NAME.
	 * This means that the ability could be owned by the unit itself, or by an adjacent unit.
	 */
	bool get_ability_bool(const std::string& tag_name, const map_location& loc) const;
	/**
	 * Returns true if the unit is currently under effect by an ability with this given TAG NAME.
	 * This means that the ability could be owned by the unit itself, or by an adjacent unit.
	 */
	bool get_ability_bool(const std::string &tag_name) const
	{ return get_ability_bool(tag_name, loc_); }
	unit_ability_list get_abilities(const std::string &tag_name, const map_location& loc) const;
	unit_ability_list get_abilities(const std::string &tag_name) const
	{ return get_abilities(tag_name, loc_); }
	/** Tuple of: neutral ability name, gendered ability name, description */
	std::vector<boost::tuple<t_string,t_string,t_string> > ability_tooltips(std::vector<bool> *active_list=NULL) const;
	std::vector<std::string> get_ability_list() const;
	bool has_ability_type(const std::string& ability) const;

	const game_logic::map_formula_callable_ptr& formula_vars() const { return formula_vars_; }
	void add_formula_var(std::string str, variant var);
	bool has_formula() const { return !unit_formula_.empty(); }
	bool has_loop_formula() const { return !unit_loop_formula_.empty(); }
	bool has_priority_formula() const { return !unit_priority_formula_.empty(); }
	const std::string& get_formula() const { return unit_formula_; }
	const std::string& get_loop_formula() const { return unit_loop_formula_; }
	const std::string& get_priority_formula() const { return unit_priority_formula_; }

	void backup_state();
	void apply_modifications();
	void generate_traits(bool musthaveonly=false);
	void generate_name();

	// Only see_all=true use caching
	bool invisible(const map_location& loc, bool see_all=true) const;

	bool is_visible_to_team(team const& team, bool const see_all = true, gamemap const& map = *resources::game_map) const;

	/** Mark this unit as clone so it can be inserted to unit_map
	 * @returns                   self (for convenience)
	 **/
	unit& clone(bool is_temporary=true);

	std::string TC_image_mods() const;
	const std::string& effect_image_mods() const;
	std::string image_mods() const;

	/**
	 * Gets the portrait for a unit.
	 *
	 * @param size                The size of the portrait.
	 * @param side                The side the portrait is shown on.
	 *
	 * @returns                   The portrait with the wanted size.
	 * @retval NULL               The wanted portrait doesn't exist.
	 */
	const tportrait* portrait(
		const unsigned size, const tportrait::tside side) const;

private:
	void advance_to(const config &old_cfg, const unit_type &t,
		bool use_traits);

	bool internal_matches_filter(const vconfig& cfg,const map_location& loc,
		bool use_flat_tod) const;
	/*
	 * cfg: an ability WML structure
	 */
	bool ability_active(const std::string& ability,const config& cfg,const map_location& loc) const;
	bool ability_affects_adjacent(const std::string& ability,const config& cfg,int dir,const map_location& loc) const;
	bool ability_affects_self(const std::string& ability,const config& cfg,const map_location& loc) const;
	bool resistance_filter_matches(const config& cfg,bool attacker,const std::string& damage_name, int res) const;

	bool has_ability_by_id(const std::string& ability) const;
	void remove_ability_by_id(const std::string& ability);

	/** register a trait's name and its description for UI's use*/
	void add_trait_description(const config& trait, const t_string& description);

	void set_underlying_id();

	config cfg_;
	map_location loc_;

	std::vector<std::string> advances_to_;
	const unit_type * type_;/// Never NULL. Adjusted for gender and variation.
	t_string type_name_;    /// The displayed name of the unit type.
	const unit_race* race_;	/// Never NULL, but may point to the null race.
	std::string id_;
	t_string name_;
	size_t underlying_id_;
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
	unit_type::ALIGNMENT alignment_;
	std::string flag_rgb_;
	std::string image_mods_;

	bool unrenamable_;
	int side_;
	const unit_race::GENDER gender_;

	fixed_t alpha_;

	std::string unit_formula_;
	std::string unit_loop_formula_;
	std::string unit_priority_formula_;
	game_logic::map_formula_callable_ptr formula_vars_;

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
	std::vector<bool> known_boolean_states_;
	static std::map<std::string, state_t> known_boolean_state_names_;
	config variables_;
	config events_;
	config filter_recall_;
	bool emit_zoc_;
	STATE state_;

	std::vector<std::string> overlays_;

	std::string role_;
	std::vector<attack_type> attacks_;
	map_location::DIRECTION facing_;

	std::vector<t_string> trait_names_;
	std::vector<t_string> trait_descriptions_;

	int unit_value_;
	map_location goto_, interrupted_move_;

	bool is_fearless_, is_healthy_;

	utils::string_map modification_descriptions_;
	// Animations:
	std::vector<unit_animation> animations_;

	boost::scoped_ptr<unit_animation> anim_;
	int next_idling_;
	int frame_begin_time_;


	int unit_halo_;
	bool getsHit_;
	bool refreshing_; // avoid infinite recursion
	bool hidden_;
	bool draw_bars_;
	double hp_bar_scaling_, xp_bar_scaling_;

	config modifications_;

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
	void clear_visibility_cache() const { invisibility_cache_.clear(); }
};

/**
 * Object which temporarily resets a unit's movement.
 *
 * @warning
 * The unit whose movement is reset may not be deleted while a
 * @ref unit_movement_resetter object 'holds'. So best use it only in a small
 * scope.
 */
struct unit_movement_resetter
	: private boost::noncopyable
{
	unit_movement_resetter(unit& u, bool operate=true);
	~unit_movement_resetter();

private:
	unit& u_;
	int moves_;
};

/// Used to find units in vectors by their ID. (Convenience wrapper)
std::vector<unit>::iterator find_if_matches_id(
		std::vector<unit> &unit_list,
		const std::string &unit_id);
/// Used to find units in vectors by their ID. (Convenience wrapper)
std::vector<unit>::const_iterator find_if_matches_id(
		const std::vector<unit> &unit_list,
		const std::string &unit_id);
/// Used to erase units from vectors by their ID. (Convenience wrapper)
std::vector<unit>::iterator erase_if_matches_id(
		std::vector<unit> &unit_list,
		const std::string &unit_id);

/** Returns the number of units of the side @a side_num. */
int side_units(int side_num);

/** Returns the total cost of units of side @a side_num. */
int side_units_cost(int side_num);

int side_upkeep(int side_num);

unit_map::iterator find_visible_unit(const map_location &loc,
	const team &current_team, bool see_all = false);

unit *get_visible_unit(const map_location &loc,
	const team &current_team, bool see_all = false);

struct team_data
{
	team_data() :
		units(0),
		upkeep(0),
		villages(0),
		expenses(0),
		net_income(0),
		gold(0),
		teamname()
	{
	}

	int units, upkeep, villages, expenses, net_income, gold;
	std::string teamname;
};

team_data calculate_team_data(const class team& tm, int side);

/**
 * This object is used to temporary place a unit in the unit map, swapping out
 * any unit that is already there.  On destruction, it restores the unit map to
 * its original.
 */
struct temporary_unit_placer
{
	temporary_unit_placer(unit_map& m, const map_location& loc, unit& u);
	virtual  ~temporary_unit_placer();

private:
	unit_map& m_;
	const map_location loc_;
	unit *temp_;
};

/**
 * This object is used to temporary remove a unit from the unit map.
 * On destruction, it restores the unit map to its original.
 * unit_map iterators to this unit must not be accessed while the unit is temporarily
 * removed, otherwise a collision will happen when trying to reinsert the unit.
 */
struct temporary_unit_remover
{
	temporary_unit_remover(unit_map& m, const map_location& loc);
	virtual  ~temporary_unit_remover();

private:
	unit_map& m_;
	const map_location loc_;
	unit *temp_;
};


/**
 * This object is used to temporary move a unit in the unit map, swapping out
 * any unit that is already there.  On destruction, it restores the unit map to
 * its original.
 */
struct temporary_unit_mover
{
	temporary_unit_mover(unit_map& m, const map_location& src,
	                     const map_location& dst, int new_moves);
	temporary_unit_mover(unit_map& m, const map_location& src,
	                     const map_location& dst);
	virtual  ~temporary_unit_mover();

private:
	unit_map& m_;
	const map_location src_;
	const map_location dst_;
	int old_moves_;
	unit *temp_;
};

/**
 * Gets a checksum for a unit.
 *
 * In MP games the descriptions are locally generated and might differ, so it
 * should be possible to discard them.  Not sure whether replays suffer the
 * same problem.
 *
 *  @param u                    the unit
 *
 *  @returns                    the checksum for a unit
 */
std::string get_checksum(const unit& u);

#endif
