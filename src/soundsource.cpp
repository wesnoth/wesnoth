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

#include "global.hpp"

#include <cassert>
#include <cstdlib>

#include "display.hpp"
#include "pathutils.hpp"
#include "sound.hpp"
#include "soundsource.hpp"


namespace soundsource {

unsigned int positional_source::last_id = 0;

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

void manager::add(const sourcespec &spec)
{
	positional_source_iterator it;

	if((it = _sources.find(spec.id)) == _sources.end()) {
		_sources[spec.id] = new positional_source(spec);
	} else {
		delete (*it).second;
		(*it).second = new positional_source(spec);
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

	for(positional_source_iterator it = _sources.begin(); it != _sources.end(); ++it) {
		(*it).second->update(time, _disp);
	}
}

void manager::update_positions()
{
	unsigned int time = SDL_GetTicks();

	for(positional_source_iterator it = _sources.begin(); it != _sources.end(); ++it) {
		(*it).second->update_positions(time, _disp);
	}
}

void manager::add_location(const std::string &id, const gamemap::location &loc)
{
	positional_source_iterator it = _sources.find(id);

	if(it == _sources.end())
		return;
	else
		(*it).second->add_location(loc);
}


positional_source::positional_source(const sourcespec &spec) 
				: _last_played(0), _min_delay(spec.min_delay), _chance(spec.chance), _loops(spec.loops),
					_id(last_id++), _range(spec.range), _faderange(spec.faderange), 
					_check_fogged(spec.check_fogged), _files(spec.files), _locations(spec.locations)
{
	assert(_range > 0);
	assert(_faderange > 0);
}

positional_source::~positional_source()
{
	sound::reposition_sound(_id, DISTANCE_SILENT);
}

void positional_source::update(unsigned int time, const display &disp)
{
	if(time - _last_played < _min_delay || sound::is_sound_playing(_id))
		return;

	unsigned int i = rand() % 100 + 1;

	if(i <= _chance) {
		_last_played = time;

		// If no locations have been specified, treat the source as if
		// it was present everywhere on the map
		if(_locations.size() == 0) {
			sound::play_sound_positioned(_files, _id, _loops, 0);	// max volume
			return;
		}

		int distance_volume = DISTANCE_SILENT;
		for(std::vector<gamemap::location>::iterator i = _locations.begin(); i != _locations.end(); ++i) {
			int v = calculate_volume(*i, disp);
			if(v < distance_volume) {
				distance_volume = v;
			}
		}

		if(distance_volume >= DISTANCE_SILENT)
			return;

		sound::play_sound_positioned(_files, _id, _loops, distance_volume);
	}
}

void positional_source::update_positions(unsigned int time, const display &disp)
{
	int distance_volume = DISTANCE_SILENT;
	for(std::vector<gamemap::location>::iterator i = _locations.begin(); i != _locations.end(); ++i) {
		if(disp.shrouded(*i) || (_check_fogged && disp.fogged(*i)))
			continue;

		int v = calculate_volume(*i, disp);
		if(v < distance_volume) {
			distance_volume = v;
		}
	}

	if(sound::is_sound_playing(_id)) {
		sound::reposition_sound(_id, distance_volume);
	} else {
		update(time, disp);
	}
}

int positional_source::calculate_volume(const gamemap::location &loc, const display &disp)
{
	assert(_range > 0);
	assert(_faderange > 0);

	SDL_Rect area = disp.map_area();
	gamemap::location center = disp.hex_clicked_on(area.x + area.w / 2, area.y + area.h / 2);
	int distance = distance_between(loc, center);

	if(distance <= _range) {
		return 0;
	}

	return static_cast<int>(( ( (distance - _range) / (double) _faderange) * DISTANCE_SILENT));
}

void positional_source::add_location(const gamemap::location &loc)
{
	_locations.push_back(loc);
}

} // namespace soundsource

