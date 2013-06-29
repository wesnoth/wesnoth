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

#ifndef CAMPAIGN_SERVER_WML_PROTOCOL_HPP
#define CAMPAIGN_SERVER_WML_PROTOCOL_HPP

#include <string>
#include <vector>
#include <istream>
#include <boost/asio.hpp>

#include "campaign_server/server/generic_factory.hpp"
#include "campaign_server/actions/request_license_action.hpp"
#include "campaign_server/wml_reply.hpp"
#include "campaign_server/wml_request.hpp"

class wml_protocol
{
private:
   generic_factory<basic_wml_action> action_factory;

public:
   wml_protocol()
   {
      action_factory.register_product("request_license", std::unique_ptr<basic_wml_action>(new request_license_action()));
   }

   void handle_request(std::iostream& raw_request_stream) const
   {
      wml_request request(raw_request_stream);
      std::unique_ptr<basic_wml_action> action = action_factory.make_product(request.name());
      wml_reply reply = action->execute(request);
      reply.send(raw_request_stream);
   }
};

#endif // CAMPAIGN_SERVER_WML_PROTOCOL_HPP
