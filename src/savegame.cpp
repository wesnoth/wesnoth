/*
   Copyright (C) 2003 - 2018 by Jörg Hinrichs, refactored from various
   places formerly created by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
#include "units/ptr.hpp"
#include "units/unit.hpp"
#include "units/make.hpp"
#include "units/types.hpp"
#include "units/attack_type.hpp"
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
#include "game_version.hpp"
#include "video.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>

static lg::log_domain log_engine("engine");
#define LOG_SAVE LOG_STREAM(info, log_engine)
#define ERR_SAVE LOG_STREAM(err, log_engine)

static lg::log_domain log_enginerefac("enginerefac");
#define DBG_RG LOG_STREAM(debug, log_enginerefac)
#define LOG_RG LOG_STREAM(info, log_enginerefac)


namespace savegame {

bool save_game_exists(std::string name, compression::format compressed)
{
	name += compression::format_extension(compressed);
	return filesystem::file_exists(filesystem::get_saves_dir() + "/" + name);
}

void clean_saves(const std::string& label)
{
	const std::string prefix = label + "-" + _("Auto-Save");
	LOG_SAVE << "Cleaning saves with prefix '" << prefix << "'\n";

	for(const auto& save : get_saves_list()) {
		if(save.name().compare(0, prefix.length(), prefix) == 0) {
			LOG_SAVE << "Deleting savegame '" << save.name() << "'\n";
			delete_game(save.name());
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
		if(!difficulty_dlg.show()) {
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
        } catch (const utf8::invalid_utf8_exception&) {
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
	, title_(title)
	, gamestate_(gamestate)
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

std::string savegame::create_filename(unsigned int turn_number) const
{
	return create_initial_filename(turn_number);
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
	} catch(const game::save_game_failed& e) {
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
	} catch(const filesystem::io_exception& e) {
		throw game::save_game_failed(e.what());
	}
}

// Throws game::save_game_failed
filesystem::scoped_ostream savegame::open_save_game(const std::string &label)
{
	try {
		return filesystem::ostream_file(filesystem::get_saves_dir() + "/" + label);
	} catch(const filesystem::io_exception& e) {
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

	if(!gamestate().get_starting_point().validate_wml()) {
		throw game::save_game_failed(_("Game state is corrupted"));
	}

	savegame::write_game(out);

	gamestate().write_carryover(out);
	out.write_child("snapshot",gamestate().get_starting_point());
	out.write_child("replay_start", gamestate().replay_start());
	out.open_child("replay");
	gamestate().get_replay().write(out);
	out.close_child("replay");
}

std::string convert_version_to_suffix(config& cfg)
{
	std::string suffix = "";
	version_info loaded_version(cfg["version"]);

	if(loaded_version < version_info("1.5.0") && loaded_version >= version_info("1.3.0"))
		suffix = "_1_4";
	if(loaded_version < version_info("1.7.0") && loaded_version >= version_info("1.5.0"))
		suffix = "_1_6";
	if(loaded_version < version_info("1.9.0") && loaded_version >= version_info("1.7.0"))
		suffix = "_1_8";
	if(loaded_version < version_info("1.11.0") && loaded_version >= version_info("1.9.0"))
		suffix = "_1_10";
	if(loaded_version < version_info("1.13.0") && loaded_version >= version_info("1.11.0"))
		suffix = "_1_12";

	return suffix;
}

std::string append_suffix_to_unit_type(std::string unit_type, config& cfg)
{
	const std::string suffix = convert_version_to_suffix(cfg);
	return unit_type + suffix;
}

void update_start_unit_types(config& replay_start, config& cfg)
{
	const std::string suffix = convert_version_to_suffix(cfg);
	if(not suffix.empty())
	{
		for(config& side : replay_start.child_range("side"))
		{
 			std::ostringstream ss, tt, uu;
 			// BFW 1.4, 1.6
 			std::string sep = "";
			for(const std::string& leader : utils::split(side["leader"], ','))
			{
				ss << sep << leader << suffix;
				sep = ",";
			}
			std::string new_leader = ss.str();
			side["leader"] = new_leader;
 			if (side.has_attribute("recruit"))
			{
 				sep = "";
				for(const std::string& recruit : utils::split(side["recruit"], ','))
				{
					uu << sep << recruit << suffix;
					sep = ",";
				}
				std::string new_recruit = uu.str();
				side["recruit"] = new_recruit;
			}
 			if (side.has_attribute("random_leader"))
			{
 				sep = "";
				for(const std::string& leader : utils::split(side["random_leader"], ','))
				{
					tt << sep << leader << suffix;
					sep = ",";
				}
				std::string new_random = tt.str();
				side["random_leader"] = new_random;
			}
 			if (side.has_attribute("type")) side["type"] = side["type"].str()+suffix;

 			// BFW 1.8
 			if (side.has_attribute("previous_recruits"))
			{
 				sep = "";
				for(const std::string& recruit : utils::split(side["previous_recruits"], ','))
				{
					uu << sep << recruit << suffix;
					sep = ",";
				}
			std::string new_recruit = uu.str();
			side["previous_recruits"] = new_recruit;
			}
			for(config& unit : side.child_range("unit"))
			{
				bool found = unit["type"].str().find(suffix, 0);
				if(found == std::string::npos)
				{
					unit["type"] = unit["type"].str()+suffix;
 					std::ostringstream vv;
 					sep = "";
					for(const std::string& leader : utils::split(unit["advances_to"], ','))
					{
						vv << sep << leader << suffix;
						sep = ",";
					}
					std::string new_advance = vv.str();
					unit["advances_to"] = new_advance;
				}
			}
		}
	}
}

void update_replay_unit_types(config& replay, config& cfg)
{
	const std::string suffix = convert_version_to_suffix(cfg);
	if(suffix.empty()) return;
	for(config& command : replay.child_range("command"))
	{
 		// BFW 1.12
 		if (config& recruit = command.child("recruit"))
		{
 			recruit["type"] = recruit["type"] + suffix;
		}
		if (config& attack = command.child("attack"))
		{
			if (attack.has_attribute("attacker_type"))
			{
				attack["attacker_type"] = attack["attacker_type"] + suffix;
				attack["defender_type"] = attack["defender_type"] + suffix;
			}
		}
	}
}

void feral_trait_1_8_enter_hex(config& event)
{
	std::stringstream ss;
	ss << "		name=enter_hex\n";
	ss << "		first_time_only=no\n";
	ss << "		[filter]\n";
	ss << "			race=bats_1_8\n";
	ss << "			[filter_location]\n";
	ss << "				terrain=Ss^Vhs,Uu^Vu*,Dd^Vd*,Hh^Vhh*,Hhd^Vhh*,Aa^V*\n";
	ss << "			[/filter_location]\n";
	ss << "		[/filter]\n";
	ss << "     [command]\n";
	ss << "         [object]\n";
	ss << "             id=compatible_1_8_feral\n";
	ss << "             take_only_once=no\n";
	ss << "             duration=forever\n";
	ss << "             silent=yes\n";
	ss << "             description=\"In BFW 1.8, feral don't apply to this village.\"\n";
	ss << "             [filter]\n";
	ss << "                 id=$unit.id\n";
	ss << "             [/filter]\n";
	ss << "             [effect]\n";
	ss << "                 apply_to=defense\n";
	ss << "                 replace=yes\n";
	ss << "                 [defense]\n";
	ss << "                     village=40\n";
	ss << "                 [/defense]\n";
	ss << "             [/effect]\n";
	ss << "		    [/object]\n";
	ss << "         [on_undo]\n";
	ss << "             [remove_object]\n";
	ss << "                 object_id=compatible_1_8_feral\n";
	ss << "                 id=$unit.id\n";
	ss << "             [/remove_object]\n";
	ss << "         [/on_undo]\n";
	ss << "         [allow_undo]\n";
	ss << "         [/allow_undo]\n";
	ss << "     [/command]\n";
	read(event, ss);
}

void feral_trait_1_8_exit_hex(config& event)
{
	std::stringstream ss;
	ss << "		name=exit_hex\n";
	ss << "		first_time_only=no\n";
	ss << "		[filter]\n";
	ss << "			race=bats_1_8\n";
	ss << "			[filter_location]\n";
	ss << "				terrain=Ss^Vhs,Uu^Vu*,Dd^Vd*,Hh^Vhh*,Hhd^Vhh*,Aa^V*\n";
	ss << "			[/filter_location]\n";
	ss << "		[/filter]\n";
	ss << "     [command]\n";
	ss << "         [remove_object]\n";
	ss << "             object_id=compatible_1_8_feral\n";
	ss << "             id=$unit.id\n";
	ss << "         [/remove_object]\n";
	ss << "     [/command]\n";
	read(event, ss);
}

void rounding_trait_1_8_add_one_hitpoint(config& event)
{
	std::stringstream ss;
	ss << "        name=\"add one hitpoint at recruit\"\n";
	ss << "        first_time_only=no\n";
	ss << "        [command]\n";
	ss << "            [object]\n";
	ss << "                duration = forever\n";
	ss << "                id=\"compatible_1_8_rounding_trait\"\n";
	ss << "                silent=yes\n";
	ss << "                take_only_once=no\n";
	ss << "                [filter]\n";
	ss << "                    x,y=$x1,$y1\n";
	ss << "                [/filter]\n";
	ss << "                [effect]\n";
	ss << "                    apply_to=\"hitpoints\"\n";
	ss << "                    increase_total=1\n";
	ss << "                    heal_full=yes\n";
	ss << "                [/effect]\n";
	ss << "            [/object]\n";
	ss << "        [/command]\n";
	read(event, ss);
}

void make_quick_leader_1_6_prestart(config& event)
{
	std::stringstream ss;
	ss << "        name=\"prestart\"\n";
	ss << "        [store_unit]\n";
	ss << "            kill=yes\n";
	ss << "            variable=\"leaders_to_make_quick\"\n";
	ss << "            [filter]\n";
	ss << "                canrecruit=yes\n";
	ss << "                [filter_wml]\n";
	ss << "                    max_moves=4\n";
	ss << "                [/filter_wml]\n";
	ss << "            [/filter]\n";
	ss << "        [/store_unit]\n";
	ss << "        [set_variable]\n";
	ss << "            name=\"i\"\n";
	ss << "            value=0\n";
	ss << "        [/set_variable]\n";
	ss << "        [while]\n";
	ss << "            [variable]\n";
	ss << "                less_than=\"$leaders_to_make_quick.length\"\n";
	ss << "                name=\"i\"\n";
	ss << "            [/variable]\n";
	ss << "            [do]\n";
	ss << "                [if]\n";
	ss << "                    [variable]\n";
	ss << "                        boolean_equals=yes\n";
	ss << "                        name=\"leaders_to_make_quick[$i].variables.dont_make_me_quick\"\n";
	ss << "                    [/variable]\n";
	ss << "                    [then]\n";
	ss << "                        [unstore_unit]\n";
	ss << "                            variable=\"leaders_to_make_quick[$i]\"\n";
	ss << "                        [/unstore_unit]\n";
	ss << "                    [/then]\n";
	ss << "                    [else]\n";
	ss << "                        [set_variables]\n";
	ss << "                            name=\"temp\"\n";
	ss << "                            [literal]\n";
	ss << "                                [trait]\n";
	ss << "#textdomain wesnoth-help\n";
	ss << "                                    female_name=_\"female^quick\"\n";
	ss << "                                    id=\"quick\"\n";
	ss << "                                    male_name=_\"quick\"\n";
	ss << "                                    [effect]\n";
	ss << "                                        apply_to=\"movement\"\n";
	ss << "                                        increase=1\n";
	ss << "                                    [/effect]\n";
	ss << "                                    [effect]\n";
	ss << "                                        apply_to=\"hitpoints\"\n";
	ss << "                                        increase_total=\"-5%\"\n";
	ss << "                                    [/effect]\n";
	ss << "                                [/trait]\n";
	ss << "                            [/literal]\n";
	ss << "                        [/set_variables]\n";
	ss << "                        [set_variables]\n";
	ss << "                            mode=\"append\"\n";
	ss << "                            name=\"leaders_to_make_quick[$i].modifications.trait\"\n";
	ss << "                            [insert_tag]\n";
	ss << "                                name=\"literal\"\n";
	ss << "                                variable=\"temp.trait\"\n";
	ss << "                            [/insert_tag]\n";
	ss << "                        [/set_variables]\n";
	ss << "                        [clear_variable]\n";
	ss << "                            name=\"leaders_to_make_quick[$i].max_moves,leaders_to_make_quick[$i].moves,leaders_to_make_quick[$i].max_hitpoints,leaders_to_make_quick[$i].hitpoints\"\n";
	ss << "                        [/clear_variable]\n";
	ss << "                        [unstore_unit]\n";
	ss << "                            variable=\"leaders_to_make_quick[$i]\"\n";
	ss << "                        [/unstore_unit]\n";
	ss << "                    [/else]\n";
	ss << "                [/if]\n";
	ss << "                [set_variable]\n";
	ss << "                    add=1\n";
	ss << "                    name=\"i\"\n";
	ss << "                [/set_variable]\n";
	ss << "            [/do]\n";
	ss << "        [/while]\n";
	ss << "        [clear_variable]\n";
	ss << "            name=\"i\"\n";
	ss << "        [/clear_variable]\n";
	ss << "        [clear_variable]\n";
	ss << "            name=\"leaders_to_make_quick,temp\"\n";
	ss << "        [/clear_variable]\n";
	read(event, ss);
}

void healthy_trait_1_6_side_turn(config& event)
{
	std::stringstream ss;
	ss << "        id=\"healthy side turn\"\n";
	ss << "        name=\"side turn\"\n";
	ss << "        first_time_only=no\n";
	ss << "        [heal_unit]\n";
	ss << "            [filter]\n";
	ss << "                side=$side_number\n";
	ss << "                [filter_wml]\n";
	ss << "                    [not]\n";
	ss << "                        [status]\n";
	ss << "                            poisoned=yes\n";
	ss << "                        [/status]\n";
	ss << "                    [/not]\n";
	ss << "                    [not]\n";
	ss << "                        [status]\n";
	ss << "                            fighting=yes\n";
	ss << "                        [/status]\n";
	ss << "                    [/not]\n";
	ss << "                    [modifications]\n";
	ss << "                        [trait]\n";
	ss << "                            id=healthy_1_6\n";
	ss << "                        [/trait]\n";
	ss << "                    [/modifications]\n";
	ss << "                [/filter_wml]\n";
	ss << "            [/filter]\n";
	ss << "            amount=2\n";
	ss << "            animate=yes\n";
	ss << "            restore_statuses=no\n";
	ss << "        [/heal_unit]\n";
	read(event, ss);
}

void healthy_trait_1_6_poisoned(config& event)
{
	std::stringstream ss;
	ss << "        id=\"poisoned healthy side turn\"\n";
	ss << "        name=\"side turn\"\n";
	ss << "        first_time_only=no\n";
	ss << "        [heal_unit]\n";
	ss << "            [filter]\n";
	ss << "                side=$side_number\n";
	ss << "                [filter_wml]\n";
	ss << "                    [status]\n";
	ss << "                        poisoned=yes\n";
	ss << "                    [/status]\n";
	ss << "                    [modifications]\n";
	ss << "                        [trait]\n";
	ss << "                            id=healthy_1_6\n";
	ss << "                        [/trait]\n";
	ss << "                    [/modifications]\n";
	ss << "                [/filter_wml]\n";
	ss << "            [/filter]\n";
	ss << "            amount=2\n";
	ss << "            animate=yes\n";
	ss << "            restore_statuses=no\n";
	ss << "        [/heal_unit]\n";
	read(event, ss);
}

void healthy_trait_1_4_side_turn(config& event)
{
	std::stringstream ss;
	ss << "        id=\"healthy side turn\"\n";
	ss << "        name=\"side turn\"\n";
	ss << "        first_time_only=no\n";
	ss << "        [heal_unit]\n";
	ss << "            [filter]\n";
	ss << "                side=$side_number\n";
	ss << "                [filter_wml]\n";
	ss << "                    [not]\n";
	ss << "                        [status]\n";
	ss << "                            poisoned=yes\n";
	ss << "                        [/status]\n";
	ss << "                    [/not]\n";
	ss << "                    [not]\n";
	ss << "                        [status]\n";
	ss << "                            fighting=yes\n";
	ss << "                        [/status]\n";
	ss << "                    [/not]\n";
	ss << "                    [modifications]\n";
	ss << "                        [trait]\n";
	ss << "                            id=healthy_1_4\n";
	ss << "                        [/trait]\n";
	ss << "                    [/modifications]\n";
	ss << "                [/filter_wml]\n";
	ss << "            [/filter]\n";
	ss << "            amount=2\n";
	ss << "            animate=yes\n";
	ss << "            restore_statuses=no\n";
	ss << "        [/heal_unit]\n";
	read(event, ss);
}

void healthy_trait_1_4_turn_refresh(config& event)
{
	std::stringstream ss;
	ss << "        name=\"healthy turn refresh\"\n";
	ss << "        name=\"turn refresh\"\n";
	ss << "        first_time_only=no\n";
	ss << "        [modify_unit]\n";
	ss << "            [filter]\n";
	ss << "                side=$side_number\n";
	ss << "            [/filter]\n";
	ss << "            [object]\n";
	ss << "            [effect]\n";
	ss << "                apply_to=status\n";
	ss << "                remove=fighting\n";
	ss << "            [/effect]\n";
	ss << "            [/object]\n";
	ss << "        [/modify_unit]\n";
	read(event, ss);
}

void healthy_trait_1_4_attack_end(config& event)
{
	std::stringstream ss;
	ss << "        name=\"healthy attack end\"\n";
	ss << "        name=\"attack end\"\n";
	ss << "        first_time_only=no\n";
	ss << "        [modify_unit]\n";
	ss << "            [filter]\n";
	ss << "                id=$unit.id\n";
	ss << "            [/filter]\n";
	ss << "            [object]\n";
	ss << "                [effect]\n";
	ss << "                    apply_to=status\n";
	ss << "                    add=fighting\n";
	ss << "                [/effect]\n";
	ss << "            [/object]\n";
	ss << "        [/modify_unit]\n";
	ss << "        [modify_unit]\n";
	ss << "            [filter]\n";
	ss << "                id=$second_unit.id\n";
	ss << "            [/filter]\n";
	ss << "            [object]\n";
	ss << "                [effect]\n";
	ss << "                    apply_to=status\n";
	ss << "                    add=fighting\n";
	ss << "                [/effect]\n";
	ss << "            [/object]\n";
	ss << "        [/modify_unit]\n";
	read(event, ss);
}

void update_prestart_detect_factions(config& cfg)
{
	if(config& replay = cfg.child("replay_start"))
	{
		for(config& event: replay.child_range("event"))
		{
			if(event["name"] == "prestart")
			{
				if(config& fire_event = event.child("fire_event"))
				{
					if(fire_event["name"] == "place_units")
					{
						config& lua = event.child("lua");

						std::string changes[] = {"if searched == actual then;if actual:sub(1, #searched) == searched  then",
												"if searched == actual.type then;if actual.type:sub(1, #searched) == searched then"};
						std::ostringstream ss;
				 		for (std::string& line : utils::split(lua["code"].str(), '\n'))
				 		{
							std::size_t found;
							std::string oldcode, newcode;
							for(auto it=std::begin(changes); it!=std::end(changes); ++it)
							{
								int sep = (*it).find(";");
								oldcode = (*it).substr(0, sep);
								newcode = (*it).substr(sep+1);
								found = line.find(oldcode, 0);
								if(found != std::string::npos)
								{
									line.replace(found, oldcode.length(), newcode);
									break;
								}
							}
							ss << line << "\n";
						}
						lua["code"] = ss.str();
					}
				}
			}
		}
	}
}

void update_pre_recruited_loyals(config& cfg)
{
	const std::string suffix = convert_version_to_suffix(cfg);
	if(suffix.empty()) return;
	if(config& replay = cfg.child("replay_start"))
	{
		for(config& event: replay.child_range("event"))
		{
			if(event["name"] == "place_units")
			{
				for(config& switch_: event.child_range("switch"))
				{
					for(config& case_: switch_.child_range("case"))
					{
						for(config& unit: case_.child_range("unit"))
						{
							unit["type"] = unit["type"] + suffix;
						}
					}
					if(config& else_ = switch_.child("else"))
					{
						for(config& unit: else_.child_range("unit"))
						{
							unit["type"] = unit["type"] + suffix;
						}
					}
				}
			}
		}
	}
}

void convert_old_saves_1_03_0_for_BFW1_10(config& cfg)
{
    config& snapshot = cfg.child("snapshot");
    config& replay_start = cfg.child("replay_start");
    config& replay = cfg.child("replay");

	cfg.clear_children("player");
	replay_start.clear_children("player");

    // Petrify units on Caves of the Basilisk, Sulla’s Ruins, ...
	if(config& replay = cfg.child("replay_start"))
	{
		config& era = replay.child("era");
		config& side_turn_event = era.add_child("event");
        healthy_trait_1_4_side_turn(side_turn_event);
		config& turn_refresh_event = era.add_child("event");
		healthy_trait_1_4_turn_refresh(turn_refresh_event);
		config& attack_end_event = era.add_child("event");
		healthy_trait_1_4_attack_end(attack_end_event);
		for(config& side: replay.child_range("side"))
		{
			for(config& unit: side.child_range("unit"))
			{
				if(config& status = unit.child("status"))
				{
					if(status.has_attribute("stone"))
					{
						status.remove_attribute("stone");
						status["stoned"]="on";
					}
				}
			}
		}
	}
    if(config& replay = cfg.child("replay"))
    {
        for(config& command: replay.child_range("command"))
        {
            if(config& speak = command.child("speak"))
            {
                speak["id"] = speak["description"];
                speak.remove_attribute("description");
            }
            if(config& move = command.child("move"))
            {
                config& source = move.child("source");
                config& destination = move.child("destination");
                std::stringstream xx, yy;
                xx << source["x"] << "," << destination["x"];
                yy << source["y"] << "," << destination["y"];
                move["x"] = xx.str();
                move["y"] = yy.str();
                move.clear_children("source");
                move.clear_children("destination");
            }
        }
    }
}

void convert_old_saves_1_03_0_for_BFW1_14(config& cfg)
{
}

void convert_old_saves_1_05_0_for_BFW1_10(config& cfg)
{
	config& snapshot = cfg.child("snapshot");
	config& replay_start = cfg.child("replay_start");
	config& replay = cfg.child("replay");

    // Petrify units on Caves of the Basilisk, Sulla’s Ruins, ...
	if(config& replay = cfg.child("replay_start"))
	{
		for(config& side: replay.child_range("side"))
		{
			for(config& unit: side.child_range("unit"))
			{
				if(config& status = unit.child("status"))
				{
					if(status.has_attribute("stoned"))
					{
						status.remove_attribute("stoned");
						status["petrified"]="yes";
					}
				}
			}
		}
	}
	version_info loaded_version(cfg["version"]);
	if(loaded_version >= version_info("1.5.0"))
	{
		if(config& replay = cfg.child("replay_start"))
		{
			config& era = replay.child("era");
			config& quick_leader_event = era.add_child("event");
			make_quick_leader_1_6_prestart(quick_leader_event);
			config& side_turn_event = era.add_child("event");
			healthy_trait_1_6_side_turn(side_turn_event);
			config& poisoned_event = era.add_child("event");
			healthy_trait_1_6_poisoned(poisoned_event);
			config& turn_refresh_event = era.add_child("event");
			healthy_trait_1_4_turn_refresh(turn_refresh_event);
			config& attack_end_event = era.add_child("event");
			healthy_trait_1_4_attack_end(attack_end_event);
		}
	}
	int max_side_id = 0;
	std::vector<config::attribute_value> recruit_lists;
	if(config& replay = cfg.child("replay_start"))
	{
		cfg["scenario"] = replay["scenario"];
		cfg["next_scenario"] = "";
		for(config& side: replay.child_range("side"))
		{
			config leader_create;
			leader_create["id"] = side["id"];
			leader_create["name"] = side["name"];
			leader_create["side"] = side["side"];
			leader_create["type"] = append_suffix_to_unit_type(side["type"], cfg);
			leader_create["gender"] = side["gender"];
			// from src/gamestatus.cpp, handle_leader()
			leader_create["canrecruit"] = "yes";
			leader_create["placement"] = "map,leader";

			unit_ptr leader = make_unit_ptr(leader_create /*, use_traits=false*/);
			config& leader_config = side.add_child("unit");
			leader->write(leader_config);

			side["previous_recruits"] = side["recruit"];
			side.remove_attribute("recruit");
		}
		// Uses result from previous loop side["previous_recruits"]
		for(config& side: replay.child_range("side"))
		{
			int side_id = side["side"].to_int();
			max_side_id = side_id > max_side_id ? side_id : max_side_id;
			recruit_lists.push_back(side["previous_recruits"]);
		}
	}
	if(config& replay = cfg.child("replay"))
	{
		config new_replay;
		config init_command;
		init_command["sent"] = "yes";
		init_command.add_child("init_side");
		for(config& command: replay.child_range("command"))
		{
			new_replay.add_child("command", command);
			if(command.child("start") || command.child("end_turn"))
			{
				new_replay.add_child("command", init_command);
			}
		}
		cfg.clear_children("replay");
		cfg.add_child("replay", new_replay);
	}
	int active_side_id = 0;
	if(config& replay = cfg.child("replay"))
	{
		for(config& command: replay.child_range("command"))
		{
			if(config& init_side = command.child("init_side"))
			{
				active_side_id = active_side_id+1 > max_side_id ? 1 : active_side_id+1;
			}
			if(config& recruit = command.child("recruit"))
			{
				int recruit_list_index = recruit["value"].to_int();
				std::vector<std::string> recruit_list = utils::split(recruit_lists[active_side_id-1], ',');

				std::sort(recruit_list.begin(), recruit_list.end());
				std::string recruit_type = recruit_list[recruit_list_index];

				// Need to swap the numbers from generate_traits and generate_name
				// As is, we don't care for keeping the name numbers correct
				int offset = 1;
				if (unit_types.find(recruit_type)->genders().size() < 2) offset = 0;

				// Offset of 12 numbers for random name and 1 for gender
				int random_number_offset = 12 + 1;
				const int random_calls = command.child_count("random");
				switch (random_calls)
				{
					case 16: // This is 1.4 replays (units have 2 random traits)
					case 15: // This is 1.6 replays (for units with 2 random traits)
						{
						config& first_trait_stored = command.child("random", random_number_offset);
						config& second_trait_stored = command.child("random", random_number_offset+1);
						config& first_trait_new = command.child("random", offset);
						config& second_trait_new = command.child("random", offset+1);
						first_trait_new["value"] = first_trait_stored["value"];
						second_trait_new["value"] = second_trait_stored["value"];
						break;
						}
					case 14: // This is 1.6 replays (for units with 1 random trait: bat, goblin)
						{
						config& first_trait_stored = command.child("random", random_number_offset);
						config& first_trait_new = command.child("random", offset);
						first_trait_new["value"] = first_trait_stored["value"];
						break;
						}
				}
				config& last_stored = command.child("random", random_calls-1);
				// Remove the checksum when we can't get it right
				if (offset == 1) last_stored.clear_children("results");
			}
		}
	}
}

