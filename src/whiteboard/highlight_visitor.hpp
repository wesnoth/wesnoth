/* $Id$ */
/*
 Copyright (C) 2010 by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
 Part of the Battle for Wesnoth Project http://www.wesnoth.org

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 2
 or at your option any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY.

 See the COPYING file for more details.
 */

/**
 * @file highlight_visitor.hpp
 */

#ifndef WB_HIGHLIGHT_VISITOR_HPP_
#define WB_HIGHLIGHT_VISITOR_HPP_

#include "visitor.hpp"

#include "map_location.hpp"

#include <boost/weak_ptr.hpp>

static lg::log_domain log_whiteboard_highlight("whiteboard/highlight");
#define ERR_WB_H LOG_STREAM(err, log_whiteboard_highlight)
#define WRN_WB_H LOG_STREAM(warn, log_whiteboard_highlight)
#define LOG_WB_H LOG_STREAM(info, log_whiteboard_highlight)
#define DBG_WB_H LOG_STREAM(debug, log_whiteboard_highlight)

namespace wb
{


class highlight_visitor: public wb::visitor
{
public:
	highlight_visitor(const unit_map& unit_map, side_actions_ptr side_actions);
	virtual ~highlight_visitor();

	void set_selected_unit(unit& unit);
	void set_mouseover_hex(const map_location& hex);
	const map_location& get_mouseover_hex() const {return mouseover_hex_; }

	void highlight();
	void unhighlight();

	action_ptr get_execute_target();
	action_ptr get_delete_target();
	action_ptr get_bump_target();

	virtual void visit_move(move_ptr move);

private:

	void clear();
	void visit_all_actions();

	void find_main_highlight();
	void find_secondary_highlights();

	enum mode {
		FIND_MAIN_HIGHLIGHT,
		FIND_SECONDARY_HIGHLIGHTS,
		HIGHLIGHT_MAIN,
		HIGHLIGHT_SECONDARY,
		UNHIGHLIGHT,
		NONE
	};

	mode mode_;

	const unit_map& unit_map_;
	side_actions_ptr side_actions_;

	unit* selected_unit_;
	map_location mouseover_hex_;
	unit* owner_unit_;

	weak_action_ptr main_highlight_;
	std::deque<weak_action_ptr> secondary_highlights_;

	std::string color_backup_;
};

} // end namespace wb

#endif /* WB_HIGHLIGHT_VISITOR_HPP_ */
