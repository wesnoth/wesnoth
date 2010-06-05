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
 * @file mapbuilder_visitor.h
 */

#ifndef WB_MAPBUILDER_VISITOR_HPP_
#define WB_MAPBUILDER_VISITOR_HPP_

#include "visitor.hpp"

namespace wb
{

class mapbuilder_visitor: public visitor
{
public:
	mapbuilder_visitor();
	virtual ~mapbuilder_visitor();

	virtual void visit_move(move& p_move);
};

}

#endif /* WB_MAPBUILDER_VISITOR_HPP_ */
