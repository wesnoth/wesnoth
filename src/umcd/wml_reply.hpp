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

#ifndef UMCD_WML_REPLY_HPP
#define UMCD_WML_REPLY_HPP

#include <ostream>
#include <boost/asio.hpp>

#include "config.hpp"

class wml_reply
{
public:
   std::string metadata;
   std::string size_header;
   
   wml_reply();
   wml_reply(const config& metadata);
   std::vector<boost::asio::const_buffer> to_buffers() const;
};

std::string make_size_header(size_t num_bytes);

#endif // UMCD_WML_REPLY_HPP
