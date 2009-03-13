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

/** @file unit_map.hpp */

#ifndef UNIT_MAP_H_INCLUDED
#define UNIT_MAP_H_INCLUDED

class unit;
#include <cstring>
#include "map_location.hpp"

// We used to just open-code a std::map<location,unit>,
// but as unit gained weight leading up to 1.1.3,
// manipulating the map caused significant performance issues
// for the AI, which had to actually move units for accurate
// simulation with the new, more powerful filtering.
// This class eases the transition, by providing a wrapper
// which acts like a map of units, not unit pointers,
// except implemented with pointers and hence providing
// a cheap move function.
//
// Further extended to prevent invalidating iterators when
// changes are made to the map, and to add more powerful iterators/
// accessors.

class unit_map
{
private:

	/**
	 * Used so unit_map can keep a count of iterators and clean invalid pointers
	 * when no iterators exist. Every iterator and accessor has a counter
	 * instance.
	 */
	struct iterator_counter {
		iterator_counter() : map_(NULL) {}
		iterator_counter(const unit_map* map) : map_(map)
			{ map_->add_iter(); }

		iterator_counter(const iterator_counter& i) : map_(i.map_) {
			if (map_) map_->add_iter();
		}

		iterator_counter &operator =(const iterator_counter &that) {
			if (this == &that)
				return *this;

			if (map_) map_->remove_iter();

			map_=that.map_;
			if (map_) map_->add_iter();

			return *this;
		}

		~iterator_counter() {if (map_) map_->remove_iter(); }
	private:
		const unit_map* map_;
	};


public:
	unit_map() : map_(), lmap_(), num_iters_(0), num_invalid_(0) { };
	unit_map(const unit_map &that);
	unit_map &operator =(const unit_map &that);
	~unit_map();

	/**
	 * Keyed with unit's underlying_id. bool flag is whether the following pair
	 * pointer is valid. pointer to pair used to imitate a map<location, unit>
	 */
	typedef std::map<size_t,std::pair<bool, std::pair<map_location,unit>*> > umap;

	/** Maps locations to the underlying_id() of the unit at that location. */
	typedef std::map<map_location, size_t> lmap;

	struct const_unit_iterator;
	struct unit_xy_iterator;
	struct const_unit_xy_iterator;
	struct xy_accessor;
	struct const_xy_accessor;

	/**
	 * For iterating over every unit. Iterator is valid as long as there is
	 * there is a unit w/ matching underlying_id in the map.
	 */
	struct unit_iterator
	{
		unit_iterator() :
			counter_(),
			i_(),
			map_(0)
		{ }

		unit_iterator(const unit_iterator &i) : counter_(i.map_), i_(i.i_), map_(i.map_) { }
		unit_iterator(umap::iterator i, unit_map* map) : counter_(map), i_(i), map_(map) { }

		std::pair<map_location,unit> *operator->() const;
		std::pair<map_location,unit>& operator*() const;

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
		iterator_counter counter_;

		umap::iterator i_;
		unit_map* map_;
	};

	struct const_unit_iterator
	{
		const_unit_iterator(const unit_iterator &i) : counter_(i.map_), i_(i.i_), map_(i.map_) { }

		const_unit_iterator() :
			counter_(),
			i_(),
			map_(0)
		{ }

		const_unit_iterator(const const_unit_iterator &i) : counter_(i.map_), i_(i.i_), map_(i.map_) { }
		const_unit_iterator(umap::const_iterator i, const unit_map* map): counter_(map), i_(i), map_(map) { }

		const std::pair<map_location,unit>* operator->() const;
		const std::pair<map_location,unit>& operator*() const;

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
		iterator_counter counter_;

		umap::const_iterator i_;
		const unit_map* map_;
	};

	typedef unit_iterator iterator;
	typedef const_unit_iterator const_iterator;

	/**
	 * Similar to unit_iterator, except that becomes invalid if unit is moved
	 * while the iterator points at it.
	 */
	struct unit_xy_iterator
	{
		unit_xy_iterator(const unit_iterator &i);

		unit_xy_iterator() :
			counter_(),
			i_(),
			map_(0),
			loc_()
		{}

		unit_xy_iterator(const unit_xy_iterator &i) :
			counter_(i.map_),
			i_(i.i_),
			map_(i.map_),
			loc_(i.valid() ? i.loc_ : map_location())
			{
			}

		unit_xy_iterator(umap::iterator i, unit_map* map, map_location loc): counter_(map), i_(i), map_(map), loc_(loc) { }

		std::pair<map_location,unit>* operator->() const;
		std::pair<map_location,unit>& operator*() const;

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
		iterator_counter counter_;

		umap::iterator i_;
		unit_map* map_;

		map_location loc_;
	};

	struct const_unit_xy_iterator
	{
		const_unit_xy_iterator(const unit_iterator &i);
		const_unit_xy_iterator(const const_unit_iterator &i);

		const_unit_xy_iterator() :
			counter_(),
			i_(),
			map_(0),
			loc_()
		{
		}

		const_unit_xy_iterator(umap::const_iterator i, const unit_map* map, map_location loc): counter_(map), i_(i), map_(map), loc_(loc)  { }

		const_unit_xy_iterator(const unit_xy_iterator &i) :
			counter_(i.map_),
			i_(i.i_),
			map_(i.map_),
			loc_(i.valid() ? i.loc_ : map_location())
		{
		}

