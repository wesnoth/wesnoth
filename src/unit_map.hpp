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

/** @file unit_map.hpp */

#ifndef UNIT_MAP_H_INCLUDED
#define UNIT_MAP_H_INCLUDED

#include "map_location.hpp"

#include <cassert>

#include <map>

class unit;

// unit_map is used primarily as a map<location, unit>, but we need to be able to move a unit
// without copying it as the ai moves units a lot.
// Also, we want iterators to remain valid as much as reasonable as the underlying map is changed
// and we want a way to test an iterator for validity.

class unit_map
{
private:

	/**
	 * Used so unit_map can keep a count of iterators and clean invalid pointers
	 * only when no iterators exist. Every iterator and accessor has a counter
	 * instance.
	 */
	struct iterator_counter {
		iterator_counter() : map_(NULL) {}
		iterator_counter(const unit_map* map) : map_(map)
			{ if (map_) map_->add_iter(); }

		iterator_counter(const iterator_counter& i) : map_(i.map_)
			{ if (map_) map_->add_iter(); }

		iterator_counter& operator=(const iterator_counter &that) {
			if (map_) map_->remove_iter();

			map_ = that.map_;
			if (map_) map_->add_iter();

			return *this;
		}

		~iterator_counter()
			{if (map_) map_->remove_iter(); }
	private:
		const unit_map* map_;
	};

	struct node {
		bool valid;
		unit *ptr;

		node(bool v, unit *p) : valid(v), ptr(p) { }
		node() : valid(0), ptr(NULL) { }
	};

public:

	/** unit.underlying_id() type */
	typedef size_t unit_id_type;

	/** unit.underlying_id() -> unit_map::node . This requires that underlying_id() be unique (which is enforced in unit_map::add).*/
	typedef std::map<unit_id_type,node> umap;

	/** location -> unit.underlying_id() of the unit at that location */
	typedef std::map<map_location, unit_id_type> lmap;

	unit_map() : map_(), lmap_(), num_iters_(0), num_invalid_(0) { };
	unit_map(const unit_map &that);
	unit_map& operator=(const unit_map &that);

	/** A unit map with a copy of a single unit in it. */
	unit_map(const map_location &loc, const unit &u);
	~unit_map();


// ~~~ Begin iterator code ~~~

	template<typename iter_types>
	struct iterator_base {
		typedef unit value_type;
		typedef std::forward_iterator_tag iterator_category;
		typedef int difference_type;
		typedef unit *pointer;
		typedef unit &reference;
		typedef typename iter_types::map_type map_type;
		typedef typename iter_types::iterator_type iterator_type;

		iterator_base()
		: counter_(), map_(NULL), i_() { }
		iterator_base(iterator_type i, map_type* m)
		: counter_(m), map_(m), i_(i) { }

		pointer operator->() const
		{ assert(valid()); return i_->second.ptr; }
		reference operator*() const
		{ assert(valid()); return *i_->second.ptr; }

		iterator_base& operator++()
		{
			assert(i_ != map_->map_.end());
			do ++i_;
			while (i_ != map_->map_.end() && !i_->second.valid);
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
			assert(i_ != map_->map_.begin());
			do --i_;
			while (i_ != map_->map_.begin() && !i_->second.valid);
			return *this;
		}

		iterator_base operator--(int)
		{
			iterator_base temp(*this);
			operator--();
			return temp;
		}

		bool valid() const
		{ return i_ != map_->map_.end() && i_->second.valid; }

		bool operator==(const iterator_base& rhs) const { return i_ == rhs.i_; }
		bool operator!=(const iterator_base& rhs) const { return !operator==(rhs); }

		template <typename that_types>
		iterator_base(const iterator_base<that_types> &that) :
			counter_(that.counter_),
			map_(that.map_),
			i_(that.i_)
		{
		}

		map_type* get_map() const { return map_; }

		template<typename Y> friend struct iterator_base;

	private:
		iterator_counter counter_;
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



