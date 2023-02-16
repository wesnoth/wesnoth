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

#pragma once

#include <map>
#include <string>
#include <vector>

#include "config.hpp"
#include "tstring.hpp"

/**
 * Represents a single achievement and its data.
 */
struct achievement
{
	/** The ID of the achievement. Must be unique per achievement_group */
	std::string id_;
	/** The name of the achievement to show on the UI. */
	t_string name_;
	/** The name of the achievement to show on the UI if the achievement is completed. */
	t_string name_completed_;
	/** The description of the achievement to show on the UI. */
	t_string description_;
	/** The name of the achievement to show on the UI if the achievement is completed. */
	t_string description_completed_;
	/** The icon of the achievement to show on the UI. */
	std::string icon_;
	/** The icon of the achievement to show on the UI if the achievement is completed. */
	std::string icon_completed_;
	/** Whether to show the achievement's actual name and description on the UI before it's been completed. */
	bool hidden_;
	/** Whether the achievement has been completed. */
	bool achieved_;
	/** When the achievement's current progress matches or equals this value, then it should be marked as completed */
	int max_progress_;
	/** The current progress value of the achievement */
	int current_progress_;
	/** The path to a sound to play when an achievement is completed */
	std::string sound_path_;

	achievement(const config& cfg, bool achieved, int progress)
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
	}
};

/**
 * A set of achievements tied to a particular content. Achievements can be added to any content from any add-on, even if it's entirely unrelated.
 */
struct achievement_group
{
	/** The name of the content to display on the UI. */
	t_string display_name_;
	/** The internal ID used for this content. */
	std::string content_for_;
	/** The achievements associated to this content. */
	std::vector<achievement> achievements_;

	achievement_group(const config& cfg);
};

/**
 * This class is responsible for reading all available achievements from mainline's and any add-ons' achievements.cfg files for use in achievements_dialog.
 */
class achievements
{
public:
	achievements();
	void reload();
	std::vector<achievement_group>& get_list()
	{
		return achievement_list_;
	}

private:
	config read_achievements_file(const std::string& path);
	void process_achievements_file(const config& cfg, const std::string& content_source);

	std::vector<achievement_group> achievement_list_;
};
