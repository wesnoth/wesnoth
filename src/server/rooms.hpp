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

#ifndef SERVER_ROOMS_HPP_INCLUDED
#define SERVER_ROOMS_HPP_INCLUDED

#include "player_connection.hpp"

#include <boost/bimap/unordered_multiset_of.hpp>

namespace wesnothd
{

class Room : public boost::noncopyable
{
	std::string name_;
	std::string topic_;

	public:
	Room(const std::string& name);
	~Room();

	const std::string& topic() const { return topic_; }
	void set_topic(const std::string& topic) { topic_ = topic; }
};

typedef boost::shared_ptr<Room> room_ptr;

class RoomList : public boost::noncopyable
{
	room_ptr lobby_;

	typedef boost::bimaps::bimap<
		boost::bimaps::unordered_multiset_of<socket_ptr>, 
		boost::bimaps::unordered_multiset_of<std::string>,
		boost::bimaps::set_of_relation<>,
		boost::bimaps::with_info<room_ptr>
	> RoomMap;
	RoomMap room_map_;
	RoomMap stored_room_map_; // remember rooms of players that are in game
	PlayerConnections& player_connections_;

	room_ptr make_room(const std::string& room_name);
	public:
	RoomList(PlayerConnections& player_connections);
	
	bool in_lobby(socket_ptr socket) {
		return room_map_.left.count(socket);
	}

	void enter_room(const std::string& room_name, socket_ptr socket);
	void leave_room(const std::string& room_name, socket_ptr socket);
	void remove_player(socket_ptr socket);

	void send_to_room(const std::string& room_name, simple_wml::document& doc, socket_ptr exclude = socket_ptr()) const;
	void send_server_message(const std::string& room_name, const std::string& message, socket_ptr exclude = socket_ptr()) const;

	void enter_lobby(socket_ptr socket);
	void enter_lobby(wesnothd::game&);
	void exit_lobby(socket_ptr socket);

	Room& room(const std::string& room_name);

	private:
	void fill_member_list(const std::string room_name, simple_wml::node& root);
};

} //namespace wesnothd

#endif
