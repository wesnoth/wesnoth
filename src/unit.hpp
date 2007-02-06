/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef UNIT_H_INCLUDED
#define UNIT_H_INCLUDED

#include "config.hpp"
#include "map.hpp"
#include "race.hpp"
#include "team.hpp"
#include "unit_types.hpp"
#include "image.hpp"
#include "unit_map.hpp"

class unit;
class display;
class gamestatus;

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
	private:

};


class unit
{
	public:
		friend struct unit_movement_resetter;
		// Copy constructor
		unit(const unit& u);
		// Initilizes a unit from a config
		unit(const game_data& gamedata, const config& cfg);
		unit(const game_data* gamedata, unit_map* unitmap, const gamemap* map, const gamestatus* game_status, const std::vector<team>* teams, const config& cfg);
		// Initilizes a unit from a unit type
		unit(const unit_type* t, int side, bool use_traits=false, bool dummy_unit=false, unit_race::GENDER gender=unit_race::MALE);
		unit(const game_data* gamedata, unit_map* unitmap, const gamemap* map, const gamestatus* game_status, const std::vector<team>* teams, const unit_type* t, int side, bool use_traits=false, bool dummy_unit=false, unit_race::GENDER gender=unit_race::MALE);
		virtual ~unit();
		unit& operator=(const unit&);

		void set_game_context(const game_data* gamedata, unit_map* unitmap, const gamemap* map, const gamestatus* game_status, const std::vector<team>* teams);
		void write_checksum(std::string& str) const;

		// Advances this unit to another type
		void advance_to(const unit_type* t);
		const std::vector<std::string> advances_to() const;

		// the current type id
		const std::string& id() const;
		const unit_type& type() const;
		// the actual name of the unit
		const std::string& name() const;
		void rename(const std::string& name);
		// the unit type name
		const std::string& description() const;
		const std::string& underlying_description() const;
		const t_string& language_name() const;
		const std::string& undead_variation() const;
		// the unit's profile
		const std::string& profile() const;
		//information about the unit -- a detailed description of it
		const std::string& unit_description() const;

		int hitpoints() const;
		int max_hitpoints() const;
		int experience() const;
		int max_experience() const;
		int level() const;
		// adds 'xp' points to the units experience; returns true if advancement should occur
		bool get_experience(int xp);
		SDL_Colour hp_color() const;
		SDL_Colour xp_color() const;
		bool unrenamable() const; /** < Set to true for some scenario-specific units which should not be renamed */
		unsigned int side() const;
        Uint32 team_rgb() const;
        std::vector<Uint32> team_rgb_range() const;
        const std::vector<Uint32>& flag_rgb() const;
        const std::string& team_color() const;
		unit_race::GENDER gender() const;
		void set_side(unsigned int new_side);
		fixed_t alpha() const;

		bool can_recruit() const;
		bool incapacitated() const;
		const std::vector<std::string>& recruits() const;
		int total_movement() const;
		int movement_left() const;
		void set_hold_position(bool value);
		bool hold_position() const;
		void set_user_end_turn(bool value=true);
		bool user_end_turn() const;
		int attacks_left() const;
		void set_movement(int moves);
		void set_attacks(int left);
		void unit_hold_position();
		void end_unit_turn();
		void new_turn();
		void end_turn();
		void new_level();
		void refresh() {if(anim_ && !refreshing_) anim_->update_current_frame(); }

		bool take_hit(int damage);
		void heal(int amount);
		void heal_all();
		bool resting() const;
		void set_resting(bool rest);

		const std::string get_state(const std::string& state) const;
		void set_state(const std::string& state, const std::string& value);

		bool has_moved() const;
		bool has_goto() const;
		int emits_zoc() const;
		/* cfg: standard unit filter */
		bool matches_filter(const config& cfg,const gamemap::location& loc,bool use_flat_tod=false) const;
		void add_overlay(const std::string& overlay);
		void remove_overlay(const std::string& overlay);
		const std::vector<std::string>& overlays() const;
		/**
		* Initializes this unit from a cfg object.
		*
		* \param cfg  Configuration object from which to read the unit
		*/
		void read(const config& cfg);
		void write(config& cfg) const;

		void assign_role(const std::string& role);
		const std::vector<attack_type>& attacks() const;
		std::vector<attack_type>& attacks();

