/*
	Copyright (C) 2003 - 2024
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

/**
 * @file
 * Routines related to game-maps, terrain, locations, directions. etc.
 */

#include "map/map.hpp"

#include "config.hpp"
#include "game_config_manager.hpp"
#include "log.hpp"
#include "map/exception.hpp"
#include "serialization/parser.hpp"
#include "serialization/string_utils.hpp"
#include "terrain/terrain.hpp"
#include "terrain/type_data.hpp"
#include "utils/general.hpp"
#include "wml_exception.hpp"

#include <algorithm>
#include <sstream>
#include <utility>

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)
#define LOG_G LOG_STREAM(info, lg::general())
#define DBG_G LOG_STREAM(debug, lg::general())

/** Gets the list of terrains. */
const t_translation::ter_list& gamemap::get_terrain_list() const
{
	return tdata_->list();
}

/** Shortcut to get_terrain_info(get_terrain(loc)). */
const terrain_type& gamemap::get_terrain_info(const map_location &loc) const
{
	return tdata_->get_terrain_info(get_terrain(loc));
}

const t_translation::ter_list& gamemap::underlying_mvt_terrain(const map_location& loc) const
	{ return underlying_mvt_terrain(get_terrain(loc)); }
const t_translation::ter_list& gamemap::underlying_def_terrain(const map_location& loc) const
	{ return underlying_def_terrain(get_terrain(loc)); }
const t_translation::ter_list& gamemap::underlying_union_terrain(const map_location& loc) const
	{ return underlying_union_terrain(get_terrain(loc)); }
std::string gamemap::get_terrain_string(const map_location& loc) const
	{ return get_terrain_string(get_terrain(loc)); }
std::string gamemap::get_terrain_editor_string(const map_location& loc) const
	{ return get_terrain_editor_string(get_terrain(loc)); }

bool gamemap::is_village(const map_location& loc) const
	{ return on_board(loc) && is_village(get_terrain(loc)); }
int gamemap::gives_healing(const map_location& loc) const
	{ return on_board(loc) ?  gives_healing(get_terrain(loc)) : 0; }
bool gamemap::is_castle(const map_location& loc) const
	{ return on_board(loc) && is_castle(get_terrain(loc)); }
bool gamemap::is_keep(const map_location& loc) const
	{ return on_board(loc) && is_keep(get_terrain(loc)); }


/* Forwarded methods of tdata_ */
const t_translation::ter_list& gamemap::underlying_mvt_terrain(const t_translation::terrain_code & terrain) const
	{ return tdata_->underlying_mvt_terrain(terrain); }
const t_translation::ter_list& gamemap::underlying_def_terrain(const t_translation::terrain_code & terrain) const
	{ return tdata_->underlying_def_terrain(terrain); }
const t_translation::ter_list& gamemap::underlying_union_terrain(const t_translation::terrain_code & terrain) const
	{ return tdata_->underlying_union_terrain(terrain); }
std::string gamemap::get_terrain_string(const t_translation::terrain_code & terrain) const
	{ return tdata_->get_terrain_string(terrain); }
std::string gamemap::get_terrain_editor_string(const t_translation::terrain_code & terrain) const
	{ return tdata_->get_terrain_editor_string(terrain); }
std::string gamemap::get_underlying_terrain_string(const t_translation::terrain_code& terrain) const
	{ return tdata_->get_underlying_terrain_string(terrain); }
bool gamemap::is_village(const t_translation::terrain_code & terrain) const
	{ return tdata_->get_terrain_info(terrain).is_village(); }
int gamemap::gives_healing(const t_translation::terrain_code & terrain) const
	{ return tdata_->get_terrain_info(terrain).gives_healing(); }
bool gamemap::is_castle(const t_translation::terrain_code & terrain) const
	{ return tdata_->get_terrain_info(terrain).is_castle(); }
bool gamemap::is_keep(const t_translation::terrain_code & terrain) const
	{ return tdata_->get_terrain_info(terrain).is_keep(); }

const terrain_type& gamemap::get_terrain_info(const t_translation::terrain_code & terrain) const
	{ return tdata_->get_terrain_info(terrain); }

