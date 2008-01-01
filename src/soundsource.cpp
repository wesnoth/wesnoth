/* $Id$ */
/*
   Copyright (C) 2006 - 2007 by Karol Nowak <grzywacz@sul.uni.lodz.pl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include <cstdlib>

#include "display.hpp"
#include "sound.hpp"
#include "soundsource.hpp"


static int calculate_volume(int x, int y, const display &disp)
{
	SDL_Rect area = disp.map_outside_area();

	int dx = area.w / 2 - x; dx *= dx;
	int dy = area.h / 2 - y; dy *= dy;

	// An obscure formula to calculate SDL_Mixer's "distance" based on the source's
	// distance from screen's center
	return maximum<int>(0, 128 * static_cast<int>(std::sqrt((double) (dx + dy)) / (std::sqrt((double) (area.w*area.w + area.h * area.h)))));
}

namespace soundsource {

unsigned int manager::positional_source::last_id = 0;

manager::manager(const display &disp) : _disp(disp)
{
	_disp.scroll_event().attach_handler(this);
	update_positions();
}

manager::~manager()
{
	for(positional_source_iterator it = _sources.begin(); it != _sources.end(); ++it) {
		delete (*it).second;
	}

	_sources.clear();
}

void manager::handle_generic_event(const std::string &event_name)
{
	if(event_name == "scrolled")
		update_positions();
}

void manager::add(const std::string &id, const std::string &files, int min_delay, int chance, bool play_fogged)
{
	positional_source_iterator it;

	if((it = _sources.find(id)) == _sources.end()) {
		_sources[id] = new positional_source(files, min_delay, chance, play_fogged);
	}
	else {
		delete (*it).second;
		(*it).second = new positional_source(files, min_delay, chance, play_fogged);
	}
}

void manager::remove(const std::string &id)
{
	positional_source_iterator it;

	if((it = _sources.find(id)) == _sources.end())
		return;
	else {
		delete (*it).second;
		_sources.erase(it);
	}
}

void manager::update()
{
	unsigned int time = SDL_GetTicks();

	for(positional_source_iterator it = _sources.begin(); it != _sources.end(); ++it)
		(*it).second->update(time, _disp);
}

void manager::update_positions()
{
	unsigned int time = SDL_GetTicks();

	for(positional_source_iterator it = _sources.begin(); it != _sources.end(); ++it)
		(*it).second->update_positions(time, _disp);
}

void manager::add_location(const std::string &id, const gamemap::location &loc)
{
	positional_source_iterator it = _sources.find(id);

	if(it == _sources.end())
		return;
	else
		(*it).second->add_location(loc);
}


manager::positional_source::positional_source(const std::string &files, int min_delay, int chance, bool play_fogged)
						: _last_played(0), _min_delay(min_delay), _chance(chance),
							_id(last_id++), _play_fogged(play_fogged), _visible(false),
							_files(files)
{
}

void manager::positional_source::update(unsigned int time, const display &disp)
{
	if(time - _last_played < _min_delay || !_visible)
		return;

	_last_played = time;
	unsigned int i = rand() % 100 + 1;


	if(i <= _chance) {
		// If no locations have been specified, treat the source as if
		// it was present everywhere on the map
		if(_locations.size() == 0) {
			sound::play_sound_positioned(_files, last_id, 0);	// max volume
			return;
		}

//		SDL_Rect area = disp.map_outside_area();

		int distance_volume = 256;
		for(std::list<gamemap::location>::iterator i = _locations.begin(); i != _locations.end(); ++i) {
			int locx = disp.get_location_x(*i);
			int locy = disp.get_location_y(*i);
/*
			if(disp.outside_area(area, locx, locy) || disp.shrouded((*i).x, (*i).y)
				|| (!_play_fogged && disp.fogged((*i).x, (*i).y)))
					continue;
			else {*/
				// Finds the location with the lowest distance == highest volume
				int v = calculate_volume(locx, locy, disp);
				if(v < distance_volume)
					distance_volume = v;
	/*		}*/
		}

		if(!sound::is_sound_playing(last_id))
			sound::play_sound_positioned(_files, last_id, distance_volume);
	}
}

void manager::positional_source::update_positions(unsigned int time, const display &disp)
{
	const bool was_visible = _visible;
	SDL_Rect area = disp.map_outside_area();

	_visible = false;

	for(std::list<gamemap::location>::iterator i =  _locations.begin(); i != _locations.end(); ++i) {
		int locx = disp.get_location_x(*i);
		int locy = disp.get_location_y(*i);

		if(disp.outside_area(area, locx, locy) || disp.shrouded(*i)
			|| (!_play_fogged && disp.fogged(*i)))
				continue;
		else {
			_visible = true;
			if(!sound::is_sound_playing(last_id)) {
				if(!was_visible)
					_last_played = 0;	// hack make the previously invisible source to play

				update(time, disp);
				continue;
			}

			sound::reposition_sound(last_id, calculate_volume(locx, locy, disp));
		}
	}

	if(!_visible)
		sound::stop_sound(last_id);
}

void manager::positional_source::add_location(const gamemap::location &loc)
{
	_locations.push_back(loc);
}

} // namespace soundsource

