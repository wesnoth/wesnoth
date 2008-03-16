/* $Id$ */
/*
   Copyright (C) 2006 - 2008 by Karol Nowak <grzywacz@sul.uni.lodz.pl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef SOUNDSOURCE_HPP_INCLUDED
#define SOUNDSOURCE_HPP_INCLUDED

#include <map>
#include <string>
#include <vector>

#include "generic_event.hpp"
#include "map.hpp"

class display;

namespace soundsource {

class sourcespec;
class manager;

/*
 * Sound source is an object on a map (a location) which has one or more
 * sounds effects associated with it, which are played randomly and with
 * appropriate delays, when sound emiting object is visible on screen.
 */
class positional_source {
	friend class manager;

	unsigned int _last_played;
	unsigned int _min_delay;
	unsigned int _chance;
	unsigned int _loops;
	const unsigned int _id;
	unsigned int _range;
	bool _check_fogged;
	std::string _files;
	std::vector<gamemap::location> _locations;

	// Last assigned id; this can, of course, overflow, but I'd
	// never expect to see 4 billions sound sources being created...
	static unsigned int last_id;

	// min_delay is a minimum time in seconds, which must pass before
	// this sound source can be played again if it remains visible
	//
	// chance is a chance ;-) (in %) that the sound source will emit
	// sound every second after the delay has passed or once the source
	// becomes visible
	positional_source(const sourcespec &spec);

	void update(unsigned int time, const display &disp);
	void update_positions(unsigned int time, const display &disp);

	void add_location(const gamemap::location &loc);
	void remove_location(const gamemap::location &loc);
	void replace_location(const gamemap::location &oldloc, const gamemap::location &newloc);
	
	int calculate_volume(const gamemap::location &loc, const display &disp);
};

class manager : public events::observer {

	typedef std::map<std::string, positional_source *> positional_source_map;
	typedef positional_source_map::iterator positional_source_iterator;

	positional_source_map _sources;
	const display &_disp;

	// checks which sound sources are visible
	void update_positions();

public:
	manager(const display &disp);
	~manager();

	// event interface
	void handle_generic_event(const std::string &event_name);

	// add or replace a soundsource
	void add(const sourcespec &source);
	void remove(const std::string &id);
	void update();

	void add_location(const std::string &id, const gamemap::location &loc);
};

/*
 * A class encapsulating parameters, so that they're easier to pass around/extend/read.
 */
class sourcespec {
	const std::string &id;
	const std::string &files;

	int min_delay;
	int chance;

	int loops;
	int range;
	bool check_fogged;

	std::vector<gamemap::location> locations;

public:
	sourcespec(const std::string &id_, const std::string &files_, int min_delay_, int chance_)
		: id(id_), files(files_), min_delay(min_delay_), chance(chance_), loops(0), check_fogged(false)
	{ 
	}

	sourcespec& loop(int loops_) {
		loops = loops_;
		return *this;
	}

	sourcespec& check_fog(bool fogged) {
		check_fogged = fogged;
		return *this;
	}

	sourcespec& location(const gamemap::location &loc) {
		locations.push_back(loc);
		return *this;
	}

	sourcespec& full_range(int range_) {
		range = maximum<int>(1, range_);
		return *this;
	}

	friend class manager;
	friend class positional_source;
};

} // namespace soundsourcespec

#endif
