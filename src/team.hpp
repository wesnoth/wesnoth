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
		double aggression;
		std::vector<int> enemies;

		enum CONTROLLER { HUMAN, AI, NETWORK };
		CONTROLLER controller;

		int villages_per_scout;
		double leader_value, village_value;

		std::vector<target> targets;

		bool use_shroud, use_fog;

		std::string music;
	};

	team(const config& cfg, int gold=100);

	void write(config& cfg) const;

	void get_tower(const gamemap::location&);
	void lose_tower(const gamemap::location&);
	const std::set<gamemap::location>& towers() const;
	bool owns_tower(const gamemap::location&) const;

	int gold() const;
	int income() const;
	void new_turn();
	void spend_gold(int amount);

	const std::set<std::string>& recruits() const;
	std::set<std::string>& recruits();
	const std::vector<std::string>& recruitment_pattern() const;
	const std::string& name() const;

	bool is_enemy(int side) const;
	double aggression() const;

	bool is_human() const;
	bool is_network() const;
	bool is_ai() const;

	double leader_value() const;
	double village_value() const;

	int villages_per_scout() const;

	std::vector<target>& targets();

	bool uses_shroud() const;
	bool shrouded(size_t x, size_t y) const;
	void clear_shroud(size_t x, size_t y);

	bool uses_fog() const;
	bool fogged(size_t x, size_t y) const;
	void clear_fog(size_t x, size_t y);
	void refog();

	const std::string& music() const;
private:
	int gold_;
	std::set<gamemap::location> towers_;

	std::vector<std::vector<bool> > shroud_;
	std::vector<std::vector<bool> > fog_;

	team_info info_;
};

#endif
