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
 * @file mapbuilder_visitor.cpp
 */

#include "mapbuilder_visitor.hpp"
#include "move.hpp"

#include "unit.hpp"
#include "unit_map.hpp"

namespace wb
{

mapbuilder_visitor::mapbuilder_visitor(unit_map& unit_map)
	: unit_map_(unit_map)
    , excluded_units_()
	, modifiers_()
{
}

mapbuilder_visitor::~mapbuilder_visitor()
{
	size_t stack_size = modifiers_.size();
	for (size_t i = 0; i < stack_size; ++i )
	{
		modifiers_.pop();
	}
}

void mapbuilder_visitor::visit_move(boost::shared_ptr<move> move)
{
	if (excluded_units_.find(&move->get_unit()) == excluded_units_.end())
	{
		modifiers_.push(move->apply_temp_modifier(unit_map_));
	}
}

}
