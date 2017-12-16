/* $Id$ */
/*
   Copyright (C) 2016 - 2017 by Sergey Popov <loonycyborg@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "simple_wml.hpp"
#include "player.hpp"
#include "server_base.hpp"

#ifndef _WIN32
#define  BOOST_ASIO_DISABLE_THREADS
#endif
#include <boost/asio.hpp>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/hashed_index.hpp>

namespace wesnothd
{

class player_record
{
	const socket_ptr socket_;
	mutable player player_;
	std::shared_ptr<game> game_;

	public:

	const socket_ptr socket() const { return socket_; }
	player& info() const { return player_; }
	const std::string& name() const { return player_.name(); }
	const std::shared_ptr<game> get_game() const;
	std::shared_ptr<game>& get_game();
	int game_id() const;
	static void set_game(player_record&, std::shared_ptr<game>);
	static void enter_lobby(player_record&);

	player_record(const socket_ptr socket, const player& player) : socket_(socket), player_(player) {}
};

struct socket_t{};
struct name_t{};
struct game_t{};

using namespace boost::multi_index;

typedef multi_index_container<
    player_record,
	indexed_by<
		ordered_unique<
            tag<socket_t>, BOOST_MULTI_INDEX_CONST_MEM_FUN(player_record,const socket_ptr,socket)>,
		hashed_unique<
            tag<name_t>, BOOST_MULTI_INDEX_CONST_MEM_FUN(player_record,const std::string&,name)>,
		ordered_non_unique<
            tag<game_t>, BOOST_MULTI_INDEX_CONST_MEM_FUN(player_record,int,game_id)>
	>
> player_connections;

} // namespace wesnothd
