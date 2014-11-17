/*
   Copyright (C) 2009 - 2014 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include "game.hpp"
#include "player_network.hpp"
#include "room_manager.hpp"

#include "../serialization/parser.hpp"
#include "../serialization/binary_or_text.hpp"
#include "../serialization/string_utils.hpp"
#include "../util.hpp"
#include "../filesystem.hpp"
#include "../log.hpp"

#include <boost/foreach.hpp>

static lg::log_domain log_server_lobby("server/lobby");
#define ERR_LOBBY LOG_STREAM(err, log_server_lobby)
#define WRN_LOBBY LOG_STREAM(warn, log_server_lobby)
#define LOG_LOBBY LOG_STREAM(info, log_server_lobby)
#define DBG_LOBBY LOG_STREAM(debug, log_server_lobby)

static lg::log_domain log_server("server");
#define ERR_SERVER LOG_STREAM(err, log_server)
#define WRN_SERVER LOG_STREAM(warn, log_server)
#define LOG_SERVER LOG_STREAM(info, log_server)
#define DBG_SERVER LOG_STREAM(debug, log_server)

namespace wesnothd {

const char* const room_manager::lobby_name_ = "lobby";

room_manager::room_manager(player_map &all_players)
	: all_players_(all_players)
	, lobby_(NULL)
	, rooms_by_name_()
	, rooms_by_player_()
	, player_stored_rooms_()
	, filename_()
	, compress_stored_rooms_(true)
	, new_room_policy_(PP_EVERYONE)
	, dirty_(false)
{
}

room_manager::~room_manager()
{
	// this assumes the server is shutting down, so there's no need to
	// send the actual room-quit messages to clients
	write_rooms();
	BOOST_FOREACH(t_rooms_by_name_::value_type i, rooms_by_name_) {
		delete i.second;
	}
}

room_manager::PRIVILEGE_POLICY room_manager::pp_from_string(const std::string& str)
{
	if (str == "everyone") {
		return PP_EVERYONE;
	} else if (str == "registered") {
		return PP_REGISTERED;
	} else if (str == "admins") {
		return PP_ADMINS;
	} else if (str == "nobody") {
		return PP_NOBODY;
	}
	return PP_COUNT;
}

void room_manager::load_config(const config& cfg)
{
	filename_ = cfg["room_save_file"].str();
	compress_stored_rooms_ = cfg["compress_stored_rooms"].to_bool(true);
	PRIVILEGE_POLICY pp = pp_from_string(cfg["new_room_policy"]);
	if (pp != PP_COUNT) new_room_policy_ = pp;
}

void room_manager::read_rooms()
{
	if (!filename_.empty() && filesystem::file_exists(filename_)) {
		LOG_LOBBY << "Reading rooms from " <<  filename_ << "\n";
		config cfg;
		filesystem::scoped_istream file = filesystem::istream_file(filename_);
		if (compress_stored_rooms_) {
			read_gz(cfg, *file);
		} else {
			read(cfg, *file);
		}

		BOOST_FOREACH(const config &c, cfg.child_range("room")) {
			room* r(new room(c));
			if (room_exists(r->name())) {
				ERR_LOBBY << "Duplicate room ignored in stored rooms: "
					<< r->name() << "\n";
				delete r;
			} else {
				rooms_by_name_.insert(std::make_pair(r->name(), r));
			}
		}
	}
	lobby_ = get_room(lobby_name_);
	if (lobby_ == NULL) {
		lobby_ = create_room(lobby_name_);
		lobby_->set_persistent(true);
		lobby_->set_logged(true);
		dirty_ = true;
	}
}

void room_manager::write_rooms()
{
	if (filename_.empty()) return;
	LOG_LOBBY << "Writing rooms to " << filename_ << "\n";
	config cfg;
	BOOST_FOREACH(const t_rooms_by_name_::value_type& v, rooms_by_name_) {
		const room& r = *v.second;
		if (r.persistent()) {
			config& c = cfg.add_child("room");
			r.write(c);
		}
	}

	filesystem::scoped_ostream file = filesystem::ostream_file(filename_);
	config_writer writer(*file, compress_stored_rooms_);
	writer.write(cfg);
	dirty_ = false;
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
		bool can_create = false;
		switch (new_room_policy_) {
			case PP_EVERYONE:
				can_create = true;
				break;
			case PP_REGISTERED:
				{
					player_map::iterator i = all_players_.find(player);
					if (i != all_players_.end()) {
						can_create = i->second.registered();
					}
				}
				break;
			case PP_ADMINS:
				{
					player_map::iterator i = all_players_.find(player);
					if (i != all_players_.end()) {
						can_create = i->second.is_moderator();
					}
				}
				break;
			default:
				break;
		}
		if (can_create) { //TODO: check if player can create room
			//TODO: filter room names for abuse?
			r = create_room(name);
		} else {
			lobby_->send_server_message("The room does not exist", player);
			return NULL;
		}
	}
	return r;
}

void room_manager::enter_lobby(network::connection player)
{
	lobby_->add_player(player);
	unstore_player_rooms(player);
}

void room_manager::enter_lobby(const wesnothd::game &game)
{
	BOOST_FOREACH(network::connection player, game.all_game_users()) {
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
		BOOST_FOREACH(room* r, i->second) {
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
		BOOST_FOREACH(room* r, i->second) {
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
			<< "a nonexistent room '" << room_name << "'\n";
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
	BOOST_FOREACH(room* r, i->second) {
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
	BOOST_FOREACH(const std::string& room_name, it->second) {
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
		join_msg.set_attr_dup("topic", r->topic().c_str());
		send_to_one(doc, user->first);
	}
}

void room_manager::process_message(simple_wml::document &data, const player_map::iterator user)
{
	simple_wml::node* const message = data.root().child("message");
	assert (message);
	message->set_attr_dup("sender", user->second.name().c_str());
	std::string room_name = message->attr("room").to_string();
	if (room_name.empty()) room_name = lobby_name_;
	room* r = require_member(room_name, user, "message");
	if (r == NULL) {
		std::stringstream ss;
		ss << "You are not a member of the room '" << room_name << "'. "
			<< "Your message has not been relayed.";
		lobby_->send_server_message(ss.str(), user->first);
		return;
	}
	if (user->second.is_message_flooding()) {
		r->send_server_message(
				"Warning: you are sending too many messages too fast. "
				"Your message has not been relayed.", user->first);
		return;
	}
	const simple_wml::string_span& msg = (*message)["message"];
	chat_message::truncate_message(msg, *message);
	if (r->logged()) {
		if (msg.size() >= 3 && simple_wml::string_span(msg.begin(), 4) == "/me ") {
			LOG_SERVER << network::ip_address(user->first)
				<< "\t<" << user->second.name()
				<< simple_wml::string_span(msg.begin() + 3, msg.size() - 3)
				<< ">\n";
		} else {
			LOG_SERVER << network::ip_address(user->first) << "\t<"
				<< user->second.name() << "> " << msg << "\n";
		}
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
	msg->set_attr_dup("topic", r->topic().c_str());
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
	if (r->empty() && !r->persistent()) {
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

	/* room-specific queries */
	room* r = require_room(room_name, user, "query");
	if (r == NULL) return;
	resp.set_attr_dup("room", room_name.c_str());
	q = msg->child("names");
	if (q != NULL) {
		fill_member_list(r, resp);
		send_to_one(doc, user->first);
		return;
	}
	q = msg->child("persist");
	if (q != NULL) {
		if (user->second.is_moderator()) {
			WRN_LOBBY << "Attempted room set persistent by non-moderator";
		} else {
			if (q->attr("value").empty()) {
				if (r->persistent()) {
					resp.set_attr("message", "Room is persistent.");
				} else {
					resp.set_attr("message", "Room is not persistent.");
				}
			} else if (q->attr("value").to_bool()) {
				r->set_persistent(true);
				resp.set_attr("message", "Room set as persistent.");
				dirty_ = true;
			} else {
				r->set_persistent(false);
				resp.set_attr("message", "Room set as not persistent.");
				dirty_ = true;
			}
			send_to_one(doc, user->first);
		}
		return;
	}
	q = msg->child("logged");
	if (q != NULL) {
		if (user->second.is_moderator()) {
			WRN_LOBBY << "Attempted room set logged by non-moderator.";
		} else {
			if (q->attr("value").empty()) {
				if (r->persistent()) {
					resp.set_attr("message", "Room is logged.");
				} else {
					resp.set_attr("message", "Room is not logged.");
				}
			} else if (q->attr("value").to_bool()) {
				r->set_logged(true);
				resp.set_attr("message", "Room set as logged.");
				dirty_ = true;
			} else {
				r->set_logged(false);
				resp.set_attr("message", "Room set as not logged.");
				dirty_ = true;
			}
			send_to_one(doc, user->first);
		}
		return;
	}
	q = msg->child("topic");
	if (q != NULL) {
		if (q->attr("value").empty()) {
			resp.set_attr_dup("topic", r->topic().c_str());
			send_to_one(doc, user->first);
		} else {
			if (user->second.is_moderator()) {
				WRN_LOBBY << "Attempted room set topic by non-moderator.";
			} else {
				r->set_topic(q->attr("value").to_string());
				resp.set_attr("message", "Room topic changed.");
				send_to_one(doc, user->first);
			}
		}
	}
	r->send_server_message("Unknown room query type", user->first);
}

void room_manager::fill_room_list(simple_wml::node& root)
{
	simple_wml::node& rooms = root.add_child("rooms");
	BOOST_FOREACH(const t_rooms_by_name_::value_type& tr, rooms_by_name_) {
		const room& r = *tr.second;
		simple_wml::node& room = rooms.add_child("room");
		room.set_attr_dup("name", r.name().c_str());
		room.set_attr_dup("size", lexical_cast_default<std::string>(r.members().size()).c_str());
	}
}

void room_manager::fill_member_list(const room* room, simple_wml::node& root)
{
	simple_wml::node& members = root.add_child("members");
	BOOST_FOREACH(network::connection m, room->members()) {
		simple_wml::node& member = members.add_child("member");
		player_map::const_iterator mi = all_players_.find(m);
		if (mi != all_players_.end()) {
			member.set_attr_dup("name", mi->second.name().c_str());
		}
	}
}
} //namespace wesnothd
