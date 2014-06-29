/*
   Copyright (C) 2006 - 2009 by Rusty Russell <rusty@rustcorp.com.au>
   Copyright (C) 2010 - 2014 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
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

#include <boost/foreach.hpp>

static lg::log_domain log_engine_unit_map("engine");
#define ERR_NG LOG_STREAM(err, log_engine_unit_map)
#define WRN_NG LOG_STREAM(warn, log_engine_unit_map)
#define LOG_NG LOG_STREAM(info, log_engine_unit_map)
#define DBG_NG LOG_STREAM(debug, log_engine_unit_map)

static lg::log_domain log_engine_refac_unit_map("enginerefac");
#define DBG_UR LOG_STREAM(debug, log_engine_refac_unit_map)

unit_map::unit_map()
	: umap_()
	, lmap_()
	, total_num_iters_(0)
{
	DBG_UR << "unit_map constructed" << std::endl;
}

unit_map::unit_map(const unit_map& that)
	: umap_()
	, lmap_()
	, total_num_iters_(0)
{
	DBG_UR << "unit_map copy constructed" << std::endl;
	DBG_UR << "unit_map size" << size() << std::endl;
	DBG_UR << "num_iters" << num_iters() << std::endl;

	for (const_unit_iterator i = that.begin(); i != that.end(); ++i) {
		add(i->get_location(), *i);
	}
	DBG_UR << "finished copying" << std::endl;
	DBG_UR << "unit_map size" << size() << std::endl;
	DBG_UR << "num_iters" << num_iters() << std::endl;
}

unit_map &unit_map::operator=(unit_map that) {
	DBG_UR << "copy assignment operator"<< std::endl;
	DBG_UR << "unit_map size" << size() << std::endl;
	DBG_UR << "num_iters" << num_iters() << std::endl;

	swap(that);

	DBG_UR << "finished swapping"<< std::endl;
	DBG_UR << "unit_map size" << size() << std::endl;
	DBG_UR << "num_iters" << num_iters() << std::endl;

	return *this;
}

void unit_map::swap(unit_map &o) {
	assert(num_iters()==0 && o.num_iters() == 0);

	std::swap(umap_, o.umap_);
	std::swap(lmap_, o.lmap_);
}

unit_map::~unit_map() {
	clear(true);
	DBG_UR << "destroying unit_map" << std::endl;
}

unit_map::t_umap::iterator unit_map::begin_core() const {
	self_check();
	t_umap::iterator i = umap_.begin();
	while (i != umap_.end() && (!i->second)) { ++i; }
	return i;
}

std::pair<unit_map::unit_iterator, bool> unit_map::add(const map_location &l, const unit &u) {
	self_check();
	UnitPtr p = UnitPtr (new unit(u)); //TODO: should this instead take a shared pointer to a unit, rather than make a copy?
	p->set_location(l);
	std::pair<unit_map::unit_iterator, bool> res( insert(p) );
	if(res.second == false) { p.reset(); }
	return res;
}

std::pair<unit_map::unit_iterator, bool> unit_map::move(const map_location &src, const map_location &dst) {
	self_check();
	DBG_NG << "Unit map: Moving unit from " << src << " to " << dst << "\n";

	//Find the unit at the src location
	t_lmap::iterator i = lmap_.find(src);
	if(i == lmap_.end()) { return std::make_pair(make_unit_iterator(i), false);}

	t_umap::iterator uit(i->second);

	if(src == dst){ return std::make_pair(make_unit_iterator(uit), true);}

	//Fail if there is no unit to move
	UnitPtr p = uit->second;
	if(!p){ return std::make_pair(make_unit_iterator(uit), false);}

	p->set_location(dst);

	lmap_.erase(i);

	std::pair<t_lmap::iterator,bool> res = lmap_.insert(std::make_pair(dst, uit));

	//Fail and don't move if the destination is already occupied
	if(res.second == false) {
		p->set_location(src);
		lmap_.insert(std::make_pair(src, uit));
		return std::make_pair(make_unit_iterator(uit), false);
	}

	self_check();

	return std::make_pair(make_unit_iterator(uit), true);
}


/** Inserts the unit pointed to by @a p into the unit_map.

It needs to succeed on the insertion to the umap and to the lmap
otherwise all operations are reverted.
1. Construct a unit_pod
2. Try insertion into the umap
3. Try insertion in the lmap and remove the umap entry on failure

The one oddity is that to facilitate non-invalidating iterators the list
sometimes has NULL pointers which should be used when they correspond
to uids previously used.
 */
