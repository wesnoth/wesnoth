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

#ifndef UMCD_WML_PROTOCOL_HPP
#define UMCD_WML_PROTOCOL_HPP

#include <string>
#include <vector>
#include <istream>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

#include "umcd/server/generic_factory.hpp"
#include "umcd/actions/request_license_action.hpp"
#include "umcd/wml_reply.hpp"
#include "umcd/wml_request.hpp"

class wml_protocol
{
private:
   const config& server_config;
   generic_factory<basic_wml_action> action_factory;

public:
   wml_protocol(const config& server_config)
   : server_config(server_config)
   {
      action_factory.register_product("request_license", boost::make_shared<request_license_action>());
   }

   void handle_request(std::iostream& raw_request_stream) const
   {
      wml_request request(raw_request_stream, server_config);
      boost::shared_ptr<basic_wml_action> action = action_factory.make_product(request.name());
      wml_reply reply = action->execute(request);
      reply.send(raw_request_stream);
   }
};

#endif // UMCD_WML_PROTOCOL_HPP
