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

#include <cstdlib>

#include "display.hpp"
#include "sound.hpp"
#include "soundsource.hpp"

namespace soundsource {

manager::manager(const display &disp) : _disp(disp)
{
}

manager::~manager()
{
	for(soundsource_map_iterator it = _sources.begin(); it != _sources.end(); ++it) {
		delete (*it).second;
	}

	_sources.clear();
}

void manager::add(const std::string &name, const std::vector<std::string> &files, int min_delay, int chance, bool play_fogged)
{
	soundsource_map_iterator it;

	if((it = _sources.find(name)) == _sources.end()) {
		_sources[name] = new soundsource(files, min_delay, chance, play_fogged);
	}
	else {
		delete (*it).second;
		(*it).second = new soundsource(files, min_delay, chance, play_fogged);
	}
}

void manager::remove(const std::string &name)
{
	soundsource_map_iterator it;

	if((it = _sources.find(name)) == _sources.end())
		return;
	else {
		delete (*it).second;
		_sources.erase(it);
	}
}

void manager::update()
{
	unsigned int time = SDL_GetTicks();

	for(soundsource_map_iterator it = _sources.begin(); it != _sources.end(); ++it)
		(*it).second->update(time, _disp);
}

void manager::add_location(const std::string &name, const gamemap::location &loc)
{
	soundsource_map_iterator it = _sources.find(name);

	if(it == _sources.end())
		return;
	else
		(*it).second->add_location(loc);
}


manager::soundsource::soundsource(const std::vector<std::string> &files, int min_delay, int chance, bool play_fogged)
									: _last_played(0), _min_delay(min_delay), _chance(chance), 
									 _play_fogged(play_fogged), _files(files)
{
}

void manager::soundsource::update(unsigned int time, const display &disp)
{
	if(time - _last_played < _min_delay)
		return;

	_last_played = time;
	unsigned int i = rand() % 100 + 1;

	if(i <= _chance) {
		// If no locations have been specified, treat the source as if 
		// it was present everywhere on the map
		if(_locations.size() == 0) {
			sound::play_sound(_files[rand() % _files.size()]);
			return;
		}

		SDL_Rect area = disp.map_area();

		for(std::list<gamemap::location>::iterator i =  _locations.begin(); i != _locations.end(); ++i) {
			int locx = disp.get_location_x(*i);
			int locy = disp.get_location_y(*i);

			if(disp.outside_area(area, locx, locy) || disp.shrouded((*i).x, (*i).y) 
				|| (!_play_fogged && disp.fogged((*i).x, (*i).y)))
					continue;
			else {
				sound::play_sound(_files[rand() % _files.size()]);
				break;
			}
		}
	}
}

void manager::soundsource::add_location(const gamemap::location &loc)
{
	_locations.push_back(loc);
}

} // namespace soundsource

