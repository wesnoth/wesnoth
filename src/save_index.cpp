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

#include "save_index.hpp"

#include "config.hpp"
#include "filesystem.hpp"
#include "format_time_summary.hpp"
#include "game_end_exceptions.hpp"
#include "game_errors.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "preferences/game.hpp"
#include "serialization/binary_or_text.hpp"
#include "serialization/parser.hpp"
#include "team.hpp"

#include <boost/algorithm/string/replace.hpp>
#include <boost/iostreams/filter/gzip.hpp>

static lg::log_domain log_engine("engine");
#define LOG_SAVE LOG_STREAM(info, log_engine)
#define ERR_SAVE LOG_STREAM(err, log_engine)

static lg::log_domain log_enginerefac("enginerefac");
#define LOG_RG LOG_STREAM(info, log_enginerefac)

namespace savegame
{
void extract_summary_from_config(config&, config&);

void save_index_class::rebuild(const std::string& name)
{
	std::time_t modified = filesystem::file_modified_time(filesystem::get_saves_dir() + "/" + name);
	rebuild(name, modified);
}

void save_index_class::rebuild(const std::string& name, const std::time_t& modified)
{
	log_scope("load_summary_from_file");

	config& summary = data(name);

	try {
		config full;
		std::string dummy;
		read_save_file(name, full, &dummy);

		extract_summary_from_config(full, summary);
	} catch(const game::load_game_failed&) {
		summary["corrupt"] = true;
	}

	summary["mod_time"] = std::to_string(static_cast<int>(modified));
	write_save_index();
}

void save_index_class::remove(const std::string& name)
{
	config& root = data();
	root.remove_attribute(name);
	write_save_index();
}

void save_index_class::set_modified(const std::string& name, const std::time_t& modified)
{
	modified_[name] = modified;
}

config& save_index_class::get(const std::string& name)
{
	config& result = data(name);
	std::time_t m = modified_[name];

	config::attribute_value& mod_time = result["mod_time"];
	if(mod_time.empty() || mod_time.to_time_t() != m) {
		rebuild(name, m);
	}

	return result;
}

void save_index_class::write_save_index()
{
	log_scope("write_save_index()");

	try {
		filesystem::scoped_ostream stream = filesystem::ostream_file(filesystem::get_save_index_file());

		if(preferences::save_compression_format() != compression::NONE) {
			// TODO: maybe allow writing this using bz2 too?
			write_gz(*stream, data());
		} else {
			write(*stream, data());
		}
	} catch(const filesystem::io_exception& e) {
		ERR_SAVE << "error writing to save index file: '" << e.what() << "'" << std::endl;
	}
}

save_index_class::save_index_class()
	: loaded_(false)
	, data_()
	, modified_()
{
}

config& save_index_class::data(const std::string& name)
{
	config& cfg = data();
	if(config& sv = cfg.find_child("save", "save", name)) {
		fix_leader_image_path(sv);
		return sv;
	}

	config& res = cfg.add_child("save");
	res["save"] = name;
	return res;
}

config& save_index_class::data()
{
	const std::string si_file = filesystem::get_save_index_file();

	// Don't try to load the file if it doesn't exist.
	if(loaded_ == false && filesystem::file_exists(si_file)) {
		try {
			filesystem::scoped_istream stream = filesystem::istream_file(si_file);
			try {
				read_gz(data_, *stream);
			} catch(const boost::iostreams::gzip_error&) {
				stream->seekg(0);
				read(data_, *stream);
			}
		} catch(const filesystem::io_exception& e) {
			ERR_SAVE << "error reading save index: '" << e.what() << "'" << std::endl;
		} catch(const config::error& e) {
			ERR_SAVE << "error parsing save index config file:\n" << e.message << std::endl;
			data_.clear();
		}

		loaded_ = true;
	}

	return data_;
}

void save_index_class::fix_leader_image_path(config& data)
{
	for(config& leader : data.child_range("leader")) {
		std::string leader_image = leader["leader_image"];
		boost::algorithm::replace_all(leader_image, "\\", "/");

		leader["leader_image"] = leader_image;
	}
}

save_index_class save_index_manager;

class filename_filter
{
public:
	filename_filter(const std::string& filter)
		: filter_(filter)
	{
	}

