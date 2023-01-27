/*
	Copyright (C) 2003 - 2022
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "achievements.hpp"

#include "filesystem.hpp"
#include "game_config.hpp"
#include "log.hpp"
#include "preferences/general.hpp"
#include "serialization/parser.hpp"
#include "serialization/preprocessor.hpp"

static lg::log_domain log_config("config");
#define ERR_CONFIG LOG_STREAM(err, log_config)

achievements::achievements()
	: achievement_list_()
{
	reload();
}

/**
 * Reads the mainline achievements.cfg and then all the achievements of each installed add-on.
 *
 * This is intentionally handled separately from other WML loading so that:
 * a) All achievements and their status are able to be displayed on the main menu right after Wesnoth starts and regardless of which add-ons are active.
 * b) Add-ons can add additional achievements to other content, whether UMC or mainline. For example, a modification that adds more achievements for mainline campaigns.
 *
 * NOTE: These are *not* in any way related to Steam achievements!
 */
void achievements::reload()
{
	achievement_list_.clear();
	// mainline
	try {
		config cfg = read_achievements_file(game_config::path + "/data/achievements.cfg");
		process_achievements_file(cfg, "Mainline");
	} catch(const game::error& e) {
		ERR_CONFIG << "Error processing mainline achievements, ignoring: " << e.what();
	}

	// add-ons
	std::vector<std::string> dirs;
	filesystem::get_files_in_dir(filesystem::get_addons_dir(), nullptr, &dirs);
	for(const std::string& dir : dirs) {
		try {
			config cfg = read_achievements_file(filesystem::get_addons_dir() + "/" + dir + "/achievements.cfg");
			process_achievements_file(cfg, dir);
		} catch(const game::error& e) {
			ERR_CONFIG << "Error processing add-on " << dir << " achievements, ignoring: " << e.what();
		}
	}
}

/**
 * Reads an achievements.cfg file into a config.
 *
 * @param path The path to the achievements.cfg file.
 * @return The config containing all the achievements.
 */
config achievements::read_achievements_file(const std::string& path)
{
	config cfg;
	if(filesystem::file_exists(path)) {
		filesystem::scoped_istream stream = preprocess_file(path);
		read(cfg, *stream);
	}
	return cfg;
}

/**
 * Processes a config object to add new achievements to @a achievement_list_.
 *
 * @param cfg The config containing additional achievements.
 * @param content_source The source of the additional achievements - either mainline or an add-on.
 */
void achievements::process_achievements_file(const config& cfg, const std::string& content_source)
{
	for(const config& achgrp : cfg.child_range("achievement_group")) {
		if(achgrp["content_for"].str().empty()) {
			ERR_CONFIG << content_source + " achievement_group missing content_for attribute:\n" << achgrp.debug();
			continue;
		}
		achievement_list_.emplace_back(achgrp);
	}
}

achievement_group::achievement_group(const config& cfg)
	: display_name_(cfg["display_name"].t_str())
	, content_for_(cfg["content_for"].str())
	, achievements_()
{
	for(const config& ach : cfg.child_range("achievement")) {
		std::string id = ach["id"].str();

		if(id.empty()) {
			ERR_CONFIG << content_for_ + " achievement missing id attribute:\n" << ach.debug();
		} else if(id.find(',') != std::string::npos) {
			ERR_CONFIG << content_for_ + " achievement missing id " << id << " contains a comma, skipping.";
			continue;
		} else {
			achievements_.emplace_back(ach, preferences::achievement(content_for_, id), preferences::progress_achievement(content_for_, id));
		}
	}
}
