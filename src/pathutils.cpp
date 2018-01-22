/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
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
 * Various pathfinding functions and utilities.
 */

#include "pathutils.hpp"

#include "map/map.hpp"


/**
 * Function that will add to @a result all locations exactly @a radius tiles
 * from @a center (or nothing if @a radius is not positive). @a result must be
 * a std::vector of locations.
 */
void get_tile_ring(const map_location& center, const int radius,
                   std::vector<map_location>& result)
{
	if ( radius <= 0 ) {
		return;
	}

	map_location loc = center.get_direction(map_location::SOUTH_WEST, radius);

	for(int n = 0; n != 6; ++n) {
		const map_location::DIRECTION dir = static_cast<map_location::DIRECTION>(n);
		for(int i = 0; i != radius; ++i) {
			result.push_back(loc);
			loc = loc.get_direction(dir, 1);
		}
	}
}


/**
 * Function that will add to @a result all locations within @a radius tiles
 * of @a center (excluding @a center itself). @a result must be a std::vector
 * of locations.
 */
void get_tiles_in_radius(const map_location& center, const int radius,
                         std::vector<map_location>& result)
{
	for(int n = 1; n <= radius; ++n) {
		get_tile_ring(center, n, result);
	}
}


/**
 * Function that will add to @a result all locations within @a radius tiles
 * of @a center (including @a center itself). @a result must be a std::set
 * of locations.
 */
void get_tiles_radius(const map_location& center, size_t radius,
                      std::set<map_location>& result)
{
	// Re-use some logic.
	std::vector<map_location> internal_result(1, center);
	get_tiles_in_radius(center, static_cast<int>(radius), internal_result);

	// Convert to a set.
	result.insert(internal_result.begin(), internal_result.end());
}


namespace { // Helpers for get_tiles_radius() without a radius filter.

	// Ranges of rows are stored as pairs of a row number and a number of rows.
	typedef std::pair<int, size_t> row_range;
	// This is a map from column numbers to sets of ranges of rows.
	typedef std::map<int, std::set<row_range> > column_ranges;


	/**
	 * Function that will collect all locations within @a radius tiles of an
	 * element of @a locs, subject to the restriction col_begin <= x < col_end.
	 */
	// Complexity: O(nr lg(nr)), where n = locs.size() and r = radius.
	// In this formula, r is bound by col_end-col_begin (but that is
	// probably a rare event).
	void get_column_ranges(column_ranges & collected_tiles,
	                       const std::vector<map_location>& locs,
	                       const size_t radius,
	                       const int col_begin, const int col_end)
	{
		// Shorter names for the directions we'll use.
		const map_location::DIRECTION NORTH_WEST = map_location::NORTH_WEST;
		const map_location::DIRECTION NORTH_EAST = map_location::NORTH_EAST;
		const map_location::DIRECTION SOUTH_EAST = map_location::SOUTH_EAST;

		// Perform this conversion once.
		const int radius_i = static_cast<int>(radius);

		for (const map_location &loc : locs)
			if ( loc != map_location::null_location() )
			{
				// Calculate the circle of hexes around this one.
				size_t height = radius;
				map_location top = loc.get_direction(NORTH_WEST, radius_i);
				// Don't start off the map edge.
				if ( top.x < col_begin ) {
					const int col_shift = std::min(col_begin, loc.x) - top.x;
					top = top.get_direction(NORTH_EAST, col_shift);
					height += col_shift;
				}
				// The left side.
				const int end_l = std::min(loc.x, col_end);
				for ( ; top.x < end_l; top = top.get_direction(NORTH_EAST, 1) )
					collected_tiles[top.x].insert(row_range(top.y, ++height));
				// Extra increment so the middle column is tall enough.
				height += 2;
				// Don't start off the map edge (we allow loc to be off-board).
				if ( top.x < col_begin ) {
					const int col_shift = col_begin - top.x;
					top = top.get_direction(SOUTH_EAST, col_shift);
					height -= col_shift;
				}
				// The middle column and right side.
				const int end_r = std::min(loc.x + radius_i + 1, col_end);
				for ( ; top.x < end_r; top = top.get_direction(SOUTH_EAST, 1) )
					collected_tiles[top.x].insert(row_range(top.y, --height));
			}
	}

