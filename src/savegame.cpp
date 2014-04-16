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

#ifdef _WIN32
	#ifdef INADDR_ANY
		#undef INADDR_ANY
	#endif
	#ifdef INADDR_BROADCAST
		#undef INADDR_BROADCAST
	#endif
	#ifdef INADDR_NONE
		#undef INADDR_NONE
	#endif

	#include <windows.h>

	/**
	 * conv_ansi_utf8()
	 *   - Convert a string between ANSI encoding (for Windows filename) and UTF-8
	 *  string &name
	 *     - filename to be converted
	 *  bool a2u
	 *     - if true, convert the string from ANSI to UTF-8.
	 *     - if false, reverse. (convert it from UTF-8 to ANSI)
	 */
	void conv_ansi_utf8(std::string &name, bool a2u) {
		int wlen = MultiByteToWideChar(a2u ? CP_ACP : CP_UTF8, 0,
									   name.c_str(), -1, NULL, 0);
		if (wlen == 0) return;
		WCHAR *wc = new WCHAR[wlen];
		if (wc == NULL) return;
		if (MultiByteToWideChar(a2u ? CP_ACP : CP_UTF8, 0, name.c_str(), -1,
								wc, wlen) == 0) {
			delete [] wc;
			return;
		}
		int alen = WideCharToMultiByte(!a2u ? CP_ACP : CP_UTF8, 0, wc, wlen,
									   NULL, 0, NULL, NULL);
		if (alen == 0) {
			delete [] wc;
			return;
		}
		CHAR *ac = new CHAR[alen];
		if (ac == NULL) {
			delete [] wc;
			return;
		}
		WideCharToMultiByte(!a2u ? CP_ACP : CP_UTF8, 0, wc, wlen,
							ac, alen, NULL, NULL);
		delete [] wc;
		if (ac == NULL) {
			return;
		}
		name = ac;
		delete [] ac;

		return;
	}

	void replace_underbar2space(std::string &name) {
		LOG_SAVE << "conv(A2U)-from:[" << name << "]" << std::endl;
		conv_ansi_utf8(name, true);
		LOG_SAVE << "conv(A2U)-to:[" << name << "]" << std::endl;
		LOG_SAVE << "replace_underbar2space-from:[" << name << "]" << std::endl;
		std::replace(name.begin(), name.end(), '_', ' ');
		LOG_SAVE << "replace_underbar2space-to:[" << name << "]" << std::endl;
	}

	void replace_space2underbar(std::string &name) {
		LOG_SAVE << "conv(U2A)-from:[" << name << "]" << std::endl;
		conv_ansi_utf8(name, false);
		LOG_SAVE << "conv(U2A)-to:[" << name << "]" << std::endl;
		LOG_SAVE << "replace_space2underbar-from:[" << name << "]" << std::endl;
		std::replace(name.begin(), name.end(), ' ', '_');
		LOG_SAVE << "replace_space2underbar-to:[" << name << "]" << std::endl;
	}
#else /* ! _WIN32 */
	void replace_underbar2space(std::string &name) {
		std::replace(name.begin(),name.end(),'_',' ');
	}
	void replace_space2underbar(std::string &name) {
		std::replace(name.begin(),name.end(),' ','_');
	}
#endif /* _WIN32 */

