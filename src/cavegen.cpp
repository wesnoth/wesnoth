/* $Id$ */
/*
   Copyright (C) 2003 - 2007 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file cavegen.cpp 
//! Map-generator for caves.

#include "global.hpp"

#include "cavegen.hpp"
#include "log.hpp"
#include "pathfind.hpp"
#include "util.hpp"
#include "serialization/string_utils.hpp"

#include <cassert>

#define LOG_NG LOG_STREAM(info, engine)

cave_map_generator::cave_map_generator(const config* cfg) : wall_(t_translation::CAVE_WALL),
	clear_(t_translation::CAVE), village_(t_translation::UNDERGROUND_VILLAGE),
    castle_(t_translation::DWARVEN_CASTLE), keep_(t_translation::DWARVEN_KEEP),
	cfg_(cfg), width_(50), height_(50),
	village_density_(0), flipx_(false), flipy_(false)
{
	if(cfg_ == NULL) {
		static const config default_cfg;
		cfg_ = &default_cfg;
	}

	width_ = atoi((*cfg_)["map_width"].c_str());
	height_ = atoi((*cfg_)["map_height"].c_str());
	width_ += 2 * gamemap::default_border;
	height_ += 2 * gamemap::default_border;

	village_density_ = atoi((*cfg_)["village_density"].c_str());

	const int r = rand()%100;
	const int chance = atoi((*cfg_)["flipx_chance"].c_str());

	flipx_ = r < chance;

	LOG_NG << "flipx: " << r << " < " << chance << " = " << (flipx_ ? "true" : "false") << "\n";
	flipy_ = (rand()%100) < atoi((*cfg_)["flipy_chance"].c_str());

}

size_t cave_map_generator::translate_x(size_t x) const
{
	if(flipx_) {
		x = width_ - x - 1;
	}

	return x;
}

size_t cave_map_generator::translate_y(size_t y) const
{
	if(flipy_) {
		y = height_ - y - 1;
	}

	return y;
}

std::string cave_map_generator::create_map(const std::vector<std::string>& args)
{
	const config res = create_scenario(args);
	return res["map_data"];
}

config cave_map_generator::create_scenario(const std::vector<std::string>& /*args*/)
{
	map_ = t_translation::t_map(width_, t_translation::t_list(height_, wall_));
	chambers_.clear();
	passages_.clear();

	res_.clear();
	const config* const settings = cfg_->child("settings");
	if(settings != NULL) {
		res_ = *settings;
	}

	LOG_NG << "creating scenario....\n";
	generate_chambers();

	LOG_NG << "placing chambers...\n";
	for(std::vector<chamber>::const_iterator c = chambers_.begin(); c != chambers_.end(); ++c) {
		place_chamber(*c);
	}

	LOG_NG << "placing passages...\n";

	for(std::vector<passage>::const_iterator p = passages_.begin(); p != passages_.end(); ++p) {
		place_passage(*p);
	}

	LOG_NG << "outputting map....\n";

	res_["map_data"] = gamemap::default_map_header + 
		t_translation::write_game_map(map_, starting_positions_);

	LOG_NG << "returning result...\n";

	return res_;
}

void cave_map_generator::build_chamber(gamemap::location loc, std::set<gamemap::location>& locs, size_t size, size_t jagged)
{
	if(size == 0 || locs.count(loc) != 0 || !on_board(loc))
		return;

	locs.insert(loc);

	gamemap::location adj[6];
	get_adjacent_tiles(loc,adj);
	for(size_t n = 0; n != 6; ++n) {
		if((rand() % 100) < (100l - static_cast<long>(jagged))) {
			build_chamber(adj[n],locs,size-1,jagged);
		}
	}
}

