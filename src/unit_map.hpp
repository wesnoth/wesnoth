/*
   Copyright (C) 2006 - 2009 by Rusty Russell <rusty@rustcorp.com.au>
   Copyright (C) 2010 - 2013 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
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
 * underlying id is inserted in the map.  In other words it is a doubly indexed ordered map
 with persistent iterators (that never invalidate)

 @note The unit_map is implemented as 2 unordered maps storing iterators from a list of reference counted pointers to units.
 The unordered maps provide O(1) find times.  The list allows arbitrary ordering of units (not yet implemented).
 The reference counting is what guarantees the persistent iterators.
 Storing an iterator prevents only that dead unit's list location from being recovered.

@note Prefered usages for tight loops follows.
Use the std::pair<iterator, bool> format which checks the preconditions and returns
false in the bool to indicate failure with no change to the unit_map.  true indicates success and the new iterator is in first.
Storing the result iterator prevents the old iterator from entering the fallback recovery code.
This is faster than the old methodology of find to check if empty, insert and then find to check for success.
It is the same method as std::map uses, the C++ way.

unit_iterator i;  //results are stored here

Add
std::pair<unit_iterator, bool> try_add(units->add(loc, unit));
if(try_add.second){i = try_add.first;}

Move
std::pair<unit_iterator, bool> try_add(units->move(src, dst));
if(try_add.second){i = try_add.first;}

Insert
std::pair<unit_iterator, bool> try_add(units->insert(unitp));
if(try_add.second){i = try_add.first;}

Replace  (preferred over erase and then add)
std::pair<unit_iterator, bool> try_add(units->move(loc, unit));
if(try_add.second){i = try_add.first;}




 @note The previous implementation was 2 binary tree based maps one the location map pointing to the other.
 Lookups were O(2*log(N)) and O(log(N)).  Order was implicit in the id map choosen as the base.
 Persistence was provided by reference counting all iterators collectively and only recovering space when
 there were no iterators outstanding.  Even 1 iterator being stored caused a leak, because
 all space for dead units is not recovered.

 * @note Units are owned by the container.
 * @note The indirection does not involve map lookups whenever an iterator
 *       is dereferenced, it just causes a pointer indirection. The downside
 *       is that incrementing iterator is not O(1).
 * @note The code does not involve any magic, so units moved while being
 *       iterated upon may be skipped or visited twice.
 * @note Iterators prevent ghost units from being collected. So they should
 *       never be stored into data structures, as it will cause slowdowns!
 @note By popular demand iterators are effectively permanent.  They are handles and not iterators.
  Any access might cause a full lookup.  Keeping iterators around holds onto memory.
 */
class unit_map {
	/// The pointer to the unit and a reference counter to record the number of extant iterators
	/// pointing to this unit.
	struct unit_pod {

		unit_pod()
			: unit(NULL)
			, ref_count()
			, deleted_uid(0)
		{
		}

		class unit * unit;
		mutable n_ref_counter::t_ref_counter<signed int> ref_count;

		unsigned deleted_uid;  ///UID of the deleted, moved, added, or otherwise invalidated iterator to facilitate a new lookup.
	};

	/// A list pointing to unit and their reference counters.  Dead units have a unit pointer equal to NULL.
	/// The list element is remove iff the reference counter equals zero and there are no more
	///iterators pointing to this unit.
	typedef std::list<unit_pod> t_ilist;

	///Maps of id and location to list iterator.
	///@note list iterators never invalidate due to resizing or deletion.
	typedef boost::unordered_map<size_t, t_ilist::iterator> t_umap;
	typedef boost::unordered_map<map_location, t_ilist::iterator> t_lmap;

public:

// ~~~ Begin iterator code ~~~

	struct standard_iter_types {
		typedef unit_map container_type;
		typedef unit_map::t_ilist::iterator iterator_type;
		typedef unit value_type;
	};

	struct const_iter_types {
		typedef unit_map const container_type;
		typedef unit_map::t_ilist::iterator iterator_type;
		typedef const unit value_type;
	};

	template<typename iter_types>
	struct iterator_base
	{
		typedef std::forward_iterator_tag iterator_category;
		typedef int difference_type;
		typedef typename iter_types::value_type value_type;
		typedef value_type* pointer;
		typedef value_type& reference;
		typedef typename iter_types::container_type container_type;
		typedef typename iter_types::iterator_type iterator_type;

		~iterator_base()	{ dec(); }

		iterator_base(): i_(), tank_(NULL) { }

		iterator_base(iterator_type i, container_type *m) : i_(i), tank_(m) {
			inc();
			valid_exit();
		}

		iterator_base(const iterator_base &that) : i_(that.i_), tank_(that.tank_) {
			inc();
			valid_exit();
		}

		iterator_base &operator=(const iterator_base &that) {
			if(*this != that){
				dec();
				tank_ = that.tank_;
				i_ = that.i_;
				inc();
				valid_exit();
			}
			return *this;
		}

