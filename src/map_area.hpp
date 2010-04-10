/* $Id: map_label.hpp 41350 2010-02-22 06:14:34Z fendrin $ */
/*
   Copyright (C) 2003 - 2010 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef MAP_AREA_HPP_INCLUDED
#define MAP_AREA_HPP_INCLUDED

#include "savegame_config.hpp"
#include "map_location.hpp"
#include "config.hpp"

#include <map>
#include <set>
#include <string>

class config;
class map_area;

class map_areas : public savegame::savegame_config
{
public:
	map_areas() : areas_() {}
	map_areas(const config& cfg) : areas_() { read(cfg); }
	~map_areas();

	config to_config() const;

	void read(const config &cfg);
	void write(config& res) const;

	void add_area(const map_area& area);
	void add_area(const std::string& area_id, const std::set<map_location>& locations);

	const map_area& get_area(const std::string& area_id) const;
	const std::vector<std::string> get_area_ids() const;

	void clear();

//	void operator=(const labels&);

private:
	typedef std::map<std::string, map_area> area_map;
	area_map areas_;
};

class map_area : public savegame::savegame_config
{
public:
//	map_area(const std::string& area_id,
//			 const std::set<map_location>& locations);

	map_area() : area_id_(), slf_() {};

//	void operator=(const map_area&);

	map_area(const config& cfg);

	map_area(const map_area& area);

	map_area(std::string area_id, std::set<map_location> locs);

	~map_area();

	config to_config() const;

	void write(config& res) const;
	void read(const config &cfg);

	const std::string& get_area_id() { return area_id_; };
	const config& get_slf() { return slf_; };

	const std::set<map_location> get_simple_locations();

private:
//TODO make the privates const
	std::string area_id_;
	config slf_;
};

#endif
