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

#ifndef SERVER_WML_PROTOCOL_HPP
#define SERVER_WML_PROTOCOL_HPP

#include <string>
#include <vector>
#include <istream>
#include <boost/asio.hpp>

#include "serialization/parser.hpp"
#include "serialization/one_hierarchy_validator.hpp"

#include "campaign_server/generic_factory.hpp"
#include "campaign_server/generic_action.hpp"
#include "campaign_server/server/network_data.hpp"

class wml_reply
{
private:
   network_data data;
public:
   wml_reply() = default;
   wml_reply(network_data&& data)
   : data(std::move(data))
   {}

   void send(std::ostream& raw_data_stream) const
   {
      write(raw_data_stream, data.get_metadata());
   }
};

class wml_request
{
private:
   network_data data;
public:
   wml_request(std::istream& raw_data_stream)
   {
      using namespace schema_validation;
      std::unique_ptr<one_hierarchy_validator> validator(new one_hierarchy_validator("test.cfg"));
      read(data.get_metadata(), raw_data_stream, validator.get());
      std::cout << "[Info] Request read:\n" << data.get_metadata();
   }

   network_data& get_data() { return data; }
};


class wml_protocol
{
private:
   typedef generic_action<wml_reply, wml_request> wml_action;
   generic_factory<wml_action> action_factory;

   struct umc_download_action : wml_action
   {
      virtual wml_reply execute(wml_request)
      {
         return wml_reply();
      }
   };

public:
   typedef wml_reply reply_type;

   wml_protocol()
   {
      action_factory.register_product("umc_download_request", std::unique_ptr<wml_action>(new umc_download_action()));
   }

   void handle_request(std::iostream& raw_request_stream) const
   {
      wml_request request(raw_request_stream);
      wml_reply reply(std::move(request.get_data()));
      reply.send(raw_request_stream);
   }
};

#endif // SERVER_WML_PROTOCOL_HPP
