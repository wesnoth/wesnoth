/* $Id$ */
/*
Copyright (C) 2003 by David White <dave@whitevine.net>
Copyright (C) 2005 - 2008 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
Part of the Battle for Wesnoth Project http://www.wesnoth.org/

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License version 2
or at your option any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY.

See the COPYING file for more details.
*/

#ifndef ASTARNODE_H_INCLUDED
#define ASTARNODE_H_INCLUDED

#include "pathutils.hpp"
#include <set>

struct a_star_node
{
public:
	double g, h;  // g: already traveled time, h: estimated time still to travel
	gamemap::location loc;
	a_star_node* nodeParent;
	bool isInCloseList;

	void initNode(gamemap::location const &pos, gamemap::location const &dst,
	              double cost, a_star_node *parent, std::set<gamemap::location> const *teleports);

	inline double heuristic(const gamemap::location& src, const gamemap::location& dst)
	{
		return distance_between(src, dst);
	}
};

class a_star_world
{
	class poss_a_star_node;
	poss_a_star_node *pool_;
	typedef std::vector<a_star_node*> vect_a_star_node;
	vect_a_star_node vectAStarNode_;
	size_t width_, nbNode_;

public:
	void resize_IFN(size_t parWidth, size_t parHeight);
	void clear();
	a_star_node* getNodeFromLocation(gamemap::location const &loc, bool& isCreated);
	a_star_world();
	~a_star_world();
};

#endif

