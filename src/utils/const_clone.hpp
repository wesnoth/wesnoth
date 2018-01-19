/*
   Copyright (C) 2012 - 2018 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "utils/type_trait_aliases.hpp"

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
	static const bool is_source_const =
		std::is_const<
			utils::remove_pointer_t<
				utils::remove_reference_t<S>
			>
		>::value;

	/** The destination type, possibly const qualified. */
	using type =
		utils::conditional_t<is_source_const, const D, D>;

	/** A reference to the destination type, possibly const qualified. */
	using reference =
		utils::conditional_t<is_source_const, const D&, D&>;

	/** A pointer to the destination type, possibly const qualified. */
	using pointer =
		utils::conditional_t<is_source_const, const D*, D*>;
};

template<typename D, typename S>
using const_clone_t = typename const_clone<D, S>::type;

template<typename D, typename S>
using const_clone_ref = typename const_clone<D, S>::reference;

template<typename D, typename S>
using const_clone_ptr = typename const_clone<D, S>::pointer;

} // namespace utils
