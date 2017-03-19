/*
   Copyright (C) 2014 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file A list whose iterators never invalidate.
 */

#ifndef UTILS_SMART_LIST_HPP_INCLUDED
#define UTILS_SMART_LIST_HPP_INCLUDED

#include <cassert>
#include <iterator>
#include <vector>

namespace utils
{


/* ** smart_list ** */

/// This is a variant on a list that never invalidates iterators (unless, of
/// course, the list ceases to exist). In particular, an erase() will only flag
/// an element for removal from the list. Flagged elements are ignored by list
/// functions, but they still exist from the perspective of iterators that had
/// pointed (and still point) to the element.
///
/// Assignment is incompatible with the goal of preserving iterators, so it is
/// not implemented (declared private though, to emphasize that this is
/// intentional).
///
/// The insert() and erase() members are static, as they do not require a
/// reference to the list.
///
/// Instead of being undefined, ++end() is equal to begin(), and --begin() is
/// equal to end().
template <class Data>
class  smart_list
{
	/// Nodes in the smart list.
	struct node_t
	{
		/// Default constructor. This is for creating the root of a list.
		node_t() : dat_ptr(nullptr), ref_count(1), next(this), prev(this)
		{}
		/// Initialized constructor. This is for creating a node in a list.
		explicit node_t(const Data & d) : dat_ptr(new Data(d)), ref_count(1), next(nullptr), prev(nullptr)
		{}
		/// Destructor.
		~node_t();

		/// The data of the node. This is nullptr for a list's root.
		Data * const dat_ptr;
		/// ref_count counts 1 for the list and 2 for each iterator pointing to
		/// this node. (So nodes are flagged for deletion when ref_count is even.)
		mutable size_t ref_count;
		// List linking.
		node_t * next;
		node_t * prev;

	private: // Disallowed (and unneeded) functions (declared but not implemented).
		/// No assignments.
		node_t & operator=(const node_t &);
		/// No copying.
		node_t(const node_t & that);
	};

	/// The base for the list's iterator classes.
	/// Value is the value type of this iterator.
	/// Reversed is true for the reverse iterators.
	/// The actual iterators must have neither data nor destructors.
	template <class Value, bool Reversed>
	class iterator_base
	{
		friend class smart_list<Data>;
		typedef typename smart_list<Data>::node_t node_t;

	public:
		// Types required of an iterator:
		typedef Value                           value_type;
		typedef value_type *                    pointer;
		typedef value_type &                    reference;
		typedef std::bidirectional_iterator_tag iterator_category;
		typedef ptrdiff_t                       difference_type;

	protected: // Construct this via derived classes.
		/// Default constructor
		iterator_base() : ptr_(nullptr) {}
		/// Initialized constructor
		explicit iterator_base(node_t * ptr) : ptr_(ptr)
		{ skip_flagged(); refer(); }
		/// Conversion constructors.
		template <class V, bool R>
		iterator_base(const iterator_base<V,R> & that) : ptr_(that.ptr_)
		{ refer(); }
		/// Copy constructor (the default overrides the conversion template).
		iterator_base(const iterator_base & that) : ptr_(that.ptr_)
		{ refer(); }
	public:
		/// Destructor
		/// (Not virtual, since the derived classes are mere shells.)
		~iterator_base()                   { unref(ptr_); }

		// Assignment
		iterator_base & operator=(const iterator_base & that)
		// NOTE: This is defined here because MSVC was unable to match the
		//       definition to the declaration when they were separate.
		{
			// Update our pointer.
			node_t * old_ptr = ptr_;
			ptr_ = that.ptr_;

			// Update reference counts.
			refer();
			unref(old_ptr);

			return *this;
		}

		// Comparison:
		bool operator==(const iterator_base & that) const  { return ptr_ == that.ptr_; }
		bool operator!=(const iterator_base & that) const  { return ptr_ != that.ptr_; }

		// Dereference:
		reference operator*() const        { return *ptr_->dat_ptr; }
		pointer  operator->() const        { return ptr_->dat_ptr; }

