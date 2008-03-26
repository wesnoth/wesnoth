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
#include "log.hpp"

#include <cassert>

#define WRN_NG LOG_STREAM(warn, engine)
#define LOG_NG LOG_STREAM(info, engine)

typedef std::pair<std::string, std::pair<bool, std::pair<gamemap::location, unit>*> > umap_pair;

//! A unit map with a copy of a single unit in it.
unit_map::unit_map(const gamemap::location &loc, const unit &u) : num_iters_(0), num_invalid_(0)
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
	num_iters_ = 0;
	num_invalid_ = 0;
	for (umap::const_iterator i = that.map_.begin(); i != that.map_.end(); i++) {
		if (i->second.first) {
			add(i->second.second);
		} 
	}
	return *this;
}

unit_map::~unit_map()
{
	delete_all();
}

std::pair<gamemap::location,unit>* unit_map::unit_iterator::operator->() const
{
	assert(valid());
	return i_->second.second;
}

std::pair<gamemap::location,unit>& unit_map::unit_iterator::operator*() const
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
const std::pair<gamemap::location,unit>* unit_map::const_unit_iterator::operator->() const
{
	assert(valid());
	return i_->second.second;
}

const std::pair<gamemap::location,unit>& unit_map::const_unit_iterator::operator*() const
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

unit_map::unit_xy_iterator::unit_xy_iterator(const unit_iterator &i) : counter(i.map_), i_(i.i_), map_(i.map_) { 
	if (i.valid()) loc_ = i->first; 
}


std::pair<gamemap::location,unit>* unit_map::unit_xy_iterator::operator->() const
{
	assert(valid());
	return i_->second.second;
}

std::pair<gamemap::location,unit>& unit_map::unit_xy_iterator::operator*() const
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
	gamemap::location pre_loc = loc_;
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

unit_map::const_unit_xy_iterator::const_unit_xy_iterator(const unit_iterator &i) : counter(i.map_), i_(i.i_), map_(i.map_) { 
	if (i.valid()) loc_ = i->first; 
}

unit_map::const_unit_xy_iterator::const_unit_xy_iterator(const const_unit_iterator &i) : counter(i.map_), i_(i.i_), map_(i.map_)  { 
	if (i.valid()) loc_ = i->first; 
}

const std::pair<gamemap::location,unit>* unit_map::const_unit_xy_iterator::operator->() const {
	assert(valid());
	return i_->second.second;
}

