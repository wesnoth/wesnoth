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

#ifndef UMCD_PROTOCOL_ECHO_HPP
#define UMCD_PROTOCOL_ECHO_HPP

#include <string>
#include <array>
#include <vector>

class echo_protocol
{
public:
   class reply
   {
      std::string request_data;
   public:
      reply(std::istream& raw_data_stream)
      {
         raw_data_stream >> request_data;
      }

      void send(std::ostream& raw_data_stream) const
      {
         raw_data_stream << request_data;
      }
   };
   typedef reply reply_type;

   void handle_request(std::iostream& raw_request_stream) const
   {
      reply_type(raw_request_stream).send(raw_data_stream);
   }
};

#endif // UMCD_PROTOCOL_ECHO_HPP
