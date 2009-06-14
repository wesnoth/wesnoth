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

#include "player_network.hpp"
#include "room_manager.hpp"
#include "../foreach.hpp"
#include "../log.hpp"
static lg::log_domain log_server_lobby("server/lobby");
#define ERR_LOBBY LOG_STREAM(err, log_server_lobby)
#define WRN_LOBBY LOG_STREAM(warn, log_server_lobby)
#define LOG_LOBBY LOG_STREAM(info, log_server_lobby)
#define DBG_LOBBY LOG_STREAM(debug, log_server_lobby)

namespace wesnothd {

room_manager::room_manager(player_map &all_players)
: all_players_(all_players), lobby_(NULL)
{
	lobby_ = create_room("lobby");
}

room_manager::~room_manager()
{
	// this assumes the server is shutting down, so there's no need to
	// send the actual room-quit messages to clients
	foreach (t_rooms_by_name_::value_type i, rooms_by_name_) {
		delete i.second;
	}
}

room* room_manager::get_room(const std::string &name)
{
	t_rooms_by_name_::iterator i = rooms_by_name_.find(name);
	if (i != rooms_by_name_.end()) {
		return i->second;
	} else {
		return NULL;
	}
}

bool room_manager::room_exists(const std::string &name) const
{
	return rooms_by_name_.find(name) != rooms_by_name_.end();
}

room* room_manager::create_room(const std::string &name)
{
	if (room_exists(name)) {
		DBG_LOBBY << "Requested creation of already existing room '" << name << "'\n";
		return NULL;
	}
	room* r = new room(name);
	rooms_by_name_.insert(std::make_pair(name, r));
	return r;
}

void room_manager::delete_room(const std::string &name)
{
	room* r = get_room(name);
	if (r == NULL) {
		DBG_LOBBY << "Requested deletion of nonexistant room '" << name << "'\n";
		return;
	}
	simple_wml::document doc;
	simple_wml::node& exit = doc.root().add_child("exit_room");
	exit.set_attr_dup("room", name.c_str());
	exit.set_attr("reason", "room deleted");
	r->send_data(doc);
	rooms_by_name_.erase(name);
	foreach (network::connection p, r->members()) {
		rooms_by_player_[p].erase(r);
	}
}

void room_manager::enter_lobby(network::connection player)
{
	lobby_->add_player(player);
}

void room_manager::enter_lobby(const wesnothd::game &game)
{
	lobby_->add_players(game);
}

void room_manager::exit_lobby(network::connection player)
{
	lobby_->remove_player(player);
}

bool room_manager::in_lobby(network::connection player) const
{
	return lobby_->is_member(player);
}

void room_manager::remove_player(network::connection player)
{
	lobby_->remove_player(player);
	t_rooms_by_player_::iterator i = rooms_by_player_.find(player);
	if (i != rooms_by_player_.end()) {
		foreach (room* r, i->second) {
			r->remove_player(player);
		}
	}
	rooms_by_player_.erase(player);
}

void room_manager::player_joins_room(network::connection player, wesnothd::room *room)
{
	room->add_player(player);
	rooms_by_player_[player].insert(room);
	simple_wml::document doc;
	simple_wml::node& join = doc.root().add_child("join");
	join.set_attr_dup("room", room->name().c_str());
	player_map::const_iterator i = all_players_.find(player);
	if (i == all_players_.end()) {
		WRN_LOBBY << "player " << player << " joins room but is not in all_players\n";
		return;
	}
	join.set_attr_dup("player", i->second.name().c_str());
	room->send_data(doc, player);
	foreach (network::connection m, room->members()) {
		simple_wml::node& member = join.add_child("member");
		player_map::const_iterator mi = all_players_.find(m);
		if (mi != all_players_.end()) {
			member.set_attr_dup("name", mi->second.name().c_str());
		}
	}
	send_to_one(doc, player);
}

void room_manager::player_quits_room(network::connection player, wesnothd::room *room)
{
	room->remove_player(player);
	rooms_by_player_[player].erase(room);
	simple_wml::document doc;
	simple_wml::node& quit = doc.root().add_child("quit");
	quit.set_attr_dup("room", room->name().c_str());
	player_map::const_iterator i = all_players_.find(player);
	if (i == all_players_.end()) {
		WRN_LOBBY << "player " << player << " quits room but is not in all_players\n";
		return;
	}
	quit.set_attr_dup("player", i->second.name().c_str());
	room->send_data(doc);
}

void room_manager::process_message(simple_wml::document &data, const player_map::iterator user)
{
	if (user->second.silenced()) {
		return;
	} else if (user->second.is_message_flooding()) {
		lobby_->send_server_message(
				"Warning: you are sending too many messages too fast. "
				"Your message has not been relayed.", user->first);
		return;
	}
	simple_wml::node* const message = data.root().child("message");
	assert (message);
	message->set_attr_dup("sender", user->second.name().c_str());
	std::string room_name = message->attr("room").to_string();
	//todo: dispatch to the appropriate room, check if the player is in that room ...
	const simple_wml::string_span& msg = (*message)["message"];
	chat_message::truncate_message(msg, *message);
	if (msg.size() >= 3 && simple_wml::string_span(msg.begin(), 4) == "/me ") {
		LOG_LOBBY << network::ip_address(user->first)
			<< "\t<" << user->second.name()
			<< simple_wml::string_span(msg.begin() + 3, msg.size() - 3)
			<< ">\n";
	} else {
		LOG_LOBBY << network::ip_address(user->first) << "\t<"
			<< user->second.name() << "> " << msg << "\n";
	}
	lobby_->send_data(data, user->first, "message");
}


} //namespace wesnothd