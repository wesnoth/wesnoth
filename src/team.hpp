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
#ifndef TEAM_H_INCLUDED
#define TEAM_H_INCLUDED

#include "config.hpp"
#include "map.hpp"

#include <set>
#include <string>
#include <vector>

#include "SDL.h"

//This class stores all the data for a single 'side' (in game nomenclature).
//e.g. there is only one leader unit per team.
class team
{
	class shroud_map {
	public:
		shroud_map() : enabled_(false) {}
		
		void place(size_t x, size_t y);
		bool clear(size_t x, size_t y);
		void reset();
		
		bool value(size_t x, size_t y) const;
		bool shared_value(const std::vector<const shroud_map*>& maps, size_t x, size_t y) const;
		
		bool copy_from(const std::vector<const shroud_map*>& maps);
		
		std::string write() const;
		void read(const std::string& shroud_data);
		
		bool enabled() const { return enabled_; }
		void set_enabled(bool enabled) { enabled_ = enabled; }	
	private:
		bool enabled_;
		std::vector<std::vector<bool> > data_;
	};
public:

	struct target {
		explicit target(const config& cfg);
		void write(config& cfg) const;
		config criteria;
		double value;
	};

	struct team_info
	{
		team_info(const config& cfg);
		void write(config& cfg) const;
		std::string name;
		std::string gold;
		std::string income;
		int income_per_village;
		std::set<std::string> can_recruit;
		std::vector<std::string> recruitment_pattern;
		std::vector<int> enemies;
		std::string team_name;
		std::string save_id;

		std::string flag;

		std::string description;

		enum CONTROLLER { HUMAN, AI, NETWORK, EMPTY };
		CONTROLLER controller;
		bool persistent;
		std::string ai_algorithm;

		std::vector<config> ai_params;

		int villages_per_scout;
		double leader_value, village_value;

		std::vector<target> targets;

		bool share_maps, share_view;

		std::string music;

		int colour;
	};

	team(const config& cfg, int gold=100);

	void write(config& cfg) const;

	void get_village(const gamemap::location&);
	void lose_village(const gamemap::location&);
	void clear_villages();
	const std::set<gamemap::location>& villages() const;
	bool owns_village(const gamemap::location&) const;

	int gold() const;
	int income() const;
	void new_turn();
	void set_time_of_day(int turn, const struct time_of_day& tod);
	void get_shared_maps();
	void spend_gold(int amount);
	void set_income(int amount);

	const std::set<std::string>& recruits() const;
	std::set<std::string>& recruits();
	const std::vector<std::string>& recruitment_pattern() const;
	const std::string& name() const;
	const std::string& save_id() const;

	bool is_enemy(int n) const {
		const size_t index = size_t(n-1);
		if(index < enemies_.size()) {
			return enemies_[index];
		} else {
			return calculate_enemies(index);
		}
	}

	double aggression() const;
	double caution() const;

	bool is_human() const;
	bool is_network() const;
	bool is_ai() const;
	bool is_empty() const;

	bool is_persistent() const;

	void make_human();
	void make_network();
	void make_ai();

	const std::string& team_name() const;
	void change_team(const std::string& name);

	const std::string& flag() const;

	const std::string& ai_algorithm() const;
	const config& ai_parameters() const;

	double leader_value() const;
	double village_value() const;

	int villages_per_scout() const;

	std::vector<target>& targets();

	//Returns true if the hex is shrouded/fogged for this side, or
	//any other ally with shared vision.
	bool shrouded(int x, int y) const;
	bool fogged(int x, int y) const;
	
	bool uses_shroud() const { return shroud_.enabled(); }
	bool uses_fog() const { return fog_.enabled(); }
	bool fog_or_shroud() const { return uses_shroud() || uses_fog(); }
	bool clear_shroud(int x, int y) { return shroud_.clear(x+1,y+1); }
	void place_shroud(int x, int y) { shroud_.place(x+1,y+1); }
	bool clear_fog(int x, int y) { return fog_.clear(x+1,y+1); }
	void refog() { fog_.reset(); }
	
	bool knows_about_team(size_t index) const;
	bool copy_ally_shroud();
	
	bool auto_shroud_updates() const { return auto_shroud_updates_; }
	void set_auto_shroud_updates(bool value) { auto_shroud_updates_ = value; }
	
	const std::string& music() const;

	int map_colour_to() const;

	static int nteams();

	//function which, when given a 1-based side will return the colour used by that side.
	static const SDL_Color& get_side_colour(int side);
	static int get_side_colour_index(int side);

private:
	//Make these public if you need them, but look at knows_about_team(...) first.
	bool share_maps() const { return info_.share_maps; }
	bool share_view() const { return info_.share_view; }
	
	const std::vector<const shroud_map*>& ally_shroud(const std::vector<team>& teams) const;
	const std::vector<const shroud_map*>& ally_fog(const std::vector<team>& teams) const;
	
	int gold_;
	std::set<gamemap::location> villages_;

	shroud_map shroud_, fog_;

	bool auto_shroud_updates_;

	team_info info_;

	config aiparams_;

	//cached values for ai parameters
	double aggression_, caution_;

	bool calculate_enemies(size_t index) const;
	bool calculate_is_enemy(size_t index) const;
	mutable std::vector<bool> enemies_;

	mutable std::vector<const shroud_map*> ally_shroud_, ally_fog_;
};

struct teams_manager {
	teams_manager(std::vector<team>& teams);
	~teams_manager();
};

bool is_observer();

//function which will validate a side. Throws gamestatus::game_error
//if the side is invalid
void validate_side(int side); //throw gamestatus::game_error

#endif
