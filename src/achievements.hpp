/*
	Copyright (C) 2003 - 2025
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

#include <string>
#include <vector>

#include "config.hpp"
#include "tstring.hpp"

/**
 * Represents a distinct sub-achievement within another achievement.
 * This is intentionally a much simpler object than the regular achievements.
 */
struct sub_achievement
{
	/** The ID of the sub-achievement. Must be unique per achievement */
	std::string id_;
	/** The description of the sub-achievement to be shown in its tooltip */
	t_string description_;
	/** The icon of the sub-achievement to show on the UI when not completed. */
	std::string icon_;
	/** The icon of the sub-achievement to show on the UI when completed. */
	std::string icon_completed_;
	/** Whether the sub-achievement has been completed. */
	bool achieved_;

	sub_achievement(const config& cfg, bool achieved);
};

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
	/** The list of distinct sub-achievements for this achievement */
	std::vector<sub_achievement> sub_achievements_;

	achievement(const config& cfg, const std::string& content_for, bool achieved, int progress);
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
