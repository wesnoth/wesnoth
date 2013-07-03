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
#include "umcd/wml_reply.hpp"
#include "serialization/parser.hpp"

wml_reply::wml_reply(){}

wml_reply::wml_reply(const network_data& data)
: data(data)
{}

void wml_reply::send(std::ostream& raw_data_stream) const
{
   write(raw_data_stream, data.get_metadata());
}

wml_reply make_reply(const config& metadata, const std::string& data)
{
   return wml_reply(network_data(metadata, data));
}

wml_reply make_reply(const config& metadata)
{
   return make_reply(metadata, "");
}