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

#ifndef SERVER_PLAYER_CONNECTION_HPP_INCLUDED
#define SERVER_PLAYER_CONNECTION_HPP_INCLUDED

#include "simple_wml.hpp"
#include "player.hpp"

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/hashed_index.hpp>

namespace wesnothd
{

typedef boost::shared_ptr<boost::asio::ip::tcp::socket> socket_ptr;

class PlayerRecord
{
	const socket_ptr socket_;
	mutable player player_;
	boost::shared_ptr<game> game_;

	public:

	const socket_ptr socket() const { return socket_; }
	player& info() const { return player_; }
	const std::string& name() const { return player_.name(); }
	const boost::shared_ptr<game> get_game() const;
	boost::shared_ptr<game>& get_game();
	int game_id() const;
	static void set_game(PlayerRecord&, boost::shared_ptr<game>);
	static void enter_lobby(PlayerRecord&);

	PlayerRecord(const socket_ptr socket, const player& player) : socket_(socket), player_(player) {}
};

struct socket_t{};
struct name_t{};
struct game_t{};

using namespace boost::multi_index;

typedef multi_index_container<
	PlayerRecord,
	indexed_by<
		ordered_unique<
			tag<socket_t>, BOOST_MULTI_INDEX_CONST_MEM_FUN(PlayerRecord,const socket_ptr,socket)>,
		hashed_unique<
			tag<name_t>, BOOST_MULTI_INDEX_CONST_MEM_FUN(PlayerRecord,const std::string&,name)>,
		ordered_non_unique<
			tag<game_t>, BOOST_MULTI_INDEX_CONST_MEM_FUN(PlayerRecord,int,game_id)>
	>
> PlayerConnections;

void send_to_player(socket_ptr socket, simple_wml::document& doc);

template<typename Container>
void send_to_players(simple_wml::document& data, const Container& players, socket_ptr exclude = socket_ptr())
{
	typename Container::const_iterator iter = players.begin(), iter_end = players.end();
	for(;iter != iter_end; ++iter)
		if(*iter != exclude)
			send_to_player(*iter, data);
}

void send_server_message(socket_ptr socket, const std::string& message);

std::string client_address(socket_ptr socket);

} // namespace wesnothd

#endif
