/*
   Copyright (C) 2008 - 2016 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#define GETTEXT_DOMAIN "wesnoth-editor"

#include "editor/action/action_base.hpp"
#include "editor_map.hpp"
#include "formula_string_utils.hpp"

#include "display.hpp"
#include "gettext.hpp"
#include "map_exception.hpp"
#include "map_label.hpp"
#include "wml_exception.hpp"

#include "terrain_type_data.hpp"

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>

namespace editor {

editor_map_load_exception wrap_exc(const char* type, const std::string& e_msg, const std::string& filename)
{
	WRN_ED << type << " error in load map " << filename << ": " << e_msg << std::endl;
	utils::string_map symbols;
	symbols["type"] = type;
	const char* error_msg = "There was an error ($type) while loading the file:";
	std::string msg = vgettext(error_msg, symbols);
	msg += "\n";
	msg += e_msg;
	return editor_map_load_exception(filename, msg);
}

editor_map::editor_map(const config& terrain_cfg)
	: gamemap(boost::make_shared<terrain_type_data>(terrain_cfg), "")
	, selection_()
{
}

editor_map::editor_map(const config& terrain_cfg, const std::string& data)
	: gamemap(boost::make_shared<terrain_type_data>(terrain_cfg), data)
	, selection_()
{
	sanity_check();
}

editor_map editor_map::from_string(const config& terrain_cfg, const std::string& data)
{
	try {
		return editor_map(terrain_cfg, data);
	} catch (incorrect_map_format_error& e) {
		throw wrap_exc("format", e.message, "");
	} catch (twml_exception& e) {
		throw wrap_exc("wml", e.user_message, "");
	} catch (config::error& e) {
		throw wrap_exc("config", e.message, "");
	}
}

editor_map::editor_map(const config& terrain_cfg, size_t width, size_t height, const t_translation::t_terrain & filler)
	: gamemap(boost::make_shared<terrain_type_data>(terrain_cfg), t_translation::write_game_map(
		t_translation::t_map(width + 2, t_translation::t_list(height + 2, filler))))
	, selection_()
{
	sanity_check();
}

editor_map::editor_map(const gamemap& map)
	: gamemap(map)
	, selection_()
{
	sanity_check();
}

editor_map::~editor_map()
{
}

void editor_map::sanity_check()
{
	int errors = 0;
	if (total_width() != static_cast<int>(tiles_.size())) {
		ERR_ED << "total_width is " << total_width() << " but tiles_.size() is " << tiles_.size() << std::endl;
		++errors;
	}
	if (total_height() != static_cast<int>(tiles_[0].size())) {
		ERR_ED << "total_height is " << total_height() << " but tiles_[0].size() is " << tiles_.size() << std::endl;
		++errors;
	}
	if (w() + 2 * border_size() != total_width()) {
		ERR_ED << "h is " << h_ << " and border_size is " << border_size() << " but total_width is " << total_width() << std::endl;
		++errors;
	}
	if (h() + 2 * border_size() != total_height()) {
		ERR_ED << "w is " << w_ << " and border_size is " << border_size() << " but total_height is " << total_height() << std::endl;
		++errors;
	}
	for (size_t i = 1; i < tiles_.size(); ++i) {
		if (tiles_[i].size() != tiles_[0].size()) {
			ERR_ED << "tiles_[ " << i << "] has size() " << tiles_[i].size() << " but tiles[0] has size() " << tiles_[0].size() << std::endl;
			++errors;
		}
	}
	BOOST_FOREACH(const map_location& loc, selection_) {
		if (!on_board_with_border(loc)) {
			ERR_ED << "Off-map tile in selection: " << loc << std::endl;
		}
	}
	if (errors) {
		throw editor_map_integrity_error();
	}
}

std::set<map_location> editor_map::get_contiguous_terrain_tiles(const map_location& start) const
{
	t_translation::t_terrain terrain = get_terrain(start);
	std::set<map_location> result;
	std::deque<map_location> queue;
	result.insert(start);
	queue.push_back(start);
	//this is basically a breadth-first search along adjacent hexes
	do {
		map_location adj[6];
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

std::set<map_location> editor_map::set_starting_position_labels(display& disp)
{
	std::set<map_location> label_locs;
	std::string label = _("Player");
	label += " ";
	for (int i = 0, size = starting_positions_.size(); i < size; ++i) {
		if (starting_positions_[i].valid()) {
			disp.labels().set_label(starting_positions_[i], label + lexical_cast<std::string>(i + 1));
			label_locs.insert(starting_positions_[i]);
		}
	}
	return label_locs;
}

bool editor_map::in_selection(const map_location& loc) const
{
	return selection_.find(loc) != selection_.end();
}

bool editor_map::add_to_selection(const map_location& loc)
{
	return on_board_with_border(loc) ? selection_.insert(loc).second : false;
}

bool editor_map::set_selection(const std::set<map_location>& area)
{
	clear_selection();
	BOOST_FOREACH(const map_location& loc, area) {
		if (!add_to_selection(loc))
			return false;
	}
	return true;
}

bool editor_map::remove_from_selection(const map_location& loc)
{
	return selection_.erase(loc) != 0;
}

void editor_map::clear_selection()
{
	selection_.clear();
}

void editor_map::invert_selection()
{
	std::set<map_location> new_selection;
	for (int x = -1; x < w() + 1; ++x) {
		for (int y = -1; y < h() + 1; ++y) {
			if (selection_.find(map_location(x, y)) == selection_.end()) {
				new_selection.insert(map_location(x, y));
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

bool editor_map::everything_selected() const
{
	LOG_ED << selection_.size() << " " << total_width() * total_height() << "\n";
	return static_cast<int>(selection_.size()) == total_width() * total_height();
}

void editor_map::resize(int width, int height, int x_offset, int y_offset,
	const t_translation::t_terrain & filler)
{
	int old_w = w();
	int old_h = h();
	if (old_w == width && old_h == height && x_offset == 0 && y_offset == 0) {
		return;
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
		for(size_t i = 0; i < starting_positions_.size(); ++i) {
			if(starting_positions_[i] != map_location()) {
				starting_positions_[i].x -= x_offset;
				starting_positions_[i].y -= y_offset;
			}
		}
	}
	sanity_check();
}

gamemap editor_map::mask_to(const gamemap& target) const
{
	if (target.w() != w() || target.h() != h()) {
		throw editor_action_exception(_("The size of the target map is different from the current map"));
	}
	gamemap mask(target);
	map_location iter;
	for (iter.x = -border_size(); iter.x < w() + border_size(); ++iter.x) {
		for (iter.y = -border_size(); iter.y < h() + border_size(); ++iter.y) {
			if (target.get_terrain(iter) == get_terrain(iter)) {
				mask.set_terrain(iter, t_translation::FOGGED);
			}
		}
	}
	return mask;
}

bool editor_map::same_size_as(const gamemap& other) const
{
	return h() == other.h()
		&& w() == other.w();
}

t_translation::t_list editor_map::clone_column(int x, const t_translation::t_terrain & filler)
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

void editor_map::expand_right(int count, const t_translation::t_terrain & filler)
{
	int w = tiles_.size();
	for (int x = 0; x < count; ++x) {
		tiles_.push_back(clone_column(w - 1	, filler));
	}
	w_ += count;
	total_width_ += count;
}

void editor_map::expand_left(int count, const t_translation::t_terrain & filler)
{
	for (int x = 0; x < count; ++x) {
		tiles_.insert(tiles_.begin(), 1, clone_column(0, filler));
		clear_border_cache();
	}
	w_ += count;
	total_width_ += count;
}

void editor_map::expand_top(int count, const t_translation::t_terrain & filler)
{
	for (int y = 0; y < count; ++y) {
		for (int x = 0; x < static_cast<int>(tiles_.size()); ++x) {
			t_translation::t_terrain terrain =
				filler != t_translation::NONE_TERRAIN ?
				filler :
				tiles_[x][0];
			assert(terrain != t_translation::NONE_TERRAIN);
			tiles_[x].insert(tiles_[x].begin(), 1, terrain);
			clear_border_cache();
		}
	}
	h_ += count;
	total_height_ += count;
}

void editor_map::expand_bottom(int count, const t_translation::t_terrain & filler)
{
	int h = tiles_[1].size();
	for (int y = 0; y < count; ++y) {
		for (int x = 0; x < static_cast<int>(tiles_.size()); ++x) {
			t_translation::t_terrain terrain =
				filler != t_translation::NONE_TERRAIN ?
				filler :
				tiles_[x][h - 1];
			assert(terrain != t_translation::NONE_TERRAIN);
			tiles_[x].push_back(terrain);
		}
	}
	h_ += count;
	total_height_ += count;
}

void editor_map::shrink_right(int count)
{
	if(count < 0 || count > static_cast<int>(tiles_.size())) {
		throw editor_map_operation_exception();
	}
	tiles_.resize(tiles_.size() - count);
	w_ -= count;
	total_width_ -= count;
}

void editor_map::shrink_left(int count)
{
	if(count < 0 || count > static_cast<int>(tiles_.size())) {
		throw editor_map_operation_exception();
	}
	tiles_.erase(tiles_.begin(), tiles_.begin() + count);
	w_ -= count;
	total_width_ -= count;
}

void editor_map::shrink_top(int count)
{
	if(count < 0 || count > static_cast<int>(tiles_[0].size())) {
		throw editor_map_operation_exception();
	}
	for (size_t x = 0; x < tiles_.size(); ++x) {
		tiles_[x].erase(tiles_[x].begin(), tiles_[x].begin() + count);
	}
	h_ -= count;
	total_height_ -= count;
}

void editor_map::shrink_bottom(int count)
{
	if(count < 0 || count > static_cast<int>(tiles_[0].size())) {
		throw editor_map_operation_exception();
	}
	for (size_t x = 0; x < tiles_.size(); ++x) {
		tiles_[x].erase(tiles_[x].end() - count, tiles_[x].end());
	}
	h_ -= count;
	total_height_ -= count;
}



} //end namespace editor
