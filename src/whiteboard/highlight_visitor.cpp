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
 * @file highlight_visitor.cpp
 */

#include "highlight_visitor.hpp"
#include "manager.hpp"
#include "move.hpp"

#include "arrow.hpp"
#include "resources.hpp"

namespace wb
{

highlight_visitor::highlight_visitor(bool highlight)
: highlight_(highlight)
{
}

highlight_visitor::~highlight_visitor()
{
}

void highlight_visitor::visit_move(boost::shared_ptr<move> move)
{
	assert (!resources::whiteboard->has_planned_unit_map());

	if (highlight_)
	{
		move->get_arrow()->set_alpha(move::ALPHA_HIGHLIGHT);
		move->get_fake_unit()->set_ghosted(false);
		move->get_unit().set_selecting();
	}
	else
	{
		move->get_arrow()->set_alpha(move::ALPHA_NORMAL);
		move->get_fake_unit()->set_disabled_ghosted(false);
		move->get_unit().set_standing(true);
	}
}

} // end namespace wb
