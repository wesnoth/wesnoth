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

#include <cassert>
#include <set>
#include <string>
#include <vector>

class team
{
public:

	struct target {
		explicit target(config& cfg);
		config criteria;
		double value;
	};

	struct team_info
	{
		team_info(config& cfg);
		std::string name;
		std::string gold;
		std::set<std::string> can_recruit;
		std::vector<std::string> recruitment_pattern;
		double aggression;
		std::vector<int> enemies;
		bool human;

		double leader_value, village_value;

		std::vector<target> targets;
	};

	team(config& cfg, int gold=100);
	void get_tower(const gamemap::location&);
	void lose_tower(const gamemap::location&);
	int towers() const;
	bool owns_tower(const gamemap::location&) const;

	int gold() const;
	int income() const;
	void new_turn();
	void spend_gold(int amount);

	const std::set<std::string>& recruits() const;
	const std::vector<std::string>& recruitment_pattern() const;
	const std::string& name() const;

	bool is_enemy(int side) const;
	double aggression() const;

	bool is_human() const;

	double leader_value() const;
	double village_value() const;

	std::vector<target>& targets();
private:
	int gold_;
	std::set<gamemap::location> towers_;

	team_info info_;
};

#endif