		// Increment/decrement:
		iterator_base & operator++()       { advance(Reversed);  return *this; }
		iterator_base & operator--()       { advance(!Reversed); return *this; }
		iterator_base operator++(int)
		// NOTE: This is defined here because MSVC was unable to match the
		//       definition to the declaration when they were separate.
		{
			iterator_base retval(*this);
			advance(Reversed);
			return retval;
		}
		iterator_base operator--(int)
		// NOTE: This is defined here because MSVC was unable to match the
		//       definition to the declaration when they were separate.
		{
			iterator_base retval(*this);
			advance(!Reversed);
			return retval;
		}

		/// Test for being in a list, rather than past-the-end (or unassigned).
		bool derefable() const             { return derefable(ptr_); }


	private: // functions
		/// Test for being in a list, rather than past-the-end (or unassigned).
		static bool derefable(node_t * ptr){ return ptr  &&  ptr->dat_ptr; }
		/// Advances our pointer to an unflagged node, possibly in the reverse direction.
		void advance(bool reverse);
		/// Advances our pointer one step, possibly in the reverse direction.
		void inc(bool reverse)             { ptr_ = reverse ? ptr_->prev : ptr_->next; }
		/// Make sure we are not pointing to a flagged node.
		void skip_flagged(bool reverse=Reversed);
		/// Add a reference.
		void refer() const;
		/// Remove a reference.
		static void unref(node_t * old_ptr);

	private: // data
		node_t * ptr_;
	};
	template <class Value, bool Reversed> friend class iterator_base;

public: // The standard list interface, minus assignment.
	typedef Data                value_type;
	typedef value_type *        pointer;
	typedef value_type &        reference;
	typedef const value_type &  const_reference;
	typedef size_t              size_type;

	// Making these structs instead of typedefs cuts down on the length of
	// compiler messages and helps define which conversions are allowed.
	struct iterator : public iterator_base<Data, false> {
		/// Default constructor
		iterator() : iterator_base<Data, false>() {}
		/// Initialized constructor
		explicit iterator(node_t * ptr) : iterator_base<Data, false>(ptr) {}
		/// Copy constructor.
		iterator(const iterator & that) : iterator_base<Data, false>(that) {}
		/// Conversion from reverse_iterator.
		explicit iterator(const iterator_base<Data, true> & that) : iterator_base<Data, false>(that) {}
	};
	struct const_iterator : public iterator_base<const Data, false> {
		/// Default constructor
		const_iterator() : iterator_base<const Data, false>() {}
		/// Initialized constructor
		explicit const_iterator(node_t * ptr) : iterator_base<const Data, false>(ptr) {}
		/// Copy constructor.
		const_iterator(const const_iterator & that) : iterator_base<const Data, false>(that) {}
		/// Conversion from iterator.
		const_iterator(const iterator_base<Data, false> & that) : iterator_base<const Data, false>(that) {}
		/// Conversion from const_reverse_iterator.
		explicit const_iterator(const iterator_base<const Data, true> & that) : iterator_base<const Data, false>(that) {}
	};
	struct reverse_iterator : public iterator_base<Data, true> {
		/// Default constructor
		reverse_iterator() : iterator_base<Data, true>() {}
		/// Initialized constructor
		explicit reverse_iterator(node_t * ptr) : iterator_base<Data, true>(ptr) {}
		/// Copy constructor.
		reverse_iterator(const reverse_iterator & that) : iterator_base<Data, true>(that) {}
		/// Conversion from iterator.
		explicit reverse_iterator(const iterator_base<Data, false> & that) : iterator_base<Data, true>(that) {}
	};
	struct const_reverse_iterator : public iterator_base<const Data, true> {
		/// Default constructor
		const_reverse_iterator() : iterator_base<const Data, true>() {}
		/// Initialized constructor
		explicit const_reverse_iterator(node_t * ptr) : iterator_base<const Data, true>(ptr) {}
		/// Copy constructor.
		const_reverse_iterator(const const_reverse_iterator & that) : iterator_base<const Data, true>(that) {}
		/// Conversion from reverse_iterator.
		const_reverse_iterator(const iterator_base<Data, true> & that) : iterator_base<const Data, true>(that) {}
		/// Conversion from const_iterator.
		explicit const_reverse_iterator(const iterator_base<const Data, false> & that) : iterator_base<const Data, true>(that) {}
	};


