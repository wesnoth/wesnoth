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
#include "umcd/wml_request.hpp"

#include <stdexcept>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

wml_request::wml_request(){}

config& wml_request::get_metadata() 
{ 
	return metadata_; 
}

static void check_stream_state(std::istream& raw_data_stream, std::string error_msg)
{
	if(!raw_data_stream.good())
	{
		throw std::runtime_error(error_msg);
	}
}

std::string peek_request_name(std::istream& raw_data_stream)
{
	// Try to read the first tag which is the name of the packet.
	std::string error_msg("Invalid packet. The request name could not have been read.");
	char first_bracket;
	raw_data_stream >> first_bracket;
	check_stream_state(raw_data_stream, error_msg);
	if(first_bracket != '[')
		throw std::runtime_error(error_msg);
	std::string request_name;
	getline(raw_data_stream, request_name, ']');
	check_stream_state(raw_data_stream, error_msg);

	// Put back name in the stream. So the parsing of the config will be ok.
	raw_data_stream.putback(']');
	std::string::const_reverse_iterator b = request_name.rbegin();
	std::string::const_reverse_iterator e = request_name.rend();
	for(; b != e; ++b)
		raw_data_stream.putback(*b);
	raw_data_stream.putback(first_bracket);

	return request_name;
}