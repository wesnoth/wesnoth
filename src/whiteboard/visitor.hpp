/*
 Copyright (C) 2010 - 2017 by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
 Part of the Battle for Wesnoth Project http://www.wesnoth.org

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY.

 See the COPYING file for more details.
 */

/**
 * @file
 * visitor is an abstract interface :
 *       action.accept(visitor)   calls    visitor.visit(action)
 */

#ifndef WB_VISITOR_HPP_
#define WB_VISITOR_HPP_

#include "typedefs.hpp"

namespace wb
{

/**
 * Abstract base class for all the visitors (cf GoF Visitor Design Pattern) the whiteboard uses.
 */
class visitor
{
public:
	virtual void visit(move_ptr move) = 0;
	virtual void visit(attack_ptr attack) = 0;
	virtual void visit(recruit_ptr recruit) = 0;
	virtual void visit(recall_ptr recall) = 0;
	virtual void visit(suppose_dead_ptr sup_d) = 0;

protected:
	virtual ~visitor() {}
};

}

#endif /* WB_VISITOR_HPP_ */