void convert_old_saves_1_05_0_for_BFW1_14(config& cfg)
{
	int max_side_id = 0;
	std::vector<config::attribute_value> recruit_lists;
	if(config& replay = cfg.child("replay_start"))
	{
		// Uses result from convert_old_saves_1_05_0_for_BFW1_10
		for(config& side: replay.child_range("side"))
		{
			int side_id = side["side"].to_int();
			max_side_id = side_id > max_side_id ? side_id : max_side_id;
			recruit_lists.push_back(side["previous_recruits"]);
		}
	}

	// Move [era] events to [replay_start]
	if(config& replay = cfg.child("replay_start"))
	{
		if(config& era = replay.child("era"))
		{
			for(config& era_event: era.child_range("event"))
			{
				if(not era_event.empty()) replay.add_child("event", era_event);
			}
			era.clear_children("event");
		}
	}

	int active_side_id = 0;
	if(config& replay = cfg.child("replay"))
	{
		for(config& command: replay.child_range("command"))
		{
			if(config& init_side = command.child("init_side"))
			{
				active_side_id = active_side_id+1 > max_side_id ? 1 : active_side_id+1;
			}
			if(config& recruit = command.child("recruit"))
			{
				int recruit_list_index = recruit["value"].to_int();
				std::vector<std::string> recruit_list = utils::split(recruit_lists[active_side_id-1], ',');

				std::sort(recruit_list.begin(), recruit_list.end());
				std::string recruit_type = recruit_list[recruit_list_index];

				bool gender_choice = true;
				if (unit_types.find(recruit_type)->genders().size() < 2) gender_choice = false;

				const int random_calls = command.child_count("random");
				switch (random_calls)
				{
					case 16: // This is 1.4 replays (units have 2 random traits)
						{
						command.remove_child("random", random_calls-1);
						if(not gender_choice) command.remove_child("random", random_calls-2);
						break;
						}
					case 15: // This is 1.6 replays (for units with 2 random traits)
						{
						if(not gender_choice) command.remove_child("random", random_calls-1);
						break;
						}
					case 14: // This is 1.6 replays (for units with 1 random trait: bat, goblin)
						{
						if((unit_types.find(recruit_type)->num_traits() == 0) ||
							(unit_types.find(recruit_type)->race()->id() == "undead"))
						{
							// This is 1.4 replays (for units with 0 random trait: wose)
							// This is 1.4 replays (for units with 1 random trait: undead (is a trait))
							command.remove_child("random", random_calls-1);
							command.remove_child("random", random_calls-2);
						} else if(not gender_choice) command.remove_child("random", random_calls-1);
						break;
						}
					case 13: // This is 1.4 replays (for units with 0 random trait: walking corpse)
						{
						command.remove_child("random", random_calls-1);
						break;
						}
				}
			}
		}
	}
	// Remove the random number in attack for unit created from plague
	if(config& replay = cfg.child("replay"))
	{
		for(config& command: replay.child_range("command"))
		{
			if(config& attack = command.child("attack"))
			{
				unsigned int count = 0;
				bool remove_additional_numbers = false;
				for(config& random: command.child_range("random"))
				{
					++count;
					if(config& results = random.child("results"))
					{
						if(results["dies"] == "yes")
						{
							remove_additional_numbers = true;
							break;
						}
					}
					//if(remove_additional_numbers) command.remove_child("random", count);
					//else { ++count; }
				}
				if(remove_additional_numbers && count < command.child_count("random")-1)
				{
					for(unsigned int i = command.child_count("random")-1; i >= count; i--)
					{
						command.remove_child("random", i);
					}
				}
			}
		}
	}
}

