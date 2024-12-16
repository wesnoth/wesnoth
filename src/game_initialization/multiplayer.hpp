/*
	Copyright (C) 2005 - 2024
	by Philippe Plantier <ayin@anathas.org>
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

#include <functional>
#include <string>

class commandline_options;
class config;

namespace ng { class connect_engine; }

/** Main entry points of multiplayer mode. */
namespace mp
{
/** Max length of a player name. */
const std::size_t max_login_size = 20;

/**
 * Starts a multiplayer game in client mode.
 *
 * @param host                The host to connect to.
 */
void start_client(const std::string& host);

/** Starts a multiplayer game in single-user mode. */
void start_local_game();

/**
 * Starts a multiplayer game in single-user mode using command line settings.
 *
 * @param cmdline_opts        The commandline options.
 */
void start_local_game_commandline(const commandline_options& cmdline_opts);

/**
 * Opens the MP Staging screen and sets the game state according to the changes made.
 * Meant to be used between scenarios in a campaign.
 *
 * @param engine              A connect_engine instance to pass to MP Staging.
 */
bool goto_mp_staging(ng::connect_engine& engine);

/**
 * Opens the MP Join Game screen and sets the game state according to the changes made.
 * Meant to be used between scenarios in a campaign.
 *
 * @param observe             Whether entering as an observer or player.
 */
bool goto_mp_wait(bool observe);

/** Gets whether the currently logged-in user is a moderator. */
bool logged_in_as_moderator();

/** Gets the forum profile link for the given user. */
std::string get_profile_link(int user_id);

/** Returns the lobby_info object for the given session. */
class lobby_info* get_lobby_info();

/** Attempts to send given data to server if a connection is open. */
void send_to_server(const config& data);

/** RAII helper class to register a network handler. */
class network_registrar
{
public:
	using handler = std::function<void(const config&)>;

	network_registrar(const handler& func);
	~network_registrar();

private:
	std::function<void()> remove_handler{};
};

} // namespace mp
