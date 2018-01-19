/*
   Copyright (C) 2009 - 2018 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * Managing the AIs configuration - headers
 * @file
 * */

#pragma once

#include "config.hpp"
#include "ai/game_info.hpp"

namespace ai {

/**
 * AI parameters. class to deal with AI parameters. It is an implementation detail.
 * We need implementation which will allow easy access to all the parameters
 * which match any of the pre-defined set of filters
 * such as 'select from ai_parameters where time_of_day=first watch'
 * or 'select from ai_parameters where active(current_game_state)=true'
 * it should be noted that there may be several variables with a same name but
 * different filters. the proposed rules, in general, are:
 * 1) scenario_parameter_in_SIDE_section > scenario_parameter_in_AI_section > default_value
 * then, if (1) is equal:
 * 2) use scenario-creator supplied priority ( 'not set' = 0)
 * then, if (2) is equal:
 * 3) more restricted parameter > less restricted parameter
 * then, if (3) is equal:
 * use any and loudly complain.
 */

struct description {
public:
	description()
		: text()
		, id()
		, cfg()
	{
	}

	t_string text;
	std::string id;
	config cfg;
};

class configuration {
public:

	/**
	 * Init the parameters of ai configuration parser
	 * @param game_config game config
	 */
	static void init(const config &game_config);
	static void add_era_ai_from_config(const config &game_config);
	static void add_mod_ai_from_config(config::const_child_itors configs);


	/**
	 * get default AI parameters
	 * @return default AI parameters
	 */
	static const config& get_default_ai_parameters();


	/**
	 * Return the config for a specified ai
	 */
	static const config& get_ai_config_for(const std::string &id);


	/**
	 * Returns a list of available AIs.
	 * @return the list of available AIs.
	 */
	static std::vector<description*> get_available_ais();


	/**
	 * get side config from file
	 * @param file the file name to open. follows usual WML convention.
	 * @param[out] cfg the config to be written from file.
	 * @return was all ok?
	 * @retval true success
	 * @retval false failure
	 */
	static bool get_side_config_from_file( const std::string& file, config& cfg );


	/**
	 * @param[in] original_cfg the config to be read
	 * @param[out] cfg parsed config
	 * @return was all ok?
	 * @retval true success
	 * @retval false failure
	 */
	static bool parse_side_config(side_number side, const config& original_cfg, config &cfg);


	/**
	 * Expand simplified aspects, similar to the change from 1.7.2 to 1.7.3
	 * but with some additional syntax options.
	 */
	static void expand_simplified_aspects(side_number side, config &cfg);
private:

	typedef std::map<std::string, description> description_map;
	static description_map ai_configurations_;
	static description_map era_ai_configurations_;
	static description_map mod_ai_configurations_;
	static config default_config_;

};

} //end of namespace ai
