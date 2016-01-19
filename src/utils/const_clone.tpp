/*
   Copyright (C) 2012 - 2016 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef UTILS_CONST_CLONE_HPP_INCLUDED
#define UTILS_CONST_CLONE_HPP_INCLUDED

#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>

namespace utils {

/**
 * Helper struct to clone the constness of one type to another.
 *
 * It's function is similar to the @ref tconst_duplicator, but a bit more
 * flexible.
 *
 * @warning It seems @c *this in a const member function is not a const object,
 * use @c this, which is a pointer to a const object.
 *
 * @tparam D                      The destination type, it should have no
 *                                cv-qualifier and not be a pointer or
 *                                reference.
 * @tparam S                      The source type, this type may be a pointer
 *                                or reference and obviously is allowed to have
 *                                a cv-qualifier, although @c volatile has no
 *                                effect.
 * @tparam E                      The enable parameter for
 *                                @ref boost::enable_if.
 */
template<
	  class D
	, class S
	, typename E = void
	>
struct tconst_clone
{
	/** The destination type, possibly const qualified. */
	typedef D type;

	/** A reference to the destination type, possibly const qualified. */
	typedef D& reference;

	/** A pointer to the destination type, possibly const qualified. */
	typedef D* pointer;
};

/**
 * The specialised version of @ref tconst_clone.
 *
 * This version is used when the @p S is const-qualified.
 */
template<
	  class D
	, class S
	>
struct tconst_clone<
	  D
	, S
	, typename boost::enable_if<
		boost::is_const<
			typename boost::remove_pointer<
				typename boost::remove_reference<S>::type
				>::type
			>
		>::type
	>
{
	typedef const D type;
	typedef const D& reference;
	typedef const D* pointer;
};

} // namespace utils

#endif
