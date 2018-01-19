/*
   Copyright (C) 2006 - 2009 by Rusty Russell <rusty@rustcorp.com.au>
   Copyright (C) 2010 - 2018 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
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

#pragma once

#include "map/location.hpp"
#include "units/ptr.hpp"
#include "utils/reference_counter.hpp"

#include <cassert>
#include <list>
#include <map>
#include <unordered_map>

//#define DEBUG_UNIT_MAP

/**
 * Container associating units to locations.
 *
 * The indirection location -> underlying_id -> unit ensures that iterators stay valid even if WML
 * modifies or moves units on the fly. They even stay valid if a unit is erased from the map and
 * another unit with the same underlying id is inserted in the map. In other words, it is a doubly
 * indexed ordered map with persistent iterators (that never invalidate).
 *
 * @note The unit_map is implemented as 2 maps: an unordered map that stores iterators from the
 * ordered map of reference-counted pointers to units.
 *
 * The unordered map provides O(1) find times. The ordered map ensures ordering of units by
 * underlying_id. The reference counting is what guarantees the persistent iterators. Storing an
 * iterator only prevents that dead unit's id-map entry from being recovered.
 *
 * @note Prefered usages for tight loops follows:
 *
 * Use the std::pair<iterator, bool> format which checks the preconditions and returns false
 * (second) to indicate failure and no change to the unit_map. True indicates success along with the
 * new iterator (first).
 *
 * Storing the result iterator prevents the old iterator from entering the fallback recovery code.
 * This is faster than the old methodology of find to check if empty, insert, and then find to check
 * for success. It is the same method as std::map uses, the C++ way.
 *
 * ================================================================================================
 *
 * unit_iterator i;  //results are stored here
 *
 * ADD:
 *  std::pair<unit_iterator, bool> try_add(units->add(loc, unit));
 *  if(try_add.second) { i = try_add.first; }
 *
 * MOVE:
 *  std::pair<unit_iterator, bool> try_add(units->move(src, dst));
 *  if(try_add.second) { i = try_add.first; }
 *
 * INSERT:
 *  std::pair<unit_iterator, bool> try_add(units->insert(unitp));
 *  if(try_add.second) { i = try_add.first; }
 *
 * REPLACE (preferred over erase and then add)
 *  std::pair<unit_iterator, bool> try_add(units->move(loc, unit));
 *  if(try_add.second){ i = try_add.first; }
 *
 * ================================================================================================
 *
 *
 * @note The previous implementation was 2 binary tree based maps: a location map pointing to another.
 * Lookups were O(2*log(N)) and O(log(N)). Order was implicit in the id map choosen as the base.
 * Persistence was provided by reference counting all iterators collectively and only recovering space
 * when there were no iterators outstanding. Even 1 iterator being stored caused a leak, because all
 * space for dead units was not recovered.
 *
 * @note Units are owned by the container.
 *
 * @note The indirection does not involve map lookups whenever an iterator is dereferenced, it just
 * causes a pointer indirection. The downside is that incrementing iterators is not O(1).
 *
 * @note The code does not involve any magic, so units moved while being iterated upon may be skipped
 * or visited twice.
 *
 * @note Iterators prevent ghost units from being collected, so they should never be stored in data
 * structures, as it will cause slowdowns!
 *
 * @note By popular demand iterators are effectively permanent. They are handles and not iterators.
 * Any access might cause a full lookup. Keeping iterators around holds onto memory.
 */
class unit_map
{
	/// The pointer to the unit and a reference counter to record the number of extant iterators
	/// pointing to this unit.
	struct unit_pod
	{
		unit_pod()
			: unit()
			, ref_count()
		{
		}

		unit_ptr unit;
		mutable n_ref_counter::ref_counter<signed int> ref_count;
	};

	/*
	 * Map of underlying_id to unit and a reference counter. Dead units have a unit pointer equal to nullptr.
	 * The map entry is removed iff the reference counter equals zero and there are no more
	 * iterators pointing to this unit.
	 */
	typedef std::map<size_t, unit_pod> umap;

	/** Map of location to umap iterator. */
	typedef std::unordered_map<map_location, umap::iterator> lmap;

public:
	// ~~~ Begin iterator code ~~~

	struct standard_iter_types
	{
		typedef unit_map container_type;
		typedef unit_map::umap::iterator iterator_type;
		typedef unit value_type;
	};

	struct const_iter_types
	{
		typedef unit_map const container_type;
		typedef unit_map::umap::iterator iterator_type;
		typedef const unit value_type;
	};

	template<typename iter_types>
	struct iterator_base
	{
		typedef std::forward_iterator_tag iterator_category;
		typedef int difference_type;
		typedef typename iter_types::value_type value_type;
		typedef boost::intrusive_ptr<value_type> pointer;
		typedef value_type& reference;
		typedef typename iter_types::container_type container_type;
		typedef typename iter_types::iterator_type iterator_type;

		~iterator_base()
		{
			dec();
		}

		iterator_base()
			: i_()
			, tank_(nullptr)
		{
		}

