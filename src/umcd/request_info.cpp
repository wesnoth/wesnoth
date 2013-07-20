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

#include "umcd/request_info.hpp"

request_info::request_info(const action_ptr& action, const validator_ptr& validator)
: umcd_action(action)
, request_validator(validator)
{}

typename request_info::action_ptr request_info::action()
{
   return umcd_action;
}

typename request_info::validator_ptr request_info::validator()
{
   return request_validator;
}

boost::shared_ptr<request_info> request_info::clone() const
{
   return boost::make_shared<request_info>(
      umcd_action->clone(), 
      boost::make_shared<validator_type>(*request_validator)
   );
}