	bool operator()(const std::string& filename) const
	{
		return filename.end() == std::search(filename.begin(), filename.end(), filter_.begin(), filter_.end());
	}

private:
	std::string filter_;
};

/** Get a list of available saves. */
std::vector<save_info> get_saves_list(const std::string* dir, const std::string* filter)
{
	create_save_info creator(dir);

	std::vector<std::string> filenames;
	filesystem::get_files_in_dir(creator.dir, &filenames);

	if(filter) {
		filenames.erase(
			std::remove_if(filenames.begin(), filenames.end(), filename_filter(*filter)), filenames.end());
	}

	std::vector<save_info> result;
	std::transform(filenames.begin(), filenames.end(), std::back_inserter(result), creator);
	std::sort(result.begin(), result.end(), save_info_less_time());

	return result;
}

const config& save_info::summary() const
{
	return save_index_manager.get(name());
}

std::string save_info::format_time_local() const
{
	if(std::tm* tm_l = std::localtime(&modified())) {
		const std::string format = preferences::use_twelve_hour_clock_format()
			? _("%a %b %d %I:%M %p %Y")
			: _("%a %b %d %H:%M %Y");

		return translation::strftime(format, tm_l);
	}

	LOG_SAVE << "localtime() returned null for time " << this->modified() << ", save " << name();
	return "";
}

std::string save_info::format_time_summary() const
{
	std::time_t t = modified();
	return utils::format_time_summary(t);
}

bool save_info_less_time::operator()(const save_info& a, const save_info& b) const
{
	if(a.modified() > b.modified()) {
		return true;
	} else if(a.modified() < b.modified()) {
		return false;
	} else if(a.name().find(_(" replay")) == std::string::npos && b.name().find(_(" replay")) != std::string::npos) {
		// Special funky case; for files created in the same second,
		// a replay file sorts less than a non-replay file.  Prevents
		// a timing-dependent bug where it may look like, at the end
		// of a scenario, the replay and the autosave for the next
		// scenario are displayed in the wrong order.
		return true;
	} else if(a.name().find(_(" replay")) != std::string::npos && b.name().find(_(" replay")) == std::string::npos) {
		return false;
	} else {
		return a.name() > b.name();
	}
}

static filesystem::scoped_istream find_save_file(
		const std::string& name, const std::vector<std::string>& suffixes)
{
	for(const std::string& suf : suffixes) {
		filesystem::scoped_istream file_stream =
			filesystem::istream_file(filesystem::get_saves_dir() + "/" + name + suf);

		if(!file_stream->fail()) {
			return file_stream;
		}
	}

	LOG_SAVE << "Could not open supplied filename '" << name << "'\n";
	throw game::load_game_failed();
}

void read_save_file(const std::string& name, config& cfg, std::string* error_log)
{
	static const std::vector<std::string> suffixes{"", ".gz", ".bz2"};
	filesystem::scoped_istream file_stream = find_save_file(name, suffixes);

	cfg.clear();
	try {
		/*
		 * Test the modified name, since it might use a .gz
		 * file even when not requested.
		 */
		if(filesystem::is_gzip_file(name)) {
			read_gz(cfg, *file_stream);
		} else if(filesystem::is_bzip2_file(name)) {
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
	} catch(const config::error& err) {
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
	if(countdown == infinite_auto_saves) {
		return;
	}

	std::vector<save_info> games = get_saves_list(nullptr, &auto_save);
	for(std::vector<save_info>::iterator i = games.begin(); i != games.end(); ++i) {
		if(countdown-- <= 0) {
			LOG_SAVE << "Deleting savegame '" << i->name() << "'\n";
			delete_game(i->name());
		}
	}
}

void delete_game(const std::string& name)
{
	filesystem::delete_file(filesystem::get_saves_dir() + "/" + name);

	save_index_manager.remove(name);
}

create_save_info::create_save_info(const std::string* d)
	: dir(d ? *d : filesystem::get_saves_dir())
{
}

save_info create_save_info::operator()(const std::string& filename) const
{
	std::time_t modified = filesystem::file_modified_time(dir + "/" + filename);
	save_index_manager.set_modified(filename, modified);
	return save_info(filename, modified);
}

void extract_summary_from_config(config& cfg_save, config& cfg_summary)
{
	const config& cfg_snapshot = cfg_save.child("snapshot");

	// Servergenerated replays contain [scenario] and no [replay_start]
	const config& cfg_replay_start = cfg_save.child("replay_start")
		? cfg_save.child("replay_start")
		: cfg_save.child("scenario");

	const config& cfg_replay = cfg_save.child("replay");
	const bool has_replay = cfg_replay && !cfg_replay.empty();
	const bool has_snapshot = cfg_snapshot && cfg_snapshot.has_child("side");

	cfg_summary["replay"] = has_replay;
	cfg_summary["snapshot"] = has_snapshot;

	cfg_summary["label"] = cfg_save["label"];
	cfg_summary["campaign_type"] = cfg_save["campaign_type"];

	if(cfg_save.has_child("carryover_sides_start")) {
		cfg_summary["scenario"] = cfg_save.child("carryover_sides_start")["next_scenario"];
	} else {
		cfg_summary["scenario"] = cfg_save["scenario"];
	}

	cfg_summary["difficulty"] = cfg_save["difficulty"];
	cfg_summary["random_mode"] = cfg_save["random_mode"];

	cfg_summary["active_mods"] = cfg_save.child_or_empty("multiplayer")["active_mods"];
	cfg_summary["campaign"] = cfg_save["campaign"];
	cfg_summary["version"] = cfg_save["version"];
	cfg_summary["corrupt"] = "";

	if(has_snapshot) {
		cfg_summary["turn"] = cfg_snapshot["turn_at"];
		if(cfg_snapshot["turns"] != "-1") {
			cfg_summary["turn"] = cfg_summary["turn"].str() + "/" + cfg_snapshot["turns"].str();
		}
	}

	// Ensure we don't get duplicate [leader] tags
	cfg_summary.clear_children("leader");

	// Find the human leaders so we can display their icons and names in the load menu.
	config leader_config;

	bool shrouded = false;

	if(const config& snapshot = *(has_snapshot ? &cfg_snapshot : &cfg_replay_start)) {
		for(const config& side : snapshot.child_range("side")) {
			std::string leader;
			std::string leader_image;
			std::string leader_image_tc_modifier;
			std::string leader_name;
			int gold = side["gold"];
			int units = 0, recall_units = 0;

			if(side["controller"] != team::CONTROLLER::enum_to_string(team::CONTROLLER::HUMAN)) {
				continue;
			}

			if(side["shroud"].to_bool()) {
				shrouded = true;
			}

			for(const config& u : side.child_range("unit")) {
				if(u.has_attribute("x") && u.has_attribute("y")) {
					units++;
				} else {
					recall_units++;
				}

				// Only take the first leader
				if(!leader.empty() || !u["canrecruit"].to_bool()) {
					continue;
				}

				const std::string tc_color = team::get_side_color_id_from_config(side);

				// Don't count it among the troops
				units--;
				leader = u["id"].str();
				leader_name = u["name"].str();
				leader_image = u["image"].str();
				leader_image_tc_modifier = "~RC(" + u["flag_rgb"].str() + ">" + tc_color + ")";
			}

			// We need a binary path-independent path to the leader image here so it can be displayed
			// for campaign-specific units even when the campaign isn't loaded yet.
			std::string leader_image_path = filesystem::get_independent_image_path(leader_image);

			// If the image path was found, we append the leader TC modifier. If it's not (such as in
			// the case where the binary path hasn't been loaded yet, perhaps due to save_index being
			// deleted), the unaltered image path is used and will be parsed by get_independent_image_path
			// at runtime.
			if(!leader_image_path.empty()) {
				leader_image_path += leader_image_tc_modifier;

				leader_image = leader_image_path;
			}

			leader_config["leader"] = leader;
			leader_config["leader_name"] = leader_name;
			leader_config["leader_image"] = leader_image;
			leader_config["leader_image_tc_modifier"] = leader_image_tc_modifier;
			leader_config["gold"] = gold;
			leader_config["units"] = units;
			leader_config["recall_units"] = recall_units;

			cfg_summary.add_child("leader", leader_config);
		}
	}

	if(!shrouded) {
		if(has_snapshot) {
			if(!cfg_snapshot.find_child("side", "shroud", "yes") && cfg_snapshot.has_attribute("map_data")) {
				cfg_summary["map_data"] = cfg_snapshot["map_data"].str();
			} else {
				ERR_SAVE << "Not saving map because there is shroud" << std::endl;
			}
		} else if(has_replay) {
			if(!cfg_replay_start.find_child("side", "shroud", "yes") && cfg_replay_start.has_attribute("map_data")) {
				cfg_summary["map_data"] = cfg_replay_start["map_data"];
			} else {
				ERR_SAVE << "Not saving map because there is shroud" << std::endl;
			}
		}
	}
}

} // end namespace savegame
