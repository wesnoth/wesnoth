/* $Id$ */
/*
   Copyright (C) 2012 by Mark de Wever <koraq@xs4all.nl>
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
 * @file
 * Define the common filters for the @ref gui2::tpane class.
 */

#ifndef GUI_AUXILIARY_FILTER_HPP_INCLUDED
#define GUI_AUXILIARY_FILTER_HPP_INCLUDED

#include "map_utils.hpp"
#include "util.hpp"

namespace gui2 {

template <class T>
inline bool sort(
		  const tpane::titem& lhs
		, const tpane::titem& rhs
		, const std::string& tag
		, const bool ascending)
{
	if(ascending) {
		return
			  lexical_cast<T>(at(lhs.tags, tag))
			< lexical_cast<T>(at(rhs.tags, tag));
	} else {
		return
			  lexical_cast<T>(at(lhs.tags, tag))
			> lexical_cast<T>(at(rhs.tags, tag));
	}
}

} // namespace gui2

#endif
