/*
	Copyright (C) 2016 - 2025
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
	template<typename SocketPtr, typename... Args>
	player_record(const SocketPtr socket, Args&&... args)
		: login_time_(std::chrono::steady_clock::now())
		, socket_(socket)
		, player_(std::forward<Args>(args)...)
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

	std::chrono::steady_clock::duration time_logged_on() const
	{
		return std::chrono::steady_clock::now() - login_time_;
	}

private:
	std::chrono::steady_clock::time_point login_time_;
	const any_socket_ptr socket_;
	mutable player player_;
	std::shared_ptr<game> game_;
	std::string ip_address;
};

struct socket_t {};
struct name_t {};
struct game_t {};

namespace bmi = boost::multi_index;

// Key extraction declarations for use by key-based indices to obtain the indexing keys from the elements of the multi_index_container.
// const_mem_fun is a Key Extractor returning as key the result of invoking a given constant member function of a class.
using extract_by_socket = bmi::const_mem_fun<player_record, const any_socket_ptr, &player_record::socket>;
using extract_by_name   = bmi::const_mem_fun<player_record, const std::string&,   &player_record::name>;
using extract_by_game   = bmi::const_mem_fun<player_record, int,                  &player_record::game_id>;

// Index declarations with assigned tags for easier access.
using index_by_socket = bmi::ordered_unique<bmi::tag<socket_t>, extract_by_socket>;
using index_by_name   = bmi::hashed_unique<bmi::tag<name_t>, extract_by_name>;
using index_by_game   = bmi::ordered_non_unique<bmi::tag<game_t>, extract_by_game>;

using player_connections = boost::multi_index_container<player_record, bmi::indexed_by<index_by_socket, index_by_name, index_by_game>>;

typedef player_connections::index<socket_t>::type player_by_socket;
typedef player_connections::index<name_t>  ::type player_by_name;
typedef player_connections::index<game_t>  ::type player_by_game;

typedef player_connections::const_iterator player_iterator;

} // namespace wesnothd