namespace savegame {

class save_index_class
{
public:
	void rebuild(const std::string& name) {
		std::string filename = name;
		replace_space2underbar(filename);
		time_t modified = file_create_time(get_saves_dir() + "/" + filename);
		rebuild(name, modified);
	}
	void rebuild(const std::string& name, const time_t& modified) {
		log_scope("load_summary_from_file");
		config& summary = data(name);
		try {
			config full;
			std::string dummy;
			read_save_file(name, full, &dummy);
			::extract_summary_from_config(full, summary);
		} catch(game::load_game_failed&) {
			summary["corrupt"] = true;
		}
		summary["mod_time"] = str_cast(static_cast<int>(modified));
		write_save_index();
	}
	void remove(const std::string& name) {
		config& root = data();
		root.remove_attribute(name);
		write_save_index();
	}
	void set_modified(const std::string& name, const time_t& modified) {
		modified_[name] = modified;
	}
	config& get(const std::string& name) {
		config& result = data(name);
		time_t m = modified_[name];
		config::attribute_value& mod_time = result["mod_time"];
		if (mod_time.empty() || static_cast<time_t>(mod_time.to_int()) != m) {
			rebuild(name, m);
		}
		return result;
	}
public:
	void write_save_index() {
		log_scope("write_save_index()");
		try {
			scoped_ostream stream = ostream_file(get_save_index_file());
			if (preferences::save_compression_format() != compression::NONE) {
				// TODO: maybe allow writing this using bz2 too?
				write_gz(*stream, data());
			} else {
				write(*stream, data());
			}
		} catch(io_exception& e) {
			ERR_SAVE << "error writing to save index file: '" << e.what() << "'\n";
		}
	}

public:
	save_index_class()
		: loaded_(false)
		, data_()
		, modified_()
   {
   }
private:
	config& data(const std::string& name) {
		config& cfg = data();
		if (config& sv = cfg.find_child("save", "save", name)) {
			return sv;
		}

		config& res = cfg.add_child("save");
		res["save"] = name;
		return res;
	}
	config& data() {
		if(loaded_ == false) {
			try {
				scoped_istream stream = istream_file(get_save_index_file());
				try {
					read_gz(data_, *stream);
				} catch (boost::iostreams::gzip_error&) {
					stream->seekg(0);
					read(data_, *stream);
				}
			} catch(io_exception& e) {
				ERR_SAVE << "error reading save index: '" << e.what() << "'\n";
			} catch(config::error&) {
				ERR_SAVE << "error parsing save index config file\n";
				data_.clear();
			}
			loaded_ = true;
		}
		return data_;
	}
private:
	bool loaded_;
	config data_;
	std::map< std::string, time_t > modified_;
} save_index_manager;

class filename_filter {
public:
	filename_filter(const std::string& filter) : filter_(filter) {
	}
	bool operator()(const std::string& filename) const {
		return filename.end() == std::search(filename.begin(), filename.end(),
						     filter_.begin(), filter_.end());
	}
private:
	std::string filter_;
};

class create_save_info {
public:
	create_save_info(const std::string* d = NULL) : dir(d ? *d : get_saves_dir()) {
	}
	save_info operator()(const std::string& filename) const {
		std::string name = filename;
		replace_underbar2space(name);
		time_t modified = file_create_time(dir + "/" + filename);
		save_index_manager.set_modified(name, modified);
		return save_info(name, modified);
	}
	const std::string dir;
};

/** Get a list of available saves. */
std::vector<save_info> get_saves_list(const std::string* dir, const std::string* filter)
{
	create_save_info creator(dir);

	std::vector<std::string> filenames;
	get_files_in_dir(creator.dir,&filenames);

	if (filter) {
		filenames.erase(std::remove_if(filenames.begin(), filenames.end(),
                                               filename_filter(*filter)),
                                filenames.end());
	}

	std::vector<save_info> result;
	std::transform(filenames.begin(), filenames.end(),
		       std::back_inserter(result), creator);
	std::sort(result.begin(),result.end(),save_info_less_time());
	return result;
}


const config& save_info::summary() const {
	return save_index_manager.get(name());
}

std::string save_info::format_time_local() const
{
	char time_buf[256] = {0};
	tm* tm_l = localtime(&modified());
	if (tm_l) {
		const size_t res = strftime(time_buf,sizeof(time_buf),
			(preferences::use_twelve_hour_clock_format() ? _("%a %b %d %I:%M %p %Y") : _("%a %b %d %H:%M %Y")),
			tm_l);
		if(res == 0) {
			time_buf[0] = 0;
		}
	} else {
		LOG_SAVE << "localtime() returned null for time " << this->modified() << ", save " << name();
	}

	return time_buf;
}

std::string save_info::format_time_summary() const
{
	time_t t = modified();
	return util::format_time_summary(t);
}

bool save_info_less_time::operator() (const save_info& a, const save_info& b) const {
	if (a.modified() > b.modified()) {
		return true;
	} else if (a.modified() < b.modified()) {
		return false;
		// Special funky case; for files created in the same second,
		// a replay file sorts less than a non-replay file.  Prevents
		// a timing-dependent bug where it may look like, at the end
		// of a scenario, the replay and the autosave for the next
		// scenario are displayed in the wrong order.
	} else if (a.name().find(_(" replay"))==std::string::npos && b.name().find(_(" replay"))!=std::string::npos) {
		return true;
	} else if (a.name().find(_(" replay"))!=std::string::npos && b.name().find(_(" replay"))==std::string::npos) {
		return false;
	} else {
		return  a.name() > b.name();
	}
}

static std::istream* find_save_file(const std::string &name, const std::string &alt_name, const std::vector<std::string> &suffixes) {
	BOOST_FOREACH(const std::string &suf, suffixes) {
		std::istream *file_stream = istream_file(get_saves_dir() + "/" + name + suf);
		if (file_stream->fail()) {
			delete file_stream;
			file_stream = istream_file(get_saves_dir() + "/" + alt_name + suf);
		}
		if (!file_stream->fail())
			return file_stream;
		else
			delete file_stream;
	}
	LOG_SAVE << "Could not open supplied filename '" << name << "'\n";
	throw game::load_game_failed();
}

void read_save_file(const std::string& name, config& cfg, std::string* error_log)
{
	std::string modified_name = name;
	replace_space2underbar(modified_name);

	static const std::vector<std::string> suffixes = boost::assign::list_of("")(".gz")(".bz2");
	scoped_istream file_stream = find_save_file(modified_name, name, suffixes);

	cfg.clear();
	try{
		/*
		 * Test the modified name, since it might use a .gz
		 * file even when not requested.
		 */
		if(is_gzip_file(modified_name)) {
			read_gz(cfg, *file_stream);
		} else if(is_bzip2_file(modified_name)) {
			read_bz2(cfg, *file_stream);
		} else {
			read(cfg, *file_stream);
		}
	} catch(const std::ios_base::failure& e) {
		LOG_SAVE << e.what();
		if(error_log) {
			*error_log += e.what();
		}
		throw game::load_game_failed();
	} catch(const config::error &err) {
		LOG_SAVE << err.message;
		if(error_log) {
			*error_log += err.message;
		}
		throw game::load_game_failed();
	}

	if(cfg.empty()) {
		LOG_SAVE << "Could not parse file data into config\n";
		throw game::load_game_failed();
	}
}

bool save_game_exists(const std::string& name, compression::format compressed)
{
	std::string fname = name;
	replace_space2underbar(fname);

	fname += compression::format_extension(compressed);

	return file_exists(get_saves_dir() + "/" + fname);
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

void remove_old_auto_saves(const int autosavemax, const int infinite_auto_saves)
{
	const std::string auto_save = _("Auto-Save");
	int countdown = autosavemax;
	if (countdown == infinite_auto_saves)
		return;

	std::vector<save_info> games = get_saves_list(NULL, &auto_save);
	for (std::vector<save_info>::iterator i = games.begin(); i != games.end(); ++i) {
		if (countdown-- <= 0) {
			LOG_SAVE << "Deleting savegame '" << i->name() << "'\n";
			delete_game(i->name());
		}
	}
}

void delete_game(const std::string& name)
{
	std::string modified_name = name;
	replace_space2underbar(modified_name);

	remove((get_saves_dir() + "/" + name).c_str());
	remove((get_saves_dir() + "/" + modified_name).c_str());

	save_index_manager.remove(name);
}

loadgame::loadgame(display& gui, const config& game_config, game_state& gamestate)
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
		throw load_game_cancelled_exception();
	}

