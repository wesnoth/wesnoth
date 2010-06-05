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

#include "visitor.hpp"

namespace wb {

/**
 * Superclass for all the whiteboard planned actions.
 */
class action
{
public:
	action();
	virtual ~action();

	virtual void accept(visitor& v) = 0;
};

} // end namespace wb

#endif /* WB_ACTION_HPP_ */
