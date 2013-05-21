/*
   Copyright (C) 2003 - 2013 by David White <dave@whitevine.net>
   Copyright (C) 2009 - 2013 by Ignacio R. Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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
#include "util.hpp"
#if !defined(_WIN32) && !defined(__APPLE__)
#include "vorbis/vorbisfile.h"
#endif

static lg::log_domain log_audio("audio");
#define ERR_AUDIO LOG_STREAM(err, log_audio)
#define LOG_AUDIO LOG_STREAM(info, log_audio)

namespace sound {

music_track::music_track() :
	id_(),
	file_path_(),
	ms_before_(0),
	ms_after_(0),
	once_(false),
	append_(false),
	immediate_(false)
{
}

music_track::music_track(const config& node) :
	id_(node["name"]),
	file_path_(),
	title_(),
	ms_before_(node["ms_before"]),
	ms_after_(node["ms_after"]),
	once_(node["play_once"].to_bool()),
	append_(node["append"].to_bool()),
	immediate_(node["immediate"].to_bool())
{
	resolve();
}

music_track::music_track(const std::string& v_name) :
	id_(v_name),
	file_path_(),
	title_(),
	ms_before_(0),
	ms_after_(0),
	once_(false),
	append_(false),
	immediate_(false)
{
	resolve();
}

void music_track::resolve()
{
	if (id_.empty()) {
		ERR_AUDIO << "empty track filename specified\n";
		return;
	}

	file_path_ = get_binary_file_location("music", id_);

	if (file_path_.empty()) {
		ERR_AUDIO << "could not find track '" << id_ << "'\n";
		return;
	}

#if !defined(_WIN32) && !defined(__APPLE__)
	FILE* f;
	f = fopen(file_path_.c_str(), "r");
	if (f == NULL) {
		ERR_AUDIO << "Error opening file '" << file_path_ << "'\n";
		return;
	}

	OggVorbis_File vf;
	if(ov_open(f, &vf, NULL, 0) < 0) {
		ERR_AUDIO << "track does not appear to be an Ogg file '" << id_ << "'\n";
		ov_clear(&vf);
		return;
	}

	vorbis_comment* comments = ov_comment(&vf, -1);
	char** user_comments = comments->user_comments;

	bool found = false;
	for (int i=0; i< comments->comments; i++) {
		const std::string comment_string(user_comments[i]);
		const std::vector<std::string> sowas = utils::split(comment_string, '=');

		if (sowas[0] == "TITLE" || sowas[0] == "title") {
			title_ = sowas[1];
			found = true;
		}
	}
	if (!found) {
		ERR_AUDIO << "No title for music track '" << id_ << "'\n";
	}

	ov_clear(&vf);
#endif
	LOG_AUDIO << "resolved music track '" << id_ << "' into '" << file_path_ << "'\n";
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
}

} /* end namespace sound */
