/* $Id$ */
/*
   Copyright (C) 2006 - 2007 by Rusty Russell <rusty@rustcorp.com.au>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file unit_map.cpp
//!

#include "unit.hpp"
#include "unit_map.hpp"

#include <cassert>

//! A unit map with a copy of a single unit in it.
unit_map::unit_map(const gamemap::location &loc, const unit &u)
{
	add(new std::pair<gamemap::location,unit>(loc, u));
}

unit_map::unit_map(const unit_map &that)
{
	*this = that;
}

unit_map &unit_map::operator =(const unit_map &that)
{
	clear();
	for (pmap::const_iterator i = that.map_.begin(); i != that.map_.end(); i++) {
		add(new std::pair<gamemap::location,unit>(*i->second));
	}
	return *this;
}

unit_map::~unit_map()
{
	delete_all();
}

// Due to unit <-> unit_map dependencies, must be out of line.
std::pair<gamemap::location,unit>& unit_map::iterator::operator*() const
{
	return *i_->second;
}
const std::pair<gamemap::location,unit>& unit_map::const_iterator::operator*() const
{
	return *i_->second;
}

void unit_map::add(std::pair<gamemap::location,unit> *p)
{
	std::pair<pmap::iterator,bool> res = map_.insert(std::pair<gamemap::location,std::pair<gamemap::location,unit>*>(p->first, p));
	assert(res.second);
}

void unit_map::replace(std::pair<gamemap::location,unit> *p)
{
	if (erase(p->first) != 1)
		assert(0);
	map_.insert(std::pair<gamemap::location,std::pair<gamemap::location,unit>*>(p->first, p));
}

void unit_map::delete_all()
{
	for (pmap::iterator i = map_.begin(); i != map_.end(); i++) {
		delete(i->second);
	}
}

//! Extract (like erase, but don't delete).
std::pair<gamemap::location,unit> *unit_map::extract(const gamemap::location &loc)
{
	pmap::iterator i = map_.find(loc);
	if (i == map_.end())
		return NULL;
	std::pair<gamemap::location,unit> *res = i->second;
	map_.erase(i);
	return res;
}

size_t unit_map::erase(const gamemap::location &loc)
{
	pmap::iterator i = map_.find(loc);
	if (i == map_.end())
		return 0;

	delete i->second;
	map_.erase(i);
	return 1;
}

void unit_map::erase(iterator pos)
{
	if (erase(pos->first) != 1)
		assert(0);
}

void unit_map::clear()
{
	delete_all();
	map_.clear();
}
