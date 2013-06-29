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

#ifndef CAMPAIGN_SERVER_REQUEST_LICENSE_ACTION_HPP
#define CAMPAIGN_SERVER_REQUEST_LICENSE_ACTION_HPP

#include "campaign_server/actions/basic_wml_action.hpp"

class request_license_action : public basic_wml_action
{
public:
   virtual wml_reply execute(wml_request& request)
   {
      return wml_reply(std::move(request.get_data()));
   }
};

#endif // CAMPAIGN_SERVER_REQUEST_LICENSE_ACTION_HPP