	smart_list() : root_() {}
	smart_list(size_type n);
	smart_list(size_type n, const value_type & d);
	smart_list(const smart_list & that);
	template <class InputIterator>
	smart_list(const InputIterator & f, const InputIterator & l);
	~smart_list()                          { clear(); }

	iterator begin()                       { return iterator(root_.next); }
	iterator end()                         { return iterator(&root_); }
	const_iterator begin() const           { return const_iterator(root_.next); }
	const_iterator end() const             { return const_iterator(const_cast<node_t *>(&root_)); }
	reverse_iterator rbegin()              { return reverse_iterator(root_.prev); }
	reverse_iterator rend()                { return reverse_iterator(&root_); }
	const_reverse_iterator rbegin() const  { return const_reverse_iterator(root_.prev); }
	const_reverse_iterator rend() const    { return const_reverse_iterator(const_cast<node_t *>(&root_)); }

	size_type size() const;
	size_type max_size() const             { return size_type(-1); }
	/// Note: if empty() returns true, the list might still contain nodes
	/// flagged for deletion.
	bool empty() const                     { return begin() == end(); }

	// Note: these functions use iterators so flagged nodes are skipped.
	reference front()                      { return *begin(); }
	const_reference front() const          { return *begin(); }
	reference back()                       { return *rbegin(); }
	const_reference back() const           { return *rbegin(); }
	void push_front(const value_type & d)  { insert(root_.next, d); }
	void push_back(const value_type & d)   { insert(&root_, d); }
	void pop_front()                       { erase(begin()); }
	void pop_back()                        { erase(iterator(rbegin())); }

	void swap(smart_list & that);

	static iterator insert(const iterator & pos, const value_type & d);
	template <class InputIterator>
	static void insert(const iterator & pos, const InputIterator & f, const InputIterator & l);
	static void insert(const iterator & pos, size_type n, const value_type & d);

	static iterator erase(const iterator & pos);
	static iterator erase(const iterator & start, const iterator & stop);
	void clear()                           { erase(begin(), end()); }

	void resize(size_type n)               { resize(n, value_type()); }
	void resize(size_type n, const value_type & d);

	void splice(const iterator & pos, smart_list & L);
	void splice(const iterator & pos, smart_list & L, const iterator & i);
	void splice(const iterator & pos, smart_list & L, const iterator & f, const iterator & l);

	void remove(const value_type & value);
	template <class Predicate>
	void remove_if(const Predicate & p);

	void unique()                          { unique(std::equal_to<value_type>()); }
	template <class BinaryPredicate>
	void unique(const BinaryPredicate p);

	void merge(smart_list & L)             { merge(L, std::less<value_type>()); }
	template <class BinaryPredicate>
	void merge(smart_list & L, const BinaryPredicate & p);

	void sort()                            { sort(std::less<value_type>()); }
	template <class BinaryPredicate>
	void sort(const BinaryPredicate & p);

private: // functions
	/// No implementation of operator=() since that would not preserve iterators.
	smart_list & operator=(const smart_list &);

	/// Returns true if @a node has been flagged for deletion.
	static bool flagged(const node_t & node)  { return node.ref_count % 2 == 0; }
	/// Flags @a node for deletion.
	static void flag(const node_t & node)     { node.ref_count &= ~size_t(1); }

