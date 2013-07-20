/*
   Copyright (C) 2013 by Pierre Talbot <ptalbot@mopong.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef UMCD_BASIC_UMCD_ACTION_HPP
#define UMCD_BASIC_UMCD_ACTION_HPP

#include "umcd/server/generic_action.hpp"

class umcd_protocol;

typedef generic_action<void, boost::shared_ptr<umcd_protocol> > basic_umcd_action;

#endif // UMCD_BASIC_WML_ACTION_HPP
