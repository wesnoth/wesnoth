/*
   Copyright (C) 2011 - 2017 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/auxiliary/iterator/walker.hpp"

#include <cstring>

namespace gui2
{

namespace iteration
{

namespace policy
{

namespace visit
{

/**
 * This policy skips the current level.
 */
class skip_level
{
public:
	/**
	 * Acts like @ref walker_base::next for the level where the policy is used.
	 */
	walker_base::state_t next(walker_base&)
	{
		return walker_base::fail;
	}

	/**
	 * Acts like @ref walker_base::at_end for the level where the policy is used.
	 */
	bool at_end(const walker_base&) const
	{
		return true;
	}

	/**
	 * Acts like @ref walker_base::get for the level where the policy is used.
	 */
	gui2::widget* get(walker_base&)
	{
		return nullptr;
	}
};

/**
 * This policy tries to visit the current level.
 *
 * @tparam level                  The level to visit.
 */
template <walker_base::level level>
class visit_level
{
public:
	/**
	 * Acts like @ref walker_base::next for the level where the policy is used.
	 */
	walker_base::state_t next(walker_base& visitor)
	{
		return visitor.next(level);
	}

	/**
	 * Acts like @ref walker_base::at_end for the level where the policy is used.
	 */
	bool at_end(const walker_base& visitor) const
	{
		return visitor.at_end(level);
	}

	/**
	 * Acts like @ref walker_base::get for the level where the policy is used.
	 */
	gui2::widget* get(walker_base& visitor)
	{
		return visitor.get(level);
	}
};

} // namespace visit

/**
 * Helper class to select to visit or skip a level.
 *
 * @tparam level                  The level to determine the policy for.
 */
template <bool, walker_base::level level>
class visit_level
{
};

/** Specialized to select the @ref visit::skip_level policy. */
template <walker_base::level level>
class visit_level<false, level> : public visit::skip_level
{
};

/** Specialized to select the @ref visit::visit_level policy. */
template <walker_base::level level>
class visit_level<true, level> : public visit::visit_level<level>
{
};

} // namespace policy

} // namespace iteration

} // namespace gui2
