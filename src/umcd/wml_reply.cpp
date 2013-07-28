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
#include "umcd/protocol/wml/umcd_protocol.hpp"
#include "serialization/parser.hpp"

wml_reply::wml_reply(){}

wml_reply::wml_reply(const config& metadata)
: metadata_(metadata.to_string())
, size_header_(make_size_header(this->metadata_.size()))
{
}

std::vector<boost::asio::const_buffer> wml_reply::to_buffers() const
{
  std::vector<boost::asio::const_buffer> buffers;
  buffers.push_back(boost::asio::buffer(size_header_));
  buffers.push_back(boost::asio::buffer(metadata_));
  return buffers;
}

std::string make_size_header(std::size_t num_bytes)
{
  std::string value = boost::lexical_cast<std::string>(num_bytes);
  int missing_zero = umcd_protocol::REQUEST_HEADER_SIZE_FIELD_LENGTH - value.size();
  std::string res;
  for(int i=0; i < missing_zero; ++i)
    res.push_back('0');
  return res + value;
}