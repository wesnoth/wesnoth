/*
   Copyright (C) 2013 - 2013 by David White <dave@whitevine.net>
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

#ifndef ITERATOR_HPP_INCLUDED
#define ITERATOR_HPP_INCLUDED

#include <iterator>

namespace util {

// Forward declarations.
template <typename Value, class Container, class Deref, class Key> class iterator_extend;
template <typename Value, class Container, class Deref, class Key> class const_iterator_extend;


/// A work-around for the inability to declare a template parameter as a friend.
/// (Instead this struct is declared a friend, which effectively makes anyone
/// who has access to the template parameter a friend of iterator_extend.)
template<class Key>
struct iter_get {
	/// Returns the underlying iterator.
	template <typename Value, class Container, class Deref>
	static const typename Container::iterator & base(
		const iterator_extend<Value,Container,Deref,Key> & iter)
	{ return iter.base(); }

	/// Returns the underlying iterator.
	template <typename Value, class Container, class Deref>
	static const typename Container::const_iterator & base(
		const const_iterator_extend<Value,Container,Deref,Key> & iter)
	{ return iter.base(); }
};


/// This is a base class for extenders of iterators; it preserves most of an
/// underlying iterator, but allows dereference to be overridden.
/// Since the other operations are passed directly to the underlying iterator,
/// there is very little overhead.
///
/// To complete the iterator, derive a child class and override dereference.
/// Also suggested are a default constructor and a constructor from Iter.
/// A conversion from iterator to const_iterator might also be desired.
///
/// Value  = Intended value_type.
/// Iter   = Iterator type being adapted.
template <typename Value, class Iter>
class iterator_extend_base {
public:
	typedef Value   value_type;
	typedef Value * pointer;
	typedef Value & reference;

	typedef typename Iter::difference_type   difference_type;
	typedef typename Iter::iterator_category iterator_category;

	/// Default constructor
	iterator_extend_base() : iter_() {}
	/// Initialized constructor
	explicit iterator_extend_base(const Iter & iter) : iter_(iter) {}
	/// Virtual destructor (to support inheritance).
	virtual ~iterator_extend_base() {}


	// Overridable dereference:
	virtual reference operator*() const = 0;


	// Comparison:
	bool operator==(const iterator_extend_base & that) const { return iter_ == that.iter_; }
	bool operator!=(const iterator_extend_base & that) const { return iter_ != that.iter_; }
	// For random-access iterators:
	bool operator<(const iterator_extend_base & that)  const { return iter_ < that.iter_; }
	bool operator>(const iterator_extend_base & that)  const { return iter_ > that.iter_; }
	bool operator<=(const iterator_extend_base & that) const { return iter_ <= that.iter_; }
	bool operator>=(const iterator_extend_base & that) const { return iter_ >= that.iter_; }

	// Dereference:
	/// Member-dereference, defined in terms of the overridable operator*.
	pointer operator->() const { return &*(*this); }

	// Increment/decrement:
	iterator_extend_base & operator++() { ++iter_; return *this; }
	iterator_extend_base & operator--() { --iter_; return *this; }
	iterator_extend_base operator++(int) { return iterator_extend_base(iter_++); }
	iterator_extend_base operator--(int) { return iterator_extend_base(iter_--); }

	// Arithmetic (for random-access iterators):
	iterator_extend_base & operator+=(difference_type n) { iter_ += n; return *this; }
	iterator_extend_base & operator-=(difference_type n) { iter_ -= n; return *this; }
	iterator_extend_base operator+(difference_type n) const { return iterator_extend_base(iter_ + n); }
	iterator_extend_base operator-(difference_type n) const { return iterator_extend_base(iter_ - n); }
	difference_type operator-(const iterator_extend_base & that) const { return iter_ - that.iter_; }
	reference operator[](difference_type n) const { return *(*this + n); }

protected:
	// Protected access means only select classes can read the underlying iterator.
	const Iter & base() const { return iter_; }

private:
	/// The underlying base iterator.
	Iter iter_;
};


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
///                 static const T& eval(const Container::const_iterator &)
///             can be used for dereferencing. (The const's might seem odd, but
///             they allow the same class to be used for const_iterator_extend.)
///             A const_cast will be used to convert the return value to T&.
/// Key       = A class that unlocks the underlying iterator. (This way the
///             underlying iterator is not exposed to everyone.) If Key is
///             accessible, then the underlying iterator can be obtained via
///             	iter_get<Key>::base(const iterator_extend &)
///             (Ideally, this parameter would instead be a class to be made a
///             friend, but the original C++ standard explicitly forbids
///             declaring template parameters as friends, so a proxy is needed.)
template <typename Value, class Container, class Deref, class Key>
class iterator_extend :
	public iterator_extend_base<Value, typename Container::iterator>
{
public:
	// Handy shortcut.
	typedef typename Container::iterator  base_iter_type;

	/// Default constructor
	iterator_extend() : iterator_extend_base<Value, base_iter_type>() {}
	/// Initialized constructor
	explicit iterator_extend(const base_iter_type & iter) :
		iterator_extend_base<Value, base_iter_type>(iter)
	{}

	// Dereference:
	virtual Value & operator*() const
	{ return const_cast<Value &>(Deref::eval(this->base())); }

	// Allow access to the underlying iterator.
	friend class iter_get<Key>;
	// Friendship is needed for the definition of conversion to const_iterator.
	friend class const_iterator_extend<Value, Container, Deref, Key>;
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
///                 static const T& eval(const Container::const_iterator &)
///             can be used for dereferencing. (This same class can be used
///             for iterator_extend.)
/// Key       = A class that unlocks the underlying const_iterator. (This way
///             the underlying const_iterator is not exposed to everyone.) If
///             Key is accessible, then the underlying const_iterator can be
///             obtained via
///             	iter_get<Key>::base(const const_iterator_extend &)
///             (Ideally, this parameter would instead be a class to be made a
///             friend, but the original C++ standard explicitly forbids
///             declaring template parameters as friends, so a proxy is needed.)
template <typename Value, class Container, class Deref, class Key>
class const_iterator_extend :
	public iterator_extend_base<const Value, typename Container::const_iterator>
{
public:
	// Handy shortcut.
	typedef typename Container::const_iterator  base_iter_type;

	/// Default constructor
	const_iterator_extend() : iterator_extend_base<const Value, base_iter_type>() {}
	/// Initialized constructor
	explicit const_iterator_extend(const base_iter_type & iter) :
		iterator_extend_base<const Value, base_iter_type>(iter)
	{}
	/// Conversion from iterator_extend (same parameters).
	const_iterator_extend(const iterator_extend<Value, Container, Deref, Key> & iter) :
		iterator_extend_base<const Value, base_iter_type>(iter.base())
	{}

	// Dereference:
	virtual const Value & operator*() const
	{ return  Deref::eval(this->base()); }

	// Allow access to the underlying iterator.
	friend class iter_get<Key>;
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
		data_.push_back(NULL); // (This two-step approach is for exception safety.)
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
			delete *util::iter_get<key>::base(it); // Accessing the underlying iterator.
	}
};

*/

#endif // ITERATOR_HPP_INCLUDED
