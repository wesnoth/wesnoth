/* $Id$ */
/*
   Copyright (C) 2008 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file action.cpp
//! Editor action classes

#include "action.hpp"
#include "editor_common.hpp"
#include "../foreach.hpp"

namespace editor2 {

int editor_action::next_id_ = 1;
int editor_action::instance_count_ = 0;

std::string editor_action::get_description()
{
	return "Unknown action";
}

editor_action* editor_action::perform(editor_map& map) const
{
	editor_action* undo = new editor_action_whole_map(map);
	perform_without_undo(map);
	return undo;
}

void draw_terrain(editor_map& map, t_translation::t_terrain terrain, 
	const gamemap::location& loc, const bool one_layer_only = false)
{
    if (!one_layer_only) {
        terrain = map.get_terrain_info(terrain).terrain_with_default_base();
    }
	t_translation::t_terrain old_terrain = map.get_terrain(loc);
	if (terrain != old_terrain) {
		if (terrain.base == t_translation::NO_LAYER) {
			map.set_terrain(loc, terrain, gamemap::OVERLAY);
		} else if (one_layer_only) {
			map.set_terrain(loc, terrain, gamemap::BASE);
		} else {
			map.set_terrain(loc, terrain);
		}
	}
}

void draw_terrain(editor_map& map, t_translation::t_terrain terrain, 
	const std::set<gamemap::location>& locs, const bool one_layer_only = false)
{
    if (!one_layer_only) {
        terrain = map.get_terrain_info(terrain).terrain_with_default_base();
    }
	foreach (const gamemap::location& loc, locs) {
		t_translation::t_terrain old_terrain = map.get_terrain(loc);
		if (terrain != old_terrain) {
			if (terrain.base == t_translation::NO_LAYER) {
				map.set_terrain(loc, terrain, gamemap::OVERLAY);
			} else if (one_layer_only) {
				map.set_terrain(loc, terrain, gamemap::BASE);
			} else {
				map.set_terrain(loc, terrain);
			}
		}
	}
}
	
void editor_action_whole_map::perform_without_undo(editor_map& m) const {
	m = m_;
}

editor_action_chain::~editor_action_chain()
{
	foreach (editor_action* a, actions_) {
		delete a;
	}
}
editor_action_chain* editor_action_chain::perform(editor_map& m) const {
	std::vector<editor_action*> undo;
	foreach (editor_action* a, actions_) {
		undo.push_back(a->perform(m));
	}
	std::reverse(undo.begin(), undo.end());
	return new editor_action_chain(undo);
}
void editor_action_chain::perform_without_undo(editor_map& m) const
{
    foreach (editor_action* a, actions_) {
		a->perform_without_undo(m);
	}
}

bool editor_action_area::add_location(const gamemap::location& loc)
{
	return area_.insert(loc).second;
}

editor_action_paste* editor_action_paste::perform(editor_map& map) const
{
	map_fragment mf(map, paste_.get_offset_area(loc_));
	editor_action_paste* undo = new editor_action_paste(gamemap::location(0,0), mf);
	perform_without_undo(map);
	return undo;
}
void editor_action_paste::perform_without_undo(editor_map& map) const
{
	paste_.paste_into(map, loc_);
}

editor_action_paint_hex* editor_action_paint_hex::perform(editor_map& map) const
{
	editor_action_paint_hex* undo = new editor_action_paint_hex(loc_, map.get_terrain(loc_));
	perform_without_undo(map);
	return undo;
}
void editor_action_paint_hex::perform_without_undo(editor_map& map) const
{
	draw_terrain(map, t_, loc_);
}

editor_action_paste* editor_action_paint_area::perform(editor_map& map) const
{
	map_fragment mf(map, area_);
	editor_action_paste* undo = new editor_action_paste(gamemap::location(0,0), mf);
	perform_without_undo(map);
	return undo;
}	
void editor_action_paint_area::perform_without_undo(editor_map& map) const
{
	draw_terrain(map, t_, area_);
}

editor_action_paint_area* editor_action_fill::perform(editor_map& map) const
{
	std::set<gamemap::location> to_fill = map.get_contigious_terrain_tiles(loc_);
	editor_action_paint_area* undo = new editor_action_paint_area(to_fill, map.get_terrain(loc_));
	draw_terrain(map, t_, to_fill);
	return undo;
}
void editor_action_fill::perform_without_undo(editor_map& map) const
{
	std::set<gamemap::location> to_fill = map.get_contigious_terrain_tiles(loc_);
	draw_terrain(map, t_, to_fill);
}

editor_action_select_xor* editor_action_select_xor::perform(editor_map& map) const
{
	perform_without_undo(map);
	return new editor_action_select_xor(area_);
}
void editor_action_select_xor::perform_without_undo(editor_map& map) const
{
	foreach (const gamemap::location& loc, area_) {
		if (map.in_selection(loc)) {
			map.remove_from_selection(loc);
		} else {
			map.add_to_selection(loc);
		}
	}
}

editor_action_select_xor* editor_action_select::perform(editor_map& map) const
{
	std::set<gamemap::location> undo_locs;
	foreach (const gamemap::location& loc, area_) {
		if (!map.in_selection(loc)) {
			undo_locs.insert(loc);
		}
	}
	perform_without_undo(map);
	return new editor_action_select_xor(undo_locs);
}
void editor_action_select::perform_without_undo(editor_map& map) const
{
	foreach (const gamemap::location& loc, area_) {
		map.add_to_selection(loc);
	}
}

editor_action_select_xor* editor_action_deselect::perform(editor_map& map) const
{
	std::set<gamemap::location> undo_locs;
	foreach (const gamemap::location& loc, area_) {
		if (map.in_selection(loc)) {
			undo_locs.insert(loc);
		}
	}
	perform_without_undo(map);
	return new editor_action_select_xor(undo_locs);
}
void editor_action_deselect::perform_without_undo(editor_map& map) const
{
	foreach (const gamemap::location& loc, area_) {
		map.remove_from_selection(loc);
	}
}

void editor_action_resize_map::perform_without_undo(editor_map& map) const
{
	map.resize(x_size_, y_size_, x_offset_, y_offset_);
}

void editor_action_rotate_map::perform_without_undo(editor_map& /*map*/) const
{
	throw editor_action_not_implemented();
}

void editor_action_flip_x::perform_without_undo(editor_map& map) const
{
	map.flip_x();
}
void editor_action_flip_y::perform_without_undo(editor_map& map) const
{
	map.flip_y();
}

editor_action_paste* editor_action_plot_route::perform(editor_map& /*map*/) const
{
	throw editor_action_not_implemented();
}
void editor_action_plot_route::perform_without_undo(editor_map& /*map*/) const
{
	throw editor_action_not_implemented();
}

} //end namespace editor2