void convert_old_saves_1_07_0_for_BFW1_10(config& cfg)
{
	config& snapshot = cfg.child("snapshot");
	config& replay_start = cfg.child("replay_start");
	config& replay = cfg.child("replay");

	config& enter_hex_event = replay_start.add_child("event");
	feral_trait_1_8_enter_hex(enter_hex_event);
	config& exit_hex_event = replay_start.add_child("event");
	feral_trait_1_8_exit_hex(exit_hex_event);

    // Petrify units on Caves of the Basilisk, Sulla’s Ruins, ...
	if(config& replay = cfg.child("replay_start"))
	{
		for(config& side: replay.child_range("side"))
		{
			for(config& unit: side.child_range("unit"))
			{
				if(config& status = unit.child("status"))
				{
					if(status.has_attribute("petrified"))
					{
						// Sometimes it is "on" and it isn't a valid value.
						status["petrified"]="yes";
						unit["ellipse"]="none";
						unit["zoc"]="no";
					}
				}
			}
		}
	}

	// Fix Aethermath events for opening border
	if(config& replay = cfg.child("replay_start"))
	{
		for(config& event: replay.child_range("event"))
		{
			for(config& color: event.child_range("colour_adjust"))
			{
				event.add_child("color_adjust", color);
			}
			event.clear_children("colour_adjust");
		}
	}

	int max_side_id = 0;
	std::vector<config::attribute_value> recruit_lists;
	std::vector<std::pair<int, map_location> > leader_positions;
	if(config& replay = cfg.child("replay_start"))
	{
		for(config& side: replay.child_range("side"))
		{
			int side_id = side["side"].to_int();
			if (side["controller"] != "null") // The statues in CotB, no problem in 1.4-1.6?!
			{
			    max_side_id = side_id > max_side_id ? side_id : max_side_id;
			    recruit_lists.push_back(side["previous_recruits"]);
			}
		}

		int counter_x = 0, counter_y = 0;
		for (std::string& line : utils::split(replay["map_data"].str(), '\n'))
		{
			if (line.empty() || line.find("usage") == 0 || line.find("border_size") == 0) continue;
			counter_x = 0;
			for (std::string& hex : utils::split(line, ','))
			{
				for (auto it=hex.begin(); it!=hex.end(); it++)
				{
					char** null = 0;
					if (*it == ' ') continue;
					if (int side = strtol(&(*it), null, 10))
					{
						// See map_location::map_location(config): "we want the coordinates as 0-based."
						leader_positions.push_back(std::make_pair(side, map_location(counter_x-1, counter_y-1)));
					}
					break;
				}
				counter_x++;
			}
			counter_y++;
		}
	}

	int active_side_id = 0;
	map_location leader_pos = map_location();
	if(config& replay = cfg.child("replay"))
	{
		for(config& command: replay.child_range("command"))
		{
			if(config& init_side = command.child("init_side"))
			{
				active_side_id = active_side_id+1 > max_side_id ? 1 : active_side_id+1;
				command["sent"] = "";
				init_side["side_number"] = active_side_id;
				for (auto it = leader_positions.begin() ; it != leader_positions.end(); ++it)
				{
					if ((*it).first == active_side_id)
					{
						leader_pos = (*it).second;
						break;
					}
				}
			}
			if(config& move = command.child("move"))
			{
				std::vector<map_location> steps;
				read_locations(move, steps);

				for (unsigned i=0 ; i<leader_positions.size(); i++)
				{
					if (leader_positions[i].first == active_side_id)
					{
						if (leader_positions[i].second == steps.front())
						{
							std::stringstream ss;
							ss << "Leader recorded move: ";
							for (auto it=steps.begin() ; it!=steps.end(); it++)
							{
								ss << (*it).x << "." << (*it).y << "|";
							}
							DBG_RG << ss.str() << "\n";
							leader_positions[i] = std::make_pair(active_side_id, steps.back());
							leader_pos = leader_positions[i].second;
							break;
						}
					}
				}
			}
			if(config& recruit = command.child("recruit"))
			{
				const int random_calls = command.child_count("random");
				config& last_stored = command.child("random", random_calls-1);
				// Remove the checksum that we can't get right
				last_stored.clear_children("results");

				config& leader = recruit.add_child("from");
				leader["x"] = leader_pos.x+1;
				leader["y"] = leader_pos.y+1;
				command["value"] = command["value"].to_int();
			}
		}
	}
}

