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

#include "typedefs.hpp"
#include "map/location.hpp"
#include "game_errors.hpp"

namespace wb {

class visitor;

/**
 * Abstract base class for all the whiteboard planned actions.
 */
class action : public std::enable_shared_from_this<action>
{
public:
	action(size_t team_index, bool hidden);
	action(const config&, bool hidden); // For deserialization
	virtual ~action();

	virtual std::ostream& print(std::ostream& s) const = 0;

	virtual void accept(visitor& v) = 0;

	/**
	 * Output parameters:
	 *   success:  Whether or not to continue an execute-all after this execution
	 *   complete: Whether or not to delete this action after execution
	 */
	virtual void execute(bool& success, bool& complete) = 0;

	/** Applies temporarily the result of this action to the specified unit map. */
	virtual void apply_temp_modifier(unit_map& unit_map) = 0;
	/** Removes the result of this action from the specified unit map. */
	virtual void remove_temp_modifier(unit_map& unit_map) = 0;

	/** Gets called by display when drawing a hex, to allow actions to draw to the screen. */
	virtual void draw_hex(const map_location& hex) = 0;
	/** Redrawing function, called each time the action situation might have changed. */
	virtual void redraw(){}

	/** Sets whether or not the action should be drawn on the screen. */
	void hide();
	void show();
	bool hidden() const {return hidden_;}

	/** Indicates whether this hex is the preferred hex to draw the numbering for this action. */
	bool is_numbering_hex(const map_location& hex) const {return hex==get_numbering_hex();}
	virtual map_location get_numbering_hex() const = 0;

	/** Return the unit targeted by this action. Null if unit doesn't exist. */
	virtual unit_ptr get_unit() const = 0;

	/**
	 * Returns the id of the unit targeted by this action.
	 * @retval 0 no unit is targeted.
	 */
	size_t get_unit_id() const;

	/** @return pointer to the fake unit used only for visuals */
	virtual fake_unit_ptr get_fake_unit() = 0;
	/** Returns the index of the team that owns this action */
	size_t team_index() const { return team_index_; }
	/** Returns the number of the side that owns this action, i.e. the team index + 1. */
	int side_number() const
	{
		return static_cast<int>(team_index_) + 1;
	}

	/** Constructs and returns a config object representing this object. */
	virtual config to_config() const;
	/** Constructs an object of a subclass of wb::action using a config. Current behavior is to return a null pointer for unrecognized config. */
	static action_ptr from_config(const config&, bool hidden);

	struct ctor_err	: public game::error
	{
		ctor_err(const std::string& message) : game::error(message){}
	};

	/**
	 * Possible errors.
	 *
	 * Returned by the @ref check function.
	 */
	enum error
	{
		OK,
		INVALID_LOCATION,
		NO_UNIT,
		UNIT_CHANGED,
		LOCATION_OCCUPIED,
		TOO_FAR,
		NO_TARGET,
		NO_ATTACK_LEFT,
		NOT_AN_ENEMY,
		UNIT_UNAVAILABLE,
		NOT_ENOUGH_GOLD,
		NO_LEADER
	};

	/**
	 * Check the validity of the action.
	 *
	 * @return the error preventing the action from being executed.
	 * @retval OK if there isn't any error (the action can be executed.)
	 */
	virtual error check_validity() const = 0;

	/**
	 * Returns whether this action is valid or not.
	 *
	 * @note This value is now calculated each time the function is called.
	 */
	bool valid(){ return check_validity()==OK; }

private:
	/** Called by the non-virtual hide() and show(), respectively. */
	virtual void do_hide() {}
	virtual void do_show() {}

	size_t team_index_;
	bool hidden_;
};

std::ostream& operator<<(std::ostream& s, action_ptr action);
std::ostream& operator<<(std::ostream& s, action_const_ptr action);

} // end namespace wb
