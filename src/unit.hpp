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

#include <set>
#include <string>
#include <vector>

class unit;
class display;

typedef std::map<gamemap::location,unit> unit_map;

class unit
{
public:
	friend struct unit_movement_resetter;

	unit(const unit& u) { *this=u ; unit_halo_ = 0; }
	unit(const game_data& data, const config& cfg);
	unit(const unit_type* t, int side, bool use_traits=false, bool dummy_unit=false, unit_race::GENDER gender=unit_race::MALE);

	//a constructor used when advancing a unit
	unit(const unit_type* t, const unit& u);
	virtual ~unit();
	const unit_type& type() const;
	std::string name() const;
	const std::string& description() const;
	const std::string& underlying_description() const;
	const std::string& profile() const;

	//information about the unit -- a detailed description of it
	const std::string& unit_description() const;

	void rename(const std::string& new_description);
        static const int UNIT_ID_MAX;

	int hitpoints() const;
	int max_hitpoints() const;
  SDL_Colour hp_color() const;
	int experience() const;
  SDL_Colour xp_color() const;
	int max_experience() const;
	bool get_experience(int xp);
	bool unrenamable() const; /** < Set to true for some scenario-specific units which should not be renamed */
	bool advances() const;
	unsigned int side() const;
        Uint32 team_rgb() const;
        std::vector<Uint32> team_rgb_range() const;
	unit_race::GENDER gender() const;
	void set_side(unsigned int new_side);
	fixed_t alpha() const;
	void make_recruiter();
	bool can_recruit() const;
	int total_movement() const;
	int movement_left() const;
	void set_hold_position(bool value);
	bool hold_position() const;
	void set_user_end_turn(bool value=true);
	bool user_end_turn() const;
	bool can_attack() const;
	void set_movement(int moves);
	void set_attacked();
	void unit_hold_position();
	void end_unit_turn();
	void new_turn();
	void end_turn();
	void new_level();

	void set_resting(bool resting);
	bool is_resting() const;

	bool gets_hit(int damage);
	void heal();
	void heal(int amount);
	void heal_all();

	bool invisible(const std::string& terrain, int lawful_bonus,
			const gamemap::location& loc,
			const unit_map& units,const std::vector<team>& teams) const;
	bool poisoned() const;
	bool slowed() const;
	bool stone() const;

	bool incapacitated() const;
	bool healable() const;

	bool has_moved() const;
	bool has_goto() const;

	bool emits_zoc() const;

	bool matches_filter(const config& cfg) const;

	void set_flag(const std::string& flag);
	void remove_flag(const std::string& flag);
	bool has_flag(const std::string& flag) const;

	void add_overlay(const std::string& overlay);
	void remove_overlay(const std::string& overlay);
	const std::vector<std::string>& overlays() const;

	/**
	 * Initializes this unit from a cfg object.
	 *
	 * \param data The global game_data object
	 * \param cfg  Configuration object from which to read the unit
	 */
	void read(const game_data& data, const config& cfg);

	void write(config& cfg) const;

	void assign_role(const std::string& role);

	const std::vector<attack_type>& attacks() const;

	int movement_cost(const gamemap& map, gamemap::TERRAIN terrain) const;
	int defense_modifier(const gamemap& map, gamemap::TERRAIN terrain) const;
	int damage_against(const attack_type& attack) const;

	//gets the unit image that should currently be displayed
	//(could be in the middle of an attack etc)
	const std::string& absolute_image() const {return type_->image();}
	const image::locator image_loc() const;
	void refresh_unit(display& disp,const int& x,const int& y,const double& submerge);

	void set_standing();
	void set_defending(bool hits, std::string range, int start_frame, int acceleration);
	void update_frame();
	void set_attacking( const attack_type* type=NULL, int ms=0);

	void set_leading();
	void set_healing();
	void set_walking(const std::string terrain,gamemap::location::DIRECTION,int acceleration);

	bool facing_left() const;
	void set_facing_left(bool newval);

	const t_string& traits_description() const;

	int value() const;
	bool is_guardian() const;

	const gamemap::location& get_goto() const;
	void set_goto(const gamemap::location& new_goto);

	int upkeep() const;

	bool is_flying() const;

