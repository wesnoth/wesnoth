/*
   Copyright (C) 2006 - 2009 by Rusty Russell <rusty@rustcorp.com.au>
   Copyright (C) 2010 - 2017 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file */

#include "log.hpp"
#include "units/id.hpp"
#include "units/unit.hpp"

#include "units/map.hpp"
#include <functional>

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define WRN_NG LOG_STREAM(warn, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)
#define DBG_NG LOG_STREAM(debug, log_engine)

unit_map::unit_map()
	: umap_()
	, lmap_()
{
}

unit_map::unit_map(const unit_map& that)
	: umap_()
	, lmap_()
{
	for(const_unit_iterator i = that.begin(); i != that.end(); ++i) {
		add(i->get_location(), *i);
	}
}

unit_map& unit_map::operator=(const unit_map& that)
{
	unit_map temp(that);
	swap(temp);
	return *this;
}

void unit_map::swap(unit_map& o)
{
	assert(num_iters() == 0 && o.num_iters() == 0);

	std::swap(umap_, o.umap_);
	std::swap(lmap_, o.lmap_);
}

unit_map::~unit_map()
{
	clear(true);
}

unit_map::umap::iterator unit_map::begin_core() const
{
	self_check();

	umap::iterator i = umap_.begin();
	while(i != umap_.end() && (!i->second.unit)) {
		++i;
	}

	return i;
}

unit_map::umap_retval_pair_t unit_map::add(const map_location& l, const unit& u)
{
	self_check();

	 // TODO: should this take a shared pointer to a unit rather than make a copy?
	unit_ptr p = unit_ptr(new unit(u));
	p->set_location(l);

	unit_map::umap_retval_pair_t res(insert(p));
	if(res.second == false) {
		p.reset();
	}

	return res;
}

unit_map::umap_retval_pair_t unit_map::move(const map_location& src, const map_location& dst)
{
	self_check();
	DBG_NG << "Unit map: Moving unit from " << src << " to " << dst << "\n";

	// Find the unit at the src location
	lmap::iterator i = lmap_.find(src);
	if(i == lmap_.end()) {
		return std::make_pair(make_unit_iterator(i), false);
	}

	umap::iterator uit(i->second);

	if(src == dst) {
		return std::make_pair(make_unit_iterator(uit), true);
	}

	// Fail if there is no unit to move.
	unit_ptr p = uit->second.unit;
	if(!p) {
		return std::make_pair(make_unit_iterator(uit), false);
	}

	p->set_location(dst);

	lmap_.erase(i);

	std::pair<lmap::iterator, bool> res = lmap_.emplace(dst, uit);

	// Fail and don't move if the destination is already occupied.
	if(res.second == false) {
		p->set_location(src);
		lmap_.emplace(src, uit);
		return std::make_pair(make_unit_iterator(uit), false);
	}

	self_check();

	return std::make_pair(make_unit_iterator(uit), true);
}

