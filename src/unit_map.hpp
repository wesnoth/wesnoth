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

//! @file unit_map.hpp
//!

#ifndef UNIT_MAP_H_INCLUDED
#define UNIT_MAP_H_INCLUDED

class unit;
#include <cstring>
#include "map.hpp"

// We used to just open-code a std::map<location,unit>,
// but as unit gained weight leading up to 1.1.3,
// manipulating the map caused significant performance issues
// for the AI, which had to actually move units for accurate
// simulation with the new, more powerful filtering.
// This class eases the transition, by providing a wrapper
// which acts like a map of units, not unit pointers,
// except implemented with pointers and hence providing
// a cheap move function.

class unit_map
{
private:
	
	//! Used so unit_map can keep a count of iterators and clean invalid pointers when no iterators exist. Every iterator and accessor has a counter instance.
	struct iterator_counter {
			iterator_counter() : has_map_(false) {}
			iterator_counter(const unit_map* map) : map_(map), has_map_(true) 
			{ map_->add_iter(); }
			
			iterator_counter(const iterator_counter& i) : has_map_(i.has_map_) 	{
				if (has_map_) 
					{ map_ = i.map_; map_->add_iter(); } 
			}
			
			iterator_counter &operator =(const iterator_counter &that) {
				if (this == &that)
					return *this;
				
				if (has_map_) map_->remove_iter();
				
				has_map_ = that.has_map_;
								
				if (has_map_) {
					map_=that.map_;
					map_->add_iter();
				}	
				
				return *this;			
			}
						
			~iterator_counter() {if (has_map_) map_->remove_iter(); }
		private:	
			const unit_map* map_;
			bool has_map_;			
	};
	
public:
	unit_map() : num_iters_(0), num_invalid_(0) { };
	unit_map(const unit_map &that);
	unit_map &operator =(const unit_map &that);
	//! A unit map with a single unit in it.
	explicit unit_map(const gamemap::location &loc, const unit &u);
	~unit_map();
	
	//! Keyed with unit's underlying_id. bool flag is whether the following pair pointer is valid. pointer to pair used to imitate a map<location, unit>
	typedef std::map<std::string,std::pair<bool, std::pair<gamemap::location,unit>*> > umap;
	
	//! Maintains only the valid pairs. Used to determine validity in umap, and for faster lookup by location.
	typedef std::map<gamemap::location, std::pair<gamemap::location, unit>*> lmap;
		
	struct const_unit_iterator;	
	struct unit_xy_iterator;
	struct const_unit_xy_iterator;
	struct xy_accessor;
	struct const_xy_accessor;

	//! For iterating over every unit. Iterator is valid as long as there is there is a unit w/ matching underlying_id in the map.
	struct unit_iterator
	{
		unit_iterator() { }
		
		unit_iterator(const unit_iterator &i) : counter(i.map_), i_(i.i_), map_(i.map_) { }
		unit_iterator(umap::iterator i, unit_map* map) : counter(map), i_(i), map_(map) { }
					
		std::pair<gamemap::location,unit> *operator->() const;
		std::pair<gamemap::location,unit>& operator*() const;

		unit_iterator operator++();
		unit_iterator operator++(int);
		

		bool operator==(const unit_iterator &that) const
			{ return that.i_ == this->i_; }

		bool operator!=(const unit_iterator &that) const
			{ return that.i_ != this->i_; }
			
		bool valid() const
			{ return i_ != map_->map_.end() && i_->second.first; }
			
		friend struct const_unit_iterator;
		friend struct unit_xy_iterator;
		friend struct const_unit_xy_iterator;
		friend struct xy_accessor;
		friend struct const_xy_accessor;
		
	private:
		iterator_counter counter;
		
		umap::iterator i_;
		unit_map* map_;
	};
	
	struct const_unit_iterator
	{
		const_unit_iterator(const unit_iterator &i) : counter(i.map_), i_(i.i_), map_(i.map_) { }
		
		const_unit_iterator() { }
				
		const_unit_iterator(const const_unit_iterator &i) : counter(i.map_), i_(i.i_), map_(i.map_) { }
		const_unit_iterator(umap::const_iterator i, const unit_map* map): counter(map), i_(i), map_(map) { }
		
		const std::pair<gamemap::location,unit>* operator->() const;
		const std::pair<gamemap::location,unit>& operator*() const;

		const_unit_iterator operator++();

		const_unit_iterator operator++(int);

		const_unit_iterator operator--();

		bool operator==(const const_unit_iterator &that) const
			{ return that.i_ == this->i_; }

		bool operator!=(const const_unit_iterator &that) const
			{ return that.i_ != this->i_; }
			
		bool valid() const
			{ return i_ != map_->map_.end() && i_->second.first; }
			
		friend struct const_unit_xy_iterator;
		friend struct const_xy_accessor;
		
	private:	
		iterator_counter counter;
					
		umap::const_iterator i_;	
		const unit_map* map_;
	};
	
	typedef unit_iterator iterator;
	typedef const_unit_iterator const_iterator;
	
	//! Similar to unit_iterator, except that becomes invalid if unit is moved while the iterator points at it. Can update to the new position w/ update_loc().
	struct unit_xy_iterator
	{
		unit_xy_iterator(const unit_iterator &i);
		
		unit_xy_iterator() { }
				
		unit_xy_iterator(const unit_xy_iterator &i) : counter(i.map_), i_(i.i_), map_(i.map_)
			{ if (i.valid()) loc_ = i.loc_; }
			
		unit_xy_iterator(umap::iterator i, unit_map* map, gamemap::location loc): counter(map), i_(i), map_(map), loc_(loc) { }
		
		std::pair<gamemap::location,unit>* operator->() const;
		std::pair<gamemap::location,unit>& operator*() const;

