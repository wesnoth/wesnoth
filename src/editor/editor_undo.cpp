/*
  Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
  Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License.
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

map_undo_action::map_undo_action(const gamemap::TERRAIN& old_tr,
								 const gamemap::TERRAIN& new_tr,
								 const gamemap::location& lc){
	add_terrain(old_tr, new_tr, lc);
	undo_type_ = REGULAR;
}

map_undo_action::map_undo_action() {
	undo_type_ = REGULAR;
}

const std::map<gamemap::location,gamemap::TERRAIN>& map_undo_action::undo_terrains() const {
	return old_terrain_;
}

const std::map<gamemap::location,gamemap::TERRAIN>& map_undo_action::redo_terrains() const {
	return new_terrain_;
}

const std::set<gamemap::location> map_undo_action::undo_selection() const {
	return old_selection_;
}
	
const std::set<gamemap::location> map_undo_action::redo_selection() const {
	return new_selection_;
}

void map_undo_action::add_terrain(const gamemap::TERRAIN& old_tr,
								  const gamemap::TERRAIN& new_tr,
								  const gamemap::location& lc) {
	old_terrain_[lc] = old_tr;
	new_terrain_[lc] = new_tr;
}

void map_undo_action::set_selection(const std::set<gamemap::location> &new_selection,
									const std::set<gamemap::location> &old_selection) {
	new_selection_ = new_selection;
	old_selection_ = old_selection;
}

std::string map_undo_action::old_map_data() const {
	return old_map_data_;
}

std::string map_undo_action::new_map_data() const {
	return new_map_data_;
}

void map_undo_action::set_map_data(const std::string &old_data,
								   const std::string &new_data) {
	old_map_data_ = old_data;
	new_map_data_ = new_data;
}

void map_undo_action::set_type(const UNDO_TYPE new_type) {
	undo_type_ = new_type;
}

map_undo_action::UNDO_TYPE map_undo_action::undo_type() const {
	return undo_type_;
}


void add_undo_action(map_undo_action &action) {
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
