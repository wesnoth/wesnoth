/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
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

#include "utils/make_enum.hpp"

#include <vector>

class config;

/// The default difficulty setting for campaigns.
extern const std::string DEFAULT_DIFFICULTY;

//meta information of the game
class game_classification
{
public:
	game_classification();
	explicit game_classification(const config& cfg);
	game_classification(const game_classification& gc);

	config to_config() const;
	std::string get_tagname() const;
	bool is_normal_mp_game() const;

	std::string label;                               /**< Name of the game (e.g. name of save file). */
	std::string version;                             /**< Version game was created with. */
	MAKE_ENUM (CAMPAIGN_TYPE,			 /**< Type of the game - campaign, multiplayer etc. */
		(SCENARIO, 	"scenario")
		(MULTIPLAYER, 	"multiplayer")
		(TEST,		"test")
		(TUTORIAL,	"tutorial")
	)
	CAMPAIGN_TYPE campaign_type;
	std::string campaign_define;                     /**< If there is a define the campaign uses to customize data */
	std::vector<std::string> campaign_xtra_defines;  /**< more customization of data */
	std::string scenario_define;                     /**< If there is a define the scenario uses to customize data */
	std::string era_define;                          /**< If there is a define the era uses to customize data */
	std::vector<std::string> mod_defines;            /**< If there are defines the modifications use to customize data */

	std::string campaign;                            /**< The id of the campaign being played */
	std::string campaign_name;                       /**< The name of the campaign being played. */

	std::string abbrev;                              /**< the campaign abbreviation */
	bool end_credits;                                /**< whether to show the standard credits at the end */
	std::string end_text;                            /**< end-of-campaign text */
	unsigned int end_text_duration;                  /**< for how long the end-of-campaign text is shown */
	std::string difficulty; /**< The difficulty level the game is being played on. */
	std::string random_mode;
	bool oos_debug;
};
