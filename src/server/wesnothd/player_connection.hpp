/*
	Copyright (C) 2016 - 2024
	by Sergey Popov <loonycyborg@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "server/wesnothd/player.hpp"
#include "server/common/server_base.hpp"

#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index_container.hpp>

namespace wesnothd
{
class game;

class player_record
{
public:
	template<class SocketPtr>
	player_record(const SocketPtr socket, const player& player)
		: login_time(std::chrono::steady_clock::now())
		, socket_(socket)
		, player_(player)
		, game_()
		, ip_address(client_address(socket))
	{
	}

	const any_socket_ptr socket() const
	{
		return socket_;
	}

	std::string client_ip() const
	{
		return ip_address;
	}

	player& info() const
	{
		return player_;
	}

	const std::string& name() const
	{
		return player_.name();
	}

	const std::shared_ptr<game> get_game() const;

	std::shared_ptr<game>& get_game();

	int game_id() const;

	void set_game(std::shared_ptr<game> new_game);

	void enter_lobby();

	const std::chrono::steady_clock::time_point login_time;

private:
	const any_socket_ptr socket_;
	mutable player player_;
	std::shared_ptr<game> game_;
	std::string ip_address;
};

struct socket_t {};
struct name_t {};
struct game_t {};

namespace bmi = boost::multi_index;

using player_connections = bmi::multi_index_container<player_record, bmi::indexed_by<
	bmi::ordered_unique<bmi::tag<socket_t>,
		bmi::const_mem_fun<player_record, const any_socket_ptr, &player_record::socket>>,
	bmi::hashed_unique<bmi::tag<name_t>,
		bmi::const_mem_fun<player_record, const std::string&, &player_record::name>>,
	bmi::ordered_non_unique<bmi::tag<game_t>,
		bmi::const_mem_fun<player_record, int, &player_record::game_id>>
>>;

typedef player_connections::const_iterator player_iterator;

} // namespace wesnothd