std::pair<unit_map::unit_iterator, bool> unit_map::insert(UnitPtr p) {
	self_check();
	assert(p);

	const map_location &loc = p->get_location();

	if (!loc.valid()) {
		ERR_NG << "Trying to add " << p->name()
			<< " - " << p->id() << " at an invalid location; Discarding.\n";
		return std::make_pair(make_unit_iterator(umap_.end()), false);
	}

	DBG_NG << "Adding unit " << p->underlying_id() << " - " << p->id()
		<< " to location: (" << loc << ")\n";

	std::pair<t_umap::iterator, bool> uinsert = umap_.insert(std::make_pair(p->underlying_id(), p ));

	if (! uinsert.second) {
		UnitPtr q = uinsert.first->second;
		ERR_NG << "Trying to add " << p->name()
			   << " - " << p->id() << " - " << p->underlying_id()
			   << " ("  << loc << ") over " << q->name()
			   << " - " << q->id() << " - " << q->underlying_id()
			   << " ("  << q->get_location()
			   << "). The new unit will be assigned underlying_id="
			   << (1 + n_unit::id_manager::instance().get_save_id())
			   << " to prevent duplicate id conflicts.\n";

		p->clone(false);
		uinsert = umap_.insert(std::make_pair(p->underlying_id(), p ));
		int guard(0);
		while (!uinsert.second && (++guard < 1e6) ) {
			if(guard % 10 == 9){
				ERR_NG << "\n\nPlease Report this error to https://gna.org/bugs/index.php?18591 "
					"\nIn addition to the standard details of operating system and wesnoth version "
					"and how it happened, please answer the following questions "
					"\n 1. Were you playing multi-player?"
					"\n 2. Did you start/restart/reload the game/scenario?"
					"\nThank you for your help in fixing this bug.\n";
			}
			p->clone(false);
			uinsert = umap_.insert(std::make_pair(p->underlying_id(), p )); 
		}
		if (!uinsert.second) {
			throw game::error("One million collisions in unit_map");
		}
	}

	std::pair<t_lmap::iterator,bool> linsert = lmap_.insert(std::make_pair(loc, uinsert.first ));

	//Fail if the location is occupied
	if(! linsert.second) {
		umap_.erase(uinsert.first);

		DBG_NG << "Trying to add " << p->name()
			   << " - " << p->id() << " at location ("<<loc <<"); Occupied  by "
			   <<(linsert.first->second->second)->name()<< " - " << linsert.first->second->second->id() <<"\n";

		return std::make_pair(make_unit_iterator(umap_.end()), false);
	}

	self_check();
	return std::make_pair( make_unit_iterator( uinsert.first ), true);
}

std::pair<unit_map::unit_iterator, bool> unit_map::replace(const map_location &l, const unit &u) {
	self_check();
	//when 'l' is the reference to map_location that is part
	//of that unit iterator which is to be deleted by erase,
	// 'l' is invalidated by erase, too. Thus, 'add(l,u)' fails.
	// So, we need to make a copy of that map_location.
	map_location loc = l;
	erase(loc);
	return add(loc, u);
}

void unit_map::clear(bool force) {
	assert(force  || (num_iters() == 0));

	for (t_umap::iterator i = umap_.begin(); i != umap_.end(); ++i) {
		if (is_valid(i)) {
			DBG_NG << "Delete unit " << i->second->underlying_id() << "\n";
			i->second.reset();
		}
	}

	lmap_.clear();
	umap_.clear();
}

