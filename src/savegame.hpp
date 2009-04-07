/* $Id$ */
/*
   Copyright (C) 2003 - 2009 by Jörg Hinrichs, refactored from various
   places formerly created by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef SAVEGAME_H_INCLUDED
#define SAVEGAME_H_INCLUDED

#include "global.hpp"
#include "show_dialog.hpp"
#include "gamestatus.hpp"
#include "play_controller.hpp"

#include <string>

#include "SDL_mutex.h"

class save_blocker {
public:
	save_blocker();
	~save_blocker();
	static bool saves_are_blocked();
	static void on_unblock(play_controller* controller, void (play_controller::*callback)());

protected:
	friend class play_controller;
	static void block();
	static bool try_block();
	static void unblock();

	class save_unblocker {
	public:
		save_unblocker() {}
		~save_unblocker() { save_blocker::unblock(); }
	};

private:
	static play_controller *controller_;
	static void (play_controller::*callback_)();
	static SDL_sem* sem_;
};

class savegame
{
public:
	savegame(game_state& gamestate, const std::string title = "Save");

	void save_game(const std::string& filename);
	void save_game(display* gui = NULL);
	void save_game_interactive(display& gui, const std::string& message, 
		gui::DIALOG_TYPE dialog_type, const bool has_exit_button = false,
		const bool ask_for_filename = true);

	const std::string filename() const { return filename_; }

protected:
	void set_filename(std::string filename);
	void set_error_message(std::string error_message) { error_message_ = error_message; }
	game_state& gamestate() { return gamestate_; }
	config& snapshot() { return snapshot_; }

	virtual void before_save();

private:
	virtual void create_filename() {}
	virtual void write_game_snapshot(const display& /*gui*/) {}

	void save_game_internal(const std::string& filename);

	game_state& gamestate_;
	config snapshot_;
	std::string filename_;
	const std::string title_;
	std::string error_message_;
	bool interactive_;
};

class game_savegame : public savegame
{
public:
	game_savegame(game_state& gamestate, const config& level_cfg,
		const game_display& gui, const std::vector<team>& teams,
		const unit_map& units, const gamestatus& gamestatus,
		const gamemap& map);

private:
	virtual void create_filename();
	virtual void before_save();
	void write_game_snapshot();

protected:
	const config& level_cfg_;
	const game_display& gui_;
	const std::vector<team>& teams_;
	const unit_map& units_;
	const gamestatus& gamestatus_;
	const gamemap& map_;
};

class replay_savegame : public savegame
{
public:
	replay_savegame(game_state& gamestate);

private:
	virtual void create_filename();
};

class autosave_savegame : public game_savegame
{
public:
	autosave_savegame(game_state &gamestate, const config& level_cfg, 
							 const game_display& gui, const std::vector<team>& teams, 
							 const unit_map& units, const gamestatus& gamestatus,
							 const gamemap& map);

private:
	virtual void create_filename();
};

class scenariostart_savegame : public savegame
{
public:
	scenariostart_savegame(game_state& gamestate);

private:
	virtual void before_save();
};

///** Autosave */
//std::string save_autosave(unsigned turn, const config& snapshot, game_state& gamestate);
//
///** Replay save created by player interaction */
//void save_replay(std::string filename, game_state& gamestate);
//
///** Replay save, created automatically at the end of a scenario */
//void save_replay(game_state& gamestate);
//
//std::string create_filename(const std::string& filename_base, const unsigned turn);
//std::string create_replay_filename(const std::string& filename_base);

#endif
