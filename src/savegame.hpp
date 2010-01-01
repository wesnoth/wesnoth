/* $Id$ */
/*
   Copyright (C) 2003 - 2010 by Jörg Hinrichs, refactored from various
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
#include "filesystem.hpp"
#include "gamestatus.hpp"
#include "tod_manager.hpp"
#include "show_dialog.hpp"

#include <string>

class config_writer;
class game_display;

struct load_game_cancelled_exception {};
struct illegal_filename_exception {};

namespace savegame {

/** Filename and modification date for a file list */
struct save_info {
	save_info(const std::string& n, time_t t) : name(n), time_modified(t) {}
	std::string name;
	time_t time_modified;

	const std::string format_time_summary() const;
	const std::string format_time_local() const;
};

/**
Container for a couple of savefile manipulating methods.
Note: You are not supposed to instantiate this class.
*/
class manager
{
public:
	/** Read summary information out of an existing savefile. */
	static void load_summary(const std::string& name, config& cfg_summary, std::string* error_log);
	/** Read the complete config information out of a savefile. */
	static void read_save_file(const std::string& name, config& cfg, std::string* error_log);

	/** Returns true if there is already a savegame with that name. */
	static bool save_game_exists(const std::string& name, const bool compress_saves);
	/** Get a list of available saves. */
	static std::vector<save_info> get_saves_list(const std::string *dir = NULL, const std::string* filter = NULL);

	/** Delete all autosaves of a certain scenario. */
	static void clean_saves(const std::string &label);
	/** Remove autosaves that are no longer needed (according to the autosave policy in the preferences). */
	static void remove_old_auto_saves(const int autosavemax, const int infinite_auto_saves);
	/** Delete a savegame. */
	static void delete_game(const std::string& name);

private:
	/** Default-Constructor (don't instantiate this class) */
	manager() {}
};

/**
Container for methods dealing with the save_index file.
Note: You are not supposed to instantiate this class.
*/
class save_index
{
public:
	/** Determines/Adds the "save"-child of the summary config. */
	static config& save_summary(std::string save);
	/** Update the save_index file with changed savegame information. */
	static void write_save_index();

private:
	/** Default-Constructor (don't instantiate this class) */
	save_index() {}

	/** Read the complete config information out of the save_index file. */
	static config& load();

	static bool save_index_loaded; /** Tells, if the save_index config has been already loaded. */
	static config save_index_cfg; /** save_index config (who would have guessed that ;-). */
};

/** The class for loading a savefile. */
class loadgame
{
public:
	loadgame(display& gui, const config& game_config, game_state& gamestate);
	virtual ~loadgame() {}

	/** Load a game without providing any information. */
	void load_game();
	/** Load a game with pre-setting information for the load-game dialog. */
	void load_game(std::string& filename, bool show_replay, bool cancel_orders);
	/** Loading a game from within the multiplayer-create dialog. */
	void load_multiplayer_game();
	/** Populates the level-config for multiplayer from the loaded savegame information. */
	void fill_mplevel_config(config& level);
	/** Generate the gamestate out of the loaded game config. */
	void set_gamestate();

	// Getter-methods
	bool show_replay() const { return show_replay_; }
	bool cancel_orders() const { return cancel_orders_; }
	const std::string filename() const { return filename_; }

private:
	/** Display the load-game dialog. */
	void show_dialog(bool show_replay, bool cancel_orders);
	/** Check if the version of the savefile is compatible with the current version. */
	void check_version_compatibility();
	/** Copy era information into the snapshot. */
	void copy_era(config& cfg);

	const config& game_config_;
	display& gui_;

	game_state& gamestate_; /** Primary output information. */
	std::string filename_; /** Name of the savefile to be loaded. */
	config load_config_; /** Config information of the savefile to be loaded. */
	bool show_replay_; /** State of the "show_replay" checkbox in the load-game dialog. */
	bool cancel_orders_; /** State of the "cancel_orders" checkbox in the load-game dialog. */
};

/** The base class for all savegame stuff */
class savegame
{
public:
	/** The only constructor of savegame. The title parameter is only necessary if you
		intend to do interactive saves. */
	savegame(game_state& gamestate, const bool compress_saves, const std::string& title = "Save");

	virtual ~savegame() {}

	/** Save a game without any further user interaction. Atm, this is only used by the
		console_handler save actions. The return value denotes, if the save was successful or not. */
	//bool save_game(const std::string& filename);

	/** Saves a game without user interaction, unless the file exists and it should be asked
		to overwrite it. The return value denotes, if the save was successful or not.
		This is used by automatically generated replays, start-of-scenario saves and autosaves.
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

	const std::string& title() const { return title_; }
	game_state& gamestate() const { return gamestate_; }
	config& snapshot() { return snapshot_; }

	/** If there needs to be some data fiddling before saving the game, this is the place to go. */
	virtual void before_save();

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

	/** Writing the savegame config to a file. */
	void write_game(config_writer &out) const;
	/** Update the save_index */
	void finish_save_game(const config_writer &out);
	/** Throws game::save_game_failed. */
	scoped_ostream open_save_game(const std::string &label);
	/** Read the summary data from a savegame to display this information in the load-game dialog. */
	void extract_summary_data_from_save(config& out);

	game_state& gamestate_;

	/** Gamestate information at the time of saving. Note that this object is needed here, since
		even if it is empty the code relies on it to be there. */
	config snapshot_;

	std::string filename_; /** Filename of the savegame file on disk */

	const std::string title_; /** Title of the savegame dialog */

	std::string error_message_; /** Error message to be displayed if the savefile could not be generated. */

	bool show_confirmation_; /** Determines if a confirmation of successful saving the game is shown. */

	bool compress_saves_; /** Determines, if compression is used for the savegame file */
};

/** Class for "normal" midgame saves. The additional members are needed for creating the snapshot
	information. */
class game_savegame : public savegame
{
public:
	game_savegame(game_state& gamestate,
		game_display& gui, const config& snapshot_cfg, const bool compress_saves);

private:
	/** Create a filename for automatic saves */
	virtual void create_filename();

	/** Builds the snapshot config. */
	virtual void before_save();

	/** For normal game saves. Builds a snapshot config object out of the relevant information. */
	void write_game_snapshot();

protected:
	game_display& gui_;
};

/** Class for replay saves (either manually or automatically). */
class replay_savegame : public savegame
{
public:
	replay_savegame(game_state& gamestate, const bool compress_saves);

private:
	/** Create a filename for automatic saves */
	virtual void create_filename();
};

/** Class for autosaves. */
class autosave_savegame : public game_savegame
{
public:
	autosave_savegame(game_state &gamestate,
					 game_display& gui, const config& snapshot_cfg, const bool compress_saves);

	void autosave(const bool disable_autosave, const int autosave_max, const int infinite_autosaves);
private:
	/** Create a filename for automatic saves */
	virtual void create_filename();
};

class oos_savegame : public game_savegame
{
public:
	oos_savegame(const config& snapshot_cfg);

private:
	/** Display the save game dialog. */
	virtual int show_save_dialog(CVideo& video, const std::string& message, const gui::DIALOG_TYPE dialog_type);
};

/** Class for start-of-scenario saves */
class scenariostart_savegame : public savegame
{
public:
	scenariostart_savegame(game_state& gamestate, const bool compress_saves);

private:
	/** Adds the player information to the starting position (= [replay_start]). */
	virtual void before_save();
};

} //end of namespace savegame

void replace_underbar2space(std::string &name);
void replace_space2underbar(std::string &name);

#endif
