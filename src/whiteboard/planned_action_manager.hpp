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
 * @file planned_action_manager.hpp
 */

#ifndef PLANNED_ACTION_SET_HPP_
#define PLANNED_ACTION_SET_HPP_

#include "planned_action.hpp"

#include <deque>

#include <boost/noncopyable.hpp>

typedef std::deque<planned_action> planned_action_set;

class planned_action_manager : private boost::noncopyable // Singleton -> Non-copyable
{
public:

	virtual ~planned_action_manager();

	static planned_action_manager* get_singleton();


private:
	/// Singleton -> private constructor
	planned_action_manager();

	static planned_action_manager* instance_;

	planned_action_set planned_actions;
};

#endif /* PLANNED_ACTION_SET_HPP_ */
