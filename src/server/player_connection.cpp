/*
   Copyright (C) 2016 - 2018 by Sergey Popov <loonycyborg@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "server/player_connection.hpp"
#include "server/game.hpp"

namespace wesnothd
{

const std::shared_ptr<game> player_record::get_game() const
{
	return game_;
}

std::shared_ptr<game>& player_record::get_game()
{
	return game_;
}

int player_record::game_id() const
{
	return game_ ? game_->id() : 0;
}

void player_record::set_game(player_record& record, std::shared_ptr<game> new_game)
{
	record.game_ = new_game;
}

void player_record::enter_lobby(player_record& record)
{
	record.game_.reset();
}

}
