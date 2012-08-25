/* $Id$ */
/*
   Copyright (C) 2006 - 2009 by Rusty Russell <rusty@rustcorp.com.au>
   Copyright (C) 2010 - 2012 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
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

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define WRN_NG LOG_STREAM(warn, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)
#define DBG_NG LOG_STREAM(debug, log_engine)

unit_map::unit_map()
	: umap_()
	, lmap_()
	, ilist_()
	, the_end_()
{
	init_end();
}

unit_map::unit_map(const unit_map& that)
	: umap_()
	, lmap_()
	, ilist_()
	, the_end_()
{
	init_end();
	for (const_unit_iterator i = that.begin(); i != that.end(); ++i) {
		add(i->get_location(), *i);
	}
}

unit_map &unit_map::operator=(const unit_map &that) {
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
}

unit_map::~unit_map() {
	clear(true);
}

unit_map::t_ilist::iterator unit_map::begin_core() const {
	self_check();
	t_ilist::iterator i = ilist_.begin();
	while (i != the_end_ && (i->unit == NULL)) { ++i; }
	return i;
}

std::pair<unit_map::unit_iterator, bool> unit_map::add(const map_location &l, const unit &u) {
	self_check();
	unit *p = new unit(u);
	p->set_location(l);
	std::pair<unit_map::unit_iterator, bool> res( insert(p) );
	if(res.second == false) { delete p; }
	return res;
}

std::pair<unit_map::unit_iterator, bool> unit_map::move(const map_location &src, const map_location &dst) {
	self_check();
	DBG_NG << "Unit map: Moving unit from " << src << " to " << dst << "\n";

	//Find the unit at the src location
	t_lmap::iterator i = lmap_.find(src);
	if(i == lmap_.end()) { return std::make_pair(make_unit_iterator(the_end_), false);}

	t_ilist::iterator lit(i->second);

	if(src == dst){ return std::make_pair(make_unit_iterator(lit),true);}

	//Fail if there is no unit to move
	unit *p = lit->unit;
	if(p == NULL){ return std::make_pair(make_unit_iterator(lit), false);}

	p->set_location(dst);

	///@todo upgrade to quick_erase when boost 1.42 supported by wesnoth
	lmap_.erase(i);

	std::pair<t_lmap::iterator,bool> res = lmap_.insert(std::make_pair(dst, lit));

	//Fail and don't move if the destination is already occupied
	if(res.second == false) {
		p->set_location(src);
		lmap_.insert(std::make_pair(src, lit));
		return std::make_pair(make_unit_iterator(lit), false);
	}

	self_check();

	return std::make_pair(make_unit_iterator(lit), true);
}


/** Inserts the unit pointed to by @a p into the unit_map.

It needs to succeed on the insertion to the umap and to the lmap
otherwise all operations are reverted.
1. Insert a unit_pod into the list
2. Try insertion into the umap and remove the list item on failure
3. Try insertion in the lmap and remove the umap and the list item on failure

The one oddity is that to facilitate non-invalidating iterators the list
sometimes has NULL pointers which should be used when they correspond
to uids previously used.
 */