	/**
	 * All iterators are invalidated on unit_map::clear(), unit_map::swap(), unit_map::delete_all(), etc.
	 * Several implicit conversion are supplied, you can convert from more restrictive to less restrictive.
	 * That is:
	 * non-const -> const
	 */

	/**
	 * unit_iterators iterate over all units in the unit_map. A unit_iterator is invalidated if
	 * unit_map::erase, unit_map::extract, or unit_map::replace are called with the location of the
	 * unit that the unit_iterator points to. It will become valid again if unit_map::add is called with
	 * a pair containing the unit that the iterator points to. Basically, as long as the unit is on the
	 * gamemap somewhere, the iterator will be valid.
	 */
	typedef iterator_base<standard_iter_types> unit_iterator;
	typedef iterator_base<const_iter_types> const_unit_iterator;

	/** provided as a convenience as unit_map used to be an std::map */
	typedef unit_iterator iterator;
	typedef const_unit_iterator const_iterator;


	/**
	 * Lookups can be done by map_location, by unit::underlying_id().
	 * Returned iterators can be implicitly converted to the other types.
	 */
	unit_iterator find(const map_location& loc) ;
	unit_iterator find(const unit_id_type& id);

	const_unit_iterator find(const map_location &loc) const
	{ return const_cast<unit_map *>(this)->find(loc); }
	const_unit_iterator find(const unit_id_type &id) const
	{ return const_cast<unit_map *>(this)->find(id); }

	unit_iterator find_leader(int side);
	const_unit_iterator find_leader(int side) const
	{ return const_cast<unit_map *>(this)->find_leader(side); }
	unit_iterator find_first_leader(int side);

	size_t count(const map_location& loc) const { return lmap_.count(loc); }

	/**
	 * Return iterators are implicitly converted to other types as needed.
	 */
	unit_iterator begin();
	const_unit_iterator begin() const;

	unit_iterator end() { return iterator(map_.end(), this); }
	const_unit_iterator end() const { return const_iterator(map_.end(), this); }

	size_t size() const { return lmap_.size(); }

	void clear();

	/**
	 * Adds a copy of unit @a u at location @a l of the map.
	 */
	void add(const map_location &l, const unit &u);

	/**
	 * Adds the unit to the map.
	 * @pre The location is empty.
	 * @pre The unit::underlying_id should not be used by the map already.
	 * @note The map takes ownership of the pointed object.
	 * @note This function should be used in conjunction with #extract only.
	 */
	void insert(unit *p);

	/**
	 * Moves a unit from location @a src to location @a dst.
	 */
	void move(const map_location &src, const map_location &dst);

	/** Like add, but loc must be occupied (implicitly erased). */
	void replace(const map_location &l, const unit &u);

	/** Erases the pair<location, unit> of this location. */
	template <typename T>
	void erase(const T& iter);
	size_t erase(const map_location &loc);

	/** Extract (like erase, but don't delete). */
	unit *extract(const map_location &loc);

	/** Invalidates all iterators on both maps */
	void swap(unit_map& o) {
		std::swap(map_, o.map_);
		std::swap(lmap_, o.lmap_);
		std::swap(num_invalid_, o.num_invalid_);
	}

private:
	/** Removes invalid entries in map_ if safe and needed. */
	void clean_invalid();

	void invalidate(umap::iterator& i)
		{ if(i == map_.end()) return; i->second.valid = false; ++num_invalid_; }
	void validate(umap::iterator& i)
		{ if(i == map_.end()) return; i->second.valid = true; --num_invalid_; }

	void delete_all();

	void add_iter() const { ++num_iters_; }
	void remove_iter() const { --num_iters_; }

	bool is_valid(const umap::const_iterator& i) const { return i != map_.end() && i->second.valid; }

	/**
	 * unit.underlying_id() -> unit_map::node
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

template <typename T>
void unit_map::erase(const T& iter) {
	assert(iter.valid());

	if (erase(iter->get_location()) != 1)
		assert(0);
}

#endif	// UNIT_MAP_H_INCLUDED
