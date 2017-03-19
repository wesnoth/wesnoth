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
 * @file Generic iterator definitions, to take some mess out of other headers.
 */

#ifndef UTILS_ITERATOR_HPP_INCLUDED
#define UTILS_ITERATOR_HPP_INCLUDED

#include <iterator>

namespace util {

// Forward declaration.
template<typename Value, class Container, class Deref, class Key> class const_iterator_extend;

/// This is an iterator class that extends an existing iterator by overriding
/// dereference. Access to the underlying iterator is controlled, promoting
/// a black-box approach.
///
/// The expected use of this class is as a typedef.
/// If used in conjunction with a typedef of const_iterator_extend (with the
/// same template parameters), you get conversion to your const iterator.
///
/// Value     = Intended value_type (result of dereferencing).
/// Container = The container whose iterator is being extended.
/// Deref     = A class whose member
///                 static const Value& eval(const Container::const_iterator &)
///             can be used for dereferencing. (The const's might seem odd, but
///             they allow the same class to be used for const_iterator_extend.)
///             A const_cast will be used to convert the return value to Value&.
/// Key       = A class that unlocks the underlying iterator. (This way the
///             underlying iterator is not exposed to everyone.) If Key is
///             accessible, then the underlying iterator can be obtained via
///             get(const Key &).
template <typename Value, class Container, class Deref, class Key>
class iterator_extend
{
public:
	// Handy shortcut.
	typedef typename Container::iterator  base_iter_type;

	// Types required of an iterator:
	typedef Value        value_type;
	typedef value_type * pointer;
	typedef value_type & reference;
	typedef typename base_iter_type::difference_type   difference_type;
	typedef typename base_iter_type::iterator_category iterator_category;

	/// Default constructor
	iterator_extend() : iter_() {}
	/// Initialized constructor
	explicit iterator_extend(const base_iter_type & iter) : iter_(iter) {}


	// Comparison:
	bool operator==(const iterator_extend & that) const { return iter_ == that.iter_; }
	bool operator!=(const iterator_extend & that) const { return iter_ != that.iter_; }
	// For random-access iterators:
	bool operator<(const iterator_extend & that)  const { return iter_ < that.iter_; }
	bool operator>(const iterator_extend & that)  const { return iter_ > that.iter_; }
	bool operator<=(const iterator_extend & that) const { return iter_ <= that.iter_; }
	bool operator>=(const iterator_extend & that) const { return iter_ >= that.iter_; }

	// Dereference:
	reference operator*() const { return const_cast<Value &>(Deref::eval(iter_)); }
	pointer  operator->() const { return &*(*this); }

	// Increment/decrement:
	iterator_extend & operator++() { ++iter_; return *this; }
	iterator_extend & operator--() { --iter_; return *this; }
	iterator_extend operator++(int) { return iterator_extend(iter_++); }
	iterator_extend operator--(int) { return iterator_extend(iter_--); }
	// Arithmetic (for random-access iterators):
	iterator_extend & operator+=(difference_type n) { iter_ += n; return *this; }
	iterator_extend & operator-=(difference_type n) { iter_ -= n; return *this; }
	iterator_extend operator+(difference_type n) const { return iterator_extend(iter_ + n); }
	iterator_extend operator-(difference_type n) const { return iterator_extend(iter_ - n); }
	difference_type operator-(const iterator_extend & that) const { return iter_ - that.iter_; }
	difference_type operator-(const const_iterator_extend<Value, Container, Deref, Key> & that) const
	{ return -(that - const_iterator_extend<Value, Container, Deref, Key>(*this)); }
	reference operator[](difference_type n) const { return *(*this + n); }

	// Allow access to the underlying iterator to those with the key.
	const base_iter_type & get(const Key &) const { return iter_; }

private:
	/// The underlying base iterator.
	base_iter_type iter_;
};


/// This is a const_iterator class that extends an existing const_iterator by
/// overriding dereference. Access to the underlying iterator is controlled,
/// promoting a black-box approach.
///
/// The expected use of this class is as a typedef.
/// If used in conjunction with a typedef of iterator_extend (with the same
/// template parameters), you get conversion from your regular iterator.
///
/// Value     = Intended value_type, minus "const".
/// Container = The container whose const_iterator is being extended.
/// Deref     = A class whose member
///                 static const Value& eval(const Container::const_iterator &)
///             can be used for dereferencing. (This same class can be used
///             for iterator_extend.)
/// Key       = A class that unlocks the underlying const_iterator. (This way
///             the underlying const_iterator is not exposed to everyone.) If
///             Key is accessible, then the underlying const_iterator can be
///             obtained via get(const Key &).
template <typename Value, class Container, class Deref, class Key>
class const_iterator_extend
{
public:
	// Handy shortcut.
	typedef typename Container::const_iterator  base_iter_type;

