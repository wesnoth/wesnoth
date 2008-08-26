/* $Id$ */
/*
   Copyright (C) 2008 by Pauli Nieminen <paniemin@cc.hut.fi>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "config_cache.hpp"
#include "filesystem.hpp"
#include "foreach.hpp"
#include "gettext.hpp"
#include "game_config.hpp"
#include "game_display.hpp"
#include "loadscreen.hpp"
#include "log.hpp"
#include "marked-up_text.hpp"
#include "show_dialog.hpp"
#include "sha1.hpp"
#include "serialization/binary_wml.hpp"
#include "serialization/binary_or_text.hpp"
#include "serialization/parser.hpp"

#define ERR_CONFIG LOG_STREAM(err, config)
#define LOG_CONFIG LOG_STREAM(info, config)
#define DBG_CONFIG LOG_STREAM(debug, config)

#define ERR_FS LOG_STREAM(err, filesystem)

namespace game_config {
	config_cache config_cache::cache_;

	config_cache& config_cache::instance()
	{
		return cache_;
	}

	config_cache::config_cache() : 
		force_valid_cache_(false),
		use_cache_(true),
		defines_map_()
	{
		// To settup initial defines map correctly
		clear_defines();
	}

	const preproc_map& config_cache::get_preproc_map() const
	{
		return defines_map_;
	}

	void config_cache::clear_defines()
	{
		defines_map_.clear();
		// settup default defines map

#ifdef USE_TINY_GUI
		defines_map_["TINY"] = preproc_define();
#endif

		if (game_config::small_gui)
			defines_map_["SMALL_GUI"] = preproc_define();

#ifdef HAVE_PYTHON
		defines_map_["PYTHON"] = preproc_define();
#endif

#if defined(__APPLE__)
		defines_map_["APPLE"] = preproc_define();
#endif

	}

	config_ptr config_cache::get_config(const std::string& path)
	{
		config_ptr ret(new config());
		load_configs(path, *ret);

		return ret;
	}

	void config_cache::write_file(std::string path, const config& cfg)
	{
		bool gzip = false;
		switch(game_config::cache_type)
		{
			case GZIP:
				gzip = true;
				path += ".gz";
			case BWML:
				{
					scoped_ostream stream = ostream_file(path);
					config_writer writer(*stream,gzip,"",game_config::cache_compression_level);
					writer.write(cfg);
				}
				break;
		}
	}

	void config_cache::read_file(const std::string& path, config& cfg)
	{
		std::string error_log;
		switch(game_config::cache_type)
		{
			case BWML:
				{
					scoped_istream stream = istream_file(path);
					read(cfg, *stream);
				}
				break;
			case GZIP:
				{
					scoped_istream stream = istream_file(path + ".gz");
					read_gz(cfg, *stream);
				}
				break;
		}
	}

	void config_cache::read_configs(const std::string& path, config& cfg)
	{
		preproc_map defines_map(defines_map_);

		std::string error_log;
		//read the file and then write to the cache
		scoped_istream stream = preprocess_file(path, &defines_map, &error_log);

		//reset the parse counter before reading the game files
		if (loadscreen::global_loadscreen) {
			loadscreen::global_loadscreen->parser_counter = 0;
		}

		read(cfg, *stream, &error_log);
		if (!error_log.empty())
		{
			throw config::error(error_log);
		}
	}

	void config_cache::read_cache(const std::string& path, config& cfg)
	{
		bool is_valid = true;
		std::stringstream defines_string;
		defines_string << path;
		for(preproc_map::const_iterator i = defines_map_.begin(); i != defines_map_.end(); ++i) {
			if(i->second.value != "" || i->second.arguments.empty() == false) {
				is_valid = false;
				ERR_CONFIG << "Preproc define not valid\n";
				break;
			}

			defines_string << " " << i->first;
		}

		// Do cache check only if  define map is valid and
		// caching is allowed
		if(is_valid) {
			const std::string& cache = get_cache_dir();
			if(cache != "") {
				sha1_hash sha(defines_string.str()); // use a hash for a shorter display of the defines
				const std::string fname = cache + "/cache-v" + game_config::version + "-" + sha.display();
				const std::string fname_checksum = fname + ".checksum";

				file_tree_checksum dir_checksum;

				if(!force_valid_cache_) {
					try {
						if(file_exists(fname_checksum)) {
							config checksum_cfg;
							read_file(fname_checksum, checksum_cfg);
							dir_checksum = file_tree_checksum(checksum_cfg);
						}
					} catch(config::error&) {
						ERR_CONFIG << "cache checksum is corrupt\n";
					} catch(io_exception&) {
						ERR_CONFIG << "error reading cache checksum\n";
					}
				}

				if(force_valid_cache_) {
					LOG_CONFIG << "skipping cache validation (forced)\n";
				}

				if(file_exists(fname) && (force_valid_cache_ || (dir_checksum == data_tree_checksum()))) {
					LOG_CONFIG << "found valid cache at '" << fname << "' using it\n";
					log_scope("read cache");
					try {
						read_file(fname,cfg);
						return;
					} catch(config::error&) {
						ERR_CONFIG << "cache is corrupt. Loading from files\n";
					} catch(io_exception&) {
						ERR_CONFIG << "error reading cache. Loading from files\n";
					}
				}

				LOG_CONFIG << "no valid cache found. Writing cache to '" << fname << " with defines_map "<< defines_string.str() << "'\n";

				read_configs(path, cfg);

				try {
					write_file(fname, cfg);
					config checksum_cfg;
					data_tree_checksum().write(checksum_cfg);
					write_file(fname_checksum, checksum_cfg);
				} catch(io_exception&) {
					ERR_FS << "could not write to cache '" << fname << "'\n";
				}
				return;
			}
		}
		LOG_CONFIG << "Loading plain config instead of cache\n";
		load_configs(path, cfg);
	}

	void config_cache::load_configs(const std::string& path, config& cfg)
	{
		if (use_cache_)
		{
			read_cache(path, cfg);
		} else {
			read_configs(path, cfg);
		}

		return;
		ERR_CONFIG << "caching cannot be done. Reading file\n";

		std::string error_log;

		read_configs(cfg, error_log);
		if(!error_log.empty()) {
			gui::show_error_message(*game_display::get_singleton(),
					_("Warning: Errors occurred while loading game configuration files: '") +
					font::nullify_markup(error_log));

		}
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
		defines_map_[define] = preproc_define();
	}

	void config_cache::remove_define(const std::string& define)
	{
		defines_map_.erase(define);
	}
}
