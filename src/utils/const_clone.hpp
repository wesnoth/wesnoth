/*
   Copyright (C) 2012 - 2017 by Mark de Wever <koraq@xs4all.nl>
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

#include <type_traits>

namespace utils
{
/**
 * Helper struct to clone the constness of one type to another.
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
 */
template<typename D, typename S>
struct const_clone
{
private:
	using is_source_const =
		typename std::is_const<
			typename std::remove_pointer<
				typename std::remove_reference<S>::type
			>::type
		>;

public:
	/** The destination type, possibly const qualified. */
	using type =
		typename std::conditional<is_source_const::value, const D, D>::type;

	/** A reference to the destination type, possibly const qualified. */
	using reference =
		typename std::conditional<is_source_const::value, const D&, D&>::type;

	/** A pointer to the destination type, possibly const qualified. */
	using pointer =
		typename std::conditional<is_source_const::value, const D*, D*>::type;
};

} // namespace utils

#endif
