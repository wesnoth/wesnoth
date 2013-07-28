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
   
#include "wml_exception.hpp"

#include "umcd/actions/basic_umcd_action.hpp"
#include "umcd/wml_request.hpp"
#include "umcd/server/connection.hpp"
#include "umcd/umcd_logger.hpp"
#include "umcd/server/generic_factory.hpp"
#include "umcd/wml_reply.hpp"
#include "umcd/request_info.hpp"

class wml_request;

class umcd_protocol : public boost::enable_shared_from_this<umcd_protocol>
{
public:
   static const std::size_t REQUEST_HEADER_MAX_SIZE = 8192;
   static const std::size_t REQUEST_HEADER_SIZE_FIELD_LENGTH = 10;
private:
   typedef basic_umcd_action action_type;
   typedef boost::shared_ptr<request_info> info_ptr;
   typedef schema_validation::schema_validator validator_type;
   typedef generic_factory<request_info> action_factory_type;
   typedef connection<umcd_protocol> connection_type;
   typedef boost::shared_ptr<connection_type> connection_ptr;

   template <class Action>
   void register_request_info(const std::string& request_name);

public:

   // This constructor is only called once in main, so the factory will be created once as well.
   umcd_protocol(const config& server_config);

   // We only copy shared data.
   umcd_protocol(const umcd_protocol& protocol);

   // Precondition: (bool)client == true
   void handle_request(connection_ptr client);
   // Precondition: handle_request has been called and connection has been initialized.
   void async_send_reply();

   wml_reply& get_reply();
   config& get_metadata();

private:
   void complete_request(const boost::system::error_code& error, std::size_t bytes_transferred);

   void async_send_error(const boost::system::error_condition& error);
   void async_send_invalid_packet(const std::string &where, const std::exception& e);
   void async_send_invalid_packet(const std::string &where, const twml_exception& e);

   // Precondition: size_of_request must be read.
   void read_request_body(const boost::system::error_code& error, std::size_t bytes_transferred);

   // Precondition: request_body must be read.
   void dispatch_request(const boost::system::error_code& error, std::size_t bytes_transferred);

private:
   const config& server_config_;
   // The shared_ptr avoid the factory to be copied/duplicated.
   boost::shared_ptr<action_factory_type> action_factory_;
   connection_ptr client_connection_;
   boost::array<char, REQUEST_HEADER_SIZE_FIELD_LENGTH> raw_request_size_;
   std::string request_body_;
   wml_reply reply_;
   wml_request request_;
};

template <class Action>
void umcd_protocol::register_request_info(const std::string& request_name)
{
   action_factory_->register_product(
      request_name, 
      make_request_info<Action, validator_type>(server_config_, request_name)
   );
}

#endif // UMCD_PROTOCOL_HPP