		const_unit_xy_iterator(const const_unit_xy_iterator &i) :
			counter_(i.map_),
			i_(i.i_),
			map_(i.map_),
			loc_(i.valid() ? i.loc_ : map_location())
		{
		}

		const std::pair<map_location,unit>* operator->() const;
		const std::pair<map_location,unit>& operator*() const;

		const_unit_xy_iterator operator++();

		const_unit_xy_iterator operator++(int);

		bool operator==(const const_unit_xy_iterator &that) const
			{ return that.i_ == this->i_; }

		bool operator!=(const const_unit_xy_iterator &that) const
			{ return that.i_ != this->i_; }

		bool valid() const;

		friend struct const_xy_accessor;

	private:
		iterator_counter counter_;

		umap::const_iterator i_;
		const unit_map* map_;

		map_location loc_;
	};

	/**
	 * Used to access the unit at a given position. Is valid as long as any unit
	 * is in that position. Can switch from invalid to valid.
	 */
	struct xy_accessor
	{
		xy_accessor(const unit_iterator &i);
		xy_accessor(const unit_xy_iterator &i);
		xy_accessor() :
			counter_(),
			i_(),
			map_(),
			loc_()
		{
		}

		std::pair<map_location,unit>* operator->();
		std::pair<map_location,unit>& operator*();

		bool valid();

	private:
		iterator_counter counter_;

		umap::iterator i_;
		unit_map* map_;

		map_location loc_;
	};

	struct const_xy_accessor
	{
		const_xy_accessor(const unit_iterator &i);
		const_xy_accessor(const unit_xy_iterator &i);
		const_xy_accessor(const const_unit_iterator &i);
		const_xy_accessor(const const_unit_xy_iterator &i);

		const_xy_accessor() :
			counter_(),
			i_(),
			map_(),
			loc_()
		{
		}

		const std::pair<map_location,unit>* operator->();
		const std::pair<map_location,unit>& operator*();

		bool valid();

	private:
		iterator_counter counter_;

		umap::const_iterator i_;
		const unit_map* map_;

		map_location loc_;
	};

	/**
	 * Return object can be implicitly converted to any of the other iterators
	 * or accessors
	 */
	unit_iterator find(const map_location &loc) ;
	unit_iterator find(const size_t& id);
	unit_iterator find(const std::string& id);

	/**
	 * Return object can be implicity converted to any of the other const
	 * iterators or accessors
	 */
	const_unit_iterator find(const map_location &loc) const;
	const_unit_iterator find(const size_t& id) const;
	const_unit_iterator find(const std::string& id) const;

	size_t count(const map_location &loc) const {
		return lmap_.count(loc);
	}

	/**
	 * Return object can be implicitly converted to any of the other iterators
	 * or accessors
	 */
	unit_iterator begin();

	/**
	 * Return object can be implicity converted to any of the other const
	 * iterators or accessors
	 */
	const_unit_iterator begin() const {
		umap::const_iterator i = map_.begin();
		while (i != map_.end() && !i->second.first) {
			++i;
		}
		return const_unit_iterator(i, this);
	}

	/**
	 * Return object can be implicitly converted to any of the other iterators
	 * or accessors
	 */
	unit_iterator end() {
		return iterator(map_.end(), this);
	}

	/**
	 * Return object can be implicity converted to any of the other const
	 * iterators or accessors
	 */
	const_unit_iterator end() const {
		return const_iterator(map_.end(), this);
	}

	size_t size() const {
		return lmap_.size();
	}

	void clear();

	/** Extract (like erase, but don't delete). */
	std::pair<map_location,unit> *extract(const map_location &loc);

	/**
	 * Adds a copy of unit @a u at location @a l of the map.
	 */
	void add(const map_location &l, const unit &u);

	/**
	 * Adds the pair location/unit to the map.
	 * @pre The location is empty.
	 * @pre The unit::underlying_id should not be used by the map already.
	 * @note The map takes ownership of the pointed object.
	 * @note This function should be used in conjunction with #extract only.
	 */
	void insert(std::pair<map_location,unit> *p);

	/**
	 * Moves a unit from location @a src to location @a dst.
	 */
	void move(const map_location &src, const map_location &dst);

	/** Like add, but loc must be occupied (implicitly erased). */
	void replace(const map_location &l, const unit &u);

	void erase(xy_accessor pos);
	size_t erase(const map_location &loc);

	void swap(unit_map& o) {
		map_.swap(o.map_);
		lmap_.swap(o.lmap_);
	}


private:
	/** Removes invalid entries in map_. Called automatically when safe and needed. */
	void clean_invalid();

	void invalidate(umap::iterator i)
		{if(i == map_.end()) return; i->second.first = false; ++num_invalid_;}
	void validate(umap::iterator i)
		{if(i == map_.end()) return; i->second.first = true; --num_invalid_;}

	void delete_all();

	void add_iter() const { ++num_iters_; }
	void remove_iter() const { --num_iters_; }


	/**
	 * Key: unit's underlying_id. bool indicates validity of pointer. pointer to
	 * pair used to imitate a map<location, unit>
	 */
	umap map_;

	/** location -> unit.underlying_id(). Unit_map is usually used as though it
	 * is a map<location, unit> and so we need this map for efficient
	 * access/modification.
	 */
	lmap lmap_;

	mutable size_t num_iters_;
	size_t num_invalid_;
};

#endif	// UNIT_MAP_H_INCLUDED