		operator iterator_base<const_iter_types> () const {
			return iterator_base<const_iter_types>(i_, tank_);
		}

	private:
		///Construct an iterator from the uid map
		iterator_base(t_umap::iterator ui, container_type *m) : i_(ui->second), tank_(m) {
			inc();
			valid_exit();
		}

		///Construct an iterator from the location map
		iterator_base(t_lmap::iterator ui, container_type *m) : i_(ui->second), tank_(m) {
			inc();
			valid_exit();
		}

	public:
		pointer operator->() const   {
			assert(valid());
			tank_->self_check();
			return  i_->unit; }
		reference operator*() const {
			tank_->self_check();
#if 0
			// debug!
			if(!valid()){
				if(!tank_){std::cerr<<"tank is NULL"<<"\n";}
				if(i_==the_end()){std::cerr<<"i_ is the end"<<"\n";}
				if(i_->unit==NULL){std::cerr<<"i_ unit is NULL with uid="<<i_->deleted_uid<<"\n";}
			}
#endif
			assert(valid());
			return *i_->unit; }

		iterator_base& operator++() {
			assert( valid_entry() );
			tank_->self_check();
			iterator_type new_i(i_);
			do{
				++new_i;
			}while ((new_i->unit == NULL) && (new_i != the_end() )) ;
			dec();
			i_ = new_i;
			inc();
			valid_exit();
			return *this;
		}

		iterator_base operator++(int) {
			iterator_base temp(*this);
			operator++();
			return temp;
		}

		iterator_base& operator--() {
			assert(  tank_ && i_ != the_list().begin() );
			tank_->self_check();
			iterator_type begin(the_list().begin());
			dec();
			do {
				--i_ ;
			}while(i_ != begin && (i_->unit ==  NULL));
			inc();

			valid_exit();
			return *this;
		}

		iterator_base operator--(int) {
			iterator_base temp(*this);
			operator--();
			return temp;
		}

		bool valid() const {
			if(valid_for_dereference()) {
				if(i_->unit == NULL){
					recover_unit_iterator(); }
				return  i_->unit != NULL;
			}
			return false; }

		bool operator==(const iterator_base &rhs) const { return (tank_ == rhs.tank_) && ( i_ == rhs.i_ ); }
		bool operator!=(const iterator_base &rhs) const { return !operator==(rhs); }

		//		container_type* get_map() const { return tank_; }

		template<typename Y> friend struct iterator_base;

	private:
		bool valid_for_dereference() const { return (tank_ != NULL) && (i_ != the_end()); }
		bool valid_entry() const { return  ((tank_ != NULL) && (i_ != the_end())) ; }
		void valid_exit() const {
			if(tank_ != NULL) {
				assert(!the_list().empty());
				assert(i_ != the_list().end());
				if(i_ != the_end()){
					assert(i_->ref_count > 0);
				} else {
					assert(i_->ref_count == 1);
				}
			}}
		bool valid_ref_count() const { return (tank_ != NULL) && (i_ != the_end()) ; }

		///Increment the reference counter
		void inc() { if(valid_ref_count()) { ++(i_->ref_count); } }

		///Decrement the reference counter
		///Delete the list element and the dangling umap reference if the unit is gone and the reference counter is zero
		///@note this deletion will advance i_ to the next list element.
		void dec() {
			if( valid_ref_count() ){
				assert(i_->ref_count != 0);
				if( (--(i_->ref_count) == 0)  && (i_->unit == NULL) ){
					if(tank_->umap_.erase(i_->deleted_uid) != 1){
						tank_->error_recovery_externally_changed_uid(i_); }
					i_ = the_list().erase(i_);
				} } }

		unit_map::t_ilist & the_list() const { return tank_->ilist_; }

		///Returns the sentinel at the end of the list.
		///This provides a stable unit_iterator for hoisting out of loops
		iterator_type the_end() const { return tank_->the_end_; }

		/**
	 * Attempt to find a deleted unit in the unit_map, by looking up the stored UID.
	 * @pre deleted_uid != 0
	 */
		void recover_unit_iterator() const {
			assert(i_->deleted_uid != 0);
			iterator_base new_this( tank_->find( i_->deleted_uid ));
			const_cast<iterator_base *>(this)->operator=( new_this );
		}
		friend class unit_map;

		iterator_type i_; ///local iterator
		container_type* tank_; ///the unit_map for i_
	};


// ~~~ End iterator code ~~~


	unit_map();
	unit_map(const unit_map &that);
	unit_map& operator=(const unit_map &that);

	~unit_map();
	void swap(unit_map& o);

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

	size_t count(const map_location& loc) const { return lmap_.count(loc); }

	unit_iterator begin() { return make_unit_iterator( begin_core() ); }
	const_unit_iterator begin() const { return make_const_unit_iterator( begin_core() ); }

	unit_iterator end() { return make_unit_iterator(the_end_); }
	const_unit_iterator end() const { return make_const_unit_iterator(the_end_); }

