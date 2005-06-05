/* $Id$ */
/*
Copyright (C) 2003 by David White <davidnwhite@comcast.net>
Part of the Battle for Wesnoth Project http://www.wesnoth.org/

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY.

See the COPYING file for more details.
*/

#ifndef ASTARNODE_H_INCLUDED
#define ASTARNODE_H_INCLUDED

#include "map.hpp"
#include "pathutils.hpp"
#include <set>
#include <vector>

#ifdef _PARANO_ASTAR_
# define assertParanoAstar(param) assert(param)
#else
# define assertParanoAstar(param) {}
#endif

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

class poss_a_star_node
{
private:
	typedef std::vector<a_star_node*> vect_page_a_star_node;

	vect_page_a_star_node vectPageAStarNode;
	size_t nbElemByPage;
	size_t capacity;
	size_t curIndex;

public:
	poss_a_star_node(void);
	void addPage(void);
	a_star_node* getAStarNode(void);
	void reduce(void);
};

class a_star_world
{
protected:
	typedef std::vector<a_star_node*> vect_a_star_node;

	vect_a_star_node _vectAStarNode;
	size_t _width;

public:
	size_t _nbNode;

	void resize_IFN(const size_t parWidth, const size_t parHeight);
	void clear(void);
	void erase(gamemap::location const &loc);
	a_star_node* getNodeFromLocation(gamemap::location const &loc, bool& isCreated);
	bool empty(void);
	bool reallyEmpty(void);
	a_star_world(void) : _width(0), _nbNode(0) {};
};

#endif

