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

const char* const room_manager::lobby_name_ = "lobby";

room_manager::room_manager(player_map &all_players)
: all_players_(all_players), lobby_(NULL)
{
	lobby_ = create_room(lobby_name_);
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

room* room_manager::require_member(const std::string& room_name,
                                   const player_map::iterator user,
                                   const char *log_string)
{
	room* r = get_room(room_name);
	if (r == NULL) {
		WRN_LOBBY << "Player " << user->second.name()
			<< " (conn " << user->first << ")"
			<< " attempted to " << log_string
			<< "a nonexistant room '" << room_name << "'\n";
		return NULL;
	}
	if (!r->is_member(user->first)) {
		WRN_LOBBY << "Player " << user->second.name()
			<< " (conn " << user->first << ")"
			<< " attempted to " << log_string
			<< "room '" << room_name << "', but is not a member of that room\n";
		return NULL;
	}
	return r;
}

bool room_manager::player_enters_room(network::connection player, wesnothd::room *room)
{
	if (room->is_member(player)) {
		room->send_server_message("You are already in this room", player);
		return false;
	}
	//TODO: implement per-room bans, check ban status here
	room->add_player(player);
	rooms_by_player_[player].insert(room);
	room->send_server_message(room->name().c_str(), player); //debug
	return true;
}

void room_manager::player_exits_room(network::connection player, wesnothd::room *room)
{
	room->remove_player(player);
	rooms_by_player_[player].erase(room);
	room->send_server_message(room->name().c_str(), player); //debug
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
	if (room_name.empty()) room_name = lobby_name_;
	room* r = require_member(room_name, user, "message");
	if (r == NULL) return;

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
	r->send_data(data, user->first, "message");
}

void room_manager::process_room_join(simple_wml::document &data, const player_map::iterator user)
{
	simple_wml::node* const msg = data.root().child("room_join");
	assert(msg);
	std::string room_name = msg->attr("room").to_string();
	room* r = get_room(room_name);
	if (r == NULL) {
		if (1) { //TODO: check if player can create room
			//TODO: filter room names for abuse?
			r = create_room(room_name);
		} else {
			lobby_->send_server_message("The room does not exist", user->first);
			return;
		}
	}
	if (!player_enters_room(user->first, r)) {
		return; //player was unable to join room
	}
	// notify other members
	msg->set_attr_dup("player", user->second.name().c_str());
	r->send_data(data, user->first);
	// send member list to the new member
	foreach (network::connection m, r->members()) {
		simple_wml::node& member = msg->add_child("member");
		player_map::const_iterator mi = all_players_.find(m);
		if (mi != all_players_.end()) {
			member.set_attr_dup("name", mi->second.name().c_str());
		}
	}
	send_to_one(data, user->first);
}

void room_manager::process_room_part(simple_wml::document &data, const player_map::iterator user)
{
	simple_wml::node* const msg = data.root().child("room_part");
	assert(msg);
	std::string room_name = msg->attr("room").to_string();
	if (room_name == lobby_name_) {
		lobby_->send_server_message("You cannot quit the lobby", user->first);
		return;
	}
	room* r = require_member(room_name, user, "quit");
	if (r == NULL) return;
	player_exits_room(user->first, r);
	msg->set_attr_dup("player", user->second.name().c_str());
	r->send_data(data);
}

} //namespace wesnothd
