/* $Id$ */
/*
   Copyright (C) 2008 by Pauli Nieminen <paniemin@cc.hut.fi>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "filesystem.hpp"
#include "config_cache.hpp"
#include "game_config.hpp"

namespace game_config {
	config_cache::config_cache() : 
		game_config_(), 
		force_valid_cache_(false),
		use_cache_(true),
		dirty_(true),
		config_root_("data/"),
		user_config_root_(get_addon_campaigns_dir()),
		defines_map_()
	{
		// To settup initial defines map correctly
		clear_defines();
	}

	std::string config_cache::get_config_root() const
	{
		return config_root_;
	}

	void config_cache::set_config_root(const std::string& path)
	{
		config_root_ = path;
	}

	std::string config_cache::get_user_config_root() const
	{
		return user_config_root_;
	}

	const preproc_map& config_cache::get_preproc_map() const
	{
		return defines_map_;
	}

	void config_cache::clear_defines()
	{
		defines_map_.clear();
		// settup default defines map

#ifdef USE_TINY_GUI
		defines_map_["TINY"] = preproc_define();
#endif

		if (game_config::small_gui)
			defines_map_["SMALL_GUI"] = preproc_define();

#ifdef HAVE_PYTHON
		defines_map_["PYTHON"] = preproc_define();
#endif

#if defined(__APPLE__)
		defines_map_["APPLE"] = preproc_define();
#endif

	}

	void config_cache::reload_translations()
	{
		if (dirty_)
		{
			reload_configs();
		} else {
			game_config_.reset_translation();
			game_config::load_config(game_config_.child("game_config"));
		}
	}

	config& config_cache::get_config()
	{
		if (!dirty_)
			return game_config_;

		reload_configs();
		dirty_ = true;

		return game_config_;
	}

	void config_cache::reload_configs(bool recheck_cache)
	{
		file_tree_checksum checksum = data_tree_checksum(recheck_cache); 
	}

	void config_cache::set_use_cache(bool use)
	{
		use_cache_ = use;
	}

	void config_cache::add_define(const std::string& define)
	{
		defines_map_[define] = preproc_define();
	}
}
