/*
   Copyright (C) 2006 - 2017 by Karol Nowak <grzywacz@sul.uni.lodz.pl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "display.hpp"
#include "log.hpp"
#include "serialization/string_utils.hpp"
#include "sound.hpp"
#include "soundsource.hpp"

#include <SDL.h> // Travis doesn't like this, although it works on my machine -> '#include <SDL_sound.h>

namespace soundsource {

const unsigned DEFAULT_CHANCE           = 100;
const unsigned DEFAULT_DELAY            = 1000;

unsigned int positional_source::last_id = 0;

manager::manager(const display &disp) :
	observer(),
	sources_(),
	disp_(disp)
{
	disp_.scroll_event().attach_handler(this);
	update_positions();
}

manager::~manager()
{
	sources_.clear();
}

void manager::handle_generic_event(const std::string &event_name)
{
	if(event_name == "scrolled")
		update_positions();
}

void manager::add(const sourcespec &spec)
{
	sources_[spec.id()].reset(new positional_source(spec));
}

config manager::get(const std::string &id)
{
	config cfg;
	positional_source_iterator it = sources_.find(id);
	if(it != sources_.end()) {
		it->second->write_config(cfg);
	}
	return cfg;
}

void manager::remove(const std::string &id)
{
	positional_source_iterator it;

	if((it = sources_.find(id)) == sources_.end())
		return;
	else {
		sources_.erase(it);
	}
}

void manager::update()
{
	unsigned int time = SDL_GetTicks();

	for(positional_source_iterator it = sources_.begin(); it != sources_.end(); ++it) {
		(*it).second->update(time, disp_);
	}
}

void manager::update_positions()
{
	unsigned int time = SDL_GetTicks();

	for(positional_source_iterator it = sources_.begin(); it != sources_.end(); ++it) {
		(*it).second->update_positions(time, disp_);
	}
}

void manager::write_sourcespecs(config& cfg) const
{
	for(positional_source_const_iterator i = sources_.begin(); i != sources_.end(); ++i) {
		assert(i->second);

		config& child = cfg.add_child("sound_source");
		child["id"] = i->first;
		i->second->write_config(child);
	}
}

positional_source::positional_source(const sourcespec &spec) :
	last_played_(0),
	min_delay_(spec.minimum_delay()),
	chance_(spec.chance()),
	loops_(spec.loops()),
	id_(last_id++),
	range_(spec.full_range()),
	faderange_(spec.fade_range()),
	check_fogged_(spec.check_fogged()),
	check_shrouded_(spec.check_shrouded()),
	files_(spec.files()),
	locations_(spec.get_locations())
{
	assert(range_ > 0);
	assert(faderange_ > 0);
}

positional_source::~positional_source()
{
	sound::reposition_sound(id_, DISTANCE_SILENT);
}

bool positional_source::is_global()
{
	return locations_.empty();
}

void positional_source::update(unsigned int time, const display &disp)
{
	if (time - last_played_ < unsigned(min_delay_) || sound::is_sound_playing(id_))
		return;

	int i = rand() % 100 + 1;

	if(i <= chance_) {
		last_played_ = time;

		// If no locations have been specified, treat the source as if
		// it was present everywhere on the map
		if(locations_.empty()) {
			sound::play_sound_positioned(files_, id_, loops_, 0);	// max volume
			return;
		}

		int distance_volume = DISTANCE_SILENT;
		for(const map_location& l : locations_) {
			int v = calculate_volume(l, disp);
			if(v < distance_volume) {
				distance_volume = v;
			}
		}

		if(distance_volume >= DISTANCE_SILENT)
			return;

		sound::play_sound_positioned(files_, id_, loops_, distance_volume);
	}
}

void positional_source::update_positions(unsigned int time, const display &disp)
{
	if(is_global()) {
		return;
	}

	int distance_volume = DISTANCE_SILENT;
	for(std::vector<map_location>::iterator i = locations_.begin(); i != locations_.end(); ++i) {
		int v = calculate_volume(*i, disp);
		if(v < distance_volume) {
			distance_volume = v;
		}
	}

	if(sound::is_sound_playing(id_)) {
		sound::reposition_sound(id_, distance_volume);
	} else {
		update(time, disp);
	}
}

int positional_source::calculate_volume(const map_location &loc, const display &disp)
{
	assert(range_ > 0);
	assert(faderange_ > 0);

	if((check_shrouded_ && disp.shrouded(loc)) || (check_fogged_ && disp.fogged(loc)))
		return DISTANCE_SILENT;

	SDL_Rect area = disp.map_area();
	map_location center = disp.hex_clicked_on(area.x + area.w / 2, area.y + area.h / 2);
	int distance = distance_between(loc, center);

	if(distance <= range_) {
		return 0;
	}

	return static_cast<int>((((distance - range_)
			/ static_cast<double>(faderange_)) * DISTANCE_SILENT));
}

void positional_source::write_config(config& cfg) const
{
	cfg["sounds"] = files_;
	cfg["delay"] = min_delay_;
	cfg["chance"] = chance_;
	cfg["check_fogged"] = check_fogged_;
	cfg["check_shrouded"] = check_shrouded_;
	cfg["loop"] = loops_;
	cfg["full_range"] = range_;
	cfg["fade_range"] = faderange_;
	write_locations(locations_, cfg);
}

sourcespec::sourcespec(const config& cfg) :
	id_(cfg["id"]),
	files_(cfg["sounds"]),
	min_delay_(cfg["delay"].to_int(DEFAULT_DELAY)),
	chance_(cfg["chance"].to_int(DEFAULT_CHANCE)),
	loops_(cfg["loop"]),
	range_(cfg["full_range"].to_int(3)),
	faderange_(cfg["fade_range"].to_int(14)),
	check_fogged_(cfg["check_fogged"].to_bool(true)),
	check_shrouded_(cfg["check_shrouded"].to_bool(true)),
	locations_()
{
	read_locations(cfg, locations_);
}

} // namespace soundsource

