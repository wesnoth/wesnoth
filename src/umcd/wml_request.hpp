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

#include "serialization/one_hierarchy_validator.hpp"
#include "serialization/parser.hpp"
#include "umcd/network_data.hpp"
#include "umcd/umcd_logger.hpp"

class wml_request
{
public:
   typedef schema_validation::one_hierarchy_validator validator_type;
   typedef boost::shared_ptr<validator_type> validator_ptr;
   typedef boost::asio::ip::tcp::iostream network_stream;
private:
   network_data data;
   network_stream& stream;

public:
   wml_request(network_stream& raw_data_stream, const validator_ptr& validator);

   network_data& get_data();
   network_stream& get_stream();
   std::string name() const;
};

std::string peek_request_name(std::istream& raw_data_stream);

wml_request make_request(boost::asio::ip::tcp::iostream& raw_data_stream, const std::string& validator_file_path);

#endif // UMCD_WML_REQUEST_HPP