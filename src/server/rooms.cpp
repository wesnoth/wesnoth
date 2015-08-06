/* $Id$ */
/*
   Copyright (C) 2012 by Sergey Popov <loonycyborg@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "rooms.hpp"
#include "log.hpp"

#include <boost/foreach.hpp>

static lg::log_domain log_server("server");
#define ERR_SERVER LOG_STREAM(err, log_server)
#define WRN_SERVER LOG_STREAM(warn, log_server)
#define LOG_SERVER LOG_STREAM(info, log_server)
#define DBG_SERVER LOG_STREAM(debug, log_server)

Room::Room(const std::string& name) : name_(name)
{
	LOG_SERVER << "Created room '" << name_ << "'\n";
}

Room::~Room()
{
	LOG_SERVER << "Destroyed room '" << name_ << "'\n";
}

RoomList::RoomList(PlayerMap& player_connections) : lobby_(make_room("lobby")), player_connections_(player_connections)
{
}

room_ptr RoomList::make_room(const std::string& room_name)
{
	return room_ptr(new Room(room_name));
}

void RoomList::enter_room(const std::string& room_name, socket_ptr socket)
{
	RoomMap::right_iterator existing_room_iter = room_map_.right.find(room_name);
	bool inserted;
	RoomMap::iterator iter;
	boost::tie(iter, inserted) = room_map_.insert(RoomMap::value_type(socket, room_name));
	if(inserted) {
		if(room_name == "lobby")
			iter->info = lobby_;
		else {
			if(existing_room_iter == room_map_.right.end())
				iter->info = make_room(room_name);
			else
				iter->info = existing_room_iter->info;
		}
		simple_wml::document room_join;
		simple_wml::node& msg = room_join.root().add_child("room_join");
		msg.set_attr_dup("player", player_connections_.left.at(iter->left).c_str());
		msg.set_attr_dup("room", room_name.c_str());
		send_to_room(room_name, room_join, socket);

		fill_member_list(room_name, msg);
		send_to_player(socket, room_join);
	}
}

void RoomList::fill_member_list(const std::string room_name, simple_wml::node& root)
{
	simple_wml::node& members = root.add_child("members");
	BOOST_FOREACH(const RoomMap::right_value_type& value, room_map_.right.equal_range(room_name)) {
		simple_wml::node& member = members.add_child("member");
		member.set_attr_dup("name", player_connections_.left.at(value.second).c_str());
	}
}

void RoomList::leave_room(const std::string& room_name, socket_ptr socket)
{
	room_map_.erase(RoomMap::value_type(socket, room_name));
}

void RoomList::remove_player(socket_ptr socket)
{
	std::pair<RoomMap::left_iterator, RoomMap::left_iterator> range = room_map_.left.equal_range(socket);
	room_map_.left.erase(range.first, range.second);
}

void RoomList::send_to_room(const std::string& room_name, simple_wml::document& doc, socket_ptr exclude) const
{
	BOOST_FOREACH(const RoomMap::right_value_type& value, room_map_.right.equal_range(room_name)) {
		socket_ptr recipient = value.second;
		if(recipient != exclude)
			send_to_player(recipient, doc);
	}
}

void RoomList::send_server_message(const std::string& room_name, const std::string& message, socket_ptr exclude) const
{
	simple_wml::document server_message;
	simple_wml::node& msg = server_message.root().add_child("message");
	msg.set_attr("sender", "server");
	msg.set_attr_dup("message", message.c_str());
	send_to_room(room_name, server_message, exclude);
}

void RoomList::exit_lobby(socket_ptr socket)
{
	RoomMap::left_iterator iter, iter_end;
	boost::tie(iter, iter_end) = room_map_.left.equal_range(socket);
	for(; iter != iter_end; ++iter) {
		stored_room_map_.insert(*(room_map_.project_up(iter)));
	}
	room_map_.left.erase(socket);
}

Room& RoomList::room(const std::string& room_name)
{
	RoomMap::right_iterator iter = room_map_.right.find(room_name);
	if(iter == room_map_.right.end())
		throw std::runtime_error("Room '" + room_name + "' doesn't exist");
	
	return *(iter->info);
}
