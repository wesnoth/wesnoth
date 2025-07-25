/*
	Copyright (C) 2025 - 2025
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

#include "config.hpp"
#include "editor/editor_common.hpp"
#include "game_config_view.hpp"
#include "sound_music_track.hpp"
#include "time_of_day.hpp"

//
// Helper methods for the editor_controller class
//

namespace utils
{
/**
 * Retuns a vector whose elements are initialized from the given range.
 *
 * @todo Make this a global utility.
 */
template<typename T, typename Range>
inline std::vector<T> from_range(Range&& range)
{
	return std::vector<T>(range.begin(), range.end());
}

} // namespace utils

namespace editor
{
struct schedule_metadata
{
	explicit schedule_metadata(const config& cfg)
		: id(cfg["id"])
		, name(cfg["name"].str(id))
		, times(utils::from_range<time_of_day>(cfg.child_range("time")))
	{
	}

	schedule_metadata(const std::string& id, const std::string& name, const std::vector<time_of_day>& times)
		: id(id)
		, name(name)
		, times(times)
	{
	}

	std::string id;
	std::string name;

	std::vector<time_of_day> times;
};

inline auto parse_editor_times(const config_array_view& editor_times_range)
{
	std::map<std::string, schedule_metadata> schedules;

	for(const config& schedule : editor_times_range) {
		const std::string schedule_id = schedule["id"];
		if(schedule_id.empty()) {
			ERR_ED << "Missing ID attribute in a TOD Schedule";
			continue;
		}

		auto [iter, success] = schedules.try_emplace(schedule_id, schedule);
		if(!success) {
			ERR_ED << "Duplicate TOD Schedule ID '" << schedule_id << "'";
		}
	}

	if(schedules.empty()) {
		ERR_ED << "No editor time schedules defined";
	}

	return schedules;
}

inline auto parse_editor_music(const config_array_view& editor_music_range)
{
	std::vector<std::shared_ptr<sound::music_track>> tracks;

	for(const config& editor_music : editor_music_range) {
		for(const config& music : editor_music.child_range("music")) {
			if(auto track = sound::music_track::create(music)) {
				tracks.push_back(std::move(track));
			}
		}
	}

	if(tracks.empty()) {
		ERR_ED << "No editor music defined";
	}

	return tracks;
}

} // namespace editor
