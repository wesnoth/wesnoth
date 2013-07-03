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

#include "umcd/network_data.hpp"

class wml_request
{
private:
   network_data data;
   const config& server_conf;

   void check_stream_state(std::istream& raw_data_stream, std::string error_msg);
   std::string peek_name(std::istream& raw_data_stream);

public:
   wml_request(std::istream& raw_data_stream, const config& server_conf);
   network_data& get_data();
   const config& get_conf() const;
   std::string name() const;
};


#endif // UMCD_WML_REQUEST_HPP