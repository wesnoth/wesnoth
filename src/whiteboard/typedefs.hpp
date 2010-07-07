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
 * @file typedefs.hpp
 * Contains typedefs for the whiteboard.
 */


#ifndef WB_TYPEDEFS_HPP_
#define WB_TYPEDEFS_HPP_

#include "log.hpp"
static lg::log_domain log_whiteboard("whiteboard");
#define ERR_WB LOG_STREAM(err, log_whiteboard)
#define WRN_WB LOG_STREAM(warn, log_whiteboard)
#define LOG_WB LOG_STREAM(info, log_whiteboard)
#define DBG_WB LOG_STREAM(debug, log_whiteboard)

#include <boost/shared_ptr.hpp>
#include <deque>

class arrow;
struct map_location; //not used in the typedefs, saves a few forward declarations
class unit;
class unit_map; //not used in the typedefs, saves a few forward declarations

namespace wb {

class action;
class move;
class side_actions;

typedef boost::shared_ptr<arrow> arrow_ptr;
typedef boost::shared_ptr<unit> fake_unit_ptr;

typedef boost::shared_ptr<action> action_ptr;
typedef std::deque<action_ptr> action_queue;
typedef boost::shared_ptr<move> move_ptr;
typedef boost::shared_ptr<side_actions> side_actions_ptr;

} // end namespace wb

#endif /* WB_TYPEDEFS_HPP_ */
