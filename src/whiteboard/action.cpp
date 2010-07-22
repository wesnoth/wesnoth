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
 * @file action.cpp
 */

#include "action.hpp"

namespace wb {

std::ostream& operator<<(std::ostream& s, action_const_ptr action)
{
	return action->print(s);
}

std::ostream& action::print(std::ostream& s) const
{
	return s;
}

action::~action()
{
}

action::action()
{

}

} // end namespace wb
