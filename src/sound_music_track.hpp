/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
   Copyright (C) 2009 - 2017 by Ignacio R. Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef SOUND_MUSIC_TRACK_HPP_INCLUDED
#define SOUND_MUSIC_TRACK_HPP_INCLUDED

#include <string>
#include <memory>

class config;

namespace sound {

/**
 * Internal representation of music tracks.
 */
class music_track
{
public:
	music_track();
	music_track(const config& node);
	explicit music_track(const std::string& v_name);
	void write(config& parent_node, bool append) const;

	bool valid() const { return file_path_.empty() != true; }

	bool append() const { return append_; }
	bool immediate() const { return immediate_; }
	bool shuffle() const { return shuffle_; }
	bool play_once() const { return once_; }
	int ms_before() const { return ms_before_; }
	int ms_after()  const { return ms_after_;  }

	const std::string& file_path() const { return file_path_; }
	const std::string& id() const { return id_; }
	const std::string& title() const { return title_; }

	void set_play_once(bool v) { once_ = v; }
	void set_shuffle(bool v) { shuffle_ = v; }
	void set_ms_before(int v) { ms_before_ = v; }
	void set_ms_after(int v) { ms_after_ = v; }

private:
	void resolve();

	std::string id_;
	std::string file_path_;
	std::string title_;

	int ms_before_, ms_after_;

	bool once_;
	bool append_;
	bool immediate_;
	bool shuffle_;
};

std::shared_ptr<music_track> get_track(unsigned int i);
void set_track(unsigned int i, const std::shared_ptr<music_track>& to);

} /* end namespace sound */

inline bool operator==(const sound::music_track& a, const sound::music_track& b) {
	return a.file_path() == b.file_path();
}
inline bool operator!=(const sound::music_track& a, const sound::music_track& b) {
	return a.file_path() != b.file_path();
}

#endif /* ! SOUND_MUSIC_TRACK_HPP_INCLUDED */
