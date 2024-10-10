/*
	Copyright (C) 2003 - 2024
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
#include "pathutils_impl.hpp"

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

	map_location loc = center.get_direction(map_location::direction::south_west, radius);

	for(int n = 0; n != 6; ++n) {
		const map_location::direction dir{ n };
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
void get_tiles_radius(const map_location& center, std::size_t radius,
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
	typedef std::pair<int, std::size_t> row_range;
	// This is a map from column numbers to sets of ranges of rows.
	typedef std::map<int, std::set<row_range>> column_ranges;


	/**
	 * Function that will collect all locations within @a radius tiles of an
	 * element of @a locs, subject to the restriction col_begin <= x < col_end.
	 */
	// Complexity: O(nr lg(nr)), where n = locs.size() and r = radius.
	// In this formula, r is bound by col_end-col_begin (but that is
	// probably a rare event).
	void get_column_ranges(column_ranges & collected_tiles,
	                       const std::vector<map_location>& locs,
	                       const std::size_t radius,
	                       const int col_begin, const int col_end)
	{
#ifdef __cpp_using_enum // c++20
		using enum map_location::direction;
#else
		// Shorter names for the directions we'll use.
		const map_location::direction north_west = map_location::direction::north_west;
		const map_location::direction north_east = map_location::direction::north_east;
		const map_location::direction south_east = map_location::direction::south_east;
#endif

		// Perform this conversion once.
		const int radius_i = static_cast<int>(radius);

		for (const map_location &loc : locs)
			if ( loc != map_location::null_location() )
			{
				// Calculate the circle of hexes around this one.
				std::size_t height = radius;
				map_location top = loc.get_direction(north_west, radius_i);
				// Don't start off the map edge.
				if ( top.x < col_begin ) {
					const int col_shift = std::min(col_begin, loc.x) - top.x;
					top = top.get_direction(north_east, col_shift);
					height += col_shift;
				}
				// The left side.
				const int end_l = std::min(loc.x, col_end);
				for ( ; top.x < end_l; top = top.get_direction(north_east, 1) )
					collected_tiles[top.x].insert(row_range(top.y, ++height));
				// Extra increment so the middle column is tall enough.
				height += 2;
				// Don't start off the map edge (we allow loc to be off-board).
				if ( top.x < col_begin ) {
					const int col_shift = col_begin - top.x;
					top = top.get_direction(south_east, col_shift);
					height -= col_shift;
				}
				// The middle column and right side.
				const int end_r = std::min(loc.x + radius_i + 1, col_end);
				for ( ; top.x < end_r; top = top.get_direction(south_east, 1) )
					collected_tiles[top.x].insert(row_range(top.y, --height));
			}
	}

	/**
	 * Function that interprets @a collected_tiles and adds to @a result those
	 * whose y-coordinate satisfies row_begin <= y < row_end.
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
		// Note: This hint will get incremented later, which is the only
		// reason we require result to be initially non-empty.
		auto insert_hint = result.begin();

		for(const auto& [column, range] : collected_tiles) {
			// For this loop, the order within the set is crucial; we need
			// rows.first to be non-decreasing with each iteration.
			// Loop invariant: within this column, all rows before next_row
			// have been processed and either added to result or skipped.
			// There is no going back (nor a need to).
			int next_row = row_begin;

			for(const auto& [row_index, num_rows] : range) {
				// Skipping some rows?
				if(next_row < row_index) {
					next_row = row_index;
				}

				// Add this range of hexes.
				const int end = std::min(row_index + static_cast<int>(num_rows), row_end);
				for(; next_row < end; ++next_row) {
					insert_hint = result.insert(++insert_hint, map_location(column, next_row));
				}

				// Have we reached the end of the board?
				if(next_row >= row_end) {
					break;
				}
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
                      std::size_t radius, std::set<map_location>& result,
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
                      std::size_t radius, std::set<map_location> &result,
                      bool with_border, const xy_pred& pred)
{
	typedef std::set<map_location> location_set;
	location_set not_visited(locs.begin(), locs.end());

	get_tiles_radius(std::move(not_visited), radius, result,
		[&](const map_location& l) {
			return with_border ? map.on_board_with_border(l) : map.on_board(l);
		},
		[&](const map_location& l) {
			return pred(l);
		}
	);
}
