/* $Id$ */
/*
   Copyright (C) 2006 - 2010 by Rusty Russell <rusty@rustcorp.com.au>
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

unit& unit_map::node::get_unit() const { return ptr_->second; }
map_location& unit_map::node::get_location() const { return ptr_->first; }

unit_map::unit_map(const unit_map& that) :
	/* Initialize to silence compiler warnings. */
	map_(),
	lmap_(),
	num_iters_(0),
	num_invalid_(0)
{
	for (const_unit_iterator i = that.begin(); i != that.end(); i++) {
		add(i->first, i->second);
	}
}

unit_map &unit_map::operator=(const unit_map &that)
{
	unit_map temp(that);
	swap(temp);
	return *this;
}


unit_map::~unit_map()
{
	assert(num_iters_ == 0);
	delete_all();
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

unit_map::unit_iterator unit_map::find(const size_t &id) {
	umap::iterator iter = map_.find(id);
	iter = is_valid(iter) ? iter : map_.end();

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
	insert(new std::pair<map_location,unit>(l, u));
}

void unit_map::move(const map_location &src, const map_location &dst) {
	std::pair<map_location,unit> *p = extract(src);
	assert(p);
	p->first = dst;
	insert(p);
}

void unit_map::insert(std::pair<map_location,unit> *p)
{
	unit_id_type unit_id = p->second.underlying_id();
	umap::iterator iter = map_.find(unit_id);

	if (!p->first.valid()) {
		ERR_NG << "Trying to add " << p->second.name()
			<< " - " << p->second.id() << " at an invalid location; Discarding.\n";
		delete p;
		return;
	}

	p->second.set_location(p->first);

	if (iter == map_.end()) {
		map_[unit_id] = node(true, p);
	} else if(!is_valid(iter)) {
		iter->second.ptr_ = p;
		validate(iter);
	} else if(iter->second.get_location() == p->first) {
		erase(p->first);
		insert(p);
		return;
	} else {
		ERR_NG << "Trying to add " << p->second.name() <<
			" - " << p->second.id() <<
			" - " << p->second.underlying_id() <<
			" ("  << p->first <<
			") over " << iter->second.get_unit().name() <<
			" - " << iter->second.get_unit().id() <<
			" - " << iter->second.get_unit().underlying_id() <<
			" ("  << iter->second.get_location() <<
			"). The new unit will be assigned underlying_id="
			<< (1 + n_unit::id_manager::instance().get_save_id())
			<< " to prevent duplicate id conflicts.\n";
		p->second.clone(false);
		insert(p);
		return;
	}

	DBG_NG << "Adding unit " << p->second.underlying_id()<< " - " << p->second.id() << " to location: (" << p->first.x+1 << "," << p->first.y+1
			<< ")\n";

	std::pair<lmap::iterator,bool> res = lmap_.insert(std::make_pair(p->first, unit_id));
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

void unit_map::delete_all()
{
	for (umap::iterator i = map_.begin(); i != map_.end(); ++i) {
		if (is_valid(i)) {
			DBG_NG << "Delete unit " << i->second.get_unit().underlying_id() << "\n";
			delete(i->second.ptr_);
		}
	}

	lmap_.clear();
	map_.clear();
}

std::pair<map_location,unit> *unit_map::extract(const map_location &loc)
{
	lmap::iterator i = lmap_.find(loc);
	if (i == lmap_.end())
		return NULL;

	umap::iterator iter = map_.find(i->second);
	std::pair<map_location,unit> *res = iter->second.ptr_;

	DBG_NG << "Extract unit " << i->second << "\n";
	invalidate(iter);
	lmap_.erase(i);

	return res;
}

size_t unit_map::erase(const map_location &loc)
{
	lmap::iterator i = lmap_.find(loc);
	if (i == lmap_.end())
		return 0;

	umap::iterator iter = map_.find(i->second);
	DBG_NG << "Erase unit " << i->second << "\n";

	invalidate(iter);

	delete iter->second.ptr_;
	lmap_.erase(i);

	return 1;
}

void unit_map::clear()
{
	delete_all();
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
		if (static_cast<int>(i->second.side()) == side && i->second.can_recruit())
			return i;
	}
	return i_end;
}
