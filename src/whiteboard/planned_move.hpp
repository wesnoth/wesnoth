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
 * @file planned_move.hpp
 */

#ifndef PLANNED_MOVE_H_
#define PLANNED_MOVE_H_

#include "planned_action.hpp"

class planned_move: public planned_action
{
public:
	planned_move();
	virtual ~planned_move();
};

#endif /* PLANNED_MOVE_H_ */
