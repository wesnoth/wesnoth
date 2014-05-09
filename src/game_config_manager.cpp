/*
   Copyright (C) 2013 - 2014 by Andrius Silinskas <silinskas.andrius@gmail.com>
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
#include "gui/dialogs/wml_error.hpp"
#include "language.hpp"
#include "loadscreen.hpp"
#include "log.hpp"
#include "preferences.hpp"
#include "resources.hpp"
#include "scripting/lua.hpp"
#include "hotkey/hotkey_item.hpp"
#include "hotkey/hotkey_command.hpp"

#include <boost/foreach.hpp>

static lg::log_domain log_config("config");
#define ERR_CONFIG LOG_STREAM(err, log_config)
#define WRN_CONFIG LOG_STREAM(warn, log_config)
#define LOG_CONFIG LOG_STREAM(info, log_config)

game_config_manager::game_config_manager(
		const commandline_options& cmdline_opts,
		game_display& display,
		const bool jump_to_editor) :
	cmdline_opts_(cmdline_opts),
	disp_(display),
	jump_to_editor_(jump_to_editor),
	game_config_(),
	old_defines_map_(),
	paths_manager_(),
	cache_(game_config::config_cache::instance())
{
	resources::config_manager = this;

	if(cmdline_opts_.nocache) {
		cache_.set_use_cache(false);
	}
	if(cmdline_opts_.validcache) {
		cache_.set_force_valid_cache(true);
	}
}

game_config_manager::~game_config_manager()
{
	resources::config_manager = NULL;
}

bool game_config_manager::init_game_config(FORCE_RELOAD_CONFIG force_reload)
{
	// Add preproc defines according to the command line arguments.
	game_config::scoped_preproc_define multiplayer("MULTIPLAYER",
		cmdline_opts_.multiplayer);
	game_config::scoped_preproc_define test("TEST", cmdline_opts_.test);
	game_config::scoped_preproc_define editor("EDITOR", jump_to_editor_);
	game_config::scoped_preproc_define title_screen("TITLE_SCREEN",
		!cmdline_opts_.multiplayer && !cmdline_opts_.test && !jump_to_editor_);

	load_game_config(force_reload);

	game_config::load_config(game_config_.child("game_config"));

	hotkey::deactivate_all_scopes();
	hotkey::set_scope_active(hotkey::SCOPE_GENERAL);
	hotkey::set_scope_active(hotkey::SCOPE_MAIN_MENU);

	hotkey::load_hotkeys(game_config(), true);
	::init_textdomains(game_config());
	about::set_about(game_config());
	ai::configuration::init(game_config());

	return true;
}

void game_config_manager::load_game_config(FORCE_RELOAD_CONFIG force_reload,
	game_classification const* classification)
{
	// Make sure that 'debug mode' symbol is set
	// if command line parameter is selected
	// also if we're in multiplayer and actual debug mode is disabled.
	game_config::scoped_preproc_define debug_mode("DEBUG_MODE",
	    game_config::debug || game_config::mp_debug);

	// Game_config already holds requested config in memory.
	if(!game_config_.empty() &&
		(force_reload == NO_FORCE_RELOAD)
		&& old_defines_map_ == cache_.get_preproc_map()) {
		return;
	}

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

		// Load the selected core
		cache_.get_config(get_wml_location(preferences::wml_tree_root()), game_config_);
		// Load the mainline core definitions to make sure switching back is always possible.
		config default_core_cfg;
		cache_.get_config(game_config::path, default_core_cfg);
		game_config_.append(default_core_cfg);

		main_transaction.lock();

		// Put the gfx rules aside so that we can prepend the add-on
		// rules to them.
		config core_terrain_rules;
		core_terrain_rules.splice_children(game_config_, "terrain_graphics");

		load_addons_cfg();

		// If multiplayer campaign is being loaded, [scenario] tags should
		// become [multiplayer] tags and campaign's id should be added to them
		// to allow to recognize which scenarios belongs to a loaded campaign.
		if (classification != NULL) {
			if (classification->campaign_type == "multiplayer" &&
				!classification->campaign_define.empty()) {

				const config& campaign = game_config().find_child("campaign",
					"define", classification->campaign_define);
				const std::string& campaign_id = campaign["id"];
				const bool require_campaign =
					campaign["require_campaign"].to_bool(true);

				const config::const_child_itors &ci =
					game_config().child_range("scenario");
				std::vector<config> scenarios(ci.first, ci.second);

				game_config_.clear_children("scenario");

				BOOST_FOREACH(config& cfg, scenarios) {
					cfg["campaign_id"] = campaign_id;
					cfg["require_scenario"] = require_campaign;
					game_config_.add_child("multiplayer", cfg);
				}
			}
		}

		// Extract the Lua scripts at toplevel.
		extract_preload_scripts(game_config_);
		game_config_.clear_children("lua");

		// Put the gfx rules back to game config.
		game_config_.splice_children(core_terrain_rules, "terrain_graphics");

		set_multiplayer_hashes();
		set_color_info();
		set_unit_data();

		terrain_builder::set_terrain_rules_cfg(game_config());
		::init_strings(game_config());
		theme::set_known_themes(&game_config());
	} catch(game::error& e) {
		ERR_CONFIG << "Error loading game configuration files\n" << e.message << '\n';

		if (preferences::wml_tree_root() != "/"){
			gui2::twml_error::display(
					_("Error loading custom game configuration files. The game will fallback to the default files."),
					e.message, disp_.video());
			preferences::set_wml_tree_root("/");
			preferences::set_core_id("default");
			load_game_config(force_reload, classification);
		} else {
			gui2::twml_error::display(
					_("Error loading default game configuration files. The game will now exit."),
					e.message, disp_.video());
			throw;
		}
	}

	old_defines_map_ = cache_.get_preproc_map();

	// Set new binary paths.
	paths_manager_.set_paths(game_config());
}

void game_config_manager::load_addons_cfg()
{
	const std::string user_campaign_dir = get_addon_campaigns_dir();

	std::vector<std::string> error_addons;
	std::vector<std::string> user_dirs;
	std::vector<std::string> user_files;
	std::vector<std::string> addons_to_load;

	get_files_in_dir(user_campaign_dir, &user_files, &user_dirs,
		ENTIRE_FILE_PATH);

	std::vector<std::string> error_log;

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
				if(dirs.size() > 0) {
					ok = false;
				}
				if(files.size() > 1) {
					ok = false;
				}
				if(files.size() == 1 && files[0] != "_info.cfg") {
					ok = false;
				}
			}
			if(!ok) {
				const int userdata_loc = file.find("data/add-ons") + 5;
				ERR_CONFIG << "error reading usermade add-on '"
					<< file << "'\n";
				error_addons.push_back(file);
				error_log.push_back("The format '~" + file.substr(userdata_loc)
					+ "' is only for single-file add-ons, use '~"
					+ file.substr(userdata_loc,
						size_minus_extension - userdata_loc)
					+ "/_main.cfg' instead.");
			}
			else {
				addons_to_load.push_back(file);
			}
		}
	}

	// Append the $user_campaign_dir/*/_main.cfg files to addons_to_load.
	BOOST_FOREACH(const std::string& uc, user_dirs) {

		const std::string info_cfg = uc + "/_info.cfg";
		if (file_exists(info_cfg)) {
		
			config info;
			cache_.get_config(info_cfg, info);
			const config info_tag = info.child_or_empty("info");
			std::string core = info_tag["core"];
			if (core.empty()) core = "default";
			if ( !info_tag.empty() && // Don't skip addons which have no [info], they are most likely manually installed.
					info_tag["type"] != "core" && // Don't skip cores, we want them selectable at all times.
					core != preferences::core_id() // Don't skip addons matching our current core.
			)
				continue; // Skip add-ons not matching our current core.
		}

		const std::string main_cfg = uc + "/_main.cfg";
		if(file_exists(main_cfg)) {
			addons_to_load.push_back(main_cfg);
		}
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
			ERR_CONFIG << err.message << '\n';
			error_addons.push_back(uc);
			error_log.push_back(err.message);
		} catch(preproc_config::error& err) {
			ERR_CONFIG << "error reading usermade add-on '" << uc << "'\n";
			ERR_CONFIG << err.message << '\n';
			error_addons.push_back(uc);
			error_log.push_back(err.message);
		} catch(io_exception&) {
			ERR_CONFIG << "error reading usermade add-on '" << uc << "'\n";
			error_addons.push_back(uc);
		}
	}
	if(error_addons.empty() == false) {
		const size_t n = error_addons.size();
		const std::string& msg1 =
			_n("The following add-on had errors and could not be loaded:",
			   "The following add-ons had errors and could not be loaded:",
			   n);
		const std::string& msg2 =
			_n("Please report this to the author or maintainer of this add-on.",
			   "Please report this to the respective authors or maintainers of these add-ons.",
			   n);

		const std::string& report = utils::join(error_log, "\n\n");

		gui2::twml_error::display(msg1, msg2, error_addons, report,
								  disp_.video());
	}
}

