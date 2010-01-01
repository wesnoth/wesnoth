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

#define GETTEXT_DOMAIN "wesnoth-test"

#include <boost/test/unit_test.hpp>

#include "copy_policy.hpp"


/** Set to 1 if you want debug output to std::cerr. */
#define TEST_POLICY_DEBUG 0

#if TEST_POLICY_DEBUG
#include <iostream>
#endif

BOOST_AUTO_TEST_SUITE( policy )


namespace intrusive {

/** Class to test the copy policies. */
template <template<class> class copy_policy >
struct ttest : public copy_policy<ttest<copy_policy> >
{
	typedef copy_policy<ttest<copy_policy> > policy;
	/*
	 * This typedef first was
	 * typedef typename ttest<copy_policy>::rhs_type rhs_type;
	 *
	 * Unfortunately MSVC 2008 chokes on it and aborts with an internal
	 * compiler error. So used another name for the type.
	 */
	typedef typename ttest<copy_policy>::rhs_type ttest_rhs_type;

	ttest()
		: policy()
		, copied_constructed_(false)
		, assigned_(false)
		, cloned_(false)
		, invalidated_(false)
	{
#if TEST_POLICY_DEBUG
		std::cerr << "Default constructor " << __func__ << ".\n";
#endif
	}

	ttest(ttest_rhs_type rhs)
		: policy(rhs)
		, copied_constructed_(true)
		, assigned_(rhs.assigned_)
		, cloned_(rhs.cloned_)
		, invalidated_(rhs.invalidated_)
	{
#if TEST_POLICY_DEBUG
		std::cerr << "Copy constructor " << __func__ << ".\n";
#endif
		copy(rhs);
	}

	ttest& operator=(ttest_rhs_type rhs)
	{
#if TEST_POLICY_DEBUG
		std::cerr << __func__ << ".\n";
#endif
		copied_constructed_ = rhs.copied_constructed_;
		assigned_ = true;
		cloned_ = rhs.cloned_;
		invalidated_ = rhs.invalidated_;

		copy(rhs);

		return *this;
	}

	/** Mandatory helper for the tmove_copy policy. */
	void invalidate() { invalidated_ = true; }

	/** Mandatory helper for the tdeep_copy policy. */
	void clone() { cloned_ = true; }

	/** A group helper variables  */
	bool copied_constructed_
		, assigned_
		, cloned_
		, invalidated_
		;
};

// Not really required but doesn't hurt.
#if TEST_POLICY_DEBUG
template<template<class >class T>
std::ostream& operator<<(std::ostream &s, const ttest<T>& test)
{
	s << "copied_constructed_ " << test.copied_constructed_
		<< " assigned_ " << test.assigned_
		<< " cloned_ " << test.cloned_
		<< " invalidated_ " << test.invalidated_
		;

	return s;
}
#endif

/** Tests the copy constructor of the policy. */
template<template<class >class T>
void copy_test(const bool orig_invalidated, const bool copy_cloned)
{
#if TEST_POLICY_DEBUG
	std::cerr << __func__ << ".\n";
#endif

	ttest<T> orig;
	ttest<T> cpy(orig);

#if TEST_POLICY_DEBUG
	std::cerr << "orig " << orig
		<< "\ncopy " << cpy
		<< ".\n";
#endif

	BOOST_REQUIRE_EQUAL(orig.copied_constructed_, false);
	BOOST_REQUIRE_EQUAL(orig.assigned_, false);
	BOOST_REQUIRE_EQUAL(orig.cloned_, false);
	BOOST_REQUIRE_EQUAL(orig.invalidated_, orig_invalidated);

	BOOST_REQUIRE_EQUAL(cpy.copied_constructed_, true);
	BOOST_REQUIRE_EQUAL(cpy.assigned_, false);
	BOOST_REQUIRE_EQUAL(cpy.cloned_, copy_cloned);
	BOOST_REQUIRE_EQUAL(cpy.invalidated_, false);
}

/** Tests the assignment operator of the policy. */
template<template<class >class T>
void assign_test(const bool orig_invalidated, const bool copy_cloned)
{
#if TEST_POLICY_DEBUG
	std::cerr << __func__ << ".\n";
#endif
	ttest<T> orig;
	ttest<T> cpy;

	cpy = orig;

#if TEST_POLICY_DEBUG
	std::cerr << "orig " << orig
		<< "\ncopy " << cpy
		<< ".\n";
#endif

	BOOST_REQUIRE_EQUAL(orig.copied_constructed_, false);
	BOOST_REQUIRE_EQUAL(orig.assigned_, false);
	BOOST_REQUIRE_EQUAL(orig.cloned_, false);
	BOOST_REQUIRE_EQUAL(orig.invalidated_, orig_invalidated);

	BOOST_REQUIRE_EQUAL(cpy.copied_constructed_, false);
	BOOST_REQUIRE_EQUAL(cpy.assigned_, true);
	BOOST_REQUIRE_EQUAL(cpy.cloned_, copy_cloned);
	BOOST_REQUIRE_EQUAL(cpy.invalidated_, false);
}

/**
 * Tests a policy.
 *
 * @param orig_invalidated        Should the original object be invalidated when
 *                                used as rhs in an assignment or as parameter
 *                                in a copy constructor.
 * @param copy_cloned             Should the copy be cloned when use as lhs in
 *                                an assignment or when being copy constructed.
 */
template<template<class >class T>
void test(const bool orig_invalidated, const bool copy_cloned)
{
	copy_test<T>(orig_invalidated, copy_cloned);
	assign_test<T>(orig_invalidated, copy_cloned);
}

} // namespace intrusive