	bool can_advance() const;

        std::map<std::string,std::string> advancement_icons() const;
        std::vector<std::pair<std::string,std::string> > amla_icons() const;

	config::child_list get_modification_advances() const;

	size_t modification_count(const std::string& type, const std::string& id) const;

	void add_modification(const std::string& type, const config& modification,
	                      bool no_add=false);

	const t_string& modification_description(const std::string& type) const;

	bool move_interrupted() const;
	const gamemap::location& get_interrupted_move() const;
	void set_interrupted_move(const gamemap::location& interrupted_move);
	//is set to the number of moves left, ATTACKED if attacked,
	// MOVED if moved and then pressed "end turn"
	// NOT_MOVED if not moved and pressed "end turn"
	enum MOVES { ATTACKED=-1, MOVED=-2, NOT_MOVED=-3 };
	enum STATE { STATE_NORMAL, STATE_ATTACKING, STATE_DEFENDING,
		STATE_LEADING, STATE_HEALING, STATE_WALKING};
	STATE state() const {return state_;}
private:
	const std::string& image() const;
	unit_race::GENDER generate_gender(const unit_type& type, bool use_genders);
	unit_race::GENDER gender_;
	std::string variation_;

	const unit_type* type_;

	STATE state_;
	const attack_type* attackType_;
	int attackingMilliseconds_;
	bool getsHit_;

	int hitpoints_;
	int maxHitpoints_, backupMaxHitpoints_;
	int experience_;
	int maxExperience_, backupMaxExperience_;

	unsigned int side_;

	int moves_;
	bool user_end_turn_;
	bool facingLeft_;
	int maxMovement_, backupMaxMovement_;
	bool resting_;
	bool hold_position_;

	std::string underlying_description_, description_, profile_;

	//this field is used if the scenario creator places a custom unit description
	//with a certain unit. If this field is empty, then the more general unit description
	//from the unit's base type will be used
	std::string custom_unit_description_;

	bool recruit_;

	std::string role_;

	std::set<std::string> statusFlags_;
	std::vector<std::string> overlays_;

	//this field stores user-variables associated with the unit
	config variables_;

	std::vector<attack_type> attacks_;
	std::vector<attack_type> backupAttacks_;

	config modifications_;

	t_string traitsDescription_;

	string_map modificationDescriptions_;

	bool guardian_;

	gamemap::location goto_, interrupted_move_;

	enum UPKEEP_COST { UPKEEP_FREE, UPKEEP_LOYAL, UPKEEP_FULL_PRICE };

	UPKEEP_COST upkeep_;

	bool unrenamable_;

	unit_animation *anim_;

	void reset_modifications();
	void apply_modifications();
	void remove_temporary_modifications();
	void generate_traits();
	void generate_traits_description();
	int unit_halo_;
	int unit_anim_halo_;
		
};

//object which temporarily resets a unit's movement
struct unit_movement_resetter
{
	unit_movement_resetter(unit& u, bool operate=true) : u_(u), moves_(u.moves_)
	{
		if(operate) {
			u.moves_ = u.total_movement();
		}
	}

	~unit_movement_resetter()
	{
		u_.moves_ = moves_;
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
		const gamemap& map, int lawful_bonus,
		const std::vector<team>& teams, const team& current_team);
unit_map::const_iterator find_visible_unit(const unit_map& units,
		const gamemap::location loc,
		const gamemap& map, int lawful_bonus,
		const std::vector<team>& teams, const team& current_team);

struct team_data
{
	int units, upkeep, villages, expenses, net_income, gold;
};

team_data calculate_team_data(const class team& tm, int side, const unit_map& units);

std::string get_team_name(unsigned int side, const unit_map& units);

const std::set<gamemap::location> vacant_villages(const std::set<gamemap::location>& villages, const unit_map& units);

//this object is used to temporary place a unit in the unit map, swapping out any unit
//that is already there. On destruction, it restores the unit map to its original state.
struct temporary_unit_placer
{
	temporary_unit_placer(unit_map& m, const gamemap::location& loc, const unit& u);
	~temporary_unit_placer();

private:
	unit_map& m_;
	const gamemap::location& loc_;
	const unit temp_;
	bool use_temp_;
};

#endif
