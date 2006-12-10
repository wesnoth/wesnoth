/* $Id$ */
/*
   Copyright (C) 2006 by Karol Nowak <grzywacz@sul.uni.lodz.pl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef SOUNDSOURCE_HPP_INCLUDED
#define SOUNDSOURCE_HPP_INCLUDED

#include <list>
#include <map>
#include <string>
#include <vector>

#include "map.hpp"


namespace soundsource {

class manager {
	/*
	 * Sound source is an object on a map (a location) which has one or more
	 * sounds effects associated with it, which are played randomly and with
	 * appropriate delays, when sound emiting object is visible on screen.
	 */
	class soundsource {
		unsigned int _last_played;
		unsigned int _min_delay;
		unsigned int _chance;
		bool _play_fogged;
		std::vector<std::string> _files;
		std::list<gamemap::location> _locations;

	public:
		// min_delay is a minimum time in seconds, which must pass before
		// this sound source can be played again
		//
		// chance is a chance ;-) (in %) that the sound source will emit
		// sound every second
		soundsource(const std::vector<std::string> &files, int min_delay, int chance, bool play_fogged = false);

		void update(unsigned int time, const display &disp);

		void add_location(const gamemap::location &loc);
		void remove_location(const gamemap::location &loc);
		void replace_location(const gamemap::location &oldloc, const gamemap::location &newloc);
	};

	typedef std::map<std::string, soundsource *> soundsource_map;
	typedef soundsource_map::iterator soundsource_map_iterator;

	soundsource_map _sources;
	const display &_disp;

public:
	manager(const display &disp);
	~manager();

	// add or replace a soundsource
	void add(const std::string &name, const std::vector<std::string> &files, int min_delay, int chance, bool play_fogged = false);
	void remove(const std::string &name);
	void update();

	void add_location(const std::string &name, const gamemap::location &loc);
};

} // namespce soundsource

#endif