namespace non_intrusive {

struct ttest
{
	ttest()
		: cloned_(false)
		, invalidated_(false)
	{
#if TEST_POLICY_DEBUG
		std::cerr << "Default constructor " << __func__ << ".\n";
#endif
	}

#if TEST_POLICY_DEBUG

	ttest(const ttest& rhs)
		: cloned_(rhs.cloned_)
		, invalidated_(rhs.invalidated_)
	{
		std::cerr << "Copy constructor " << __func__ << ".\n";
	}

	ttest& operator=(const ttest& rhs)
	{
		std::cerr << "Assign operator " << __func__ << ".\n";

		cloned_ = rhs.cloned_;
		invalidated_ = rhs.invalidated_;

		return *this;
	}
#endif

	/** Mandatory helper for the tmove_copy policy. */
	void invalidate() { invalidated_ = true; }

	/** Mandatory helper for the tdeep_copy policy. */
	void clone() { cloned_ = true; }

	/** A group helper variables  */
	bool cloned_, invalidated_;
};

// Not really required but doesn't hurt.
#if TEST_POLICY_DEBUG
static std::ostream& operator<<(std::ostream &s, const ttest& test)
{
	s << "cloned_ " << test.cloned_
		<< " invalidated_ " << test.invalidated_
		;

	return s;
}
#endif

/** Tests the copy constructor of the policy. */
template<class T, template<class >class U>
void copy_test(const bool orig_invalidated, const bool copy_cloned)
{
#if TEST_POLICY_DEBUG
	std::cerr << __func__ << ".\n";
#endif

	policies::tcopy_policy<T, U> orig;
	policies::tcopy_policy<T, U> cpy(orig);

#if TEST_POLICY_DEBUG
	std::cerr << "orig " << orig
		<< "\ncopy " << cpy
		<< ".\n";
#endif

	BOOST_REQUIRE_EQUAL(orig.cloned_, false);
	BOOST_REQUIRE_EQUAL(orig.invalidated_, orig_invalidated);

	BOOST_REQUIRE_EQUAL(cpy.cloned_, copy_cloned);
	BOOST_REQUIRE_EQUAL(cpy.invalidated_, false);
}

/** Tests the assignment operator of the policy. */
template<class T, template<class >class U>
void assign_test(const bool orig_invalidated, const bool copy_cloned)
{
#if TEST_POLICY_DEBUG
	std::cerr << __func__ << ".\n";
#endif
	policies::tcopy_policy<T, U> orig;
	policies::tcopy_policy<T, U> cpy;

	cpy = orig;

#if TEST_POLICY_DEBUG
	std::cerr << "orig " << orig
		<< "\ncopy " << cpy
		<< ".\n";
#endif

	BOOST_REQUIRE_EQUAL(orig.cloned_, false);
	BOOST_REQUIRE_EQUAL(orig.invalidated_, orig_invalidated);

	BOOST_REQUIRE_EQUAL(cpy.cloned_, copy_cloned);
	BOOST_REQUIRE_EQUAL(cpy.invalidated_, false);
}

/**
 * Tests a policy.
 *
 * @param orig_invalidated        Should the original object be invalidated when
 *                                used as rhs in an assignment or as parameter
 *                                in a copy constructor.
 * @param copy_cloned             Should the copy be cloned when use as lhs in
 *                                an assignment or when being copy constructed.
 */
template<class T, template<class >class U>
void test(const bool orig_invalidated, const bool copy_cloned)
{
	copy_test<T, U>(orig_invalidated, copy_cloned);
	assign_test<T, U>(orig_invalidated, copy_cloned);
}

} // namespace non_intrusive

BOOST_AUTO_TEST_CASE( test_copy_policy )
{
/*
 * The no copy policy shouldn't compile so it's commented out. The first part
 * enables the basics and can also test whether the compiler does a RVO [1],
 * if the compiler does the copy1 can be constructed. The second part should
 * always fail. (The non intrusive version is not added here.)
 *
 * [1] http://www.parashift.com/c++-faq-lite/ctors.html#faq-10.9
 */
#if 0
	intrusive::ttest<policies::tno_copy> orig;

	// Might or might not compile.
	intrusive::ttest<policies::tno_copy> copy1(ttest<policies::tno_copy>());
#if 0
	// Must fail to compile.
	intrusive::ttest<policies::tno_copy> copy2(orig);

	// Must fail to compile.
	orig = orig;
#endif
#endif

#if TEST_POLICY_DEBUG
	std::cerr << std::boolalpha;
#endif

	/* Intrusive tests */

#if TEST_POLICY_DEBUG
	std::cerr << "Test intrusive shallow copy\n";
#endif
	intrusive::test<policies::tshallow_copy>(false, false);

#if TEST_POLICY_DEBUG
	std::cerr << "\n\nTest intrusive deep copy\n";
#endif
	intrusive::test<policies::tdeep_copy>(false, true);

#if TEST_POLICY_DEBUG
	std::cerr << "\n\nTest intrusive move copy\n";
#endif
	intrusive::test<policies::tmove_copy>(true, false);

	/* Non-intrusive tests */

#if TEST_POLICY_DEBUG
	std::cerr << "Test non-intrusive shallow copy\n";
#endif
	non_intrusive::test<non_intrusive::ttest, policies::tshallow_copy>(false, false);

#if TEST_POLICY_DEBUG
	std::cerr << "\n\nTest non-intrusive deep copy\n";
#endif
	non_intrusive::test<non_intrusive::ttest, policies::tdeep_copy>(false, true);

#if TEST_POLICY_DEBUG
	std::cerr << "\n\nTest non-intrusive move copy\n";
#endif
	non_intrusive::test<non_intrusive::ttest, policies::tmove_copy>(true, false);

}

/* vim: set ts=4 sw=4: */
BOOST_AUTO_TEST_SUITE_END()
