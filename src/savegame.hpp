/*
   Copyright (C) 2003 - 2018 by Jörg Hinrichs, refactored from various
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

#pragma once

#include "config.hpp"
#include "filesystem.hpp"
#include "lua_jailbreak_exception.hpp"
#include "saved_game.hpp"
#include "serialization/compression.hpp"

#include <exception>

class config_writer;
class version_info;

namespace savegame {
/** converts saves from older versions of wesnoth*/
void convert_old_saves(config& cfg);
/** Returns true if there is already a savegame with that name. */
bool save_game_exists(const std::string& name, compression::format compressed);

/** Delete all autosaves of a certain scenario. */
void clean_saves(const std::string& label);

struct load_game_metadata {
	/** Name of the savefile to be loaded. */
	std::string filename;

	/** The difficulty the save is meant to be loaded with. */
	std::string difficulty;

	/** State of the "show_replay" checkbox in the load-game dialog. */
	bool show_replay;

	/** State of the "cancel_orders" checkbox in the load-game dialog. */
	bool cancel_orders;

	/** State of the "change_difficulty" checkbox in the load-game dialog. */
	bool select_difficulty;

	/** Summary config of the save selected in the load game dialog. */
	config summary;

	/** Config information of the savefile to be loaded. */
	config load_config;

	explicit load_game_metadata(const std::string& fname = "", const std::string& hard = "",
			bool replay = false, bool stop = false, bool change = false,
			const config& summary = config(), const config& info = config())
		: filename(fname), difficulty(hard)
		, show_replay(replay), cancel_orders(stop), select_difficulty(change)
		, summary(summary), load_config(info)
	{
	}
};

/**
* Exception used to signal that the user has decided to abortt a game,
* and to load another game instead.
*/
class load_game_exception
	: public lua_jailbreak_exception, public std::exception
{
public:
	load_game_exception(const std::string& fname)
		: lua_jailbreak_exception()
		, data_(fname)
	{
	}

	load_game_exception(load_game_metadata&& data)
		: lua_jailbreak_exception()
		, data_(data)
	{
	}
	load_game_metadata data_;
private:

	IMPLEMENT_LUA_JAILBREAK_EXCEPTION(load_game_exception)
};

/** The class for loading a savefile. */
class loadgame
{
public:
	loadgame(const config& game_config, saved_game& gamestate);
	virtual ~loadgame() {}

	/* In any of the following three function, a bool value of false indicates
	   some failure or abort of the load process */

	/** Load a game without providing any information. */
	bool load_game_ingame();
	/** Load a game with pre-setting information for the load-game dialog. */
	bool load_game();
	/** Loading a game from within the multiplayer-create dialog. */
	bool load_multiplayer_game();
	/** Generate the gamestate out of the loaded game config. */
	void set_gamestate();

	// Getter-methods
	load_game_metadata& data()
	{
		return load_data_;
	}

	/** GUI Dialog sequence which confirms attempts to load saves from previous game versions. */
	static bool check_version_compatibility(const version_info & version);

	static bool is_replay_save(const config& cfg)
	{
		return cfg["replay"].to_bool() && !cfg["snapshot"].to_bool(true);
	}

private:
	/** Display the difficulty dialog. */
	bool show_difficulty_dialog();
	/** Call check_version_compatibility above, using the version of this savefile. */
	bool check_version_compatibility();
	/** Copy era information into the snapshot. */
	void copy_era(config& cfg);

	const config& game_config_;

	saved_game& gamestate_; /** Primary output information. */

	load_game_metadata load_data_;
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
	enum DIALOG_TYPE { YES_NO, OK_CANCEL};

	virtual ~savegame() {}

	/** Saves a game without user interaction, unless the file exists and it should be asked
		to overwrite it. The return value denotes, if the save was successful or not.
		This is used by automatically generated replays, start-of-scenario saves, autosaves,
		and saves from the console (e.g. ":w").
	*/
	bool save_game_automatic(bool ask_for_overwrite = false, const std::string& filename = "");

