/* $Id$ */
/*
   Copyright (C) 2008 - 2011 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file action.cpp
 * Editor action classes
 */
#define GETTEXT_DOMAIN "wesnoth-editor"
#include "action.hpp"
#include "map_context.hpp"

#include "../foreach.hpp"
#include "../gettext.hpp"

namespace editor {

int editor_action::next_id_ = 1;
int editor_action::instance_count_ = 0;

editor_action::editor_action()
: id_(next_id_++)
{
	instance_count_++;
#ifdef EDITOR_DEBUG_ACTION_LIFETIME
	LOG_ED << "Action " << std::setw(2) << id_ << " ctor " << this << " (count is " << instance_count << "\n";
#endif
}

editor_action::~editor_action()
{
	instance_count_--;
#ifdef EDITOR_DEBUG_ACTION_LIFETIME
	LOG_ED << "Action " << std::setw(2) << id_ << " dtor " << this << " (count is " << instance_count << "\n";
#endif
}

int editor_action::action_count() const
{
	return 1;
}

std::string editor_action::get_description() const
{
	return "Unknown action";
}

editor_action* editor_action::perform(map_context& mc) const
{
	std::auto_ptr<editor_action> undo(new editor_action_whole_map(mc.get_map()));
	perform_without_undo(mc);
	return undo.release();
}

editor_action_whole_map* editor_action_whole_map::clone() const
{
	return new editor_action_whole_map(*this);
}
void editor_action_whole_map::perform_without_undo(map_context& mc) const {
	mc.set_map(m_);
}

editor_action_chain::editor_action_chain(const editor::editor_action_chain &other)
	: editor_action(), actions_()
{
	BOOST_FOREACH (editor_action* a, other.actions_) {
		actions_.push_back(a->clone());
	}
}
editor_action_chain& editor_action_chain::operator=(const editor_action_chain& other)
{
	if (this == &other) return *this;
	BOOST_FOREACH (editor_action* a, actions_) {
		delete a;
	}
	actions_.clear();
	BOOST_FOREACH (editor_action* a, other.actions_) {
		actions_.push_back(a->clone());
	}
	return *this;
}
editor_action_chain::~editor_action_chain()
{
	BOOST_FOREACH (editor_action* a, actions_) {
		delete a;
	}
}
editor_action_chain* editor_action_chain::clone() const
{
	return new editor_action_chain(*this);
}
int editor_action_chain::action_count() const {
	int count = 0;
	BOOST_FOREACH (const editor_action* a, actions_) {
		if (a) {
			count += a->action_count();
		}
	}
	return count;
}
void editor_action_chain::append_action(editor_action* a) {
	actions_.push_back(a);
}
void editor_action_chain::prepend_action(editor_action* a) {
	actions_.push_front(a);
}
bool editor_action_chain::empty() const {
	return actions_.empty();
}
editor_action* editor_action_chain::pop_last_action() {
	if (empty()) throw editor_action_exception("pop_last_action requested on an empty action_chain");
	editor_action* last = actions_.back();
	actions_.pop_back();
	return last;
}
editor_action* editor_action_chain::pop_first_action() {
	if (empty()) throw editor_action_exception("pop_first_action requested on an empty action_chain");
	editor_action* last = actions_.front();
	actions_.pop_front();
	return last;
}
editor_action_chain* editor_action_chain::perform(map_context& mc) const {
	std::auto_ptr<editor_action_chain> undo(new editor_action_chain());
	BOOST_FOREACH (editor_action* a, actions_) {
		if (a != NULL) {
			undo->append_action(a->perform(mc));
		}
	}
	std::reverse(undo->actions_.begin(), undo->actions_.end());
	return undo.release();
}
void editor_action_chain::perform_without_undo(map_context& mc) const
{
	BOOST_FOREACH (editor_action* a, actions_) {
		if (a != NULL) {
			a->perform_without_undo(mc);
		}
	}
}

void editor_action_area::extend(const editor_map& /*map*/, const std::set<map_location>& locs)
{
	area_.insert(locs.begin(), locs.end());
}

editor_action_paste* editor_action_paste::clone() const
{
	return new editor_action_paste(*this);
}
void editor_action_paste::extend(const editor_map& map, const std::set<map_location>& locs)
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


editor_action_paint_area* editor_action_paint_area::clone() const
{
	return new editor_action_paint_area(*this);
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

editor_action_fill* editor_action_fill::clone() const
{
	return new editor_action_fill(*this);
}
editor_action_paint_area* editor_action_fill::perform(map_context& mc) const
{
	std::set<map_location> to_fill = mc.get_map().get_contigious_terrain_tiles(loc_);
	std::auto_ptr<editor_action_paint_area> undo(new editor_action_paint_area(to_fill, mc.get_map().get_terrain(loc_)));
	mc.draw_terrain(t_, to_fill, one_layer_);
	mc.set_needs_terrain_rebuild();
	return undo.release();
}
void editor_action_fill::perform_without_undo(map_context& mc) const
{
	std::set<map_location> to_fill = mc.get_map().get_contigious_terrain_tiles(loc_);
	mc.draw_terrain(t_, to_fill, one_layer_);
	mc.set_needs_terrain_rebuild();
}

editor_action_starting_position* editor_action_starting_position::clone() const
{
	return new editor_action_starting_position(*this);
}
editor_action* editor_action_starting_position::perform(map_context& mc) const
{
	std::auto_ptr<editor_action> undo;
	int old_player = mc.get_map().is_starting_position(loc_) + 1;
	map_location old_loc = mc.get_map().starting_position(player_);
	LOG_ED << "ssp perform, player_" << player_ << ", loc_ " << loc_ << ", old_player " << old_player << ", old_loc " << old_loc << "\n";
	if (old_player != -1) {
		// If another player was starting at the location, we actually perform two actions, so the undo is an action_chain.
		editor_action_chain* undo_chain = new editor_action_chain();
		undo_chain->append_action(new editor_action_starting_position(loc_, old_player));
		undo_chain->append_action(new editor_action_starting_position(old_loc, player_));
		undo.reset(undo_chain);
		LOG_ED << "ssp actual: " << old_player << " to " << map_location() << "\n";
		mc.get_map().set_starting_position(old_player, map_location());
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
		mc.get_map().set_starting_position(old_player, map_location());
	}
	mc.get_map().set_starting_position(player_, loc_);
	mc.set_needs_labels_reset();
}


editor_action_select* editor_action_select::clone() const
{
	return new editor_action_select(*this);
}
void editor_action_select::extend(const editor_map& map, const std::set<map_location>& locs)
{
	BOOST_FOREACH (const map_location& loc, locs) {
		LOG_ED << "Checking " << loc << "\n";
		if (map.in_selection(loc)) {
			LOG_ED << "Extending by " << loc << "\n";
			area_.insert(loc);
		}
	}
}
editor_action* editor_action_select::perform(map_context& mc) const
{
	std::set<map_location> undo_locs;
	BOOST_FOREACH (const map_location& loc, area_) {
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
	BOOST_FOREACH (const map_location& loc, area_) {
		mc.get_map().add_to_selection(loc);
		mc.add_changed_location(loc);
	}
}

editor_action_deselect* editor_action_deselect::clone() const
{
	return new editor_action_deselect(*this);
}
void editor_action_deselect::extend(const editor_map& map, const std::set<map_location>& locs)
{
	BOOST_FOREACH (const map_location& loc, locs) {
		LOG_ED << "Checking " << loc << "\n";
		if (!map.in_selection(loc)) {
			LOG_ED << "Extending by " << loc << "\n";
			area_.insert(loc);
		}
	}
}
editor_action* editor_action_deselect::perform(map_context& mc) const
{
	std::set<map_location> undo_locs;
	BOOST_FOREACH (const map_location& loc, area_) {
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
	BOOST_FOREACH (const map_location& loc, area_) {
		mc.get_map().remove_from_selection(loc);
		mc.add_changed_location(loc);
	}
}

editor_action_select_all* editor_action_select_all::clone() const
{
	return new editor_action_select_all(*this);
}
editor_action_deselect* editor_action_select_all::perform(map_context& mc) const
{
	std::set<map_location> current = mc.get_map().selection();
	mc.get_map().select_all();
	std::set<map_location> all = mc.get_map().selection();
	std::set<map_location> undo_locs;
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

editor_action_select_none* editor_action_select_none::clone() const
{
	return new editor_action_select_none(*this);
}
editor_action_select* editor_action_select_none::perform(map_context& mc) const
{
	std::set<map_location> current = mc.get_map().selection();
	mc.get_map().clear_selection();
	mc.set_everything_changed();
	return new editor_action_select(current);
}
void editor_action_select_none::perform_without_undo(map_context& mc) const
{
	mc.get_map().clear_selection();
	mc.set_everything_changed();
}

editor_action_select_inverse* editor_action_select_inverse::clone() const
{
	return new editor_action_select_inverse(*this);
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

editor_action_resize_map* editor_action_resize_map::clone() const
{
	return new editor_action_resize_map(*this);
}
void editor_action_resize_map::perform_without_undo(map_context& mc) const
{
	mc.get_map().resize(x_size_, y_size_, x_offset_, y_offset_, fill_);
	mc.set_needs_reload();
}

editor_action_apply_mask* editor_action_apply_mask::clone() const
{
	return new editor_action_apply_mask(*this);
}
void editor_action_apply_mask::perform_without_undo(map_context& mc) const
{
	mc.get_map().overlay(mask_, config(), 0, 0, true);
	mc.set_needs_terrain_rebuild();
}

editor_action_create_mask* editor_action_create_mask::clone() const
{
	return new editor_action_create_mask(*this);
}
void editor_action_create_mask::perform_without_undo(map_context& mc) const
{
	mc.get_map() = editor_map(mc.get_map().mask_to(target_));
	mc.set_needs_terrain_rebuild();
}

editor_action_shuffle_area* editor_action_shuffle_area::clone() const
{
	return new editor_action_shuffle_area(*this);
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
	std::vector<map_location> shuffle;
	std::copy(area_.begin(), area_.end(), std::inserter(shuffle, shuffle.begin()));
	std::random_shuffle(shuffle.begin(), shuffle.end());
	std::vector<map_location>::const_iterator shuffle_it = shuffle.begin();
	std::set<map_location>::const_iterator orig_it = area_.begin();
	while (orig_it != area_.end()) {
		t_translation::t_terrain tmp = mc.get_map().get_terrain(*orig_it);
		mc.draw_terrain(mc.get_map().get_terrain(*shuffle_it), *orig_it);
		mc.draw_terrain(tmp, *shuffle_it);
		orig_it++;
		shuffle_it++;
	}
	mc.set_needs_terrain_rebuild();
}

} //end namespace editor
