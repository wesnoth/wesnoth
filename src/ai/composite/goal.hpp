/* $Id$ */
/*
   Copyright (C) 2009 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file ai/composite/goal.hpp
 */

#ifndef AI_COMPOSITE_GOAL_HPP_INCLUDED
#define AI_COMPOSITE_GOAL_HPP_INCLUDED

#include "../../global.hpp"

#include "../game_info.hpp"

#include <map>
#include <stack>
#include <vector>
#include <deque>


namespace ai {

class goal {
public:
	enum TYPE { VILLAGE, LEADER, EXPLICIT, THREAT, BATTLE_AID, MASS, SUPPORT };


	goal(const config &cfg);


	goal(const map_location &loc, double value, TYPE type=VILLAGE);


	virtual ~goal();


	bool matches_unit(unit_map::const_iterator u);


	config to_config() const;


	double value()
	{
		return value_;
	}

private:
	config cfg_;
	map_location loc_;
	double value_;
	TYPE type_;
};

} //end of namespace ai

#endif