		unit_xy_iterator operator++();

		unit_xy_iterator operator++(int);

		bool operator==(const unit_xy_iterator &that) const
			{ return that.i_ == this->i_; }

		bool operator!=(const unit_xy_iterator &that) const
			{ return that.i_ != this->i_; }
			
		bool valid() const;
			
		friend struct const_unit_xy_iterator;
		friend struct xy_accessor;
		friend struct const_xy_accessor;
		
	private:	
		iterator_counter counter;	
		
		umap::iterator i_;	
		unit_map* map_;
		
		gamemap::location loc_;	
	};
	
	struct const_unit_xy_iterator
	{
		const_unit_xy_iterator(const unit_iterator &i);
		const_unit_xy_iterator(const const_unit_iterator &i);
		
		const_unit_xy_iterator() { }
							
		const_unit_xy_iterator(umap::const_iterator i, const unit_map* map, gamemap::location loc): counter(map), i_(i), map_(map), loc_(loc)  { }
					
		const_unit_xy_iterator(const unit_xy_iterator &i) : counter(i.map_), i_(i.i_), map_(i.map_) 
			{ if (i.valid()) loc_ = i.loc_; }			
		const_unit_xy_iterator(const const_unit_xy_iterator &i) : counter(i.map_), i_(i.i_), map_(i.map_)
			{ if (i.valid()) loc_ = i.loc_; }

		const std::pair<gamemap::location,unit>* operator->() const;
		const std::pair<gamemap::location,unit>& operator*() const;

		const_unit_xy_iterator operator++();

		const_unit_xy_iterator operator++(int);

		bool operator==(const const_unit_xy_iterator &that) const
			{ return that.i_ == this->i_; }

		bool operator!=(const const_unit_xy_iterator &that) const
			{ return that.i_ != this->i_; }
			
		bool valid() const;
		
		friend struct const_xy_accessor;
		
	private:		
		iterator_counter counter;
		
		umap::const_iterator i_;	
		const unit_map* map_;
		
		gamemap::location loc_;	
	};
	
	//! Used to access the unit at a given position. Is valid as long as any unit is in that position. Can switch from invalid to valid.
	struct xy_accessor
	{
		xy_accessor(const unit_iterator &i);
		xy_accessor(const unit_xy_iterator &i);		
		xy_accessor() { }
		
		std::pair<gamemap::location,unit>* operator->();
		std::pair<gamemap::location,unit>& operator*();
		
		bool valid();
		
	private:
		iterator_counter counter;
		
		umap::iterator i_;
		unit_map* map_;
				
		gamemap::location loc_;		
	};
	
	struct const_xy_accessor
	{
		const_xy_accessor(const unit_iterator &i);
		const_xy_accessor(const unit_xy_iterator &i);
		const_xy_accessor(const const_unit_iterator &i);
		const_xy_accessor(const const_unit_xy_iterator &i);
				
		const_xy_accessor() { }
		
		const std::pair<gamemap::location,unit>* operator->();
		const std::pair<gamemap::location,unit>& operator*();
		
		bool valid();
		
	private:
		iterator_counter counter;
		
		umap::const_iterator i_;
		const unit_map* map_;
				
		gamemap::location loc_;		
	};
	
	//! Return object can be implicitly converted to any of the other iterators or accessors
	unit_iterator find(const gamemap::location &loc) ;
	
	//! Return object can be implicity converted to any of the other const iterators or accessors
	const_unit_iterator find(const gamemap::location &loc) const;
	
	size_t count(const gamemap::location &loc) const {
		return lmap_.count(loc);
	}

	//! Return object can be implicitly converted to any of the other iterators or accessors
	unit_iterator begin();

	//! Return object can be implicity converted to any of the other const iterators or accessors
	const_unit_iterator begin() const {
		umap::const_iterator i = map_.begin();
		while (i != map_.end() && !i->second.first) { 
			++i; 
		}
		return const_unit_iterator(i, this);
	}

	//! Return object can be implicitly converted to any of the other iterators or accessors
	unit_iterator end() {
		return iterator(map_.end(), this);
	}

	//! Return object can be implicity converted to any of the other const iterators or accessors
	const_unit_iterator end() const {
		return const_iterator(map_.end(), this);
	}	
	
	size_t size() const {
		return lmap_.size();
	}

	void clear();

	//! Extract (like erase, only don't delete).
	std::pair<gamemap::location,unit> *extract(const gamemap::location &loc);

	//! Map owns pointer after this.  Loc must be currently empty. unit's underlying_id should not be present in the map already
	void add(std::pair<gamemap::location,unit> *p);

	//! Like add, but loc must be occupied (implicitly erased).
	void replace(std::pair<gamemap::location,unit> *p);

	void erase(xy_accessor pos);
	size_t erase(const gamemap::location &loc);

	void swap(unit_map& o) {
		map_.swap(o.map_);
		lmap_.swap(o.lmap_);
	}
	
	//! Removes invalid entries in map_. Called automatically when safe and needed.
	void clean_invalid();
		
private:
	
	void update_validity(umap::iterator iter);	
	void delete_all();
	
	void add_iter() const { ++num_iters_; }
	void remove_iter() const { --num_iters_; }

	
	//! Key: unit's underlying_id. bool indicates validity of pointer. pointer to pair used to imitate a map<location, unit>
	std::map<std::string,std::pair<bool, std::pair<gamemap::location,unit>*> > map_;
	
	//! Maintains only the valid pairs. Used to determine validity in umap, and for faster lookup by location.
	std::map<gamemap::location, std::pair<gamemap::location, unit>*> lmap_;
	
	mutable size_t num_iters_;
	size_t num_invalid_;
};

#endif	// UNIT_MAP_H_INCLUDED