	difficulty_ = difficulties[difficulty_dlg.selected_index()];
}


void loadgame::load_game()
{
	show_dialog(false, false);

	if(filename_ != "")
		throw game::load_game_exception(filename_, show_replay_, cancel_orders_, select_difficulty_, difficulty_);
}

void loadgame::load_game(
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
		throw load_game_cancelled_exception();

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
		if ( config & carryover = load_config_.child("carryover_sides_start") )
			carryover["difficulty"] = difficulty_;
	}

	gamestate_.classification().campaign_define = load_config_["campaign_define"].str();
	gamestate_.classification().campaign_type = load_config_["campaign_type"].str();
	gamestate_.classification().campaign_xtra_defines = utils::split(load_config_["campaign_extra_defines"]);
	gamestate_.classification().version = load_config_["version"].str();
	if (config & carryover_sides_start = load_config_.child("carryover_sides_start")) {
		std::string load_config_difficulty = carryover_sides_start["difficulty"];
		gamestate_.classification().difficulty = load_config_difficulty;
	} else {
		gamestate_.classification().difficulty = difficulty_;
	}

	check_version_compatibility();

}

void loadgame::check_version_compatibility()
{
	if (gamestate_.classification().version == game_config::version) {
		return;
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
		throw load_game_cancelled_exception();
	}

	// Even minor version numbers indicate stable releases which are
	// compatible with each other.
	if (wesnoth_version.minor_version() % 2 == 0 &&
	    wesnoth_version.major_version() == save_version.major_version() &&
	    wesnoth_version.minor_version() == save_version.minor_version())
	{
		return;
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
		throw load_game_cancelled_exception();
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
		throw load_game_cancelled_exception();
	}
}

