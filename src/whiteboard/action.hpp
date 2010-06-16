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
 * @file action.hpp
 */

#ifndef WB_ACTION_HPP_
#define WB_ACTION_HPP_

#include "log.hpp"

#include <boost/shared_ptr.hpp>

static lg::log_domain log_whiteboard("whiteboard");
#define ERR_WB LOG_STREAM(err, log_whiteboard)
#define WRN_WB LOG_STREAM(warn, log_whiteboard)
#define LOG_WB LOG_STREAM(info, log_whiteboard)
#define DBG_WB LOG_STREAM(debug, log_whiteboard)


struct temporary_unit_map_modifier;

namespace wb {

class action;
class visitor;

typedef boost::shared_ptr<temporary_unit_map_modifier> modifier_ptr;

typedef boost::shared_ptr<action> action_ptr;


/**
 * Superclass for all the whiteboard planned actions.
 */
class action
{
public:
	action();
	virtual ~action();

	virtual void accept(visitor& v) = 0;

	virtual void execute() = 0;
};

} // end namespace wb

#endif /* WB_ACTION_HPP_ */
