/* $Id$ */
/*
   Copyright (C) 2003 - 2011 by Jörg Hinrichs, refactored from various
   places formerly created by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "savegame.hpp"

#include "dialogs.hpp" //FIXME: get rid of this as soon as the two remaining dialogs are moved to gui2
#include "foreach.hpp"
#include "game_display.hpp"
#include "game_end_exceptions.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "gui/dialogs/game_load.hpp"
#include "gui/dialogs/game_save.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "log.hpp"
#include "map.hpp"
#include "map_label.hpp"
#include "replay.hpp"
#include "resources.hpp"
#include "serialization/binary_or_text.hpp"
#include "serialization/parser.hpp"
#include "statistics.hpp"
//#include "unit.hpp"
#include "unit_id.hpp"
#include "version.hpp"

static lg::log_domain log_engine("engine");
#define LOG_SAVE LOG_STREAM(info, log_engine)
#define ERR_SAVE LOG_STREAM(err, log_engine)

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
		LOG_SAVE << "replace_underbar2space-from:[" << name << "]" << std::endl;
		std::replace(name.begin(), name.end(), ' ', '_');
		LOG_SAVE << "replace_underbar2space-to:[" << name << "]" << std::endl;
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

const std::string save_info::format_time_local() const{
	char time_buf[256] = {0};
	tm* tm_l = localtime(&time_modified);
	if (tm_l) {
		const size_t res = strftime(time_buf,sizeof(time_buf),_("%a %b %d %H:%M %Y"),tm_l);
		if(res == 0) {
			time_buf[0] = 0;
		}
	} else {
		LOG_SAVE << "localtime() returned null for time " << time_modified << ", save " << name;
	}

	return time_buf;
}

const std::string save_info::format_time_summary() const
{
	time_t t = time_modified;
	time_t curtime = time(NULL);
	const struct tm* timeptr = localtime(&curtime);
	if(timeptr == NULL) {
		return "";
	}

	const struct tm current_time = *timeptr;

	timeptr = localtime(&t);
	if(timeptr == NULL) {
		return "";
	}

	const struct tm save_time = *timeptr;

	const char* format_string = _("%b %d %y");

	if(current_time.tm_year == save_time.tm_year) {
		const int days_apart = current_time.tm_yday - save_time.tm_yday;
		if(days_apart == 0) {
			// save is from today
			format_string = _("%H:%M");
		} else if(days_apart > 0 && days_apart <= current_time.tm_wday) {
			// save is from this week
			format_string = _("%A, %H:%M");
		} else {
			// save is from current year
			format_string = _("%b %d");
		}
	} else {
		// save is from a different year
		format_string = _("%b %d %y");
	}

	char buf[40];
	const size_t res = strftime(buf,sizeof(buf),format_string,&save_time);
	if(res == 0) {
		buf[0] = 0;
	}

	return buf;
}

/**
 * A structure for comparing to save_info objects based on their modified time.
 * If the times are equal, will order based on the name.
 */
struct save_info_less_time {
	bool operator()(const save_info& a, const save_info& b) const {
		if (a.time_modified > b.time_modified) {
		        return true;
		} else if (a.time_modified < b.time_modified) {
			return false;
		// Special funky case; for files created in the same second,
		// a replay file sorts less than a non-replay file.  Prevents
		// a timing-dependent bug where it may look like, at the end
		// of a scenario, the replay and the autosave for the next
		// scenario are displayed in the wrong order.
		} else if (a.name.find(_(" replay"))==std::string::npos && b.name.find(_(" replay"))!=std::string::npos) {
			return true;
		} else if (a.name.find(_(" replay"))!=std::string::npos && b.name.find(_(" replay"))==std::string::npos) {
			return false;
		} else {
			return  a.name > b.name;
		}
	}
};