	size_t size() const { return lmap_.size(); }
	size_t num_iters() const ;

	void clear(bool force = false);

	/**
	 * Adds a copy of unit @a u at location @a l of the map.
	 @return std::pair<unit_iterator, bool>  a bool indicating success and
	 an iterator pointing to the new unit, or the unit already occupying location.
	 @note It is 3 times as fast to attempt to insert a unit at @a l and check for
	 success than it is to verify that the location is empty, insert the unit check the
	 location for the unit.
	 */
	std::pair<unit_iterator, bool> add(const map_location &l, const unit &u);

	/**
	 * Adds the unit to the map.
	 @return std::pair<unit_iterator, bool>  a bool indicating success and
	 an iterator pointing to the new unit, or the unit already occupying location.
	 @note It is 3 times as fast to attempt to insert a unit at @a l and check for
	 success than it is to verify that the location is empty, insert the unit check the
	 location for the unit.
	 * @note If the unit::underlying_id is already in use, a new one
	 *       will be generated.
	 * @note The map takes ownership of the pointed object, only if it succeeds.
	 */
	std::pair<unit_iterator, bool> insert(unit *p);

	/**
	 * Moves a unit from location @a src to location @a dst.
	 @return std::pair<unit_iterator, bool>  a bool indicating success and
	 an iterator pointing to the new unit, or the unit already occupying location.
	 @note It is 3 times as fast to attempt to insert a unit at @a l and check for
	 success than it is to verify that the location is empty, insert the unit check the
	 location for the unit.
	 */
	std::pair<unit_iterator, bool> move(const map_location &src, const map_location &dst);

	/**
	 * Works like unit_map::add; but @a l is emptied first, if needed.
	 @return std::pair<unit_iterator, bool>  a bool indicating success and
	 an iterator pointing to the new unit, or the unit already occupying location.
	 */
	std::pair<unit_iterator, bool> replace(const map_location &l, const unit &u);

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
	size_t erase(const T &iter);

	/**
	 * Extracts a unit from the map.
	 * The unit is no longer owned by the map.
	 * It can be reinserted later, if needed.
	 */
	unit *extract(const map_location &loc);

	///Checks invariants.  For debugging purposes only.  Doesn't do anything in non-debug mode.
	bool self_check() const
#ifndef DEBUG
	{ return true; }
#endif
	;

	/**
	 * Is the unit in the map?
	 *
	 * @pre                       @p u != @c NULL
	 *
	 * @param u                   Pointer to the unit to find.
	 *
	 * @returns                   True if found, false otherwise.
	 */
	bool has_unit(const unit * const u);

private:
	///Creates and inserts a unit_pod called the_end_ as a sentinel in the ilist
	void init_end(){
		assert(ilist_.empty());
		unit_pod upod;
		upod.unit = NULL;
		upod.deleted_uid = 0;
		++upod.ref_count; //dummy count
		ilist_.push_front(upod);
		the_end_ = ilist_.begin();
	};

	t_ilist::iterator begin_core() const ;

	bool is_valid(const t_ilist::const_iterator &i) const {
		return i != the_end_  && is_found(i) && (i->unit !=  NULL); }
	bool is_valid(const t_umap::const_iterator &i) const {
		return is_found(i) && (i->second->unit != NULL); }
	bool is_valid(const t_lmap::const_iterator &i) const {
		return is_found(i) && (i->second->unit != NULL); }

	bool is_found(const t_ilist::const_iterator &i) const { return i != ilist_.end(); }
	bool is_found(const t_umap::const_iterator &i) const { return i != umap_.end() ; }
	bool is_found(const t_lmap::const_iterator &i) const { return i != lmap_.end(); }

	template <typename X>
	unit_map::unit_iterator make_unit_iterator(X const & i) {
		if (!is_found( i )) { return unit_iterator(the_end_, this); }
		return unit_iterator(i , this); }
	template <typename X>
	unit_map::const_unit_iterator make_const_unit_iterator(X const & i) const {
		if (!is_found( i )) { return const_unit_iterator(the_end_, this); }
		return const_unit_iterator(i , this); }

	///Finds and deletes the umap_ item associated with @a lit when the underlying_id()
	///has been changed externally after insertion and before extraction
	void error_recovery_externally_changed_uid(t_ilist::iterator const & lit) const;

	/**
	 * underlying_id -> ilist::iterator. This requires that underlying_id be
	 * unique (which is enforced in unit_map::insert).
	 */
	mutable t_umap umap_;

	/**
	 * location -> ilist::iterator.
	 */
	t_lmap lmap_;

	/**
	 *  List of unit pointers and reference counts for the iterators
	 */
	mutable t_ilist ilist_;

	/// The last list item, a sentinel that allows BOOST::foreach to hoist end()
	t_ilist::iterator the_end_;

};

template <typename T>
size_t unit_map::erase(const T& iter) {
	assert(iter.valid());

	return erase(iter->get_location());
}



#endif	// UNIT_MAP_H_INCLUDED
