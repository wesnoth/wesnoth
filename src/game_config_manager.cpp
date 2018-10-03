/*
   Copyright (C) 2013 - 2018 by Andrius Silinskas <silinskas.andrius@gmail.com>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
#include "cursor.hpp"
#include "events.hpp"
#include "formatter.hpp"
#include "game_config.hpp"
#include "gettext.hpp"
#include "game_classification.hpp"
#include "gui/dialogs/loading_screen.hpp"
#include "gui/dialogs/wml_error.hpp"
#include "hotkey/hotkey_item.hpp"
#include "hotkey/hotkey_command.hpp"
#include "language.hpp"
#include "log.hpp"
#include "preferences/general.hpp"
#include "scripting/game_lua_kernel.hpp"
#include "terrain/builder.hpp"
#include "terrain/type_data.hpp"
#include "units/types.hpp"
#include "game_version.hpp"
#include "theme.hpp"
#include "picture.hpp"

static lg::log_domain log_config("config");
#define ERR_CONFIG LOG_STREAM(err, log_config)
#define WRN_CONFIG LOG_STREAM(warn, log_config)
#define LOG_CONFIG LOG_STREAM(info, log_config)

static game_config_manager * singleton;

game_config_manager::game_config_manager(
		const commandline_options& cmdline_opts,
		const bool jump_to_editor) :
	cmdline_opts_(cmdline_opts),
	jump_to_editor_(jump_to_editor),
	game_config_(),
	old_defines_map_(),
	paths_manager_(),
	cache_(game_config::config_cache::instance())
{
	assert(!singleton);
	singleton = this;

	if(cmdline_opts_.nocache) {
		cache_.set_use_cache(false);
	}
	if(cmdline_opts_.validcache) {
		cache_.set_force_valid_cache(true);
	}
}

game_config_manager::~game_config_manager()
{
	assert(singleton);
	singleton = nullptr;
}

game_config_manager * game_config_manager::get() {
	return singleton;
}

bool game_config_manager::init_game_config(FORCE_RELOAD_CONFIG force_reload)
{
	// Add preproc defines according to the command line arguments.
	game_config::scoped_preproc_define multiplayer("MULTIPLAYER",
		cmdline_opts_.multiplayer);
	game_config::scoped_preproc_define test("TEST", bool(cmdline_opts_.test));
	game_config::scoped_preproc_define mptest("MP_TEST", cmdline_opts_.mptest);
	game_config::scoped_preproc_define editor("EDITOR", jump_to_editor_);
	game_config::scoped_preproc_define title_screen("TITLE_SCREEN",
		!cmdline_opts_.multiplayer && !cmdline_opts_.test && !jump_to_editor_);

	game_config::reset_color_info();
	load_game_config_with_loadscreen(force_reload);

	game_config::load_config(game_config_.child("game_config"));

	hotkey::deactivate_all_scopes();
	hotkey::set_scope_active(hotkey::SCOPE_MAIN_MENU);

	// Load the standard hotkeys, then apply any player customizations.
	hotkey::load_hotkeys(game_config(), true);
	preferences::load_hotkeys();

	::init_textdomains(game_config());
	about::set_about(game_config());
	ai::configuration::init(game_config());

	return true;
}

namespace {
/// returns true if every define in special is also defined in general
bool map_includes(const preproc_map& general, const preproc_map& special)
{
	for (const preproc_map::value_type& pair : special)
	{
		preproc_map::const_iterator it = general.find(pair.first);
		if (it == general.end() || it->second != pair.second) {
			return false;
		}
	}
	return true;
}
} // end anonymous namespace

void game_config_manager::load_game_config_with_loadscreen(FORCE_RELOAD_CONFIG force_reload,
	game_classification const* classification)
{
	game_config::scoped_preproc_define debug_mode("DEBUG_MODE",
		game_config::debug || game_config::mp_debug);

	// Game_config already holds requested config in memory.
	if (!game_config_.empty()) {
		if ((force_reload == NO_FORCE_RELOAD) && old_defines_map_ == cache_.get_preproc_map()) {
			return;
		}
		if ((force_reload == NO_INCLUDE_RELOAD) && map_includes(old_defines_map_, cache_.get_preproc_map())) {
			return;
		}
	}

	gui2::dialogs::loading_screen::display([this, force_reload, classification]() {
		load_game_config(force_reload, classification);
	});
}

void game_config_manager::load_game_config(FORCE_RELOAD_CONFIG force_reload,
	game_classification const* classification)
{
		// Make sure that 'debug mode' symbol is set
	// if command line parameter is selected
	// also if we're in multiplayer and actual debug mode is disabled.

	// The loadscreen will erase the titlescreen.
	// NOTE: even without loadscreen, needed after MP lobby.
	try {
		// Read all game configs.
		// First we load all core configs, the mainline one and the ones from the addons.
		// Validate the cores and discard the invalid.
		// Then find the path to the selected core.
		// Load the selected core.
		// Handle terrains so that they are last loaded from the core.
		// Load every compatible addon.
		gui2::dialogs::loading_screen::progress(loading_stage::verify_cache);
		filesystem::data_tree_checksum();
		gui2::dialogs::loading_screen::progress(loading_stage::create_cache);

		// Start transaction so macros are shared.
		game_config::config_cache_transaction main_transaction;

		config cores_cfg;
		// Load mainline cores definition file.
		cache_.get_config(game_config::path + "/data/cores.cfg", cores_cfg);

		// Append the $user_campaign_dir/*/cores.cfg files to the cores.
		std::vector<std::string> user_dirs;
		{
			const std::string user_campaign_dir = filesystem::get_addons_dir();
			std::vector<std::string> user_files;
			filesystem::get_files_in_dir(user_campaign_dir, &user_files, &user_dirs,
					filesystem::ENTIRE_FILE_PATH);
		}
		for (const std::string& umc : user_dirs) {
			const std::string cores_file = umc + "/cores.cfg";
			if (filesystem::file_exists(cores_file)) {
				config cores;
				cache_.get_config(cores_file, cores);
				cores_cfg.append(cores);
			}
		}

		// Validate every core
		config valid_cores;
		bool current_core_valid = false;
		std::string wml_tree_root;
		for (const config& core : cores_cfg.child_range("core")) {

			const std::string& id = core["id"];
			if (id.empty()) {
				events::call_in_main_thread([&]() {
					gui2::dialogs::wml_error::display(
						_("Error validating data core."),
						_("Found a core without id attribute.")
						+ '\n' +  _("Skipping the core."));
				});
				continue;
			}
			if (*&valid_cores.find_child("core", "id", id)) {
				events::call_in_main_thread([&]() {
					gui2::dialogs::wml_error::display(
						_("Error validating data core."),
						_("Core ID: ") + id
						+ '\n' + _("The ID is already in use.")
						+ '\n' + _("Skipping the core."));
				});
				continue;
			}

			const std::string& path = core["path"];
			if (!filesystem::file_exists(filesystem::get_wml_location(path))) {
				events::call_in_main_thread([&]() {
					gui2::dialogs::wml_error::display(
						_("Error validating data core."),
						_("Core ID: ") + id
						+ '\n' + _("Core Path: ") + path
						+ '\n' + _("File not found.")
						+ '\n' + _("Skipping the core."));
				});
				continue;
			}

			if (id == "default" && !current_core_valid) {
				wml_tree_root = path;
			}
			if (id == preferences::core_id()) {
				current_core_valid = true;
				wml_tree_root = path;
			}

			valid_cores.add_child("core", core);  // append(core);
		}

		if (!current_core_valid) {
			events::call_in_main_thread([&]() {
				gui2::dialogs::wml_error::display(
					_("Error loading core data."),
					_("Core ID: ") + preferences::core_id()
					+ '\n' + _("Error loading the core with named id.")
					+ '\n' + _("Falling back to the default core."));
			});
			preferences::set_core_id("default");
		}

		// check if we have a valid default core which should always be the case.
		if (wml_tree_root.empty()) {
			events::call_in_main_thread([&]() {
				gui2::dialogs::wml_error::display(
					_("Error loading core data."),
					_("Can't locate the default core.")
					+ '\n' + _("The game will now exit."));
			});
			throw;
		}

		// Load the selected core
		cache_.get_config(filesystem::get_wml_location(wml_tree_root), game_config_);
		game_config_.append(valid_cores);

		main_transaction.lock();

		// Put the gfx rules aside so that we can prepend the add-on
		// rules to them.
		config core_terrain_rules;
		core_terrain_rules.splice_children(game_config_, "terrain_graphics");

		if (!game_config::no_addons && !cmdline_opts_.noaddons)
			load_addons_cfg();

		// If multiplayer campaign is being loaded, [scenario] tags should
		// become [multiplayer] tags and campaign's id should be added to them
		// to allow to recognize which scenarios belongs to a loaded campaign.
		if (classification != nullptr) {
			if (const config& campaign = game_config().find_child("campaign", "id", classification->campaign))
			{
				const bool require_campaign = campaign["require_campaign"].to_bool(true);
				for (config& scenario : game_config_.child_range("scenario"))
				{
					scenario["require_scenario"] = require_campaign;
				}
			}
		}

		// Extract the Lua scripts at toplevel.
		game_lua_kernel::extract_preload_scripts(game_config_);
		game_config_.clear_children("lua");

		// Put the gfx rules back to game config.
		game_config_.splice_children(core_terrain_rules, "terrain_graphics");

		set_multiplayer_hashes();
		set_unit_data();
		game_config::add_color_info(game_config_);

		terrain_builder::set_terrain_rules_cfg(game_config());
		tdata_ = std::make_shared<terrain_type_data>(game_config_);
		::init_strings(game_config());
		theme::set_known_themes(&game_config());
	} catch(const game::error& e) {
		ERR_CONFIG << "Error loading game configuration files\n" << e.message << '\n';

		// Try reloading without add-ons
		if (!game_config::no_addons) {
			game_config::no_addons = true;
			events::call_in_main_thread([&]() {
				gui2::dialogs::wml_error::display(
					_("Error loading custom game configuration files. The game will try without loading add-ons."),
					e.message);
			});
			load_game_config(force_reload, classification);
		} else if (preferences::core_id() != "default") {
			events::call_in_main_thread([&]() {
				gui2::dialogs::wml_error::display(
					_("Error loading custom game configuration files. The game will fallback to the default core files."),
					e.message);
			});
			preferences::set_core_id("default");
			game_config::no_addons = false;
			load_game_config(force_reload, classification);
		} else {
			events::call_in_main_thread([&]() {
				gui2::dialogs::wml_error::display(
					_("Error loading default core game configuration files. The game will now exit."),
					e.message);
			});
			throw;
		}
	}

	old_defines_map_ = cache_.get_preproc_map();

	// Set new binary paths.
	paths_manager_.set_paths(game_config());
}

