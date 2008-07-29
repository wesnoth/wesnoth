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

#include "../display.hpp"
#include "../filesystem.hpp"
#include "../foreach.hpp"
#include "../gettext.hpp"
#include "../pathutils.hpp"

#include <cassert>
#include <deque>


namespace editor2 {

const int editor_map::max_action_stack_size_ = 100;

editor_map::editor_map(const config& terrain_cfg, const std::string& data)
: gamemap(terrain_cfg, data), filename_(), actions_since_save_(0)
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

void editor_map::clear_starting_position_labels(display& disp) const
{
	foreach (const gamemap::location& loc, startingPositions_) {
		if (loc.valid()) {
			disp.labels().set_label(loc, "");
		}
	}
}
	
void editor_map::set_starting_position_labels(display& disp) const
{
	std::string label = _("Player");
	label += " ";
	for (int i = 1; i <= gamemap::MAX_PLAYERS; i++) {
		if (startingPositions_[i].valid()) {
			disp.labels().set_label(startingPositions_[i], label + lexical_cast<std::string>(i));
		}
	}
}

bool editor_map::save()
{
	std::string data = write();
	write_file(get_filename(), data);
	actions_since_save_ = 0;

	return true;
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

void editor_map::resize(int width, int height, int x_offset, int y_offset,
	bool do_expand, t_translation::t_terrain filler)
{
	int old_w = w();
	int old_h = h();
	if (old_w == width && old_h == height && x_offset == 0 && y_offset == 0) {
		return;
	}
	if (do_expand) {
		filler = t_translation::NONE_TERRAIN;
	}

	// Determine the amount of resizing is required
	const int left_resize = -x_offset;
	const int right_resize = (width - old_w) + x_offset;
	const int top_resize = -y_offset;
	const int bottom_resize = (height - old_h) + y_offset;

	if(right_resize > 0) {
		expand_right(right_resize, filler);
	} else if(right_resize < 0) {
		shrink_right(-right_resize);
	}
	if(bottom_resize > 0) {
		expand_bottom(bottom_resize, filler);
	} else if(bottom_resize < 0) {
		shrink_bottom(-bottom_resize);
	}
	if(left_resize > 0) {
		expand_left(left_resize, filler);
	} else if(left_resize < 0) {
		shrink_left(-left_resize);
	}
	if(top_resize > 0) {
		expand_top(top_resize, filler);
	} else if(top_resize < 0) {
		shrink_top(-top_resize);
	}

	// fix the starting positions
	if(x_offset || y_offset) {
		for(size_t i = 0; i < MAX_PLAYERS+1; ++i) {
			if(startingPositions_[i] != gamemap::location()) {
				startingPositions_[i].x -= x_offset;
				startingPositions_[i].y -= y_offset;
			}
		}
	}
}

void editor_map::flip_x()
{
	// Due to the hexes we need some mirror tricks when mirroring over the
	// X axis. We resize the map and fill it. The odd columns will be extended
	// with the data in row 0 the even columns are extended with the data in
	// the last row
	const size_t middle = (tiles_[0].size() / 2); // the middle if reached we flipped all
	const size_t end = tiles_[0].size() - 1; // the last row _before_ resizing
	for(size_t x = 0; x < tiles_.size(); ++x) {
		if(x % 2) {
			// odd lines
			tiles_[x].resize(tiles_[x].size() + 1, tiles_[x][0]);
			for(size_t y1 = 0, y2 = end; y1 < middle; ++y1, --y2) {
				swap_starting_position(x, y1, x, y2);
				std::swap(tiles_[x][y1], tiles_[x][y2]);
			}
		} else {
			// even lines
			tiles_[x].resize(tiles_[x].size() + 1, tiles_[x][end]);
			for(size_t y1 = 0, y2 = end + 1; y1 < middle; ++y1, --y2) {
				swap_starting_position(x, y1, x, y2);
				std::swap(tiles_[x][y1], tiles_[x][y2]);
			}

		}
	}
}

void editor_map::flip_y()
{
	// Flipping on the Y axis requires no resize,
	// so the code is much simpler.
	const size_t middle = (tiles_.size() / 2);
	const size_t end = tiles_.size() - 1;
	for(size_t y = 0; y < tiles_[0].size(); ++y) {
		for(size_t x1 = 0, x2 = end; x1 < middle; ++x1, --x2) {
			swap_starting_position(x1, y, x2, y);
			std::swap(tiles_[x1][y], tiles_[x2][y]);
		}
	}
}

void editor_map::set_starting_position(int pos, const location& loc)
{
	startingPositions_[pos] = loc;
}

void editor_map::swap_starting_position(int x1, int y1, int x2, int y2)
{
	int pos1 = is_starting_position(location(x1, y1));
	int pos2 = is_starting_position(location(x2, y2));
	if(pos1 != -1) {
		set_starting_position(pos1 + 1, location(x2, y2));
	}
	if(pos2 != -1) {
		set_starting_position(pos2 + 1, location(x1, y1));
	}
}

t_translation::t_list editor_map::clone_column(int x, t_translation::t_terrain filler)
{
	int h = tiles_[1].size();
	t_translation::t_list column(h);
	for (int y = 0; y < h; ++y) {
		column[y] =
			filler != t_translation::NONE_TERRAIN ?
			filler :
			tiles_[x][y];
		assert(column[y] != t_translation::NONE_TERRAIN);
	}
	return column;
}

void editor_map::expand_right(int count, t_translation::t_terrain filler)
{
	int w = tiles_.size();
	for (int x = 0; x < count; ++x) {
		tiles_.push_back(clone_column(w, filler));
	}
}

void editor_map::expand_left(int count, t_translation::t_terrain filler)
{
	for (int x = 0; x < count; ++x) {
		tiles_.insert(tiles_.begin(), 1, clone_column(0, filler));
		clear_border_cache();
	}
}

void editor_map::expand_top(int count, t_translation::t_terrain filler)
{
	for (int y = 0; y < count; ++y) {
		for (int x = 0; x < tiles_.size(); ++x) {
			t_translation::t_terrain terrain =
				filler != t_translation::NONE_TERRAIN ?
				filler :
				tiles_[x][0];
			assert(terrain != t_translation::NONE_TERRAIN);
			tiles_[x].insert(tiles_[x].begin(), 1, terrain);
			clear_border_cache();
		}
	}
}

void editor_map::expand_bottom(int count, t_translation::t_terrain filler)
{
	int h = tiles_[1].size();
	for (int y = 0; y < count; ++y) {
		for (int x = 0; x < tiles_.size(); ++x) {
			t_translation::t_terrain terrain =
				filler != t_translation::NONE_TERRAIN ?
				filler :
				tiles_[x][h];
			assert(terrain != t_translation::NONE_TERRAIN);
			tiles_[x].push_back(terrain);
		}
	}
}

void editor_map::shrink_right(int count)
{
	if(count < 0 || count > tiles_.size()) {
		throw editor_map_operation_exception();
	}
	tiles_.resize(tiles_.size() - count);
}

void editor_map::shrink_left(int count)
{
	if(count < 0 || count > tiles_.size()) {
		throw editor_map_operation_exception();
	}
	tiles_.erase(tiles_.begin(), tiles_.begin() + count);
}

void editor_map::shrink_top(int count)
{
	if(count < 0 || count > tiles_[0].size()) {
		throw editor_map_operation_exception();
	}
	for (size_t x = 0; x < tiles_.size(); ++x) {
		tiles_[x].erase(tiles_[x].begin(), tiles_[x].begin() + count);
	}
}

void editor_map::shrink_bottom(int count)
{
	if(count < 0 || count > tiles_[0].size()) {
		throw editor_map_operation_exception();
	}
	for (size_t x = 0; x < tiles_.size(); ++x) {
		tiles_[x].erase(tiles_[x].end() - count, tiles_[x].end());
	}
}



} //end namespace editor2
