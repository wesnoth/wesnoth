/*
   Copyright (C) 2003 - 2015 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef PLAYER_HPP_INCLUDED
#define PLAYER_HPP_INCLUDED


#include "simple_wml.hpp"

#include <ctime>
#include <set>

namespace wesnothd {

class game;

class player
{
public:
	enum STATUS {
		LOBBY,
		PLAYING,
		OBSERVING
	};

	player(const std::string& n, simple_wml::node& cfg, bool registered,
	       const size_t max_messages=4, const size_t time_period=10,
	       const bool sp=false, const bool moderator=false);

	void set_status(STATUS status);

	// mark a player as member of the game 'game_id' or as located in the lobby
	void mark_available(const int game_id=0, const std::string& location="");

	//Mark a player as registered if he has authorized
	void mark_registered(bool registered =true);
	bool registered() const {return registered_;}


	const std::string& name() const { return name_; }
	bool selective_ping() const { return selective_ping_ ; }

	const simple_wml::node* config_address() const { return &cfg_; }

	bool is_message_flooding();

	/**
	 * @return true iff the player is in a game
	 */
	bool in_game() const { return get_game() != NULL; }

	/**
	 * @return a pointer to the game the player is in, or NULL if he/she is not
	 * in a game at the moment
	 */
	const game* get_game() const;

	void set_game(game* g);

	void set_moderator(bool moderator) { moderator_ = moderator; }
	bool is_moderator() const { return moderator_; }
	bool has_updated_level() const { return has_updated_level_; }
	void set_updated_level(bool temp) { has_updated_level_ = temp; }

private:
	const std::string name_;
	simple_wml::node& cfg_;
	const bool selective_ping_ ;

	bool registered_;

	time_t flood_start_;
	unsigned int messages_since_flood_start_;
	const size_t MaxMessages;
	const time_t TimePeriod;
	STATUS status_;
	bool moderator_;
	bool has_updated_level_; // this is keep track of which users have transitioned to the next level when the host updates.
};

} //namespace wesnothd

#endif