void gamemap::write_terrain(const map_location &loc, config& cfg) const
{
	cfg["terrain"] = t_translation::write_terrain_code(get_terrain(loc));
}

gamemap::gamemap(const std::string& data):
		gamemap_base(1, 1),
		tdata_(),
		villages_()
{
	if(const auto* gcm = game_config_manager::get()) {
		tdata_ = gcm->terrain_types();
	} else {
		// Should only be encountered in unit tests
		tdata_ = std::make_shared<terrain_type_data>(game_config_view::wrap({}));
	}

	DBG_G << "loading map: '" << data << "'";
	read(data);
}

gamemap_base::gamemap_base(int w, int h, terrain_code t)
	: tiles_(w, h, t)
	, starting_positions_()
{

}

gamemap_base::~gamemap_base()
{
}

void gamemap::read(const std::string& data, const bool allow_invalid)
{
	tiles() = t_translation::ter_map();
	villages_.clear();
	special_locations().clear();

	if(data.empty()) {
		if(allow_invalid) return;
	}

	int offset = read_header(data);

	const std::string& data_only = std::string(data, offset);

	try {
		tiles() = t_translation::read_game_map(data_only, special_locations(), t_translation::coordinate{ border_size(), border_size() });

	} catch(const t_translation::error& e) {
		// We re-throw the error but as map error.
		// Since all codepaths test for this, it's the least work.
		throw incorrect_map_format_error(e.message);
	}

	// Post processing on the map
	VALIDATE((total_width() >= 1 && total_height() >= 1), "A map needs at least 1 tile, the map cannot be loaded.");

	for(int x = 0; x < total_width(); ++x) {
		for(int y = 0; y < total_height(); ++y) {

			// Is the terrain valid?
			t_translation::terrain_code t = tiles().get(x, y);
			if(tdata_->map().count(t) == 0) {
				if(!tdata_->is_known(t)) {
					std::stringstream ss;
					ss << "Unknown tile in map: (" << t_translation::write_terrain_code(t)
						   << ") '" << t << "'";
					throw incorrect_map_format_error(ss.str().c_str());
				}
			}

			// Is it a village?
			if(x >= border_size() && y >= border_size()
					&& x < total_width()- border_size() && y < total_height()- border_size()
					&& tdata_->is_village(tiles().get(x, y))) {
				villages_.push_back(map_location(x - border_size(), y - border_size()));
			}
		}
	}
}

int gamemap::read_header(const std::string& data)
{
	// Test whether there is a header section
	std::size_t header_offset = data.find("\n\n");
	if(header_offset == std::string::npos) {
		// For some reason Windows will fail to load a file with \r\n
		// lineending properly no problems on Linux with those files.
		// This workaround fixes the problem the copy later will copy
		// the second \r\n to the map, but that's no problem.
		header_offset = data.find("\r\n\r\n");
	}
	const std::size_t comma_offset = data.find(",");
	// The header shouldn't contain commas, so if the comma is found
	// before the header, we hit a \n\n inside or after a map.
	// This is no header, so don't parse it as it would be.

	if (!(!(header_offset == std::string::npos || comma_offset < header_offset)))
		return 0;

	std::string header_str(std::string(data, 0, header_offset + 1));
	config header;
	::read(header, header_str);

	return header_offset + 2;
}


std::string gamemap::write() const
{
	return t_translation::write_game_map(tiles(), special_locations(), t_translation::coordinate{ border_size(), border_size() }) + "\n";
}

