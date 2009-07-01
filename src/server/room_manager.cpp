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

room* room_manager::get_create_room(const std::string &name, network::connection player)
{
	room* r = get_room(name);
	if (r == NULL) {
		if (1) { //TODO: check if player can create room
			//TODO: filter room names for abuse?
			r = create_room(name);
		} else {
			lobby_->send_server_message("The room does not exist", player);
			return NULL;
		}
	}
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
	unstore_player_rooms(player);
}

void room_manager::enter_lobby(const wesnothd::game &game)
{
	foreach (network::connection player, game.all_game_users()) {
		enter_lobby(player);
	}
}

void room_manager::exit_lobby(network::connection player)
{
	// No messages are sent to the rooms the player is in because other members
	// will receive the "player-entered-game" message (or similar) anyway, and
	// will be able to deduce that he or she is no longer in any rooms
	lobby_->remove_player(player);
	store_player_rooms(player);
	t_rooms_by_player_::iterator i = rooms_by_player_.find(player);
	if (i != rooms_by_player_.end()) {
		foreach (room* r, i->second) {
			r->remove_player(player);
		}
	}
	rooms_by_player_.erase(player);
}

bool room_manager::in_lobby(network::connection player) const
{
	return lobby_->is_member(player);
}

void room_manager::remove_player(network::connection player)
{
	// No messages are sent since a player-quit message is sent to everyone
	// anyway.
	lobby_->remove_player(player);
	t_rooms_by_player_::iterator i = rooms_by_player_.find(player);
	if (i != rooms_by_player_.end()) {
		foreach (room* r, i->second) {
			r->remove_player(player);
		}
	}
	rooms_by_player_.erase(player);
	player_stored_rooms_.erase(player);
}

room* room_manager::require_room(const std::string& room_name,
                                   const player_map::iterator user,
                                   const char *log_string)
{
	room* r = get_room(room_name);
	if (r == NULL) {
		lobby_->send_server_message("The room does not exist", user->first);
		WRN_LOBBY << "Player " << user->second.name()
			<< " (conn " << user->first << ")"
			<< " attempted to " << log_string
			<< "a nonexistant room '" << room_name << "'\n";
		return NULL;
	}
	return r;
}

room* room_manager::require_member(const std::string& room_name,
                                   const player_map::iterator user,
                                   const char *log_string)
{
	room* r = require_room(room_name, user, log_string);
	if (r == NULL) return NULL;
	if (!r->is_member(user->first)) {
		lobby_->send_server_message("You are not a member of this room", user->first);
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
	return true;
}

void room_manager::player_exits_room(network::connection player, wesnothd::room *room)
{
	room->remove_player(player);
	rooms_by_player_[player].erase(room);
}

void room_manager::store_player_rooms(network::connection player)
{
	t_rooms_by_player_::iterator i = rooms_by_player_.find(player);
	if (i == rooms_by_player_.end()) {
		return;
	}
	if (i->second.size() < 1) {
		return;
	}
	t_player_stored_rooms_::iterator it =
		player_stored_rooms_.insert(std::make_pair(player, std::set<std::string>())).first;
	std::set<std::string>& store = it->second;
	foreach (room* r, i->second) {
		store.insert(r->name());
	}
}

void room_manager::unstore_player_rooms(network::connection player)
{
	player_map::iterator i = all_players_.find(player);
	if (i != all_players_.end()) {
		unstore_player_rooms(i);
	}
}

void room_manager::unstore_player_rooms(const player_map::iterator user)
{
	t_player_stored_rooms_::iterator it = player_stored_rooms_.find(user->first);
	if (it == player_stored_rooms_.end()) {
		return;
	}
	simple_wml::document doc;
	simple_wml::node& join_msg = doc.root().add_child("room_join");
	join_msg.set_attr_dup("player", user->second.name().c_str());
	foreach (const std::string& room_name, it->second) {
		room* r = get_create_room(room_name, user->first);
		if (r == NULL) {
			LOG_LOBBY << "Player " << user->second.name() << " unable to rejoin room " << room_name << "\n";
			continue;
		}
		player_enters_room(user->first, r);
		join_msg.set_attr_dup("room", room_name.c_str());
		r->send_data(doc, user->first);
		join_msg.remove_child("members", 0);
		fill_member_list(r, join_msg);
		send_to_one(doc, user->first);
	}
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
	room* r = get_create_room(room_name, user->first);
	if (r == NULL) {
		return;
	}
	if (!player_enters_room(user->first, r)) {
		return; //player was unable to join room
	}
	// notify other members
	msg->set_attr_dup("player", user->second.name().c_str());
	r->send_data(data, user->first);
	// send member list to the new member
	fill_member_list(r, *msg);
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
	if (r->members().empty()) {
		LOG_LOBBY << "Last player left room " << room_name << ". Deleting room.\n";
		rooms_by_name_.erase(room_name);
		delete r;
	}
	send_to_one(data, user->first);
}

void room_manager::process_room_query(simple_wml::document& data, const player_map::iterator user)
{
	simple_wml::node* const msg = data.root().child("room_query");
	assert(msg);
	simple_wml::document doc;
	simple_wml::node& resp = doc.root().add_child("room_query_response");
	simple_wml::node* q;
	q = msg->child("rooms");
	if (q != NULL) {
		fill_room_list(resp);
		send_to_one(doc, user->first);
		return;
	}
	std::string room_name = msg->attr("room").to_string();
	if (room_name.empty()) room_name = lobby_name_;
	room* r = require_room(room_name, user, "query");
	if (r == NULL) return;
	resp.set_attr_dup("room", room_name.c_str());
	q = msg->child("names");
	if (q != NULL) {
		fill_member_list(r, resp);
		send_to_one(doc, user->first);
		return;
	}
	r->send_server_message("Unknown room query type", user->first);
}

void room_manager::fill_room_list(simple_wml::node& root)
{
	simple_wml::node& rooms = root.add_child("rooms");
	foreach (const t_rooms_by_name_::value_type& tr, rooms_by_name_) {
		const room& r = *tr.second;
		simple_wml::node& room = rooms.add_child("room");
		room.set_attr_dup("name", r.name().c_str());
		room.set_attr_dup("size", lexical_cast<std::string>(r.members().size()).c_str());
	}
}

void room_manager::fill_member_list(const room* room, simple_wml::node& root)
{
	simple_wml::node& members = root.add_child("members");
	foreach (network::connection m, room->members()) {
		simple_wml::node& member = members.add_child("member");
		player_map::const_iterator mi = all_players_.find(m);
		if (mi != all_players_.end()) {
			member.set_attr_dup("name", mi->second.name().c_str());
		}
	}
}
} //namespace wesnothd
