/*
   Copyright (C) 2008 - 2014 by Pauli Nieminen <paniemin@cc.hut.fi>
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
#include "game_display.hpp"
#include "log.hpp"
#include "marked-up_text.hpp"
#include "show_dialog.hpp"
#include "sha1.hpp"
#include "serialization/binary_or_text.hpp"
#include "serialization/parser.hpp"
#include "version.hpp"

#include <boost/foreach.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/iostreams/filter/gzip.hpp>

static lg::log_domain log_cache("cache");
#define ERR_CACHE LOG_STREAM(err, log_cache)
#define LOG_CACHE LOG_STREAM(info, log_cache)
#define DBG_CACHE LOG_STREAM(debug, log_cache)

namespace game_config {

	config_cache& config_cache::instance()
	{
		static config_cache cache;
		return cache;
	}

	config_cache::config_cache() :
		force_valid_cache_(false),
		use_cache_(true),
		fake_invalid_cache_(false),
		defines_map_()
	{
		// To set-up initial defines map correctly
		clear_defines();
	}

	struct output {
		void operator()(const preproc_map::value_type& def)
		{
			DBG_CACHE << "key: " << def.first << " " << def.second << "\n";
		}
	};
	const preproc_map& config_cache::get_preproc_map() const
	{
		return defines_map_;
	}

	void config_cache::clear_defines()
	{
		LOG_CACHE << "Clearing defines map!\n";
		defines_map_.clear();
		// set-up default defines map

#ifdef LOW_MEM
		defines_map_["LOW_MEM"] = preproc_define();
#endif

#if defined(__APPLE__)
		defines_map_["APPLE"] = preproc_define();
#endif

		defines_map_["WESNOTH_VERSION"] = preproc_define(game_config::wesnoth_version.str());

	}

	void config_cache::get_config(const std::string& path, config& cfg)
	{
		load_configs(path, cfg);
	}

	void config_cache::write_file(std::string path, const config& cfg)
	{
		scoped_ostream stream = ostream_file(path);
		const bool gzip = true;
		config_writer writer(*stream, gzip, game_config::cache_compression_level);
		writer.write(cfg);
	}
	void config_cache::write_file(std::string path, const preproc_map& defines_map)
	{
		if (defines_map.empty())
		{
			if (file_exists(path))
			{
				delete_directory(path);
			}
			return;
		}
		scoped_ostream stream = ostream_file(path);
		const bool gzip = true;
		config_writer writer(*stream, gzip, game_config::cache_compression_level);

		// write all defines to stream
		BOOST_FOREACH(const preproc_map::value_type &define, defines_map) {
			define.second.write(writer, define.first);
		}
	}

	void config_cache::read_file(const std::string& path, config& cfg)
	{
		scoped_istream stream = istream_file(path);
		read_gz(cfg, *stream);
	}

	preproc_map& config_cache::make_copy_map()
	{
		return config_cache_transaction::instance().get_active_map(defines_map_);
	}


	static bool compare_define(const preproc_map::value_type& a, const preproc_map::value_type& b)
	{
		if (a.first < b.first)
			return true;
		if (b.first < a.first)
			return false;
		if (a.second < b.second)
			return true;
		return false;
	}

	void config_cache::add_defines_map_diff(preproc_map& defines_map)
	{
		return config_cache_transaction::instance().add_defines_map_diff(defines_map);
	}


	void config_cache::read_configs(const std::string& path, config& cfg, preproc_map& defines_map)
	{
		//read the file and then write to the cache
		scoped_istream stream = preprocess_file(path, &defines_map);
		read(cfg, *stream);
	}

	void config_cache::read_cache(const std::string& path, config& cfg)
	{
		const std::string extension = ".gz";
		bool is_valid = true;
		std::stringstream defines_string;
		defines_string << path;
		for(preproc_map::const_iterator i = defines_map_.begin(); i != defines_map_.end(); ++i) {
			if(i->second.value != "" || i->second.arguments.empty() == false) {
				// VERSION is defined non-empty by the engine,
				// it should be safe to rely on caches containing it.
				if(i->first != "WESNOTH_VERSION") {
					is_valid = false;
					ERR_CACHE << "Preprocessor define not valid\n";
					break;
				}
			}

			defines_string << " " << i->first;
		}

		// Do cache check only if  define map is valid and
		// caching is allowed
		if(is_valid) {
			const std::string& cache = get_cache_dir();
			if(cache != "") {
				sha1_hash sha(defines_string.str()); // use a hash for a shorter display of the defines
				const std::string fname = cache + "/cache-v" +
					boost::algorithm::replace_all_copy(game_config::revision, ":", "_") +
					"-" + sha.display();
				const std::string fname_checksum = fname + ".checksum" + extension;

				file_tree_checksum dir_checksum;

				if(!force_valid_cache_ && !fake_invalid_cache_) {
					try {
						if(file_exists(fname_checksum)) {
							DBG_CACHE << "Reading checksum: " << fname_checksum << "\n";
							config checksum_cfg;
							read_file(fname_checksum, checksum_cfg);
							dir_checksum = file_tree_checksum(checksum_cfg);
						}
					} catch(config::error&) {
						ERR_CACHE << "cache checksum is corrupt\n";
					} catch(io_exception&) {
						ERR_CACHE << "error reading cache checksum\n";
					}
				}

				if(force_valid_cache_) {
					LOG_CACHE << "skipping cache validation (forced)\n";
				}

				if(file_exists(fname + extension) && (force_valid_cache_ || (dir_checksum == data_tree_checksum()))) {
					LOG_CACHE << "found valid cache at '" << fname << extension << "' with defines_map " << defines_string.str() << "\n";
					log_scope("read cache");
					try {
						read_file(fname + extension,cfg);
						const std::string define_file = fname + ".define" + extension;
						if (file_exists(define_file))
						{
							config_cache_transaction::instance().add_define_file(define_file);
						}
						return;
					} catch(config::error& e) {
						ERR_CACHE << "cache " << fname << extension << " is corrupt. Loading from files: "<< e.message<<"\n";
					} catch(io_exception&) {
						ERR_CACHE << "error reading cache " << fname << extension << ". Loading from files\n";
					} catch (boost::iostreams::gzip_error& e) {
						//read_file -> ... -> read_gz can throw this exeption.
						ERR_CACHE << "cache " << fname << extension << " is corrupt. Error code: " << e.error() << "\n";
					}
				}

				LOG_CACHE << "no valid cache found. Writing cache to '" << fname << extension << " with defines_map "<< defines_string.str() << "'\n";
				// Now we need queued defines so read them to memory
				read_defines_queue();

				preproc_map copy_map(make_copy_map());

				read_configs(path, cfg, copy_map);

				add_defines_map_diff(copy_map);

				try {
					write_file(fname + extension, cfg);
					write_file(fname + ".define" + extension, copy_map);
					config checksum_cfg;
					data_tree_checksum().write(checksum_cfg);
					write_file(fname_checksum, checksum_cfg);
				} catch(io_exception&) {
					ERR_CACHE << "could not write to cache '" << fname << "'\n";
				}
				return;
			}
		}
		LOG_CACHE << "Loading plain config instead of cache\n";
		preproc_map copy_map(make_copy_map());
		read_configs(path, cfg, copy_map);
		add_defines_map_diff(copy_map);
	}


	void config_cache::read_defines_file(const std::string& path)
	{
		config cfg;
		read_file(path, cfg);

		DBG_CACHE << "Reading cached defines from: " << path << "\n";

		// use static preproc_define::read_pair(config) to make a object
		// and pass that object config_cache_transaction::insert_to_active method
		BOOST_FOREACH(const config::any_child &value, cfg.all_children_range()) {
			config_cache_transaction::instance().insert_to_active(
				preproc_define::read_pair(value.cfg));
		}
	}

	void config_cache::read_defines_queue()
	{
		const config_cache_transaction::filenames& files = config_cache_transaction::instance().get_define_files();
		BOOST_FOREACH(const std::string &path, files) {
			read_defines_file(path);
		}
	}

	void config_cache::load_configs(const std::string& path, config& cfg)
	{
		// Make sure that we have fake transaction if no real one is going on
		fake_transaction fake;

		if (use_cache_) {
			read_cache(path, cfg);
		} else {
			preproc_map copy_map(make_copy_map());
			read_configs(path, cfg, copy_map);
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
		data_tree_checksum(true);
	}

	void config_cache::add_define(const std::string& define)
	{
		DBG_CACHE << "adding define: " << define << "\n";
		defines_map_[define] = preproc_define();
		if (config_cache_transaction::is_active())
		{
			// we have to add this to active map too
			config_cache_transaction::instance().get_active_map(defines_map_).insert(
					std::make_pair(define, preproc_define()));
		}

	}

	void config_cache::remove_define(const std::string& define)
	{
		DBG_CACHE << "removing define: " << define << "\n";
		defines_map_.erase(define);
		if (config_cache_transaction::is_active())
		{
			// we have to remove this from active map too
			config_cache_transaction::instance().get_active_map(defines_map_).erase(define);
		}
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

	const config_cache_transaction::filenames& config_cache_transaction::get_define_files() const
	{
		return define_filenames_;
	}

	void config_cache_transaction::add_define_file(const std::string& file)
	{
		define_filenames_.push_back(file);
	}

	preproc_map& config_cache_transaction::get_active_map(const preproc_map& defines_map)
	{
		if(active_map_.empty())
		{
			active_map_.insert(defines_map.begin(), defines_map.end());
			if ( get_state() == NEW)
				state_ = ACTIVE;
		 }

		return active_map_;
	}

	void config_cache_transaction::add_defines_map_diff(preproc_map& new_map)
	{

		if (get_state() == ACTIVE)
		{
			preproc_map temp;
			std::set_difference(new_map.begin(),
					new_map.end(),
					active_map_.begin(),
					active_map_.end(),
					std::insert_iterator<preproc_map>(temp,temp.begin()),
					&compare_define);

			BOOST_FOREACH(const preproc_map::value_type &def, temp) {
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
