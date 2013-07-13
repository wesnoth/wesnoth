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

#ifndef UMCD_PROTOCOL_HPP
#define UMCD_PROTOCOL_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "umcd/umcd_logger.hpp"
#include "umcd/server/generic_factory.hpp"
#include "umcd/actions/request_license_action.hpp"
#include "umcd/actions/request_umc_upload_action.hpp"
#include "umcd/wml_reply.hpp"
#include "umcd/wml_request.hpp"
#include "umcd/request_info.hpp"
#include "umcd/special_packet.hpp"


class umcd_protocol
{
private:
   typedef basic_umcd_action action_type;
   typedef boost::shared_ptr<request_info> info_ptr;
   typedef schema_validation::one_hierarchy_validator validator_type;

   const config& server_config;
   generic_factory<request_info> action_factory;

   template <class Action>
   void register_request_info(const std::string& request_name)
   {
      action_factory.register_product(
         request_name, 
         make_request_info<Action, validator_type>(server_config, request_name)
      );
   }

public:
   typedef boost::asio::ip::tcp::iostream network_stream;

   umcd_protocol(const config& server_config)
   : server_config(server_config)
   {
      register_request_info<request_license_action>("request_license");
      register_request_info<request_umc_upload_action>("request_umc_upload_action");
   }

   void handle_request(network_stream& raw_request_stream) const
   {
      wml_reply reply;
      try
      {
         std::string request_name = peek_request_name(raw_request_stream);
         UMCD_LOG_IP(info, raw_request_stream) << " -- request: " << request_name;
         info_ptr info = action_factory.make_product(request_name);
         wml_request request(raw_request_stream, info->validator());
         reply = info->action()->execute(request, server_config);
      }
      catch(std::exception&)
      {
         reply = make_error_reply("The packet you sent is invalid. It could be a protocol bug and administrators have been contacted, the problem should be fixed soon.");
         UMCD_LOG_IP(error, raw_request_stream) << " -- invalid request";
      }
      try
      {
         if(raw_request_stream.good())
         {
            reply.send(raw_request_stream);
         }
      }
      catch(std::exception& e)
      {
         UMCD_LOG_IP(info, raw_request_stream) << " -- unable to send data to the client (" << e.what() << "). Connection dropped.";
      }
   }
};

#endif // UMCD_PROTOCOL_HPP
