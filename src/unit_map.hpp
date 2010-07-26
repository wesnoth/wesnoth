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

/** @file */

#ifndef UNIT_MAP_H_INCLUDED
#define UNIT_MAP_H_INCLUDED

#include "map_location.hpp"

#include <cassert>
#include <map>

class unit;

/**
 * Container associating units to locations.
 * An indirection location -> underlying_id -> unit ensures that iterators
 * stay valid even if WML modifies or moves units on the fly. They even stay
 * valid if a unit is erased from the map and another unit with the same
 * underlying id is inserted in the map.
 * @note Units are owned by the container.
 * @note The indirection does not involve map lookups whenever an iterator
 *       is dereferenced, it just causes a pointer indirection. The downside
 *       is that incrementing iterator is not O(1).
 * @note The code does not involve any magic, so units moved while being
 *       iterated upon may be skipped or visited twice.
 * @note Iterators prevent ghost units from being collected. So they should
 *       never be stored into data structures, as it will cause slowdowns!
 */
class unit_map
{
public:
	typedef std::map<size_t, unit *> umap;
	typedef std::map<map_location, size_t> lmap;

	unit_map() : map_(), lmap_(), num_iters_(0), num_invalid_(0) { };
	unit_map(const unit_map &that);
	unit_map& operator=(const unit_map &that);

	/** A unit map with a copy of a single unit in it. */
	unit_map(const map_location &loc, const unit &u);
	~unit_map();


// ~~~ Begin iterator code ~~~

	template<typename iter_types>
	struct iterator_base
	{
		typedef unit value_type;
		typedef std::forward_iterator_tag iterator_category;
		typedef int difference_type;
		typedef unit *pointer;
		typedef unit &reference;
		typedef typename iter_types::map_type map_type;
		typedef typename iter_types::iterator_type iterator_type;

		iterator_base(): map_(NULL), i_() { }

		iterator_base(iterator_type i, map_type *m)
			: map_(m), i_(i)
		{ map_->add_iter(); }

		~iterator_base()
		{ if (map_) map_->remove_iter(); }

		template<typename that_types>
		iterator_base(const iterator_base<that_types> &that)
			: map_(that.map_), i_(that.i_)
		{ if (map_) map_->add_iter(); }

		template<typename that_types>
		iterator_base &operator=(const iterator_base<that_types> &that)
		{
			if (map_) map_->remove_iter();
			map_ = that.map_;
			i_ = that.i_;
			if (map_) map_->add_iter();
			return *this;
		}

		iterator_base(const iterator_base &that)
			: map_(that.map_), i_(that.i_)
		{ if (map_) map_->add_iter(); }

		iterator_base &operator=(const iterator_base &that)
		{
			if (map_) map_->remove_iter();
			map_ = that.map_;
			i_ = that.i_;
			if (map_) map_->add_iter();
			return *this;
		}

		pointer operator->() const
		{ assert(valid()); return i_->second; }
		reference operator*() const
		{ assert(valid()); return *i_->second; }

		iterator_base& operator++()
		{
			assert(map_ && i_ != map_->map_.end());
			do ++i_;
			while (i_ != map_->map_.end() && !i_->second);
			return *this;
		}

		iterator_base operator++(int)
		{
			iterator_base temp(*this);
			operator++();
			return temp;
		}

		iterator_base& operator--()
		{
			assert(map_ && i_ != map_->map_.begin());
			do --i_;
			while (i_ != map_->map_.begin() && !i_->second);
			return *this;
		}

		iterator_base operator--(int)
		{
			iterator_base temp(*this);
			operator--();
			return temp;
		}

		bool valid() const
		{ return map_ && map_->is_valid(i_); }

		bool operator==(const iterator_base& rhs) const { return i_ == rhs.i_; }
		bool operator!=(const iterator_base& rhs) const { return !operator==(rhs); }

		map_type* get_map() const { return map_; }

		template<typename Y> friend struct iterator_base;

	private:
		map_type* map_;
		iterator_type i_;
	};

	struct standard_iter_types {
		typedef unit_map map_type;
		typedef unit_map::umap::iterator iterator_type;
		typedef unit value_type;
		typedef value_type* pointer_type;
		typedef value_type& reference_type;
	};

	struct const_iter_types {
		typedef const unit_map map_type;
		typedef unit_map::umap::const_iterator iterator_type;
		typedef const unit value_type;
		typedef value_type* pointer_type;
		typedef value_type& reference_type;
	};

// ~~~ End iterator code ~~~

	typedef iterator_base<standard_iter_types> unit_iterator;
	typedef iterator_base<const_iter_types> const_unit_iterator;

	// Provided as a convenience, since unit_map used to be an std::map.
	typedef unit_iterator iterator;
	typedef const_unit_iterator const_iterator;

	unit_iterator find(const map_location& loc) ;
	unit_iterator find(size_t id);

	const_unit_iterator find(const map_location &loc) const
	{ return const_cast<unit_map *>(this)->find(loc); }
	const_unit_iterator find(size_t id) const
	{ return const_cast<unit_map *>(this)->find(id); }

	unit_iterator find_leader(int side);
	const_unit_iterator find_leader(int side) const
	{ return const_cast<unit_map *>(this)->find_leader(side); }
	unit_iterator find_first_leader(int side);

	size_t count(const map_location& loc) const { return lmap_.count(loc); }

	unit_iterator begin();
	const_unit_iterator begin() const;

	unit_iterator end() { return iterator(map_.end(), this); }
	const_unit_iterator end() const { return const_iterator(map_.end(), this); }

	size_t size() const { return lmap_.size(); }

	void clear();

	/**
	 * Adds a copy of unit @a u at location @a l of the map.
	 * @pre Location @a l has to be empty.
	 */
	void add(const map_location &l, const unit &u);

	/**
	 * Adds the unit to the map.
	 * @pre The target location (given by unit::get_location) has to be empty.
	 * @note If the unit::underlying_id is already in use, a new one
	 *       will be generated.
	 * @note The map takes ownership of the pointed object.
	 */
	void insert(unit *p);

	/**
	 * Moves a unit from location @a src to location @a dst.
	 */
	void move(const map_location &src, const map_location &dst);

	/**
	 * Works like unit_map::add; but @a l is emptied first, if needed.
	 */
	void replace(const map_location &l, const unit &u);

	/**
	 * Erases the unit at location @a l, if any.
	 * @returns the number of erased units (0 or 1).
	 */
	size_t erase(const map_location &l);

	/**
	 * Erases a unit given by a pointer or an iterator.
	 * @pre The unit is on the map.
	 */
	template <typename T>
	void erase(const T &iter);

	/**
	 * Extracts a unit from the map.
	 * The unit is no longer owned by the map.
	 * It can be reinserted later, if needed.
	 */
	unit *extract(const map_location &loc);

	void swap(unit_map& o);

private:
	/** Removes invalid entries in map_ if safe and needed. */
	void clean_invalid();

	void add_iter() const { ++num_iters_; }
	void remove_iter() const { --num_iters_; }

	bool is_valid(const umap::const_iterator &i) const
	{ return i != map_.end() && i->second; }

	/**
	 * underlying_id -> unit. This requires that underlying_id be
	 * unique (which is enforced in unit_map::insert).
	 */
	umap map_;

	/**
	 * location -> underlying_id.
	 */
	lmap lmap_;

	mutable size_t num_iters_;
	size_t num_invalid_;
};

template <typename T>
void unit_map::erase(const T& iter) {
	assert(iter.valid());

	if (erase(iter->get_location()) != 1)
		assert(0);
}

#endif	// UNIT_MAP_H_INCLUDED
