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
#include "map_context.hpp"

#include "../foreach.hpp"

#include <algorithm>
#include <memory>

namespace editor2 {

int editor_action::next_id_ = 1;
int editor_action::instance_count_ = 0;

editor_action::editor_action()
: id_(next_id_++)
{
	instance_count_++;
}

editor_action::~editor_action()
{
	instance_count_--;
}

std::string editor_action::get_description()
{
	return "Unknown action";
}

editor_action* editor_action::perform(map_context& mc) const
{
	std::auto_ptr<editor_action> undo(new editor_action_whole_map(mc.get_map()));
	perform_without_undo(mc);
	return undo.release();
}

void editor_action_whole_map::perform_without_undo(map_context& mc) const {
	mc.get_map() = m_;
	mc.set_needs_reload();
}

editor_action_chain::~editor_action_chain()
{
	foreach (editor_action* a, actions_) {
		delete a;
	}
}
void editor_action_chain::append_action(editor_action* a) {
	actions_.push_back(a);
}
editor_action_chain* editor_action_chain::perform(map_context& mc) const {
	std::auto_ptr<editor_action_chain> undo(new editor_action_chain());
	foreach (editor_action* a, actions_) {
		undo->append_action(a->perform(mc));
	}
	std::reverse(undo->actions_.begin(), undo->actions_.end());
	return undo.release();
}
void editor_action_chain::perform_without_undo(map_context& mc) const
{
    foreach (editor_action* a, actions_) {
		a->perform_without_undo(mc);
	}
}

bool editor_action_area::add_location(const gamemap::location& loc)
{
	return area_.insert(loc).second;
}
void editor_action_area::add_locations(const std::set<gamemap::location>& locs)
{
	area_.insert(locs.begin(), locs.end());
}
void editor_action_area::extend(const editor_map& /*map*/, const std::set<gamemap::location>& locs)
{
	area_.insert(locs.begin(), locs.end());
}

void editor_action_paste::extend(const editor_map& map, const std::set<gamemap::location>& locs)
{
	paste_.add_tiles(map, locs);
}
editor_action_paste* editor_action_paste::perform(map_context& mc) const
{
	map_fragment mf(mc.get_map(), paste_.get_offset_area(offset_));
	std::auto_ptr<editor_action_paste> undo(new editor_action_paste(mf));
	perform_without_undo(mc);
	return undo.release();
}
void editor_action_paste::perform_without_undo(map_context& mc) const
{
	paste_.paste_into(mc.get_map(), offset_);
	mc.add_changed_location(paste_.get_offset_area(offset_));
	mc.set_needs_terrain_rebuild();
}

editor_action_paint_hex* editor_action_paint_hex::perform(map_context& mc) const
{
	std::auto_ptr<editor_action_paint_hex> undo(new editor_action_paint_hex(loc_, mc.get_map().get_terrain(loc_)));
	perform_without_undo(mc);
	return undo.release();
}
void editor_action_paint_hex::perform_without_undo(map_context& mc) const
{
	mc.draw_terrain(t_, loc_);
	mc.set_needs_terrain_rebuild();
}

editor_action_paste* editor_action_paint_area::perform(map_context& mc) const
{
	map_fragment mf(mc.get_map(), area_);
	std::auto_ptr<editor_action_paste> undo(new editor_action_paste(mf));
	perform_without_undo(mc);
	return undo.release();
}	
void editor_action_paint_area::perform_without_undo(map_context& mc) const
{
	mc.draw_terrain(t_, area_, one_layer_);
	mc.set_needs_terrain_rebuild();
}

editor_action_paint_area* editor_action_fill::perform(map_context& mc) const
{
	std::set<gamemap::location> to_fill = mc.get_map().get_contigious_terrain_tiles(loc_);
	std::auto_ptr<editor_action_paint_area> undo(new editor_action_paint_area(to_fill, mc.get_map().get_terrain(loc_)));
	mc.draw_terrain(t_, to_fill, one_layer_);
	mc.set_needs_terrain_rebuild();
	return undo.release();
}
void editor_action_fill::perform_without_undo(map_context& mc) const
{
	std::set<gamemap::location> to_fill = mc.get_map().get_contigious_terrain_tiles(loc_);
	mc.draw_terrain(t_, to_fill, one_layer_);
	mc.set_needs_terrain_rebuild();
}

editor_action* editor_action_starting_position::perform(map_context& mc) const
{
	std::auto_ptr<editor_action> undo;
	int old_player = mc.get_map().is_starting_position(loc_) + 1;
	gamemap::location old_loc = mc.get_map().starting_position(player_);
	LOG_ED << "ssp perform, player_" << player_ << ", loc_ " << loc_ << ", old_player " << old_player << ", old_loc " << old_loc << "\n";
	if (old_player != -1) {
		// If another player was starting at the location, we actually perform two actions, so the undo is an action_chain.
		editor_action_chain* undo_chain = new editor_action_chain();
		undo_chain->append_action(new editor_action_starting_position(loc_, old_player));
		undo_chain->append_action(new editor_action_starting_position(old_loc, player_));
		undo.reset(undo_chain);
		LOG_ED << "ssp actual: " << old_player << " to " << gamemap::location() << "\n";
		mc.get_map().set_starting_position(old_player, gamemap::location());
	} else {
		undo.reset(new editor_action_starting_position(old_loc, player_));
	}
	LOG_ED << "ssp actual: " << player_ << " to " << loc_ << "\n";
	mc.get_map().set_starting_position(player_, loc_);
	mc.set_needs_labels_reset();
	return undo.release();
}
void editor_action_starting_position::perform_without_undo(map_context& mc) const
{
	int old_player = mc.get_map().is_starting_position(loc_);
	if (old_player != -1) {
		mc.get_map().set_starting_position(old_player, gamemap::location());
	}
	mc.get_map().set_starting_position(player_, loc_);
	mc.set_needs_labels_reset();
}

void editor_action_select::extend(const editor_map& map, const std::set<gamemap::location>& locs)
{
	foreach (const gamemap::location& loc, locs) {
		LOG_ED << "Checking " << loc << "\n";
		if (map.in_selection(loc)) {
			LOG_ED << "Extending by " << loc << "\n";
			area_.insert(loc);
		}
	}
}
editor_action* editor_action_select::perform(map_context& mc) const
{
	std::set<gamemap::location> undo_locs;
	foreach (const gamemap::location& loc, area_) {
		if (!mc.get_map().in_selection(loc)) {
			undo_locs.insert(loc);
			mc.add_changed_location(loc);
		}
	}
	perform_without_undo(mc);
	return new editor_action_deselect(undo_locs);
}
void editor_action_select::perform_without_undo(map_context& mc) const
{
	foreach (const gamemap::location& loc, area_) {
		mc.get_map().add_to_selection(loc);
		mc.add_changed_location(loc);
	}
}

void editor_action_deselect::extend(const editor_map& map, const std::set<gamemap::location>& locs)
{
	foreach (const gamemap::location& loc, locs) {
		LOG_ED << "Checking " << loc << "\n";
		if (!map.in_selection(loc)) {
			LOG_ED << "Extending by " << loc << "\n";
			area_.insert(loc);
		}
	}
}
editor_action* editor_action_deselect::perform(map_context& mc) const
{
	std::set<gamemap::location> undo_locs;
	foreach (const gamemap::location& loc, area_) {
		if (mc.get_map().in_selection(loc)) {
			undo_locs.insert(loc);
			mc.add_changed_location(loc);
		}
	}
	perform_without_undo(mc);
	return new editor_action_select(undo_locs);
}
void editor_action_deselect::perform_without_undo(map_context& mc) const
{
	foreach (const gamemap::location& loc, area_) {
		mc.get_map().remove_from_selection(loc);
		mc.add_changed_location(loc);
	}
}

editor_action_deselect* editor_action_select_all::perform(map_context& mc) const
{
	
	std::set<gamemap::location> current = mc.get_map().selection();
	mc.get_map().select_all();
	std::set<gamemap::location> all = mc.get_map().selection();
	std::set<gamemap::location> undo_locs;
	std::set_difference(all.begin(), all.end(), 
		current.begin(), current.end(), 
		std::inserter(undo_locs, undo_locs.begin()));
	mc.set_everything_changed();
	return new editor_action_deselect(undo_locs);
}
void editor_action_select_all::perform_without_undo(map_context& mc) const
{
	mc.get_map().select_all();
	mc.set_everything_changed();
}

editor_action_select* editor_action_select_none::perform(map_context& mc) const
{
	
	std::set<gamemap::location> current = mc.get_map().selection();
	mc.get_map().clear_selection();
	mc.set_everything_changed();
	return new editor_action_select(current);
}
void editor_action_select_none::perform_without_undo(map_context& mc) const
{
	mc.get_map().clear_selection();
	mc.set_everything_changed();
}


editor_action_select_inverse* editor_action_select_inverse::perform(map_context& mc) const
{
	perform_without_undo(mc);
	return new editor_action_select_inverse();
}
void editor_action_select_inverse::perform_without_undo(map_context& mc) const
{
	mc.get_map().invert_selection();
	mc.set_everything_changed();
}

void editor_action_resize_map::perform_without_undo(map_context& mc) const
{
	mc.get_map().resize(x_size_, y_size_, x_offset_, y_offset_, fill_);
	mc.set_needs_reload();
}

void editor_action_rotate_map::perform_without_undo(map_context& /*mc*/) const
{
	throw editor_action_not_implemented();
}

void editor_action_flip_x::perform_without_undo(map_context& mc) const
{
	mc.get_map().flip_x();
	mc.set_needs_reload();
}

void editor_action_flip_y::perform_without_undo(map_context& mc) const
{
	mc.get_map().flip_y();
	mc.set_needs_reload();
}

editor_action_paste* editor_action_plot_route::perform(map_context& /*mc*/) const
{
	throw editor_action_not_implemented();
}
void editor_action_plot_route::perform_without_undo(map_context& /*mc*/) const
{
	throw editor_action_not_implemented();
}

editor_action_paste* editor_action_shuffle_area::perform(map_context& mc) const
{
	map_fragment mf(mc.get_map(), area_);
	std::auto_ptr<editor_action_paste> undo(new editor_action_paste(mf));
	perform_without_undo(mc);
	return undo.release();
}

void editor_action_shuffle_area::perform_without_undo(map_context& mc) const
{
	std::vector<gamemap::location> shuffle;
	std::copy(area_.begin(), area_.end(), std::inserter(shuffle, shuffle.begin()));
	std::random_shuffle(shuffle.begin(), shuffle.end());
	std::vector<gamemap::location>::const_iterator shuffle_it = shuffle.begin();
	std::set<gamemap::location>::const_iterator orig_it = area_.begin();
	while (orig_it != area_.end()) {
		t_translation::t_terrain tmp = mc.get_map().get_terrain(*orig_it);
		mc.draw_terrain(mc.get_map().get_terrain(*shuffle_it), *orig_it);
		mc.draw_terrain(tmp, *shuffle_it);
		orig_it++;
		shuffle_it++;
	}
	mc.set_needs_terrain_rebuild();
}

} //end namespace editor2
