/* $Id: map_label.cpp 41350 2010-02-22 06:14:34Z fendrin $ */
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

#include "global.hpp"

#include "foreach.hpp"
#include "map_area.hpp"
#include "serialization/string_utils.hpp"

//map_areas::map_areas(const config& cfg)
//{
//	read(cfg);
//}

//map_areas::map_areas()
//{
//
//}

map_areas::~map_areas()
{

}

void map_areas::read(const config &cfg)
{
//	clear_all();

	foreach (const config &i, cfg.child_range("named_area"))
	{
		map_area* area = new map_area(i);
		add_area(*area);
	}
}

//TODO think about the const
void map_areas::write(config& res) const
{
	for (area_map::const_iterator area_it = areas_.begin(); area_it != areas_.end(); ++area_it)
	{
		area_it->second.write(res);
	}
}

config map_areas::to_config() const
{
	config res;
	write(res);
	return res;
}


void map_areas::add_area(const map_area& area)
{
	map_area area_ = area;
	areas_[area_.get_area_id()] = area_;
}

void map_areas::add_area(const std::string& area_id, const std::set<map_location>& locations)
{
	map_area* area = new map_area(area_id, locations);
	add_area(*area);
}

const map_area& map_areas::get_area(const std::string& area_id) const
{
	const map_area* res = new map_area();
	area_map::const_iterator areas_it = areas_.find(area_id);
	if (areas_it != areas_.end()) {
		res = &(areas_it->second);
	}
	return *res;
}

const std::vector<std::string> map_areas::get_area_ids() const
{
	std::vector<std::string> res;
	for(area_map::const_iterator it = areas_.begin(); it != areas_.end(); ++it) {
	  res.push_back(it->first);
//	  cout << it->first << "\n";
	}
	return res;
}

void map_areas::clear()
{
	areas_.clear();
}

//TODO reuse code to implement map transformations
//void map_labels::scroll(double xmove, double ymove)
//{
//	for(team_label_map::const_iterator i = labels_.begin(); i != labels_.end(); ++i)
//	{
//		for (label_map::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
//		{
//			j->second->scroll(xmove, ymove);
//		}
//	}
//}

map_area::map_area(const config& cfg) : slf_(cfg)
{
	area_id_ = cfg["area_id"];
}

map_area::map_area(const map_area& area) : area_id_(area.area_id_), slf_(area.slf_)
{
}

map_area::map_area(std::string area_id, std::set<map_location> locs)
	: area_id_(area_id), slf_()
{
	std::stringstream ssx, ssy;
	std::set<map_location>::const_iterator i = locs.begin();
	if (i != locs.end()) {
//		ssx << "x = " << i->x + 1;
//		ssy << "y = " << i->y + 1;
		ssx << i->x + 1;
		ssy << i->y + 1;
		++i;
		while (i != locs.end()) {
			ssx << ", " << i->x + 1;
			ssy << ", " << i->y + 1;
			++i;
		}
		slf_["x"] = ssx.str();
		slf_["y"] = ssy.str();
//		ssx << "\n" << ssy.str() << "\n";
//		copy_to_clipboard(ssx.str(), false);
	}
}

//void map_area::operator=(const map_area&)
//{
////	this->
//}


map_area::~map_area()
{

}

config map_area::to_config() const
{
	config res;
	write(res);
	return res;
}

void map_area::write(config& res) const
{
	config item = slf_;
	item["area_id"] = area_id_;
	res.add_child("named_area", item);
}

void map_area::read(const config &cfg)
{
	area_id_ = cfg["area_id"];
	slf_ = cfg;
}

const std::set<map_location> map_area::get_simple_locations()
{
	std::string x = slf_["x"];
	std::string y = slf_["y"];
	const std::vector<std::string> xvals = utils::split(x);
	const std::vector<std::string> yvals = utils::split(y);
	std::set<map_location> locations;
	map_location loc;
	for(size_t i = 0; i != std::min(xvals.size(),yvals.size()); ++i) {
		//		if(i==0){
		loc.x = atoi(xvals[i].c_str())-1;
		loc.y = atoi(yvals[i].c_str())-1;
		//			if (!game_map->on_board(src)) {
		//				ERR_CF << "invalid move_unit_fake source: " << src << '\n';
		//				break;
		//			}
		locations.insert(loc);
	}
	return locations;
}