void convert_old_saves_1_07_0_for_BFW1_14(config& cfg)
{
	if(config& replay = cfg.child("replay_start"))
	{
		// From data/tools/wmllint
		// While wmllimt set Gg^Emf for flower, it seems BFW1.14 defines Gg^Efm (Gg^Emf shows as mushroom)
		// From Cynsaun Battlefield (default): Ggf^Fet, Ggf^Fp
		// From Arcanclave Citadel (map pack?): Ggf^Fms
		std::string changes[] = {"^Voha;^Voa", "^Voh;^Vo", "^Vhms;^Vhha", "^Vhm;^Vhh", "^Vcha;^Vca", "^Vch;^Vc", "^Vcm;^Vc", "Ggf^Fms;Gg^Fms", "Ggf^Fet;Gg^Fet", "Ggf^Fp;Gg^Fms", "Ggf^Uf;Gs^Uf", "Ggf;Gg^Efm",  "Qv;Mv"};
		std::ostringstream ss;
		std::size_t found;
		ss << "border_size=1\nusage=map\n\n";
 		for (std::string& line : utils::split(replay["map_data"].str(), '\n'))
 		{
			if (line.empty() || line.find("usage") == 0 || line.find("border_size") == 0)
			{
				//ss << line << "\n";
				continue;
			} else {
				std::string oldmap, newmap;
				for(auto it=std::begin(changes); it!=std::end(changes); ++it)
				{
					int sep = (*it).find(";");
					oldmap = (*it).substr(0, sep);
					newmap = (*it).substr(sep+1);
					std::size_t begin = 0;
					found = line.find(oldmap, begin);
					while(found != std::string::npos)
					{
						line.replace(found, oldmap.length(), newmap);
						begin = found+newmap.length();
						found = line.find(oldmap, begin);
					}
				}
				ss << line << "\n";
			}
		}
		replay["map_data"] = ss.str();

		for(config& side: replay.child_range("side"))
		{
			if(side.has_attribute("colour")) side["color"] = side["colour"];
		}
	}

	{
	// We need to find all quick 1.8 [29,30]hp units to give them 1 additional hitpoint
	int max_side_id = 0;
	std::vector<config::attribute_value> recruit_lists;
	if(config& replay = cfg.child("replay_start"))
	{
	    config& add_one_hitpoint_event = replay.add_child("event");
	    rounding_trait_1_8_add_one_hitpoint(add_one_hitpoint_event);
		for(config& side: replay.child_range("side"))
		{
            int side_id = side["side"].to_int();

			if(side["controller"] != "null")
			{
				max_side_id = side_id > max_side_id ? side_id : max_side_id;
				recruit_lists.push_back(side["previous_recruits"]);
			}
		}
	}
	if(config& replay = cfg.child("replay"))
	{
		config new_replay;
		int active_side_id = 0;
		if(config& log = replay.child("upload_log"))
		{
			new_replay["upload_log"] = replay["upload_log"];
		}
		for(config& command: replay.child_range("command"))
		{
			if(config& init_side = command.child("init_side"))
			{
				active_side_id = active_side_id+1 > max_side_id ? 1 : active_side_id+1;
			}
			config fire_event;
			if(config& recruit = command.child("recruit"))
			{
            	int recruit_list_index = recruit["value"].to_int();
				std::vector<std::string> recruit_list = utils::split(recruit_lists[active_side_id-1], ',');
				std::sort(recruit_list.begin(), recruit_list.end());
				std::string this_recruit = recruit_list[recruit_list_index];
				int matching_traits = 0;
				if (!this_recruit.compare(/*pos=*/ 0, /*len=*/ 7, "Footpad") ||
					!this_recruit.compare(/*pos=*/ 0, /*len=*/ 13, "Merman Hunter"))
				{
					int first_trait_pool = 4;
					int second_trait_pool = first_trait_pool-1;
					// Default traits are defined as: {TRAIT_STRONG, TRAIT_QUICK, TRAIT_INTELLIGENT, TRAIT_RESILIENT}
					int quick_first_trait = 1; // looking in a set of 4
					int intelligent_first_trait = 2; // looking in a set of 4
					int quick_second_trait = 1; // looking in a set of 3
					int random_number_offset = 1; // Footpad has 2 genders, so one random number for gender
					if (!this_recruit.compare(/*pos=*/ 0, /*len=*/ 13, "Merman Hunter")) random_number_offset = 0;

					config::const_child_itors it_random = command.child_range("random");
					int count = 0;
					for(const config& random: it_random)
					{
						if(count == random_number_offset)
						{
							if((random["value"].to_int() % first_trait_pool) == quick_first_trait)
							{
								matching_traits = 2;
								break;
							} else if((random["value"].to_int() % first_trait_pool) == intelligent_first_trait) {
								++matching_traits;
							}
						}
						if(count == random_number_offset+1 && ((random["value"].to_int() % second_trait_pool) == quick_second_trait))
						{
							++matching_traits;
						}
						++count;
					}
				}
				if (!this_recruit.compare(/*pos=*/ 0, /*len=*/ 13, "Elvish Archer"))
				{
					int first_trait_pool = 5;
					int second_trait_pool = first_trait_pool-1;
					// Default traits are defined as: {TRAIT_STRONG, TRAIT_QUICK, TRAIT_INTELLIGENT, TRAIT_RESILIENT, TRAIT_DEXTROUS}
					int strong_first_trait = 0; // looking in a set of 5
					int quick_second_trait = 0; // looking in a set of 4
					int random_number_offset = 1; // Elvish Archer has 2 genders, so one random number for gender

					config::const_child_itors it_random = command.child_range("random");
					int count = 0;
					for(const config& random: it_random)
					{
						if(count == random_number_offset && ((random["value"].to_int() % first_trait_pool) == strong_first_trait))
						{
							++matching_traits;
						}
						if(count == random_number_offset+1 && ((random["value"].to_int() % second_trait_pool) == quick_second_trait))
						{
							++matching_traits;
						}
						++count;
					}
				}
				if(matching_traits == 2)
				{
					fire_event["from_side"] = active_side_id;
					config& event = fire_event.add_child("fire_event");
					event["raise"] = "add one hitpoint at recruit";
					config& source = event.add_child("source");
					source["x"] = recruit["x"];
					source["y"] = recruit["y"];
				}
			}
			new_replay.add_child("command", command);
			if(fire_event.has_child("fire_event"))
			{
				new_replay.add_child("command", fire_event);
			}
		}
		cfg.clear_children("replay");
		cfg.add_child("replay", new_replay);
	}
	}
}

