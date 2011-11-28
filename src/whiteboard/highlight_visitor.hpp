/* $Id$ */
/*
 Copyright (C) 2010 - 2011 by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
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

#ifndef WB_HIGHLIGHT_VISITOR_HPP_
#define WB_HIGHLIGHT_VISITOR_HPP_

#include "visitor.hpp"

#include "map_location.hpp"

static lg::log_domain log_whiteboard_highlight("whiteboard/highlight");
#define ERR_WB_H LOG_STREAM(err, log_whiteboard_highlight)
#define WRN_WB_H LOG_STREAM(warn, log_whiteboard_highlight)
#define LOG_WB_H LOG_STREAM(info, log_whiteboard_highlight)
#define DBG_WB_H LOG_STREAM(debug, log_whiteboard_highlight)

namespace wb
{

/**
 * Visitor that handles highlighting planned actions as you hover over them,
 * and determine the right target for contextual execution.
 */
class highlight_visitor
	: private visitor
	, private enable_visit_all<highlight_visitor>
{
	friend class enable_visit_all<highlight_visitor>;

public:
	highlight_visitor(unit_map& unit_map, side_actions_ptr side_actions);
	virtual ~highlight_visitor();

	void set_mouseover_hex(const map_location& hex);
	const map_location& get_mouseover_hex() const {return mouseover_hex_; }

	void highlight();
	void clear();

	action_ptr get_execute_target();
	action_ptr get_delete_target();
	action_ptr get_bump_target();
	unit* get_selection_target();

	/// @return the action the currently receives the highlight focus
	weak_action_ptr get_main_highlight() { return main_highlight_; }
	typedef std::deque<weak_action_ptr> secondary_highlights_t;
	/// @return the collection of actions that are highlighted but don't have the focus
	secondary_highlights_t get_secondary_highlights() { return secondary_highlights_; }

private:
	virtual void visit(move_ptr move);
	virtual void visit(attack_ptr attack);
	virtual void visit(recruit_ptr recruit);
	virtual void visit(recall_ptr recall);
	virtual void visit(suppose_dead_ptr sup_d);

	//"Inherited" from enable_visit_all
	bool process(size_t team_index, team&, side_actions&, side_actions::iterator);

	void unhighlight();

	void find_main_highlight();
	void find_secondary_highlights();

	enum mode {
		FIND_MAIN_HIGHLIGHT,
		FIND_SECONDARY_HIGHLIGHTS,
		HIGHLIGHT_MAIN,
		HIGHLIGHT_SECONDARY,
		UNHIGHLIGHT_MAIN,
		UNHIGHLIGHT_SECONDARY,
		NONE
	};

	mode mode_;

	unit_map& unit_map_;

	map_location mouseover_hex_;
	std::set<map_location> exclusive_display_hexes_;
	unit* owner_unit_;
	unit* selection_candidate_;

	weak_action_ptr selected_action_;
	weak_action_ptr main_highlight_;
	secondary_highlights_t secondary_highlights_;

	side_actions_ptr side_actions_;
};

} // end namespace wb

#endif /* WB_HIGHLIGHT_VISITOR_HPP_ */
