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
 * @file planned_move.cpp
 */

#include "planned_move.hpp"
#include "unit.hpp"

planned_move::planned_move(unit& subject, const map_location& target_hex)
: unit_(subject), target_hex_(target_hex)
{

}

planned_move::~planned_move()
{

}