void cave_map_generator::generate_chambers()
{
	const config::child_list& chambers = cfg_->get_children("chamber");
	for(config::child_list::const_iterator i = chambers.begin(); i != chambers.end(); ++i) {
		// If there is only a chance of the chamber appearing, deal with that here.
		const std::string& chance = (**i)["chance"];
		if(chance != "" && (rand()%100) < atoi(chance.c_str())) {
			continue;
		}

		const std::string& xpos = (**i)["x"];
		const std::string& ypos = (**i)["y"];

		size_t min_xpos = 0, min_ypos = 0, max_xpos = width_, max_ypos = height_;

		if(xpos != "") {
			const std::vector<std::string>& items = utils::split(xpos, '-');
			if(items.empty() == false) {
				min_xpos = atoi(items.front().c_str()) - 1;
				max_xpos = atoi(items.back().c_str());
			}
		}

		if(ypos != "") {
			const std::vector<std::string>& items = utils::split(ypos, '-');
			if(items.empty() == false) {
				min_ypos = atoi(items.front().c_str()) - 1;
				max_ypos = atoi(items.back().c_str());
			}
		}

		const size_t x = translate_x(min_xpos + (rand()%(max_xpos-min_xpos)));
		const size_t y = translate_y(min_ypos + (rand()%(max_ypos-min_ypos)));

		const std::string& size = (**i)["size"];
		size_t chamber_size = 3;
		if(size != "") {
			chamber_size = atoi(size.c_str());
		}

		const std::string& jagged = (**i)["jagged"];
		size_t jagged_edges = 0;
		if(jagged != "") {
			jagged_edges = atoi(jagged.c_str());
		}

		chamber new_chamber;
		new_chamber.center = gamemap::location(x,y);
		build_chamber(new_chamber.center,new_chamber.locs,chamber_size,jagged_edges);

		new_chamber.items = (**i).child("items");

		const std::string& id = (**i)["id"];
		if(id != "") {
			chamber_ids_[id] = chambers_.size();
		}

		chambers_.push_back(new_chamber);

		const config::child_list& passages = (**i).get_children("passage");
		for(config::child_list::const_iterator p = passages.begin(); p != passages.end(); ++p) {
			const std::string& dst = (**p)["destination"];

			// Find the destination of this passage
			const std::map<std::string,size_t>::const_iterator itor = chamber_ids_.find(dst);
			if(itor == chamber_ids_.end())
				continue;

			assert(itor->second < chambers_.size());

			passages_.push_back(passage(new_chamber.center,chambers_[itor->second].center,**p));
		}
	}
}

void cave_map_generator::place_chamber(const chamber& c)
{
	for(std::set<gamemap::location>::const_iterator i = c.locs.begin(); i != c.locs.end(); ++i) {
		set_terrain(*i,clear_);
	}

	if(c.items != NULL) {
		place_items(c,c.items->ordered_begin(),c.items->ordered_end());
	}
}

void cave_map_generator::place_items(const chamber& c, config::all_children_iterator i1, config::all_children_iterator i2)
{
	if(c.locs.empty()) {
		return;
	}

	size_t index = 0;
	while(i1 != i2) {
		const std::string& key = *(*i1).first;
		config cfg = *(*i1).second;
		config* const filter = cfg.child("filter");
		config* const object = cfg.child("object");
		config* object_filter = NULL;
		if(object != NULL) {
			object_filter = object->child("filter");
		}

		if(!utils::string_bool(cfg["same_location_as_previous"])) {
			index = rand()%c.locs.size();
		}
		const std::string loc_var = cfg["store_location_as"];

		std::set<gamemap::location>::const_iterator loc = c.locs.begin();
		std::advance(loc,index);

		char xbuf[50];
		snprintf(xbuf,sizeof(xbuf),"%d",loc->x+1);
		cfg.values["x"] = xbuf;
		if(filter != NULL) {
			(*filter)["x"] = xbuf;
		}

		if(object_filter != NULL) {
			(*object_filter)["x"] = xbuf;
		}

		char ybuf[50];
		snprintf(ybuf,sizeof(ybuf),"%d",loc->y+1);
		cfg.values["y"] = ybuf;
		if(filter != NULL) {
			(*filter)["y"] = ybuf;
		}

		if(object_filter != NULL) {
			(*object_filter)["y"] = ybuf;
		}

		// If this is a side, place a castle for the side
		if(key == "side" && !utils::string_bool(cfg["no_castle"])) {
			place_castle(cfg["side"],*loc);
		}

		res_.add_child(key,cfg);

		if(!loc_var.empty()) {
			config &temp = res_.add_child("event");
			temp["name"] = "prestart";
			config &xcfg = temp.add_child("set_variable");
			xcfg["name"] = loc_var + "_x";
			xcfg["value"] = xbuf;
			config &ycfg = temp.add_child("set_variable");
			ycfg["name"] = loc_var + "_y";
			ycfg["value"] = ybuf;
		}

		++i1;
	}
}