		iterator_base(iterator_type i, container_type* m)
			: i_(i)
			, tank_(m)
		{
			inc();
			valid_exit();
		}

		iterator_base(const iterator_base& that)
			: i_(that.i_)
			, tank_(that.tank_)
		{
			inc();
			valid_exit();
		}

		iterator_base& operator=(const iterator_base& that)
		{
			if(*this != that) {
				dec();
				tank_ = that.tank_;
				i_ = that.i_;
				inc();
				valid_exit();
			}

			return *this;
		}

		operator iterator_base<const_iter_types>() const
		{
			return iterator_base<const_iter_types>(i_, tank_);
		}

	private:
		/** Constructs an iterator from the location map. */
		iterator_base(lmap::iterator ui, container_type* m)
			: i_(ui->second)
			, tank_(m)
		{
			inc();
			valid_exit();
		}

	public:
		pointer operator->() const
		{
			assert(valid());
			tank_->self_check();
			return i_->second.unit;
		}

		/**
		 * This is exactly the same as operator-> but it's slightly more readable, and can replace
		 * &*iter syntax easily.
		 */
		pointer get_shared_ptr() const
		{
			return operator->();
		}

		reference operator*() const
		{
			tank_->self_check();
			assert(valid());
			return *i_->second.unit;
		}

		iterator_base& operator++()
		{
			assert(valid_entry());
			tank_->self_check();
			iterator_type new_i(i_);
			do {
				++new_i;
			} while((new_i != the_map().end()) && (!new_i->second.unit));
			dec();
			i_ = new_i;
			inc();
			valid_exit();
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
			assert(tank_ && i_ != the_map().begin());
			tank_->self_check();
			iterator_type begin(the_map().begin());
			dec();
			do {
				--i_;
			} while(i_ != begin && (!i_->second.unit));
			inc();

			valid_exit();
			return *this;
		}

		iterator_base operator--(int)
		{
			iterator_base temp(*this);
			operator--();
			return temp;
		}

		bool valid() const
		{
			return (valid_for_dereference() && i_->second.unit);
		}

		explicit operator bool() const
		{
			return valid();
		}

		bool operator==(const iterator_base& rhs) const
		{
			return (tank_ == rhs.tank_) && (i_ == rhs.i_);
		}

		bool operator!=(const iterator_base& rhs) const
		{
			return !operator==(rhs);
		}

		template<typename Y>
		friend struct iterator_base;

	private:
		bool valid_for_dereference() const
		{
			return (tank_ != nullptr) && (i_ != the_map().end());
		}

		bool valid_entry() const
		{
			return ((tank_ != nullptr) && (i_ != the_map().end()));
		}

		void valid_exit() const
		{
			if((tank_ != nullptr) && i_ != the_map().end()) {
				assert(i_->second.ref_count > 0);
			}
		}

		bool valid_ref_count() const
		{
			return (tank_ != nullptr) && (i_ != the_map().end());
		}

		/** Increment the reference counter. */
		void inc()
		{
			if(valid_ref_count()) {
				++(i_->second.ref_count);
			}
		}

		/**
		 * Decrement the reference counter
		 * Delete the umap entry if the unit is gone and the reference counter is zero.
		 *
		 * @note this deletion will advance i_ to the next umap entry.
		 */
		void dec()
		{
			if(valid_ref_count()) {
				assert(i_->second.ref_count != 0);
				if((--(i_->second.ref_count) == 0) && (!i_->second.unit)) {
					iterator_type old = i_++;
					tank_->umap_.erase(old);
				}
			}
		}

		unit_map::umap& the_map() const
		{
			return tank_->umap_;
		}

		friend class unit_map;

		iterator_type i_;      /// local iterator
		container_type* tank_; /// the unit_map for i_
	};

	// ~~~ End iterator code ~~~

	unit_map();
	unit_map(const unit_map& that);
	unit_map& operator=(const unit_map& that);

	~unit_map();
	void swap(unit_map& o);

	typedef iterator_base<standard_iter_types> unit_iterator;
	typedef iterator_base<const_iter_types> const_unit_iterator;

	// Provided as a convenience, since unit_map used to be an std::map.
	typedef unit_iterator iterator;
	typedef const_unit_iterator const_iterator;

	unit_iterator find(size_t id);
	unit_iterator find(const map_location& loc);

	const_unit_iterator find(const map_location& loc) const
	{
		return const_cast<unit_map*>(this)->find(loc);
	}

	const_unit_iterator find(size_t id) const
	{
		return const_cast<unit_map*>(this)->find(id);
	}

	unit_iterator find_leader(int side);

	const_unit_iterator find_leader(int side) const
	{
		return const_cast<unit_map*>(this)->find_leader(side);
	}

	unit_iterator find_first_leader(int side);

	std::vector<unit_iterator> find_leaders(int side);

	std::vector<const_unit_iterator> find_leaders(int side) const;

	size_t count(const map_location& loc) const
	{
		return lmap_.count(loc);
	}

	unit_iterator begin()
	{
		return make_unit_iterator(begin_core());
	}

	const_unit_iterator begin() const
	{
		return make_const_unit_iterator(begin_core());
	}