	/**
	 * Function that interprets @a collected_tiles and adds to @a result those
	 * whose y-coordinate satisifies row_begin <= y < row_end.
	 * When passed to this function, @a result must not be empty. (This allows
	 * a code simplification and is currently always the case anyway.)
	 */
	// Complexity: O(number of distinct hexes collected), assuming that the
	// insertion hint makes insertions O(1). Furthermore, hexes outside the
	// interval [row_begin, row_end) are skipped and do not count towards the
	// complexity.
	void ranges_to_tiles(std::set<map_location> & result,
	                     const column_ranges & collected_tiles,
	                     int row_begin, int row_end)
	{
		// This should help optimize the insertions (since we will be
		// processing hexes in their lexicographical order).
		std::set<map_location>::const_iterator insert_hint = result.begin();
		// Note: This hint will get incremented later, which is the only
		// reason we require result to be initially non-empty.

		for (const column_ranges::value_type & column : collected_tiles)
		{
			// For this loop, the order within the set is crucial; we need
			// rows.first to be non-decreasing with each iteration.
			// Loop invariant: within this column, all rows before next_row
			// have been processed and either added to result or skipped.
			// There is no going back (nor a need to).
			int next_row = row_begin;
			for (const row_range &rows : column.second)
			{
				// Skipping some rows?
				if ( next_row < rows.first )
					next_row = rows.first;

				// Add this range of hexes.
				const int end = std::min(rows.first + static_cast<int>(rows.second),
				                         row_end);
				for ( ; next_row < end; ++next_row )
					insert_hint = result.insert(++insert_hint,
						              map_location(column.first, next_row));

				// Have we reached the end of the board?
				if ( next_row >= row_end )
					break;
			}
		}
	}

} // namespage for get_tiles_radius() helpers.


/**
 * Function that will add to @a result all elements of @a locs, plus all
 * on-board locations that are within @a radius tiles of an element of locs.
 * @a result must be a std::set of locations.
 */
// Complexity: O(nr lg(nr) + nr^2), where n = locs.size(), r = radius.
// The nr^2 term is bounded by the size of the board.
void get_tiles_radius(const gamemap& map, const std::vector<map_location>& locs,
                      size_t radius, std::set<map_location>& result,
                      bool with_border)
{
	// Make sure the provided locations are included.
	// This would be needed in case some of the provided locations are off-map.
	// It also allows simpler processing if the radius is zero.
	// For efficiency, do this first since locs is potentially unsorted.
	result.insert(locs.begin(), locs.end());

	if ( radius != 0  &&  !locs.empty() )
	{
		const int border = with_border ? map.border_size() : 0;
		column_ranges collected_tiles;

		// Collect the hexes within the desired disks into collected_tiles.
		// This maps each x-value to a set of ranges of y-values that
		// are covered by the disks around each element of locs.
		// (So the data size at this point is proportional to the number
		// of x-values involved, which is O(nr). The lg(nr) factor comes
		// from the data being sorted.)
		get_column_ranges(collected_tiles, locs, radius, -border, map.w() + border);

		// Now that all the tiles have been collected, add them to result.
		// (There are O(nr^2) hexes to add.) By collecting before adding, each
		// hex will be processed only once, even when disks overlap. This is
		// how we can get good performance if there is significant overlap, and
		// how the work required can be bound by the size of the board.
		ranges_to_tiles(result, collected_tiles, -border, map.h() + border);
	}
}


/**
 * Function that will add to @a result all elements of @a locs, plus all
 * on-board locations matching @a pred that are connected to elements of
 * locs by a chain of at most @a radius tiles, each of which matches @a pred.
 * @a result must be a std::set of locations.
 */
void get_tiles_radius(const gamemap& map, const std::vector<map_location>& locs,
                      size_t radius, std::set<map_location> &result,
                      bool with_border, const xy_pred& pred)
{
	typedef std::set<map_location> location_set;

	location_set must_visit, filtered_out;
	location_set not_visited(locs.begin(), locs.end());

	for ( ; radius != 0  &&  !not_visited.empty(); --radius )
	{
		location_set::const_iterator it = not_visited.begin();
		location_set::const_iterator it_end = not_visited.end();

		result.insert(it, it_end);
		for(; it != it_end; ++it) {
			map_location adj[6];
			get_adjacent_tiles(*it, adj);
			for(size_t i = 0; i != 6; ++i) {
				const map_location& loc = adj[i];
				if ( with_border ? map.on_board_with_border(loc) :
				                   map.on_board(loc) ) {
					if ( !result.count(loc) && !filtered_out.count(loc) ) {
						if ( pred(loc) )
							must_visit.insert(loc);
						else
							filtered_out.insert(loc);
					}
				}
			}
		}

		not_visited.swap(must_visit);
		must_visit.clear();
	}

	result.insert(not_visited.begin(), not_visited.end());
}

