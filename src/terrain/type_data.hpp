/*
	Copyright (C) 2014 - 2025
	by Chris Beck <render787@gmail.com>
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

#include "terrain/terrain.hpp"

#include <map>
#include <memory>

class game_config_view;

/**
 * Contains the database of all known terrain types, both those defined
 * explicitly by WML [terrain_type]s and those made by combining pairs of
 * (base, overlay).
 *
 * A [terrain_type] isn't limited to (only a base) or (only an overlay). For example,
 * the various impassible mountains Mm^Xm, Ms^Xm, etc are defined by [terrain_type]s
 * for those specific (base, overlay) pairs.
 *
 * Implementation note: the ones defined by WML [terrain_type]'s are loaded
 * during the first call to lazy_initialization(). Any terrains made by
 * combining pairs of (base, overlay) are lazy-created during a later call to
 * find_or_create().
 *
 * The is_known() method will trigger creation of the terrain if needed.
 */
class terrain_type_data {
private:
	mutable t_translation::ter_list terrainList_;
	using tcodeToTerrain_t = std::map<t_translation::terrain_code, terrain_type>;
	mutable tcodeToTerrain_t tcodeToTerrain_;
	mutable bool initialized_;
	const game_config_view & game_config_;

	terrain_type_data(const game_config_view & game_config);

public:
	/**
	 * Regenerates the terrain database from the given game config.
	 *
	 * @returns A pointer to the new terrain database.
	 */
	static std::shared_ptr<terrain_type_data> reset(const game_config_view& game_config);

	/**
	 * @returns A pointer to the current terrain database instance.
	 */
	static std::shared_ptr<terrain_type_data> get();

	/**
	 * On the first call to this function, parse all of the [terrain_type]s
	 * that are defined in WML. This is separated from the constructor so that
	 * game_config_manager can create an instance while on the title screen,
	 * without the delay of loading the data (and it's likely that a different
	 * config will be loaded before entering the game).
	 */
	void lazy_initialization() const;

	const t_translation::ter_list & list() const;
	const std::map<t_translation::terrain_code, terrain_type> & map() const;

	/**
	 * Get the corresponding terrain_type information object for a given type
	 * of terrain.
	 *
	 * If the given terrain is not known, and can not be constructed from the
	 * known terrains, returns a default-constructed instance.
	 */
	const terrain_type& get_terrain_info(const t_translation::terrain_code & terrain) const;

	/**
	 * Get a formatted terrain name -- terrain (underlying terrains)
	 */
	t_string get_terrain_string(const t_translation::terrain_code& terrain) const;
	t_string get_terrain_editor_string(const t_translation::terrain_code& terrain) const;
	t_string get_underlying_terrain_string(const t_translation::terrain_code& terrain) const;

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
	 * Tries to find a new terrain which is the combination of old and new
	 * terrain using the merge_settings. Here "merge" means to find the
	 * best-fitting terrain code, it does not change any already-created
	 * instance of terrain_data. Think of using the editor's
	 * paint-with-one-layer functionality for the purpose of this.
	 *
	 * Relevant parameters are "layer" and "replace_conflicting"
	 * "layer" specifies the layer that should be replaced (base or overlay, default is both).
	 * If "replace_conflicting" is true the new terrain will replace the old one if merging failed
	 * (using the default base if new terrain is an overlay terrain)
	 * Will return the resulting terrain or NONE_TERRAIN if merging failed
	 */
	t_translation::terrain_code merge_terrains(const t_translation::terrain_code & old_t, const t_translation::terrain_code & new_t, const merge_mode mode, bool replace_if_failed = false) const;

	/**
	 * Returns true if get_terrain_info(terrain) would succeed, or false if
	 * get_terrain_info(terrain) would return a default-constructed instance.
	 *
	 * This has no connection to prefs::get().encountered_terrains().
	 *
	 * Implementation note: if necessary, will trigger the lazy-creation and
	 * add the resulting terrain to the terrain list.
	 */
	bool is_known(const t_translation::terrain_code & terrain) const;

private:
	tcodeToTerrain_t::const_iterator find_or_create(t_translation::terrain_code) const;

	static inline std::shared_ptr<terrain_type_data> singleton_;
};