UnitPtr unit_map::extract(const map_location &loc) {
	self_check();
	t_lmap::iterator i = lmap_.find(loc);
	if (i == lmap_.end()) { return UnitPtr(); }

	t_umap::iterator uit(i->second);

	UnitPtr u = uit->second;
	size_t uid( u->underlying_id() );

	DBG_NG << "Extract unit " << uid << " - " << u->id()
			<< " from location: (" << loc << ")\n";

	assert(uit->first == uit->second->underlying_id());
	umap_.erase(uit);

	lmap_.erase(i);
	self_check();

	return u;
}


size_t unit_map::erase(const map_location &loc) {
	self_check();
	UnitPtr u = extract(loc);
	if (!u) return 0;
	u.reset();
	return 1;
}

unit_map::unit_iterator unit_map::find(size_t id) {
	self_check();
	t_umap::iterator i(umap_.find(id));
	if((i != umap_.end()) && !i->second){ i = umap_.end() ;}
	return make_unit_iterator<t_umap::iterator>( i );
}

unit_map::unit_iterator unit_map::find(const map_location &loc) {
	self_check();
	return make_unit_iterator<t_lmap::iterator>(lmap_.find(loc) );
}

unit_map::unit_iterator unit_map::find_leader(int side) {
	unit_map::iterator i = begin(), i_end = end();
	for (; i != i_end; ++i) {
		if (i->side() == side && i->can_recruit())
			return i;
	}
	return i_end;
}

unit_map::unit_iterator unit_map::find_first_leader(int side) {
	unit_map::iterator i = begin(), i_end = end();
	unit_map::iterator first_leader = end();

	for (; i != i_end; ++i) {
		if (i->side() == side && i->can_recruit()){
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
		if(i->side() == side && i->can_recruit()){
			leaders.push_back(i);
		}
	}
	return leaders;
}
std::vector<unit_map::const_unit_iterator> unit_map::find_leaders(int side)const{
	const std::vector<unit_map::unit_iterator> &leaders = const_cast<unit_map*>(this)->find_leaders(side);
	std::vector<unit_map::const_unit_iterator> const_leaders (leaders.begin(), leaders.end());
	return const_leaders;
}

#ifdef DEBUG_UNIT_MAP

bool unit_map::self_check() const {
	bool good(true);

	t_umap::const_iterator uit(umap_.begin());
	for(; uit != umap_.end(); ++uit){
		if(uit->second->ref_count() < 0){
			good=false;
			ERR_NG << "unit_map pod ref_count <0 is " << uit->second->ref_count()<< std::endl;
		}
		if(uit->second){
			uit->second->id(); //crash if bad pointer
		}
		if(uit->first <= 0){
			good=false;
			ERR_NG << "unit_map umap uid <=0 is " << uit->first << std::endl;
		}
		if(!uit->second && uit->second->ref_count() == 0 ){
			good=false;
			ERR_NG << "unit_map umap unit==NULL when refcount == 0" << std::endl;
		}
		if(uit->second && uit->second->underlying_id() != uit->first){
			good=false;
			ERR_NG << "unit_map umap uid("<<uit->first<<") != underlying_id()["<< uit->second->underlying_id()<< "]" << std::endl;
		}
	}
	t_lmap::const_iterator locit(lmap_.begin());
	for(; locit != lmap_.end(); ++locit){
		if(locit->second == umap_.end() ){
			good=false;
			ERR_NG << "unit_map lmap element == umap_.end() "<< std::endl;
		}
		if(locit->first != locit->second->second->get_location()){
			good=false;
			ERR_NG << "unit_map lmap location != unit->get_location() " << std::endl;
		}
	}
	//assert(good);
	return good;
}

#endif

bool unit_map::has_unit(const unit * const u) const
{
	assert(u);

	BOOST_FOREACH(const t_umap::value_type& item, umap_) {
		if(item.second.get() == u) {
			return true;
		}
	}
	return false;
}
