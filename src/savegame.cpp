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

#include <boost/assign/list_of.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include "savegame.hpp"

#include "save_index.hpp"
#include "carryover.hpp"
#include "config_assign.hpp"
#include "game_initialization/configure_engine.hpp"
#include "dialogs.hpp" //FIXME: get rid of this as soon as the two remaining dialogs are moved to gui2
#include "format_time_summary.hpp"
#include "formula_string_utils.hpp"
#include "game_display.hpp"
#include "game_end_exceptions.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "gui/dialogs/game_load.hpp"
#include "gui/dialogs/game_save.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/campaign_difficulty.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "log.hpp"
#include "map.hpp"
#include "map_label.hpp"
#include "persist_manager.hpp"
#include "replay.hpp"
#include "resources.hpp"
#include "serialization/binary_or_text.hpp"
#include "serialization/parser.hpp"
#include "statistics.hpp"
//#include "unit.hpp"
#include "unit_id.hpp"
#include "version.hpp"

#include <boost/foreach.hpp>

static lg::log_domain log_engine("engine");
#define LOG_SAVE LOG_STREAM(info, log_engine)
#define ERR_SAVE LOG_STREAM(err, log_engine)

static lg::log_domain log_enginerefac("enginerefac");
#define LOG_RG LOG_STREAM(info, log_enginerefac)