void manager::read_save_file(const std::string& name, config& cfg, std::string* error_log)
{
	std::string modified_name = name;
	replace_space2underbar(modified_name);

	// Try reading the file both with and without underscores
	scoped_istream file_stream = istream_file(get_saves_dir() + "/" + modified_name);
	if (file_stream->fail())
		file_stream = istream_file(get_saves_dir() + "/" + name);

	cfg.clear();
	try{
		if(is_gzip_file(name)) {
			read_gz(cfg, *file_stream, error_log);
		} else {
			detect_format_and_read(cfg, *file_stream, error_log);
		}
	} catch (config::error &err)
	{
		LOG_SAVE << err.message;
		throw game::load_game_failed();
	}

	if(cfg.empty()) {
		LOG_SAVE << "Could not parse file data into config\n";
		throw game::load_game_failed();
	}
}

void manager::load_summary(const std::string& name, config& cfg_summary, std::string* error_log){
	log_scope("load_game_summary");

	config cfg;
	read_save_file(name,cfg,error_log);

	::extract_summary_from_config(cfg, cfg_summary);
}

bool manager::save_game_exists(const std::string& name, const bool compress_saves)
{
	std::string fname = name;
	replace_space2underbar(fname);

	if(compress_saves) {
		fname += ".gz";
	}

	return file_exists(get_saves_dir() + "/" + fname);
}

std::vector<save_info> manager::get_saves_list(const std::string *dir, const std::string* filter)
{
	// Don't use a reference, it seems to break on arklinux with GCC-4.3.
	const std::string saves_dir = (dir) ? *dir : get_saves_dir();

	std::vector<std::string> saves;
	get_files_in_dir(saves_dir,&saves);

	std::vector<save_info> res;
	for(std::vector<std::string>::iterator i = saves.begin(); i != saves.end(); ++i) {
		if(filter && std::search(i->begin(), i->end(), filter->begin(), filter->end()) == i->end()) {
			continue;
		}

		const time_t modified = file_create_time(saves_dir + "/" + *i);

		replace_underbar2space(*i);
		res.push_back(save_info(*i,modified));
	}

	std::sort(res.begin(),res.end(),save_info_less_time());

	return res;
}

void manager::clean_saves(const std::string &label)
{
	std::vector<save_info> games = get_saves_list();
	std::string prefix = label + "-" + _("Auto-Save");
	std::cerr << "Cleaning saves with prefix '" << prefix << "'\n";
	for (std::vector<save_info>::iterator i = games.begin(); i != games.end(); ++i) {
		if (i->name.compare(0, prefix.length(), prefix) == 0) {
			std::cerr << "Deleting savegame '" << i->name << "'\n";
			delete_game(i->name);
		}
	}
}

void manager::remove_old_auto_saves(const int autosavemax, const int infinite_auto_saves)
{
	const std::string auto_save = _("Auto-Save");
	int countdown = autosavemax;
	if (countdown == infinite_auto_saves)
		return;

	std::vector<save_info> games = get_saves_list(NULL, &auto_save);
	for (std::vector<save_info>::iterator i = games.begin(); i != games.end(); ++i) {
		if (countdown-- <= 0) {
			LOG_SAVE << "Deleting savegame '" << i->name << "'\n";
			delete_game(i->name);
		}
	}
}

void manager::delete_game(const std::string& name)
{
	std::string modified_name = name;
	replace_space2underbar(modified_name);

	remove((get_saves_dir() + "/" + name).c_str());
	remove((get_saves_dir() + "/" + modified_name).c_str());
}

bool save_index::save_index_loaded = false;
config save_index::save_index_cfg;

config& save_index::load()
{
	if(save_index_loaded == false) {
		try {
			scoped_istream stream = istream_file(get_save_index_file());
			detect_format_and_read(save_index_cfg, *stream);
		} catch(io_exception& e) {
			ERR_SAVE << "error reading save index: '" << e.what() << "'\n";
		} catch(config::error&) {
			ERR_SAVE << "error parsing save index config file\n";
			save_index_cfg.clear();
		}

		save_index_loaded = true;
	}

	return save_index_cfg;
}

config& save_index::save_summary(std::string save)
{
	/*
	 * All saves are .gz files now so make sure we use that name when opening
	 * a file. If not some parts of the code use the name with and some parts
	 * without the .gz suffix.
	 */
	if(save.length() < 3 || save.substr(save.length() - 3) != ".gz") {
		save += ".gz";
	}

	config& cfg = load();
	if (config &sv = cfg.find_child("save", "save", save))
		return sv;

	config &res = cfg.add_child("save");
	res["save"] = save;
	return res;
}

