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

#include "save_index.hpp"

#include "format_time_summary.hpp"
#include "formula_string_utils.hpp"
#include "game_display.hpp"
#include "game_end_exceptions.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "serialization/binary_or_text.hpp"
#include "serialization/parser.hpp"

#include "filesystem.hpp"
#include "config.hpp"
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


void save_index_class::rebuild(const std::string& name) {
	std::string filename = name;
	replace_space2underbar(filename);
	time_t modified = file_create_time(get_saves_dir() + "/" + filename);
	rebuild(name, modified);
}

void save_index_class::rebuild(const std::string& name, const time_t& modified) {
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

void save_index_class::remove(const std::string& name) {
	config& root = data();
	root.remove_attribute(name);
	write_save_index();
}

void save_index_class::set_modified(const std::string& name, const time_t& modified) {
	modified_[name] = modified;
}

config& save_index_class::get(const std::string& name) {
	config& result = data(name);
	time_t m = modified_[name];
	config::attribute_value& mod_time = result["mod_time"];
	if (mod_time.empty() || static_cast<time_t>(mod_time.to_int()) != m) {
		rebuild(name, m);
	}
	return result;
}

void save_index_class::write_save_index() {
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
		ERR_SAVE << "error writing to save index file: '" << e.what() << "'" << std::endl;
	}
}

save_index_class::save_index_class()
	: loaded_(false)
	, data_()
	, modified_()
{
}

config& save_index_class::data(const std::string& name) {
	config& cfg = data();
	if (config& sv = cfg.find_child("save", "save", name)) {
		return sv;
	}
		config& res = cfg.add_child("save");
		res["save"] = name;
	return res;
}

config& save_index_class::data() {
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
			ERR_SAVE << "error reading save index: '" << e.what() << "'" << std::endl;
		} catch(config::error& e) {
			ERR_SAVE << "error parsing save index config file:\n" << e.message << std::endl;
			data_.clear();
		}
		loaded_ = true;
	}
	return data_;
}

save_index_class save_index_manager;

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



create_save_info::create_save_info(const std::string* d)
	: dir(d ? *d : get_saves_dir())
{
}

save_info create_save_info::operator()(const std::string& filename) const
{
	std::string name = filename;
	replace_underbar2space(name);
	time_t modified = file_create_time(dir + "/" + filename);
	save_index_manager.set_modified(name, modified);
	return save_info(name, modified);
}

} // end namespace savegame

void extract_summary_from_config(config& cfg_save, config& cfg_summary)
{
	const config &cfg_snapshot = cfg_save.child("snapshot");
	//Servergenerated replays contain [scenario] and no [replay_start]
	const config &cfg_replay_start = cfg_save.child("replay_start") ? cfg_save.child("replay_start") : cfg_save.child("scenario") ;

	const config &cfg_replay = cfg_save.child("replay");
	const bool has_replay = cfg_replay && !cfg_replay.empty();
	const bool has_snapshot = cfg_snapshot && cfg_snapshot.child("side");

	cfg_summary["replay"] = has_replay;
	cfg_summary["snapshot"] = has_snapshot;

	cfg_summary["label"] = cfg_save["label"];
	cfg_summary["campaign_type"] = cfg_save["campaign_type"];

	if(cfg_save.has_child("carryover_sides_start")){
		cfg_summary["scenario"] = cfg_save.child("carryover_sides_start")["next_scenario"];
	} else {
		cfg_summary["scenario"] = cfg_save["scenario"];
	}

	cfg_summary["difficulty"] = cfg_save["difficulty"];
	cfg_summary["random_mode"] = cfg_save["random_mode"];


	cfg_summary["campaign"] = cfg_save["campaign"];
	cfg_summary["version"] = cfg_save["version"];
	cfg_summary["corrupt"] = "";

	if(has_snapshot) {
		cfg_summary["turn"] = cfg_snapshot["turn_at"];
		if (cfg_snapshot["turns"] != "-1") {
			cfg_summary["turn"] = cfg_summary["turn"].str() + "/" + cfg_snapshot["turns"].str();
		}
	}

	// Find the first human leader so we can display their icon in the load menu.

	/** @todo Ideally we should grab all leaders if there's more than 1 human player? */
	std::string leader;
	std::string leader_image;

	//BOOST_FOREACH(const config &p, cfg_save.child_range("player"))
	//{
	//	if (p["canrecruit"].to_bool(false))) {
	//		leader = p["save_id"];
	//	}
	//}

	bool shrouded = false;

	//if (!leader.empty())
	//{
		if (const config &snapshot = *(has_snapshot ? &cfg_snapshot : &cfg_replay_start))
		{
			BOOST_FOREACH(const config &side, snapshot.child_range("side"))
			{
				if (side["controller"] != team::CONTROLLER_to_string(team::HUMAN)) {
					continue;
				}

				if (side["shroud"].to_bool()) {
					shrouded = true;
				}

				if (side["canrecruit"].to_bool())
				{
						leader = side["id"].str();
						leader_image = side["image"].str();
						break;
				}

				BOOST_FOREACH(const config &u, side.child_range("unit"))
				{
					if (u["canrecruit"].to_bool()) {
						leader = u["id"].str();
						leader_image = u["image"].str();
						break;
					}
				}
			}
		}
	//}

	cfg_summary["leader"] = leader;
	// We need a binary path-independent path to the leader image here
	// so it can be displayed for campaign-specific units in the dialog
	// even when the campaign isn't loaded yet.
	cfg_summary["leader_image"] = get_independent_image_path(leader_image);

	if(!shrouded) {
		if(has_snapshot) {
			if (!cfg_snapshot.find_child("side", "shroud", "yes")) {
				cfg_summary.add_child("map", cfg_snapshot.child_or_empty("map"));
			}
		} else if(has_replay) {
			if (!cfg_replay_start.find_child("side","shroud","yes")) {
				cfg_summary.add_child("map", cfg_replay_start.child_or_empty("map"));
			}
		}
	}
}