void loadgame::set_gamestate()
{
	gamestate_ = game_state(load_config_, show_replay_);

	// Get the status of the random in the snapshot.
	// For a replay we need to restore the start only, the replaying gets at
	// proper location.
	// For normal loading also restore the call count.
	int seed = load_config_["random_seed"].to_int(42);
	if(seed == 42){
		config cfg = load_config_.child_or_empty("carryover_sides_start");
		seed = cfg["random_seed"].to_int(42);
	}
	unsigned calls = show_replay_ ? 0 : gamestate_.snapshot["random_calls"].to_int();
	carryover_info sides(gamestate_.carryover_sides_start);
	sides.rng().seed_random(seed, calls);
	gamestate_.carryover_sides_start = sides.to_config();
}

void loadgame::load_multiplayer_game()
{
	show_dialog(false, false);

	if (filename_.empty())
		throw load_game_cancelled_exception();

	std::string error_log;
	{
		cursor::setter cur(cursor::WAIT);
		log_scope("load_game");

		read_save_file(filename_, load_config_, &error_log);
		copy_era(load_config_);

		gamestate_ = game_state(load_config_);
	}

	if(!error_log.empty()) {
		gui2::show_error_message(gui_.video(),
				_("The file you have tried to load is corrupt: '") +
				error_log);
		throw load_game_cancelled_exception();
	}

	if(gamestate_.classification().campaign_type != "multiplayer") {
		gui2::show_transient_error_message(gui_.video(), _("This is not a multiplayer save."));
		throw load_game_cancelled_exception();
	}

	check_version_compatibility();
}

