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

#include "global.hpp"
#include "astarnode.hpp"

#include <cassert>

poss_a_star_node* SingletonPOSS_AStarNode = NULL;

void a_star_node::initNode(const gamemap::location& pos, const gamemap::location& dst,
													double cost, a_star_node* parent,
													const std::set<gamemap::location>* teleports)
{	
	isInCloseList = false;
	loc = pos;
	nodeParent = parent;
	g = cost;
	h = heuristic(pos, dst);
	//if there are teleport locations, correct the heuristic to take them into account
	if (teleports != NULL) {
		double srch = h, dsth = h;
		std::set<gamemap::location>::const_iterator i;
		for(i = teleports->begin(); i != teleports->end(); ++i) {
			const double new_srch = heuristic(pos, *i);
			const double new_dsth = heuristic(*i, dst);
			if(new_srch < srch) {
				srch = new_srch;
			}
			if(new_dsth < dsth) {
				dsth = new_dsth;
			}
		}
		if(srch + dsth + 1.0 < h) {
			h = srch + dsth + 1.0;
		}
	}		
}

poss_a_star_node::poss_a_star_node(void) :
capacity(0), curIndex(0)
{
	nbElemByPage = size_t((4096 - 24) / sizeof(a_star_node));		
	assert(nbElemByPage > 0);
	addPage(); 
	SingletonPOSS_AStarNode = this;	
} 

void poss_a_star_node::addPage(void)
{
	a_star_node* locPageAStarNode;

	locPageAStarNode = new a_star_node[nbElemByPage];
	vectPageAStarNode.push_back(locPageAStarNode);
	capacity += nbElemByPage;
}	

a_star_node* poss_a_star_node::getAStarNode(void)
{
	//----------------- PRE_CONDITIONS ------------------
	assert(capacity > 0);
	assert(curIndex <= capacity);
	//---------------------------------------------------
	a_star_node* locPageAStarNode;
	
	if (curIndex == capacity)
		addPage();

	const size_t locIndexPage = curIndex / nbElemByPage; 
	const size_t locIndexInsidePage = curIndex % nbElemByPage; 
	++curIndex;

	assertParanoAstar(locIndexPage < vectPageAStarNode.size());
	locPageAStarNode = vectPageAStarNode[locIndexPage];	
	assertParanoAstar(locIndexInsidePage < nbElemByPage);
	return (&(locPageAStarNode[locIndexInsidePage]));
}

void poss_a_star_node::reduce(void)
{
	if (capacity > nbElemByPage)
	{
		for (vect_page_a_star_node::iterator iter = vectPageAStarNode.begin() + 1; iter != vectPageAStarNode.end(); ++iter)
			delete[] (*iter);
		vectPageAStarNode.resize(1);
		capacity = nbElemByPage;
	}	
	curIndex = 0;	
	//----------------- POST_CONDITIONS -----------------
	assert(capacity == nbElemByPage);
	assert(vectPageAStarNode.size() == 1);
	//---------------------------------------------------
} 

void a_star_world::resize_IFN(const size_t parWidth, const size_t parHeight)
{
	//----------------- PRE_CONDITIONS ------------------
	assert(_nbNode == 0);
	//---------------------------------------------------
	_width = parWidth;
	if (_vectAStarNode.size() != parWidth * parHeight)
	{
		_vectAStarNode.reserve(parWidth * parHeight);
		_vectAStarNode.resize(parWidth * parHeight);
	}
}

void a_star_world::clear(void)
{
	if (_nbNode > 0)
	{
		a_star_node* locNode = NULL;
		std::fill(_vectAStarNode.begin(), _vectAStarNode.end(), locNode);
		_nbNode = 0;
	}		
}

bool a_star_world::empty(void)
{	
	return (_nbNode == 0);
}

bool a_star_world::reallyEmpty(void)
{	
	for (vect_a_star_node::iterator iter = _vectAStarNode.begin(); iter != _vectAStarNode.end(); ++iter) 
	{
		if (*iter != NULL)
			return (false);
	}
	return (true);
}	

void a_star_world::erase(gamemap::location const &loc)
{
	//----------------- PRE_CONDITIONS ------------------
	assert(loc.valid());
	assert(loc.x + (loc.y * _width) < _vectAStarNode.size());
	//---------------------------------------------------
	_vectAStarNode[loc.x + (loc.y * _width)] = NULL;
}

a_star_node* a_star_world::getNodeFromLocation(gamemap::location const &loc, bool& isCreated)
{	
	//----------------- PRE_CONDITIONS ------------------
	assert(loc.valid());
	//---------------------------------------------------
	a_star_node*		node;
	size_t				index;

	index = size_t(loc.x + (loc.y * _width));
	assertParanoAstar(index < _vectAStarNode.size());
	node = _vectAStarNode[index];
	if (node == NULL)
	{
		isCreated = true;
		assert(SingletonPOSS_AStarNode != NULL);
		node = SingletonPOSS_AStarNode->getAStarNode();
		_vectAStarNode[index] = node;	
		++_nbNode;
	}
	else
		isCreated = false;
	return (node);
}
