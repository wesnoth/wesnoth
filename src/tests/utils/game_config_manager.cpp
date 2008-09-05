/* $Id$ */
/*
   Copyright (C) 2008 by Pauli Nieminen <paniemin@cc.hut.fi>
   Part of thie Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "tests/utils/game_config_manager.hpp"

#include "config.hpp"
#include "config_cache.hpp"
#include "filesystem.hpp"
#include "game_config.hpp"
#include "language.hpp"
#include "playcampaign.hpp"

namespace test_utils {

	static bool match_english(const language_def& def)
	{
		return def.localename == "C";
	}

	class game_config_manager {
		config cfg_;
		public:
		game_config_manager()
		{
			load_language_list();
			game_config::config_cache::instance().add_define("TEST");
			::init_textdomains(*game_config::config_cache::instance().get_config(game_config::path + "/data/test/"));
			const std::vector<language_def>& languages = get_languages();
			std::vector<language_def>::const_iterator English = std::find_if(languages.begin(),
					languages.end(),
					match_english); // Using German because the most active translation
			::set_language(*English);
		}

		config& get_config()
		{
			return cfg_;
		}
	};

	game_config_manager manager;

	const config& get_test_config_ref()
	{
		return manager.get_config();
	}

	config get_test_config()
	{
		return manager.get_config();
	}

}
