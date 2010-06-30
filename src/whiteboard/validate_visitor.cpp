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
 * @file validate_visitor.cpp
 */

#include "validate_visitor.hpp"

namespace wb
{

validate_visitor::validate_visitor(unit_map& unit_map)
	: mapbuilder_visitor(unit_map)
{
}

validate_visitor::~validate_visitor()
{
}

void validate_visitor::visit_move(boost::shared_ptr<move> move)
{
	mapbuilder_visitor::visit_move(move);

}

}//end namespace wb
