/* $Id$ */
/*
Copyright (C) 2003 by David White <dave@whitevine.net>
Copyright (C) 2005 - 2009 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
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
	
	a_star_node() :
		g(0.0),
		h(0.0),
		loc(),
		nodeParent(0),
		isInCloseList(false)
	{
	}

	double g, h;  // g: already traveled time, h: estimated time still to travel
	map_location loc;
	a_star_node* nodeParent;
	bool isInCloseList;

	void initNode(map_location const &pos, map_location const &dst,
	              double cost, a_star_node *parent, std::set<map_location> const *teleports);

	inline double heuristic(const map_location& src, const map_location& dst)
	{
		// We will mainly use the distances in hexes
		// but we substract a tiny bonus for shorter Euclidean distance
		// based on how the path looks on the screen.
		// We must substract (and not add) to keep the heuristic 'admissible'.

		// 0.75 comes frome the horizontal hex imbrication
		double xdiff = (src.x - dst.x) * 0.75;
		// we must add 0.5 to the y coordinate when x is odd
		double ydiff = (src.y - dst.y) + ((src.x & 1) - (dst.x & 1)) * 0.5;

		// we assume a map with a maximum diagonal of 300 (bigger than a 200x200)
		// and we divide by 300 * 10000 to avoid interfering with the defense subcost
		// (see shortest_path_calculator::cost)
		return distance_between(src, dst) -
				(300.0 - sqrt( xdiff*xdiff + ydiff*ydiff)) / 3000000.0;
		
		// TODO: move the heuristic function into the cost_calculator
		// so we can use case-specific heuristic
		// and clean the definition of these numbers
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
	a_star_node* getNodeFromLocation(map_location const &loc, bool& isCreated);
	a_star_world();
	~a_star_world();
};

#endif

