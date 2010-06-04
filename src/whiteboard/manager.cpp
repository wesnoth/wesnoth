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

manager::manager()
{

}

manager::~manager()
{

}

manager* manager::get_singleton()
{
	if (instance_ == NULL)
	{
		instance_ = new manager;
	}
	return instance_;
}

const planned_action_set& manager::get_planned_actions() const
{
	return planned_actions_;
}

void manager::add_planned_move(unit& subject, const map_location& target_hex)
{
	planned_action_ptr ptr(new move(subject, target_hex));
	planned_actions_.push_back(ptr);
}

} // end namespace wb
