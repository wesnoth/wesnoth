/*
   Copyright (C) 2008 - 2018 by Tomasz Sniatowski <kailoran@gmail.com>
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
#include "editor/map/editor_map.hpp"

#include "display.hpp"
#include "formula/string_utils.hpp"
#include "gettext.hpp"
#include "map/exception.hpp"
#include "map/label.hpp"
#include "wml_exception.hpp"

#include "terrain/type_data.hpp"

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
	: gamemap(std::make_shared<terrain_type_data>(terrain_cfg), "")
	, selection_()
{
}

editor_map::editor_map(const config& terrain_cfg, const std::string& data)
	: gamemap(std::make_shared<terrain_type_data>(terrain_cfg), data)
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
	} catch (wml_exception& e) {
		throw wrap_exc("wml", e.user_message, "");
	} catch (config::error& e) {
		throw wrap_exc("config", e.message, "");
	}
}

editor_map::editor_map(const config& terrain_cfg, size_t width, size_t height, const t_translation::terrain_code & filler)
	: gamemap(std::make_shared<terrain_type_data>(terrain_cfg), t_translation::write_game_map(t_translation::ter_map(width + 2, height + 2, filler)))
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
	if (total_width() != tiles_.w) {
		ERR_ED << "total_width is " << total_width() << " but tiles_.size() is " << tiles_.w << std::endl;
		++errors;
	}
	if (total_height() != tiles_.h) {
		ERR_ED << "total_height is " << total_height() << " but tiles_[0].size() is " << tiles_.h << std::endl;
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
	for (const map_location& loc : selection_) {
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
	t_translation::terrain_code terrain = get_terrain(start);
	std::set<map_location> result;
	std::deque<map_location> queue;
	result.insert(start);
	queue.push_back(start);
	//this is basically a breadth-first search along adjacent hexes
	do {
		adjacent_loc_array_t adj;
		get_adjacent_tiles(queue.front(), adj.data());
		for (unsigned i = 0; i < adj.size(); ++i) {
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
	std::string label;


	for (const auto& pair : starting_positions_.left) {

		bool is_number = std::find_if(pair.first.begin(), pair.first.end(), [](char c) { return !std::isdigit(c); }) == pair.first.end();
		if (is_number) {
			label = vgettext("Player $side_num", utils::string_map{ { "side_num", pair.first } });
		}
		else {
			label = pair.first;
		}

		disp.labels().set_label(pair.second, label);
		label_locs.insert(pair.second);
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
	for (const map_location& loc : area) {
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
				new_selection.emplace(x, y);
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
	const t_translation::terrain_code & filler)
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
		for (auto it = starting_positions_.left.begin(); it != starting_positions_.left.end(); ++it) {
			starting_positions_.left.modify_data(it, [=](t_translation::coordinate & loc) { loc.add(-x_offset, -y_offset); });
		}
	}

	villages_.clear();

	//
	// NOTE: I'm not sure how inefficient it is to check every loc for its village-ness as
	// opposed to operating on the villages_ vector itself and figuring out how to handle
	// villages on the map border. Essentially, it's possible to simply remove all members
	// from villages_ that are no longer on the map after a resize (including those that
	// land on a border), but that doesn't account for villages that were *on* the border
	// prior to resizing. Those should be included. As a catch-all fix, I just check every
	// hex. It's possible that any more complex shenanigans would be even more inefficient.
	//
	// -- vultraz, 2018-02-25
	//
	for_each_loc([this](const map_location& loc) {
		if(is_village(loc)) {
			villages_.push_back(loc);
		}
	});

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

void editor_map::expand_right(int count, const t_translation::terrain_code & filler)
{
	t_translation::ter_map tiles_new(tiles_.w + count, tiles_.h);
	w_ += count;
	for (int x = 0, x_end = tiles_.w; x != x_end; ++x) {
		for (int y = 0, y_end = tiles_.h; y != y_end; ++y) {
			tiles_new.get(x, y) = tiles_.get(x, y);
		}
	}
	for (int x = tiles_.w, x_end = tiles_.w + count; x != x_end; ++x) {
		for (int y = 0, y_end = tiles_.h; y != y_end; ++y) {
			tiles_new.get(x, y) = filler == t_translation::NONE_TERRAIN ? tiles_.get(tiles_.w - 1, y) : filler;
		}
	}
	tiles_ = std::move(tiles_new);
}

void editor_map::expand_left(int count, const t_translation::terrain_code & filler)
{
	t_translation::ter_map tiles_new(tiles_.w + count, tiles_.h);
	w_ += count;
	for (int x = 0, x_end = tiles_.w; x != x_end; ++x) {
		for (int y = 0, y_end = tiles_.h; y != y_end; ++y) {
			tiles_new.get(x + count, y) = tiles_.get(x, y);
		}
	}
	for (int x = 0, x_end = count; x != x_end; ++x) {
		for (int y = 0, y_end = tiles_.h; y != y_end; ++y) {
			tiles_new.get(x, y) = filler == t_translation::NONE_TERRAIN ? tiles_.get(0, y) : filler;
		}
	}
	tiles_ = std::move(tiles_new);
}

void editor_map::expand_top(int count, const t_translation::terrain_code & filler)
{
	t_translation::ter_map tiles_new(tiles_.w, tiles_.h + count);
	h_ += count;
	for (int x = 0, x_end = tiles_.w; x != x_end; ++x) {
		for (int y = 0, y_end = tiles_.h; y != y_end; ++y) {
			tiles_new.get(x, y + count) = tiles_.get(x, y);
		}
	}
	for (int x = 0, x_end = tiles_.w; x != x_end; ++x) {
		for (int y = 0, y_end = count; y != y_end; ++y) {
			tiles_new.get(x, y) = filler == t_translation::NONE_TERRAIN ? tiles_.get(x, 0) : filler;
		}
	}
	tiles_ = std::move(tiles_new);
}

void editor_map::expand_bottom(int count, const t_translation::terrain_code & filler)
{
	t_translation::ter_map tiles_new(tiles_.w, tiles_.h + count);
	h_ += count;
	for (int x = 0, x_end = tiles_.w; x != x_end; ++x) {
		for (int y = 0, y_end = tiles_.h; y != y_end; ++y) {
			tiles_new.get(x, y) = tiles_.get(x, y);
		}
	}
	for (int x = 0, x_end = tiles_.w; x != x_end; ++x) {
		for (int y = tiles_.h, y_end = tiles_.h + count; y != y_end; ++y) {
			tiles_new.get(x, y) = filler == t_translation::NONE_TERRAIN ? tiles_.get(x, tiles_.h - 1) : filler;
		}
	}
	tiles_ = std::move(tiles_new);
}

void editor_map::shrink_right(int count)
{
	if(count < 0 || count > tiles_.w) {
		throw editor_map_operation_exception();
	}
	t_translation::ter_map tiles_new(tiles_.w - count, tiles_.h);
	for (int x = 0, x_end = tiles_new.w; x != x_end; ++x) {
		for (int y = 0, y_end = tiles_new.h; y != y_end; ++y) {
			tiles_new.get(x, y) = tiles_.get(x, y);
		}
	}
	w_ -= count;
	tiles_ = std::move(tiles_new);
}

void editor_map::shrink_left(int count)
{
	if (count < 0 || count > tiles_.w) {
		throw editor_map_operation_exception();
	}
	t_translation::ter_map tiles_new(tiles_.w - count, tiles_.h);
	for (int x = 0, x_end = tiles_new.w; x != x_end; ++x) {
		for (int y = 0, y_end = tiles_new.h; y != y_end; ++y) {
			tiles_new.get(x, y) = tiles_.get(x + count, y);
		}
	}
	w_ -= count;
	tiles_ = std::move(tiles_new);
}

void editor_map::shrink_top(int count)
{
	if (count < 0 || count > tiles_.h) {
		throw editor_map_operation_exception();
	}
	t_translation::ter_map tiles_new(tiles_.w, tiles_.h - count);
	for (int x = 0, x_end = tiles_new.w; x != x_end; ++x) {
		for (int y = 0, y_end = tiles_new.h; y != y_end; ++y) {
			tiles_new.get(x, y) = tiles_.get(x, y + count);
		}
	}
	h_ -= count;
	tiles_ = std::move(tiles_new);
}

void editor_map::shrink_bottom(int count)
{
	if (count < 0 || count > tiles_.h) {
		throw editor_map_operation_exception();
	}
	t_translation::ter_map tiles_new(tiles_.w, tiles_.h - count);
	for (int x = 0, x_end = tiles_new.w; x != x_end; ++x) {
		for (int y = 0, y_end = tiles_new.h; y != y_end; ++y) {
			tiles_new.get(x, y) = tiles_.get(x, y);
		}
	}
	h_ -= count;
	tiles_ = std::move(tiles_new);
}



} //end namespace editor