		int damage_from(const attack_type& attack,bool attacker,const gamemap::location& loc) const;

		// a sdl surface, ready for display for place where we need a fix image of the unit
		const surface still_image() const;
		void redraw_unit(display& disp,gamemap::location hex);

		void set_standing(const display& disp,const gamemap::location& loc, bool with_bars = true);
		void set_defending(const display &disp,const gamemap::location& loc, int damage,const attack_type* attack);
		void set_leading(const display& disp,const gamemap::location& loc);
		void set_healing(const display& disp,const gamemap::location& loc);
		void set_leveling_in(const display& disp,const gamemap::location& loc);
		void set_leveling_out(const display& disp,const gamemap::location& loc);
		void set_teleporting (const display& disp,const gamemap::location& loc);
		void set_extra_anim(const display& disp,const gamemap::location& loc, std::string flag);
		void set_dying(const display &disp,const gamemap::location& loc,const attack_type* attack);
		void set_walking(const display& disp,const gamemap::location& loc);
		const unit_animation & set_attacking(const display& disp,const gamemap::location& loc,bool hit,const attack_type& type);
		void set_recruited(const display& disp,const gamemap::location& loc);
		void set_healed(const display& disp,const gamemap::location& loc,int healing);
		void set_poisoned(const display& disp,const gamemap::location& loc,int damage);
		void restart_animation(const display& disp,int start_time);
		const unit_animation* get_animation() const {  return anim_;};
		void set_offset(double offset){offset_ = offset;}
		void set_facing(gamemap::location::DIRECTION);
		gamemap::location::DIRECTION facing() const;

		std::set<gamemap::location> overlaps(const gamemap::location &loc) const;
		const t_string& traits_description() const;

		int value() const;
		int cost() const;

		const gamemap::location& get_goto() const;
		void set_goto(const gamemap::location& new_goto);

		int upkeep() const;

		void set_hidden(bool state) {hidden_ = state;};
		bool get_hidden() { return hidden_; };
		bool is_flying() const;
		int movement_cost(gamemap::TERRAIN terrain) const;
		int defense_modifier(gamemap::TERRAIN terrain, int recurse_count=0) const;
		int resistance_against(const attack_type& damage_type,bool attacker,const gamemap::location& loc) const;
//		std::map<gamemap::TERRAIN,int> movement_type() const;

		bool can_advance() const;
		bool advances() const;

        std::map<std::string,std::string> advancement_icons() const;
        std::vector<std::pair<std::string,std::string> > amla_icons() const;

		config::child_list get_modification_advances() const;
		const config::child_list& modification_advancements() const;

		size_t modification_count(const std::string& type, const std::string& id) const;

		void add_modification(const std::string& type, const config& modification,
	                      bool no_add=false);

		const t_string& modification_description(const std::string& type) const;

		bool move_interrupted() const;
		const gamemap::location& get_interrupted_move() const;
		void set_interrupted_move(const gamemap::location& interrupted_move);

		enum STATE { STATE_STANDING, STATE_ATTACKING, STATE_DEFENDING,
		STATE_LEADING, STATE_HEALING, STATE_WALKING, STATE_LEVELIN,
		STATE_LEVELOUT, STATE_DYING, STATE_EXTRA, STATE_TELEPORT,
		STATE_RECRUITED, STATE_HEALED, STATE_POISONED};
		STATE state() const;

		//the name of the file to display (used in menus
		const std::string& absolute_image() const;
		const std::string& image_halo() const;
		const std::string& image_profile() const;
		const std::string& image_fighting(attack_type::RANGE range) const;
		const std::string& image_healing() const;
		const std::string& image_halo_healing() const;
		const std::string& get_hit_sound() const;
		const std::string& die_sound() const;
		const std::string& image_ellipse() const;

		const std::string& usage() const;
		unit_type::ALIGNMENT alignment() const;
		const std::string& race() const;

		const defensive_animation& defend_animation(const std::string &terrain,
				fighting_animation::hit_type hits,const attack_type* attack) const;
		const unit_animation& teleport_animation() const;
		const unit_animation* extra_animation(const std::string &terrain,const std::string &flag) const;
		const death_animation& die_animation(const std::string &terrain,
				fighting_animation::hit_type hits,const attack_type* attack) const;
		const movement_animation& move_animation(const std::string terrain,gamemap::location::DIRECTION) const;
		const standing_animation& stand_animation(const std::string terrain,gamemap::location::DIRECTION) const;
		const leading_animation& lead_animation(const std::string terrain,gamemap::location::DIRECTION) const;
		const healing_animation& heal_animation(const std::string terrain,gamemap::location::DIRECTION) const;

