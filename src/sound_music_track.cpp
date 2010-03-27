/* $Id$ */
/*
   Copyright (C) 2003 - 2010 by David White <dave@whitevine.net>
   Copyright (C) 2009 - 2010 by Ignacio R. Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
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

static lg::log_domain log_audio("audio");
#define ERR_AUDIO LOG_STREAM(err, log_audio)
#define LOG_AUDIO LOG_STREAM(info, log_audio)

namespace sound {

music_track::music_track() :
	id_(""),
	file_path_(""),
	ms_before_(0),
	ms_after_(0),
	once_(false),
	append_(false),
	immediate_(false)
{
	//
	// The first music_track may be initialized before main()
	// is reached. Using the logging facilities may lead to a SIGSEGV
	// because it's not guaranteed that their objects are already alive.
	//
	// We are safe only because the early music_track objects use
	// this default constructor.
	//
}

music_track::music_track(const config& node) :
	id_(node["name"]),
	file_path_(""),
	ms_before_(lexical_cast_default<int>(node["ms_before"])),
	ms_after_(lexical_cast_default<int>(node["ms_after"])),
	once_(utils::string_bool(node["play_once"])),
	append_(utils::string_bool(node["append"])),
	immediate_(utils::string_bool(node["immediate"]))
{
	if(id_.empty()) {
		ERR_AUDIO << "empty track filename specified\n";
	} else {
		this->resolve();
	}
}

music_track::music_track(const std::string& v_name) :
	id_(v_name),
	file_path_(""),
	ms_before_(0),
	ms_after_(0),
	once_(false),
	append_(false),
	immediate_(false)
{
	if(id_.empty()) {
		ERR_AUDIO << "empty track filename specified\n";
	} else {
		this->resolve();
	}
}

music_track::music_track(const music_track& mt) :
	id_(mt.id_),
	file_path_(mt.file_path_),
	ms_before_(mt.ms_before_),
	ms_after_(mt.ms_after_),
	once_(mt.once_),
	append_(mt.append_),
	immediate_(mt.immediate_)
{
	// Assume mt has already been resolved...
}

music_track& music_track::operator=(const music_track& mt)
{
	if(this != &mt) {
		this->id_ = mt.id_;
		this->file_path_ = mt.file_path_;
		this->ms_before_ = mt.ms_before_;
		this->ms_after_ = mt.ms_after_;
		this->once_ = mt.once_;
		// Assume mt has already been resolved...
	}

	return *this;
}

void music_track::resolve()
{
	if(id_.empty()) {
		LOG_AUDIO << "cannot resolve an empty track filename\n";
		return;
	}

	file_path_ = get_binary_file_location("music", this->id_);

	if(file_path_.empty()) {
		LOG_AUDIO << "could not resolve a file path to track '" << id_ << "'\n";
		return;
	}

	LOG_AUDIO << "resolved music track '" << id_ << "' into '" << file_path_ << "'\n";
}

void music_track::write(config &parent_node, bool append)
{
	config& m = parent_node.add_child("music");
	m["name"] = id_;
	m["ms_before"] = lexical_cast<std::string>(ms_before_);
	m["ms_after"] = lexical_cast<std::string>(ms_after_);
	if(append) {
		m["append"] = true;
	}
}

} /* end namespace sound */
