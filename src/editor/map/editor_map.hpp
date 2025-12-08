/*
	Copyright (C) 2008 - 2025
	by Tomasz Sniatowski <kailoran@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "editor/editor_common.hpp"
#include "map/map.hpp"

#include <boost/dynamic_bitset.hpp>

namespace editor {

struct editor_map_operation_exception : public editor_exception
{
	editor_map_operation_exception()
	: editor_exception("Map operation error. Check debug log for details.")
	{
	}
};

struct editor_map_integrity_error : public editor_exception
{
	editor_map_integrity_error()
	: editor_exception("Map integrity error. Check debug log for details.")
	{
	}
};

struct editor_map_load_exception : public editor_exception
{
	editor_map_load_exception(const std::string& fn, const std::string& msg)
	: editor_exception(msg), filename(fn)
	{
	}
	~editor_map_load_exception() noexcept {}
	std::string filename;
};

struct editor_map_save_exception : public editor_exception
{
	editor_map_save_exception(const std::string& msg)
	: editor_exception(msg)
	{
	}
	~editor_map_save_exception() noexcept {}
};


/**
 * Exception wrapping utility
 */
editor_map_load_exception wrap_exc(const char* type, const std::string& e_msg, const std::string& filename);

/**
 * This class adds extra editor-specific functionality to a normal gamemap.
 */
class editor_map : public gamemap
{
public:
	/**
	 * Empty map constructor
	 */
	editor_map();

	/**
	 * Create an editor map from a map data string
	 */
	editor_map(std::string_view data);

	/**
	 * Wrapper around editor_map(cfg, data) that catches possible exceptions
	 * and wraps them in a editor_map_load_exception
	 */
	static editor_map from_string(std::string_view data);

	/**
	 * Create an editor map with the given dimensions and filler terrain
	 */
	editor_map(std::size_t width, std::size_t height, const t_translation::terrain_code & filler);

	/**
	 * Create an editor_map by upgrading an existing gamemap. The map data is
	 * copied. Marked "explicit" to avoid potentially harmful automatic conversions.
	 */
	explicit editor_map(const gamemap& map);

	/**
	 * Get a contiguous set of tiles having the same terrain as the starting location.
	 * Useful for flood fill or magic wand selection
	 * @return a contiguous set of locations that will always contain at least the starting element
	 */
	std::set<map_location> get_contiguous_terrain_tiles(const map_location& start) const;

	/**
	 * Set labels for staring positions in the given display object.
	 * @return the locations where the labels were added
	 */
	std::set<map_location> set_starting_position_labels(display& disp);

	/**
	 * @return true when the location is part of the selection, false otherwise
	 */
	bool in_selection(const map_location& loc) const;

	/**
	 * Add a location to the selection. The location should be valid (i.e. on the map)
	 * @return true if the selected hexes set was modified
	 */
	bool add_to_selection(const map_location& loc);

	/**
	 * Remove a location to the selection. The location does not actually have to be selected
	 * @return true if the selected hexes set was modified
	 */
	bool remove_from_selection(const map_location& loc);

	/**
	 * Select the given area.
	 * @param area to select.
	 */
	void set_selection(const std::set<map_location>& area);

	/**
	 * Return the selection set.
	 */
	std::set<map_location> selection() const;

	/**
	 * Returns a set of all hexes *not* currently selected.
	 */
	std::set<map_location> selection_inverse() const;

	/**
	 * Clear the selection
	 */
	void clear_selection();

	/**
	 * Invert the selection, i.e. select all the map hexes that were not selected.
	 */
	void invert_selection();

	/**
	 * Select all map hexes
	 */
	void select_all();

	/**
	 * @return true if the entire map is selected, false otherwise
	 */
	bool everything_selected() const;

	/**
	 * @return true if at least one location is selected, false otherwise
	 */
	bool anything_selected() const;

	/**
	 * @return true if at no selections are selected, false otherwise
	 */
	bool nothing_selected() const;

	/**
	 * @return the number of locations selected.
	 *
	 * @note This is more efficient than checking selection().size() since
	 * it avoids allocating a full map_location set and simply queries the
	 * number of engaged bits in the underlying bitset.
	 */
	std::size_t num_selected() const;

	/**
	 * Resize the map. If the filler is NONE, the border terrain will be copied
	 * when expanding, otherwise the filler terrain will be inserted there
	 */
	void resize(int width, int height, int x_offset, int y_offset,
		const t_translation::terrain_code & filler = t_translation::NONE_TERRAIN);

	/**
	 * A sort-of diff operation returning a mask that, when applied to the current editor_map,
	 * will transform it into the target map.
	 */
	gamemap mask_to(const gamemap& target) const;

	/**
	 * A precondition to several map operations
	 * @return true if this map has the same dimensions as the other map
	 */
	bool same_size_as(const gamemap& other) const;

private:
	//helper functions for resizing
	void expand_right(int count, const t_translation::terrain_code & filler);
	void expand_left(int count, const t_translation::terrain_code & filler);
	void expand_top(int count, const t_translation::terrain_code & filler);
	void expand_bottom(int count, const t_translation::terrain_code & filler);
	void shrink_right(int count);
	void shrink_left(int count);
	void shrink_top(int count);
	void shrink_bottom(int count);

	class selection_mask
	{
	public:
		explicit selection_mask(const gamemap_base& map)
			: stride_(map.total_width())
			, height_(map.total_height())
		{
			bitset_.resize(map.total_area());
		}

		/**
		 * Marks @a loc as selected.
		 * @returns true if the location was previously deselected.
		 */
		bool select(const map_location& loc)
		{
			return bitset_.test_set(get_index(loc), true) == false;
		}

		/**
		 * Marks @a loc as unselected.
		 * @returns true if the location was previously selected.
		 */
		bool deselect(const map_location& loc)
		{
			return bitset_.test_set(get_index(loc), false) == true;
		}

		bool selected(const map_location& loc) const
		{
			return bitset_.test(get_index(loc));
		}

		void select_all()
		{
			bitset_.set();
		}

		void deselect_all()
		{
			bitset_.reset();
		}

		void invert()
		{
			bitset_.flip();
		}

		selection_mask inverted() const
		{
			auto res = selection_mask{*this};
			res.invert();
			return res;
		}

		static std::set<map_location> get_locations(const selection_mask& mask)
		{
			std::set<map_location> res;

			for(std::size_t i = 0; i < mask.bitset_.size(); ++i) {
				if(mask.bitset_.test(i)) {
					res.emplace(mask.get_location(i));
				}
			}

			return res;
		}

		/** Read-only access to the underlying bitset. */
		const boost::dynamic_bitset<uint64_t>& mask() const
		{
			return bitset_;
		}

	private:
		/** Indexes @a loc to its corresponding bitflag using row-major ordering. */
		std::size_t get_index(const map_location& loc) const
		{
			return static_cast<std::size_t>(loc.wml_y()) * stride_ + loc.wml_x();
		}

		/** Gets the corresponding map_location for bit index @a i. */
		map_location get_location(std::size_t i) const
		{
			auto pos = std::div(i, stride_);
			return {pos.rem, pos.quot, wml_loc{}};
		}

		boost::dynamic_bitset<uint64_t> bitset_;

		int stride_{0};
		int height_{0};
	};

	/**
	 * The selected hexes
	 */
	selection_mask selection_;
};


} //end namespace editor