unit_map::umap_retval_pair_t unit_map::insert(unit_ptr p)
{
	// 1. Construct a unit_pod.
	// 2. Try insertion into the umap.
	// 3. Try insertion in the lmap and remove the umap entry on failure.

	self_check();
	assert(p);

	size_t unit_id = p->underlying_id();
	const map_location& loc = p->get_location();

	if(!loc.valid()) {
		ERR_NG << "Trying to add " << p->name() << " - " << p->id() << " at an invalid location; Discarding.\n";
		return std::make_pair(make_unit_iterator(umap_.end()), false);
	}

	unit_pod upod;
	upod.unit = p;

	DBG_NG << "Adding unit " << p->underlying_id() << " - " << p->id() << " to location: (" << loc << ")\n";

	std::pair<umap::iterator, bool> uinsert = umap_.emplace(unit_id, upod);

	if(!uinsert.second) {
		// If the pod is empty reinsert the unit in the same list element.
		if(!uinsert.first->second.unit) {
			unit_pod& opod = uinsert.first->second;
			opod.unit = p;

			assert(opod.ref_count != 0);
		} else {
			unit_ptr q = uinsert.first->second.unit;
			ERR_NG << "Trying to add " << p->name()
				   << " - " << p->id() << " - " << p->underlying_id()
				   << " ("  << loc << ") over " << q->name()
				   << " - " << q->id() << " - " << q->underlying_id()
				   << " ("  << q->get_location()
				   << ").\n";

			p->clone(false);
			ERR_NG << "The new unit was assigned underlying_id=" << p->underlying_id()
				   << " to prevent duplicate id conflicts.\n";

			uinsert = umap_.emplace(p->underlying_id(), upod);

			int guard(0);
			while(!uinsert.second && (++guard < 1e6)) {
				if(guard % 10 == 9) {
					ERR_NG << "\n\nPlease Report this error to https://gna.org/bugs/index.php?18591 "
						"\nIn addition to the standard details of operating system and wesnoth version "
						"and how it happened, please answer the following questions "
						"\n 1. Were you playing multi-player?"
						"\n 2. Did you start/restart/reload the game/scenario?"
						"\nThank you for your help in fixing this bug.\n";
				}

				p->clone(false);
				uinsert = umap_.emplace(p->underlying_id(), upod);
			}

			if(!uinsert.second) {
				throw game::error("One million collisions in unit_map");
			}
		}
	}

	std::pair<lmap::iterator, bool> linsert = lmap_.emplace(loc, uinsert.first);

	// Fail if the location is occupied
	if(!linsert.second) {
		if(upod.ref_count == 0) {
			// Undo a virgin insertion
			umap_.erase(uinsert.first);
		} else {
			// undo a reinsertion
			uinsert.first->second.unit.reset();
		}

		DBG_NG << "Trying to add " << p->name() << " - " << p->id() << " at location (" << loc << "); Occupied  by "
			   << (linsert.first->second->second).unit->name() << " - " << linsert.first->second->second.unit->id()
			   << "\n";

		return std::make_pair(make_unit_iterator(umap_.end()), false);
	}

	self_check();
	return std::make_pair(make_unit_iterator(uinsert.first), true);
}

unit_map::umap_retval_pair_t unit_map::replace(const map_location& l, unit_ptr p)
{
	self_check();
	p->set_location(l);
	erase(l);
	return insert(p);
}

size_t unit_map::num_iters() const
{
	/// Add up number of extant iterators
	size_t num_iters(0);
	umap::const_iterator ui = umap_.begin();
	umap::const_iterator uend = umap_.end();

	for(; ui != uend; ++ui) {
		if(ui->second.ref_count < 0) {
			// Somewhere, someone generated 2^31 iterators to this unit.
			bool a_reference_counter_overflowed(false);
			assert(a_reference_counter_overflowed);
		}

		num_iters += ui->second.ref_count;
	}

	return num_iters;
}

void unit_map::clear(bool force)
{
	assert(force || (num_iters() == 0));

	for(umap::iterator i = umap_.begin(); i != umap_.end(); ++i) {
		if(is_valid(i)) {
			DBG_NG << "Delete unit " << i->second.unit->underlying_id() << "\n";
			i->second.unit.reset();
		}
	}

	lmap_.clear();
	umap_.clear();
}

unit_ptr unit_map::extract(const map_location& loc)
{
	self_check();

	lmap::iterator i = lmap_.find(loc);
	if(i == lmap_.end()) {
		return unit_ptr();
	}

	umap::iterator uit(i->second);

	unit_ptr u = uit->second.unit;
	size_t uid(u->underlying_id());

	DBG_NG << "Extract unit " << uid << " - " << u->id() << " from location: (" << loc << ")\n";

	assert(uit->first == uit->second.unit->underlying_id());
	if(uit->second.ref_count == 0) {
		umap_.erase(uit);
	} else {
		// Soft extraction keeps the old lit item if any iterators reference it
		uit->second.unit.reset();
	}

	lmap_.erase(i);
	self_check();

	return u;
}

