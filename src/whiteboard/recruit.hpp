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

#include <string>

#include "action.hpp"
#include "map/location.hpp"

namespace wb
{

/*
 *
 */
class recruit: public action
{
public:
	recruit(size_t team_index, bool hidden, const std::string& unit_name, const map_location& recruit_hex);
	recruit(const config&, bool hidden); // For deserialization
	virtual ~recruit();

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

	/** Applies temporarily the result of this action to the specified unit map. */
	virtual void apply_temp_modifier(unit_map& unit_map);
	/** Removes the result of this action from the specified unit map. */
	virtual void remove_temp_modifier(unit_map& unit_map);

	/** Gets called by display when drawing a hex, to allow actions to draw to the screen. */
	virtual void draw_hex(const map_location& hex);
	/** Redrawing function, called each time the action situation might have changed. */
	virtual void redraw();

	/**
	 * @return the preferred hex to draw the numbering for this action.
	 */
	virtual map_location get_numbering_hex() const { return recruit_hex_; }

	/** @return pointer to a fake unit representing the one that will eventually be recruited. */
	virtual unit_ptr get_unit() const { return temp_unit_; }
	/** @return pointer to the fake unit used only for visuals */
	virtual fake_unit_ptr get_fake_unit() { return fake_unit_; }

	map_location const get_recruit_hex() const { return recruit_hex_; }

	virtual config to_config() const;

protected:

	std::shared_ptr<recruit> shared_from_this() {
		return std::static_pointer_cast<recruit>(action::shared_from_this());
	}

	std::string unit_name_;
	map_location recruit_hex_;
	//Temp unit to insert in the future unit map when needed
	unit_ptr temp_unit_;
	fake_unit_ptr fake_unit_;
	int cost_;

private:
	void init();

	virtual void do_hide();
	virtual void do_show();

	unit_ptr create_corresponding_unit();
};

std::ostream& operator<<(std::ostream& s, recruit_ptr recruit);
std::ostream& operator<<(std::ostream& s, recruit_const_ptr recruit);

}
