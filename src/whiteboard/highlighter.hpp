/*
 Copyright (C) 2010 - 2018 by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
 Part of the Battle for Wesnoth Project http://www.wesnoth.org

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY.

 See the COPYING file for more details.
 */

/**
 * @file
 */

#pragma once

#include "visitor.hpp"
#include "map/location.hpp"

static lg::log_domain log_whiteboard_highlight("whiteboard/highlight");
#define ERR_WB_H LOG_STREAM(err, log_whiteboard_highlight)
#define WRN_WB_H LOG_STREAM(warn, log_whiteboard_highlight)
#define LOG_WB_H LOG_STREAM(info, log_whiteboard_highlight)
#define DBG_WB_H LOG_STREAM(debug, log_whiteboard_highlight)

namespace wb
{

/**
 * Class that handles highlighting planned actions as you hover over them
 * and determine the right target for contextual execution.
 */
class highlighter
{

public:
	highlighter(side_actions_ptr side_actions);
	virtual ~highlighter();

	void set_mouseover_hex(const map_location& hex);
	const map_location& get_mouseover_hex() const {return mouseover_hex_; }

	void highlight();
	void clear();

	action_ptr get_execute_target();
	action_ptr get_delete_target();
	action_ptr get_bump_target();
	unit_ptr get_selection_target();

	/// @return the action that currently receives the highlight focus
	weak_action_ptr get_main_highlight() { return main_highlight_; }
	typedef std::deque<weak_action_ptr> secondary_highlights_t;
	/// @return the collection of actions that are highlighted but don't have the focus
	secondary_highlights_t get_secondary_highlights() { return secondary_highlights_; }

	void set_selection_candidate(unit_ptr candidate) { selection_candidate_ = candidate; }

private:
	unit_map& get_unit_map();
	/** Unhighlight a given action (main or secondary). */
	class unhighlight_visitor;

	/** Highlight the given main action. */
	class highlight_main_visitor;

	/** Highlight the given secondary action. */
	class highlight_secondary_visitor;

	void unhighlight();
	void find_main_highlight();
	void find_secondary_highlights();

	/** Redraw the given move action when needed. */
	void last_action_redraw(move_ptr);

	map_location mouseover_hex_;
	std::set<map_location> exclusive_display_hexes_;
	unit_ptr owner_unit_;
	unit_ptr selection_candidate_;

	weak_action_ptr selected_action_;
	weak_action_ptr main_highlight_;
	secondary_highlights_t secondary_highlights_;

	side_actions_ptr side_actions_;
};

class highlighter::highlight_main_visitor: public visitor {
public:
	highlight_main_visitor(highlighter &h): highlighter_(h) {}
	void visit(move_ptr);
	void visit(attack_ptr);
	void visit(recruit_ptr);
	/// @todo: find some suitable effect for mouseover on planned recall.
	void visit(recall_ptr){}
	void visit(suppose_dead_ptr){}
private:
	highlighter &highlighter_;
};

class highlighter::highlight_secondary_visitor: public visitor {
public:
	highlight_secondary_visitor(highlighter &h): highlighter_(h) {}
	void visit(move_ptr);
	void visit(attack_ptr);
	void visit(recruit_ptr){}
	void visit(recall_ptr){}
	void visit(suppose_dead_ptr){}
private:
	highlighter &highlighter_;
};

class highlighter::unhighlight_visitor: public visitor {
public:
	unhighlight_visitor(highlighter &h): highlighter_(h) {}
	void visit(move_ptr);
	void visit(attack_ptr);
	void visit(recruit_ptr){}
	void visit(recall_ptr);
	void visit(suppose_dead_ptr){}
private:
	highlighter &highlighter_;
};

} // end namespace wb
