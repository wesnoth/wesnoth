/*
 Copyright (C) 2010 - 2017 by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
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

#include "action.hpp"

struct temporary_unit_mover;

namespace wb {

/**
 * A planned move, represented on the map by an arrow and
 * a ghosted unit in the destination hex.
 */
class move : public action
{
public:
	move(size_t team_index, bool hidden, unit& mover, const pathfind::marked_route& route,
			arrow_ptr arrow, fake_unit_ptr fake_unit);
	move(config const&, bool hidden); // For deserialization
	virtual ~move();

	virtual std::ostream& print(std::ostream& s) const;

	virtual void accept(visitor& v);

	virtual void execute(bool& success, bool& complete);

	/**
	 * Check the validity of the action.
	 *
	 * @return the error preventing the action from being executed.
	 * @retval OK if there isn't any error (the action can be executed.)
	 */
	virtual error check_validity() const;

	/** Return the unit targeted by this action. Null if unit doesn't exist. */
	virtual unit_ptr get_unit() const;
	/** @return pointer to the fake unit used only for visuals */
	virtual fake_unit_ptr get_fake_unit() { return fake_unit_; }

	virtual map_location get_source_hex() const;
	virtual map_location get_dest_hex() const;

	virtual void set_route(const pathfind::marked_route& route);
	virtual const pathfind::marked_route& get_route() const { assert(route_); return *route_; }
	/// attempts to pathfind a new marked route for this path between these two hexes;
	/// returns true and assigns it to the internal route if successful.
	virtual bool calculate_new_route(const map_location& source_hex, const map_location& dest_hex);

	virtual arrow_ptr get_arrow() { return arrow_; }

	/** Applies temporarily the result of this action to the specified unit map. */
	virtual void apply_temp_modifier(unit_map& unit_map);
	/** Removes the result of this action from the specified unit map. */
	virtual void remove_temp_modifier(unit_map& unit_map);

	/** Gets called by display when drawing a hex, to allow actions to draw to the screen. */
	virtual void draw_hex(map_location const& hex);
	/** Redrawing function, called each time the action situation might have changed. */
	void redraw();

	/** Assigns a turn number to display to this planned move. Assigning zero removes any turn number. */
	virtual void set_turn_number(int turn) { turn_number_ = turn; }

	virtual map_location get_numbering_hex() const;

	virtual config to_config() const;

	///@todo Make use of safe_enum idiom?
	enum ARROW_BRIGHTNESS {ARROW_BRIGHTNESS_STANDARD, ARROW_BRIGHTNESS_HIGHLIGHTED, ARROW_BRIGHTNESS_FOCUS};
	void set_arrow_brightness(ARROW_BRIGHTNESS x) const {arrow_brightness_=x; }
	enum ARROW_TEXTURE {ARROW_TEXTURE_VALID, ARROW_TEXTURE_INVALID};
	void set_arrow_texture(ARROW_TEXTURE x) const {arrow_texture_=x; }

protected:

	std::shared_ptr<move> shared_from_this() {
		return std::static_pointer_cast<move>(action::shared_from_this());
	}

	void calculate_move_cost();

	size_t unit_underlying_id_;
	std::string unit_id_;
	std::unique_ptr<pathfind::marked_route> route_;
	int movement_cost_;
	/// Turn end number to draw if greater than zero. Assigned by the map builder.
	int turn_number_;

	arrow_ptr arrow_;
	fake_unit_ptr fake_unit_;

	mutable ARROW_BRIGHTNESS arrow_brightness_;
	mutable ARROW_TEXTURE arrow_texture_;

private:
	virtual void do_hide();
	virtual void do_show();

	void hide_fake_unit();
	void show_fake_unit();

	void init();
	void update_arrow_style();
	std::unique_ptr<temporary_unit_mover> mover_;
	bool fake_unit_hidden_;
};

/** Dumps an move on a stream, for debug purposes. */
std::ostream &operator<<(std::ostream &s, move_ptr move);
std::ostream &operator<<(std::ostream &s, move_const_ptr move);

} // end namespace wb