void gamemap_base::overlay(const gamemap_base& m, map_location loc, const std::vector<overlay_rule>& rules, bool m_is_odd, bool ignore_special_locations)
{
	int xpos = loc.wml_x();
	int ypos = loc.wml_y();

	const int xstart = std::max<int>(0, -xpos);
	const int xend = std::min<int>(m.total_width(), total_width() - xpos);
	const int xoffset = xpos;

	const int ystart_even = std::max<int>(0, -ypos);
	const int yend_even = std::min<int>(m.total_height(), total_height() - ypos);
	const int yoffset_even = ypos;

	const int ystart_odd = std::max<int>(0, -ypos +(xpos & 1) -(m_is_odd ? 1 : 0));
	const int yend_odd = std::min<int>(m.total_height(), total_height() - ypos +(xpos & 1) -(m_is_odd ? 1 : 0));
	const int yoffset_odd = ypos -(xpos & 1) + (m_is_odd ? 1 : 0);

	for(int x1 = xstart; x1 != xend; ++x1) {
		int ystart, yend, yoffset;
		if(x1 & 1) {
			ystart = ystart_odd;
			yend = yend_odd;
			yoffset = yoffset_odd;
		}
		else {
			ystart = ystart_even;
			yend = yend_even;
			yoffset = yoffset_even;
		}
		for(int y1 = ystart; y1 != yend; ++y1) {
			const int x2 = x1 + xoffset;
			const int y2 = y1 + yoffset;

			const t_translation::terrain_code t = m.get_terrain({x1,y1, wml_loc()});
			const t_translation::terrain_code current = get_terrain({x2, y2, wml_loc()});

			if(t == t_translation::FOGGED || t == t_translation::VOID_TERRAIN) {
				continue;
			}

			// See if there is a matching rule
			const overlay_rule* rule = nullptr;
			for(const overlay_rule& current_rule : rules)
			{
				if(!current_rule.old_.empty() && !t_translation::terrain_matches(current, current_rule.old_)) {
					continue;
				}
				if(!current_rule.new_.empty() && !t_translation::terrain_matches(t, current_rule.new_)) {
					continue;
				}
				rule = &current_rule;
				break;
			}

			if (!rule) {
				set_terrain(map_location(x2, y2, wml_loc()), t, terrain_type_data::BOTH, false);
			}
			else if(!rule->use_old_) {
				set_terrain(map_location(x2, y2, wml_loc()), rule->terrain_ ? *rule->terrain_ : t , rule->mode_, rule->replace_if_failed_);
			}
		}
	}

	if (!ignore_special_locations) {
		for(auto& pair : m.special_locations().left) {

			int x = pair.second.wml_x();
			int y = pair.second.wml_y();
			if(x & 1) {
				if(x < xstart || x >= xend || y < ystart_odd || y >= yend_odd) {
					continue;
				}
			}
			else {
				if(x < xstart || x >= xend || y < ystart_even || y >= yend_even) {
					continue;
				}
			}
			int x_new = x + xoffset;
			int y_new = y + ((x & 1 ) ? yoffset_odd : yoffset_even);
			map_location pos_new = map_location(x_new, y_new, wml_loc());

			starting_positions_.left.erase(pair.first);
			starting_positions_.insert(location_map::value_type(pair.first, t_translation::coordinate(pos_new.x, pos_new.y)));
		}
	}
}
t_translation::terrain_code gamemap_base::get_terrain(const map_location& loc) const
{

	if(on_board_with_border(loc)) {
		return tiles_.get(loc.x + border_size(), loc.y + border_size());
	}

	return loc == map_location::null_location() ? t_translation::NONE_TERRAIN : t_translation::terrain_code();
}

map_location gamemap_base::special_location(const std::string& id) const
{
	auto it = starting_positions_.left.find(id);
	if (it != starting_positions_.left.end()) {
		auto& coordinate = it->second;
		return map_location(coordinate.x, coordinate.y);
	}
	else {
		return map_location();
	}
}

map_location gamemap_base::starting_position(int n) const
{
	return special_location(std::to_string(n));
}

namespace {
	bool is_number(const std::string& id) {
		return std::find_if(id.begin(), id.end(), [](char c) { return !std::isdigit(c); }) == id.end();
	}
}

int gamemap_base::num_valid_starting_positions() const
{
	int res = 0;
	for (auto pair : starting_positions_) {
		const std::string& id = pair.left;
		if (is_number(id)) {
			res = std::max(res, std::stoi(id));
		}
	}
	return res;
}

int gamemap_base::is_starting_position(const map_location& loc) const
{
	if(const std::string* locName = is_special_location(loc)) {
		if(is_number(*locName)) {
			return std::stoi(*locName);
		}
	}
	return 0;
}

