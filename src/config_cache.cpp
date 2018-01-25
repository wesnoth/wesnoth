/*
   Copyright (C) 2008 - 2018 by Pauli Nieminen <paniemin@cc.hut.fi>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "config_cache.hpp"
#include "filesystem.hpp"
#include "gettext.hpp"
#include "game_config.hpp"
#include "log.hpp"
#include "hash.hpp"
#include "serialization/binary_or_text.hpp"
#include "serialization/parser.hpp"
#include "serialization/string_utils.hpp"
#include "version.hpp"

#include <boost/algorithm/string/replace.hpp>
#include <boost/iostreams/filter/gzip.hpp>

static lg::log_domain log_cache("cache");
#define ERR_CACHE LOG_STREAM(err, log_cache)
#define LOG_CACHE LOG_STREAM(info, log_cache)
#define DBG_CACHE LOG_STREAM(debug, log_cache)

namespace game_config
{

config_cache& config_cache::instance()
{
	static config_cache cache;
	return cache;
}

config_cache::config_cache()
	: force_valid_cache_(false)
	, use_cache_(true)
	, fake_invalid_cache_(false)
	, defines_map_()
	, cache_file_prefix_("cache-v" + boost::algorithm::replace_all_copy(game_config::revision, ":", "_") + "-")
{
	// To set-up initial defines map correctly
	clear_defines();
}

const preproc_map& config_cache::get_preproc_map() const
{
	return defines_map_;
}

void config_cache::clear_defines()
{
	LOG_CACHE << "Clearing defines map!" << std::endl;

	defines_map_.clear();

	//
	// Set-up default defines map.
	//

#ifdef __APPLE__
	defines_map_["APPLE"] = preproc_define();
#endif

	defines_map_["WESNOTH_VERSION"] = preproc_define(game_config::wesnoth_version.str());
}

void config_cache::get_config(const std::string& file_path, config& cfg)
{
	load_configs(file_path, cfg);
}

void config_cache::write_file(std::string file_path, const config& cfg)
{
	filesystem::scoped_ostream stream = filesystem::ostream_file(file_path);
	config_writer writer(*stream, true, game_config::cache_compression_level);
	writer.write(cfg);
}

void config_cache::write_file(std::string file_path, const preproc_map& defines)
{
	if(defines.empty()) {
		if(filesystem::file_exists(file_path)) {
			filesystem::delete_directory(file_path);
		}
		return;
	}

	filesystem::scoped_ostream stream = filesystem::ostream_file(file_path);
	config_writer writer(*stream, true, game_config::cache_compression_level);

	// Write all defines to stream.
	for(const preproc_map::value_type& define : defines) {
		define.second.write(writer, define.first);
	}
}

void config_cache::read_file(const std::string& file_path, config& cfg)
{
	filesystem::scoped_istream stream = filesystem::istream_file(file_path);
	read_gz(cfg, *stream);
}

preproc_map& config_cache::make_copy_map()
{
	return config_cache_transaction::instance().get_active_map(defines_map_);
}

void config_cache::add_defines_map_diff(preproc_map& defines_map)
{
	return config_cache_transaction::instance().add_defines_map_diff(defines_map);
}

void config_cache::read_configs(const std::string& file_path, config& cfg, preproc_map& defines_map)
{
	//read the file and then write to the cache
	filesystem::scoped_istream stream = preprocess_file(file_path, &defines_map);
	read(cfg, *stream);
}

void config_cache::read_cache(const std::string& file_path, config& cfg)
{
	static const std::string extension = ".gz";

	std::stringstream defines_string;
	defines_string << file_path;

	bool is_valid = true;

	for(const preproc_map::value_type& d : defines_map_) {
		//
		// Only WESNOTH_VERSION is allowed to be non-empty.
		//
		if((!d.second.value.empty() || !d.second.arguments.empty()) &&
		   d.first != "WESNOTH_VERSION")
		{
			is_valid = false;
			ERR_CACHE << "Invalid preprocessor define: " << d.first << '\n';
			break;
		}

		defines_string << " " << d.first;
	}

	// Do cache check only if define map is valid and
	// caching is allowed.
	const std::string& cache_path = filesystem::get_cache_dir();

	if(is_valid && !cache_path.empty()) {
		// Use a hash for a shorter display of the defines.
		const std::string fname = cache_path + "/" +
								  cache_file_prefix_ +
								  utils::sha1(defines_string.str()).hex_digest();
		const std::string fname_checksum = fname + ".checksum" + extension;

		filesystem::file_tree_checksum dir_checksum;

		if(!force_valid_cache_ && !fake_invalid_cache_) {
			try {
				if(filesystem::file_exists(fname_checksum)) {
					config checksum_cfg;

					DBG_CACHE << "Reading checksum: " << fname_checksum << "\n";
					read_file(fname_checksum, checksum_cfg);

					dir_checksum = filesystem::file_tree_checksum(checksum_cfg);
				}
			} catch(config::error&) {
				ERR_CACHE << "cache checksum is corrupt" << std::endl;
			} catch(filesystem::io_exception&) {
				ERR_CACHE << "error reading cache checksum" << std::endl;
			}
		}

		if(force_valid_cache_) {
			LOG_CACHE << "skipping cache validation (forced)\n";
		}

		if(filesystem::file_exists(fname + extension) && (force_valid_cache_ || (dir_checksum == filesystem::data_tree_checksum()))) {
			LOG_CACHE << "found valid cache at '" << fname << extension << "' with defines_map " << defines_string.str() << "\n";
			log_scope("read cache");

			try {
				read_file(fname + extension,cfg);
				const std::string define_file = fname + ".define" + extension;

				if(filesystem::file_exists(define_file)) {
					config_cache_transaction::instance().add_define_file(define_file);
				}

				return;
			} catch(config::error& e) {
				ERR_CACHE << "cache " << fname << extension << " is corrupt. Loading from files: "<< e.message << std::endl;
			} catch(filesystem::io_exception&) {
				ERR_CACHE << "error reading cache " << fname << extension << ". Loading from files" << std::endl;
			} catch (boost::iostreams::gzip_error& e) {
				//read_file -> ... -> read_gz can throw this exception.
				ERR_CACHE << "cache " << fname << extension << " is corrupt. Error code: " << e.error() << std::endl;
			}
		}

		LOG_CACHE << "no valid cache found. Writing cache to '" << fname << extension << " with defines_map "<< defines_string.str() << "'\n";

		// Now we need queued defines so read them to memory
		read_defines_queue();

		preproc_map copy_map(make_copy_map());

		read_configs(file_path, cfg, copy_map);
		add_defines_map_diff(copy_map);

		try {
			write_file(fname + extension, cfg);
			write_file(fname + ".define" + extension, copy_map);

			config checksum_cfg;

			filesystem::data_tree_checksum().write(checksum_cfg);
			write_file(fname_checksum, checksum_cfg);
		} catch(filesystem::io_exception&) {
			ERR_CACHE << "could not write to cache '" << fname << "'" << std::endl;
		}

		return;
	}

	LOG_CACHE << "Loading plain config instead of cache\n";

	preproc_map copy_map(make_copy_map());
	read_configs(file_path, cfg, copy_map);
	add_defines_map_diff(copy_map);
}

void config_cache::read_defines_file(const std::string& file_path)
{
	config cfg;
	read_file(file_path, cfg);

	DBG_CACHE << "Reading cached defines from: " << file_path << "\n";

	// use static preproc_define::read_pair(config) to make a object
	// and pass that object config_cache_transaction::insert_to_active method
	for(const config::any_child &value : cfg.all_children_range()) {
		config_cache_transaction::instance().insert_to_active(
			preproc_define::read_pair(value.cfg));
	}
}

void config_cache::read_defines_queue()
{
	const std::vector<std::string>& files = config_cache_transaction::instance().get_define_files();

	for(const std::string &p : files) {
		read_defines_file(p);
	}
}

void config_cache::load_configs(const std::string& config_path, config& cfg)
{
	// Make sure that we have fake transaction if no real one is going on
	fake_transaction fake;

	if (use_cache_) {
		read_cache(config_path, cfg);
	} else {
		preproc_map copy_map(make_copy_map());
		read_configs(config_path, cfg, copy_map);
		add_defines_map_diff(copy_map);
	}
}

void config_cache::set_force_invalid_cache(bool force)
{
	fake_invalid_cache_ = force;
}

void config_cache::set_use_cache(bool use)
{
	use_cache_ = use;
}

void config_cache::set_force_valid_cache(bool force)
{
	force_valid_cache_ = force;
}

void config_cache::recheck_filetree_checksum()
{
	filesystem::data_tree_checksum(true);
}

void config_cache::add_define(const std::string& define)
{
	DBG_CACHE << "adding define: " << define << "\n";
	defines_map_[define] = preproc_define();

	if(config_cache_transaction::is_active()) {
		// we have to add this to active map too
		config_cache_transaction::instance().get_active_map(defines_map_).emplace(define, preproc_define());
	}

}

void config_cache::remove_define(const std::string& define)
{
	DBG_CACHE << "removing define: " << define << "\n";
	defines_map_.erase(define);

	if(config_cache_transaction::is_active()) {
		// we have to remove this from active map too
		config_cache_transaction::instance().get_active_map(defines_map_).erase(define);
	}
}

bool config_cache::clean_cache()
{
	std::vector<std::string> files, dirs;
	filesystem::get_files_in_dir(filesystem::get_cache_dir(), &files, &dirs, filesystem::ENTIRE_FILE_PATH);

	LOG_CACHE << "clean_cache(): " << files.size() << " files, "
			  << dirs.size() << " dirs to check\n";

	const std::string& exclude_current = cache_file_prefix_ + "*";

	bool status = true;

	status &= delete_cache_files(files, exclude_current);
	status &= delete_cache_files(dirs, exclude_current);

	LOG_CACHE << "clean_cache(): done\n";

	return status;
}

bool config_cache::purge_cache()
{
	std::vector<std::string> files, dirs;
	filesystem::get_files_in_dir(filesystem::get_cache_dir(), &files, &dirs, filesystem::ENTIRE_FILE_PATH);

	LOG_CACHE << "purge_cache(): deleting " << files.size() << " files, "
			  << dirs.size() << " dirs\n";

	bool status = true;

	status &= delete_cache_files(files);
	status &= delete_cache_files(dirs);

	LOG_CACHE << "purge_cache(): done\n";
	return status;
}

bool config_cache::delete_cache_files(const std::vector<std::string>& paths,
									  const std::string& exclude_pattern)
{
	const bool delete_everything = exclude_pattern.empty();
	bool status = true;

	for(const std::string& file_path : paths)
	{
		if(!delete_everything) {
			const std::string& fn = filesystem::base_name(file_path);

			if(utils::wildcard_string_match(fn, exclude_pattern)) {
				LOG_CACHE << "delete_cache_files(): skipping " << file_path
						  << " excluded by '" << exclude_pattern << "'\n";
				continue;
			}
		}

		LOG_CACHE << "delete_cache_files(): deleting " << file_path << '\n';
		if(!filesystem::delete_directory(file_path)) {
			ERR_CACHE << "delete_cache_files(): could not delete "
					  << file_path << '\n';
			status = false;
		}
	}

	return status;
}

config_cache_transaction::state config_cache_transaction::state_ = FREE;
config_cache_transaction* config_cache_transaction::active_ = 0;

config_cache_transaction::config_cache_transaction()
	: define_filenames_()
	, active_map_()
{
	assert(state_ == FREE);
	state_ = NEW;
	active_ = this;
}

config_cache_transaction::~config_cache_transaction()
{
	state_ = FREE;
	active_ = 0;
}

void config_cache_transaction::lock()
{
	state_ = LOCKED;
}

const std::vector<std::string>& config_cache_transaction::get_define_files() const
{
	return define_filenames_;
}

void config_cache_transaction::add_define_file(const std::string& file)
{
	define_filenames_.push_back(file);
}

preproc_map& config_cache_transaction::get_active_map(const preproc_map& defines_map)
{
	if(active_map_.empty()) {
		active_map_.insert(defines_map.begin(), defines_map.end());
		if(get_state() == NEW) {
			state_ = ACTIVE;
		}
	 }

	return active_map_;
}

namespace
{

bool compare_define(const preproc_map::value_type& a, const preproc_map::value_type& b)
{
	if(a.first < b.first) {
		return true;
	}

	if(b.first < a.first) {
		return false;
	}

	if(a.second < b.second) {
		return true;
	}

	return false;
}

} // end anonymous namespace

void config_cache_transaction::add_defines_map_diff(preproc_map& new_map)
{
	if(get_state() == ACTIVE) {
		preproc_map temp;
		std::set_difference(new_map.begin(),
				new_map.end(),
				active_map_.begin(),
				active_map_.end(),
				std::insert_iterator<preproc_map>(temp,temp.begin()),
				&compare_define);

		for(const preproc_map::value_type &def : temp) {
			insert_to_active(def);
		}

		temp.swap(new_map);
	} else if (get_state() == LOCKED) {
		new_map.clear();
	}
}

void config_cache_transaction::insert_to_active(const preproc_map::value_type& def)
{
	active_map_[def.first] = def.second;
}

}
