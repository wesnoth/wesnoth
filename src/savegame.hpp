/*
   Copyright (C) 2003 - 2014 by Jörg Hinrichs, refactored from various
   places formerly created by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef SAVEGAME_H_INCLUDED
#define SAVEGAME_H_INCLUDED

#include "filesystem.hpp"
#include "gamestatus.hpp"
#include "saved_game.hpp"
#include "show_dialog.hpp"
#include "serialization/compression.hpp"
class config_writer;
class game_display;

struct load_game_cancelled_exception {};
struct illegal_filename_exception {};

namespace savegame {

/** Returns true if there is already a savegame with that name. */
bool save_game_exists(const std::string& name, compression::format compressed);

/** Delete all autosaves of a certain scenario. */
void clean_saves(const std::string& label);

/** The class for loading a savefile. */
class loadgame
{
public:
	loadgame(display& gui, const config& game_config, saved_game& gamestate);
	virtual ~loadgame() {}

	/** Load a game without providing any information. */
	void load_game();
	/** Load a game with pre-setting information for the load-game dialog. */
	void load_game(
			  const std::string& filename
			, const bool show_replay
			, const bool cancel_orders
			, const bool select_difficulty
			, const std::string& difficulty);
	/** Loading a game from within the multiplayer-create dialog. */
	void load_multiplayer_game();
	/** Populates the level-config for multiplayer from the loaded savegame information. */
	void fill_mplevel_config(config& level);
	/** Generate the gamestate out of the loaded game config. */
	void set_gamestate();

	// Getter-methods
	bool show_replay() const { return show_replay_; }
	bool cancel_orders() const { return cancel_orders_; }
	const std::string & filename() const { return filename_; }
private:
	/** Display the load-game dialog. */
	void show_dialog(bool show_replay, bool cancel_orders);
	/** Display the difficulty dialog. */
	void show_difficulty_dialog();
	/** Check if the version of the savefile is compatible with the current version. */
	void check_version_compatibility();
	/** Copy era information into the snapshot. */
	void copy_era(config& cfg);

	const config& game_config_;
	display& gui_;

	saved_game& gamestate_; /** Primary output information. */
	std::string filename_; /** Name of the savefile to be loaded. */
	std::string difficulty_; /** The difficulty the save is meant to be loaded with. */
	config load_config_; /** Config information of the savefile to be loaded. */
	bool show_replay_; /** State of the "show_replay" checkbox in the load-game dialog. */
	bool cancel_orders_; /** State of the "cancel_orders" checkbox in the load-game dialog. */
	bool select_difficulty_; /** State of the "change_difficulty" checkbox in the load-game dialog. */
};

/**
 * The base class for all savegame stuff.
 * This should not be used directly, as it does not directly produce usable saves.
 * Instead, use one of the derived classes.
 */
class savegame
{
protected:
	/** The only constructor of savegame. The title parameter is only necessary if you
		intend to do interactive saves. */
	savegame(saved_game& gamestate, const compression::format compress_saves, const std::string& title = "Save");

public:
	virtual ~savegame() {}

	/** Saves a game without user interaction, unless the file exists and it should be asked
		to overwrite it. The return value denotes, if the save was successful or not.
		This is used by automatically generated replays, start-of-scenario saves, autosaves,
		and saves from the console (e.g. ":w").
	*/
	bool save_game_automatic(CVideo& video, bool ask_for_overwrite = false, const std::string& filename = "");

	/** Save a game interactively through the savegame dialog. Used for manual midgame and replay
		saves. The return value denotes, if the save was successful or not. */
	bool save_game_interactive(CVideo& video, const std::string& message,
		gui::DIALOG_TYPE dialog_type);

	const std::string& filename() const { return filename_; }

protected:
	/**
		Save a game without any further user interaction. If you want notifying messages
		or error messages to appear, you have to provide the gui parameter.
		The return value denotes, if the save was successful or not.
	*/
	bool save_game(CVideo* video = NULL, const std::string& filename = "");