void loadgame::fill_mplevel_config(config& level){
	gamestate_.mp_settings().saved_game = true;

	// If we have a start of scenario MP campaign scenario the snapshot
	// is empty the starting position contains the wanted info.
	const config& start_data = !gamestate_.snapshot.empty() ? gamestate_.snapshot : gamestate_.replay_start();

	level.add_child("map", start_data.child_or_empty("map"));
	level["id"] = start_data["id"];
	level["name"] = start_data["name"];
	level["completion"] = start_data["completion"];
	level["next_underlying_unit_id"] = start_data["next_underlying_unit_id"];
	// Probably not needed.
	level["turn"] = start_data["turn_at"];
	level["turn_at"] = start_data["turn_at"];

	level.add_child("multiplayer", gamestate_.mp_settings().to_config());

	//Start-of-scenario save
	if(gamestate_.snapshot.empty()){
		//For a start-of-scenario-save, write the data to the starting_pos and not the snapshot, since
		//there should only be snapshots for midgame reloads
		if (config &c = level.child("replay_start")) {
			c.merge_with(start_data);
		} else {
			level.add_child("replay_start") = start_data;
		}
		level.add_child("snapshot") = config();
	} else {
		level.add_child("snapshot") = start_data;
		level.add_child("replay_start") = gamestate_.replay_start();
	}
	level["random_seed"] = start_data["random_seed"];
	level["random_calls"] = start_data["random_calls"];

	// Adds the replay data, and the replay start, to the level,
	// so clients can receive it.
	level.add_child("replay") = gamestate_.replay_data;
	level.add_child("statistics") = statistics::write_stats();
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

savegame::savegame(game_state& gamestate, const compression::format compress_saves, const std::string& title)
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
	if (is_compressed_file(filename)) {
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
	static std::string parent, grandparent;

	try {
		Uint32 start, end;
		start = SDL_GetTicks();

		if (filename_ == "")
			filename_ = filename;

		before_save();

		// The magic moment that does save threading; after
		// each save, the filename of the save file becomes
		// the parent for the next. *Unless* the parent file
		// has the same name as the savefile, in which case we
		// use the grandparent name. When user loads a savegame,
		// we load its correct parent link along with it.
		if (filename_ == parent) {
			gamestate_.classification().parent = grandparent;
		} else {
			gamestate_.classification().parent = parent;
		}
		LOG_SAVE << "Setting parent of '" << filename_<< "' to " << gamestate_.classification().parent << "\n";

		write_game_to_disk(filename_);
		if (resources::persist != NULL) {
			resources::persist->end_transaction();
			resources::persist ->start_transaction();
		}

		grandparent = parent;
		parent = filename_;

		end = SDL_GetTicks();
		LOG_SAVE << "Milliseconds to save " << filename_ << ": " << end - start << "\n";

		if (video != NULL && show_confirmation_)
			gui2::show_transient_message(*video, _("Saved"), _("The game has been saved."));
		return true;
	} catch(game::save_game_failed& e) {
		ERR_SAVE << error_message_ << e.message;
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
	LOG_SAVE << "savegame::save_game";

	filename_ = filename;
	filename_ += compression::format_extension(compress_saves_);

	std::stringstream ss;
	{
		config_writer out(ss, compress_saves_);
		write_game(out);
		finish_save_game(out);
	}
	scoped_ostream os(open_save_game(filename_));
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

	gamestate_.write_config(out);
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
	} catch(io_exception& e) {
		throw game::save_game_failed(e.what());
	}
}

// Throws game::save_game_failed
scoped_ostream savegame::open_save_game(const std::string &label)
{
	std::string name = label;
	replace_space2underbar(name);

	try {
		return scoped_ostream(ostream_file(get_saves_dir() + "/" + name));
	} catch(io_exception& e) {
		throw game::save_game_failed(e.what());
	}
}

scenariostart_savegame::scenariostart_savegame(game_state &gamestate, const compression::format compress_saves)
	: savegame(gamestate, compress_saves)
{
	set_filename(gamestate.classification().label);
}

void scenariostart_savegame::write_game(config_writer &out){
	savegame::write_game(out);

	out.write_child("carryover_sides_start", gamestate().carryover_sides_start);
}

replay_savegame::replay_savegame(game_state &gamestate, const compression::format compress_saves)
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

	out.write_child("carryover_sides_start", gamestate().carryover_sides_start);
	out.write_child("carryover_sides", gamestate().carryover_sides);
	out.write_child("replay_start", gamestate().replay_start());
	out.write_child("replay", gamestate().replay_data);
}

autosave_savegame::autosave_savegame(game_state &gamestate,
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

oos_savegame::oos_savegame(const config& snapshot_cfg)
	: ingame_savegame(*resources::state_of_game, *resources::screen, snapshot_cfg, preferences::save_compression_format())
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

ingame_savegame::ingame_savegame(game_state &gamestate,
					game_display& gui, const config& snapshot_cfg, const compression::format compress_saves)
	: savegame(gamestate, compress_saves, _("Save Game")),
	gui_(gui)
{
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

void ingame_savegame::before_save()
{
	savegame::before_save();
	gamestate().write_snapshot(snapshot(), &gui_);
}

void ingame_savegame::write_game(config_writer &out) {
	log_scope("write_game");

	savegame::write_game(out);
	out.write_child("snapshot",snapshot());
	out.write_child("replay_start", gamestate().replay_start());
	out.write_child("carryover_sides", gamestate().carryover_sides);
	out.write_child("carryover_sides_start", gamestate().carryover_sides_start);
	out.write_child("replay", gamestate().replay_data);
}

}

