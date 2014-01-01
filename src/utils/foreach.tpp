/*
   Copyright (C) 2013 - 2014 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef UTILS_FOREACH_TPP_INCLUDED
#define UTILS_FOREACH_TPP_INCLUDED

/**
 * @file
 * Contains the emulation for the C++11 ranged for loop with auto:
 * @code for(auto foo : bar) @endcode.
 * The code is based upon Boost bug report [1] and uses BOOST_FOREACH and
 * BOOST_TYPEOF internally.
 *
 * [1] https://svn.boost.org/trac/boost/ticket/3643
 */

#include <boost/foreach.hpp>
#include <boost/typeof/typeof.hpp>

/**
 * The ranged based for emulation macro.
 *
 * For example iterating over a std:vector<int> named vec turns into:
 * @code
FOREACH(value, vec) {
    ...
}
@endcode
 * This iterates over the copy of a the value, using a reference looks like:
 * @code
FOREACH( & value, vec) {
    ...
}
@endcode
 * Which looks somewhat ugly. To 'fix' the problem the dummy macro @ref AUTO
 * is defined, allowing to use the following syntax:
 * @code
FOREACH(AUTO& value, vec) {
    ...
}
@endcode
 * The @ref AUTO could also have been used in the first example. For
 * readability it might be a good idea to always use the @ref AUTO macro.
 * That way the sytax also looks more like the real C++11 code.
 */
#define FOREACH(VAR, RANGE) \
	BOOST_FOREACH(BOOST_TYPEOF(*boost::begin(RANGE)) VAR, RANGE)

/**
 * Dummy macro.
 *
 * See @ref FOREACH for more info.
 */
#define AUTO

#endif
