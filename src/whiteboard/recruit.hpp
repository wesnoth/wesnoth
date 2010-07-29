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
 * @file recruit.hpp
 */

#ifndef WB_RECRUIT_HPP_
#define WB_RECRUIT_HPP_


#include "action.hpp"

#include "typedefs.hpp"

#include "map_location.hpp"

#include <boost/enable_shared_from_this.hpp>

namespace wb
{

/*
 *
 */
class recruit: public wb::action, public boost::enable_shared_from_this<recruit>
{
public:
	recruit(const std::string& unit_name, const map_location& recruit_hex);
	virtual ~recruit();

	friend class validate_visitor;
	friend class highlight_visitor;

	virtual std::ostream& print(std::ostream& s) const { return s; }

	virtual void accept(visitor& v);

	/** Returns true if the action has been completely executed and can be deleted */
	virtual bool execute();

	/** Applies temporarily the result of this action to the specified unit map. */
	virtual void apply_temp_modifier(unit_map& unit_map);
	/** Removes the result of this action from the specified unit map. */
	virtual void remove_temp_modifier(unit_map& unit_map);

	/** Gets called by display when drawing a hex, to allow actions to draw to the screen. */
	virtual void draw_hex(const map_location&){ }

	/**
	 * Indicates whether this hex is the preferred hex to draw the numbering for this action.
	 */
	virtual bool is_numbering_hex(const map_location& hex) const { return hex == recruit_hex_; }

	/** For recruit actions, always returns NULL. */
	virtual unit* get_unit() const { return NULL; }

	/**
	 * Indicates to an action whether its status is invalid, and whether it should change its
	 * display (and avoid any change to the game state) accordingly
	 */
	virtual void set_valid(bool valid) { valid_ = valid; }
	virtual bool is_valid() { return valid_; }


protected:
	virtual unit* create_corresponding_unit();

	std::string unit_name_;
	map_location recruit_hex_;
	bool valid_;
	fake_unit_ptr fake_unit_;

	int temp_cost_;
};

std::ostream& operator<<(std::ostream& s, recruit_ptr recruit);
std::ostream& operator<<(std::ostream& s, recruit_const_ptr recruit);

}

#endif /* WB_RECRUIT_HPP_ */
