/* $Id$ */
/*
  Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
  Part of the Battle for Wesnoth Project http://www.wesnoth.org/

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License version 2
  or at your option any later version.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY.

  See the COPYING file for more details.
*/

#include "editor_undo.hpp"

namespace {
	const unsigned int undo_limit = 100;
	map_editor::map_undo_list undo_stack;
	map_editor::map_undo_list redo_stack;
}


namespace map_editor {

map_undo_action::map_undo_action() {
	terrain_set_ = false;
	selection_set_ = false;
	map_data_set_ = false;
	starting_locations_set_ = false;
}

const std::map<gamemap::location, t_translation::t_terrain>& map_undo_action::undo_terrains() const
{
	return old_terrain_;
}

const std::map<gamemap::location, t_translation::t_terrain>& map_undo_action::redo_terrains() const
{
	return new_terrain_;
}

const std::set<gamemap::location> map_undo_action::undo_selection() const {
	return old_selection_;
}

const std::set<gamemap::location> map_undo_action::redo_selection() const {
	return new_selection_;
}

std::string map_undo_action::old_map_data() const {
	return old_map_data_;
}

std::string map_undo_action::new_map_data() const {
	return new_map_data_;
}

const std::map<gamemap::location, int>& map_undo_action::undo_starting_locations() const {
	return old_starting_locations_;
}

const std::map<gamemap::location, int>& map_undo_action::redo_starting_locations() const {
	return new_starting_locations_;
}

void map_undo_action::add_terrain(const t_translation::t_terrain& old_tr,
								  const t_translation::t_terrain& new_tr,
								  const gamemap::location& lc)
{
	old_terrain_[lc] = old_tr;
	new_terrain_[lc] = new_tr;
	terrain_set_ = true;
}

bool map_undo_action::terrain_set() const {
	return terrain_set_;
}

void map_undo_action::set_selection(const std::set<gamemap::location> &old_selection,
									const std::set<gamemap::location> &new_selection) {
	old_selection_ = old_selection;
	new_selection_ = new_selection;
	selection_set_ = true;
}

bool map_undo_action::selection_set() const {
	return selection_set_;
}

void map_undo_action::set_map_data(const std::string &old_data,
								   const std::string &new_data) {
	old_map_data_ = old_data;
	new_map_data_ = new_data;
	map_data_set_ = true;
}

bool map_undo_action::map_data_set() const {
	return map_data_set_;
}

void map_undo_action::add_starting_location(const int old_side, const int new_side,
											const gamemap::location &old_loc,
											const gamemap::location &new_loc) {
	old_starting_locations_[old_loc] = old_side;
	new_starting_locations_[new_loc] = new_side;
	starting_locations_set_ = true;
}

bool map_undo_action::starting_location_set() const {
	return starting_locations_set_;
}

void add_undo_action(const map_undo_action &action) {
	undo_stack.push_back(action);
	if (undo_stack.size() > undo_limit) {
		undo_stack.pop_front();
	}
	// Adding an undo action means that an operations was performed,
	// which in turns means that no further redo may be performed.
	redo_stack.clear();
}

bool exist_undo_actions() {
	return !undo_stack.empty();
}

bool exist_redo_actions() {
	return !redo_stack.empty();
}

map_undo_action pop_undo_action() {
	map_undo_action action = undo_stack.back();
	undo_stack.pop_back();
	redo_stack.push_back(action);
	if (redo_stack.size() > undo_limit) {
		redo_stack.pop_front();
	}
	return action;
}

map_undo_action pop_redo_action() {
	map_undo_action action = redo_stack.back();
	redo_stack.pop_back();
	undo_stack.push_back(action);
	if (undo_stack.size() > undo_limit) {
		undo_stack.pop_front();
	}
	return action;
}

void clear_undo_actions() {
	undo_stack.clear();
	redo_stack.clear();
}


}
