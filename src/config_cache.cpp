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
		game_config_(), 
		force_valid_cache_(false),
		use_cache_(true),
		dirty_(true),
		config_root_("data/"),
		user_config_root_(get_addon_campaigns_dir()),
		defines_map_()
	{
		// To settup initial defines map correctly
		clear_defines();
	}

	std::string config_cache::get_config_root() const
	{
		return config_root_;
	}

	void config_cache::set_config_root(const std::string& path)
	{
		if (path == config_root_)
			return;
		dirty_ = true;
		config_root_ = path;
	}

	std::string config_cache::get_user_config_root() const
	{
		return user_config_root_;
	}

	void config_cache::set_user_config_root(const std::string& path)
	{
		if (path == user_config_root_)
			return;
		dirty_ = true;
		user_config_root_ = path;
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

	void config_cache::reload_translations()
	{
		if (dirty_)
		{
			load_configs(game_config_, false);
		} else {
			game_config_.reset_translation();
			game_config::load_config(game_config_.child("game_config"));
		}
	}

	config& config_cache::get_config(bool recheck_cache)
	{
		if (!dirty_)
			return game_config_;

		load_configs(game_config_, recheck_cache);
		dirty_ = false;

		return game_config_;
	}

	void config_cache::read_configs(config& cfg, std::string& error_log)
	{
		preproc_map defines_map(defines_map_);

		std::string user_error_log;
		//read the file and then write to the cache
		scoped_istream stream = preprocess_file(config_root_, &defines_map, &error_log);

		//reset the parse counter before reading the game files
		if (loadscreen::global_loadscreen) {
			loadscreen::global_loadscreen->parser_counter = 0;
		}

		read(cfg, *stream, &error_log);
		// clone and put the gfx rules aside so that we can prepend the add-on
		// rules to them.
		config core_terrain_rules;
		// FIXME: there should be a canned algorithm for cloning child_list objects,
		// along with the memory their elements point to... little implementation detail.
		foreach(config const* p_cfg, game_config_.get_children("terrain_graphics")) {
			core_terrain_rules.add_child("terrain_graphics", *p_cfg);
		}
		game_config_.clear_children("terrain_graphics");

		// load usermade add-ons
//		const std::string user_campaign_dir = get_addon_campaigns_dir();
		std::vector< std::string > error_addons;
		// Scan addon directories
		std::vector<std::string> user_addons;

		get_files_in_dir(user_config_root_,NULL,&user_addons,ENTIRE_FILE_PATH);

		// Load the addons
		for(std::vector<std::string>::const_iterator uc = user_addons.begin(); uc != user_addons.end(); ++uc) {
			std::string oldstyle_cfg = *uc + ".cfg";
			std::string main_cfg = *uc + "/_main.cfg";
			std::string toplevel;
			if (file_exists(oldstyle_cfg))
				toplevel = oldstyle_cfg;
			else if (file_exists(main_cfg))
				toplevel = main_cfg;
			else
				continue;

			try {
				preproc_map addon_defines_map(defines_map);
				scoped_istream stream = preprocess_file(toplevel, &addon_defines_map);

				std::string addon_error_log;

				config umc_cfg;
				read(umc_cfg, *stream, &addon_error_log);

				if (addon_error_log.empty()) {
					game_config_.append(umc_cfg);
				} else {
					user_error_log += addon_error_log;
					error_addons.push_back(*uc);
				}
			} catch(config::error& err) {
				ERR_CONFIG << "error reading usermade add-on '" << *uc << "'\n";
				error_addons.push_back(*uc);
				user_error_log += err.message + "\n";
			} catch(preproc_config::error&) {
				ERR_CONFIG << "error reading usermade add-on '" << *uc << "'\n";
				error_addons.push_back(*uc);
				//no need to modify the error log here, already done by the preprocessor
			} catch(io_exception&) {
				ERR_CONFIG << "error reading usermade add-on '" << *uc << "'\n";
				error_addons.push_back(*uc);
			}
			if(error_addons.empty() == false) {
				std::stringstream msg;
				msg << _n("The following add-on had errors and could not be loaded:",
						"The following add-ons had errors and could not be loaded:",
						error_addons.size());
				for(std::vector<std::string>::const_iterator i = error_addons.begin(); i != error_addons.end(); ++i) {
					msg << "\n" << *i;
				}

				msg << "\n" << _("ERROR DETAILS:") << "\n" << font::nullify_markup(user_error_log);

				gui::show_error_message(*game_display::get_singleton(),msg.str());
			}
		}

		cfg.merge_children("units");
		cfg.append(core_terrain_rules);

	}

	void config_cache::load_configs(config& cfg, bool recheck_cache)
	{
		bool is_valid = true;
		std::stringstream defines_string;
		for(preproc_map::const_iterator i = defines_map_.begin(); i != defines_map_.end(); ++i) {
			if(i->second.value != "" || i->second.arguments.empty() == false) {
				is_valid = false;
				ERR_CONFIG << "Preproc define not valid\n";
				break;
			}

			defines_string << " " << i->first;
		}
		//std::string localename = get_locale().localename;
		//str << "-lang_" << (localename.empty() ? "default" : localename);

		if(is_valid && use_cache_) {
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
							scoped_istream stream = istream_file(fname_checksum);
							read(checksum_cfg, *stream);
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

				if(file_exists(fname) && (force_valid_cache_ || (dir_checksum == data_tree_checksum(recheck_cache)))) {
					LOG_CONFIG << "found valid cache at '" << fname << "' using it\n";
					log_scope("read cache");
					try {
						scoped_istream stream = istream_file(fname);
						read_compressed(cfg, *stream);
						return;
					} catch(config::error&) {
						ERR_CONFIG << "cache is corrupt. Loading from files\n";
					} catch(io_exception&) {
						ERR_CONFIG << "error reading cache. Loading from files\n";
					}
				}

				LOG_CONFIG << "no valid cache found. Writing cache to '" << fname << " with defines_map "<< defines_string.str() << "'\n";
				DBG_CONFIG << ((use_cache_ && file_exists(fname)) ? "yes":"no ") << " " << dir_checksum.modified << "==" << data_tree_checksum().modified << "  " << dir_checksum.nfiles << "==" << data_tree_checksum().nfiles << "  " << dir_checksum.sum_size << "==" << data_tree_checksum().sum_size << "\n";

				std::string error_log;

				read_configs(cfg, error_log);

				if(!error_log.empty()) {
					gui::show_error_message(*game_display::get_singleton(),
							_("Warning: Errors occurred while loading game configuration files: '") +
							font::nullify_markup(error_log));

				} else {
					try {
						scoped_ostream cache = ostream_file(fname);
						write_compressed(*cache, cfg);
						config checksum_cfg;
						data_tree_checksum().write(checksum_cfg);
						scoped_ostream checksum = ostream_file(fname_checksum);
						write(*checksum, checksum_cfg);
					} catch(io_exception&) {
						ERR_FS << "could not write to cache '" << fname << "'\n";
					}
				}

				return;
			}
		}

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

	void config_cache::add_define(const std::string& define)
	{
		dirty_ = true;
		defines_map_[define] = preproc_define();
	}
}
