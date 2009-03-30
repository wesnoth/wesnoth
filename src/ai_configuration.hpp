/* $Id: ai_configuration.hpp   $ */
/*
   Copyright (C) 2009 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * Managing the AIs configuration - headers
 * @file ai_configuration.hpp
 * */

#ifndef AI_CONFIGURATION_HPP_INCLUDED
#define AI_CONFIGURATION_HPP_INCLUDED

#include "global.hpp"

#include "config.hpp"


#include <vector>

/**
 * AI parameters. class to deal with AI parameters. It is an implementation detail.
 * @todo: AI parameter/AI memory/AI effective parameter system must be reworked
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
class ai_configuration {
public:

	/**
	 * get side config from file
	 * @param file the file name to open. follows usual WML convention.
	 * @param cfg[out] the config to be written from file.
	 * @return was all ok?
	 * @retval true success
	 * @retval false failure
	 */
	static bool get_side_config_from_file( const std::string& file, config& cfg );


	/**
	 * @deprecated bug-for-bug compatibility with current parameter handling
	 * @param[in] cfg the config to be read
	 * @param[out] ai_algorithm_type al algorithm type to be written
	 * @param[out] global_ai_parameters global AI parameters to be written
	 * @param[out] ai_parameters vector of [ai] section parameters.
	 * @param[in] default_ai_parameters default ai parameters to use
	 * @param[out] ai_memory ai memory to be written
	 * @param[out] effective_ai_parameters effective ai parameters to be written
	 * @return was all ok?
	 * @retval true success
	 * @retval false failure
	 */
	static bool parse_side_config(const config& cfg,
			std::string& ai_algorithm_type,
			config& global_ai_parameters,
			std::vector<config>& ai_parameters,
			const config& default_ai_parameters,
			config& ai_memory,
			config& effective_ai_parameters );

	/**
	 * get default AI parameters
	 * @return default AI parameters
	 */
	static const config& get_default_ai_parameters();

private:
	static const config create_default_ai_parameters();
	static const config default_ai_parameters_;
};


#endif
