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

RoomList::RoomList(PlayerMap& player_connections) : lobby_(room_ptr(new Room("lobby"))), player_connections_(player_connections)
{
}

void RoomList::enter_room(const std::string& room_name, socket_ptr socket)
{
	bool inserted;
	RoomMap::iterator iter;
	boost::tie(iter, inserted) = room_map_.insert(RoomMap::value_type(socket, room_name));
	if(inserted) {
		if(room_name == "lobby")
			iter->info = lobby_;
		else
			iter->info = room_ptr(new Room(room_name));
	}
}

void RoomList::leave_room(const std::string& room_name, socket_ptr socket)
{
	room_map_.erase(RoomMap::value_type(socket, room_name));
}