	/** Save a game interactively through the savegame dialog. Used for manual midgame and replay
		saves. The return value denotes, if the save was successful or not. */
	bool save_game_interactive(const std::string& message,
		DIALOG_TYPE dialog_type);

	const std::string& filename() const { return filename_; }

	/** Build the filename according to the specific savegame's needs. */
	std::string create_filename() const
	{
		return create_filename(gamestate().get_starting_pos()["turn_at"]);
	}

	/** Build the filename for the specified turn. */
	std::string create_filename(unsigned int turn_number) const;

protected:
	/**
		Save a game without any further user interaction.
		The return value denotes, if the save was successful or not.
	*/
	bool save_game(const std::string& filename = "");

	/** Check, if the filename contains illegal constructs like ".gz". */
	bool check_filename(const std::string& filename);

	/** Customize the standard error message */
	void set_error_message(const std::string& error_message) { error_message_ = error_message; }

	const std::string& title() const { return title_; }
	const saved_game& gamestate() const { return gamestate_; }

	/** If there needs to be some data fiddling before saving the game, this is the place to go. */
	void before_save();

	/** Writing the savegame config to a file. */
	virtual void write_game(config_writer &out);

	/** Filename of the savegame file on disk */
	std::string filename_;

private:
	/** Checks if a certain character is allowed in a savefile name. */
	static bool is_illegal_file_char(char c);

	/** Subclass-specific part of filename building. */
	virtual std::string create_initial_filename(unsigned int turn_number) const = 0;
	/** Display the save game dialog. */
	virtual int show_save_dialog(const std::string& message, DIALOG_TYPE dialog_type);
	/** Ask the user if an existing file should be overwritten. */
	bool check_overwrite();

	/** The actual method for saving the game to disk. All interactive filename choosing and
		data manipulation has to happen before calling this method. */
	void write_game_to_disk(const std::string& filename);

	/** Update the save_index */
	void finish_save_game(const config_writer &out);
	/** Throws game::save_game_failed. */
	filesystem::scoped_ostream open_save_game(const std::string &label);
	friend class save_info;
	//before_save (write replay data) changes this so it cannot be const
	saved_game& gamestate_;

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
	ingame_savegame(saved_game& gamestate, const compression::format compress_saves);

private:
	/** Create a filename for automatic saves */
	virtual std::string create_initial_filename(unsigned int turn_number) const override;


	void write_game(config_writer &out) override;
};

/** Class for replay saves (either manually or automatically). */
class replay_savegame : public savegame
{
public:
	replay_savegame(saved_game& gamestate, const compression::format compress_saves);

private:
	/** Create a filename for automatic saves */
	virtual std::string create_initial_filename(unsigned int turn_number) const override;

	void write_game(config_writer &out) override;
};

/** Class for autosaves. */
class autosave_savegame : public ingame_savegame
{
public:
	autosave_savegame(saved_game &gamestate, const compression::format compress_saves);

	void autosave(const bool disable_autosave, const int autosave_max, const int infinite_autosaves);
private:
	/** Create a filename for automatic saves */
	virtual std::string create_initial_filename(unsigned int turn_number) const override;
};

class oos_savegame : public ingame_savegame
{
public:
	oos_savegame(saved_game& gamestate, bool& ignore);

private:
	/** Display the save game dialog. */
	virtual int show_save_dialog(const std::string& message, DIALOG_TYPE dialog_type) override;
	bool& ignore_;
};

/** Class for start-of-scenario saves */
class scenariostart_savegame : public savegame
{
public:
	scenariostart_savegame(saved_game& gamestate, const compression::format compress_saves);

private:
	/** Create a filename for automatic saves */
	virtual std::string create_initial_filename(unsigned int turn_number) const override;

	void write_game(config_writer &out) override;
};

} //end of namespace savegame
