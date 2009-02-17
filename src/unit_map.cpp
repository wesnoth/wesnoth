/* $Id$ */
/*
   Copyright (C) 2006 - 2009 by Rusty Russell <rusty@rustcorp.com.au>
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


#define ERR_NG LOG_STREAM(err, engine)
#define WRN_NG LOG_STREAM(warn, engine)
#define LOG_NG LOG_STREAM(info, engine)
#define DBG_NG LOG_STREAM(debug, engine)

typedef std::pair<std::string, std::pair<bool, std::pair<map_location, unit>*> > umap_pair;

unit_map::unit_map(const map_location &loc, const unit &u) :
	map_(),
	lmap_(),
	num_iters_(0),
	num_invalid_(0)
{
	add(new std::pair<map_location,unit>(loc, u));
}

unit_map::unit_map(const unit_map &that) :
	/* Initialize to silence compiler warnings. */
	map_(),
	lmap_(),
	num_iters_(0),
	num_invalid_(0)
{
	*this = that;
}

unit_map &unit_map::operator=(const unit_map &that)
{
	clear();
	num_iters_ = 0;
	num_invalid_ = 0;
	for (umap::const_iterator i = that.map_.begin(); i != that.map_.end(); i++) {
		if (i->second.first) {
			add(new std::pair<map_location,unit>(i->second.second->first, i->second.second->second));
		}
	}
	return *this;
}


unit_map::~unit_map()
{
	delete_all();
}

std::pair<map_location,unit>* unit_map::unit_iterator::operator->() const
{
	assert(valid());
	return i_->second.second;
}

std::pair<map_location,unit>& unit_map::unit_iterator::operator*() const
{
	assert(valid());
	return *i_->second.second;
}

unit_map::unit_iterator unit_map::unit_iterator::operator++() {

	assert(i_ != map_->map_.end());

	++i_;
	while (i_ != map_->map_.end() && !valid()) {
		++i_;
	}

	return *this;
}

unit_map::unit_iterator unit_map::unit_iterator::operator++(int){

	assert(i_ != map_->map_.end());

	umap::iterator iter(i_);
	++i_;
	while (i_ != map_->map_.end() && !valid()) {
		++i_;
	}

	return unit_iterator(iter, map_);
}

// Due to unit <-> unit_map dependencies, must be out of line.
const std::pair<map_location,unit>* unit_map::const_unit_iterator::operator->() const
{
	assert(valid());
	return i_->second.second;
}

const std::pair<map_location,unit>& unit_map::const_unit_iterator::operator*() const
{
	assert(valid());
	return *i_->second.second;
}

unit_map::const_unit_iterator unit_map::const_unit_iterator::operator++() {

	assert(i_ != map_->map_.end());

	++i_;
	while (i_ != map_->map_.end() && !valid()) {
		++i_;
	}

	return *this;
}

unit_map::const_unit_iterator unit_map::const_unit_iterator::operator--() {

	umap::const_iterator begin = map_->begin().i_;

	assert(i_ != begin);

	--i_;
	while (i_ != begin && !valid()) {
		--i_;
	}

	return *this;
}

unit_map::const_unit_iterator unit_map::const_unit_iterator::operator++(int){

	assert(i_ != map_->map_.end());

	umap::const_iterator iter(i_);
	++i_;
	while (i_ != map_->map_.end() && !valid()) {
		++i_;
	}

	return const_unit_iterator(iter, map_);
}

unit_map::unit_xy_iterator::unit_xy_iterator(const unit_iterator &i) :
	counter_(i.map_),
	i_(i.i_),
	map_(i.map_),
	loc_(i.valid() ? i->first : map_location())
{
}


std::pair<map_location,unit>* unit_map::unit_xy_iterator::operator->() const
{
	assert(valid());
	return i_->second.second;
}

std::pair<map_location,unit>& unit_map::unit_xy_iterator::operator*() const
{
	assert(valid());
	return *i_->second.second;
}

unit_map::unit_xy_iterator unit_map::unit_xy_iterator::operator++() {

	assert(i_ != map_->map_.end());

	++i_;
	while (i_ != map_->map_.end() && !valid()) {
		++i_;
	}

	if (i_ != map_->map_.end()) {
		loc_ = i_->second.second->first;
	}

	return *this;
}

unit_map::unit_xy_iterator unit_map::unit_xy_iterator::operator++(int){

	assert(i_ != map_->map_.end());

	umap::iterator iter(i_);
	map_location pre_loc = loc_;
	++i_;
	while (i_ != map_->map_.end() && !valid()) {
		++i_;
	}

	if (i_ != map_->map_.end()) {
		loc_ = i_->second.second->first;
	}

	return unit_xy_iterator(iter, map_, pre_loc);
}

bool unit_map::unit_xy_iterator::valid() const {
	return i_ != map_->map_.end() && i_->second.first && loc_ == i_->second.second->first;
}

