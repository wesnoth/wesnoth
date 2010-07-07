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
 * @file move.hpp
 */

#ifndef WB_MOVE_HPP_
#define WB_MOVE_HPP_

#include "action.hpp"
#include "map_location.hpp"
#include "typedefs.hpp"

#include <boost/enable_shared_from_this.hpp>

namespace wb {

/**
 * A planned move, represented on the map by an arrow and
 * a ghosted unit in the destination hex.
 */
class move : public action, public boost::enable_shared_from_this<move>
{

	friend class validate_visitor;
	friend class highlight_visitor;

public: //constants
	static const double ALPHA_HIGHLIGHT;
	static const double ALPHA_NORMAL;
	static const std::string ARROW_STYLE_VALID;
	static const std::string ARROW_STYLE_INVALID;

public:
	move(unit& subject, const map_location& source_hex, const map_location& target_hex, arrow_ptr arrow,
			fake_unit_ptr fake_unit);
	virtual ~move();

	virtual void accept(visitor& v);

	virtual bool execute();

	virtual unit& get_unit() { return unit_; }

	arrow_ptr get_arrow() { return arrow_; }
	fake_unit_ptr get_fake_unit() { return fake_unit_; }

	/** Applies temporarily the result of this action to the specified unit map. */
	virtual void apply_temp_modifier(unit_map& unit_map);
	/** Removes the result of this action from the specified unit map. */
	virtual void remove_temp_modifier(unit_map& unit_map);

	virtual bool is_related_to(const map_location& hex) const;
	virtual bool is_related_to(const unit& unit) const;

	virtual void set_valid(bool valid);
	virtual bool is_valid() { return valid_; }

private:
	unit & unit_;
	map_location source_hex_;
	map_location dest_hex_;
	int movement_cost_;

	arrow_ptr arrow_;
	fake_unit_ptr fake_unit_;

	bool valid_;
};

} // end namespace wb

#endif /* WB_MOVE_HPP_ */
