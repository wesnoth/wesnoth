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

#include <boost/lexical_cast.hpp>

#include "umcd/protocol/wml/umcd_protocol.hpp"
#include "umcd/actions/request_license_action.hpp"
#include "umcd/actions/request_umc_upload_action.hpp"
#include "umcd/special_packet.hpp"


const std::size_t umcd_protocol::REQUEST_HEADER_MAX_SIZE;
const std::size_t umcd_protocol::MAX_NUMBER_OF_DIGITS;

umcd_protocol::umcd_protocol(const config& server_config)
: server_config(server_config)
, action_factory(boost::make_shared<action_factory_type>())
{
   register_request_info<request_license_action>("request_license");
   register_request_info<request_umc_upload_action>("request_umc_upload");
}

umcd_protocol::umcd_protocol(const umcd_protocol& protocol)
: boost::enable_shared_from_this<umcd_protocol>()
, server_config(protocol.server_config)
, action_factory(protocol.action_factory)
{}

wml_reply& umcd_protocol::get_reply()
{
   return reply;
}

wml_request& umcd_protocol::get_request()
{
   return request;
}

void umcd_protocol::complete_request(const boost::system::error_code& error, std::size_t)
{
   UMCD_LOG_IP(trace, client_connection->get_socket()) << " -- umcd_protocol::complete_request.";
   if(error)
   {
      UMCD_LOG_IP(info, client_connection->get_socket()) << " -- unable to send data to the client (" << error.message() << "). Connection dropped.";
      // TODO close socket.
   }
}

void umcd_protocol::async_send_reply()
{
   UMCD_LOG_IP(trace, client_connection->get_socket()) << " -- umcd_protocol::async_send_reply.";
   boost::asio::async_write(client_connection->get_socket()
      , reply.to_buffers()
      , boost::bind(&umcd_protocol::complete_request, shared_from_this()
         , boost::asio::placeholders::error
         , boost::asio::placeholders::bytes_transferred)
   );
}

void umcd_protocol::async_send_error(const std::string& error_msg)
{
   reply = make_error_reply(error_msg);
   async_send_reply();
}

void umcd_protocol::async_send_invalid_packet(const std::string &where, const std::exception& e)
{
   async_send_error("The packet you sent is invalid. It could be a protocol bug and administrators have been contacted, the problem should be fixed soon.");
   UMCD_LOG_IP(error, client_connection->get_socket()) << " -- invalid request at " << where << " (" << e.what() << ")";
}

void umcd_protocol::read_request_body(const boost::system::error_code& error, std::size_t)
{
   UMCD_LOG_IP(trace, client_connection->get_socket()) << " -- umcd_protocol::read_request_body.";
   if(!error)
   {
      try
      {
         // NOTE: We encapsulate the boost::array into a string because it old lexical_cast does not support boost::array. Change this when it'll be supported.
         std::size_t request_size = boost::lexical_cast<std::size_t>(std::string(raw_request_size.data()));
         UMCD_LOG_IP(debug, client_connection->get_socket()) << " -- Request of size: " << request_size;
         if(request_size > REQUEST_HEADER_MAX_SIZE)
         {
            async_send_error("The request size is too big.");
         }
         else
         {
            request_body.resize(request_size);
            boost::asio::async_read(client_connection->get_socket(), boost::asio::buffer(&request_body[0], request_body.size())
            , boost::bind(&umcd_protocol::dispatch_request, shared_from_this()
               , boost::asio::placeholders::error
               , boost::asio::placeholders::bytes_transferred)
            );
         }
      }
      catch(const std::exception& e)
      {
         async_send_invalid_packet("umcd_protocol::read_request_body", e);
      }
   }
}

void umcd_protocol::dispatch_request(const boost::system::error_code& err, std::size_t)
{
   UMCD_LOG_IP(trace, client_connection->get_socket()) << " -- umcd_protocol::dispatch_request.";
   if(!err)
   {
      std::stringstream request_stream(request_body);      
      try
      {
         // Retrieve request name.
         std::string request_name = peek_request_name(request_stream);
         UMCD_LOG_IP(info, client_connection->get_socket()) << " -- request: " << request_name;
         info_ptr request_info = action_factory->make_product(request_name);

         request = wml_request(client_connection);
         // Read into config and validate metadata.
         ::read(request.get_metadata(), request_stream, request_info->validator().get());
         UMCD_LOG_IP(debug, client_connection->get_socket()) << " -- request validated.\n";

         request_info->action()->execute(shared_from_this());
      }
      catch(std::exception& e)
      {
         async_send_invalid_packet("umcd_protocol::dispatch_request", e);
      }
   }
}

void umcd_protocol::handle_request(connection_ptr client)
{
   client_connection = client;
   UMCD_LOG_IP(trace, client_connection->get_socket()) << " -- umcd_protocol::handle_request.";
   boost::asio::async_read(client_connection->get_socket(), boost::asio::buffer(raw_request_size)
      , boost::bind(&umcd_protocol::read_request_body, shared_from_this()
         , boost::asio::placeholders::error
         , boost::asio::placeholders::bytes_transferred)
   );
}