void game_config_manager::load_addons_cfg()
{
	const std::string user_campaign_dir = filesystem::get_addons_dir();

	std::vector<std::string> error_log;
	std::vector<std::string> error_addons;
	std::vector<std::string> user_dirs;
	std::vector<std::string> user_files;

	filesystem::get_files_in_dir(user_campaign_dir, &user_files, &user_dirs,
		filesystem::ENTIRE_FILE_PATH);

	// Warn player about addons using the no-longer-supported single-file format.
	for(const std::string& file : user_files) {
		const int size_minus_extension = file.size() - 4;

		if(file.substr(size_minus_extension, file.size()) == ".cfg") {
			ERR_CONFIG << "error reading usermade add-on '" << file << "'\n";

			error_addons.push_back(file);

			const int userdata_loc = file.find("data/add-ons") + 5;
			const std::string log_msg = formatter()
				<< "The format '~"
				<< file.substr(userdata_loc)
				<< "' (for single-file add-ons) is not supported anymore, use '~"
				<< file.substr(userdata_loc, size_minus_extension - userdata_loc)
				<< "/_main.cfg' instead.";

			error_log.push_back(log_msg);
		}
	}

	// Rerun the directory scan using filename only, to get the addon_ids more easily.
	user_files.clear();
	user_dirs.clear();

	filesystem::get_files_in_dir(user_campaign_dir, nullptr, &user_dirs,
		filesystem::FILE_NAME_ONLY);

	// Load the addons.
	for(const std::string& addon_id : user_dirs) {
		log_scope2(log_config, "Loading add-on '" + addon_id + "'");
		const std::string addon_dir = user_campaign_dir + "/" + addon_id;

		const std::string main_cfg = addon_dir + "/_main.cfg";
		const std::string info_cfg = addon_dir + "/_info.cfg";

		if(!filesystem::file_exists(main_cfg)) {
			continue;
		}

		// Try to find this addon's metadata. Author publishing info (_server.pbl) is given
		// precedence over addon sever-generated info (_info.cfg). If neither are found, it
		// probably means the addon was installed manually and certain defaults will be used.
		config metadata;

		if(have_addon_pbl_info(addon_id)) {
			// Publishing info needs to be read from disk.
			metadata = get_addon_pbl_info(addon_id);
		} else if(filesystem::file_exists(info_cfg)) {
			// Addon server-generated info can be fetched from cache.
			config temp;
			cache_.get_config(info_cfg, temp);

			metadata = temp.child_or_empty("info");
		}

		std::string using_core = metadata["core"];
		if(using_core.empty()) {
			using_core = "default";
		}

		// Skip add-ons not matching our current core. Cores themselves should be selectable
		// at all times, so they aren't considered here.
		if(!metadata.empty() && metadata["type"] != "core" && using_core != preferences::core_id()) {
			continue;
		}

		std::string addon_title = metadata["title"].str();
		if(addon_title.empty()) {
			addon_title = addon_id;
		}

		version_info addon_version(metadata["version"]);

		try {
			// Load this addon from the cache to a config.
			config umc_cfg;
			cache_.get_config(main_cfg, umc_cfg);

			static const std::set<std::string> tags_with_addon_id {
				"era",
				"modification",
				"resource",
				"multiplayer",
				"scenario",
				"campaign"
			};

			// Annotate appropriate addon types with addon_id info.
			for(auto child : umc_cfg.all_children_range()) {
				if(tags_with_addon_id.count(child.key) > 0) {
					auto& cfg = child.cfg;
					cfg["addon_id"] = addon_id;
					cfg["addon_title"] = addon_title;
					// Note that this may reformat the string in a canonical form.
					cfg["addon_version"] = addon_version.str();
				}
			}

			game_config_.append(std::move(umc_cfg));
		} catch(const config::error& err) {
			ERR_CONFIG << "error reading usermade add-on '" << main_cfg << "'" << std::endl;
			ERR_CONFIG << err.message << '\n';
			error_addons.push_back(main_cfg);
			error_log.push_back(err.message);
		} catch(const preproc_config::error& err) {
			ERR_CONFIG << "error reading usermade add-on '" << main_cfg << "'" << std::endl;
			ERR_CONFIG << err.message << '\n';
			error_addons.push_back(main_cfg);
			error_log.push_back(err.message);
		} catch(const filesystem::io_exception&) {
			ERR_CONFIG << "error reading usermade add-on '" << main_cfg << "'" << std::endl;
			error_addons.push_back(main_cfg);
		}
	}

	if(!error_addons.empty()) {
		const std::size_t n = error_addons.size();
		const std::string& msg1 =
			_n("The following add-on had errors and could not be loaded:",
			   "The following add-ons had errors and could not be loaded:",
			   n);
		const std::string& msg2 =
			_n("Please report this to the author or maintainer of this add-on.",
			   "Please report this to the respective authors or maintainers of these add-ons.",
			   n);

		const std::string& report = utils::join(error_log, "\n\n");
		events::call_in_main_thread([&]() {
			gui2::dialogs::wml_error::display(msg1, msg2, error_addons, report);
		});
	}
}

