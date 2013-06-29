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

#ifndef CAMPAIGN_SERVER_WML_REPLY_HPP
#define CAMPAIGN_SERVER_WML_REPLY_HPP

#include <memory>
#include <ostream>

#include "campaign_server/network_data.hpp"
#include "serialization/parser.hpp"

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


#endif // CAMPAIGN_SERVER_WML_REPLY_HPP
