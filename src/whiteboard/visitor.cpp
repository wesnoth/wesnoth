/* $Id$ */
/*
 Copyright (C) 2010 by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
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
 */

#include "validate_visitor.hpp"

#include "action.hpp"
#include "foreach.hpp"
#include "side_actions.hpp"

namespace wb
{

visitor::visitor(side_actions_ptr side_actions):
		side_actions_(side_actions)
{
}

visitor::~visitor()
{
}

void visitor::visit_all_actions()
{
	foreach(action_ptr action, *side_actions_)
	{
		action->accept(*this);
	}
}

}//end namespace wb
