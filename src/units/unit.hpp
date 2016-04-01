/*
   Copyright (C) 2003 - 2016 by David White <dave@whitevine.net>
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
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/variant.hpp>

#include "units/types.hpp"
#include "units/ptr.hpp"
#include "units/id.hpp"

class display;
class gamemap;
#if defined(_MSC_VER) && _MSC_VER <= 1600
/*
	This is needed because msvc up to 2010 fails to correcty forward declare this struct as a return value this case.
	And will create corrupt binaries without giving a warning / error.
*/
#include <SDL_video.h>
#else
struct SDL_Color;
#endif
class team;
class unit_animation_component;
class unit_formula_manager;
class vconfig;

/// The things contained within a unit_ability_list.
typedef std::pair<const config *, map_location> unit_ability;
namespace unit_detail {
	template<typename T> const T& get_or_default(const boost::scoped_ptr<T>& v)
	{
		if(v) {
			return *v;
		}
		else {
			static const T def;
			return def;
		}
	}
}
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
			, const vconfig* vcfg = nullptr
			, n_unit::id_manager* id_manager = nullptr);

	/**
	  * Initializes a unit from a unit type
	  * only real_unit may have random traits, name and gender
	  * (to prevent OOS caused by RNG calls)
	  */
	unit(const unit_type& t, int side, bool real_unit,
		unit_race::GENDER gender = unit_race::NUM_GENDERS);
	virtual ~unit();

	void swap (unit &);

	unit& operator=(unit);


	/** Advances this unit to another type */
	void advance_to(const unit_type &t, bool use_traits = false);
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
	size_t underlying_id() const { return underlying_id_.value; }

	/** The unit type name */
	const t_string& type_name() const {return type_name_;}
	const std::string& undead_variation() const {return undead_variation_;}
	const std::string& variation() const {return variation_; }

	/** The unit name for display */
	const t_string &name() const { return name_; }
	void set_name(const t_string &name) { name_ = name; }
	void rename(const std::string& name) {if (!unrenamable_) name_= name;}

	/** The unit's profile */
	std::string small_profile() const;
	std::string big_profile() const;
	/** Information about the unit -- a detailed description of it */
	t_string unit_description() const { return description_; }

	int hitpoints() const { return hit_points_; }
	int max_hitpoints() const { return max_hit_points_; }
	void set_hitpoints(int hp) { hit_points_ = hp; }
	int experience() const { return experience_; }
	int max_experience() const { return max_experience_; }
	void set_experience(int xp) { experience_ = xp; }
	void set_recall_cost(int recall_cost) { recall_cost_ = recall_cost; }
	int level() const { return level_; }
	void set_level(int level) { level_ = level; }
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

	double hp_bar_scaling() const { return hp_bar_scaling_; }
	double xp_bar_scaling() const { return xp_bar_scaling_; }

	/** Set to true for some scenario-specific units which should not be renamed */
	bool unrenamable() const { return unrenamable_; }
	void set_unrenamable(bool unrenamable) { unrenamable_ = unrenamable; }
	int side() const { return side_; }
	const std::string& team_color() const { return flag_rgb_; }
	unit_race::GENDER gender() const { return gender_; }
	void set_side(unsigned int new_side) { side_ = new_side; }
	fixed_t alpha() const { return alpha_; }

	bool can_recruit() const { return canrecruit_; }
	void set_can_recruit(bool canrecruit) { canrecruit_ = canrecruit; }
	const std::vector<std::string>& recruits() const
		{ return recruit_list_; }
	void set_recruits(const std::vector<std::string>& recruits);
	const config& recall_filter() const { return filter_recall_; }

	bool poisoned() const { return get_state(STATE_POISONED); }
	bool incapacitated() const { return get_state(STATE_PETRIFIED); }
	bool slowed() const { return get_state(STATE_SLOWED); }

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
	const std::vector<std::string>& overlays() const { return overlays_; }

	void write(config& cfg) const;

	void set_role(const std::string& role) { role_ = role; }
	const std::string &get_role() const { return role_; }

	void set_emit_zoc(bool val) { emit_zoc_ = val; }
	bool get_emit_zoc() const { return emit_zoc_; }


	const std::vector<attack_type>& attacks() const { return attacks_; }
	std::vector<attack_type>& attacks() { return attacks_; }

	int damage_from(const attack_type& attack,bool attacker,const map_location& loc) const { return resistance_against(attack,attacker,loc); }

	unit_animation_component & anim_comp() const { return *anim_comp_; }
	void set_facing(map_location::DIRECTION dir) const;
	map_location::DIRECTION facing() const { return facing_; }

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
	
	struct upkeep_full {};
	struct upkeep_loyal {};
	typedef boost::variant<upkeep_full, upkeep_loyal, int> t_upkeep;

	t_upkeep upkeep_raw() const { return upkeep_; }
	void set_upkeep(t_upkeep v) { upkeep_ = v; }
	bool loyal() const;

	void set_hidden(bool state) const;
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

	typedef boost::ptr_vector<config> t_advancements;
	void set_advancements(std::vector<config> advancements);
	const t_advancements& modification_advancements() const
	{ return advancements_; }

	size_t modification_count(const std::string& type, const std::string& id) const;

	void add_modification(const std::string& type, const config& modification,
	                      bool no_add=false);
	void expire_modifications(const std::string & duration);
	
	static const std::set<std::string> builtin_effects;
	void apply_builtin_effect(std::string type, const config& effect);
	std::string describe_builtin_effect(std::string type, const config& effect);

	bool move_interrupted() const { return movement_left() > 0 && interrupted_move_.x >= 0 && interrupted_move_.y >= 0; }
	const map_location& get_interrupted_move() const { return interrupted_move_; }
	void set_interrupted_move(const map_location& interrupted_move) { interrupted_move_ = interrupted_move; }

	/** The name of the file to game_display (used in menus). */
	std::string absolute_image() const;

	/** The default image to use for animation frames with no defined image. */
	std::string default_anim_image() const;

	std::string image_halo() const { return unit_detail::get_or_default(halo_); }

	std::string image_ellipse() const { return unit_detail::get_or_default(ellipse_); }

	std::string usage() const { return unit_detail::get_or_default(usage_); }

	void set_image_halo(const std::string& halo);
	void set_image_ellipse(const std::string& ellipse) { ellipse_.reset(new std::string(ellipse)); }
	void set_usage(const std::string& usage) { usage_.reset(new std::string(usage)); }

	config &variables() { return variables_; }
	const config &variables() const { return variables_; }
	unit_type::ALIGNMENT alignment() const { return alignment_; }
	void set_alignment(unit_type::ALIGNMENT alignment) { alignment_ = alignment; }
	/// Never returns nullptr, but may point to the null race.
	const unit_race* race() const { return race_; }


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
	std::vector<boost::tuple<t_string,t_string,t_string> > ability_tooltips(std::vector<bool> *active_list=nullptr) const;
	std::vector<std::string> get_ability_list() const;
	bool has_ability_type(const std::string& ability) const;

	unit_formula_manager & formula_manager() const { return *formula_man_; }

	void backup_state();
	void apply_modifications();
	void generate_traits(bool musthaveonly=false);
	void generate_name();

	// Only see_all=true use caching
	bool invisible(const map_location& loc, bool see_all=true) const;

	bool is_visible_to_team(team const& team, gamemap const & map , bool const see_all = true) const;

	/** Mark this unit as clone so it can be inserted to unit_map
	 * @returns                   self (for convenience)
	 **/
	unit& clone(bool is_temporary=true);

	std::string TC_image_mods() const;
	const std::string& effect_image_mods() const;
	std::string image_mods() const;

	long ref_count() const { return ref_count_; }
	friend void intrusive_ptr_add_ref(const unit *);
	friend void intrusive_ptr_release(const unit *);
