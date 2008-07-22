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

#include "../filesystem.hpp"
#include "../foreach.hpp"
#include "../pathutils.hpp"

#include <cassert>
#include <deque>


namespace editor2 {

const int editor_map::max_action_stack_size_ = 100;

editor_map::editor_map(const config& terrain_cfg, const std::string& data)
: gamemap(terrain_cfg, data), filename_()
{
}

editor_map::~editor_map()
{
	clear_stack(undo_stack_);
	clear_stack(redo_stack_);
}

editor_map editor_map::new_map(const config& terrain_cfg, size_t width, size_t height, t_translation::t_terrain filler)
{
	const t_translation::t_list column(height, filler);
	const t_translation::t_map map(width, column);
	return editor_map(terrain_cfg, gamemap::default_map_header + t_translation::write_game_map(map));
}

std::set<gamemap::location> editor_map::get_contigious_terrain_tiles(const gamemap::location& start) const
{
	t_translation::t_terrain terrain = get_terrain(start);
	std::set<gamemap::location> result;
	std::deque<gamemap::location> queue;
	result.insert(start);
	queue.push_back(start);
	//this is basically a breadth-first search along adjacent hexes
	do {
		gamemap::location adj[6];
		get_adjacent_tiles(queue.front(), adj);
		for (int i = 0; i < 6; ++i) {
			if (on_board_with_border(adj[i]) && get_terrain(adj[i]) == terrain
			&& result.find(adj[i]) == result.end()) {
				result.insert(adj[i]);
				queue.push_back(adj[i]);
			}
		}
		queue.pop_front();
	} while (!queue.empty());
	return result;
}

bool editor_map::save()
{
	std::string data = write();
	write_file(get_filename(), data);
	actions_since_save_ = 0;
}

bool editor_map::in_selection(const gamemap::location& loc) const
{
	return selection_.find(loc) != selection_.end();
}

bool editor_map::add_to_selection(const gamemap::location& loc)
{
	return selection_.insert(loc).second;
}

bool editor_map::remove_from_selection(const gamemap::location& loc)
{
	return selection_.erase(loc);
}

void editor_map::clear_selection()
{
	selection_.clear();
}

void editor_map::invert_selection()
{
	std::set<gamemap::location> new_selection;
	for (int x = 0; x < w(); ++x) {
		for (int y = 0; y < h(); ++y) {
			if (selection_.find(gamemap::location(x, y)) == selection_.end()) {
				new_selection.insert(gamemap::location(x, y));
			}
		}
	}
	selection_.swap(new_selection);
}

void editor_map::select_all()
{
	clear_selection();
	invert_selection();
}

void editor_map::perform_action(const editor_action& action)
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
	
void editor_map::perform_partial_action(const editor_action& action)
{
	LOG_ED << "Performing (partial) action " << action.get_id() << ", actions count is " << action.get_instance_count() << "\n";
	action.perform_without_undo(*this);
	clear_stack(redo_stack_);
}

bool editor_map::modified() const
{
	return actions_since_save_ != 0;
}

bool editor_map::can_undo() const
{
	return !undo_stack_.empty();
}

editor_action* editor_map::last_undo_action()
{
	return undo_stack_.empty() ? NULL : undo_stack_.back();
}

bool editor_map::can_redo() const
{
	return !redo_stack_.empty();
}

void editor_map::undo()
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

void editor_map::redo()
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

void editor_map::trim_stack(action_stack& stack)
{
	if (stack.size() > max_action_stack_size_) {
		delete stack.front();
		stack.pop_front();
	}
}

void editor_map::clear_stack(action_stack& stack)
{
	foreach (editor_action* a, stack) {
		delete a;
	}
	stack.clear();
}

void editor_map::perform_action_between_stacks(action_stack& from, action_stack& to)
{
	assert(!from.empty());
	editor_action* action = from.back();
	from.pop_back();
	editor_action* reverse_action = action->perform(*this);
	to.push_back(reverse_action);
	trim_stack(to);
}


} //end namespace editor2
