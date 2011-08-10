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

#ifndef UNIT_MAP_H_INCLUDED
#define UNIT_MAP_H_INCLUDED

#include "utils/reference_counter.hpp"
#include "map_location.hpp"

#include <cassert>
#include <list>
#include <boost/unordered_map.hpp>

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
class unit_map {
	struct unit_pod {
		unit * unit_;
		mutable n_ref_counter::t_ref_counter<signed int> ref_count_;
	};
	typedef std::list<unit_pod> t_ilist;
	typedef boost::unordered_map<size_t, t_ilist::iterator> t_umap; 
	typedef boost::unordered_map<map_location, t_ilist::iterator> t_lmap;
	

public:
	unit_map() : umap_(), lmap_(), ilist_(), num_iters_(0), num_invalid_(0) { init_end();};
	unit_map(const unit_map &that);
	unit_map& operator=(const unit_map &that);

	/** A unit map with a copy of a single unit in it. */
	unit_map(const map_location &loc, const unit &u);
	~unit_map();
	void swap(unit_map& o);

// ~~~ Begin iterator code ~~~

	template<typename iter_types>
	struct iterator_base
	{
		typedef std::forward_iterator_tag iterator_category;
		typedef int difference_type;
		typedef typename iter_types::value_type value_type;
		typedef typename iter_types::pointer_type pointer;
		typedef typename iter_types::reference_type reference;
		typedef typename iter_types::container_type container_type;
		typedef typename iter_types::iterator_type iterator_type;

		iterator_base(): tank_(NULL), i_() { }

		iterator_base(iterator_type i, container_type *m)
			: tank_(m), i_(i) { 
			inc(); }

		~iterator_base()	{ dec(); }

		template<typename that_types>
		iterator_base(const iterator_base<that_types> &that)
			: tank_(that.tank_), i_(that.i_)
		{ inc(); }

		template<typename that_types>
		iterator_base &operator=(const iterator_base<that_types> &that) {
			dec();
			tank_ = that.tank_;
			i_ = that.i_;
			inc();
			return *this;
		}

	private:
		iterator_base(t_umap::iterator ui, container_type *m)
			: tank_(m), i_(ui->second) { 
			inc(); }

		iterator_base(t_lmap::iterator ui, container_type *m)
			: tank_(m), i_(ui->second) { 
			inc(); }

		void inc() { if(valid()){ ++(i_->ref_count_); } }
		void dec() {
			if( valid() && (--(i_->ref_count_) == 0)  && (i_->unit_ == NULL) ){
				i_ = tank_->erase(i_); } }
		iterator_type the_end() const {return --tank_->end();}
		
	public:
		pointer operator->() const { assert(valid()); return i_->unit_; }
		reference operator*() const { assert(valid()); return *i_->unit_; }

		iterator_base& operator++() {
			assert( valid() );
			iterator_type new_i(i_);
			++new_i;
			while (new_i != the_end() && (new_i->unit_ == NULL)) {
				++new_i;
			}
			dec();
			i_ = new_i;
			inc();
			return *this;
		}

		iterator_base operator++(int) {
			iterator_base temp(*this);
			operator++();
			return temp;
		}

		iterator_base& operator--() {
			assert(  tank_ && i_ != tank_->begin() );
			iterator_type begin(tank_->begin());
			dec();
			do { 
				--i_ ;
			}while(i_ != begin && (i_->unit_ ==  NULL));
			inc();
			return *this;
		}

		iterator_base operator--(int) {
			iterator_base temp(*this);
			operator--();
			return temp;
		}

		bool valid() const { return tank_ && i_ != the_end(); }

		bool operator==(const iterator_base &rhs) const
		{ assert(tank_ == rhs.tank_); return i_ == rhs.i_; }
		bool operator!=(const iterator_base &rhs) const
		{ return !operator==(rhs); }

		//		container_type* get_map() const { return tank_; }

		template<typename Y> friend struct iterator_base;

	private:
		friend class unit_map;
		container_type* tank_;
		iterator_type i_;
	};

	struct standard_iter_types {
		typedef unit_map::t_ilist container_type;
		typedef unit_map::t_ilist::iterator iterator_type;
		typedef unit value_type;
		typedef value_type* pointer_type;
		typedef value_type& reference_type;
	};

	struct const_iter_types {
		typedef unit_map::t_ilist container_type;
		typedef unit_map::t_ilist::iterator iterator_type;
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

	unit_iterator find(size_t id);

	unit_iterator find(const map_location &loc);

	const_unit_iterator find(const map_location &loc) const { return const_cast<unit_map *>(this)->find(loc); }
	const_unit_iterator find(size_t id) const { return const_cast<unit_map *>(this)->find(id); }

	unit_iterator find_leader(int side);
	const_unit_iterator find_leader(int side) const { return const_cast<unit_map *>(this)->find_leader(side); }
	unit_iterator find_first_leader(int side);

	std::vector<unit_iterator> find_leaders(int side);
	std::vector<const_unit_iterator> find_leaders(int side) const;

	size_t count(const map_location& loc) const { return static_cast<size_t>(lmap_.count(loc)); }

	unit_iterator begin() { return unit_iterator( begin_core(), & ilist_); }
	const_unit_iterator begin() const { return const_unit_iterator( begin_core(), & ilist_); }

	unit_iterator end() { return iterator(the_end_, & ilist_); }
	const_unit_iterator end() const { return const_iterator(the_end_, & ilist_); }

	size_t size() const { return lmap_.size(); }
	size_t num_iters() const ;

	void clear(bool force = false);

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

private:
	void init_end(){
		assert(ilist_.empty());
		ilist_.push_front(unit_pod());
		the_end_ = ilist_.begin();
	};
	
	t_ilist::iterator begin_core() const ;

	/** Removes invalid entries in map_ if safe and needed. */
	//	void clean_invalid();

	// void add_iter() const { ++num_iters_; }
	// void remove_iter() const { --num_iters_; }

	bool is_valid(const t_ilist::const_iterator &i) const
	{ return i != the_end_ && (i->unit_ !=  NULL); }
	bool is_valid(const t_umap::const_iterator &i) const
	{ return i != umap_.end() && (i->second->unit_ != NULL); }

	/**
	 * underlying_id -> ilist::iterator. This requires that underlying_id be
	 * unique (which is enforced in unit_map::insert).
	 */
	t_umap umap_;

	/**
	 * location -> ilist::iterator.
	 */
	t_lmap lmap_;

	/**
	 *  List of unit pointers and ref counts for the iterators
	 */
	mutable t_ilist ilist_;
	t_ilist::iterator the_end_; /// Last list item

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
