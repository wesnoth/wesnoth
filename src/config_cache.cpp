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

#include "config_cache.hpp"
#include "game_config.hpp"

namespace game_config {
	config_cache::config_cache() : 
		game_config_(), 
		dir_checksum_(),
		force_valid_cache_(false),
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
	}
}
