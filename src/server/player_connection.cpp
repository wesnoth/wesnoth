/*
   Copyright (C) 2016 by Sergey Popov <loonycyborg@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "player_connection.hpp"
#include "game.hpp"

namespace wesnothd
{

const boost::shared_ptr<game> PlayerRecord::get_game() const
{
	return game_;
}

boost::shared_ptr<game>& PlayerRecord::get_game()
{
	return game_;
}

int PlayerRecord::game_id() const
{
	return game_ ? game_->id() : 0;
}

void PlayerRecord::set_game(PlayerRecord& record, boost::shared_ptr<game> new_game)
{
	record.game_ = new_game;
}

void PlayerRecord::enter_lobby(PlayerRecord& record)
{
	record.game_.reset();
}

}