protected:
	mutable long ref_count_; //used by intrusive_ptr
private:
	/*
	 * cfg: an ability WML structure
	 */
	bool ability_active(const std::string& ability,const config& cfg,const map_location& loc) const;
	bool ability_affects_adjacent(const std::string& ability,const config& cfg,int dir,const map_location& loc,const unit& from) const;
	bool ability_affects_self(const std::string& ability,const config& cfg,const map_location& loc) const;
	bool resistance_filter_matches(const config& cfg,bool attacker,const std::string& damage_name, int res) const;

public:
	bool has_ability_by_id(const std::string& ability) const;
	// ^ Needed for unit_filter
private:
	void remove_ability_by_id(const std::string& ability);

	/** register a trait's name and its description for UI's use*/
	void add_trait_description(const config& trait, const t_string& description);

	void set_underlying_id(n_unit::id_manager& id_manager);

private:
	map_location loc_;

	std::vector<std::string> advances_to_;
	const unit_type * type_;/// Never nullptr. Adjusted for gender and variation.
	t_string type_name_;    /// The displayed name of the unit type.
	const unit_race* race_;	/// Never nullptr, but may point to the null race.
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
	unit_type::ALIGNMENT alignment_;
	std::string flag_rgb_;
	std::string image_mods_;

	bool unrenamable_;

	int side_;

	unit_race::GENDER gender_;

	fixed_t alpha_;

	boost::scoped_ptr<unit_formula_manager> formula_man_;

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

	std::vector<std::string> overlays_;

	std::string role_;
	std::vector<attack_type> attacks_;
protected:
	mutable map_location::DIRECTION facing_; //TODO: I think we actually consider this to be part of the gamestate, so it might be better if it's not mutable
						 //But it's not easy to separate this guy from the animation code right now.
private:
	std::vector<t_string> trait_names_;
	std::vector<t_string> trait_descriptions_;

	int unit_value_;
	map_location goto_, interrupted_move_;

	bool is_fearless_, is_healthy_;

	utils::string_map modification_descriptions_;
	// Animations:
	friend class unit_animation_component;

private:
	boost::scoped_ptr<unit_animation_component> anim_comp_;

	bool getsHit_;
	mutable bool hidden_;
	double hp_bar_scaling_, xp_bar_scaling_;

	config modifications_;
	config abilities_;
	t_advancements advancements_;
	t_string description_;
	boost::scoped_ptr<std::string> usage_;
	boost::scoped_ptr<std::string> halo_;
	boost::scoped_ptr<std::string> ellipse_;
	bool random_traits_;
	bool generate_name_;
	t_upkeep upkeep_;
	std::string profile_;
	std::string small_profile_;
	//TODO add a to initializer list.
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
