/*
   Copyright (C) 2013 - 2014 by Andrius Silinskas <silinskas.andrius@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef GAME_CONFIG_MANAGER_HPP_INCLUDED
#define GAME_CONFIG_MANAGER_HPP_INCLUDED

#include "commandline_options.hpp"
#include "config_cache.hpp"
#include "game_display.hpp"
#include "filesystem.hpp"

class config;
class game_classification;

class game_config_manager
{
public:
	game_config_manager(const commandline_options& cmdline_opts,
		game_display& disp,	const bool jump_to_editor);
	~game_config_manager();

	enum FORCE_RELOAD_CONFIG { FORCE_RELOAD, NO_FORCE_RELOAD };

	const config& game_config() const { return game_config_; }
	const preproc_map& old_defines_map() const { return old_defines_map_; }

	bool init_game_config(FORCE_RELOAD_CONFIG force_reload);
	void reload_changed_game_config();

	void load_game_config_for_editor();
	void load_game_config_for_game(const game_classification& classification);

private:
	game_config_manager(const game_config_manager&);
	void operator=(const game_config_manager&);

	void load_game_config(FORCE_RELOAD_CONFIG force_reload,
		game_classification const* classification = NULL);

	// load_game_config() helper functions.
	void load_addons_cfg();
	void set_multiplayer_hashes();
	void set_color_info();
	void set_unit_data();

	const commandline_options& cmdline_opts_;
	game_display& disp_;
	const bool jump_to_editor_;

	config game_config_;

	preproc_map old_defines_map_;

	filesystem::binary_paths_manager paths_manager_;

	game_config::config_cache& cache_;
};

#endif