	// Types required of an iterator:
	typedef const Value  value_type;
	typedef value_type * pointer;
	typedef value_type & reference;
	typedef typename base_iter_type::difference_type   difference_type;
	typedef typename base_iter_type::iterator_category iterator_category;

	/// Default constructor
	const_iterator_extend() : iter_() {}
	/// Initialized constructor
	explicit const_iterator_extend(const base_iter_type & iter) : iter_(iter) {}
	/// Conversion from iterator_extend (same parameters).
	const_iterator_extend(const iterator_extend<Value, Container, Deref, Key> & iter) :
		iter_(iter.get(Key()))
	{}

	// Comparison:
	bool operator==(const const_iterator_extend & that) const { return iter_ == that.iter_; }
	bool operator!=(const const_iterator_extend & that) const { return iter_ != that.iter_; }
	// For random-access iterators:
	bool operator<(const const_iterator_extend & that)  const { return iter_ < that.iter_; }
	bool operator>(const const_iterator_extend & that)  const { return iter_ > that.iter_; }
	bool operator<=(const const_iterator_extend & that) const { return iter_ <= that.iter_; }
	bool operator>=(const const_iterator_extend & that) const { return iter_ >= that.iter_; }

	// Dereference:
	reference operator*() const { return Deref::eval(iter_); }
	pointer  operator->() const { return &*(*this); }

	// Increment/decrement:
	const_iterator_extend & operator++() { ++iter_; return *this; }
	const_iterator_extend & operator--() { --iter_; return *this; }
	const_iterator_extend operator++(int) { return const_iterator_extend(iter_++); }
	const_iterator_extend operator--(int) { return const_iterator_extend(iter_--); }
	// Arithmetic (for random-access iterators):
	const_iterator_extend & operator+=(difference_type n) { iter_ += n; return *this; }
	const_iterator_extend & operator-=(difference_type n) { iter_ -= n; return *this; }
	const_iterator_extend operator+(difference_type n) const { return const_iterator_extend(iter_ + n); }
	const_iterator_extend operator-(difference_type n) const { return const_iterator_extend(iter_ - n); }
	difference_type operator-(const const_iterator_extend & that) const { return iter_ - that.iter_; }
	reference operator[](difference_type n) const { return *(*this + n); }

	// Allow access to the underlying iterator to those with the key.
	const base_iter_type & get(const Key &) const { return iter_; }

private:
	/// The underlying base iterator.
	base_iter_type iter_;
};

}// namespace util


/*
 * An example of how to use these iterators:

#include <vector>

class int_vect {
	// We will store a vector of pointers to int.
	typedef std::vector<int *> pint_vector;
	pint_vector data_;

	// We need a struct to control access to underlying iterators.
	// Also, it is convenient to put evaluation in a private struct.
	struct key {
		// Here we define the conversion from a pint_vector iterator to int.
		static const int& eval(const pint_vector::const_iterator & iter)
		{ return **iter; }
	};

public:
	// To the public, we will look like a container of ints.
	typedef util::iterator_extend      <int, pint_vector, key, key> iterator;
	typedef util::const_iterator_extend<int, pint_vector, key, key> const_iterator;
	// This gives us iterators that dereference to _int_, but otherwise
	// behave like iterators of _pint_vector_. The first _key_ defines
	// how to dereference, while the second _key_ prevents others from
	// accessing underlying iterators (because key is private).

public:
	// That's the basic definition. Let's move on to some uses, in the guise
	// of giving others access to our data. But first, so this class is not
	// completely useless, maybe we should be able to add data to it.
	void push_back(int i)
	{
		data_.push_back(nullptr); // (This two-step approach is for exception safety.)
		data_.back() = new int(i);
	}

	// Now we give read-write access via iterators.
	iterator begin() { return iterator(data_.begin()); } // Simple creation of iterators.
	iterator end()   { return iterator(data_.end()); }
	// We won't bother with the const versions for now.

	// Almost done. To avoid leaking memory, we'll need a destructor.
	~int_vect()
	{
		// Free all the memory we allocated.
		for ( iterator it = begin(); it != end(); ++it )
			delete *it.get(key()); // Accessing the underlying iterator.
	}
};

*/

#endif // UTILS_ITERATOR_HPP_INCLUDED