const std::pair<gamemap::location,unit>& unit_map::const_unit_xy_iterator::operator*() const {
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
	
	gamemap::location pre_loc = loc_;
	
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


unit_map::xy_accessor::xy_accessor(const unit_iterator &i) : counter(i.map_), i_(i.i_), map_(i.map_) {
	if (i.valid()) loc_ = i->first; 
}

unit_map::xy_accessor::xy_accessor(const unit_xy_iterator &i) : counter(i.map_), i_(i.i_), map_(i.map_) {
	if (i.valid()) loc_ = i->first; 
}
	
		
std::pair<gamemap::location,unit>* unit_map::xy_accessor::operator->() {
	if (!valid()) { assert(0); }
	return i_->second.second;
}

std::pair<gamemap::location,unit>& unit_map::xy_accessor::operator*() {
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

unit_map::const_xy_accessor::const_xy_accessor(const const_unit_iterator &i) : counter(i.map_), i_(i.i_), map_(i.map_) {
	if (i.valid()) loc_ = i->first; 
}

unit_map::const_xy_accessor::const_xy_accessor(const unit_iterator &i) : counter(i.map_), i_(i.i_), map_(i.map_) {
	if (i.valid()) loc_ = i->first; 
}

unit_map::const_xy_accessor::const_xy_accessor(const const_unit_xy_iterator &i) : counter(i.map_), i_(i.i_), map_(i.map_) {
	if (i.valid()) loc_ = i->first; 
}

unit_map::const_xy_accessor::const_xy_accessor(const unit_xy_iterator &i) : counter(i.map_), i_(i.i_), map_(i.map_) {
	if (i.valid()) loc_ = i->first; 
}	
		
const std::pair<gamemap::location,unit>* unit_map::const_xy_accessor::operator->() {
	if (!valid()) { assert(0); }
	return i_->second.second;
}

const std::pair<gamemap::location,unit>& unit_map::const_xy_accessor::operator*() {
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


unit_map::unit_iterator unit_map::find(const gamemap::location &loc) {
	lmap::const_iterator iter = lmap_.find(loc);
	if (iter == lmap_.end()) {
		return unit_iterator(map_.end(), this);
	}
	
	umap::iterator i = map_.find(iter->second->second.underlying_id());
	i->second.second->first = loc;
	
	return unit_iterator(i , this);
}


unit_map::const_unit_iterator unit_map::find(const gamemap::location &loc) const {
	lmap::const_iterator iter = lmap_.find(loc);
	if (iter == lmap_.end()) {
		return const_unit_iterator(map_.end(), this);
	}
	
	umap::const_iterator i = map_.find(iter->second->second.underlying_id());
	i->second.second->first = loc;
	
	return const_unit_iterator(i , this);
}

unit_map::unit_iterator unit_map::begin() {		
		// clean if there are more invalid than valid
		if (num_invalid_ > lmap_.size() && num_iters_ == 0) {
			clean_invalid();
		}
		
		umap::iterator i = map_.begin();
		while (i != map_.end() && !i->second.first) { 
			++i; 
		}
		return unit_iterator(i, this);
}


void unit_map::add(std::pair<gamemap::location,unit> *p)
{
	std::pair<lmap::iterator,bool> res = lmap_.insert(std::pair<gamemap::location,std::pair<gamemap::location,unit>*>(p->first, p));
	assert(res.second);
	
	umap::iterator iter = map_.find(p->second.underlying_id());
	
	if (iter != map_.end() && iter->second.first && iter->second.second->first != p->first) {
		WRN_NG << "unit_map::add -- duplicate unique id in unit_map: " << p->second.underlying_id() << "\n";
	}
	
	if (iter != map_.end() && !iter->second.first) {
		num_invalid_--;
	}
	
	map_[p->second.underlying_id()] = std::pair<bool, std::pair<gamemap::location, unit>*>(true, p);	
}

void unit_map::replace(std::pair<gamemap::location,unit> *p)
{
	if (erase(p->first) != 1)
		assert(0);
	add(p);
}

void unit_map::delete_all()
{
	for (lmap::iterator i = lmap_.begin(); i != lmap_.end(); ++i) {
		delete(i->second);
	}
		
	lmap_.clear();
	map_.clear();
}

//! Extract (like erase, but don't delete).
std::pair<gamemap::location,unit> *unit_map::extract(const gamemap::location &loc)
{
	lmap::iterator i = lmap_.find(loc);
	if (i == lmap_.end())
		return NULL;
	std::pair<gamemap::location,unit> *res = i->second;	
		
	umap::iterator iter = map_.find(i->second->second.underlying_id());
	
	lmap_.erase(i);	
	
	update_validity(iter);
		
	return res;
}

size_t unit_map::erase(const gamemap::location &loc)
{
	lmap::iterator i = lmap_.find(loc);
	if (i == lmap_.end())
		return 0;
			
	umap::iterator iter = map_.find(i->second->second.underlying_id());
			
	delete i->second;
	lmap_.erase(i);
	
	update_validity(iter);
	
	return 1;
}

void unit_map::update_validity(umap::iterator iter)
{
	lmap::const_iterator i;
	for (i = lmap_.begin(); i != lmap_.end(); ++i) {
		if (i->second->second.underlying_id() == iter->first) {
			iter->second.second = i->second;
			break;
		}
	}
	
	if (i == lmap_.end()) {
		if (iter->second.first) {
			iter->second.first = false;
			++num_invalid_;
		}
	} else {
		if (!iter->second.first) {
			iter->second.first = true;
			--num_invalid_;
		}
	}
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
	
	umap::iterator iter;
	for (iter = map_.begin(); iter != map_.end(); ++iter) {
		if (!iter->second.first) {
			map_.erase(iter--);
			num_cleaned++;
		}
	}
	
	num_invalid_ -= num_cleaned;
	
	LOG_NG << "unit_map::clean_invalid - removed " << num_cleaned << " invalid map entries.\n";
}
			