namespace savegame {

bool save_game_exists(const std::string& name, compression::format compressed)
{
	std::string fname = name;
	replace_space2underbar(fname);

	fname += compression::format_extension(compressed);

	return filesystem::file_exists(filesystem::get_saves_dir() + "/" + fname);
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

loadgame::loadgame(display& gui, const config& game_config, saved_game& gamestate)
	: game_config_(game_config)
	, gui_(gui)
	, gamestate_(gamestate)
	, filename_()
	, difficulty_()
	, load_config_()
	, show_replay_(false)
	, cancel_orders_(false)
	, select_difficulty_(false)
{}

void loadgame::show_dialog(bool show_replay, bool cancel_orders)
{
	//FIXME: Integrate the load_game dialog into this class
	//something to watch for the curious, but not yet ready to go
	if (gui2::new_widgets){
		gui2::tgame_load load_dialog(game_config_);
		load_dialog.show(gui_.video());

		if (load_dialog.get_retval() == gui2::twindow::OK) {
			select_difficulty_ = load_dialog.change_difficulty();

			filename_ = load_dialog.filename();
			show_replay_ = load_dialog.show_replay();
			cancel_orders_ = load_dialog.cancel_orders();
		}
	} else {
		bool show_replay_dialog = show_replay;
		bool cancel_orders_dialog = cancel_orders;
		filename_ = dialogs::load_game_dialog(gui_, game_config_, &select_difficulty_, &show_replay_dialog, &cancel_orders_dialog);
		show_replay_ = show_replay_dialog;
		cancel_orders_ = cancel_orders_dialog;
	}
}

void loadgame::show_difficulty_dialog()
{
	create_save_info creator;
	save_info info = creator(filename_);
	const config& cfg_summary = info.summary();

	if ( cfg_summary["corrupt"].to_bool() || (cfg_summary["replay"].to_bool() && !cfg_summary["snapshot"].to_bool(true))
		|| (!cfg_summary["turn"].empty()) )
		return;

	const config::const_child_itors &campaigns = game_config_.child_range("campaign");
	std::vector<std::string> difficulty_descriptions;
	std::vector<std::string> difficulties;
	BOOST_FOREACH(const config &campaign, campaigns)
	{
		if (campaign["id"] == cfg_summary["campaign"]) {
			difficulty_descriptions = utils::split(campaign["difficulty_descriptions"], ';');
			difficulties = utils::split(campaign["difficulties"], ',');

			break;
		}
	}

	if (difficulty_descriptions.empty())
		return;

#if 0
	int default_difficulty = -1;
	for (size_t i = 0; i < difficulties.size(); i++) {
		if (difficulties[i] == cfg_summary["difficulty"]) {
			default_difficulty = i;
			break;
		}
	}
#endif

	gui2::tcampaign_difficulty difficulty_dlg(difficulty_descriptions);
	difficulty_dlg.show(gui_.video());

	if (difficulty_dlg.get_retval() != gui2::twindow::OK) {
		return;
	}

	difficulty_ = difficulties[difficulty_dlg.selected_index()];
}

void loadgame::load_game()
{
	show_dialog(false, false);

	if(filename_ != "")
		throw game::load_game_exception(filename_, show_replay_, cancel_orders_, select_difficulty_, difficulty_);
}

bool loadgame::load_game(
		  const std::string& filename
		, const bool show_replay
		, const bool cancel_orders
		, const bool select_difficulty
		, const std::string& difficulty)
{
	filename_ = filename;
	difficulty_ = difficulty;
	select_difficulty_ = select_difficulty;

	if (filename_.empty()){
		show_dialog(show_replay, cancel_orders);
	}
	else{
		show_replay_ = show_replay;
		cancel_orders_ = cancel_orders;
	}

	if (filename_.empty())
		return false;

	if (select_difficulty_)
		show_difficulty_dialog();

	std::string error_log;
	read_save_file(filename_, load_config_, &error_log);

	convert_old_saves(load_config_);

	if(!error_log.empty()) {
        try {
		    gui2::show_error_message(gui_.video(),
				    _("Warning: The file you have tried to load is corrupt. Loading anyway.\n") +
				    error_log);
        } catch (utf8::invalid_utf8_exception&) {
		    gui2::show_error_message(gui_.video(),
				    _("Warning: The file you have tried to load is corrupt. Loading anyway.\n") +
                    std::string("(UTF-8 ERROR)"));
        }
	}

	if (!difficulty_.empty()){
		load_config_["difficulty"] = difficulty_;
	}
#if 0
	gamestate_.classification().campaign_define = load_config_["campaign_define"].str();
	gamestate_.classification().campaign_type = lexical_cast_default<game_classification::CAMPAIGN_TYPE> (load_config_["campaign_type"].str(), game_classification::SCENARIO);
	gamestate_.classification().campaign_xtra_defines = utils::split(load_config_["campaign_extra_defines"]);
	gamestate_.classification().version = load_config_["version"].str();
	gamestate_.classification().difficulty = load_config_["difficulty"].str();
#else
	// read classification to for loading the game_config config object.
	gamestate_.classification() = game_classification(load_config_);
#endif

	return check_version_compatibility();
}

bool loadgame::check_version_compatibility()
{
	if (gamestate_.classification().version == game_config::version) {
		return true;
	}

	const version_info save_version = gamestate_.classification().version;
	const version_info &wesnoth_version = game_config::wesnoth_version;
	// If the version isn't good, it probably isn't a compatible stable one,
	// and the following comparisons would throw.
	if (!save_version.good()) {
		const std::string message = _("The save has corrupt version information ($version_number|) and cannot be loaded.");
		utils::string_map symbols;
		symbols["version_number"] = gamestate_.classification().version;
		gui2::show_error_message(gui_.video(), utils::interpolate_variables_into_string(message, &symbols));
		return false;
	}

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
		gui2::show_error_message(gui_.video(), utils::interpolate_variables_into_string(message, &symbols));
		return false;
	}

	int res = gui2::twindow::OK;
	if(preferences::confirm_load_save_from_different_version()) {
		const std::string message = _("This save is from a different version of the game ($version_number|). Do you wish to try to load it?");
		utils::string_map symbols;
		symbols["version_number"] = save_version.str();
		res = gui2::show_message(gui_.video(), _("Load Game"), utils::interpolate_variables_into_string(message, &symbols),
			gui2::tmessage::yes_no_buttons);
	}

	if(res == gui2::twindow::CANCEL) {
		return false;
	}

	return true;
}

void loadgame::set_gamestate()
{
	gamestate_ = saved_game(load_config_);
}

bool loadgame::load_multiplayer_game()
{
	show_dialog(false, false);

	if (filename_.empty())
		return false;

	std::string error_log;
	{
		cursor::setter cur(cursor::WAIT);
		log_scope("load_game");

		read_save_file(filename_, load_config_, &error_log);
		copy_era(load_config_);

		gamestate_ = saved_game(load_config_);
	}

	if(!error_log.empty()) {
		gui2::show_error_message(gui_.video(),
				_("The file you have tried to load is corrupt: '") +
				error_log);
		return false;
	}

	if(gamestate_.classification().campaign_type != game_classification::MULTIPLAYER) {
		gui2::show_transient_error_message(gui_.video(), _("This is not a multiplayer save."));
		return false;
	}

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
	: gamestate_(gamestate)
	, snapshot_()
	, filename_()
	, title_(title)
	, error_message_(_("The game could not be saved: "))
	, show_confirmation_(false)
	, compress_saves_(compress_saves)
{}

bool savegame::save_game_automatic(CVideo& video, bool ask_for_overwrite, const std::string& filename)
{
	if (filename == "")
		create_filename();
	else
		filename_ = filename;

	if (ask_for_overwrite){
		if (!check_overwrite(video)) {
			return save_game_interactive(video, "", gui::OK_CANCEL);
		}
	}

	return save_game(&video);
}

bool savegame::save_game_interactive(CVideo& video, const std::string& message,
									 gui::DIALOG_TYPE dialog_type)
{
	show_confirmation_ = true;
	create_filename();

	int res = gui2::twindow::OK;
	bool exit = true;

	do{
		try{
			res = show_save_dialog(video, message, dialog_type);
			exit = true;

			if (res == gui2::twindow::OK){
				exit = check_overwrite(video);
			}
		}
		catch (illegal_filename_exception){
			exit = false;
		}
	}
	while (!exit);

	if (res == 2) //Quit game
		throw end_level_exception(QUIT);

	if (res != gui2::twindow::OK)
		return false;

	return save_game(&video);
}

int savegame::show_save_dialog(CVideo& video, const std::string& message, const gui::DIALOG_TYPE dialog_type)
{
	int res = 0;

	std::string filename = filename_;

	if (dialog_type == gui::OK_CANCEL){
		gui2::tgame_save dlg(filename, title_);
		dlg.show(video);
		res = dlg.get_retval();
	}
	else if (dialog_type == gui::YES_NO){
		gui2::tgame_save_message dlg(filename, title_, message);
		dlg.show(video);
		res = dlg.get_retval();
	}

	check_filename(filename, video);
	set_filename(filename);

	return res;
}

bool savegame::check_overwrite(CVideo& video)
{
	std::string filename = filename_;
	if (save_game_exists(filename, compress_saves_)) {
		std::stringstream message;
		message << _("Save already exists. Do you want to overwrite it?") << "\n" << _("Name: ") << filename;
		int retval = gui2::show_message(video, _("Overwrite?"), message.str(), gui2::tmessage::yes_no_buttons);
		return retval == gui2::twindow::OK;
	} else {
		return true;
	}
}

void savegame::check_filename(const std::string& filename, CVideo& video)
{
	if (filesystem::is_compressed_file(filename)) {
		gui2::show_error_message(video, _("Save names should not end on '.gz' or '.bz2'. "
			"Please remove the extension."));
		throw illegal_filename_exception();
	}
}

bool savegame::is_illegal_file_char(char c)
{
	return c == '/' || c == '\\' || c == ':'
#ifdef _WIN32
	|| c == '?' || c == '|' || c == '<' || c == '>' || c == '*' || c == '"'
#endif
	;
}

void savegame::set_filename(std::string filename)
{
	filename.erase(std::remove_if(filename.begin(), filename.end(),
	            is_illegal_file_char), filename.end());
	filename_ = filename;
}

void savegame::before_save()
{
	gamestate_.replay_data = recorder.get_replay_data();
}

bool savegame::save_game(CVideo* video, const std::string& filename)
{

	try {
		Uint32 start, end;
		start = SDL_GetTicks();

		if (filename_ == "")
			filename_ = filename;

		before_save();

		write_game_to_disk(filename_);
		if (resources::persist != NULL) {
			resources::persist->end_transaction();
			resources::persist ->start_transaction();
		}

		end = SDL_GetTicks();
		LOG_SAVE << "Milliseconds to save " << filename_ << ": " << end - start << std::endl;

		if (video != NULL && show_confirmation_)
			gui2::show_transient_message(*video, _("Saved"), _("The game has been saved."));
		return true;
	} catch(game::save_game_failed& e) {
		ERR_SAVE << error_message_ << e.message << std::endl;
		if (video != NULL){
			gui2::show_error_message(*video, error_message_ + e.message);
			//do not bother retrying, since the user can just try to save the game again
			//maybe show a yes-no dialog for "disable autosaves now"?
		}

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
	out.write_key_val("next_underlying_unit_id", lexical_cast<std::string>(n_unit::id_manager::instance().get_save_id()));

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
	std::string name = label;
	replace_space2underbar(name);

	try {
		return filesystem::scoped_ostream(filesystem::ostream_file(filesystem::get_saves_dir() + "/" + name));
	} catch(filesystem::io_exception& e) {
		throw game::save_game_failed(e.what());
	}
}

scenariostart_savegame::scenariostart_savegame(saved_game &gamestate, const compression::format compress_saves)
	: savegame(gamestate, compress_saves)
{
	set_filename(gamestate.classification().label);
}

void scenariostart_savegame::write_game(config_writer &out){
	savegame::write_game(out);
	gamestate().write_carryover(out);
}

replay_savegame::replay_savegame(saved_game &gamestate, const compression::format compress_saves)
	: savegame(gamestate, compress_saves, _("Save Replay"))
{}

void replay_savegame::create_filename()
{
	std::stringstream stream;

	const std::string ellipsed_name = font::make_text_ellipsis(gamestate().classification().label,
			font::SIZE_NORMAL, 200);
	stream << ellipsed_name << " " << _("replay");

	set_filename(stream.str());
}

void replay_savegame::write_game(config_writer &out) {
	savegame::write_game(out);

	gamestate().write_carryover(out);
	out.write_child("replay_start", gamestate().replay_start());
	out.write_child("replay", gamestate().replay_data);

}

autosave_savegame::autosave_savegame(saved_game &gamestate,
					game_display& gui, const config& snapshot_cfg, const compression::format compress_saves)
	: ingame_savegame(gamestate, gui, snapshot_cfg, compress_saves)
{
	set_error_message(_("Could not auto save the game. Please save the game manually."));
}

void autosave_savegame::autosave(const bool disable_autosave, const int autosave_max, const int infinite_autosaves)
{
	if(disable_autosave)
		return;

	save_game_automatic(gui_.video());

	remove_old_auto_saves(autosave_max, infinite_autosaves);
}

void autosave_savegame::create_filename()
{
	std::string filename;
	if (gamestate().classification().label.empty())
		filename = _("Auto-Save");
	else
		filename = gamestate().classification().label + "-" + _("Auto-Save") + snapshot()["turn_at"];

	set_filename(filename);
}

oos_savegame::oos_savegame(saved_game& gamestate, game_display& gui, const config& snapshot_cfg)
	: ingame_savegame(gamestate, gui, snapshot_cfg, preferences::save_compression_format())
{}

int oos_savegame::show_save_dialog(CVideo& video, const std::string& message, const gui::DIALOG_TYPE /*dialog_type*/)
{
	static bool ignore_all = false;
	int res = 0;

	std::string filename = this->filename();

	if (!ignore_all){
		gui2::tgame_save_oos dlg(ignore_all, filename, title(), message);
		dlg.show(video);
		res = dlg.get_retval();
	}

	check_filename(filename, video);
	set_filename(filename);

	return res;
}

ingame_savegame::ingame_savegame(saved_game &gamestate,
					game_display& gui, const config& snapshot_cfg, const compression::format compress_saves)
	: savegame(gamestate, compress_saves, _("Save Game")),
	gui_(gui)
{
	gamestate.set_snapshot(snapshot_cfg);
	snapshot().merge_with(snapshot_cfg);
}

void ingame_savegame::create_filename()
{
	std::stringstream stream;

	const std::string ellipsed_name = font::make_text_ellipsis(gamestate().classification().label,
			font::SIZE_NORMAL, 200);
	stream << ellipsed_name << " " << _("Turn") << " " << snapshot()["turn_at"];
	set_filename(stream.str());
}


void ingame_savegame::write_game(config_writer &out) {
	log_scope("write_game");

	savegame::write_game(out);

	gamestate().write_carryover(out);
	out.write_child("snapshot",snapshot());
	out.write_child("replay_start", gamestate().replay_start());
	out.write_child("replay", gamestate().replay_data);
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
		BOOST_FOREACH(const config& menu_item, cfg.child_range("menu_item")){
			carryover.add_child("menu_item", menu_item);
		}
		carryover["difficulty"] = cfg["difficulty"];
		carryover["random_mode"] = cfg["random_mode"];
		//the scenario to be played is always stored as next_scenario in carryover_sides_start
		carryover["next_scenario"] = cfg["scenario"];

		config carryover_start = carryover;

		//copy sides from either snapshot or replay_start to new carryover_sides
		if(!snapshot.empty()){
			BOOST_FOREACH(const config& side, snapshot.child_range("side")){
				carryover.add_child("side", side);
			}
			//for compatibility with old savegames that use player instead of side
			BOOST_FOREACH(const config& side, snapshot.child_range("player")){
				carryover.add_child("side", side);
			}
			//save the sides from replay_start in carryover_sides_start
			BOOST_FOREACH(const config& side, replay_start.child_range("side")){
				carryover_start.add_child("side", side);
			}
			//for compatibility with old savegames that use player instead of side
			BOOST_FOREACH(const config& side, replay_start.child_range("player")){
				carryover_start.add_child("side", side);
			}
		} else if (!replay_start.empty()){
			BOOST_FOREACH(const config& side, replay_start.child_range("side")){
				carryover.add_child("side", side);
				carryover_start.add_child("side", side);
			}
			//for compatibility with old savegames that use player instead of side
			BOOST_FOREACH(const config& side, replay_start.child_range("player")){
				carryover.add_child("side", side);
				carryover_start.add_child("side", side);
			}
		}

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

	if(config& snapshot = cfg.child("snapshot"))
	{
		//make [end_level] -> [end_level_data] since its alo called [end_level_data] in the carryover.
		if(config& end_level = cfg.child("end_level") )
		{
			snapshot.add_child("end_level_data", end_level);
			snapshot.remove_child("end_level", 0);
		}
		//if we have a snapshot then we already applied carryover so there is no reason to keep this data.
		if(cfg.has_child("carryover_sides_start"))
		{
			cfg.remove_child("carryover_sides_start", 0);
		}
	}
#if 1
	//This code is needed becasue for example otherwise it won't find the (empty) era 
	if(!cfg.has_child("multiplayer")) {
		cfg.add_child("multiplayer", config_of
			("mp_era", "era_blank")
			("show_connect", false)
			("show_configure", false)
			("mp_use_map_settings", true)
		);
	}
	//the alternative code down below doesnt work replay saves or start of scenario saves 
	//becasue those don't contain a snaphot. If the code below works with we can enable that code
	//If it turns out that this code works well we can delete that code.
#else

	if(!cfg.has_child("multiplayer") && cfg["campaign_type"] == "scenario") {
		saved_game tmp(cfg);
		ng::configure_engine eng(tmp);
		eng.set_default_values();
		tmp.mp_settings().mp_era = "era_blank";
		cfg.add_child("multiplayer", tmp.mp_settings().to_config());
	}
#endif
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
	LOG_RG<<"cfg after conversion "<<cfg<<"\n";
}

}

