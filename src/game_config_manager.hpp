/*
	Copyright (C) 2013 - 2024
	by Andrius Silinskas <silinskas.andrius@gmail.com>
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

#include "achievements.hpp"
#include "commandline_options.hpp"
#include "config.hpp"
#include "config_cache.hpp"
#include "filesystem.hpp"
#include "game_config_view.hpp"
#include "terrain/type_data.hpp"
#include "utils/optional_fwd.hpp"

class game_classification;
class game_config_manager
{
	friend class game_config_view;
public:
	game_config_manager(const commandline_options& cmdline_opts);
	~game_config_manager();

	game_config_manager(const game_config_manager&) = delete;
	game_config_manager& operator=(const game_config_manager&) = delete;

	enum FORCE_RELOAD_CONFIG
	{
		/** Always reload config */
		FORCE_RELOAD,
		/** Don't reload if the previous defines equal the new defines */
		NO_FORCE_RELOAD,
		/** Don't reload if the previous defines include the new defines */
		NO_INCLUDE_RELOAD,
	};

	const game_config_view& game_config() const { return game_config_view_; }
	const preproc_map& old_defines_map() const { return old_defines_map_; }
	const std::shared_ptr<terrain_type_data>& terrain_types() const { return tdata_; }
	std::vector<achievement_group>& get_achievements() { return achievements_.get_list(); }

	bool init_game_config(FORCE_RELOAD_CONFIG force_reload);
	void reload_changed_game_config();

	void load_game_config_for_editor();
	void load_game_config_for_game(const game_classification& classification, const std::string& scenario_id);
	void load_game_config_for_create(bool is_mp, bool is_test = false);

	static game_config_manager * get();

private:
	void set_enabled_addon(const std::set<std::string>& addon_ids);
	void set_enabled_addon_all();

	void load_game_config(bool reload_everything, const game_classification* classification, const std::string& scenario_id);

	void load_game_config_with_loadscreen(FORCE_RELOAD_CONFIG force_reload, const game_classification* classification, const std::string& scenario_id);

	// load_game_config() helper functions.
	void load_addons_cfg();
	void set_multiplayer_hashes();
	void set_unit_data();

	const commandline_options& cmdline_opts_;

	config game_config_;
	game_config_view game_config_view_;

	std::map<std::string, config> addon_cfgs_;
	std::set<std::string> active_addons_;

	preproc_map old_defines_map_;

	filesystem::binary_paths_manager paths_manager_;

	game_config::config_cache& cache_;

	std::shared_ptr<terrain_type_data> tdata_;

	achievements achievements_;
};
