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
#include "unit.hpp"

#include <cassert>
#include <set>
#include <string>
#include <vector>

class team
{
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
		double aggression, caution;
		std::vector<int> enemies;
		std::string team_name;

		std::string description;

		enum CONTROLLER { HUMAN, AI, NETWORK };
		CONTROLLER controller;
		std::string ai_algorithm;
		config ai_params;

		int villages_per_scout;
		double leader_value, village_value;

		std::vector<target> targets;

		bool use_shroud, use_fog, share_maps;

		std::string music;
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
	void get_shared_maps();
	void spend_gold(int amount);
	void set_income(int amount);

	const std::set<std::string>& recruits() const;
	std::set<std::string>& recruits();
	const std::vector<std::string>& recruitment_pattern() const;
	const std::string& name() const;

	bool is_enemy(int side) const;
	double aggression() const;
	double caution() const;

	bool is_human() const;
	bool is_network() const;
	bool is_ai() const;

	void make_human();
	void make_network();
	void make_ai();

	const std::string& team_name() const;
	void change_team(const std::string& name);

	const std::string& ai_algorithm() const;
	const config& ai_parameters() const;

	double leader_value() const;
	double village_value() const;

	int villages_per_scout() const;

	std::vector<target>& targets();

	bool uses_shroud() const;
	bool shrouded(size_t x, size_t y) const;
	bool clear_shroud(size_t x, size_t y);

	bool uses_fog() const;
	bool fogged(size_t x, size_t y) const;
	bool clear_fog(size_t x, size_t y);
	void refog();

	bool auto_shroud_updates() const { return auto_shroud_updates_; }
	void set_auto_shroud_updates(bool value) { auto_shroud_updates_ = value; }
	
	const std::string& music() const;

	static int nteams();

private:
	int gold_;
	std::set<gamemap::location> villages_;

	typedef std::vector<std::vector<bool> > shroud_map;
	shroud_map shroud_, fog_;

	bool auto_shroud_updates_;

	team_info info_;
};

struct teams_manager {
	teams_manager(std::vector<team>& teams);
	~teams_manager();
};

bool is_observer();

#endif
