/*
	Copyright (C) 2003 - 2024
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

sub_achievement::sub_achievement(const config& cfg, bool achieved)
		: id_(cfg["id"].str())
		, description_(cfg["description"].t_str())
		, icon_(cfg["icon"].str()+"~GS()")
		, icon_completed_(cfg["icon"].str())
		, achieved_(achieved)
{}

achievement::achievement(const config& cfg, const std::string& content_for, bool achieved, int progress)
		: id_(cfg["id"].str())
		, name_(cfg["name"].t_str())
		, name_completed_(cfg["name_completed"].t_str())
		, description_(cfg["description"].t_str())
		, description_completed_(cfg["description_completed"].t_str())
		, icon_(cfg["icon"].str()+"~GS()")
		, icon_completed_(cfg["icon_completed"].str())
		, hidden_(cfg["hidden"].to_bool())
		, achieved_(achieved)
		, max_progress_(cfg["max_progress"].to_int(0))
		, current_progress_(progress)
		, sound_path_(cfg["sound"].str())
		, sub_achievements_()
{
	if(name_completed_.empty()) {
		name_completed_ = name_;
	}
	if(description_completed_.empty()) {
		description_completed_ = description_;
	}
	if(icon_completed_.empty()) {
		// avoid the ~GS() appended to icon_
		icon_completed_ = cfg["icon"].str();
	}

	for(const config& sub_ach : cfg.child_range("sub_achievement"))
	{
		std::string sub_id = sub_ach["id"].str();

		if(sub_id.empty()) {
			ERR_CONFIG << "Achievement " << id_ << " has a sub-achievement missing the id attribute:\n" << sub_ach.debug();
		} else {
			sub_achievements_.emplace_back(sub_ach, achieved_ || preferences::sub_achievement(content_for, id_, sub_id));
			max_progress_++;
		}
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
			ERR_CONFIG << content_for_ + " achievement id " << id << " contains a comma, skipping.";
			continue;
		} else {
			achievements_.emplace_back(ach, content_for_, preferences::achievement(content_for_, id), preferences::progress_achievement(content_for_, id));
		}
	}
}

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
