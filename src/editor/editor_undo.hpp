/*
  Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
  Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY.

  See the COPYING file for more details.
*/

// This module is used to manage actions that may be undone in the map
// editor.

#ifndef EDITOR_UNDO_H_INCLUDED
#define EDITOR_UNDO_H_INCLUDED

#include "../map.hpp"

#include <queue>
#include <vector>
#include <set>

namespace map_editor {

/// A saved action that may be undone.
class map_undo_action {
public:
	enum UNDO_TYPE { REGULAR, WHOLE_MAP };

	map_undo_action();
	
	map_undo_action(const gamemap::TERRAIN& old_tr,
					const gamemap::TERRAIN& new_tr,
					const gamemap::location& lc);
	
 	const std::map<gamemap::location,gamemap::TERRAIN>& undo_terrains() const;
	
 	const std::map<gamemap::location,gamemap::TERRAIN>& redo_terrains() const;
	
	const std::set<gamemap::location> undo_selection() const;

	const std::set<gamemap::location> redo_selection() const;
	
	void add_terrain(const gamemap::TERRAIN& old_tr,
					 const gamemap::TERRAIN& new_tr,
					 const gamemap::location& lc);
	
	void set_selection(const std::set<gamemap::location> &new_selection,
					   const std::set<gamemap::location> &old_selection);

	void set_map_data(const std::string &old_data,
					  const std::string &new_data);

	std::string new_map_data() const;
	std::string old_map_data() const;

	void set_type(const UNDO_TYPE new_type);

	UNDO_TYPE undo_type() const;
	
private:
	std::map<gamemap::location,gamemap::TERRAIN> old_terrain_;
	std::map<gamemap::location,gamemap::TERRAIN> new_terrain_;
	std::set<gamemap::location> new_selection_;
	std::set<gamemap::location> old_selection_;
	std::string new_map_data_;
	std::string old_map_data_;
	UNDO_TYPE undo_type_;
};
	
typedef std::deque<map_undo_action> map_undo_list;
	
/// Add an undo action to the undo stack. Resize the stack if it gets
/// larger than the maximum size. Add an operation to the number done
/// since save. If keep_selection is true, it indicates that the
/// selection has not changed and the currently selected terrain should
/// be kept if this action is redone/undone. Also clear the redo stack.
void add_undo_action(map_undo_action &action);

/// Return true if there exist any undo actions in the undo stack.
bool exist_undo_actions();
/// Return true if there exist any redo actions in the redo stack.
bool exist_redo_actions();


/// Remove, store in the redo stack and return the last undo action
/// stored.
map_undo_action pop_undo_action();

/// Remove, store in the undo stack and return the last redo action
/// stored.
map_undo_action pop_redo_action();

/// Clear all stored information about performed actions.
void clear_undo_actions();
	
}


#endif // EDITOR_UNDO_H_INCLUDED