unit_map::const_unit_xy_iterator::const_unit_xy_iterator(const unit_iterator &i) :
	counter_(i.map_),
	i_(i.i_),
	map_(i.map_),
	loc_(i.valid() ? i->first : map_location())
{
}

unit_map::const_unit_xy_iterator::const_unit_xy_iterator(const const_unit_iterator &i) :
	counter_(i.map_),
	i_(i.i_),
	map_(i.map_),
	loc_(i.valid() ? i->first : map_location())
{
}

const std::pair<map_location,unit>* unit_map::const_unit_xy_iterator::operator->() const {
	assert(valid());
	return i_->second.second;
}

const std::pair<map_location,unit>& unit_map::const_unit_xy_iterator::operator*() const {
	assert(valid());
	return *i_->second.second;
}

unit_map::const_unit_xy_iterator unit_map::const_unit_xy_iterator::operator++() {

	assert(i_ != map_->map_.end());

	++i_;
	while (i_ != map_->map_.end() && !valid()) {
		++i_;
	}

	if (i_ != map_->map_.end()) {
		loc_ = i_->second.second->first;
	}

	return *this;
}

unit_map::const_unit_xy_iterator unit_map::const_unit_xy_iterator::operator++(int){

	assert(i_ != map_->map_.end());

	map_location pre_loc = loc_;

	umap::const_iterator iter(i_);
	++i_;
	while (i_ != map_->map_.end() && !valid()) {
		++i_;
	}

	if (i_ != map_->map_.end()) {
		loc_ = i_->second.second->first;
	}

	return const_unit_xy_iterator(iter, map_, pre_loc);
}

bool unit_map::const_unit_xy_iterator::valid() const {
	return i_ != map_->map_.end() && i_->second.first && loc_ == i_->second.second->first;
}


unit_map::xy_accessor::xy_accessor(const unit_iterator &i) :
	counter_(i.map_),
	i_(i.i_),
	map_(i.map_),
	loc_(i.valid() ? i->first : map_location())
{
}

unit_map::xy_accessor::xy_accessor(const unit_xy_iterator &i) :
	counter_(i.map_),
	i_(i.i_),
	map_(i.map_),
	loc_(i.valid() ? i->first : map_location())
{
}


std::pair<map_location,unit>* unit_map::xy_accessor::operator->() {
	if (!valid()) { assert(0); }
	return i_->second.second;
}

std::pair<map_location,unit>& unit_map::xy_accessor::operator*() {
	if (!valid()) { assert(0); }
	return *i_->second.second;
}


bool unit_map::xy_accessor::valid() {
	if (i_->second.first && i_->second.second->first == loc_) {
		return true;
	}

	unit_iterator u_iter = map_->find(loc_);

	if (u_iter.valid()) {
		i_ = u_iter.i_;
		loc_ = i_->second.second->first;
		return true;
	}

	return false;
}

unit_map::const_xy_accessor::const_xy_accessor(const const_unit_iterator &i) :
	counter_(i.map_),
	i_(i.i_),
	map_(i.map_),
	loc_(i.valid() ? i->first : map_location())
{
}

unit_map::const_xy_accessor::const_xy_accessor(const unit_iterator &i) :
	counter_(i.map_),
	i_(i.i_),
	map_(i.map_),
	loc_(i.valid() ? i->first : map_location())
{
}

unit_map::const_xy_accessor::const_xy_accessor(const const_unit_xy_iterator &i) :
	counter_(i.map_),
	i_(i.i_),
	map_(i.map_),
	loc_(i.valid() ? i->first : map_location())
{
}

unit_map::const_xy_accessor::const_xy_accessor(const unit_xy_iterator &i) :
	counter_(i.map_),
	i_(i.i_),
	map_(i.map_),
	loc_(i.valid() ? i->first : map_location())
{
}

const std::pair<map_location,unit>* unit_map::const_xy_accessor::operator->() {
	if (!valid()) { assert(0); }
	return i_->second.second;
}

const std::pair<map_location,unit>& unit_map::const_xy_accessor::operator*() {
	if (!valid()) { assert(0); }
	return *i_->second.second;
}


bool unit_map::const_xy_accessor::valid() {
	if (i_->second.first && i_->second.second->first == loc_) {
		return true;
	}

	const_unit_iterator u_iter = map_->find(loc_);

	if (u_iter.valid()) {
		i_ = u_iter.i_;
		loc_ = i_->second.second->first;
		return true;
	}

	return false;
}


unit_map::unit_iterator unit_map::find(const map_location &loc) {
	lmap::const_iterator i = lmap_.find(loc);
	if (i == lmap_.end()) {
		return unit_iterator(map_.end(), this);
	}

	umap::iterator iter = map_.find(i->second);

	assert(iter->second.first);
	return unit_iterator(iter , this);
}


unit_map::const_unit_iterator unit_map::find(const map_location &loc) const {
	lmap::const_iterator iter = lmap_.find(loc);
	if (iter == lmap_.end()) {
		return const_unit_iterator(map_.end(), this);
	}

	umap::const_iterator i = map_.find(iter->second);

	assert(i->second.first);
	return const_unit_iterator(i , this);
}