		bool get_ability_bool(const std::string& ability, const gamemap::location& loc) const;
		unit_ability_list get_abilities(const std::string& ability, const gamemap::location& loc) const;
		std::vector<std::string> ability_tooltips(const gamemap::location& loc) const;
		std::vector<std::string> unit_ability_tooltips() const;
		bool has_ability_type(const std::string& ability) const;

		void reset_modifications();
		void backup_state();
		void apply_modifications();
		void remove_temporary_modifications();
		void generate_traits();
		void generate_traits_description();
		std::string generate_description() const;

		bool invisible(const gamemap::location& loc,
			const unit_map& units,const std::vector<team>& teams) const;

		unit_race::GENDER generate_gender(const unit_type& type, bool gen);

	private:

		/*
		 * cfg: an ability WML structure
		 */
		bool ability_active(const std::string& ability,const config& cfg,const gamemap::location& loc) const;
		bool ability_affects_adjacent(const std::string& ability,const config& cfg,int dir,const gamemap::location& loc) const;
		bool ability_affects_self(const std::string& ability,const config& cfg,const gamemap::location& loc) const;
		bool resistance_filter_matches(const config& cfg,bool attacker,const attack_type& damage_type) const;
		int movement_cost_internal(gamemap::TERRAIN terrain, int recurse_count=0) const;
		bool has_ability_by_id(const std::string& ability) const;

		config cfg_;
		config movement_b_;
		config defense_b_;
		config resistance_b_;

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

		bool unrenamable_;
		unsigned int side_;
		unit_race::GENDER gender_;

		fixed_t alpha_;

		std::vector<std::string> recruits_;

		int movement_;
		int max_movement_, max_movement_b_;
		mutable std::map<gamemap::TERRAIN,int> movement_costs_; // movement cost cache
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
		std::vector<attack_type> attacks_, attacks_b_;
		gamemap::location::DIRECTION facing_;

		t_string traits_description_;
		int unit_value_;
		gamemap::location goto_, interrupted_move_;
		bool flying_;

//		std::map<gamemap::TERRAIN,int> movement_costs_, movement_costs_b_;
//		std::map<gamemap::TERRAIN,int> defense_mods_, defense_mods_b_;

		string_map modification_descriptions_;
		std::vector<defensive_animation> defensive_animations_;

		std::vector<unit_animation> teleport_animations_;

		std::multimap<std::string,unit_animation> extra_animations_;

		std::vector<death_animation> death_animations_;

		std::vector<movement_animation> movement_animations_;

		std::vector<standing_animation> standing_animations_;

		std::vector<leading_animation> leading_animations_;

		std::vector<healing_animation> healing_animations_;
		unit_animation *anim_;
		int frame_begin_time;
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

};

//object which temporarily resets a unit's movement
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
std::string team_name(int side, const unit_map& units);
unit_map::iterator find_visible_unit(unit_map& units,
		const gamemap::location loc,
		const gamemap& map,
		const std::vector<team>& teams, const team& current_team);
unit_map::const_iterator find_visible_unit(const unit_map& units,
		const gamemap::location loc,
		const gamemap& map,
		const std::vector<team>& teams, const team& current_team);

struct team_data
{
	int units, upkeep, villages, expenses, net_income, gold;
};

team_data calculate_team_data(const class team& tm, int side, const unit_map& units);

std::string get_team_name(unsigned int side, const unit_map& units);

const std::set<gamemap::location> vacant_villages(const std::set<gamemap::location>& villages, const unit_map& units);

//this object is used to temporary place a unit in the unit map, swapping out any unit
//that is already there. On destruction, it restores the unit map to its original .
struct temporary_unit_placer
{
	temporary_unit_placer(unit_map& m, const gamemap::location& loc, const unit& u);
	~temporary_unit_placer();

private:
	unit_map& m_;
	const gamemap::location& loc_;
	std::pair<gamemap::location,unit> *temp_;
};

#endif
