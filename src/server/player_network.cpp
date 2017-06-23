/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
   Copyright (C) 2009 - 2017 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "server/player_network.hpp"
#include "log.hpp"
#include "serialization/unicode.hpp"

lg::log_domain log_config_pn("config");
#define WRN_CONFIG LOG_STREAM(warn, log_config_pn)

namespace wesnothd {

namespace chat_message {

const size_t max_message_length = 256;

void truncate_message(const simple_wml::string_span& str, simple_wml::node& message)
{
	// testing for msg.size() is not sufficient but we're not getting false negatives
	// and it's cheaper than always converting to ucs4::string.
	if(str.size() > static_cast<int>(chat_message::max_message_length)) {
		std::string tmp(str.begin(), str.end());
		// The string can contain utf-8 characters so truncate as ucs4::string otherwise
		// a corrupted utf-8 string can be returned.
		utf8::truncate_as_ucs4(tmp, max_message_length);
		message.set_attr_dup("message", tmp.c_str());
	}
}

} // end chat_message namespace

} //end namespace wesnothd