size_t unit_map::erase(const map_location& loc)
{
	self_check();

	unit_ptr u = extract(loc);
	if(!u) {
		return 0;
	}

	u.reset();
	return 1;
}

unit_map::unit_iterator unit_map::find(size_t id)
{
	self_check();

	umap::iterator i(umap_.find(id));
	if((i != umap_.end()) && !i->second.unit) {
		i = umap_.end();
	}

	return make_unit_iterator<umap::iterator>(i);
}

unit_map::unit_iterator unit_map::find(const map_location& loc)
{
	self_check();
	return make_unit_iterator<lmap::iterator>(lmap_.find(loc));
}

unit_map::unit_iterator unit_map::find_leader(int side)
{
	unit_map::iterator i = begin(), i_end = end();
	for(; i != i_end; ++i) {
		if(i->side() == side && i->can_recruit()) {
			return i;
		}
	}

	return i_end;
}

unit_map::unit_iterator unit_map::find_first_leader(int side)
{
	unit_map::iterator i = begin(), i_end = end();
	unit_map::iterator first_leader = end();

	for(; i != i_end; ++i) {
		if(i->side() == side && i->can_recruit()) {
			if((first_leader == end()) || (i->underlying_id() < first_leader->underlying_id())) {
				first_leader = i;
			}
		}
	}

	return first_leader;
}

std::vector<unit_map::unit_iterator> unit_map::find_leaders(int side)
{
	unit_map::unit_iterator i = begin(), i_end = end();

	std::vector<unit_map::unit_iterator> leaders;
	for(; i != i_end; ++i) {
		if(i->side() == side && i->can_recruit()) {
			leaders.push_back(i);
		}
	}

	return leaders;
}

std::vector<unit_map::const_unit_iterator> unit_map::find_leaders(int side) const
{
	const std::vector<unit_map::unit_iterator>& leaders = const_cast<unit_map*>(this)->find_leaders(side);
	std::vector<unit_map::const_unit_iterator> const_leaders(leaders.begin(), leaders.end());

	return const_leaders;
}

bool unit_map::self_check() const
{
#ifdef DEBUG_UNIT_MAP
	bool good(true);

	umap::const_iterator uit(umap_.begin());
	for(; uit != umap_.end(); ++uit) {
		if(uit->second.ref_count < 0) {
			good = false;
			ERR_NG << "unit_map pod ref_count <0 is " << uit->second.ref_count << std::endl;
		}

		if(uit->second.unit) {
			uit->second.unit->id(); // crash if bad pointer
		}

		if(uit->first <= 0) {
			good = false;
			ERR_NG << "unit_map umap uid <=0 is " << uit->first << std::endl;
		}

		if(!uit->second.unit && uit->second.ref_count == 0) {
			good = false;
			ERR_NG << "unit_map umap unit==nullptr when refcount == 0" << std::endl;
		}

		if(uit->second.unit && uit->second.unit->underlying_id() != uit->first) {
			good = false;
			ERR_NG << "unit_map umap uid(" << uit->first << ") != underlying_id()[" << uit->second.unit->underlying_id()
				   << "]" << std::endl;
		}
	}

	lmap::const_iterator locit(lmap_.begin());
	for(; locit != lmap_.end(); ++locit) {
		if(locit->second == umap_.end()) {
			good = false;
			ERR_NG << "unit_map lmap element == umap_.end() " << std::endl;
		}
		if(locit->first != locit->second->second.unit->get_location()) {
			good = false;
			ERR_NG << "unit_map lmap location != unit->get_location() " << std::endl;
		}
	}

	// assert(good);
	return good;
#else
	return true;
#endif
}

bool unit_map::has_unit(const unit* const u) const
{
	assert(u);

	for(const umap::value_type& item : umap_) {
		if(item.second.unit.get() == u) {
			return true;
		}
	}

	return false;
}

bool unit_map::has_unit_at(const map_location& loc) const
{
	return find(loc) != end();
}

void swap(unit_map& lhs, unit_map& rhs)
{
	lhs.swap(rhs);
}
