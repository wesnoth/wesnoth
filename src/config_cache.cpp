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
#include "log.hpp"
#include "marked-up_text.hpp"
#include "show_dialog.hpp"
#include "sha1.hpp"
#include "serialization/binary_wml.hpp"
#include "serialization/binary_or_text.hpp"
#include "serialization/parser.hpp"

#include <boost/bind.hpp>

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
		defines_map_(),
		defines_active_map_()
	{
		// To settup initial defines map correctly
		clear_defines();
	}

	struct output {
		void operator()(const preproc_map::value_type& def)
		{
			std::cout << "key: " << def.first << " " << def.second << "\n";
		}
	};
	const preproc_map& config_cache::get_preproc_map() const
	{
		return defines_map_;
	}

	void config_cache::clear_defines()
	{
		LOG_CONFIG << "Clearing defines map!\n";
		defines_map_.clear();
		defines_active_map_.clear();
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

	void config_cache::get_config(const std::string& path, config& cfg)
	{
		load_configs(path, cfg);
	}

	config_ptr config_cache::get_config(const std::string& path)
	{
		config_ptr ret(new config());
		load_configs(path, *ret);

		return ret;
	}


	void config_cache::write_file(std::string path, const config& cfg)
	{
		preproc_map temp;
		write_file(path,cfg, temp);
	}
	void config_cache::write_file(std::string path, const config& cfg, const preproc_map& defines_map)
	{
		scoped_ostream stream = ostream_file(path);
		const bool gzip = true;
		config_writer writer(*stream,gzip,"",game_config::cache_compression_level);
		writer.write(cfg);
		if (!defines_map.empty())
		{
			writer.write(defines_map);
		}
	}

	void config_cache::read_file(const std::string& path, config& cfg)
	{
		scoped_istream stream = istream_file(path);
		read_gz(cfg, *stream);
	}
	
	void config_cache::read_file(const std::string& path, config& cfg, preproc_map& /*defines_map*/)
	{
		std::string error_log;
		scoped_istream stream = istream_file(path);
		read_gz(cfg, *stream, &error_log);

		if (!error_log.empty())
		{
			throw config::error(error_log);
		}

	}
	preproc_map& config_cache::make_copy_map()
	{
		if(config_cache_transaction::get_state() == config_cache_transaction::FREE)
		{
			defines_active_map_.clear();
			std::copy(defines_map_.begin(),
					defines_map_.end(),
					std::insert_iterator<preproc_map>(defines_active_map_, defines_active_map_.begin()));
		}
		return defines_active_map_;
	}

	void config_cache::insert_to_active(const preproc_map::value_type& def)
	{
		defines_active_map_[def.first] = def.second;
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
		switch(config_cache_transaction::get_state())
		{
			case config_cache_transaction::FREE:
				defines_active_map_.clear();
				defines_map.clear();
				break;
			case config_cache_transaction::START:
				{
					preproc_map temp;
					std::set_difference(defines_map.begin(),
							defines_map.end(),
							defines_active_map_.begin(),
							defines_active_map_.end(),
							std::insert_iterator<preproc_map>(temp,temp.begin()),
							&compare_define);
					std::for_each(temp.begin(), temp.end(),boost::bind(&config_cache::insert_to_active,this, _1));

					temp.swap(defines_map);
				}
				break;
			case config_cache_transaction::LOCK:
				defines_map.clear();
				break;
		}
	}


	void config_cache::read_configs(const std::string& path, config& cfg, preproc_map& defines_map)
	{
		std::string error_log;
		//read the file and then write to the cache
		scoped_istream stream = preprocess_file(path, &defines_map, &error_log);

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
				ERR_CONFIG << "Preprocessor define not valid\n";
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
					LOG_CONFIG << "found valid cache at '" << fname << "' with defines_map " << defines_string.str() << "\n";
					log_scope("read cache");
					try {
						preproc_map defines_map;
						read_file(fname,cfg, defines_map);
						add_defines_map_diff(defines_map);
						return;
					} catch(config::error& e) {
						ERR_CONFIG << "cache " << fname << " is corrupt. Loading from files: "<< e.message<<"\n";
					} catch(io_exception&) {
						ERR_CONFIG << "error reading cache " << fname << ". Loading from files\n";
					}
				}

				LOG_CONFIG << "no valid cache found. Writing cache to '" << fname << " with defines_map "<< defines_string.str() << "'\n";
				preproc_map copy_map(make_copy_map());

				read_configs(path, cfg, copy_map);

				add_defines_map_diff(copy_map);

				try {
					write_file(fname, cfg,copy_map);
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
		preproc_map copy_map(make_copy_map());
		read_configs(path, cfg, copy_map);
		add_defines_map_diff(copy_map);
	}

	void config_cache::load_configs(const std::string& path, config& cfg)
	{
		if (use_cache_)
		{
			read_cache(path, cfg);
		} else {
			preproc_map copy_map(make_copy_map());
			read_configs(path, cfg, copy_map);
			add_defines_map_diff(copy_map);
		}

		return;
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
		DBG_CONFIG << "adding define: " << define << "\n";
		defines_map_[define] = preproc_define();
	}

	void config_cache::remove_define(const std::string& define)
	{
		DBG_CONFIG << "removing define: " << define << "\n";
		defines_map_.erase(define);
	}

	config_cache_transaction::state config_cache_transaction::transaction_ = FREE;
	
	config_cache_transaction::config_cache_transaction()
	{
		assert(transaction_ == FREE);
		transaction_ = START;
	}
	
	config_cache_transaction::~config_cache_transaction()
	{
		transaction_ = FREE;
	}

	void config_cache_transaction::lock()
	{
		transaction_ = LOCK;			
	}

}