void save_index::write_save_index()
{
	log_scope("write_save_index()");
	try {
		scoped_ostream stream = ostream_file(get_save_index_file());
		write(*stream, load());
	} catch(io_exception& e) {
		ERR_SAVE << "error writing to save index file: '" << e.what() << "'\n";
	}
}

loadgame::loadgame(display& gui, const config& game_config, game_state& gamestate)
	: game_config_(game_config)
	, gui_(gui)
	, gamestate_(gamestate)
	, filename_()
	, load_config_()
	, show_replay_(false)
	, cancel_orders_(false)
{}

void loadgame::show_dialog(bool show_replay, bool cancel_orders)
{
	//FIXME: Integrate the load_game dialog into this class
	//something to watch for the curious, but not yet ready to go
	if (gui2::new_widgets){
		gui2::tgame_load load_dialog(game_config_);
		load_dialog.show(gui_.video());

		if (load_dialog.get_retval() == gui2::twindow::OK){
			filename_ = load_dialog.filename();
			show_replay_ = load_dialog.show_replay();
			cancel_orders_ = load_dialog.cancel_orders();
		}
	}
	else
	{
		bool show_replay_dialog = show_replay;
		bool cancel_orders_dialog = cancel_orders;
		filename_ = dialogs::load_game_dialog(gui_, game_config_, &show_replay_dialog, &cancel_orders_dialog);
		show_replay_ = show_replay_dialog;
		cancel_orders_ = cancel_orders_dialog;
	}
}

void loadgame::load_game()
{
	show_dialog(false, false);

	if(filename_ != "")
		throw game::load_game_exception(filename_, show_replay_, cancel_orders_);
}

void loadgame::load_game(std::string& filename, bool show_replay, bool cancel_orders)
{
	filename_ = filename;

	if (filename_.empty()){
		show_dialog(show_replay, cancel_orders);
	}
	else{
		show_replay_ = show_replay;
		cancel_orders_ = cancel_orders;
	}

	if (filename_.empty())
		throw load_game_cancelled_exception();

	std::string error_log;
	manager::read_save_file(filename_, load_config_, &error_log);

	if(!error_log.empty()) {
        try {
		    gui2::show_error_message(gui_.video(),
				    _("Warning: The file you have tried to load is corrupt. Loading anyway.\n") +
				    error_log);
        } catch (utils::invalid_utf8_exception&) {
		    gui2::show_error_message(gui_.video(),
				    _("Warning: The file you have tried to load is corrupt. Loading anyway.\n") +
                    std::string("(UTF-8 ERROR)"));
        }
	}

	gamestate_.classification().difficulty = load_config_["difficulty"];
	gamestate_.classification().campaign_define = load_config_["campaign_define"];
	gamestate_.classification().campaign_type = load_config_["campaign_type"];
	gamestate_.classification().campaign_xtra_defines = utils::split(load_config_["campaign_extra_defines"]);
	gamestate_.classification().version = load_config_["version"];

	check_version_compatibility();

}

void loadgame::check_version_compatibility()
{
	if (gamestate_.classification().version == game_config::version) {
		return;
	}

	const version_info save_version = gamestate_.classification().version;
	const version_info &wesnoth_version = game_config::wesnoth_version;
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
		gui2::show_message(gui_.video(), "", _("This save is from a version too old to be loaded."));
		throw load_game_cancelled_exception();
	}

	const int res = gui2::show_message(gui_.video(), "", _("This save is from a different version of the game. Do you want to try to load it?"),
		gui2::tmessage::yes_no_buttons);

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
	const int seed = lexical_cast_default<int>
		(load_config_["random_seed"], 42);
	const unsigned calls = show_replay_ ? 0 :
		lexical_cast_default<unsigned> (gamestate_.snapshot["random_calls"]);
	gamestate_.rng().seed_random(seed, calls);
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

		manager::read_save_file(filename_, load_config_, &error_log);
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
		gui2::show_message(gui_.video(), "", _("This is not a multiplayer save"));
		throw load_game_cancelled_exception();
	}

	check_version_compatibility();
}

