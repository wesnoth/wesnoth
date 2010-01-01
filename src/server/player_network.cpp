/* $Id$ */
/*
   Copyright (C) 2003 - 2010 by David White <dave@whitevine.net>
   Copyright (C) 2009 - 2010 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "player_network.hpp"
#include "../log.hpp"
#include "serialization/string_utils.hpp"

static lg::log_domain log_config("config");
#define WRN_CONFIG LOG_STREAM(warn, log_config)

namespace wesnothd {

namespace chat_message {

const size_t max_message_length = 256;

void truncate_message(const simple_wml::string_span& str, simple_wml::node& message)
{
	// testing for msg.size() is not sufficient but we're not getting false negatives
	// and it's cheaper than always converting to wstring.
	if(str.size() > static_cast<int>(chat_message::max_message_length)) {
		std::string tmp(str.begin(), str.end());
		// The string can contain utf-8 characters so truncate as wide_string otherwise
		// a corrupted utf-8 string can be returned.
		utils::truncate_as_wstring(tmp, max_message_length);
		message.set_attr_dup("message", tmp.c_str());
	}
}

} // end chat_message namespace

player_map::const_iterator find_user(const player_map& all_players,
									 const simple_wml::string_span& name)
{
	player_map::const_iterator pl;
	for (pl = all_players.begin(); pl != all_players.end(); ++pl) {
		if (name == pl->second.name().c_str()) {
			return pl;
		}
	}
	return all_players.end();
}

bool send_to_one(simple_wml::document& data, const network::connection sock, std::string packet_type)
{
	if (packet_type.empty())
		packet_type = data.root().first_child().to_string();
	try {
		simple_wml::string_span s = data.output_compressed();
		network::send_raw_data(s.begin(), s.size(), sock, packet_type);
	} catch (simple_wml::error& e) {
		WRN_CONFIG << __func__ << ": simple_wml error: " << e.message << std::endl;
		return false;
	}
	return true;
}

void send_to_many(simple_wml::document& data, const connection_vector& vec,
				  const network::connection exclude, std::string packet_type)
{
	if (packet_type.empty())
		packet_type = data.root().first_child().to_string();
	try {
		simple_wml::string_span s = data.output_compressed();
		for(connection_vector::const_iterator i = vec.begin(); i != vec.end(); ++i) {
			if (*i != exclude) {
				network::send_raw_data(s.begin(), s.size(), *i, packet_type);
			}
		}
	} catch (simple_wml::error& e) {
		WRN_CONFIG << __func__ << ": simple_wml error: " << e.message << std::endl;
	}
}

void send_to_many(simple_wml::document& data, const connection_vector& vec,
				  boost::function<bool (network::connection)> pred,
				  const network::connection exclude, std::string packet_type)
{
	if (packet_type.empty())
		packet_type = data.root().first_child().to_string();
	try {
		simple_wml::string_span s = data.output_compressed();
		for(connection_vector::const_iterator i = vec.begin(); i != vec.end(); ++i) {
			if ((*i != exclude) && pred(*i)) {
				network::send_raw_data(s.begin(), s.size(), *i, packet_type);
			}
		}
	} catch (simple_wml::error& e) {
		WRN_CONFIG << __func__ << ": simple_wml error: " << e.message << std::endl;
	}

}

} //end namespace wesnothd