std::pair<unit_map::unit_iterator, bool> unit_map::insert(unit *p) {
	self_check();
	assert(p);

	size_t unit_id = p->underlying_id();
	const map_location &loc = p->get_location();

	if (!loc.valid()) {
		ERR_NG << "Trying to add " << p->name()
			<< " - " << p->id() << " at an invalid location; Discarding.\n";
		return std::make_pair(make_unit_iterator(the_end_), false);
	}

	unit_pod upod;
	upod.unit = p;
	upod.deleted_uid = unit_id;
	ilist_.push_front(upod);
	t_ilist::iterator lit(ilist_.begin());

	DBG_NG << "Adding unit " << p->underlying_id() << " - " << p->id()
		<< " to location: (" << loc << ")\n";

	std::pair<t_umap::iterator, bool> uinsert = umap_.insert(std::make_pair(unit_id, lit ));

	if (! uinsert.second) {
		//If the UID is empty reinsert the unit in the same list element
		if ( uinsert.first->second->unit == NULL) {
			ilist_.pop_front();
			lit = uinsert.first->second;
			lit->unit = p;
			assert(lit->ref_count != 0);
		} else {
			unit *q = uinsert.first->second->unit;
			ERR_NG << "Trying to add " << p->name()
				   << " - " << p->id() << " - " << p->underlying_id()
				   << " ("  << loc << ") over " << q->name()
				   << " - " << q->id() << " - " << q->underlying_id()
				   << " ("  << q->get_location()
				   << "). The new unit will be assigned underlying_id="
				   << (1 + n_unit::id_manager::instance().get_save_id())
				   << " to prevent duplicate id conflicts.\n";

			p->clone(false);
			uinsert = umap_.insert(std::make_pair(p->underlying_id(), lit ));
			int guard(0);
			while (!uinsert.second && (++guard < 1e6) ) {
				if(guard % 10 == 9){
					ERR_NG << "\n\nPlease Report this error to https://gna.org/bugs/index.php?18591 "
						"\nIn addition to the standard details of operating system and wesnoth version "
						"and how it happened, please answer the following questions "
						"\n 1. Were you playing mutli-player?"
						"\n 2. Did you start/restart/reload the game/scenario?"
						"\nThank you for your help in fixing this bug.\n";
				}
				p->clone(false);
				uinsert = umap_.insert(std::make_pair(p->underlying_id(), lit )); }
			if (!uinsert.second) {
				throw "One million collisions in unit_map"; }
		}
	}

	std::pair<t_lmap::iterator,bool> linsert = lmap_.insert(std::make_pair(loc, lit ));

	//Fail if the location is occupied
	if(! linsert.second) {
		if(lit->ref_count == 0) {
			//Undo a virgin insertion
			ilist_.pop_front();
			///@todo replace with quick_erase(i) when wesnoth supports  boost 1.42 min version
			umap_.erase(uinsert.first);
		} else {
			//undo a reinsertion
			uinsert.first->second->unit = NULL;
		}
		DBG_NG << "Trying to add " << p->name()
			   << " - " << p->id() << " at location ("<<loc <<"); Occupied  by "
			   <<(linsert.first->second)->unit->name()<< " - " << linsert.first->second->unit->id() <<"\n";

		return std::make_pair(make_unit_iterator(the_end_), false);
	}

	self_check();
	return std::make_pair( make_unit_iterator( lit ), true);
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

size_t unit_map::num_iters() const  {
	///Add up number of extant iterators
	size_t num_iters(0);
	t_ilist::const_iterator ii(ilist_.begin());
	for( ; ii != the_end_ ; ++ii){
		if(ii->ref_count < 0) {
			//Somewhere, someone generated 2^31 iterators to this unit
			bool a_reference_counter_overflowed(false);
			assert(a_reference_counter_overflowed); }
		num_iters += ii->ref_count; }

	return num_iters;
}

void unit_map::clear(bool force) {
	assert(force  || (num_iters() == 0));

	for (t_ilist::iterator i = ilist_.begin(); i != the_end_; ++i) {
		if (is_valid(i)) {
			DBG_NG << "Delete unit " << i->unit->underlying_id() << "\n";
			delete i->unit;
		}
	}

	lmap_.clear();
	umap_.clear();
	ilist_.clear();
}

unit *unit_map::extract(const map_location &loc) {
	self_check();
	t_lmap::iterator i = lmap_.find(loc);
	if (i == lmap_.end()) { return NULL; }

	t_ilist::iterator lit(i->second);

	unit *u = lit->unit;
	size_t uid( u->underlying_id() );

	DBG_NG << "Extract unit " << uid << " - " << u->id()
			<< " from location: (" << loc << ")\n";

	if(lit->ref_count == 0){
		assert(lit != the_end_);
		if(umap_.erase(uid) != 1){
			error_recovery_externally_changed_uid(lit); }
		ilist_.erase( lit );
	} else {
		//Soft extraction keeps the old lit item if any iterators reference it
		lit->unit = NULL;
		lit->deleted_uid = uid;
		assert( uid != 0);
	}

	lmap_.erase(i);
	self_check();

	return u;
}

/** error_recovery_externally_changed_uid is called when an attempt is made to
	delete a umap_ element, which fails because the underlying_id() has been changed externally.
	It searches the entire umap_ to for a matching ilist_ element to erase.
 */
void unit_map::error_recovery_externally_changed_uid(t_ilist::iterator const & lit) const {
	std::string name, id ;
	size_t uid;
	if(lit->unit != NULL){
		unit const * u(lit->unit);
		name = u->name();
		id = u->id();
		uid = u->underlying_id();
	} else {
		name = "unknown";
		id = "unknown";
		uid = lit->deleted_uid;
	}
	t_umap::iterator uit(umap_.begin());
	for(; uit != umap_.end(); ++uit){
		if(uit->second == lit){ break; }
	}
	std::stringstream oldidss;
	if (uit != umap_.end()) {
		size_t olduid =  uit->first ;
		umap_.erase(uit);
		oldidss << olduid;
	} else {
		oldidss << "unknown";
	}

	ERR_NG << "Extracting " << name << " - " << id
		   << " from unit map. Underlying ID changed from "<< oldidss.str()  << " to " << uid
			<< " between insertion and extraction, requiring an exhaustive search of umap_.\n";
}


size_t unit_map::erase(const map_location &loc) {
	self_check();
	unit *u = extract(loc);
	if (!u) return 0;
	delete u;
	return 1;
}

unit_map::unit_iterator unit_map::find(size_t id) {
	self_check();
	t_umap::iterator i(umap_.find(id));
	if((i != umap_.end()) && i->second->unit==NULL){ i = umap_.end() ;}
	return make_unit_iterator<t_umap::iterator>( i ); }

unit_map::unit_iterator unit_map::find(const map_location &loc) {
	self_check();
	return make_unit_iterator<t_lmap::iterator>(lmap_.find(loc) ); }

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

#ifdef DEBUG

bool unit_map::self_check() const {
	bool found_the_end(false), good(true);
	t_ilist::const_iterator lit(ilist_.begin());
	for(; lit != ilist_.end(); ++lit){
		if(lit == the_end_){ found_the_end = true; continue; }
		if(lit->ref_count < 0){
			good=false;
			ERR_NG << "unit_map list element ref_count <0 is " << lit->ref_count<<"\n"; }
		if(lit->unit != NULL){
			lit->unit->id(); //crash if bad pointer
		} else {
			if(lit->ref_count <= 0){
				good=false;
				ERR_NG << "unit_map list element ref_count <=0 is " << lit->ref_count<<", when unit deleted.\n"; }
			if(lit->deleted_uid <= 0 ){
				good=false;
				ERR_NG << "unit_map list element deleted_uid <=0 is " << lit->deleted_uid<<"\n"; }
		}
	}

	if(!found_the_end){
		good=false;
		ERR_NG << "unit_map list the_end_ is missing. " <<"\n";}

	t_umap::const_iterator uit(umap_.begin());
	for(; uit != umap_.end(); ++uit){
		if(uit->first <= 0){
			good=false;
			ERR_NG << "unit_map umap uid <=0 is " << uit->first <<"\n"; }
		if(uit->second == the_end_ ){
			good=false;
			ERR_NG << "unit_map umap element == the_end_ "<<"\n"; }
		if(uit->second->unit == NULL && uit->second->ref_count == 0 ){
			good=false;
			ERR_NG << "unit_map umap unit==NULL when refcount == 0 uid="<<uit->second->deleted_uid<<"\n";
		}
		if(uit->second->unit && uit->second->unit->underlying_id() != uit->first){
			good=false;
			ERR_NG << "unit_map umap uid("<<uit->first<<") != underlying_id()["<< uit->second->unit->underlying_id()<< "]\n"; }
	}
	t_lmap::const_iterator locit(lmap_.begin());
	for(; locit != lmap_.end(); ++locit){
		if(locit->second == the_end_ ){
			good=false;
			ERR_NG << "unit_map lmap element == the_end_ "<<"\n"; }
		if(locit->first != locit->second->unit->get_location()){
			good=false;
			ERR_NG << "unit_map lmap location != unit->get_location() " <<"\n"; }
	}
	//assert(good);
	return good;
}

#endif

bool unit_map::has_unit(const unit * const u)
{
	assert(u);

	BOOST_FOREACH(const unit_pod& item, ilist_) {
		if(item.unit == u) {
			return true;
		}
	}
	return false;
}
