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

#include <boost/iostreams/filter/gzip.hpp>

#include "savegame.hpp"

#include "save_index.hpp"
#include "carryover.hpp"
#include "cursor.hpp"
#include "format_time_summary.hpp"
#include "formatter.hpp"
#include "formula/string_utils.hpp"
#include "game_end_exceptions.hpp"
#include "game_errors.hpp"
#include "preferences/game.hpp"
#include "gettext.hpp"
#include "gui/dialogs/game_load.hpp"
#include "gui/dialogs/game_save.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/campaign_difficulty.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/retval.hpp"
#include "log.hpp"
#include "persist_manager.hpp"
#include "resources.hpp"
#include "save_index.hpp"
#include "saved_game.hpp"
#include "serialization/binary_or_text.hpp"
#include "serialization/parser.hpp"
#include "statistics.hpp"
#include "version.hpp"
#include "video.hpp"

#include <algorithm>

static lg::log_domain log_engine("engine");
#define LOG_SAVE LOG_STREAM(info, log_engine)
#define ERR_SAVE LOG_STREAM(err, log_engine)

static lg::log_domain log_enginerefac("enginerefac");
#define LOG_RG LOG_STREAM(info, log_enginerefac)


namespace savegame {

bool save_game_exists(std::string name, compression::format compressed)
{
	name += compression::format_extension(compressed);
	return filesystem::file_exists(filesystem::get_saves_dir() + "/" + name);
}

void clean_saves(const std::string& label)
{
	std::vector<save_info> games = get_saves_list();
	std::string prefix = label + "-" + _("Auto-Save");
	LOG_SAVE << "Cleaning saves with prefix '" << prefix << "'\n";
	for (std::vector<save_info>::iterator i = games.begin(); i != games.end(); ++i) {
		if (i->name().compare(0, prefix.length(), prefix) == 0) {
			LOG_SAVE << "Deleting savegame '" << i->name() << "'\n";
			delete_game(i->name());
		}
	}
}

loadgame::loadgame(const config& game_config, saved_game& gamestate)
	: game_config_(game_config)
	, gamestate_(gamestate)
	, load_data_()
{}

bool loadgame::show_difficulty_dialog()
{
	if(load_data_.summary["corrupt"].to_bool()) {
		return false;
	}

	std::string campaign_id = load_data_.summary["campaign"];

	for(const config &campaign : game_config_.child_range("campaign"))
	{
		if(campaign["id"] != campaign_id) {
			continue;
		}

		gui2::dialogs::campaign_difficulty difficulty_dlg(campaign);
		difficulty_dlg.show();

		// Return if canceled, since otherwise load_data_.difficulty will be set to 'CANCEL'
		if (difficulty_dlg.get_retval() != gui2::retval::OK) {
			return false;
		}

		load_data_.difficulty = difficulty_dlg.selected_difficulty();

		// Exit loop
		break;
	}

	return true;
}

// Called only by play_controller to handle in-game attempts to load. Instead of returning true,
// throws a "load_game_exception" to signal a resulting load game request.
bool loadgame::load_game_ingame()
{
	if(CVideo::get_singleton().faked()) {
		return false;
	}

	if(!gui2::dialogs::game_load::execute(game_config_, load_data_)) {
		return false;
	}

	if(load_data_.filename.empty()) {
		return false;
	}

	if(load_data_.select_difficulty) {
		if(!show_difficulty_dialog()) {
			return false;
		}
	}

	load_data_.show_replay |= is_replay_save(load_data_.summary);

	// Confirm the integrity of the file before throwing the exception.
	// Use the summary in the save_index for this.
	const config & summary = save_index_manager.get(load_data_.filename);

	if (summary["corrupt"].to_bool(false)) {
		gui2::show_error_message(
				_("The file you have tried to load is corrupt: '"));
		return false;
	}

	if (!loadgame::check_version_compatibility(summary["version"].str())) {
		return false;
	}

	throw load_game_exception(std::move(load_data_));
}

bool loadgame::load_game()
{
	bool skip_version_check = true;

	if(load_data_.filename.empty()){
		if(!gui2::dialogs::game_load::execute(game_config_, load_data_)) {
			return false;
		}
		skip_version_check = false;
		load_data_.show_replay |= is_replay_save(load_data_.summary);
	}

	if(load_data_.filename.empty()) {
		return false;
	}

	if(load_data_.select_difficulty) {
		if(!show_difficulty_dialog()) {
			return false;
		}
	}

	std::string error_log;
	read_save_file(load_data_.filename, load_data_.load_config, &error_log);

	convert_old_saves(load_data_.load_config);

	for (config& side : load_data_.load_config.child_range("side")) {
		side.remove_attribute("is_local");
	}

	if(!error_log.empty()) {
        try {
		    gui2::show_error_message(
				    _("Warning: The file you have tried to load is corrupt. Loading anyway.\n") +
				    error_log);
        } catch (utf8::invalid_utf8_exception&) {
		    gui2::show_error_message(
				    _("Warning: The file you have tried to load is corrupt. Loading anyway.\n") +
                    std::string("(UTF-8 ERROR)"));
        }
	}

	if (!load_data_.difficulty.empty()){
		load_data_.load_config["difficulty"] = load_data_.difficulty;
	}
	// read classification to for loading the game_config config object.
	gamestate_.classification() = game_classification(load_data_.load_config);

	if (skip_version_check) {
		return true;
	}

	return check_version_compatibility();
}

bool loadgame::check_version_compatibility()
{
	return loadgame::check_version_compatibility(gamestate_.classification().version);
}

bool loadgame::check_version_compatibility(const version_info & save_version)
{
	if (save_version == game_config::version) {
		return true;
	}

	const version_info &wesnoth_version = game_config::wesnoth_version;

	// Even minor version numbers indicate stable releases which are
	// compatible with each other.
	if (wesnoth_version.minor_version() % 2 == 0 &&
	    wesnoth_version.major_version() == save_version.major_version() &&
	    wesnoth_version.minor_version() == save_version.minor_version())
	{
		return true;
	}

	// Do not load if too old. If either the savegame or the current
	// game has the version 'test', load. This 'test' version is never
	// supposed to occur, except when Soliton is testing MP servers.
	if (save_version < game_config::min_savegame_version &&
	    save_version != game_config::test_version &&
	    wesnoth_version != game_config::test_version)
	{
		const std::string message = _("This save is from an old, unsupported version ($version_number|) and cannot be loaded.");
		utils::string_map symbols;
		symbols["version_number"] = save_version.str();
		gui2::show_error_message(utils::interpolate_variables_into_string(message, &symbols));
		return false;
	}

	if(preferences::confirm_load_save_from_different_version()) {
		const std::string message = _("This save is from a different version of the game ($version_number|). Do you wish to try to load it?");
		utils::string_map symbols;
		symbols["version_number"] = save_version.str();
		const int res = gui2::show_message(_("Load Game"), utils::interpolate_variables_into_string(message, &symbols),
			gui2::dialogs::message::yes_no_buttons);
		return res == gui2::retval::OK;
	}

	return true;
}

void loadgame::set_gamestate()
{
	gamestate_.set_data(load_data_.load_config);
}

bool loadgame::load_multiplayer_game()
{
	if(!gui2::dialogs::game_load::execute(game_config_, load_data_)) {
		return false;
	}


	load_data_.show_replay |= is_replay_save(load_data_.summary);
	if(load_data_.filename.empty()) {
		return false;
	}

	// read_save_file needs to be called before we can verify the classification so the data has
	// been populated. Since we do that, we report any errors in that process first.
	std::string error_log;
	{
		cursor::setter cur(cursor::WAIT);
		log_scope("load_game");

		read_save_file(load_data_.filename, load_data_.load_config, &error_log);
		copy_era(load_data_.load_config);
	}

	if(!error_log.empty()) {
		gui2::show_error_message(
				_("The file you have tried to load is corrupt: '") +
				error_log);
		return false;
	}

	if(is_replay_save(load_data_.summary)) {
		gui2::show_transient_message(_("Load Game"), _("Replays are not supported in multiplayer mode."));
		return false;
	}

	// We want to verify the game classification before setting the data, so we don't check on
	// gamestate_.classification() and instead construct a game_classification object manually.
	if(game_classification(load_data_.load_config).campaign_type != game_classification::CAMPAIGN_TYPE::MULTIPLAYER) {
		gui2::show_transient_error_message(_("This is not a multiplayer save."));
		return false;
	}

	set_gamestate();

	return check_version_compatibility();
}

void loadgame::copy_era(config &cfg)
{
	const config &replay_start = cfg.child("replay_start");
	if (!replay_start) return;

	const config &era = replay_start.child("era");
	if (!era) return;

	config &snapshot = cfg.child("snapshot");
	if (!snapshot) return;

	snapshot.add_child("era", era);
}

savegame::savegame(saved_game& gamestate, const compression::format compress_saves, const std::string& title)
	: filename_()
	, gamestate_(gamestate)
	, title_(title)
	, error_message_(_("The game could not be saved: "))
	, show_confirmation_(false)
	, compress_saves_(compress_saves)
{}

bool savegame::save_game_automatic(bool ask_for_overwrite, const std::string& filename)
{
	if (filename.empty())
		filename_ = create_filename();
	else
		filename_ = filename;

	if (ask_for_overwrite){
		if (!check_overwrite()) {
			return save_game_interactive("", savegame::OK_CANCEL);
		}
	}

	return save_game();
}

bool savegame::save_game_interactive(const std::string& message, DIALOG_TYPE dialog_type)
{
	show_confirmation_ = true;
	filename_ = create_filename();

	const int res = show_save_dialog(message, dialog_type);

	if (res == 2) {
		throw_quit_game_exception(); //Quit game
	}

	if (res == gui2::retval::OK && check_overwrite()) {
		return save_game();
	}

	return false;
}

int savegame::show_save_dialog(const std::string& message, DIALOG_TYPE dialog_type)
{
	int res = 0;

	if (dialog_type == OK_CANCEL){
		gui2::dialogs::game_save dlg(filename_, title_);
		dlg.show();
		res = dlg.get_retval();
	}
	else if (dialog_type == YES_NO){
		gui2::dialogs::game_save_message dlg(filename_, title_, message);
		dlg.show();
		res = dlg.get_retval();
	}

	if (!check_filename(filename_)) {
		res = gui2::retval::CANCEL;
	}

	return res;
}

bool savegame::check_overwrite()
{
	if(!save_game_exists(filename_, compress_saves_)) {
		return true;
	}

	std::ostringstream message;
	message << _("Save already exists. Do you want to overwrite it?") << "\n" << _("Name: ") << filename_;
	const int res = gui2::show_message(_("Overwrite?"), message.str(), gui2::dialogs::message::yes_no_buttons);
	return res == gui2::retval::OK;

}

bool savegame::check_filename(const std::string& filename)
{
	if (filesystem::is_compressed_file(filename)) {
		gui2::show_error_message(_("Save names should not end on '.gz' or '.bz2'. "
			"Please remove the extension."));
		return false;
	}

	return true;
}

bool savegame::is_illegal_file_char(char c)
{
	return c == '/' || c == '\\' || c == ':' || (c >= 0x00 && c < 0x20)
#ifdef _WIN32
	|| c == '?' || c == '|' || c == '<' || c == '>' || c == '*' || c == '"'
#endif
	;
}

std::string savegame::create_filename(unsigned int turn_number) const
{
	std::string filename = create_initial_filename(turn_number);
	filename.erase(std::remove_if(filename.begin(), filename.end(),
	            is_illegal_file_char), filename.end());
	std::replace(filename.begin(), filename.end(), '_', ' ');
	return filename;
}

void savegame::before_save()
{
}

bool savegame::save_game(const std::string& filename)
{

	try {
		uint32_t start, end;
		start = SDL_GetTicks();

		if (filename_.empty())
			filename_ = filename;

		before_save();

		write_game_to_disk(filename_);
		if (resources::persist != nullptr) {
			resources::persist->end_transaction();
			resources::persist ->start_transaction();
		}

		// Create an entry in the save_index. Doing this here ensures all leader image paths
		// sre expanded in a context-independent fashion and can appear in the Load Game dialog
		// even if a campaign-specific sprite is used. This is because the image's full path is
		// only available if the binary-path context its a part of is loaded. Without this, if
		// a player saves a game and exits the game or reloads the cache, the leader image will
		// only be available within that specific binary context (when playing another game from
		// the came campaign, for example).
		save_index_manager.rebuild(filename_);

		end = SDL_GetTicks();
		LOG_SAVE << "Milliseconds to save " << filename_ << ": " << end - start << std::endl;

		if (show_confirmation_)
			gui2::show_transient_message(_("Saved"), _("The game has been saved."));
		return true;
	} catch(game::save_game_failed& e) {
		ERR_SAVE << error_message_ << e.message << std::endl;

		gui2::show_error_message(error_message_ + e.message);
		//do not bother retrying, since the user can just try to save the game again
		//maybe show a yes-no dialog for "disable autosaves now"?

		return false;
	};
}

void savegame::write_game_to_disk(const std::string& filename)
{
	LOG_SAVE << "savegame::save_game" << std::endl;

	filename_ = filename;
	filename_ += compression::format_extension(compress_saves_);

	std::stringstream ss;
	{
		config_writer out(ss, compress_saves_);
		write_game(out);
		finish_save_game(out);
	}
	filesystem::scoped_ostream os(open_save_game(filename_));
	(*os) << ss.str();

	if (!os->good()) {
		throw game::save_game_failed(_("Could not write to file"));
	}
}

void savegame::write_game(config_writer &out)
{
	log_scope("write_game");

	out.write_key_val("version", game_config::version);

	gamestate_.write_general_info(out);
	out.open_child("statistics");
	statistics::write_stats(out);
	out.close_child("statistics");
}

void savegame::finish_save_game(const config_writer &out)
{
	try {
		if(!out.good()) {
			throw game::save_game_failed(_("Could not write to file"));
		}
		save_index_manager.remove(gamestate_.classification().label);
	} catch(filesystem::io_exception& e) {
		throw game::save_game_failed(e.what());
	}
}

// Throws game::save_game_failed
filesystem::scoped_ostream savegame::open_save_game(const std::string &label)
{
	try {
		return filesystem::ostream_file(filesystem::get_saves_dir() + "/" + label);
	} catch(filesystem::io_exception& e) {
		throw game::save_game_failed(e.what());
	}
}

scenariostart_savegame::scenariostart_savegame(saved_game &gamestate, const compression::format compress_saves)
	: savegame(gamestate, compress_saves)
{
	filename_ = create_filename();
}

std::string scenariostart_savegame::create_initial_filename(unsigned int) const
{
	return gamestate().classification().label;
}

void scenariostart_savegame::write_game(config_writer &out){
	savegame::write_game(out);
	gamestate().write_carryover(out);
}

replay_savegame::replay_savegame(saved_game &gamestate, const compression::format compress_saves)
	: savegame(gamestate, compress_saves, _("Save Replay"))
{}

std::string replay_savegame::create_initial_filename(unsigned int) const
{
	return formatter() << gamestate().classification().label << " " << _("replay");
}

void replay_savegame::write_game(config_writer &out) {
	savegame::write_game(out);

	gamestate().write_carryover(out);
	out.write_child("replay_start", gamestate().replay_start());

	out.open_child("replay");
	gamestate().get_replay().write(out);
	out.close_child("replay");

}

autosave_savegame::autosave_savegame(saved_game &gamestate, const compression::format compress_saves)
	: ingame_savegame(gamestate, compress_saves)
{
	set_error_message(_("Could not auto save the game. Please save the game manually."));
}

void autosave_savegame::autosave(const bool disable_autosave, const int autosave_max, const int infinite_autosaves)
{
	if(disable_autosave)
		return;

	save_game_automatic();

	remove_old_auto_saves(autosave_max, infinite_autosaves);
}

std::string autosave_savegame::create_initial_filename(unsigned int turn_number) const
{
	std::string filename;
	if(gamestate().classification().label.empty())
		filename = _("Auto-Save");
	else
		filename = gamestate().classification().label + "-" + _("Auto-Save") + std::to_string(turn_number);

	return filename;
}

oos_savegame::oos_savegame(saved_game& gamestate, bool& ignore)
	: ingame_savegame(gamestate, preferences::save_compression_format())
	, ignore_(ignore)
{}

int oos_savegame::show_save_dialog(const std::string& message, DIALOG_TYPE /*dialog_type*/)
{
	int res = 0;

	if (!ignore_){
		gui2::dialogs::game_save_oos dlg(ignore_, filename_, title(), message);
		dlg.show();
		res = dlg.get_retval();
	}

	if (!check_filename(filename_)) {
		res = gui2::retval::CANCEL;
	}

	return res;
}

ingame_savegame::ingame_savegame(saved_game &gamestate, const compression::format compress_saves)
	: savegame(gamestate, compress_saves, _("Save Game"))
{
}

std::string ingame_savegame::create_initial_filename(unsigned int turn_number) const
{
	return formatter() << gamestate().classification().label
		<< " " << _("Turn") << " " << turn_number;
}

void ingame_savegame::write_game(config_writer &out) {
	log_scope("write_game");

	if(!gamestate().get_starting_pos().validate_wml()) {
		throw game::save_game_failed(_("Game state is corrupted"));
	}

	savegame::write_game(out);

	gamestate().write_carryover(out);
	out.write_child("snapshot",gamestate().get_starting_pos());
	out.write_child("replay_start", gamestate().replay_start());
	out.open_child("replay");
	gamestate().get_replay().write(out);
	out.close_child("replay");
}

//changes done during 1.11.0-dev
static void convert_old_saves_1_11_0(config& cfg)
{
	if(!cfg.has_child("snapshot")){
		return;
	}

	const config& snapshot = cfg.child("snapshot");
	const config& replay_start = cfg.child("replay_start");
	const config& replay = cfg.child("replay");

	if(!cfg.has_child("carryover_sides") && !cfg.has_child("carryover_sides_start")){
		config carryover;
		//copy rng and menu items from toplevel to new carryover_sides
		carryover["random_seed"] = cfg["random_seed"];
		carryover["random_calls"] = cfg["random_calls"];
		for(const config& menu_item : cfg.child_range("menu_item")) {
			carryover.add_child("menu_item", menu_item);
		}
		carryover["difficulty"] = cfg["difficulty"];
		carryover["random_mode"] = cfg["random_mode"];
		//the scenario to be played is always stored as next_scenario in carryover_sides_start
		carryover["next_scenario"] = cfg["scenario"];

		config carryover_start = carryover;

		//copy sides from either snapshot or replay_start to new carryover_sides
		if(!snapshot.empty()){
			for(const config& side : snapshot.child_range("side")) {
				carryover.add_child("side", side);
			}
			//for compatibility with old savegames that use player instead of side
			for(const config& side : snapshot.child_range("player")) {
				carryover.add_child("side", side);
			}
			//save the sides from replay_start in carryover_sides_start
			for(const config& side : replay_start.child_range("side")) {
				carryover_start.add_child("side", side);
			}
			//for compatibility with old savegames that use player instead of side
			for(const config& side : replay_start.child_range("player")) {
				carryover_start.add_child("side", side);
			}
		} else if (!replay_start.empty()){
			for(const config& side : replay_start.child_range("side")) {
				carryover.add_child("side", side);
				carryover_start.add_child("side", side);
			}
			//for compatibility with old savegames that use player instead of side
			for(const config& side : replay_start.child_range("player")) {
				carryover.add_child("side", side);
				carryover_start.add_child("side", side);
			}
		}

		//get variables according to old hierarchy and copy them to new carryover_sides
		if(!snapshot.empty()){
			if(const config& variables_from_snapshot = snapshot.child("variables")){
				carryover.add_child("variables", variables_from_snapshot);
				carryover_start.add_child("variables", replay_start.child_or_empty("variables"));
			} else if (const config& variables_from_cfg = cfg.child("variables")){
				carryover.add_child("variables", variables_from_cfg);
				carryover_start.add_child("variables", variables_from_cfg);
			}
		} else if (!replay_start.empty()){
			if(const config& variables = replay_start.child("variables")){
				carryover.add_child("variables", variables);
				carryover_start.add_child("variables", variables);
			}
		} else {
			carryover.add_child("variables", cfg.child("variables"));
			carryover_start.add_child("variables", cfg.child("variables"));
		}

		cfg.add_child("carryover_sides", carryover);
		cfg.add_child("carryover_sides_start", carryover_start);
	}

	//if replay and snapshot are empty we've got a start of scenario save and don't want replay_start either
	if(replay.empty() && snapshot.empty()){
		LOG_RG<<"removing replay_start \n";
		cfg.clear_children("replay_start");
	}

	//remove empty replay or snapshot so type of save can be detected more easily
	if(replay.empty()){
		LOG_RG<<"removing replay \n";
		cfg.clear_children("replay");
	}

	if(snapshot.empty()){
		LOG_RG<<"removing snapshot \n";
		cfg.clear_children("snapshot");
	}
}
//changes done during 1.13.0-dev
static void convert_old_saves_1_13_0(config& cfg)
{
	if(config& carryover_sides_start = cfg.child("carryover_sides_start"))
	{
		if(!carryover_sides_start.has_attribute("next_underlying_unit_id"))
		{
			carryover_sides_start["next_underlying_unit_id"] = cfg["next_underlying_unit_id"];
		}
	}
	if(cfg.child_or_empty("snapshot").empty())
	{
		cfg.clear_children("snapshot");
	}
	if(cfg.child_or_empty("replay_start").empty())
	{
		cfg.clear_children("replay_start");
	}
	if(config& snapshot = cfg.child("snapshot"))
	{
		//make [end_level] -> [end_level_data] since its alo called [end_level_data] in the carryover.
		if(config& end_level = cfg.child("end_level") )
		{
			snapshot.add_child("end_level_data", end_level);
			snapshot.clear_children("end_level");
		}
		//if we have a snapshot then we already applied carryover so there is no reason to keep this data.
		if(cfg.has_child("carryover_sides_start"))
		{
			cfg.clear_children("carryover_sides_start");
		}
	}
	if(!cfg.has_child("snapshot") && !cfg.has_child("replay_start"))
	{
		cfg.clear_children("carryover_sides");
	}
	//This code is needed because for example otherwise it won't find the (empty) era
	if(!cfg.has_child("multiplayer")) {
		cfg.add_child("multiplayer", config {
			"mp_era", "era_blank",
			"mp_use_map_settings", true,
		});
	}
}


//changes done during 1.13.0+dev
static void convert_old_saves_1_13_1(config& cfg)
{
	if(config& multiplayer = cfg.child("multiplayer")) {
		if(multiplayer["mp_era"] == "era_blank") {
			multiplayer["mp_era"] = "era_default";
		}
	}
	//This currently only fixes start-of-scenario saves.
	if(config& carryover_sides_start = cfg.child("carryover_sides_start"))
	{
		for(config& side : carryover_sides_start.child_range("side"))
		{
			for(config& unit : side.child_range("unit"))
			{
				if(config& modifications = unit.child("modifications"))
				{
					for(config& advancement : modifications.child_range("advance"))
					{
						modifications.add_child("advancement", advancement);
					}
					modifications.clear_children("advance");
				}
			}
		}
	}
	for(config& snapshot : cfg.child_range("snapshot")) {
		if (snapshot.has_attribute("used_items")) {
			config used_items;
			for(const std::string& item : utils::split(snapshot["used_items"])) {
				used_items[item] = true;
			}
			snapshot.remove_attribute("used_items");
			snapshot.add_child("used_items", used_items);
		}
	}
}

void convert_old_saves(config& cfg)
{
	version_info loaded_version(cfg["version"]);
	if(loaded_version < version_info("1.12.0"))
	{
		convert_old_saves_1_11_0(cfg);
	}
	// '<= version_info("1.13.0")' doesn't work
	//because version_info cannot handle 1.13.0-dev versions correctly.
	if(loaded_version < version_info("1.13.1"))
	{
		convert_old_saves_1_13_0(cfg);
	}
	if(loaded_version <= version_info("1.13.1"))
	{
		convert_old_saves_1_13_1(cfg);
	}
	LOG_RG<<"cfg after conversion "<<cfg<<"\n";
}

}
