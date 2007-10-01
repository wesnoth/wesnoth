/* $Id$ */
/*
   Copyright (C) 2003-5 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "display.hpp"
#include "font.hpp"
#include "language.hpp"
#include "map_label.hpp"

namespace {
const size_t max_label_size = 64;

//our definition of map labels being obscured is if the tile is obscured,
//or the tile below is obscured. This is because in the case where the tile
//itself is visible, but the tile below is obscured, the bottom half of the
//tile will still be shrouded, and the label being drawn looks weird
bool is_shrouded(const display& disp, const gamemap::location& loc)
{
	return disp.shrouded(loc.x,loc.y) || disp.shrouded(loc.x,loc.y+1);
}

}

map_labels::map_labels(const display& disp, const gamemap& map) : disp_(disp), map_(map)
{}

map_labels::map_labels(const display& disp, const config& cfg, const gamemap& map) : disp_(disp), map_(map)
{
	read(cfg);
}

map_labels::~map_labels()
{
	clear();
}

void map_labels::write(config& res) const
{
	for(label_map::const_iterator i = labels_.begin(); i != labels_.end(); ++i) {
		config item;
		i->first.write(item);
		item.values["text"] = get_label(i->first);
		res.add_child("label",item);
	}
}

void map_labels::read(const config& cfg)
{
	clear();

	const config::child_list& items = cfg.get_children("label");
	for(config::child_list::const_iterator i = items.begin(); i != items.end(); ++i) {
		const gamemap::location loc(**i);
		const std::string& text = (**i)["text"];
		set_label(loc,text);
	}
}

const std::string& map_labels::get_label(int index) const {
	return font::get_floating_label_text(index);
}

int map_labels::get_max_chars() {
	return max_label_size;
}

const std::string& map_labels::get_label(const gamemap::location& loc) const
{
	const label_map::const_iterator itor = labels_.find(loc);
	if(itor != labels_.end()) {
		return font::get_floating_label_text(itor->second);
	} else {
		static const std::string empty_str;
		return empty_str;
	}
}

void map_labels::set_label(const gamemap::location& loc, const std::string& str, const SDL_Color colour)
{
	// The actual data is wide_strings so test in wide_string mode
	// also cutting a wide_string at an arbritary place gives odd 
	// problems. 
	wide_string tmp = utils::string_to_wstring(str);
	if(tmp.size() > max_label_size) {
		tmp.resize(max_label_size);
	}
	std::string text = utils::wstring_to_string(tmp);

	const label_map::iterator current_label = labels_.find(loc);
	if(current_label != labels_.end()) {
		font::remove_floating_label(current_label->second);
		labels_.erase(current_label);
	}

	if(text == "") {
		return;
	}



	const gamemap::location loc_nextx(loc.x+1,loc.y);
	const gamemap::location loc_nexty(loc.x,loc.y+1);
	const int xloc = (disp_.get_location_x(loc) + disp_.get_location_x(loc_nextx)*2)/3;
	const int yloc = disp_.get_location_y(loc_nexty) - font::SIZE_NORMAL;
	const int handle = font::add_floating_label(text,font::SIZE_NORMAL,colour,xloc,yloc,0,0,-1,disp_.map_area());

	labels_.insert(std::pair<gamemap::location,int>(loc,handle));

	if(is_shrouded(disp_,loc)) {
		font::show_floating_label(handle,false);
	}
}

void map_labels::clear()
{
	for(label_map::const_iterator i = labels_.begin(); i != labels_.end(); ++i) {
		font::remove_floating_label(i->second);
	}

	labels_.clear();
}

void map_labels::scroll(double xmove, double ymove)
{
	for(label_map::const_iterator i = labels_.begin(); i != labels_.end(); ++i) {
		font::move_floating_label(i->second,xmove,ymove);
	}
}

void map_labels::recalculate_labels()
{
	const label_map labels = labels_;
	labels_.clear();
	for(label_map::const_iterator i = labels.begin(); i != labels.end(); ++i) {
		const std::string text = font::get_floating_label_text(i->second);
		font::remove_floating_label(i->second);
		set_label(i->first,text);
	}
}

void map_labels::recalculate_shroud()
{
	for(label_map::const_iterator i = labels_.begin(); i != labels_.end(); ++i) {
		font::show_floating_label(i->second,!is_shrouded(disp_,i->first));
	}
}
