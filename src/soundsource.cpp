/* $Id$ */
/*
   Copyright (C) 2006 - 2011 by Karol Nowak <grzywacz@sul.uni.lodz.pl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"


#include "display.hpp"
#include "foreach.hpp"
#include "log.hpp"
#include "serialization/string_utils.hpp"
#include "sound.hpp"
#include "soundsource.hpp"

namespace soundsource {

const unsigned DEFAULT_CHANCE           = 100;
const unsigned DEFAULT_DELAY            = 1000;
const unsigned DEFAULT_FULL_RANGE       = 3;
const unsigned DEFAULT_FADE_RANGE       = 14;

unsigned int positional_source::last_id = 0;

manager::manager(const display &disp) :
	observer(),
	savegame_config(),
	sources_(),
	disp_(disp)
{
	disp_.scroll_event().attach_handler(this);
	update_positions();
}

manager::~manager()
{
	for(positional_source_iterator it = sources_.begin(); it != sources_.end(); ++it) {
		delete (*it).second;
	}

	sources_.clear();
}

void manager::handle_generic_event(const std::string &event_name)
{
	if(event_name == "scrolled")
		update_positions();
}

void manager::add(const sourcespec &spec)
{
	positional_source_iterator it;

	if((it = sources_.find(spec.id())) == sources_.end()) {
		sources_[spec.id()] = new positional_source(spec);
	} else {
		delete (*it).second;
		(*it).second = new positional_source(spec);
	}
}

void manager::remove(const std::string &id)
{
	positional_source_iterator it;

	if((it = sources_.find(id)) == sources_.end())
		return;
	else {
		delete (*it).second;
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

config manager::to_config() const
{
	config cfg;
	write_sourcespecs(cfg);
	return cfg.child("sound_source");
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
	if(time - last_played_ < min_delay_ || sound::is_sound_playing(id_))
		return;

	unsigned int i = rand() % 100 + 1;

	if(i <= chance_) {
		last_played_ = time;

		// If no locations have been specified, treat the source as if
		// it was present everywhere on the map
		if(locations_.empty()) {
			sound::play_sound_positioned(files_, id_, loops_, 0);	// max volume
			return;
		}

		int distance_volume = DISTANCE_SILENT;
		for(std::vector<map_location>::iterator i = locations_.begin(); i != locations_.end(); ++i) {
			int v = calculate_volume(*i, disp);
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
	size_t distance = distance_between(loc, center);

	if(distance <= range_) {
		return 0;
	}

	return static_cast<int>((((distance - range_)
			/ static_cast<double>(faderange_)) * DISTANCE_SILENT));
}

void positional_source::write_config(config& cfg) const
{
	cfg["sounds"] = this->files_;
	cfg["delay"] = str_cast<unsigned int>(this->min_delay_);
	cfg["chance"] = str_cast<unsigned int>(this->chance_);
	cfg["check_fogged"] = this->check_fogged_ ? "yes" : "no";
	cfg["check_shrouded"] = this->check_shrouded_ ? "yes" : "no";

	cfg["x"] = cfg["y"] = "";
	bool first_loc = true;
	BOOST_FOREACH(const map_location& loc, locations_) {
		if(!first_loc) {
			cfg["x"] += ",";
			cfg["y"] += ",";
		} else {
			first_loc = false;
		}
		cfg["x"] += str_cast<unsigned int>(loc.x);
		cfg["y"] += str_cast<unsigned int>(loc.y);
	}

	cfg["loop"] = str_cast<unsigned int>(this->loops_);
	cfg["full_range"] = str_cast<unsigned int>(this->range_);
	cfg["fade_range"] = str_cast<unsigned int>(this->faderange_);
}

sourcespec::sourcespec(const config& cfg) :
	id_(cfg["id"]),
	files_(cfg["sounds"]),
	min_delay_(lexical_cast_default<int>(cfg["delay"], DEFAULT_DELAY)),
	chance_(lexical_cast_default<int>(cfg["chance"], DEFAULT_CHANCE)),
	loops_(lexical_cast_default<int>(cfg["loop"], 0)),
	range_(lexical_cast_default<int>(cfg["full_range"], 3)),
	faderange_(lexical_cast_default<int>(cfg["fade_range"], 14)),
	check_fogged_(utils::string_bool(cfg["check_fogged"], true)),
	check_shrouded_(utils::string_bool(cfg["check_shrouded"], true)),
	locations_()
{
	const std::vector<std::string>& vx = utils::split(cfg["x"]);
	const std::vector<std::string>& vy = utils::split(cfg["y"]);

	if(vx.empty() || vy.empty()) {
		lg::wml_error << "missing sound source locations";
	}

	if(vx.size() != vy.size()) {
		lg::wml_error << "mismatched number of sound source location coordinates";
	}
	else {
		for(unsigned int i = 0; i < std::min(vx.size(), vy.size()); ++i) {
			try {
				map_location loc(lexical_cast<int>(vx[i]), lexical_cast<int>(vy[i]));
				locations_.push_back(loc);
			} catch(bad_lexical_cast&) {
				lg::wml_error << "non-numerical coordinates for soundsource (" << vx[i] << ',' << vy[i] << ')';
			}
		}
	}
}

} // namespace soundsource

