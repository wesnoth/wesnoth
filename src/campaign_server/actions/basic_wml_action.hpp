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

#ifndef CAMPAIGN_SERVER_BASIC_WML_ACTION_HPP
#define CAMPAIGN_SERVER_BASIC_WML_ACTION_HPP

#include "campaign_server/server/generic_action.hpp"
#include "campaign_server/wml_reply.hpp"
#include "campaign_server/wml_request.hpp"

typedef generic_action<wml_reply, wml_request&> basic_wml_action;

#endif // CAMPAIGN_SERVER_BASIC_WML_ACTION_HPP
