/*
   Copyright (C) 2011 - 2015 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_AUXILIARY_ITERATOR_POLICY_VISIT_HPP_INCLUDED
#define GUI_WIDGETS_AUXILIARY_ITERATOR_POLICY_VISIT_HPP_INCLUDED

#include "gui/auxiliary/iterator/walker.hpp"

#include <cstring>

namespace gui2
{

namespace iterator
{

namespace policy
{

namespace visit
{

/**
 * This policy skips the current level.
 */
class tskip
{
public:
	/**
	 * Acts like @ref twalker_::next for the level where the policy is used.
	 */
	twalker_::tstate next(twalker_&)
	{
		return twalker_::fail;
	}

	/**
	 * Acts like @ref twalker_::at_end for the level where the policy is used.
	 */
	bool at_end(const twalker_&) const
	{
		return true;
	}

	/**
	 * Acts like @ref twalker_::get for the level where the policy is used.
	 */
	gui2::twidget* get(twalker_&)
	{
		return NULL;
	}
};

/**
 * This policy tries to visit the current level.
 *
 * @tparam level                  The level to visit.
 */
template <twalker_::tlevel level>
class tvisit
{
public:
	/**
	 * Acts like @ref twalker_::next for the level where the policy is used.
	 */
	twalker_::tstate next(twalker_& visitor)
	{
		return visitor.next(level);
	}

	/**
	 * Acts like @ref twalker_::at_end for the level where the policy is used.
	 */
	bool at_end(const twalker_& visitor) const
	{
		return visitor.at_end(level);
	}

	/**
	 * Acts like @ref twalker_::get for the level where the policy is used.
	 */
	gui2::twidget* get(twalker_& visitor)
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
template <bool, twalker_::tlevel level>
class tvisit
{
};

/** Specialized to select the @ref visit::tskip policy. */
template <twalker_::tlevel level>
class tvisit<false, level> : public visit::tskip
{
};

/** Specialized to select the @ref visit::tvisit policy. */
template <twalker_::tlevel level>
class tvisit<true, level> : public visit::tvisit<level>
{
};

} // namespace policy

} // namespace iterator

} // namespace gui2

#endif
