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

#ifndef WB_RECALL_HPP_
#define WB_RECALL_HPP_

#include "action.hpp"

#include "map_location.hpp"

namespace wb
{

class recall: public wb::action, public boost::enable_shared_from_this<recall>
{
public:
	recall(size_t team_index, bool hidden, const unit& unit, const map_location& recall_hex);
	recall(config const&, bool hidden); // For deserialization
	virtual ~recall();

	friend class validate_visitor;
	friend class highlight_visitor;

	virtual std::ostream& print(std::ostream& s) const;

	virtual void accept(visitor& v);

	virtual void execute(bool& success, bool& complete);

	/** Applies temporarily the result of this action to the specified unit map. */
	virtual void apply_temp_modifier(unit_map& unit_map);
	/** Removes the result of this action from the specified unit map. */
	virtual void remove_temp_modifier(unit_map& unit_map);

	/** Gets called by display when drawing a hex, to allow actions to draw to the screen. */
	virtual void draw_hex(const map_location& hex);

	/**
	 * Indicates whether this hex is the preferred hex to draw the numbering for this action.
	 */
	virtual map_location get_numbering_hex() const { return recall_hex_; }

	/** @return pointer to a copy of the recall unit. */
	virtual unit* get_unit() const { return temp_unit_; }

	map_location const get_recall_hex() const { return recall_hex_; }

	/**
	 * Indicates to an action whether its status is invalid, and whether it should change its
	 * display (and avoid any change to the game state) accordingly
	 */
	virtual void set_valid(bool valid) { valid_ = valid; }
	virtual bool is_valid() { return valid_; }

	virtual config to_config() const;

private:
	void init();

	virtual void do_hide();
	virtual void do_show();

	unit* temp_unit_;
	map_location recall_hex_;
	bool valid_;
	fake_unit_ptr fake_unit_;
};

std::ostream& operator<<(std::ostream& s, recall_ptr recall);
std::ostream& operator<<(std::ostream& s, recall_const_ptr recall);

}

#endif /* WB_RECALL_HPP_ */
