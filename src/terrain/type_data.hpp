/*
   Copyright (C) 2014 - 2017 by Chris Beck <render787@gmail.com>
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

#include "terrain/terrain.hpp"

#include <map>

class terrain_type_data {
private:
	mutable t_translation::ter_list terrainList_;
	mutable std::map<t_translation::terrain_code, terrain_type> tcodeToTerrain_;

	mutable bool initialized_;
	const config & game_config_;

public:
	terrain_type_data(const config & game_config);

	const t_translation::ter_list & list() const;
	const std::map<t_translation::terrain_code, terrain_type> & map() const;

	/**
	 * Get the corresponding terrain_type information object
	 * for a given type of terrain.
	 */
	const terrain_type& get_terrain_info(const t_translation::terrain_code & terrain) const;

	// The name of the terrain is the terrain itself,
	// The underlying terrain is the name of the terrain for game-logic purposes.
	// I.e. if the terrain is simply an alias, the underlying terrain name
	// is the name of the terrain that it's aliased to.
	const t_translation::ter_list& underlying_mvt_terrain(const t_translation::terrain_code & terrain) const;
	const t_translation::ter_list& underlying_def_terrain(const t_translation::terrain_code & terrain) const;
	const t_translation::ter_list& underlying_union_terrain(const t_translation::terrain_code & terrain) const;
	/**
	 * Get a formatted terrain name -- terrain (underlying, terrains)
	 */
	std::string get_terrain_string(const t_translation::terrain_code& terrain) const;
	std::string get_terrain_editor_string(const t_translation::terrain_code& terrain) const;
	std::string get_underlying_terrain_string(const t_translation::terrain_code& terrain) const;

	bool is_village(const t_translation::terrain_code & terrain) const
		{ return get_terrain_info(terrain).is_village(); }
	int gives_healing(const t_translation::terrain_code & terrain) const
		{ return get_terrain_info(terrain).gives_healing(); }
	bool is_castle(const t_translation::terrain_code & terrain) const
		{ return get_terrain_info(terrain).is_castle(); }
	bool is_keep(const t_translation::terrain_code & terrain) const
		{ return get_terrain_info(terrain).is_keep(); }

	enum merge_mode {
		BASE,
		OVERLAY,
		BOTH
		};

	/**
	 * Tries to merge old and new terrain using the merge_settings config
	 * Relevant parameters are "layer" and "replace_conflicting"
	 * "layer" specifies the layer that should be replaced (base or overlay, default is both).
	 * If "replace_conflicting" is true the new terrain will replace the old one if merging failed
	 * (using the default base if new terrain is an overlay terrain)
	 * Will return the resulting terrain or NONE_TERRAIN if merging failed
	 */
	t_translation::terrain_code merge_terrains(const t_translation::terrain_code & old_t, const t_translation::terrain_code & new_t, const merge_mode mode, bool replace_if_failed = false);

	/**
	 * Tries to find out if "terrain" can be created by combining two existing
	 * terrains Will add the resulting terrain to the terrain list if
	 * successful
	 */
	bool try_merge_terrains(const t_translation::terrain_code & terrain);

};

typedef std::shared_ptr<terrain_type_data> ter_data_cache;
