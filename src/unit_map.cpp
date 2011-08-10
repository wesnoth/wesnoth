/* $Id$ */
/*
   Copyright (C) 2006 - 2009 by Rusty Russell <rusty@rustcorp.com.au>
   Copyright (C) 2010 - 2011 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
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

#include "unit_id.hpp"
#include "log.hpp"
#include "unit.hpp"

#include <functional>
#include "unit_map.hpp"

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define WRN_NG LOG_STREAM(warn, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)
#define DBG_NG LOG_STREAM(debug, log_engine)


unit_map::unit_map(const unit_map& that) :
	umap_(),
	lmap_(),
	ilist_(),
	num_iters_(0),
	num_invalid_(0)
{
	init_end();
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

void unit_map::swap(unit_map &o) {
	assert(num_iters()==0 && o.num_iters() == 0);

	std::swap(umap_, o.umap_);
	std::swap(lmap_, o.lmap_);
	std::swap(ilist_, o.ilist_);
	std::swap(the_end_, o.the_end_);
	std::swap(num_invalid_, o.num_invalid_);
}

unit_map::~unit_map() {
	clear(true);
}

unit_map::t_ilist::iterator unit_map::begin_core() const {
	t_ilist::iterator i = ilist_.begin();
	while (i != the_end_ && (i->unit_ == NULL)) { ++i; }
	return i;
}

void unit_map::add(const map_location &l, const unit &u) {
	unit *p = new unit(u);
	p->set_location(l);
	insert(p);
}

void unit_map::move(const map_location &src, const map_location &dst) {
	DBG_NG << "Unit map: Moving unit from " << src << " to " << dst << "\n";	

	if(src != dst){
		t_lmap::iterator i = lmap_.find(src);
		assert(i != lmap_.end()) ;

		unit *p = i->second->unit_;
		assert(p);

		p->set_location(dst);

		std::pair<t_lmap::iterator,bool> res = lmap_.insert(std::make_pair(dst, i->second)); 
		assert(res.second);

		///@todo upgrade to quick_erase when boost 1.42 supported by wesnoth
		lmap_.erase(i); 
	}
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

	unit_pod upod;
	upod.unit_ = p;
	ilist_.push_front(upod);

	t_ilist::iterator lit(ilist_.begin());

	std::pair<t_umap::iterator, bool> biter =
		umap_.insert(std::make_pair(unit_id, lit ));

	if (biter.second) {
	} else if (! biter.first->second->unit_) {
		biter.first->second->unit_ = p;
		--num_invalid_;
	} else {
		unit *q = biter.first->second->unit_;
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
		<< " to location: (" << loc << ")\n";

	std::pair<t_lmap::iterator,bool> res = lmap_.insert(std::make_pair(loc, lit )); 
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

size_t unit_map::num_iters() const  {
	//Check for outstanding iterators
	size_t num_iters(0);
	t_ilist::const_iterator ii(ilist_.begin());
	for( ; ii != the_end_ ; ++ii){
		if(ii->ref_count_ < 0){
			bool a_reference_counter_overflowed(false);
			assert(a_reference_counter_overflowed); }
		num_iters += ii->ref_count_; }

	return num_iters;
}

void unit_map::clear(bool force) {
	assert(force  || (num_iters() == 0));

	for (t_ilist::iterator i = ilist_.begin(); i != the_end_; ++i) {
		if (is_valid(i)) {
			DBG_NG << "Delete unit " << i->unit_->underlying_id() << "\n";
			delete i->unit_;
		}
	}

	lmap_.clear();
	umap_.clear();
	ilist_.clear();
}

unit *unit_map::extract(const map_location &loc) {
	t_lmap::iterator i = lmap_.find(loc);
	if (i == lmap_.end()) {
		return NULL; }

	unit *res = i->second->unit_;

	DBG_NG << "Extract unit " << res->underlying_id() << " - " << res->id()
			<< " from location: (" << loc << ")\n";

	i->second->unit_ = NULL;
	if(i->second->ref_count_ == 0){ ilist_.erase( i->second ); }

	///@todo replace with quick_erase(i) when wesnoth supports  boost 1.42 min version 
	umap_.erase(res->underlying_id());
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

unit_map::unit_iterator unit_map::find(size_t id) {
	return make_unit_iterator<t_umap::iterator>(umap_.find(id) ); }

unit_map::unit_iterator unit_map::find(const map_location &loc) {
	return make_unit_iterator<t_lmap::iterator>(lmap_.find(loc) ); }

unit_map::unit_iterator unit_map::find_leader(int side) {
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

std::vector<unit_map::unit_iterator> unit_map::find_leaders(int side) {
	unit_map::unit_iterator i = begin(), i_end = end();
	std::vector<unit_map::unit_iterator> leaders;
	for(;i != i_end; ++i){
		if(static_cast<int>(i->side()) == side && i->can_recruit()){
			leaders.push_back(i);
		}
	}
	return leaders;
}
std::vector<unit_map::const_unit_iterator> unit_map::find_leaders(int side)const{
	const std::vector<unit_map::unit_iterator> &leaders = const_cast<unit_map*>(this)->find_leaders(side);
	std::vector<unit_map::const_unit_iterator> const_leaders;
	std::copy(leaders.begin(), leaders.end(), std::back_inserter(const_leaders));
	return const_leaders;
}
