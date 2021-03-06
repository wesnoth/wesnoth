/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
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

class config;

#include "map/location.hpp"
#include "terrain/translation.hpp"
#include "terrain/type_data.hpp"
#include <optional>

//class terrain_type_data; Can't forward declare because of enum

// This could be moved to a separate file pair...
class gamemap_base
{
public:
	using terrain_code = t_translation::terrain_code;
	using terrain_map = t_translation::ter_map;
	using location_map = t_translation::starting_positions;
	virtual ~gamemap_base();

	/** The default border style for a map. */
	static const int default_border = 1;

	/**
	 * Maximum number of players supported.
	 *
	 * Warning: when you increase this, you need to add
	 * more definitions to the team_colors.cfg file.
	 */
	static const int MAX_PLAYERS = 9;

	std::string to_string() const;

	/** Effective map width. */
	int w() const { return total_width() - 2 * border_size(); }

	/** Effective map height. */
	int h() const { return total_height() - 2 * border_size(); }

	/** Size of the map border. */
	int border_size() const { return default_border; }

	/** Real width of the map, including borders. */
	int total_width()  const { return tiles_.w; }

	/** Real height of the map, including borders */
	int total_height() const { return tiles_.h; }

	/** Tell if the map is of 0 size. */
	bool empty() const
	{
		return w() <= 0 || h() <= 0;
	}

	/**
	 * Tell if a location is on the map.
	 */
	bool on_board(const map_location& loc) const;
	bool on_board_with_border(const map_location& loc) const;

	/**
	 * Clobbers over the terrain at location 'loc', with the given terrain.
	 * Uses mode and replace_if_failed like merge_terrains().
	 */
	virtual void set_terrain(const map_location& loc, const terrain_code & terrain, const terrain_type_data::merge_mode mode = terrain_type_data::BOTH, bool replace_if_failed = false) = 0;

	/**
	 * Looks up terrain at a particular location.
	 *
	 * Hexes off the map may be looked up, and their 'emulated' terrain will
	 * also be returned.  This allows proper drawing of the edges of the map.
	 */
	terrain_code get_terrain(const map_location& loc) const;

	location_map& special_locations() { return starting_positions_; }
	const location_map& special_locations() const { return starting_positions_; }
	const std::vector<map_location> starting_positions() const;

	void set_special_location(const std::string& id, const map_location& loc);
	map_location special_location(const std::string& id) const;

	/** Manipulate starting positions of the different sides. */
	void set_starting_position(int side, const map_location& loc);
	map_location starting_position(int side) const;

	/** Counts the number of sides that have valid starting positions on this map */
	int num_valid_starting_positions() const;
	/** returns the side number of the side starting at position loc, 0 if no such side exists. */
	int is_starting_position(const map_location& loc) const;
	/** returns the name of the special location at position loc, null if no such location exists. */
	const std::string* is_special_location(const map_location& loc) const;

	/** Parses ranges of locations into a vector of locations, using this map's dimensions as bounds. */
	std::vector<map_location> parse_location_range(const std::string& xvals, const std::string &yvals, bool with_border = false) const;

	struct overlay_rule
	{
		t_translation::ter_list old_;
		t_translation::ter_list new_;
		terrain_type_data::merge_mode mode_;
		std::optional<t_translation::terrain_code> terrain_;
		bool use_old_;
		bool replace_if_failed_;

		overlay_rule()
			: old_()
			, new_()
			, mode_(terrain_type_data::BOTH)
			, terrain_()
			, use_old_(false)
			, replace_if_failed_(false)
		{

		}
	};

	/** Overlays another map onto this one at the given position. */
	void overlay(const gamemap_base& m, map_location loc, const std::vector<overlay_rule>& rules = std::vector<overlay_rule>(), bool is_odd = false, bool ignore_special_locations = false);

