/*
	Copyright (C) 2003 - 2025
	by David White <dave@whitevine.net>, Iris Morelle <shadowm2006@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "sound_music_track.hpp"

#include "config.hpp"
#include "filesystem.hpp"
#include "log.hpp"
#include "serialization/string_utils.hpp"
#include "vorbis/vorbisfile.h"

static lg::log_domain log_audio("audio");
#define ERR_AUDIO LOG_STREAM(err, log_audio)
#define LOG_AUDIO LOG_STREAM(info, log_audio)

namespace sound
{
namespace
{
std::string title_from_file(const std::string& track_path)
{
	OggVorbis_File vf;
	if(ov_fopen(track_path.c_str(), &vf) < 0) {
		LOG_AUDIO << "Error opening file '" << track_path << "' for track identification";
		return "";
	}

	vorbis_comment* comments = ov_comment(&vf, -1);
	char** user_comments = comments->user_comments;

	std::string title;
	for(int i = 0; i < comments->comments; i++) {
		const std::string comment_string(user_comments[i]);
		const auto comment_list = utils::split_view(comment_string, '=');

		if(comment_list[0] == "TITLE" || comment_list[0] == "title") {
			title = comment_list[1];
			break;
		}
	}

	ov_clear(&vf);
	return title;
}

utils::optional<std::string> resolve_track_path(const std::string& track_file)
{
	if(!track_file.empty()) {
		return filesystem::get_localized_path(filesystem::get_binary_file_location("music", track_file));
	}

	LOG_AUDIO << "empty track filename specified for track identification";
	return utils::nullopt;
}

} // namespace

std::shared_ptr<music_track> music_track::create(const config& cfg)
{
	if(auto path = resolve_track_path(cfg["name"])) {
		return std::make_shared<music_track>(*path, cfg);
	}

	LOG_AUDIO << "could not find track '" << cfg["name"] << "'";
	return nullptr;
}

std::shared_ptr<music_track> music_track::create(const std::string& file)
{
	if(auto path = resolve_track_path(file)) {
		return std::make_shared<music_track>(*path, file);
	}

	LOG_AUDIO << "could not find track '" << file << "'";
	return nullptr;
}

music_track::music_track(const std::string& file_path, const config& node)
	: id_(node["name"])
	, file_path_(file_path)
	, title_(node["title"])
	, ms_before_(node["ms_before"].to_int())
	, ms_after_(node["ms_after"].to_int())
	, once_(node["play_once"].to_bool())
	, append_(node["append"].to_bool())
	, immediate_(node["immediate"].to_bool())
	, shuffle_(node["shuffle"].to_bool(true))
{
	if(title_.empty()) {
		title_ = title_from_file(file_path_);
	}
}

music_track::music_track(const std::string& file_path, const std::string& file)
	: id_(file)
	, file_path_(file_path)
	, title_(title_from_file(file_path_))
{
}

void music_track::write(config &parent_node, bool append) const
{
	config& m = parent_node.add_child("music");
	m["name"] = id_;
	m["ms_before"] = ms_before_;
	m["ms_after"] = ms_after_;
	if(append) {
		m["append"] = true;
	}
	//default behaviour is to shuffle
	m["shuffle"] = shuffle_;
}

} /* end namespace sound */
