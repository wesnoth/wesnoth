/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

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
#include "team.hpp"
#include "unit_types.hpp"

#include <set>
#include <string>
#include <vector>

class unit
{
public:
	friend struct unit_movement_resetter;

	unit(game_data& data, const config& cfg);
	unit(const unit_type* t, int side, bool use_traits=false);

	//a constructor used when advancing a unit
	unit(const unit_type* t, const unit& u);
	const unit_type& type() const;
	std::string name() const;
	const std::string& description() const;
	const std::string& underlying_description() const;

	void rename(const std::string& new_description);

	int hitpoints() const;
	int max_hitpoints() const;
	int experience() const;
	int max_experience() const;
	bool get_experience(int xp);
	bool advances() const;
	int side() const;
	double alpha() const;
	void make_recruiter();
	bool can_recruit() const;
	int total_movement() const;
	int movement_left() const;
	bool can_attack() const;
	void set_movement(int moves);
	void set_attacked();
	void new_turn();
	void end_turn();
	void new_level();

	bool gets_hit(int damage);
	void heal();
	void heal(int amount);
	void heal_all();

	bool invisible(gamemap::TERRAIN terrain) const;

	bool matches_filter(const config& cfg) const;

	void set_flag(const std::string& flag);
	void remove_flag(const std::string& flag);
	bool has_flag(const std::string& flag) const;

	void read(game_data& data, const config& cfg);

	void write(config& cfg) const;

	void assign_role(const std::string& role);

	const std::vector<attack_type>& attacks() const;
	int longest_range() const;
	std::vector<attack_type> attacks_at_range(int range) const;

	int movement_cost(const gamemap& map, gamemap::TERRAIN terrain) const;
	int defense_modifier(const gamemap& map, gamemap::TERRAIN terrain) const;
	int damage_against(const attack_type& attack) const;

	//gets the unit image that should currently be displayed
	//(could be in the middle of an attack etc)
	const std::string& image() const;

	void set_defending(bool newval,
	                   attack_type::RANGE range=attack_type::LONG_RANGE);
	void set_attacking(bool newval, const attack_type* type=NULL, int ms=0);

	bool facing_left() const;
	void set_facing_left(bool newval);

	const std::string& traits_description() const;

	int value() const;
	bool is_guardian() const;

	const gamemap::location& get_goto() const;
	void set_goto(const gamemap::location& new_goto);

	int upkeep() const;

	void add_modification(const std::string& type, const config& modification,
	                      bool no_add=false);

private:
	const unit_type* type_;

	enum STATE { STATE_NORMAL, STATE_ATTACKING,
	             STATE_DEFENDING_LONG, STATE_DEFENDING_SHORT };
	STATE state_;
	const attack_type* attackType_;
	int attackingMilliseconds_;

	int hitpoints_;
	int maxHitpoints_, backupMaxHitpoints_;
	int experience_;
	int maxExperience_, backupMaxExperience_;

	int side_;

	//is set to the number of moves left, and -1 if the unit has attacked
	int moves_;
	bool facingLeft_;
	int maxMovement_, backupMaxMovement_;

	std::string underlying_description_, description_;

	bool recruit_;

	std::string role_;

	std::set<std::string> statusFlags_;

	std::vector<attack_type> attacks_;
	std::vector<attack_type> backupAttacks_;

	config modifications_;

	std::string traitsDescription_;

	bool guardian_;

	gamemap::location goto_;

	enum UPKEEP_COST { UPKEEP_FREE, UPKEEP_LOYAL, UPKEEP_FULL_PRICE };

	UPKEEP_COST upkeep_;

	void apply_modifications();
	void generate_traits();
};

//object which temporarily resets a unit's movement
struct unit_movement_resetter
{
	unit_movement_resetter(unit& u) : u_(u), moves_(u.moves_)
	{
		u.moves_ = u.total_movement();
	}

	~unit_movement_resetter()
	{
		u_.moves_ = moves_;
	}

private:
	unit& u_;
	int moves_;
};

struct compare_unit_values
{
	bool operator()(const unit& a, const unit& b) const;
};

typedef std::map<gamemap::location,unit> unit_map;

int team_units(const unit_map& units, int team_num);
int team_upkeep(const unit_map& units, int team_num);
unit_map::const_iterator team_leader(int side, const unit_map& units);
std::string team_name(int side, const unit_map& units);

struct team_data
{
	int units, upkeep, villages, expenses, net_income, gold;
};

team_data calculate_team_data(const team& tm, int side, const unit_map& units);

std::string get_team_name(int side, const unit_map& units);

#endif