	/** Sets the filename and removes invalid characters. Don't set the filename directly but
		use this method instead. */
	void set_filename(std::string filename);
	/** Check, if the filename contains illegal constructs like ".gz". */
	void check_filename(const std::string& filename, CVideo& video);

	/** Customize the standard error message */
	void set_error_message(const std::string& error_message) { error_message_ = error_message; }

	const std::string& title() { return title_; }
	const saved_game& gamestate() { return gamestate_; }
	config& snapshot() { return snapshot_; }

	/** If there needs to be some data fiddling before saving the game, this is the place to go. */
	void before_save();

	/** Writing the savegame config to a file. */
	virtual void write_game(config_writer &out);

private:
	/** Checks if a certain character is allowed in a savefile name. */
	static bool is_illegal_file_char(char c);

	/** Build the filename according to the specific savegame's needs. Subclasses will have to
		override this to take effect. */
	virtual void create_filename() {}
	/** Display the save game dialog. */
	virtual int show_save_dialog(CVideo& video, const std::string& message, const gui::DIALOG_TYPE dialog_type);
	/** Ask the user if an existing file should be overwritten. */
	bool check_overwrite(CVideo& video);

	/** The actual method for saving the game to disk. All interactive filename choosing and
		data manipulation has to happen before calling this method. */
	void write_game_to_disk(const std::string& filename);

	/** Update the save_index */
	void finish_save_game(const config_writer &out);
	/** Throws game::save_game_failed. */
	scoped_ostream open_save_game(const std::string &label);
	friend class save_info;
	//before_save (write replay data) changes this so it cannot be const
	saved_game& gamestate_;

	/** Gamestate information at the time of saving. Note that this object is needed here, since
		even if it is empty the code relies on it to be there. */
	config snapshot_;

	std::string filename_; /** Filename of the savegame file on disk */

	const std::string title_; /** Title of the savegame dialog */

	std::string error_message_; /** Error message to be displayed if the savefile could not be generated. */

	bool show_confirmation_; /** Determines if a confirmation of successful saving the game is shown. */

	compression::format compress_saves_; /** Determines, what compression format is used for the savegame file */
};

/** Class for "normal" midgame saves. The additional members are needed for creating the snapshot
	information. */
class ingame_savegame : public savegame
{
public:
	ingame_savegame(saved_game& gamestate,
		game_display& gui, const config& snapshot_cfg, const compression::format compress_saves);

private:
	/** Create a filename for automatic saves */
	virtual void create_filename();


	void write_game(config_writer &out);

protected:
	game_display& gui_;
};

/** Class for replay saves (either manually or automatically). */
class replay_savegame : public savegame
{
public:
	replay_savegame(saved_game& gamestate, const compression::format compress_saves);

private:
	/** Create a filename for automatic saves */
	virtual void create_filename();

	void write_game(config_writer &out);
};

/** Class for autosaves. */
class autosave_savegame : public ingame_savegame
{
public:
	autosave_savegame(saved_game &gamestate,
					 game_display& gui, const config& snapshot_cfg, const compression::format compress_saves);

	void autosave(const bool disable_autosave, const int autosave_max, const int infinite_autosaves);
private:
	/** Create a filename for automatic saves */
	virtual void create_filename();
};

class oos_savegame : public ingame_savegame
{
public:
	oos_savegame(saved_game& gamestate, game_display& gui, const config& snapshot_cfg);

private:
	/** Display the save game dialog. */
	virtual int show_save_dialog(CVideo& video, const std::string& message, const gui::DIALOG_TYPE dialog_type);
};

/** Class for start-of-scenario saves */
class scenariostart_savegame : public savegame
{
public:
	scenariostart_savegame(saved_game& gamestate, const compression::format compress_saves);

private:
	void write_game(config_writer &out);
};

} //end of namespace savegame

#endif
