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

#include "global.hpp"
#include "astarnode.hpp"

#include <cassert>

void a_star_node::initNode(gamemap::location const &pos, gamemap::location const &dst,
                           double cost, a_star_node *parent, std::set<gamemap::location> const *teleports)
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

class a_star_world::poss_a_star_node
{
private:
	typedef std::vector<a_star_node*> vect_page_a_star_node;
	vect_page_a_star_node vectPageAStarNode_;
	size_t nbElemByPage_;
	size_t capacity_;
	size_t curIndex_;
	void addPage();

public:
	poss_a_star_node();
	~poss_a_star_node();
	a_star_node *getAStarNode();
	void clear();
};

void a_star_world::poss_a_star_node::addPage()
{
	vectPageAStarNode_.push_back(new a_star_node[nbElemByPage_]);
	capacity_ += nbElemByPage_;
}

a_star_world::poss_a_star_node::poss_a_star_node()
	: capacity_(0), curIndex_(0)
{
	nbElemByPage_ = size_t((4096 - 24) / sizeof(a_star_node));
	assert(nbElemByPage_ > 0);
	addPage();
}

a_star_world::poss_a_star_node::~poss_a_star_node()
{
	for(vect_page_a_star_node::iterator iter = vectPageAStarNode_.begin(),
	                                    iend = vectPageAStarNode_.end();
	    iter != iend; ++iter)
		delete[] *iter;
}

a_star_node *a_star_world::poss_a_star_node::getAStarNode()
{
	//----------------- PRE_CONDITIONS ------------------
	assert(capacity_ > 0);
	assert(curIndex_ <= capacity_);
	//---------------------------------------------------
	if (curIndex_ == capacity_)
		addPage();
	size_t i = curIndex_++;
	return &vectPageAStarNode_[i / nbElemByPage_][i % nbElemByPage_];
}

void a_star_world::poss_a_star_node::clear()
{
	if (capacity_ > nbElemByPage_) {
		for(vect_page_a_star_node::iterator iter = vectPageAStarNode_.begin() + 1,
		                                    iend = vectPageAStarNode_.end();
		    iter != iend; ++iter)
			delete[] *iter;
		vectPageAStarNode_.resize(1);
		capacity_ = nbElemByPage_;
	}
	curIndex_ = 0;
	//----------------- POST_CONDITIONS -----------------
	assert(capacity_ == nbElemByPage_);
	assert(vectPageAStarNode_.size() == 1);
	//---------------------------------------------------
}

a_star_world::a_star_world()
	: pool_(new poss_a_star_node), width_(0), nbNode_(0)
{
}

a_star_world::~a_star_world()
{
	delete pool_;
}

void a_star_world::resize_IFN(size_t parWidth, size_t parHeight)
{
	//----------------- PRE_CONDITIONS ------------------
	assert(nbNode_ == 0);
	//---------------------------------------------------
	width_ = parWidth;
	size_t sz = parWidth * parHeight;
	if (vectAStarNode_.size() == sz)
		return;
	vectAStarNode_.reserve(sz);
	vectAStarNode_.resize(sz);
}

void a_star_world::clear(void)
{
	a_star_node *locNode = NULL;
	std::fill(vectAStarNode_.begin(), vectAStarNode_.end(), locNode);
	nbNode_ = 0;
	pool_->clear();
}

a_star_node *a_star_world::getNodeFromLocation(gamemap::location const &loc, bool &isCreated)
{
	//----------------- PRE_CONDITIONS ------------------
	assert(loc.valid());
	assert(loc.x + loc.y * width_ < vectAStarNode_.size());
	//---------------------------------------------------

	a_star_node *&node = vectAStarNode_[loc.x + loc.y * width_];
	if ((isCreated = (node == NULL))) {
		node = pool_->getAStarNode();
		++nbNode_;
	}
	return node;
}
