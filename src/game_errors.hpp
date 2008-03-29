/* $Id$ */
/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2008 by Yann Dirson <ydirson@altern.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef GAME_ERRORS_HPP_INCLUDED
#define GAME_ERRORS_HPP_INCLUDED

#include <string>

namespace game {
struct error {
	error() : 
		message() 
		{}
	error(const std::string& msg) : message(msg)
	{}

	std::string message;
};

//an exception object used when loading a game fails.
struct load_game_failed : public error {
	load_game_failed() {}
	load_game_failed(const std::string& msg) : error("load_game_failed: " + msg) {}
};

//an exception object used when saving a game fails.
struct save_game_failed : public error {
	save_game_failed() {}
	save_game_failed(const std::string& msg) : error("save_game_failed: " + msg) {}
};

//an exception object used for any general game error.
//e.g. data files are corrupt.
struct game_error : public error {
	game_error(const std::string& msg) : error("game_error: " + msg) {}
};

//an exception object used to signal that the user has decided to abort
//a game, and load another game instead
struct load_game_exception {
	load_game_exception(const std::string& game, bool show_replay, bool cancel_orders) 
	: game(game), show_replay(show_replay), cancel_orders(cancel_orders) {}
	std::string game;
	bool show_replay;
	bool cancel_orders;
};
}

#endif
