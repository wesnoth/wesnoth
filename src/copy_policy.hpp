/* $Id$ */
/*
   Copyright (C) 2008 - 2010 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Defines the copy policies for classes.
 *
 * When a class can have multiple copy policies these templates can be used.
 * When a class uses this class it should define its own copy constructor to do
 * a shallow copy and call copy() afterwards. Also it should define a assignment
 * operator which does a shallow copy and then call copy() (copy() test for self
 * assignment, but the assignment operator can also do the test.
 *
 * Another option is to use the tcopy_policy class, which uses the default
 * constructor, copy constructor and assignment operator of the class. This way
 * it can easily be added to an existing class without changes.
 *
 * See tests/test_policy.cpp for an example implementation of this policy, both
 * the intrusive and non-intrusive version.
 */

#ifndef COPY_POLICY_HPP_INCLUDED
#define COPY_POLICY_HPP_INCLUDED

#include <boost/static_assert.hpp>

/** Set to 1 if you want debug output to std::cerr. */
#define COPY_POLICY_DEBUG 0

#if COPY_POLICY_DEBUG
#include <iostream>
#endif

/** Contains various policies for policy based designs. */
namespace policies {

/** Utilities for the policies. */
namespace utils {

/** Gets the reference version of type T. */
template <class T>
class treference_type
{
	template<class U>
	struct thelper
	{
		typedef U& type;
	};
	template<class U>
	struct thelper<U&>
	{
		typedef U type;
	};
public:
	typedef typename thelper<T>::type type;
};

} // namespace utils

/**
 * Allow no copies.
 *
 * When a class never should be copyable boost::noncopyable is a better
 * alternative (or just do it manually).
 */
template<class T>
class tno_copy
{
public:
	/** The type to use in the copy constructor and assignment operator. */
	typedef typename utils::treference_type<const T>::type rhs_type;

	void copy(rhs_type /*rhs*/)
	{
		BOOST_STATIC_ASSERT(sizeof(T) == 0);
	}
};

/**
 * Makes a shallow copy.
 *
 * Since the subclass already does the shallow part we do nothing.
 */
template<class T>
class tshallow_copy
{
public:
	/** The type to use in the copy constructor and assignment operator. */
	typedef typename utils::treference_type<const T>::type rhs_type;

	void copy(rhs_type /*rhs*/)
	{
#if COPY_POLICY_DEBUG
		std::cerr << __func__ << ".\n";
#endif
	}
};

/**
 * Makes a deep copy.
 *
 * The subclass must define a function void clone() which gets called to do
 * the copying.
 */
template<class T>
class tdeep_copy
{
public:
	/** The type to use in the copy constructor and assignment operator. */
	typedef typename utils::treference_type<const T>::type rhs_type;

	void copy(rhs_type rhs)
	{
#if COPY_POLICY_DEBUG
		std::cerr << __func__ << ".\n";
#endif
		if(&rhs != this) {
			static_cast<typename utils::treference_type<T>::type>(*this).clone();
		}
	}
};

/**
 * Makes a move copy.
 *
 * The shared resources are moved from the original class to the copy and
 * thus the original object no longer owns them. The subclass must define a
 * function void dispose() which should clear the resources.
 */
template<class T>
class tmove_copy
{
public:
	/** The type to use in the copy constructor and assignment operator. */
	typedef typename utils::treference_type<T>::type rhs_type;

	void copy(rhs_type rhs)
	{
#if COPY_POLICY_DEBUG
		std::cerr << __func__ << ".\n";
#endif
		if(&rhs != this) {
			rhs.invalidate();
		}
	}
};

/**
 * Helper class to add a policy to an existing class.
 *
 * This
 */
template <
	class base,
	template<class> class copy_policy
>
struct tcopy_policy : public base, public copy_policy<tcopy_policy<base, copy_policy> >
{
	typedef copy_policy<tcopy_policy<base, copy_policy> > policy;
	/*
	 * This typedef first was
	 * typedef typename tcopy_policy<base, copy_policy>::rhs_type rhs_type;
	 *
	 * Unfortunately MSVC 2008 chokes on it and aborts with an internal
	 * compiler error. So used another name for the type.
	 */
	typedef typename tcopy_policy<base, copy_policy>::rhs_type
		tcopy_policy_rhs_type;

	tcopy_policy()
		: base()
		, policy()
	{
#if COPY_POLICY_DEBUG
		std::cerr << "tcopy_policy: default constructor.\n";
#endif
	}

	tcopy_policy(tcopy_policy_rhs_type rhs)
		: base(rhs)
		, policy(rhs)
	{
#if COPY_POLICY_DEBUG
		std::cerr << "tcopy_policy: copy constructor.\n";
#endif
		copy(rhs);
	}

	tcopy_policy& operator=(tcopy_policy_rhs_type rhs)
	{
#if COPY_POLICY_DEBUG
		std::cerr << "tcopy_policy: assignment operator.\n";
#endif
		static_cast<base>(*this) = rhs;
		static_cast<policy>(*this) = rhs;

		copy(rhs);

		return *this;
	}
};

} // namespace policy

#endif
