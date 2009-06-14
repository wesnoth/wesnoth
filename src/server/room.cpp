/* $Id$ */
/*
   Copyright (C) 2009 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "game.hpp"
#include "player_network.hpp"
#include "room.hpp"
#include "../foreach.hpp"
#include "../log.hpp"

static lg::log_domain log_server("server");
#define ERR_ROOM LOG_STREAM(err, log_server)
#define LOG_ROOM LOG_STREAM(info, log_server)
#define DBG_ROOM LOG_STREAM(debug, log_server)

namespace wesnothd {

room::room(const std::string& name)
	: name_(name)
	, members_()
{
}

const std::string& room::name() const
{
	return name_;
}

bool room::add_player(network::connection player)
{
	if (is_member(player)) {
		ERR_ROOM << "ERROR: Player is already in this room. (socket: "
			<< player << ")\n";
		return false;
	}
	members_.push_back(player);
	return true;
}

void room::add_players(const wesnothd::game& game)
{
	foreach (network::connection player, game.all_game_users()) {
		add_player(player);
	}
}

void room::remove_player(network::connection player)
{
	const user_vector::iterator itor =
		std::find(members_.begin(), members_.end(), player);
	if (itor != members_.end()) {
		members_.erase(itor);
	} else {
		ERR_ROOM << "ERROR: Player is not in this room. (socket: "
			<< player << ")\n";
	}
}


void room::send_data(simple_wml::document& data,
					 const network::connection exclude,
					 std::string packet_type) const
{
	wesnothd::send_to_many(data, members(), exclude, packet_type);
}

void room::send_server_message_to_all(const char* message,
									  network::connection exclude) const
{
	simple_wml::document doc;
	send_server_message(message, 0, &doc);
	send_data(doc, exclude, "message");
}

void room::send_server_message(const char* message,
							   network::connection sock,
							   simple_wml::document* docptr) const
{
	simple_wml::document docbuf;
	if(docptr == NULL) {
		docptr = &docbuf;
	}
	simple_wml::document& doc = *docptr;

	simple_wml::node& msg = doc.root().add_child("message");
	msg.set_attr("sender", "server");
	msg.set_attr_dup("message", message);

	if(sock) {
		send_to_one(doc, sock, "message");
	}
}

void room::process_message(simple_wml::document& /*data*/,
						   const player_map::iterator /*user*/)
{
}

} //end namespace wesnothd
