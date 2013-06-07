/*
   Copyright (C) 2013 by Andrius Silinskas <silinskas.andrius@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include "game_config_manager.hpp"

#include "about.hpp"
#include "addon/manager.hpp"
#include "ai/configuration.hpp"
#include "builder.hpp"
#include "cursor.hpp"
#include "game_config.hpp"
#include "gettext.hpp"
#include "gui/dialogs/message.hpp"
#include "language.hpp"
#include "loadscreen.hpp"
#include "log.hpp"
#include "resources.hpp"
#include "scripting/lua.hpp"

#include <boost/foreach.hpp>

static lg::log_domain log_config("config");
#define ERR_CONFIG LOG_STREAM(err, log_config)
#define WRN_CONFIG LOG_STREAM(warn, log_config)
#define LOG_CONFIG LOG_STREAM(info, log_config)

game_config_manager::game_config_manager(
		const commandline_options& cmdline_opts,
		game_display& display) :
	cmdline_opts_(cmdline_opts),
	disp_(display),
	game_config_(),
	defines_(),
	old_defines_map_(),
	paths_manager_(),
	cache_(game_config::config_cache::instance())
{
	resources::config_manager = this;

	if (cmdline_opts_.nocache)
		cache_.set_use_cache(false);
	if (cmdline_opts_.validcache)
		cache_.set_force_valid_cache(true);
}

game_config_manager::~game_config_manager()
{
	resources::config_manager = NULL;
}

void game_config_manager::add_define(const std::string& name, const bool add)
{
	defines_.push_back(std::make_pair(name, add));
}

void game_config_manager::add_cache_define(const std::string& name)
{
	cache_.add_define(name);
}

void game_config_manager::clear_cache_defines()
{
	cache_.clear_defines();
}

bool game_config_manager::init_config(FORCE_RELOAD_CONFIG force_reload,
                                      const bool jump_to_editor)
{
	load_game_cfg(SET_PATHS, CLEAR_CACHE, force_reload);

	// Make sure that multiplayer mode is set
	// if command line parameter is selected.
	if (cmdline_opts_.multiplayer) {
		cache_.add_define("MULTIPLAYER");
	}

	if (cmdline_opts_.test) {
		cache_.add_define("TEST");
	}

	if (jump_to_editor) {
		cache_.add_define("EDITOR");
	}

	if (!cmdline_opts_.multiplayer && !cmdline_opts_.test && !jump_to_editor) {
		cache_.add_define("TITLE_SCREEN");
	}

	game_config::load_config(game_config_.child("game_config"));

	hotkey::deactivate_all_scopes();
	hotkey::set_scope_active(hotkey::SCOPE_GENERAL);
	hotkey::set_scope_active(hotkey::SCOPE_GAME);

	hotkey::load_hotkeys(game_config(), true);
	::init_textdomains(game_config());
	about::set_about(game_config());
	ai::configuration::init(game_config());

	return true;
}

void game_config_manager::load_game_cfg(
    SET_BINARY_PATHS set_paths,
    CLEAR_CACHE_DEFINES clear_cache,
    FORCE_RELOAD_CONFIG force_reload)
{
	if (clear_cache == CLEAR_CACHE)
		cache_.clear_defines();

	typedef boost::shared_ptr<game_config::scoped_preproc_define> define_ptr;
	std::deque<define_ptr> defines;

	typedef std::pair<std::string, bool> def_pair;
	BOOST_FOREACH(const def_pair& define, defines_) {
		define_ptr newdefine
		    (new game_config::scoped_preproc_define(define.first,
			                                        define.second));
		defines.push_back(newdefine);
	}

	defines_.clear();

	// Make sure that 'debug mode' symbol is set
	// if command line parameter is selected
	// also if we're in multiplayer and actual debug mode is disabled.
	if (game_config::debug || game_config::mp_debug) {
		cache_.add_define("DEBUG_MODE");
	}

	if (!game_config_.empty() &&
	    (force_reload == NO_FORCE_RELOAD)
	    && old_defines_map_ == cache_.get_preproc_map())
		return; // Game_config already holds requested config in memory.
	old_defines_map_ = cache_.get_preproc_map();
	loadscreen::global_loadscreen_manager loadscreen_manager(disp_.video());
	cursor::setter cur(cursor::WAIT);
	// The loadscreen will erase the titlescreen.
	// NOTE: even without loadscreen, needed after MP lobby.
	try {
		// Read all game configs.
		// First we should load data/,
		// then handle terrains so that they are last loaded from data/.
		// 2nd everything in userdata.
		loadscreen::start_stage("verify cache");
		data_tree_checksum();
		loadscreen::start_stage("create cache");

		// Start transaction so macros are shared.
		game_config::config_cache_transaction main_transaction;

		cache_.get_config(game_config::path +"/data", game_config_);

		main_transaction.lock();

		// Put the gfx rules aside so that we can prepend the add-on
		// rules to them.
		config core_terrain_rules;
		core_terrain_rules.splice_children(game_config_, "terrain_graphics");

		// Load usermade add-ons.
		const std::string user_campaign_dir = get_addon_campaigns_dir();
		std::vector< std::string > error_addons;
		// Scan addon directories.
		std::vector<std::string> user_dirs;
		// Scan for standalone files.
		std::vector<std::string> user_files;

		// The addons that we'll actually load.
		std::vector<std::string> addons_to_load;

		get_files_in_dir(user_campaign_dir,&user_files,&user_dirs,
		                 ENTIRE_FILE_PATH);
		std::stringstream user_error_log;

		// Append the $user_campaign_dir/*.cfg files to addons_to_load.
		BOOST_FOREACH(const std::string& uc, user_files) {
			const std::string file = uc;
			const int size_minus_extension = file.size() - 4;
			if(file.substr(size_minus_extension, file.size()) == ".cfg") {
				bool ok = true;
				// Allowing it if the dir doesn't exist,
				// for the single-file add-on.
				if(file_exists(file.substr(0, size_minus_extension))) {
					// Unfortunately, we create the dir plus
					// _info.cfg ourselves on download.
					std::vector<std::string> dirs, files;
					get_files_in_dir(file.substr(0, size_minus_extension),
					                 &files, &dirs);
					if(dirs.size() > 0)
						ok = false;
					if(files.size() > 1)
						ok = false;
					if(files.size() == 1 && files[0] != "_info.cfg")
						ok = false;
				}
				if(!ok) {
					const int userdata_loc = file.find("data/add-ons") + 5;
					ERR_CONFIG << "error reading usermade add-on '"
					           << file << "'\n";
					error_addons.push_back(file);
					user_error_log << "The format '~" << file.substr(userdata_loc) << "' is only for single-file add-ons, use '~" << file.substr(userdata_loc, size_minus_extension - userdata_loc) << "/_main.cfg' instead.\n";
				} else
					addons_to_load.push_back(file);
			}
		}

		// Append the $user_campaign_dir/*/_main.cfg files to addons_to_load.
		BOOST_FOREACH(const std::string& uc, user_dirs) {
			const std::string main_cfg = uc + "/_main.cfg";
			if (file_exists(main_cfg))
				addons_to_load.push_back(main_cfg);
		}

		// Load the addons.
		BOOST_FOREACH(const std::string& uc, addons_to_load) {
			const std::string toplevel = uc;
			try {
				config umc_cfg;
				cache_.get_config(toplevel, umc_cfg);

				game_config_.append(umc_cfg);
			} catch(config::error& err) {
				ERR_CONFIG << "error reading usermade add-on '" << uc << "'\n";
				error_addons.push_back(uc);
				user_error_log << err.message << "\n";
			} catch(preproc_config::error& err) {
				ERR_CONFIG << "error reading usermade add-on '" << uc << "'\n";
				error_addons.push_back(uc);
				user_error_log << err.message << "\n";
			} catch(io_exception&) {
				ERR_CONFIG << "error reading usermade add-on '" << uc << "'\n";
				error_addons.push_back(uc);
			}
		}
		if(error_addons.empty() == false) {
			std::stringstream msg;
			msg << _n("The following add-on had errors and could not be loaded:",
					"The following add-ons had errors and could not be loaded:",
					error_addons.size());
			BOOST_FOREACH(const std::string& error_addon, error_addons) {
				msg << "\n" << error_addon;
			}

			msg << '\n' << _("ERROR DETAILS:") << '\n' << user_error_log.str();

			gui2::show_error_message(disp_.video(),msg.str());
		}

		// Extract the Lua scripts at toplevel.
		extract_preload_scripts(game_config_);
		game_config_.clear_children("lua");

		config colorsys_info;
		colorsys_info.splice_children(game_config_, "color_range");
		colorsys_info.splice_children(game_config_, "color_palette");

		game_config_.merge_children("units");
		game_config_.splice_children(core_terrain_rules, "terrain_graphics");

		config& hashes = game_config_.add_child("multiplayer_hashes");
		BOOST_FOREACH(const config &ch, game_config_.child_range("multiplayer")) {
			hashes[ch["id"]] = ch.hash();
		}

		game_config::add_color_info(colorsys_info);

		loadscreen::start_stage("load unit types");
		if (config &units = game_config_.child("units")) {
			unit_types.set_config(units);
		}

		terrain_builder::set_terrain_rules_cfg(game_config());

		::init_strings(game_config());

		theme::set_known_themes(&game_config());

	} catch(game::error& e) {
		ERR_CONFIG << "Error loading game configuration files\n";
		gui2::show_error_message(disp_.video(), _("Error loading game configuration files: '") +
			e.message + _("' (The game will now exit)"));
		throw;
	}

	if (set_paths == SET_PATHS)
		paths_manager_.set_paths(game_config());
}

void game_config_manager::reload_changed_game_config(const bool jump_to_editor)
{
	// Rebuild addon version info cache.
	refresh_addon_version_info_cache();

	// Force a reload of configuration information.
	cache_.recheck_filetree_checksum();
	old_defines_map_.clear();
	clear_binary_paths_cache();
	init_config(FORCE_RELOAD, jump_to_editor);
}
