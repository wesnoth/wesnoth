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

#include "action_base.hpp"
#include "editor_map.hpp"
#include "map_context.hpp"

#include "../display.hpp"
#include "../filesystem.hpp"
#include "../foreach.hpp"
#include "../gettext.hpp"
#include "../pathutils.hpp"

#include <cassert>
#include <deque>


namespace editor2 {

const int map_context::max_action_stack_size_ = 100;

map_context::map_context(const editor_map& map)
: map_(map), filename_(), actions_since_save_(0),
needs_reload_(false), needs_terrain_rebuild_(false), 
needs_labels_reset_(false), everything_changed_(false)
{
}

map_context::~map_context()
{
	clear_stack(undo_stack_);
	clear_stack(redo_stack_);
}

void map_context::draw_terrain(t_translation::t_terrain terrain, 
	const gamemap::location& loc, bool one_layer_only)
{
    if (!one_layer_only) {
        terrain = map_.get_terrain_info(terrain).terrain_with_default_base();
    }
	t_translation::t_terrain old_terrain = map_.get_terrain(loc);
	if (terrain != old_terrain) {
		if (terrain.base == t_translation::NO_LAYER) {
			map_.set_terrain(loc, terrain, gamemap::OVERLAY);
		} else if (one_layer_only) {
			map_.set_terrain(loc, terrain, gamemap::BASE);
		} else {
			map_.set_terrain(loc, terrain);
		}
		add_changed_location(loc);
	}
}

void map_context::draw_terrain(t_translation::t_terrain terrain, 
	const std::set<gamemap::location>& locs, bool one_layer_only)
{
    if (!one_layer_only) {
        terrain = map_.get_terrain_info(terrain).terrain_with_default_base();
    }
	foreach (const gamemap::location& loc, locs) {
		t_translation::t_terrain old_terrain = map_.get_terrain(loc);
		if (terrain != old_terrain) {
			if (terrain.base == t_translation::NO_LAYER) {
				map_.set_terrain(loc, terrain, gamemap::OVERLAY);
			} else if (one_layer_only) {
				map_.set_terrain(loc, terrain, gamemap::BASE);
			} else {
				map_.set_terrain(loc, terrain);
			}
			add_changed_location(loc);
		}
	}
}

void map_context::clear_changed_locations()
{
	everything_changed_ = false;
	changed_locations_.clear();
}

void map_context::add_changed_location(const gamemap::location& loc)
{
	if (!everything_changed()) {
		changed_locations_.insert(loc);
	}
}

void map_context::add_changed_location(const std::set<gamemap::location>& locs)
{
	if (!everything_changed()) {
		changed_locations_.insert(locs.begin(), locs.end());
	}
}

void map_context::set_everything_changed()
{
	everything_changed_ = true;
}

bool map_context::everything_changed() const
{
	return everything_changed_;
}

void map_context::clear_starting_position_labels(display& disp)
{
	foreach (const gamemap::location& loc, starting_position_label_locs_) {
		disp.labels().set_label(loc, "");
	}
	starting_position_label_locs_.clear();
}
	
void map_context::set_starting_position_labels(display& disp)
{
	std::set<gamemap::location> new_label_locs = map_.set_starting_position_labels(disp);
	starting_position_label_locs_.insert(new_label_locs.begin(), new_label_locs.end());
}

bool map_context::save()
{
	std::string data = map_.write();
	write_file(get_filename(), data);
	actions_since_save_ = 0;
	return true;
}

void map_context::perform_action(const editor_action& action)
{
	LOG_ED << "Performing action " << action.get_id() << ", actions count is " << action.get_instance_count() << "\n";
	editor_action* undo = action.perform(*this);
	if (actions_since_save_ < 0) {
		//set to a value that will make it impossible to get to zero, as at this point
		//it is no longer possible to get back the original map state using undo/redo
		actions_since_save_ = undo_stack_.size() + 1;
	} else {
		++actions_since_save_;
	}
	undo_stack_.push_back(undo);
	trim_stack(undo_stack_);
	clear_stack(redo_stack_);
}
	
void map_context::perform_partial_action(const editor_action& action)
{
	LOG_ED << "Performing (partial) action " << action.get_id() << ", actions count is " << action.get_instance_count() << "\n";
	action.perform_without_undo(*this);
	clear_stack(redo_stack_);
}

bool map_context::modified() const
{
	return actions_since_save_ != 0;
}

bool map_context::can_undo() const
{
	return !undo_stack_.empty();
}

editor_action* map_context::last_undo_action()
{
	return undo_stack_.empty() ? NULL : undo_stack_.back();
}

bool map_context::can_redo() const
{
	return !redo_stack_.empty();
}

void map_context::undo()
{
	LOG_ED << "undo() beg, undo stack is " << undo_stack_.size() << ", redo stack " << redo_stack_.size() << "\n";
	if (can_undo()) {
		perform_action_between_stacks(undo_stack_, redo_stack_);
		--actions_since_save_;
	} else {
		WRN_ED << "undo() called with an empty undo stack\n";
	}
	LOG_ED << "undo() end, undo stack is " << undo_stack_.size() << ", redo stack " << redo_stack_.size() << "\n";
}

void map_context::redo()
{
	LOG_ED << "redo() beg, undo stack is " << undo_stack_.size() << ", redo stack " << redo_stack_.size() << "\n";
	if (can_redo()) {
		perform_action_between_stacks(redo_stack_, undo_stack_);
		++actions_since_save_;
	} else {
		WRN_ED << "redo() called with an empty redo stack\n";
	}
	LOG_ED << "redo() end, undo stack is " << undo_stack_.size() << ", redo stack " << redo_stack_.size() << "\n";
}

void map_context::trim_stack(action_stack& stack)
{
	if (stack.size() > max_action_stack_size_) {
		delete stack.front();
		stack.pop_front();
	}
}

void map_context::clear_stack(action_stack& stack)
{
	foreach (editor_action* a, stack) {
		delete a;
	}
	stack.clear();
}

void map_context::perform_action_between_stacks(action_stack& from, action_stack& to)
{
	assert(!from.empty());
	std::auto_ptr<editor_action> action(from.back());
	from.pop_back();
	editor_action* reverse_action = action->perform(*this);
	to.push_back(reverse_action);
	trim_stack(to);
}

} //end namespace editor2
