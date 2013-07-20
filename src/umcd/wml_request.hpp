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

#ifndef UMCD_WML_REQUEST_HPP
#define UMCD_WML_REQUEST_HPP

#include <istream>
#include <string>
#include "config.hpp"
#include "umcd/umcd_logger.hpp"
#include "umcd/server/connection.hpp"

class umcd_protocol;

class wml_request
{
public:
   typedef connection<umcd_protocol> connection_type;
   typedef boost::shared_ptr<connection_type> connection_ptr;
private:
   config metadata;
   connection_ptr client_connection;

public:
   wml_request();
   wml_request(const connection_ptr& client_connection);

   config& get_metadata();
   connection_ptr& get_connection();
};

std::string peek_request_name(std::istream& raw_data_stream);

#endif // UMCD_WML_REQUEST_HPP