unit_map::unit_iterator unit_map::find(const size_t &id) {
	umap::iterator iter = map_.find(id);
	if (iter == map_.end() || !iter->second.first) {
		return unit_iterator(map_.end(), this);
	}
	return unit_iterator(iter, this);
}

unit_map::const_unit_iterator unit_map::find(const size_t &id) const {
	umap::const_iterator iter = map_.find(id);
	if (iter == map_.end() || !iter->second.first) {
		return const_unit_iterator(map_.end(), this);
	}
	return const_unit_iterator(iter, this);
}

struct match_unit_id {
	match_unit_id(const std::string& id) : id_(id)
	{}
	bool operator()(const unit_map::umap::value_type& val) const
	{
		if (!val.first)
			return false;
		return val.second.second->second.id() == id_;
	}
	private:
	const std::string& id_;
};

unit_map::unit_iterator unit_map::find(const std::string& id) {
	WRN_NG << "Finding using id is slow operation\n";
	umap::iterator iter = std::find_if(map_.begin(), map_.end(), match_unit_id(id));
	if (iter == map_.end() || !iter->second.first) {
		return unit_iterator(map_.end(), this);
	}
	return unit_iterator(iter, this);
}

unit_map::const_unit_iterator unit_map::find(const std::string& id) const {
	WRN_NG << "Finding using id is slow operation\n";
	umap::const_iterator iter = std::find_if(map_.begin(), map_.end(), match_unit_id(id));
	if (iter == map_.end() || !iter->second.first) {
		return const_unit_iterator(map_.end(), this);
	}
	return const_unit_iterator(iter, this);
}

unit_map::unit_iterator unit_map::begin() {
		// clean if there are more invalid than valid, this block just needs to go somewhere that is likely to be
		// called when num_iters_ == 0. This seems as good a place as any.
		if (num_invalid_ > lmap_.size() && num_iters_ == 0) {
			clean_invalid();
		}

		umap::iterator i = map_.begin();
		while (i != map_.end() && !i->second.first) {
			++i;
		}
		return unit_iterator(i, this);
}


void unit_map::add(std::pair<map_location,unit> *p)
{
	size_t unit_id = p->second.underlying_id();
	umap::iterator iter = map_.find(unit_id);

	if (iter == map_.end()) {
		map_[unit_id] = std::make_pair(true, p);
	} else if(!iter->second.first) {
		iter->second.second = p;
		validate(iter);
	} else if(iter->second.second->first == p->first) {
		replace(p);
		return;
	} else {
		ERR_NG << "Trying to add " << p->second.name() <<
			" - " << p->second.id() <<
			" - " << p->second.underlying_id() <<
			" ("  << p->first <<
			") over " << iter->second.second->second.name() <<
			" - " << iter->second.second->second.id() <<
			" - " << iter->second.second->second.underlying_id() <<
			" ("  << iter->second.second->first <<
			"). The new unit will be assigned underlying_id="
			<< (1 + n_unit::id_manager::instance().get_save_id())
			<< " to prevent duplicate id conflicts.\n";
		p->second.clone(false);
		add(p);
		return;
	}

	DBG_NG << "Adding unit " << p->second.underlying_id()<< " - " << p->second.id() << " to location: (" << p->first.x+1 << "," << p->first.y+1
			<< ")\n";

	std::pair<lmap::iterator,bool> res = lmap_.insert(std::make_pair(p->first, unit_id));
	assert(res.second);
}

void unit_map::replace(std::pair<map_location,unit> *p)
{
	if (erase(p->first) != 1)
		assert(0);
	DBG_NG << "Replace unit " << p->second.underlying_id() << "\n";
	add(p);
}

void unit_map::delete_all()
{
	for (umap::iterator i = map_.begin(); i != map_.end(); ++i) {
		if (i->second.first) {
			DBG_NG << "Delete unit " << i->second.second->second.underlying_id() << "\n";
			delete(i->second.second);
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
	std::pair<map_location,unit> *res = iter->second.second;

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
	DBG_NG << "Replace unit " << i->second << "\n";

	invalidate(iter);

	delete iter->second.second;
	lmap_.erase(i);

	return 1;
}

void unit_map::erase(xy_accessor pos)
{
	assert(pos.valid());

	if (erase(pos->first) != 1)
		assert(0);
}

void unit_map::clear()
{
	delete_all();
}

void unit_map::clean_invalid() {
	size_t num_cleaned = 0;

	umap::iterator iter = map_.begin();
	while (iter != map_.end()) {
		if (!iter->second.first) {
			map_.erase(iter++);
			++num_cleaned;
		} else {
			++iter;
		}
	}

	num_invalid_ -= num_cleaned;

	LOG_NG << "unit_map::clean_invalid - removed " << num_cleaned << " invalid map entries.\n";
}

