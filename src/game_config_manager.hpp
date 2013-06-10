/*
   Copyright (C) 2013 by Andrius Silinskas <silinskas.andrius@gmail.com>
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

class game_config_manager
{
public:
	game_config_manager(const commandline_options& cmdline_opts,
	                    game_display& disp);
	~game_config_manager();

	enum SET_BINARY_PATHS { SET_PATHS, NO_SET_PATHS };
	enum FORCE_RELOAD_CONFIG { FORCE_RELOAD, NO_FORCE_RELOAD };

	const config& game_config() const { return game_config_; }

	bool init_config(FORCE_RELOAD_CONFIG force_reload,
	                 const bool jump_to_editor = false);
	void load_game_cfg(SET_BINARY_PATHS set_paths,
	                   FORCE_RELOAD_CONFIG force_reload);
	void reload_changed_game_config(const bool jump_to_editor = false);

private:
	game_config_manager(const game_config_manager&);
	void operator=(const game_config_manager&);

	const commandline_options& cmdline_opts_;
	game_display& disp_;

	config game_config_;

	preproc_map old_defines_map_;

	binary_paths_manager paths_manager_;

	game_config::config_cache& cache_;
};

#endif
