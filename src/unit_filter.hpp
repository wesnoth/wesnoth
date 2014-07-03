/*
   Copyright (C) 2014 by Chris Beck <render787@gmail.com>
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
 * This namespace contains the function that checks if a unit matches
 * a filter. It helps by simplifying the unit object (which before now
 * holds the "match" function).
 *
 * TODO:
 * Make a class that abstracts a unit filter, assembles the constituent
 * side filters and terrain filters and conditional filters, and caches
 * these to speed up repeated application of the filter.
 */

class display_context;
class unit;
class vconfig;
struct map_location;

namespace unit_filter {

	bool matches_filter(const vconfig& cfg,const unit & u, const map_location& loc, const display_context * board, bool use_flat_tod=false);
	/// Determine if *this matches @a filter at its current location.
	/// (Only use for units currently on the map; otherwise use the overload
	/// that takes a location, possibly with a null location.)
	bool matches_filter(const vconfig& filter, const unit & u, const display_context* board, bool use_flat_tod=false);

}
