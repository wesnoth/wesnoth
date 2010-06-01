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
 * @file planned_action_manager.cpp
 */

#include "planned_action_manager.hpp"

#include <boost/make_shared.hpp>

planned_action_manager* planned_action_manager::instance_ = NULL;

planned_action_manager::planned_action_manager()
{

}

planned_action_manager::~planned_action_manager()
{

}

planned_action_manager* planned_action_manager::get_singleton()
{
	if (instance_ == NULL)
	{
		instance_ = new planned_action_manager;
	}
	return instance_;
}

const planned_action_set& planned_action_manager::get_planned_actions() const
{
	return planned_actions_;
}

void planned_action_manager::add_planned_move(unit& subject, const map_location& target_hex)
{
	planned_actions_.push_back(boost::make_shared<planned_move>(boost::ref(subject), target_hex));
}