const std::string* gamemap_base::is_special_location(const map_location& loc) const
{
	auto it = starting_positions_.right.find(loc);
	return it == starting_positions_.right.end() ? nullptr : &it->second;
}

void gamemap_base::set_special_location(const std::string& id, const map_location& loc)
{
	bool valid = loc.valid();
	auto it_left = starting_positions_.left.find(id);
	if (it_left != starting_positions_.left.end()) {
		if (valid) {
			starting_positions_.left.replace_data(it_left, loc);
		}
		else {
			starting_positions_.left.erase(it_left);
		}
	}
	else {
		starting_positions_.left.insert(it_left, std::pair(id, loc));
	}
}

void gamemap_base::set_starting_position(int side, const map_location& loc)
{
	set_special_location(std::to_string(side), loc);
}

bool gamemap_base::on_board(const map_location& loc) const
{
	return loc.valid() && loc.x < w() && loc.y < h();
}

bool gamemap_base::on_board_with_border(const map_location& loc) const
{
	return !tiles_.data.empty()  &&  // tiles_ is not empty when initialized.
	       loc.x >= -border_size() &&  loc.x < w() + border_size() &&
	       loc.y >= -border_size() &&  loc.y < h() + border_size();
}

void gamemap::set_terrain(const map_location& loc, const t_translation::terrain_code & terrain, const terrain_type_data::merge_mode mode, bool replace_if_failed) {
	if(!on_board_with_border(loc)) {
		DBG_G << "set_terrain: " << loc << " is not on the map.";
		// off the map: ignore request
		return;
	}

	t_translation::terrain_code new_terrain = tdata_->merge_terrains(get_terrain(loc), terrain, mode, replace_if_failed);

	if(new_terrain == t_translation::NONE_TERRAIN) {
		return;
	}

	if(on_board(loc)) {
		const bool old_village = is_village(loc);
		const bool new_village = tdata_->is_village(new_terrain);

		if(old_village && !new_village) {
			utils::erase(villages_, loc);
		} else if(!old_village && new_village) {
			villages_.push_back(loc);
		}
	}

	(*this)[loc] = new_terrain;
}

std::vector<map_location> gamemap_base::parse_location_range(const std::string &x, const std::string &y,
	bool with_border) const
{
	std::vector<map_location> res;
	const std::vector<std::string> xvals = utils::split(x);
	const std::vector<std::string> yvals = utils::split(y);
	int xmin = 1, xmax = w(), ymin = 1, ymax = h();
	if (with_border) {
		int bs = border_size();
		xmin -= bs;
		xmax += bs;
		ymin -= bs;
		ymax += bs;
	}

	for (unsigned i = 0; i < xvals.size() || i < yvals.size(); ++i)
	{
		std::pair<int,int> xrange, yrange;

		if (i < xvals.size()) {
			xrange = utils::parse_range(xvals[i]);
			if (xrange.first < xmin) xrange.first = xmin;
			if (xrange.second > xmax) xrange.second = xmax;
		} else {
			xrange.first = xmin;
			xrange.second = xmax;
		}

		if (i < yvals.size()) {
			yrange = utils::parse_range(yvals[i]);
			if (yrange.first < ymin) yrange.first = ymin;
			if (yrange.second > ymax) yrange.second = ymax;
		} else {
			yrange.first = ymin;
			yrange.second = ymax;
		}

		for(int x2 = xrange.first; x2 <= xrange.second; ++x2) {
			for(int y2 = yrange.first; y2 <= yrange.second; ++y2) {
				res.emplace_back(x2-1,y2-1);
			}
		}
	}
	return res;
}

std::string gamemap_base::to_string() const
{
	return t_translation::write_game_map(tiles_, starting_positions_, { 1, 1 }) + "\n";
}

const std::vector<map_location> gamemap_base::starting_positions() const {
	int n = num_valid_starting_positions();
	std::vector<map_location> res;
	for(int i = 1; i <= n; i++) {
		res.push_back(starting_position(i));
	}
	return res;
}