void loadgame::fill_mplevel_config(config& level){
	gamestate_.mp_settings().saved_game = true;

	// If we have a start of scenario MP campaign scenario the snapshot
	// is empty the starting position contains the wanted info.
	const config& start_data = !gamestate_.snapshot.empty() ? gamestate_.snapshot : gamestate_.starting_pos;
	level["map_data"] = start_data["map_data"];
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
		level.add_child("replay_start") = gamestate_.starting_pos;
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

savegame::savegame(game_state& gamestate, const bool compress_saves, const std::string& title)
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
	bool overwrite = true;

	if (filename == "")
		create_filename();
	else
		filename_ = filename;

	if (ask_for_overwrite){
		overwrite = check_overwrite(video);

		if (!overwrite)
			return save_game_interactive(video, "", gui::OK_CANCEL);
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
		gui2::tgame_save dlg(title_, filename);
		dlg.show(video);
		filename = dlg.filename();
		res = dlg.get_retval();
	}
	else if (dialog_type == gui::YES_NO){
		gui2::tgame_save_message dlg(title_, filename, message);
		dlg.show(video);
		filename = dlg.filename();
		res = dlg.get_retval();
	}

	check_filename(filename, video);
	set_filename(filename);

	return res;
}

bool savegame::check_overwrite(CVideo& video)
{
	std::string filename = filename_;
	if (manager::save_game_exists(filename, compress_saves_)) {
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
	if (is_gzip_file(filename)) {
		gui2::show_error_message(video, _("Save names should not end on '.gz'. "
			"Please choose a different name."));
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

		grandparent = parent;
		parent = filename_;

		end = SDL_GetTicks();
		LOG_SAVE << "Milliseconds to save " << filename_ << ": " << end - start << "\n";

		if (video != NULL && show_confirmation_)
			gui2::show_message(*video, _("Saved"), _("The game has been saved"));
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

	if (compress_saves_) {
		filename_ += ".gz";
	}

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

void savegame::write_game(config_writer &out) const
{
	log_scope("write_game");

	out.write_key_val("version", game_config::version);
	out.write_key_val("next_underlying_unit_id", lexical_cast<std::string>(n_unit::id_manager::instance().get_save_id()));
	gamestate_.write_config(out, false);
	out.write_child("snapshot",snapshot_);
	out.open_child("statistics");
	statistics::write_stats(out);
	out.close_child("statistics");
}

void savegame::finish_save_game(const config_writer &out)
{
	std::string name = gamestate_.classification().label;
	replace_space2underbar(name);
	std::string fname(get_saves_dir() + "/" + name);

	try {
		if(!out.good()) {
			throw game::save_game_failed(_("Could not write to file"));
		}

		config& summary = save_index::save_summary(gamestate_.classification().label);
		extract_summary_data_from_save(summary);
		const int mod_time = static_cast<int>(file_create_time(fname));
		summary["mod_time"] = str_cast(mod_time);
		save_index::write_save_index();
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

void savegame::extract_summary_data_from_save(config& out)
{
	const bool has_replay = gamestate_.replay_data.empty() == false;
	const bool has_snapshot = gamestate_.snapshot.child("side");

	out["replay"] = has_replay ? "yes" : "no";
	out["snapshot"] = has_snapshot ? "yes" : "no";

	out["label"] = gamestate_.classification().label;
	out["parent"] = gamestate_.classification().parent;
	out["campaign"] = gamestate_.classification().campaign;
	out["campaign_type"] = gamestate_.classification().campaign_type;
	out["scenario"] = gamestate_.classification().scenario;
	out["difficulty"] = gamestate_.classification().difficulty;
	out["version"] = gamestate_.classification().version;
	out["corrupt"] = "";

	if(has_snapshot) {
		out["turn"] = gamestate_.snapshot["turn_at"];
		if(gamestate_.snapshot["turns"] != "-1") {
			out["turn"] = out["turn"].str() + "/" + gamestate_.snapshot["turns"].str();
		}
	}

	// Find the first human leader so we can display their icon in the load menu.

	/** @todo Ideally we should grab all leaders if there's more than 1 human player? */
	std::string leader;

	bool shrouded = false;

	const config& snapshot = has_snapshot ? gamestate_.snapshot : gamestate_.starting_pos;
	BOOST_FOREACH (const config &side, snapshot.child_range("side"))
	{
		if (side["controller"] != "human") {
			continue;
		}
		if (utils::string_bool(side["shroud"])) {
			shrouded = true;
		}

		BOOST_FOREACH (const config &u, side.child_range("unit"))
		{
			if (utils::string_bool(u["canrecruit"], false)) {
				leader = u["id"];
				break;
			}
		}
	}

	out["leader"] = leader;
	out["map_data"] = "";

	if(!shrouded) {
		if(has_snapshot) {
			if (!gamestate_.snapshot.find_child("side", "shroud", "yes")) {
				out["map_data"] = gamestate_.snapshot["map_data"];
			}
		} else if(has_replay) {
			if (!gamestate_.starting_pos.find_child("side", "shroud", "yes")) {
				out["map_data"] = gamestate_.starting_pos["map_data"];
			}
		}
	}
}

scenariostart_savegame::scenariostart_savegame(game_state &gamestate, const bool compress_saves)
	: savegame(gamestate, compress_saves)
{
	set_filename(gamestate.classification().label);
}

void scenariostart_savegame::before_save()
{
	//Add the player section to the starting position so we can get the correct recall list
	//when loading the replay later on
	// if there is no scenario information in the starting pos, add the (persistent) sides from the snapshot
	// else do nothing, as persistence information was already added at the end of the previous scenario
	if (gamestate().starting_pos["id"].empty()) {
		BOOST_FOREACH(const config* snapshot_side, gamestate().snapshot.get_children("side")) {
			//add all side tags (assuming they only contain carryover information)
			gamestate().starting_pos.add_child("side", *snapshot_side);
		}
	}
}

replay_savegame::replay_savegame(game_state &gamestate, const bool compress_saves)
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

autosave_savegame::autosave_savegame(game_state &gamestate,
					game_display& gui, const config& snapshot_cfg, const bool compress_saves)
	: game_savegame(gamestate, gui, snapshot_cfg, compress_saves)
{
	set_error_message(_("Could not auto save the game. Please save the game manually."));
}

void autosave_savegame::autosave(const bool disable_autosave, const int autosave_max, const int infinite_autosaves)
{
	if(disable_autosave)
		return;

	save_game_automatic(gui_.video());

	manager::remove_old_auto_saves(autosave_max, infinite_autosaves);
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
	: game_savegame(*resources::state_of_game, *resources::screen, snapshot_cfg, preferences::compress_saves())
{}

int oos_savegame::show_save_dialog(CVideo& video, const std::string& message, const gui::DIALOG_TYPE /*dialog_type*/)
{
	static bool ignore_all = false;
	int res = 0;

	std::string filename = this->filename();

	if (!ignore_all){
		gui2::tgame_save_oos dlg(title(), filename, message);
		dlg.show(video);
		filename = dlg.filename();
		ignore_all = dlg.ignore_all();
		res = dlg.get_retval();
	}

	check_filename(filename, video);
	set_filename(filename);

	return res;
}

game_savegame::game_savegame(game_state &gamestate,
					game_display& gui, const config& snapshot_cfg, const bool compress_saves)
	: savegame(gamestate, compress_saves, _("Save Game")),
	gui_(gui)
{
	snapshot().merge_with(snapshot_cfg);
}

void game_savegame::create_filename()
{
	std::stringstream stream;

	const std::string ellipsed_name = font::make_text_ellipsis(gamestate().classification().label,
			font::SIZE_NORMAL, 200);
	stream << ellipsed_name << " " << _("Turn") << " " << snapshot()["turn_at"];
	set_filename(stream.str());
}

void game_savegame::before_save()
{
	savegame::before_save();
	write_game_snapshot();
}

void game_savegame::write_game_snapshot()
{

	snapshot()["snapshot"] = "yes";

	std::stringstream buf;
	buf << gui_.playing_team();
	snapshot()["playing_team"] = buf.str();

	write_events(snapshot());

	write_music_play_list(snapshot());

	gamestate().write_snapshot(snapshot());

	gui_.labels().write(snapshot());
}

}