void game_config_manager::set_multiplayer_hashes()
{
	config& hashes = game_config_.add_child("multiplayer_hashes");
	for (const config &ch : game_config_.child_range("multiplayer")) {
		hashes[ch["id"].str()] = ch.hash();
	}
}

void game_config_manager::set_unit_data()
{
	game_config_.merge_children("units");
	gui2::dialogs::loading_screen::progress(loading_stage::load_unit_types);
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
	filesystem::clear_binary_paths_cache();
	init_game_config(FORCE_RELOAD);

	image::flush_cache();
}

void game_config_manager::load_game_config_for_editor()
{
	game_config::scoped_preproc_define editor("EDITOR");
	load_game_config_with_loadscreen(NO_FORCE_RELOAD);
}

void game_config_manager::load_game_config_for_game(
	const game_classification& classification)
{
	game_config::scoped_preproc_define difficulty(classification.difficulty,
		!classification.difficulty.empty());
	game_config::scoped_preproc_define campaign(classification.campaign_define,
		!classification.campaign_define.empty());
	game_config::scoped_preproc_define scenario(classification.scenario_define,
		!classification.scenario_define.empty());
	game_config::scoped_preproc_define era(classification.era_define,
		!classification.era_define.empty());
	game_config::scoped_preproc_define multiplayer("MULTIPLAYER",
		classification.campaign_type == game_classification::CAMPAIGN_TYPE::MULTIPLAYER);
	game_config::scoped_preproc_define mptest("MP_TEST", cmdline_opts_.mptest &&
		classification.campaign_type == game_classification::CAMPAIGN_TYPE::MULTIPLAYER);

	//
	// NOTE: these deques aren't used here, but the objects within are utilized as RAII helpers.
	//

	typedef std::unique_ptr<game_config::scoped_preproc_define> define;

	std::deque<define> extra_defines;
	for(const std::string& extra_define : classification.campaign_xtra_defines) {
		extra_defines.emplace_back(new game_config::scoped_preproc_define(extra_define));
	}

	std::deque<define> modification_defines;
	for(const std::string& mod_define : classification.mod_defines) {
		modification_defines.emplace_back(new game_config::scoped_preproc_define(mod_define, !mod_define.empty()));
	}

	try {
		load_game_config_with_loadscreen(NO_FORCE_RELOAD, &classification);
	} catch(const game::error&) {
		cache_.clear_defines();

		std::deque<define> previous_defines;
		for(const preproc_map::value_type& preproc : old_defines_map_) {
			previous_defines.emplace_back(new game_config::scoped_preproc_define(preproc.first));
		}

		load_game_config_with_loadscreen(NO_FORCE_RELOAD);

		throw;
	}

	// This needs to be done in the main thread since this function (load_game_config_for_game)
	// might be called from a loading screen worker thread (and currently is, in fact). If the
	// image cache is purged from the worker thread, there's a possibility for a data race where
	// the main thread accesses the image cache and the worker thread simultaneously clears it.
	events::call_in_main_thread([]() { image::flush_cache(); });
}

void game_config_manager::load_game_config_for_create(bool is_mp, bool is_test)
{
	game_config::scoped_preproc_define multiplayer("MULTIPLAYER", is_mp);
	game_config::scoped_preproc_define test("TEST", is_test);
	game_config::scoped_preproc_define mptest("MP_TEST", cmdline_opts_.mptest && is_mp);
///During an mp game the default difficuly define is also defined so better already load it now if we alreeady must reload config cache.
	game_config::scoped_preproc_define normal(DEFAULT_DIFFICULTY, !map_includes(old_defines_map_, cache_.get_preproc_map()));

	typedef std::unique_ptr<game_config::scoped_preproc_define> define;
	try{
		load_game_config_with_loadscreen(NO_INCLUDE_RELOAD);
	}
	catch(const game::error&) {
		cache_.clear_defines();

		std::deque<define> previous_defines;
		for (const preproc_map::value_type& preproc : old_defines_map_) {
			previous_defines.emplace_back(new game_config::scoped_preproc_define(preproc.first));
		}

		load_game_config_with_loadscreen(NO_FORCE_RELOAD);

		throw;
	}
}
