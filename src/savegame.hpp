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

#include <string>

struct load_game_cancelled_exception
{
};

class loadgame
{
public:
	loadgame(display& gui, const config& game_config, game_state& gamestate);
	virtual ~loadgame() {}

	void load_game();
	void load_game(std::string& filename, bool show_replay, bool cancel_orders);
	void set_gamestate();

	bool show_replay() const { return show_replay_; }
	bool cancel_orders() const { return cancel_orders_; }

private:
	void show_dialog(bool show_replay, bool cancel_orders);

	const config& game_config_;
	display& gui_;

	game_state& gamestate_;
	std::string filename_;
	config load_config_;
	bool show_replay_;
	bool cancel_orders_;
};

/** The base class for all savegame stuff */
class savegame
{
public:
	/** The only constructor of savegame. The title parameter is only necessary if you 
		intend to do interactive saves. */
	savegame(game_state& gamestate, const std::string title = "Save");

	virtual ~savegame() {}

	/** Save a game without any further user interaction. Atm, this is only used by the
		console_handler save actions */
	void save_game(const std::string& filename);
	
	/** Save a game without any further user interaction. This is used by autosaves and
		automatically generated replay saves. If you want notifying messages or error messages
		to appear, you have to provide the gui parameter. */
	void save_game(display* gui = NULL);
	
	/** Save a game interactively through the savegame dialog. Used for manual midgame and replay
		saves. */
	void save_game_interactive(display& gui, const std::string& message, 
		gui::DIALOG_TYPE dialog_type, const bool has_exit_button = false,
		const bool ask_for_filename = true);

	const std::string filename() const { return filename_; }

protected:
	/** Sets the filename and removes invalid characters. Don't set the filename directly but
		use this method instead. */
	void set_filename(std::string filename);
	
	/** Customize the standard error message */
	void set_error_message(const std::string error_message) { error_message_ = error_message; }
	
	game_state& gamestate() const { return gamestate_; }
	config& snapshot() { return snapshot_; }

	/** If there needs to be some data fiddling before saving the game, this is the place to go. */
	virtual void before_save();

private:
	/** Build the filename according to the specific savegame's needs. Subclasses will have to
		override this to take effect. */
	virtual void create_filename() {}

	/** The actual method for saving the game to disk. All interactive filename choosing and 
		data manipulation has to happen before calling this method */
	void save_game_internal(const std::string& filename);

	void write_game(config_writer &out) const;
	void finish_save_game(const config_writer &out);
	void extract_summary_data_from_save(config& out);

	game_state& gamestate_;
	
	/** Gamestate information at the time of saving. Note that this object is needed here, since
		even if it is empty the code relies on it to be there. */
	config snapshot_;
	
	std::string filename_; /** Filename of the savegame file on disk */
	
	const std::string title_; /** Title of the savegame dialog */
	
	std::string error_message_; /** Error message to be displayed if the savefile could not be generated. */
	
	/** Determines if the save is done interactively or not. This controls if a filename is
		generated automatically (interactive = false) and if a message is displayed that the
		game was successfully saved (interactive = true). */
	bool interactive_;
};

/** Class for "normal" midgame saves. The additional members are needed for creating the snapshot
	information. */
class game_savegame : public savegame
{
public:
	game_savegame(game_state& gamestate, const config& level_cfg,
		const game_display& gui, const std::vector<team>& teams,
		const unit_map& units, const gamestatus& gamestatus,
		const gamemap& map);

private:
	virtual void create_filename();
	
	/** Builds the snapshot config. */
	virtual void before_save();
	
	/** For normal game saves. Builds a snapshot config object out of the relevant information. */
	void write_game_snapshot();

protected:
	const config& level_cfg_;
	const game_display& gui_;
	const std::vector<team>& teams_;
	const unit_map& units_;
	const gamestatus& gamestatus_;
	const gamemap& map_;
};

/** Class for replay saves (either manually or automatically). */
class replay_savegame : public savegame
{
public:
	replay_savegame(game_state& gamestate);

private:
	virtual void create_filename();
};

/** Class for autosaves. */
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

/** Class for start-of-scenario saves */
class scenariostart_savegame : public savegame
{
public:
	scenariostart_savegame(game_state& gamestate);

private:
	/** Adds the player information to the starting position (= [replay_start]). */
	virtual void before_save();
};

#endif
