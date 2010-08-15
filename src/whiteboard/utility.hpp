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
 * @file
 */

#ifndef WB_UTILITY_HPP_
#define WB_UTILITY_HPP_

#include "typedefs.hpp"

class unit;

namespace wb {

/**
 * For a given leader on a keep, find another leader on another keep in the same castle.
 */
map_location find_backup_leader(unit const& leader);

} //end namespace wb

#endif /* WB_UTILITY_HPP_ */
