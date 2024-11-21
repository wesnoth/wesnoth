/*
	Copyright (C) 2006 - 2024
	by Karol Nowak <grzywacz@sul.uni.lodz.pl>
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

#include "generic_event.hpp"
#include "map/location.hpp"

class config;
class display;

namespace soundsource {

class sourcespec;

/*
 * Sound source is an object on a map (a location) which has one or more
 * sounds effects associated with it, which are played randomly and with
 * appropriate delays, when sound emitting object is visible on screen.
 */
class positional_source {
	std::chrono::steady_clock::time_point last_played_;
	std::chrono::milliseconds min_delay_;
	int chance_;
	int loops_;
	const unsigned int id_;
	int range_;
	int faderange_;
	bool check_fogged_;
	bool check_shrouded_;
	std::string files_;
	std::vector<map_location> locations_;

	// Last assigned id; this can, of course, overflow, but I'd
	// never expect to see 4 billions sound sources being created...
	static unsigned int last_id;

public:
	// min_delay is a minimum time in seconds, which must pass before
	// this sound source can be played again if it remains visible
	//
	// chance is a chance ;-) (in %) that the sound source will emit
	// sound every second after the delay has passed or once the source
	// becomes visible
	positional_source(const sourcespec &spec);
	~positional_source();

	bool is_global() const;

	void update(const std::chrono::steady_clock::time_point& time, const display &disp);
	void update_positions(const std::chrono::steady_clock::time_point& time, const display &disp);

	int calculate_volume(const map_location &loc, const display &disp);

	/**
	 * Serializes attributes as WML config.
	 * @param cfg A reference to a [sound_source] tag object.
	 */
	void write_config(config& cfg) const;
};

class manager : public events::observer
{

	typedef std::map<std::string, std::unique_ptr<positional_source>> positional_source_map;
	typedef positional_source_map::iterator            positional_source_iterator;
	typedef positional_source_map::const_iterator      positional_source_const_iterator;

	positional_source_map sources_;
	const display &disp_;

public:
	manager(const display &disp);
	~manager();

	// event interface
	void handle_generic_event(const std::string &event_name);

	// add or replace a soundsource
	void add(const sourcespec &source);
	void remove(const std::string &id);
	sourcespec get(const std::string &id);
	bool contains(const std::string& id);
	void update();

	// checks which sound sources are visible
	void update_positions();

	/**
	 * Serializes information into cfg as new children of key
	 * "sound_source", appended to existing content.
	 */
	void write_sourcespecs(config& cfg) const;
};

/**
 * Sound source info class.
 * Encapsulates sound source parameters, so that they're easier to pass
 * around/extend/read.
 */
class sourcespec
{
	const std::string id_;
	std::string files_;

	std::chrono::milliseconds min_delay_;
	int chance_;

	int loops_;
	int range_;
	int faderange_;
	bool check_fogged_;
	bool check_shrouded_;

	std::vector<map_location> locations_;

public:
	/** Parameter-list constructor. */
	sourcespec(const std::string& id, const std::string& files, const std::chrono::milliseconds& min_delay, int chance) :
		id_(id),
		files_(files),
		min_delay_(min_delay),
		chance_(chance),
		loops_(0),
		range_(3),
		faderange_(14),
		check_fogged_(false),
		check_shrouded_(false),
		locations_()
	{}

	/** WML constructor. */
	sourcespec(const config& cfg);

	/**
	 * Serializes information into cfg as a new (appended)
	 * child of key "sound_source".
	 */
	void write(config& cfg) const;

	int loops() const { return loops_; }

	void set_loops(int value) {
		loops_ = value;
	}

	bool check_fogged() const { return check_fogged_; }
	bool check_shrouded() const { return check_shrouded_; }

	void set_check_fogged(bool value) {
		check_fogged_ = value;
	}

	void set_check_shrouded(bool value) {
		check_shrouded_ = value;
	}

	const std::vector<map_location>& get_locations() const {
		return locations_;
	}

	void set_locations(const std::vector<map_location>& locs) {
		locations_ = locs;
	}

	int full_range() const { return range_; }

	void set_full_range(int value) {
		range_ = value;
	}

	int fade_range() const { return faderange_; }

	void set_fade_range(int value) {
		faderange_ = value;
	}

	auto minimum_delay() const { return min_delay_; }

	void set_minimum_delay(const std::chrono::milliseconds& value) {
		min_delay_ = value;
	}

	int chance() const { return chance_; }

	void set_chance(int value) {
		chance_ = value;
	}

	const std::string& id() const { return id_; }

	const std::string& files() const { return files_; }

	void set_files(const std::string& f) {
		files_ = f;
	}
};

} // namespace soundsource