struct passage_path_calculator : cost_calculator
{
	passage_path_calculator(const t_translation::t_map& mapdata,
	t_translation::t_terrain wall, double laziness, size_t windiness):
		map_(mapdata), wall_(wall), laziness_(laziness), windiness_(windiness)
	{}

	virtual double cost(const gamemap::location& src,const gamemap::location& loc, const double so_far, const bool is_dest) const;
private:
	const t_translation::t_map& map_;
	t_translation::t_terrain wall_;
	double laziness_;
	size_t windiness_;
};

double passage_path_calculator::cost(const gamemap::location& /*src*/,const gamemap::location& loc, const double, const bool) const
{
	assert(loc.x >= 0 && loc.y >= 0 && size_t(loc.x) < map_.size() &&
	        !map_.empty() && size_t(loc.y) < map_.front().size());

	double res = 1.0;
	if(map_[loc.x][loc.y] == wall_) {
		res = laziness_;
	}

	if(windiness_ > 1) {
		res *= double(rand()%windiness_);
	}

	return res;
}

void cave_map_generator::place_passage(const passage& p)
{
	const std::string& chance = p.cfg["chance"];
	if(chance != "" && (rand()%100) < atoi(chance.c_str())) {
		return;
	}


	const size_t windiness = atoi(p.cfg["windiness"].c_str());
	const double laziness = maximum<double>(1.0,atof(p.cfg["laziness"].c_str()));

	passage_path_calculator calc(map_,wall_,laziness,windiness);

	const paths::route rt = a_star_search(p.src, p.dst, 10000.0, &calc, width_, height_);

	const size_t width = maximum<size_t>(1,atoi(p.cfg["width"].c_str()));

	const size_t jagged = atoi(p.cfg["jagged"].c_str());

	for(std::vector<gamemap::location>::const_iterator i = rt.steps.begin(); i != rt.steps.end(); ++i) {
		std::set<gamemap::location> locs;
		build_chamber(*i,locs,width,jagged);
		for(std::set<gamemap::location>::const_iterator j = locs.begin(); j != locs.end(); ++j) {
			set_terrain(*j,clear_);
		}
	}
}

void cave_map_generator::set_terrain(gamemap::location loc, t_translation::t_terrain t)
{
	if(on_board(loc)) {
		if(t == clear_ &&
                (rand() % 1000) < static_cast<long>(village_density_)) {

			t = village_;
		}

		t_translation::t_terrain& c = map_[loc.x][loc.y];
		if(c == clear_ || c == wall_ || c == village_) {
			c = t;
		}
	}
}

void cave_map_generator::place_castle(const std::string& side, gamemap::location loc)
{
	const int starting_position = lexical_cast_default<int>(side, -1);
	if(starting_position != -1) {
		set_terrain(loc, keep_);

		const struct t_translation::coordinate coord = {loc.x, loc.y};
		starting_positions_[starting_position] = coord;
	}

	gamemap::location adj[6];
	get_adjacent_tiles(loc,adj);
	for(size_t n = 0; n != 6; ++n) {
		set_terrain(adj[n],castle_);
	}
}