void convert_old_saves_1_09_0(config& cfg)
{
	config& snapshot = cfg.child("snapshot");
	config& replay_start = cfg.child("replay_start");
	config& replay = cfg.child("replay");

	if(!cfg.has_child("carryover_sides") && !cfg.has_child("carryover_sides_start")){
		config carryover;
		//copy rng and menu items from toplevel to new carryover_sides
		carryover["random_seed"] = cfg["random_seed"];
		carryover["random_calls"] = cfg["random_calls"];
		for(const config& menu_item: cfg.child_range("menu_item")){
			carryover.add_child("menu_item", menu_item);
		}
		carryover["difficulty"] = cfg["difficulty"];
		//the scenario to be played is always stored as next_scenario in carryover_sides_start
		carryover["next_scenario"] = cfg["scenario"];

		config carryover_start = carryover;

		//get variables according to old hierarchy and copy them to new carryover_sides
		if(!snapshot.empty()){
			if(const config& variables = snapshot.child("variables")){
				carryover.add_child("variables", variables);
				carryover_start.add_child("variables", replay_start.child_or_empty("variables"));
			} else if (const config& variables = cfg.child("variables")){
				carryover.add_child("variables", variables);
				carryover_start.add_child("variables", variables);
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

	bool null_controller = false;
	int max_side_id = 0;
	int next_unit_id = 1;
	int leader_unit_id = 0;
	int offset_unit_id = 0;
	std::vector<config::attribute_value> recruit_lists;
	map_location leader_pos = map_location();
	std::vector<std::pair<int, map_location> > leader_positions;
	if(config& replay = cfg.child("replay_start"))
	{
		// This fix what seems to be an alias/typo is some 1.10 maps (Ggf^Efm is invalid in BFW1.14)
		std::string changes[] = {"Ggf^Efm;Gs^Uf", "Ggf;Gg^Efm"};
		std::ostringstream ss;
		std::size_t found;
		ss << "border_size=1\nusage=map\n\n";
 		for (std::string& line : utils::split(replay["map_data"].str(), '\n'))
 		{
			if (line.empty() || line.find("usage") == 0 || line.find("border_size") == 0)
			{
				//ss << line << "\n";
				continue;
			} else {
				std::string oldmap, newmap;
				for(auto it=std::begin(changes); it!=std::end(changes); ++it)
				{
					int sep = (*it).find(";");
					oldmap = (*it).substr(0, sep);
					newmap = (*it).substr(sep+1);
					std::size_t begin = 0;
					found = line.find(oldmap, begin);
					while(found != std::string::npos)
					{
						line.replace(found, oldmap.length(), newmap);
						begin = found+newmap.length();
						found = line.find(oldmap, begin);
					}
				}
				ss << line << "\n";
			}
		}
		replay["map_data"] = ss.str();

		replay["random_seed"] = cfg["random_seed"];
		replay["random_calls"] = cfg["random_calls"];

		int counter_x = 0, counter_y = 0;
		for (std::string& line : utils::split(replay["map_data"].str(), '\n'))
		{
			if (line.empty() || line.find("usage") == 0 || line.find("border_size") == 0) continue;
			counter_x = 0;
			for (std::string& hex : utils::split(line, ','))
			{
				for (auto it=hex.begin(); it!=hex.end(); it++)
				{
					char** null = 0;
					if (*it == ' ') continue;
					if (int side = strtol(&(*it), null, 10))
					{
						// See map_location::map_location(config): "we want the coordinates as 0-based."
						leader_positions.push_back(std::make_pair(side, map_location(counter_x-1, counter_y-1)));
					}
					break;
				}
				counter_x++;
			}
			counter_y++;
		}

		for(config& side: replay.child_range("side"))
		{
            int side_id = side["side"].to_int();

			if(!side.child_or_empty("unit").empty())
			{
				for (auto it = leader_positions.begin() ; it != leader_positions.end(); ++it)
				{
					if ((*it).first == side_id)
					{
						leader_pos = (*it).second;
						break;
					}
				}
				for(config& unit: side.child_range("unit"))
				{
					if(unit.has_attribute("canrecruit") && unit["canrecruit"] == "yes")
					{
						++leader_unit_id;
						if(!unit.has_attribute("x"))
						{
							unit["x"] = leader_pos.x+1;
							unit["y"] = leader_pos.y+1;
						}
					} else {
						++next_unit_id;
					}
				}
			}

			if(side["controller"] == "null")
			{
				// It is expected to match CotB and SR Yetis
				null_controller = true;
			} else {
				max_side_id = side_id > max_side_id ? side_id : max_side_id;
				recruit_lists.push_back(side["previous_recruits"]);
			}
		}
	}

	// Analysis of events for pre-recruited units in Hornshark Island
	std::vector<std::string > multiplayer_side_recruits;
	std::pair<int, int> pre_recruited_units;
	unsigned int underlying_units = 0, random_calls = 0;
	if(config& replay = cfg.child("replay_start"))
	{
		for(config& event: replay.child_range("event"))
		{
			if(event["name"] == "prestart")
			{
				if(config& fire_event = event.child("fire_event"))
				{
					if(fire_event["name"] == "place_units")
					{
						config& set_variables = event.child("set_variables");

						if(config& values = set_variables.child("value"))
						{
							for(config& multiplayer_side: values.child_range("multiplayer_side"))
							{
								multiplayer_side_recruits.push_back(multiplayer_side["recruit"]);
							}
						}
					}
				}
			} else if(event["name"] == "place_units") {
				for(config& side: replay.child_range("side"))
				{
					if(side["controller"] == "null") continue;

					std::vector<std::string> recruit_list = utils::split(side["previous_recruits"], ',');
					std::sort(recruit_list.begin(), recruit_list.end());

					unsigned int found = 0;
					for (auto it = multiplayer_side_recruits.begin() ; it != multiplayer_side_recruits.end(); ++it)
					{
						LOG_RG <<recruit_list[0] << " in  '" <<*it << "'? --> " << (it->find(recruit_list[0], 0) != std::string::npos) <<"\n";
						if (it->find(recruit_list[0], 0) != std::string::npos) break;
						++found;
					}

					// We use the first switch, assuming side 1 and 2 have same number of pre-recruited units.
					config& switch_ = event.child("switch");
					if(found < switch_.child_count("case"))
					{
						unsigned int count = 0;
						for(config& case_: switch_.child_range("case"))
						{
							if(count < found) { ++count; continue; };
							underlying_units += case_.child_count("unit");
							for(config& unit: case_.child_range("unit"))
							{
								if(unit.has_attribute("id")) continue;
								if(unit_types.find(unit["type"])->race()->id() == "undead") continue;
								//if(unit_types.find(unit["type"])->genders().size() > 1) random_calls += 1;
								random_calls += 12; // Name only
							}
							break;
						}
					} else {
						if(config& else_ = switch_.child("else"))
						{
							underlying_units += else_.child_count("unit");
							for(config& unit: else_.child_range("unit"))
							{
								if(unit.has_attribute("id")) continue;
								if(unit_types.find(unit["type"])->race()->id() == "undead") continue;
								//if(unit_types.find(unit["type"])->genders().size() > 1) random_calls += 1;
								random_calls += 12; // Name only
							}
						}
					}
				}
			}
		}
	}
	pre_recruited_units = std::make_pair(underlying_units, random_calls);


	// Don't count leaders in next_unit_id for first side turn 1 (except CotB and SR)
	if (not null_controller && underlying_units == 0) offset_unit_id = leader_unit_id;
	next_unit_id += leader_unit_id + pre_recruited_units.first;

	// We're always wrong by 1 or 2 on 1.4 saves next_unit_id, so skip it
	version_info loaded_version(cfg["version"]);
	if(loaded_version < version_info("1.7.0")) next_unit_id -= 9999;

	if(config& replay = cfg.child("replay"))
	{
		config new_replay;
		if(config& log = replay.child("upload_log"))
		{
			new_replay["upload_log"] = replay["upload_log"];
		}
		config::attribute_value side;
		int active_side_id = 0;
		int last_attack_seed = cfg["random_seed"];
		int last_attack_calls = 0;
		int last_attack_guessed_calls = 0;
		for(config& command: replay.child_range("command"))
		{
			config random_seed;
			if(config& start = command.child("start"))
			{
				start["sent"] = "yes";
				config& checkup = command.add_child("checkup");
				config& result = checkup.add_child("result");
				if(next_unit_id > 0) result["next_unit_id"] = next_unit_id-offset_unit_id;
				else result["next_unit_id"] = "";
				result["random_calls"] = pre_recruited_units.second;

				if(pre_recruited_units.first > 0)
				{
					random_seed["sent"] = "yes";
					random_seed["dependent"] = "yes";
					random_seed["from_side"] = "server";
					config& rnd_seed = random_seed.add_child("random_seed");
					rnd_seed["new_seed"] = cfg["random_seed"].to_int(42);
				}
			}
			if(config& init_side = command.child("init_side"))
			{
				active_side_id = active_side_id+1 > max_side_id ? 1 : active_side_id+1;
				command["sent"] = "";
				config& checkup = command.add_child("checkup");
				config& result = checkup.add_child("result");
				if(next_unit_id > 0) result["next_unit_id"] = next_unit_id-offset_unit_id;
				else result["next_unit_id"] = "";
				result["random_calls"] = 0;
				init_side["side_number"] = active_side_id;
				offset_unit_id = 0;
			}
			if(config& move = command.child("move"))
			{
				move["skip_sighted"] = "all";
				std::vector<map_location> steps;
				read_locations(move, steps);

				auto it=steps.back();
				config& checkup = command.add_child("checkup");
				config& result = checkup.add_child("result");
				result["final_hex_x"] = it.x+1;
				result["final_hex_y"] = it.y+1;
				result["stopped_early"] = "no";
				config& random_calls = checkup.add_child("result");
				random_calls["random_calls"] = 0;
			}
			if(config& attack = command.child("attack"))
			{
				config& checkup = command.child("checkup");
				//int result_number = checkup.child_count("result");
				//config& result = checkup.child("result", result_number-1);
				random_seed["sent"] = "yes";
				random_seed["dependent"] = "yes";
				random_seed["from_side"] = "server";
				config& rnd_seed = random_seed.add_child("random_seed");
				if(attack.has_attribute("seed"))
				{
					rnd_seed["new_seed"] = attack["seed"];
					last_attack_seed = attack["seed"];
					last_attack_calls = command.child_count("random");
				} else {
					rnd_seed["current_seed"] = last_attack_seed;
					rnd_seed["random_calls"] = last_attack_calls;
					last_attack_calls += command.child_count("random");
				}
				if(last_attack_calls == 0)
				{
					// Make a guess on random_calls based on units strikes
					// This doesn't take into account the berzerk special or death
					// This is only needed if a recruit is coming next
					int counter = 0;
					for(auto it: unit_types.find(attack["attacker_type"])->attacks())
					{
						if(counter == attack["weapon"].to_int(0))
						{
							last_attack_calls = it.num_attacks();
							break;
						}
						++counter;
					}
					if(attack["defender_weapon"] != -1)
					{
						counter = 0;
						for(auto it: unit_types.find(attack["defender_type"])->attacks())
						{
							if(counter == attack["defender_weapon"].to_int(0))
							{
								last_attack_calls += it.num_attacks();
								break;
							}
							++counter;
						}
						const std::string defender = attack["defender_type"];
					}
				}

				if(attack.has_attribute("attacker_type"))
				{
					bool resolve_plague = false; // If no use has plague, just skip it
					const unit_type *p_attacker = unit_types.find(attack["attacker_type"]);
					const unit_type *p_defender = unit_types.find(attack["defender_type"]);
					// Attacker has always an attack
					attack_type *p_killing_blow = &p_attacker->attacks()[attack["weapon"].to_int(0)];
					if(p_killing_blow->specials().has_child("plague")) resolve_plague = true;
					if(attack["defender_weapon"].to_int(-1) > -1)
					{
						attack_type counter_blow = p_defender->attacks()[attack["defender_weapon"].to_int()];
						if(counter_blow.specials().has_child("plague")) resolve_plague = true;
					}

					if(resolve_plague)
					{
						if(command.has_child("random"))
						{
							int counter = 0;
							// Increment the next_unit_id is one unit dies from plague
							for(config& random: command.child_range("random"))
							{
								++counter;
								if(config& results = random.child("results"))
								{
									if(results["dies"].str() != "yes") continue;
									// Assume the attacker kills
									const unit_type *p_dead = p_defender;
									if(attack["defender_weapon"].to_int(-1) > -1)
									{
										// Check that attacker actually kills and update p_killing_blow otherwise
										bool switch_role = false;
										attack_type counter_blow = p_defender->attacks()[attack["defender_weapon"].to_int()];
										if(counter_blow.num_attacks() > 0)
										{
											// (problem here: in results unit_hit is always "defender")
											while(counter > p_killing_blow->num_attacks() + counter_blow.num_attacks())
											{
												// This is a berzerk attack
												counter -= (p_killing_blow->num_attacks() + counter_blow.num_attacks());
											}
											int low_num_attacks = counter_blow.num_attacks();
											if(p_killing_blow->num_attacks() < low_num_attacks) low_num_attacks = p_killing_blow->num_attacks();
											if(counter > 2*low_num_attacks)
											{
												if(p_killing_blow->num_attacks() == low_num_attacks) switch_role = true;
											} else {
												bool first_strike_defender = counter_blow.specials().has_child("firststrike");
												if(first_strike_defender)
												{
													if(counter%2 == 1) switch_role = true;
												} else {
													if(counter%2 == 0) switch_role = true;
												}
											}
										}
										if(switch_role)
										{
											p_dead = p_attacker;
											p_killing_blow = &p_defender->attacks()[attack["defender_weapon"].to_int()];
										}
									}
									// Can't call p_killing_blow->get_special_bool() as it requires display::get_singleton()
									if(p_killing_blow->specials().has_child("plague")
										&& (p_dead->race_id() != "undead")
										&& (p_dead->race_id() != "mechanical"))
									{
										++next_unit_id;
									}
								} else {
									// In 1.8, an arbitrary number of values are allocated, skip unused ones
									break;
								}
							}
						}
					} else {
						// We will never get it right without dies=yes/no information from random
						next_unit_id -= 10000;
					}
				} else {
					next_unit_id -= 10000;
				}
			}
			if(config& choose = command.child("choose"))
			{
				command["sent"] = "yes";
				command["dependent"] = "yes";
				command["from_side"] = active_side_id;
			}
			if(config& recruit = command.child("recruit"))
			{
            	int recruit_list_index = recruit["value"].to_int();
				const int random_calls = command.child_count("random");
				std::vector<std::string> recruit_list = utils::split(recruit_lists[active_side_id-1], ',');
				std::sort(recruit_list.begin(), recruit_list.end());
            	config& new_seed = command.child("random", random_calls-1);
				command["sent"] = "";
				random_seed["sent"] = "yes";
				random_seed["dependent"] = "yes";
				random_seed["from_side"] = "server";
				config& rnd_seed = random_seed.add_child("random_seed");
				//config& seed_checkup = random_seed.add_child("checkup");

				// If no last_attack_seed, we will rely on simulation (no valid seed needed)
				if(last_attack_calls == 0 && last_attack_guessed_calls == 0)
				{
					rnd_seed["new_seed"] = last_attack_seed;
				} else {
					rnd_seed["current_seed"] = last_attack_seed;
					if(last_attack_guessed_calls == 0)
					{
						rnd_seed["random_calls"] = last_attack_calls;
					} else {
						rnd_seed["random_calls"] = last_attack_guessed_calls;
					}
				}

				recruit.remove_attribute("value");
				recruit["type"] = recruit_list[recruit_list_index];

				last_attack_calls += command.child_count("random");
				++next_unit_id;
			}
			new_replay.add_child("command", command);
			if(random_seed.has_child("random_seed"))
			{
				new_replay.add_child("command", random_seed);
			}
		}
		cfg.clear_children("replay");
		cfg.add_child("replay", new_replay);
	}

	// All this is to fix recruit command with leader at (-999,-999)
	if(config& replay = cfg.child("replay"))
	{
		int active_side_id = 0;
		for(config& command: replay.child_range("command"))
		{
			if(config& init_side = command.child("init_side"))
			{
				active_side_id = active_side_id+1 > max_side_id ? 1 : active_side_id+1;
				for (auto it = leader_positions.begin() ; it != leader_positions.end(); ++it)
				{
					if ((*it).first == active_side_id)
					{
						leader_pos = (*it).second;
						break;
					}
				}
			}
			if(config& move = command.child("move"))
			{
				std::vector<map_location> steps;
				read_locations(move, steps);

				for (unsigned i=0 ; i<leader_positions.size(); i++)
				{
					if (leader_positions[i].first == active_side_id)
					{
						if (leader_positions[i].second == steps.front())
						{
							leader_positions[i] = std::make_pair(active_side_id, steps.back());
							leader_pos = leader_positions[i].second;
							break;
						}
					}
				}
			}
			if(config& recruit = command.child("recruit"))
			{
				config& leader = recruit.child("from");
				if(leader["x"].to_int(-1) == -999)
				{
					leader["x"] = leader_pos.x+1;
					leader["y"] = leader_pos.y+1;
				}
			}
		}
	}
	//if replay and snapshot are empty we've got a start of scenario save and don't want replay_start either
	if(replay.empty() && snapshot.empty()){
		LOG_RG<<"removing replay_start \n";
		cfg.remove_child("replay_start", 0);
	}

	//remove empty replay or snapshot so type of save can be detected more easily
	if(replay.empty()){
		LOG_RG<<"removing replay \n";
		cfg.remove_child("replay", 0);
	}

	if(snapshot.empty()){
		LOG_RG<<"removing snapshot \n";
		cfg.remove_child("snapshot", 0);
	}
}

//changes done during 1.11.0-dev
static void convert_old_saves_1_11_0(config& cfg)
{
	config& snapshot = cfg.child("snapshot");
	config& replay_start = cfg.child("replay_start");
	config& replay = cfg.child("replay");

	for(config& side: replay_start.child_range("side"))
	{
		int side_id = side["side"].to_int();
		std::string color[] = {"red", "blue", "green", "purple", "black", "brown", "orange", "white", "teal"};

		if(side.has_attribute("color"))
			if(side["color"].to_int(-1) != -1) side["color"] = color[side["color"].to_int()-1];
		else if(not side.has_attribute("colour")) side["color"] = color[side_id-1];
		else
			if(side["colour"].to_int(-1) == -1) side["color"]  = side["colour"];
			else side["color"] = color[side["colour"].to_int()-1];
	}
	if(config& replay = cfg.child("replay"))
	{
		bool skip_next_random_seed_sync = false;
		config copy_replay;
		for(config& command : replay.child_range("command"))
		{
			if(config& random_seed = command.child("random_seed"))
			{
				if(skip_next_random_seed_sync)
				{
					skip_next_random_seed_sync = false;
					continue;
				}
			}
			if(config& recruit = command.child("recruit"))
			{
				skip_next_random_seed_sync = false;
				int random_calls = 0;
				std::string recruited_unit_race = unit_types.find(recruit["type"])->race_id();
				// Check for races wth no name
				if(!recruited_unit_race.compare(/*pos=*/ 0, /*len=*/ 6, "undead")
					|| !recruited_unit_race.compare(/*pos=*/ 0, /*len=*/ 4, "bats"))
				{
					if(config& checkup = command.child("checkup"))
					{
						for(config& result : checkup.child_range("result"))
						{
							if(result.has_attribute("random_calls"))
							{
								result["random_calls"] = std::to_string(result["random_calls"].to_int()-12);
								random_calls = result["random_calls"].to_int();
								break;
							}
						}
					}
					if(command.child_count("random") >= 12)
					{
						random_calls = command.child_count("random");
						for(size_t i=0; i<12; ++i)
						{
							command.remove_child("random", random_calls-i-1);
						}
						random_calls -= 12;
					}
					if (random_calls == 0) skip_next_random_seed_sync = true; // No actual call to RNG, so don't send next random_seed
				}
			}
			if(config& attack = command.child("attack"))
			{
				if(attack.has_attribute("attacker_type"))
				{
					bool resolve_plague = false; // If no use has plague, just skip it
					const unit_type *p_attacker = unit_types.find(attack["attacker_type"]);
					const unit_type *p_defender = unit_types.find(attack["defender_type"]);
					// Attacker has always an attack
					attack_type *p_killing_blow = &p_attacker->attacks()[attack["weapon"].to_int(0)];
					if(p_killing_blow->specials().has_child("plague")) resolve_plague = true;
					if(attack["defender_weapon"].to_int(-1) > -1)
					{
						attack_type counter_blow = p_defender->attacks()[attack["defender_weapon"].to_int()];
						if(counter_blow.specials().has_child("plague")) resolve_plague = true;
					}

					if(resolve_plague)
					{
						skip_next_random_seed_sync = false;
						if(command.has_child("checkup"))
						{
							config& checkup = command.child("checkup");
							int counter = 0;
							bool plague_kill = false;
							// Remove the 12 for names in case plague on the killing blow
							// FIXME Does not cover the case where the dead unit is on village.
							for(config& result: checkup.child_range("result"))
							{
								if(result.has_attribute("hits")) ++counter;
								if(result.has_attribute("random_calls"))
								{
									if(plague_kill)
									{
										if(result["random_calls"].to_int(0) < 13) break;
										// The problem with village is plague killing ulf on village in 3+ rounds.
										result["random_calls"] = result["random_calls"].to_int()-12;
									}
									break;
								}
								if(result["dies"].str() != "yes") continue;
								// Assume the attacker kills
								const unit_type *p_dead = p_defender;
								if(attack["defender_weapon"].to_int(-1) > -1)
								{
									// Check that attacker actually kills and update p_killing_blow otherwise
									bool switch_role = false;
									attack_type counter_blow = p_defender->attacks()[attack["defender_weapon"].to_int()];
									if(counter_blow.num_attacks() > 0)
									{
										// (problem here: in result unit_hit is always "defender")
										while(counter > p_killing_blow->num_attacks() + counter_blow.num_attacks())
										{
											// This is a berzerk attack
											counter -= (p_killing_blow->num_attacks() + counter_blow.num_attacks());
										}
										int low_num_attacks = counter_blow.num_attacks();
										if(p_killing_blow->num_attacks() < low_num_attacks) low_num_attacks = p_killing_blow->num_attacks();
										if(counter > 2*low_num_attacks)
										{
											if(p_killing_blow->num_attacks() == low_num_attacks) switch_role = true;
										} else {
											bool first_strike_defender = counter_blow.specials().has_child("firststrike");
											if(first_strike_defender)
											{
												if(counter%2 == 1) switch_role = true;
											} else {
												if(counter%2 == 0) switch_role = true;
											}
										}
									}
									if(switch_role)
									{
										p_dead = p_attacker;
										p_killing_blow = &p_defender->attacks()[attack["defender_weapon"].to_int(0)];
									}
								}
								// Can't call p_killing_blow->get_special_bool() as it requires display::get_singleton()
								if(p_killing_blow->specials().has_child("plague")
									&& (p_dead->race_id() != "undead")
									&& (p_dead->race_id() != "mechanical"))
								{
									plague_kill = true;
								}
							}
						}
					}
				}
			}
			copy_replay.add_child("command", command);
		}
		cfg.clear_children("replay");
		cfg.add_child("replay", copy_replay);
	}
	if(!cfg.child_or_empty("replay_start").empty())
	{
		config& edit_replay_start = cfg.child("replay_start");
		edit_replay_start.remove_attribute("playing_team");
		if(edit_replay_start.has_attribute("random_seed"))
		{
			std::stringstream stream;
			stream << std::setfill('0') << std::setw(8) << std::hex << edit_replay_start["random_seed"].to_int();
			edit_replay_start["random_seed"] = stream.str();
		}
		for (config& side : edit_replay_start.child_range("side")) {
			if(side["controller"] == "network")
			{
				side["controller"] = "human";
			}
		}
	}
	if(config& replay = cfg.child("replay"))
	{
		config::attribute_value side;
		for(config& command : replay.child_range("command"))
		{
			if(config& init_side = command.child("init_side"))
			{
				command["sent"] = "yes";
				side = init_side["side_number"];
			}
			if(config& end_turn = command.child("end_turn"))
			{
				command["sent"] = "yes";
				end_turn["next_player_number"] = std::to_string(side.to_int(1)+1);
			}
			if(config& recruit = command.child("recruit"))
			{
				command.clear_children("checkup");
				command["sent"] = "yes";
				command["from_side"] = side;
			}
			if(config& random_seed = command.child("random_seed"))
			{
				std::stringstream stream;
				if(random_seed.has_attribute("current_seed"))
				{
					stream << std::setfill('0') << std::setw(8) << std::hex << random_seed["current_seed"].to_int();
					random_seed["current_seed"] = stream.str();
				} else {
					stream << std::setfill('0') << std::setw(8) << std::hex << random_seed["new_seed"].to_int();
					random_seed["new_seed"] = stream.str();
				}
			}
			if(config& speak = command.child("speak"))
			{
				command["undo"] = "no";
			}
		}
	}

	// Remove empty events
	if(config& replay = cfg.child("replay_start"))
	{
		config copy_event;
		for(config& event: replay.child_range("event"))
		{
			if(not event.empty()) copy_event.add_child("event", event);
		}
		replay.clear_children("event");
		for(config& event: copy_event.child_range("event"))
		{
			replay.add_child("event", event);
		}
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
		if(carryover_sides_start.has_attribute("random_seed"))
		{
			std::stringstream stream;
			stream << std::setfill('0') << std::setw(8) << std::hex << carryover_sides_start["random_seed"].to_int();
			carryover_sides_start["random_seed"] = stream.str();
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
	if(loaded_version < version_info("1.5.0"))
	{
		convert_old_saves_1_03_0_for_BFW1_10(cfg);
		convert_old_saves_1_03_0_for_BFW1_14(cfg);
	}
	if(loaded_version < version_info("1.7.0"))
	{
		convert_old_saves_1_05_0_for_BFW1_10(cfg);
		convert_old_saves_1_05_0_for_BFW1_14(cfg);
	}
	if(loaded_version < version_info("1.9.0"))
	{
		convert_old_saves_1_07_0_for_BFW1_10(cfg);
		convert_old_saves_1_07_0_for_BFW1_14(cfg);
	}
	if(loaded_version < version_info("1.11.0"))
	{
		convert_old_saves_1_09_0(cfg);
		// No hope to be able to load-to-continue
		cfg.remove_attribute("snapshot");
	}
	if(loaded_version < version_info("1.13.0"))
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

	if(not convert_version_to_suffix(cfg).empty())
	{
		config& replay = cfg.child("replay");
		config& replay_start = cfg.child("replay_start");
		update_start_unit_types(replay_start, cfg);
		update_replay_unit_types(replay, cfg);

		update_prestart_detect_factions(cfg);
		update_pre_recruited_loyals(cfg);
	}
	// To initialize {class replay: public rand_rng::rng} with replay version
	// The replay version needs to be passed to rng for RNG backward support
	if (config& replay = cfg.child("replay"))
	{
		replay["version"] = cfg["version"];
	}

	LOG_RG<<"cfg after conversion "<<cfg<<"\n";
}

}