void game_config_manager::set_multiplayer_hashes()
{
	config& hashes = game_config_.add_child("multiplayer_hashes");
	BOOST_FOREACH(const config &ch, game_config_.child_range("multiplayer")) {
		hashes[ch["id"]] = ch.hash();
	}
}

void game_config_manager::set_color_info()
{
	config colorsys_info;
	colorsys_info.splice_children(game_config_, "color_range");
	colorsys_info.splice_children(game_config_, "color_palette");
	game_config::add_color_info(colorsys_info);
}

void game_config_manager::set_unit_data()
{
	game_config_.merge_children("units");
	loadscreen::start_stage("load unit types");
	if(config &units = game_config_.child("units")) {
		unit_types.set_config(units);
	}
}

void game_config_manager::reload_changed_game_config()
{
	// Rebuild addon version info cache.
	refresh_addon_version_info_cache();

	// Force a reload of configuration information.
	cache_.recheck_filetree_checksum();
	old_defines_map_.clear();
	clear_binary_paths_cache();
	init_game_config(FORCE_RELOAD);
}

void game_config_manager::load_game_config_for_editor()
{
	game_config::scoped_preproc_define editor("EDITOR");
	load_game_config(NO_FORCE_RELOAD);
}

void game_config_manager::load_game_config_for_game(
	const game_classification& classification)
{
	game_config::scoped_preproc_define difficulty(classification.difficulty,
		!classification.difficulty.empty());
	game_config::scoped_preproc_define campaign(classification.campaign_define,
		!classification.campaign_define.empty());
	game_config::scoped_preproc_define multiplayer("MULTIPLAYER",
		classification.campaign_type == "multiplayer");

	typedef boost::shared_ptr<game_config::scoped_preproc_define> define;
	std::deque<define> extra_defines;
	BOOST_FOREACH(const std::string& extra_define,
		classification.campaign_xtra_defines) {
		define new_define
			(new game_config::scoped_preproc_define(extra_define));
		extra_defines.push_back(new_define);
	}

	try{
		load_game_config(NO_FORCE_RELOAD, &classification);
	}
	catch(game::error&) {
		cache_.clear_defines();

		std::deque<define> previous_defines;
		BOOST_FOREACH(const preproc_map::value_type& preproc, old_defines_map_) {
			define new_define
				(new game_config::scoped_preproc_define(preproc.first));
			previous_defines.push_back(new_define);
		}

		load_game_config(NO_FORCE_RELOAD);

		throw;
	}
}