	unit_iterator end()
	{
		return make_unit_iterator(umap_.end());
	}

	const_unit_iterator end() const
	{
		return make_const_unit_iterator(umap_.end());
	}

	size_t size() const
	{
		return lmap_.size();
	}

	size_t num_iters() const;

	bool empty() const
	{
		return lmap_.empty();
	}

	void clear(bool force = false);

	using umap_retval_pair_t = std::pair<unit_iterator, bool>;

	/**
	 * Adds a copy of unit @a u at location @a l of the map.
	 *
	 * @returns         A pair consisting of an iterator pointing to the new unit (or the unit
	 *                  already occupying that location) and a bool indicating success.
	 *
	 * @note            It is 3 times as fast to attempt to insert a unit at @a l and check for
	 *                  success than it is to verify that the location is empty, insert the unit,
	 *                  and then check the location for the unit.
	 */
	umap_retval_pair_t add(const map_location& l, const unit& u);

	/**
	 * Inserts the unit pointed to by @a p into the map.
	 *
	 * If insertion into either the unit or location map fails, all operations are reverted.
	 *
	 * The one oddity is that to facilitate non-invalidating iterators, the list sometimes has
	 * nullptr pointers which should be used when they correspond to uids previously used.
	 *
	 * @returns         A pair consisting of an iterator pointing to the new unit (or the unit
	 *                  already occupying that location) and a bool indicating success.
	 *
	 * @note            It is 3 times as fast to attempt to insert a unit at @a l and check for
	 *                  success than it is to verify that the location is empty, insert the unit,
	 *                  and then check the location for the unit.
	 *
	 * @note            If the unit::underlying_id is already in use, a new one will be generated.
	 * @note            The map takes ownership of the pointed object only if the operation succeeds.
	 */
	umap_retval_pair_t insert(unit_ptr p);

	/**
	 * Moves a unit from location @a src to location @a dst.
	 *
	 * @returns         A pair consisting of an iterator pointing to the new unit (or the unit
	 *                  already occupying that location) and a bool indicating success.
	 *
	 * @note            It is 3 times as fast to attempt to insert a unit at @a l and check for
	 *                  success than it is to verify that the location is empty, insert the unit,
	 *                  and then check the location for the unit.
	 */
	umap_retval_pair_t move(const map_location& src, const map_location& dst);

	/**
	 * Works like unit_map::add; but @a l is emptied first, if needed.
	 *
	 * @returns         A pair consisting of an iterator pointing to the new unit (or the unit
	 *                  already occupying that location) and a bool indicating success.
	 */
	umap_retval_pair_t replace(const map_location& l, unit_ptr p);

	/**
	 * Erases the unit at location @a l, if any.
	 * @returns the number of erased units (0 or 1).
	 */
	size_t erase(const map_location& l);

	/**
	 * Erases a unit given by a pointer or an iterator.
	 * @pre The unit is on the map.
	 */
	template<typename T>
	size_t erase(const T& iter);

	/**
	 * Extracts a unit from the map.
	 * The unit is no longer owned by the map. It can be reinserted later, if needed.
	 */
	unit_ptr extract(const map_location& loc);

	/** Checks invariants. For debugging purposes only. Doesn't do anything in non-debug mode. */
	bool self_check() const;

	/**
	 * Is the unit in the map?
	 *
	 * @pre                       @p u != @c nullptr
	 *
	 * @param u                   Pointer to the unit to find.
	 *
	 * @returns                   True if found, false otherwise.
	 */
	bool has_unit(const unit* const u) const;

	/** Tests whether a unit exists at the given location. */
	bool has_unit_at(const map_location& loc) const;

private:
	umap::iterator begin_core() const;

	bool is_valid(const umap::const_iterator& i) const
	{
		return is_found(i) && (i->second.unit != nullptr);
	}

	bool is_valid(const lmap::const_iterator& i) const
	{
		return is_found(i) && (i->second->second.unit != nullptr);
	}

	bool is_found(const umap::const_iterator& i) const
	{
		return i != umap_.end();
	}

	bool is_found(const lmap::const_iterator& i) const
	{
		return i != lmap_.end();
	}

	template<typename X>
	unit_map::unit_iterator make_unit_iterator(X const& i)
	{
		if(!is_found(i)) {
			return unit_iterator(umap_.end(), this);
		}

		return unit_iterator(i, this);
	}

	template<typename X>
	unit_map::const_unit_iterator make_const_unit_iterator(X const& i) const
	{
		if(!is_found(i)) {
			return const_unit_iterator(umap_.end(), this);
		}

		return const_unit_iterator(i, this);
	}

	/**
	 * underlying_id -> unit_pod.
	 * This requires that underlying_id be unique (which is enforced in unit_map::insert).
	 */
	mutable umap umap_;

	/**
	 * location -> umap::iterator.
	 */
	lmap lmap_;
};

/** Implement non-member swap function for std::swap (calls @ref unit_map::swap). */
void swap(unit_map& lhs, unit_map& rhs);

template<typename T>
size_t unit_map::erase(const T& iter)
{
	assert(iter.valid());

	return erase(iter->get_location());
}
