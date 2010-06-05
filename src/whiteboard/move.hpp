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
#include "arrow.hpp"

class unit;
class config;

namespace wb {

/**
 * A planned move, represented on the map by an arrow and
 * a ghosted unit in the destination hex.
 */
class move: public action
{
public:
	move(unit& subject, const map_location& target_hex);
	virtual ~move();

	virtual void accept(visitor& v);

private:
	unit & unit_;
	map_location target_hex_;

	arrow* arrow_;

};

} // end namespace wb

#endif /* WB_MOVE_HPP_ */
