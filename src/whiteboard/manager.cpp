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
 * @file manager.cpp
 */

#include "manager.hpp"

namespace wb {

manager* manager::instance_ = NULL;

manager::manager(): active_(false)
{
}

manager& manager::instance()
{
	if (instance_ == NULL)
	{
		instance_ = new manager;
	}
	return *instance_;
}

side_actions& manager::get_side_actions(size_t side)
{
	return actions_[side];
}

} // end namespace wb
