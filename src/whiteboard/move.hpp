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

namespace wb {

/**
 * A planned move, represented on the map by an arrow and
 * a ghosted unit in the destination hex.
 */
class move : public action, public boost::enable_shared_from_this<move>
{
public:
	friend class validate_visitor;
	friend class highlight_visitor;

	static const double ALPHA_HIGHLIGHT;
	static const double ALPHA_NORMAL;
	static const std::string ARROW_STYLE_VALID;
	static const std::string ARROW_STYLE_INVALID;

	///Future unit map must be valid during construction, so that move can find its unit
	move(const pathfind::marked_route& route, arrow_ptr arrow,
			fake_unit_ptr fake_unit);
	virtual ~move();

	virtual std::ostream& print(std::ostream& s) const;

	virtual void accept(visitor& v);

	virtual bool execute();

	/** Return the unit targeted by this action. Null if unit doesn't exist. */
	virtual unit* get_unit() const { return unit_; }

	virtual map_location get_source_hex() const;
	virtual map_location get_dest_hex() const;

	virtual void set_route(const pathfind::marked_route& route);
	virtual const pathfind::marked_route& get_route() const { assert(route_); return *route_; }
	/// attempts to pathfind a new marked route for this path between these two hexes;
	/// returns true and assigns it to the internal route if successful.
	virtual bool calculate_new_route(const map_location& source_hex, const map_location& dest_hex);

	virtual arrow_ptr get_arrow() { return arrow_; }
	virtual fake_unit_ptr get_fake_unit() { return fake_unit_; }

	/** Applies temporarily the result of this action to the specified unit map. */
	virtual void apply_temp_modifier(unit_map& unit_map);
	/** Removes the result of this action from the specified unit map. */
	virtual void remove_temp_modifier(unit_map& unit_map);

	/** Gets called by display when drawing a hex, to allow actions to draw to the screen. */
	virtual void draw_hex(const map_location& hex);

	virtual bool is_numbering_hex(const map_location& hex) const;

	virtual void set_valid(bool valid);
	virtual bool is_valid() { return valid_; }

protected:

	void calculate_move_cost();

	unit* unit_;
	std::string unit_id_;
	boost::scoped_ptr<pathfind::marked_route> route_;
	int movement_cost_;

	arrow_ptr arrow_;
	fake_unit_ptr fake_unit_;

	bool valid_;
};

/** Dumps an move on a stream, for debug purposes. */
std::ostream &operator<<(std::ostream &s, move_ptr move);
std::ostream &operator<<(std::ostream &s, move_const_ptr move);

} // end namespace wb

#endif /* WB_MOVE_HPP_ */
