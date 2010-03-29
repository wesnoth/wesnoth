/* $Id$ */
/*
   Copyright (C) 2006 - 2009 by Rusty Russell <rusty@rustcorp.com.au>
   Copyright (C) 2010 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file unit_map.cpp */

#include "unit.hpp"
#include "unit_id.hpp"
#include "log.hpp"

#include <functional>

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define WRN_NG LOG_STREAM(warn, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)
#define DBG_NG LOG_STREAM(debug, log_engine)

unit_map::unit_map(const unit_map& that) :
	map_(),
	lmap_(),
	num_iters_(0),
	num_invalid_(0)
{
	for (const_unit_iterator i = that.begin(); i != that.end(); i++) {
		add(i->get_location(), *i);
	}
}

unit_map &unit_map::operator=(const unit_map &that)
{
	unit_map temp(that);
	swap(temp);
	return *this;
}

void unit_map::swap(unit_map &o)
{
	assert(num_iters_ == 0 && o.num_iters_ == 0);
	std::swap(map_, o.map_);
	std::swap(lmap_, o.lmap_);
	std::swap(num_invalid_, o.num_invalid_);
}

unit_map::~unit_map()
{
	clear();
}

unit_map::unit_iterator unit_map::find(const map_location &loc) {
	lmap::const_iterator i = lmap_.find(loc);
	if (i == lmap_.end()) {
		return unit_iterator(map_.end(), this);
	}

	umap::iterator iter = map_.find(i->second);

	assert(is_valid(iter));
	return unit_iterator(iter, this);
}

unit_map::unit_iterator unit_map::find(size_t id)
{
	umap::iterator iter = map_.find(id);
	if (!is_valid(iter)) iter = map_.end();
	return unit_iterator(iter, this);
}

unit_map::unit_iterator unit_map::begin() {
	// This call just needs to go somewhere that is likely to be
	// called when num_iters_ == 0. This seems as good a place as any.
	clean_invalid();

	umap::iterator i = map_.begin();
	while (i != map_.end() && !is_valid(i)) {
		++i;
	}

	return unit_iterator(i, this);
}

unit_map::const_unit_iterator unit_map::begin() const {
	umap::const_iterator i = map_.begin();
	while (i != map_.end() && !is_valid(i)) {
		++i;
	}

	return const_unit_iterator(i, this);
}

void unit_map::add(const map_location &l, const unit &u) {
	unit *p = new unit(u);
	p->set_location(l);
	insert(p);
}

void unit_map::move(const map_location &src, const map_location &dst) {
	unit *p = extract(src);
	assert(p);
	p->set_location(dst);
	insert(p);
}

void unit_map::insert(unit *p)
{
	size_t unit_id = p->underlying_id();
	const map_location &loc = p->get_location();

	if (!loc.valid()) {
		ERR_NG << "Trying to add " << p->name()
			<< " - " << p->id() << " at an invalid location; Discarding.\n";
		delete p;
		return;
	}

	std::pair<umap::iterator, bool> biter =
		map_.insert(std::make_pair(unit_id, p));

	p->set_game_context(this);

	if (biter.second) {
	} else if (!biter.first->second) {
		biter.first->second = p;
		--num_invalid_;
	} else {
		unit *q = biter.first->second;
		ERR_NG << "Trying to add " << p->name()
			<< " - " << p->id() << " - " << p->underlying_id()
			<< " ("  << loc << ") over " << q->name()
			<< " - " << q->id() << " - " << q->underlying_id()
			<< " ("  << q->get_location()
			<< "). The new unit will be assigned underlying_id="
			<< (1 + n_unit::id_manager::instance().get_save_id())
			<< " to prevent duplicate id conflicts.\n";
		p->clone(false);
		insert(p);
		return;
	}

	DBG_NG << "Adding unit " << p->underlying_id() << " - " << p->id()
		<< " to location: (" << loc.x + 1 << "," << loc.y + 1 << ")\n";

	std::pair<lmap::iterator,bool> res = lmap_.insert(std::make_pair(loc, unit_id));
	assert(res.second);
}

void unit_map::replace(const map_location &l, const unit &u)
{
	//when 'l' is the reference to map_location that is part
	//of that unit iterator which is to be deleted by erase,
	// 'l' is invalidated by erase, too. Thus, 'add(l,u)' fails.
	// So, we need to make a copy of that map_location.
	map_location loc = l;
	erase(loc);
	add(loc, u);
}

void unit_map::clear()
{
	assert(num_iters_ == 0);

	for (umap::iterator i = map_.begin(); i != map_.end(); ++i) {
		if (is_valid(i)) {
			DBG_NG << "Delete unit " << i->second->underlying_id() << "\n";
			delete i->second;
		}
	}

	lmap_.clear();
	map_.clear();
}

unit *unit_map::extract(const map_location &loc)
{
	lmap::iterator i = lmap_.find(loc);
	if (i == lmap_.end())
		return NULL;

	umap::iterator iter = map_.find(i->second);
	unit *res = iter->second;
	res->set_game_context(NULL);

	DBG_NG << "Extract unit " << i->second << '\n';
	iter->second = NULL;
	++num_invalid_;
	lmap_.erase(i);

	return res;
}

size_t unit_map::erase(const map_location &loc)
{
	unit *u = extract(loc);
	if (!u) return 0;
	delete u;
	return 1;
}

void unit_map::clean_invalid() {
	if (num_iters_ > 0 || num_invalid_ < lmap_.size())
		return;

	size_t num_cleaned = 0;

	umap::iterator iter = map_.begin();
	while (iter != map_.end()) {
		if (!is_valid(iter)) {
			map_.erase(iter++);
			++num_cleaned;
		} else {
			++iter;
		}
	}

	num_invalid_ -= num_cleaned;

	LOG_NG << "unit_map::clean_invalid - removed " << num_cleaned << " invalid map entries.\n";
}

unit_map::unit_iterator unit_map::find_leader(int side)
{
	unit_map::iterator i = begin(), i_end = end();
	for (; i != i_end; ++i) {
		if (static_cast<int>(i->side()) == side && i->can_recruit())
			return i;
	}
	return i_end;
}

unit_map::unit_iterator unit_map::find_first_leader(int side)
{
	unit_map::iterator i = begin(), i_end = end();
	unit_map::iterator first_leader = end();

	for (; i != i_end; ++i) {
		if (static_cast<int>(i->side()) == side && i->can_recruit()){
			if ((first_leader == end()) || (i->underlying_id() < first_leader->underlying_id()) )
				first_leader = i;
		}
	}
	return first_leader;
}