	// Low-level implementations. The list is designed so that a reference
	// to the list itself is not needed.
	static node_t * insert(node_t * const pos, const value_type & d);
	static node_t * check_erase(node_t * const pos);
	static void link(node_t * const pos, node_t & begin_link, node_t & end_link);
	static void unlink(node_t & begin_unlink, node_t & end_unlink);
	static void splice(node_t * const pos, node_t & f, node_t & l);


private: // data
	/// Root of the list.
	node_t root_;
	// I started doing this using STL and Boost, but it was taking longer than
	// it would take for me to write stuff from scratch.
};

/** Sized constructor */
template <class Data>
inline smart_list<Data>::smart_list(size_type n) : root_()
{
	for ( size_type i = 0; i < n; ++i )
		push_back(value_type());
}
/** Sized constructor with a value */
template <class Data>
inline smart_list<Data>::smart_list(size_type n, const value_type & d) : root_()
{
	for ( size_type i = 0; i < n; ++i )
		push_back(d);
}
/** Copy constructor */
template <class Data>
inline smart_list<Data>::smart_list(const smart_list & that) : root_()
{
	// Copy non-flagged nodes.
	for ( const_iterator it = that.begin(); it.derefable(); ++it )
		push_back(*it);
}
/** Constructor from a range */
template <class Data> template <class InputIterator>
inline smart_list<Data>::smart_list(const InputIterator & f, const InputIterator & l) :
	root_()
{
	for ( InputIterator it = f; f != l; ++f )
		push_back(*it);
}


/** The size of the list, not counting flagged nodes. */
template <class Data>
inline typename smart_list<Data>::size_type smart_list<Data>::size() const
{
	size_type count = 0;
	// Look for nodes not flagged for deletion.
	for ( const_iterator it = begin(); it.derefable(); ++it )
		++count;
	return count;
}

/** Swaps two lists. Done in constant time, with no iterator invalidation. */
template <class Data>
inline void smart_list<Data>::swap(smart_list & that)
{
	smart_list temp_list;

	// Swap, using splices instead of assignments.
	temp_list.splice(temp_list.end(), that);
	that.splice(that.end(), *this);
	splice(end(), temp_list);
}

/** Insert a node before @a pos. */
template <class Data>
inline typename smart_list<Data>::iterator smart_list<Data>::insert
	(const iterator & pos, const value_type & d)
{
	return iterator(insert(pos.ptr_, d));
}
/** Insert a range before @a pos. */
template <class Data>
template <class InputIterator>
inline void smart_list<Data>::insert(const iterator & pos, const InputIterator & f,
                                     const InputIterator & l)
{
	for ( InputIterator it = f; it != l; ++it )
		insert(pos.ptr_, *it);
}
/** Insert @a n copies of @a d at @a pos. */
template <class Data>
inline void smart_list<Data>::insert(const iterator & pos, size_type n,
                                     const value_type & d)
{
	for ( size_type i = 0; i < n; ++i )
		insert(pos.ptr_, d);
}

/** Erase the node at @a pos. */
template <class Data>
inline typename smart_list<Data>::iterator smart_list<Data>::erase
	(const iterator & pos)
{
	flag(*pos.ptr_); // We know *pos cannot get deleted yet because pos points to it.
	return iterator(pos.ptr_->next);
}
/** Erase a range of nodes. */
template <class Data>
inline typename smart_list<Data>::iterator smart_list<Data>::erase
	(const iterator & start, const iterator & stop)
{
	// Loop through all nodes from start to stop.
	// We cannot rely on iterators because *stop might be flagged.
	node_t * node_ptr = start.ptr_;
	while ( node_ptr != stop.ptr_  &&  iterator::derefable(node_ptr) ) {
		flag(*node_ptr);
		node_ptr = check_erase(node_ptr);
	}

	// node_ptr is more reliable than stop because of the derefable() condition.
	return iterator(node_ptr);
}

/**
 * Resize the list.
 * This does not count flagged nodes.
 */
template <class Data>
inline void smart_list<Data>::resize(size_type n, const value_type & d)
{
	size_type count = 0;
	iterator it = begin();
	iterator _end = end();

	// See how our current size compares to n.
	// (Not calling size(), since that would do at least as much work.)
	while ( it != _end  &&  count < n ) {
		++it;
		++count;
	}

	if ( count < n )
		// We need more nodes.
		insert(_end, n - count, d);
	else if ( it != _end )
		// We need to remove nodes.
		erase(it, _end);
}

/** Splice all of @a L into *this before @a pos. */
template <class Data>
inline void smart_list<Data>::splice(const iterator & pos, smart_list & L)
{
	if ( L.root_.next != &L.root ) // if L has nodes to splice.
		splice(pos.ptr_, *L.root_.next, *L.root_.prev);
}
/** Splice the node @a i points to (assumed from @a L) into *this before @a pos. */
template <class Data>
inline void smart_list<Data>::splice(const iterator & pos, smart_list & /*L*/,
                                     const iterator & i)
{
	if ( i.ptr_.derefable() )
		splice(pos.ptr_, *i.ptr_, *i.ptr_);
}
/** Splice a range (assumed from @a L) into *this before @a pos. */
template <class Data>
inline void smart_list<Data>::splice(const iterator & pos, smart_list & /*L*/,
                                     const iterator & f, const iterator & l)
{
	// Abort on degenerate cases.
	if ( !f.derefable()  ||  f == l )
		return;

	// Splice from f to the node before l.
	splice(pos.ptr_, *(f.ptr_), *(l.ptr_->prev));
}

/** Remove all nodes whose data equals @a value. */
template <class Data>
inline void smart_list<Data>::remove(const value_type & value)
{
	// Look for elements to erase.
	iterator it = begin(), _end = end();
	while ( it != _end )
		if ( *it == value )
			it = erase(it);
		else
			++it;
}
/** Remove all nodes whose data satisfies @a p. */
template <class Data>
template <class Predicate>
inline void smart_list<Data>::remove_if(const Predicate & p)
{
	// Look for elements to erase.
	iterator it = begin(), _end = end();
	while ( it != _end() )
		if ( p(*it) )
			it = erase(it);
		else
			++it;
}

/** Remove nodes equal (under @a p) to their immediate predecessor. */
template <class Data>
template <class BinaryPredicate>
inline void smart_list<Data>::unique(const BinaryPredicate p)
{
	// Initial state.
	iterator _end = end();
	iterator cur = begin();
	iterator prev = cur;
	// Look at the second node, making the first node "previous".
	if ( cur != _end )
		++cur;

	// Look for elements equal to their predecessors.
	while ( cur != _end )
		if ( p(*prev, *cur) )
			// Duplicate. Remove it.
			cur = erase(cur);
		else
			// Update the tracking of our previous element.
			prev = cur++;
}

/**
 * Merge @a L into *this, using @a p to determine order.
 * If both lists are sorted under @a p, then so is the merged list.
 * This is stable; equivalent nodes retain their order, with nodes of *this
 * considered to come before elements of @a L.
 */
template <class Data>
template <class BinaryPredicate>
inline void smart_list<Data>::merge(smart_list & L, const BinaryPredicate & p)
{
	// Merging a list into itself is a no-op, not a duplication.
	if ( this == &L )
		return;

	node_t * dest = root_.next;
	node_t * source = L.root_.next;

	// Basic merge loop. Continues until one of the lists is depleted.
	while ( iterator::derefable(dest)  &&  iterator::derefable(source) ) {
		if ( p(*(source->dat_ptr), *(dest->dat_ptr)) ) {
			// We found something to merge in. See if we can merge multiple
			// nodes at once.
			node_t * end_merge = source->next;
			while ( iterator::derefable(end_merge)  &&  p(*(end_merge->dat_ptr), *(dest->dat_ptr)) )
				end_merge = end_merge->next;
			// Now end_merge is one past the nodes to merge.
			// Move the nodes over.
			splice(dest, *source, *(end_merge->prev));
			// Advance the source.
			source = end_merge;
		}
		else
			// Advance the destination.
			dest = dest->next;
	}

	// Append any remaining nodes from L.
	if ( iterator::derefable(source) ) // hence, we made it to our end.
		splice(&root_, *source, *(L.root_.prev));
}

/**
 * Sort *this, using the order given by @a p.
 * This is stable; equivalent nodes retain their order.
 * This is an efficient sort, O(n lg n).
 */
template <class Data>
template <class BinaryPredicate>
inline void smart_list<Data>::sort(const BinaryPredicate & p)
{
	// We'll leverage merge() to do a merge sort.

	// First, get the real size of the list, so we can allocate memory before
	// altering *this (exception safety).
	size_type count = 0;
	for ( node_t * node_ptr = root_.next; iterator::derefable(node_ptr); node_ptr = node_ptr->next )
		++count;
	// Abort on trivial cases.
	if ( count < 2 )
		return;

	// Split *this into 1-length pieces.
	std::vector<smart_list> sorter(count); // No memory allocations after this point.
	for ( size_type i = 0; i < count; ++i )
		sorter[i].splice(&(sorter[i].root_), *(root_.next), *(root_.next));

	// At the start of each iteration, step will be the distance between lists
	// containing data. (There will be O(lg n) iterations.)
	for ( size_type step = 1; step < count; step *= 2 )
		// Merge consecutive data-bearing lists. Summing over all iterations of
		// this (inner) loop, there will be O(n) comparisons made.
		for ( size_type i = 0; i + step < count; i += 2*step )
			sorter[i].merge(sorter[i+step], p);

	// The entire (sorted) list is now in the first element of the vector.
	swap(sorter[0]);
}


/**
 * Low-level insertion.
 * The insertion will occur before @a pos. Pass &root_ to append to the list.
 */
template <class Data>
inline typename smart_list<Data>::node_t * smart_list<Data>::insert
	(node_t * const pos, const value_type & d)
{
	node_t * new_node = new node_t(d);
	// The new node is essentially a 1-node list.
	link(pos, *new_node, *new_node);
	// Return a pointer to the new node.
	return new_node;
}
/**
 * Low-level erasure.
 * The node will only be erased if its ref_count is 0. (So the caller must
 * remove their reference before calling this.)
 * @returns the next node in the list (could be a flagged node).
 */
template <class Data>
inline typename smart_list<Data>::node_t * smart_list<Data>::check_erase
	(node_t * const pos)
{
	// Sanity check.
	if ( !iterator::derefable(pos) )
		return nullptr;

	// Remember our successor.
	node_t * ret_val = pos->next;

	// Can we actually erase?
	if ( pos->ref_count == 0 ) {
		// Disconnect *pos from the list.
		unlink(*pos, *pos);
		// Now we can delete.
		delete pos;
	}

	return ret_val;
}

/**
 * Assuming the nodes from @a begin_link to @a end_link are linked, they will
 * be inserted into *this before @a pos. (Pass &root_ to append to the list.)
 * Note that *end_link is included in the insert; this is not exactly
 * analogous to a range of iterators.
 * This is a constant-time operation; reference counts are not updated.
 */
template <class Data>
inline void smart_list<Data>::link(node_t * const pos, node_t & begin_link, node_t & end_link)
{
	// Link the new nodes into the list.
	begin_link.prev = pos->prev;
	end_link.next   = pos;

	// Link the list to the new nodes.
	begin_link.prev->next = &begin_link;
	end_link.next->prev   = &end_link;
}
/**
 * Assuming @a begin_unlink and @a end_unlink are nodes from *this, with
 * @a end_unlink a successor of @a begin_unlink, that chain of nodes will
 * be removed from *this.
 * Ownership of the removed chain is transferred to the caller, who becomes
 * repsonsible for not leaving the nodes in limbo.
 * This is a constant-time operation; reference counts are not updated.
 */
template <class Data>
inline void smart_list<Data>::unlink(node_t & begin_unlink, node_t & end_unlink)
{
	// Disconnect the list from the nodes.
	begin_unlink.prev->next = end_unlink.next;
	end_unlink.next->prev = begin_unlink.prev;

	// Disconnect the nodes from the list. This leaves the nodes in limbo.
	end_unlink.next = nullptr;
	begin_unlink.prev = nullptr;
}

/**
 * Low-level splice of the chain from @a b to @a e (including b and e)
 * to the spot before @a pos.
 */
template <class Data>
inline void smart_list<Data>::splice(node_t * const pos, node_t & b, node_t & e)
{
	// Remove from the old list.
	unlink(b, e);
	// Splice into the new list.
	link(pos, b, e);
}


/**
 * Equality, if lists contain equal elements in the same order.
 */
template <class Data>
inline bool operator==(const smart_list<Data> & a, const smart_list<Data> & b)
{
	typename smart_list<Data>::const_iterator end_a = a.end();
	typename smart_list<Data>::const_iterator end_b = b.end();
	typename smart_list<Data>::const_iterator it_a = a.begin();
	typename smart_list<Data>::const_iterator it_b = b.begin();

	for ( ; it_a != end_a  &&  it_b != end_b; ++it_a, ++it_b )
		if ( *it_a != *it_b )
			// mismatch
			return false;

	// All comparisons were equal, so they are the same if we compared everything.
	return it_a == end_a  &&  it_b == end_b;
}
/**
 * Lexicographical order.
 */
template <class Data>
inline bool operator<(const smart_list<Data> & a, const smart_list<Data> & b)
{
	typename smart_list<Data>::const_iterator end_a = a.end();
	typename smart_list<Data>::const_iterator end_b = b.end();
	typename smart_list<Data>::const_iterator it_a = a.begin();
	typename smart_list<Data>::const_iterator it_b = b.begin();

	for ( ; it_a != end_a  &&  it_b != end_b; ++it_a, ++it_b )
		if ( *it_a < *it_b )
			// a is less than b.
			return true;
		else if ( *it_b < *it_a )
			// b is less than a.
			return false;

	// All comparisons were equal, so a is less than b if a is shorter.
	return it_b != end_b;
}


/* ** smart_list::node_t ** */

template <class Data>
inline smart_list<Data>::node_t::~node_t()
{
	// Some safety checks.
	if ( dat_ptr == nullptr )
		// Root node: make sure there are no lingering iterators to the list.
		assert(next == this);
	else {
		// Normal node: make sure we are not still in a list.
		assert(next == nullptr  &&  prev == nullptr);
		// Make sure no iterators point to us.
		assert(ref_count == 0);
	}

	delete dat_ptr;
}


/* ** smart_list::iterator_base ** */

/**
 * Advances our pointer to an unflagged node, possibly in the reverse direction.
 * This will advance at least one step, and will not stop on a flagged node.
 * In addition, this takes care of updating reference counts.
 * (This is the code shared by increments and decrements.)
 */
template <class Data>
template <class Value, bool Reversed>
inline void smart_list<Data>::iterator_base<Value, Reversed>::advance(bool reverse)
{
	node_t * old_ptr = ptr_;

	// Guarantee a change.
	inc(reverse);
	// Skip as necessary.
	skip_flagged(reverse);

	// Update reference counts.
	refer();
	unref(old_ptr);
}

/** Make sure we are not pointing to a flagged node. */
template <class Data>
template <class Value, bool Reversed>
inline void smart_list<Data>::iterator_base<Value, Reversed>::skip_flagged
	(bool reverse)
{
	while ( derefable()  &&  smart_list<Data>::flagged(*ptr_) )
		inc(reverse);
}

/** Add a reference to that to which we point. */
template <class Data>
template <class Value, bool Reversed>
inline void smart_list<Data>::iterator_base<Value, Reversed>::refer() const
{
	if ( derefable() )
		ptr_->ref_count += 2;
}

/**
 * Remove a reference.
 * May delete old_ptr. So call this after updating ptr_ to a new value.
 */
template <class Data>
template <class Value, bool Reversed>
inline void smart_list<Data>::iterator_base<Value, Reversed>::unref(node_t * old_ptr)
{
	if ( derefable(old_ptr) ) {
		// Remove a reference.
		old_ptr->ref_count -= 2;
		smart_list<Data>::check_erase(old_ptr);
	}
}

}// namespace util

#endif // UTILS_SMART_LIST_HPP_INCLUDED