	template<typename F>
	void for_each_loc(const F& f) const
	{
		for(int x = 0; x < total_width(); ++x) {
			for(int y = 0; y < total_height(); ++y) {
				f(map_location{x, y , wml_loc()});
			}
		}
	}
	//Doesn't include border.
	template<typename F>
	void for_each_walkable_loc(const F& f) const
	{
		for(int x = 0; x < w(); ++x) {
			for(int y = 0; y < h(); ++y) {
				f(map_location{x, y});
			}
		}
	}
protected:
	gamemap_base() = default;
	gamemap_base(int w, int h, terrain_code default_ter = terrain_code());
	terrain_map& tiles() {return tiles_;}
	const terrain_map& tiles() const {return tiles_;}
private:
	terrain_map tiles_;
	location_map starting_positions_;
};

/**
 * Encapsulates the map of the game.
 *
 * Although the game is hexagonal, the map is stored as a grid.
 * Each type of terrain is represented by a multiletter terrain code.
 * @todo Update for new map-format.
 */
class gamemap : public gamemap_base
{
public:

	/* Get info from the terrain_type_data object about the terrain at a location */
	const t_translation::ter_list& underlying_mvt_terrain(const map_location& loc) const;
	const t_translation::ter_list& underlying_def_terrain(const map_location& loc) const;
	const t_translation::ter_list& underlying_union_terrain(const map_location& loc) const;
	std::string get_terrain_string(const map_location& loc) const;
	std::string get_terrain_editor_string(const map_location& loc) const;

	bool is_village(const map_location& loc) const;
	int gives_healing(const map_location& loc) const;
	bool is_castle(const map_location& loc) const;
	bool is_keep(const map_location& loc) const;

	/* The above wrappers, but which takes a terrain. This is the old syntax, preserved for brevity in certain cases. */
	const t_translation::ter_list& underlying_mvt_terrain(const t_translation::terrain_code & terrain) const;
	const t_translation::ter_list& underlying_def_terrain(const t_translation::terrain_code & terrain) const;
	const t_translation::ter_list& underlying_union_terrain(const t_translation::terrain_code & terrain) const;
	std::string get_terrain_string(const t_translation::terrain_code& terrain) const;
	std::string get_terrain_editor_string(const t_translation::terrain_code& terrain) const;
	std::string get_underlying_terrain_string(const t_translation::terrain_code& terrain) const;

	bool is_village(const t_translation::terrain_code & terrain) const;
	int gives_healing(const t_translation::terrain_code & terrain) const;
	bool is_castle(const t_translation::terrain_code & terrain) const;
	bool is_keep(const t_translation::terrain_code & terrain) const;

	// Also expose this for the same reason:
	const terrain_type& get_terrain_info(const t_translation::terrain_code & terrain) const;

	/* Get the underlying terrain_type_data object. */
	const std::shared_ptr<terrain_type_data>& tdata() const { return tdata_; }

	/**
	 * Loads a map.
	 *
	 * Data should be a series of lines, with each character representing one
	 * hex on the map.  Starting locations are represented by numbers.
	 *
	 * @param data the map data to load.
	 */
	gamemap(const std::string& data); // throw(incorrect_map_format_error)

	void read(const std::string& data, const bool allow_invalid = true);

	std::string write() const;

	const t_translation::terrain_code operator[](const map_location& loc) const
	{
		return tiles().get(loc.x + border_size(), loc.y + border_size());
	}
private:
	//private method, use set_terrain instead which also updates villages_.
	t_translation::terrain_code& operator[](const map_location& loc)
	{
		return tiles().get(loc.x + border_size(), loc.y + border_size());
	}
public:
	void set_terrain(const map_location& loc, const terrain_code & terrain, const terrain_type_data::merge_mode mode = terrain_type_data::BOTH, bool replace_if_failed = false) override;

	/** Writes the terrain at loc to cfg. */
	void write_terrain(const map_location &loc, config& cfg) const;

	/** Return a list of the locations of villages on the map. */
	const std::vector<map_location>& villages() const { return villages_; }

	/** Shortcut to get_terrain_info(get_terrain(loc)). */
	const terrain_type& get_terrain_info(const map_location &loc) const;

	/** Gets the list of terrains. */
	const t_translation::ter_list& get_terrain_list() const;

private:

	/**
	 * Reads the header of a map which is saved in the deprecated map_data format.
	 *
	 * @param data		          The mapdata to load.
	 */
	int read_header(const std::string& data);

	std::shared_ptr<terrain_type_data> tdata_;

protected:
	std::vector<map_location> villages_;
};
