/*
	Copyright (C) 2013 - 2024
	by Andrius Silinskas <silinskas.andrius@gmail.com>
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
#include "events.hpp"
#include "formatter.hpp"
#include "formula/string_utils.hpp"
#include "game_classification.hpp"
#include "game_config.hpp"
#include "game_version.hpp"
#include "gettext.hpp"
#include "gui/dialogs/loading_screen.hpp"
#include "gui/dialogs/wml_error.hpp"
#include "hotkey/hotkey_item.hpp"
#include "language.hpp"
#include "log.hpp"
#include "picture.hpp"
#include "preferences/preferences.hpp"
#include "scripting/game_lua_kernel.hpp"
#include "serialization/schema_validator.hpp"
#include "sound.hpp"
#include "serialization/string_utils.hpp"
#include "terrain/builder.hpp"
#include "terrain/type_data.hpp"
#include "theme.hpp"
#include "units/types.hpp"

static lg::log_domain log_config("config");
#define ERR_CONFIG LOG_STREAM(err, log_config)
#define WRN_CONFIG LOG_STREAM(warn, log_config)
#define LOG_CONFIG LOG_STREAM(info, log_config)

using gui2::dialogs::loading_screen;

static game_config_manager* singleton;

game_config_manager::game_config_manager(const commandline_options& cmdline_opts)
	: cmdline_opts_(cmdline_opts)
	, game_config_()
	, game_config_view_()
	, addon_cfgs_()
	, active_addons_()
	, old_defines_map_()
	, paths_manager_()
	, cache_(game_config::config_cache::instance())
	, achievements_()
{
	assert(!singleton);
	singleton = this;

	// All of the validation options imply --nocache, as the validation happens during cache
	// rebuilding. If the cache isn't rebuilt, validation is silently skipped.
	if(cmdline_opts_.nocache || cmdline_opts_.any_validation_option()) {
		cache_.set_use_cache(false);
	}

	if(cmdline_opts_.validcache) {
		cache_.set_force_valid_cache(true);
	}

	// Clean the cache of any old Wesnoth version's cache data
	if(const std::string last_cleaned = prefs::get()._last_cache_cleaned_ver(); !last_cleaned.empty()) {
		if(version_info{last_cleaned} < game_config::wesnoth_version) {
			if(cache_.clean_cache()) {
				prefs::get().set__last_cache_cleaned_ver(game_config::wesnoth_version.str());
			}
		}
	} else {
		// If the preference wasn't set, set it, else the cleaning will never happen :P
		prefs::get().set__last_cache_cleaned_ver(game_config::wesnoth_version.str());
	}
}

game_config_manager::~game_config_manager()
{
	assert(singleton);
	singleton = nullptr;
}

game_config_manager* game_config_manager::get()
{
	return singleton;
}

bool game_config_manager::init_game_config(FORCE_RELOAD_CONFIG force_reload)
{
	// Add preproc defines according to the command line arguments.
	game_config::scoped_preproc_define multiplayer("MULTIPLAYER", cmdline_opts_.multiplayer);
	game_config::scoped_preproc_define test("TEST", cmdline_opts_.test.has_value());
	game_config::scoped_preproc_define mptest("MP_TEST", cmdline_opts_.mptest);
	game_config::scoped_preproc_define editor("EDITOR", cmdline_opts_.editor.has_value());
	game_config::scoped_preproc_define title_screen("TITLE_SCREEN",
		!cmdline_opts_.multiplayer && !cmdline_opts_.test && !cmdline_opts_.editor);

	game_config::reset_color_info();

	load_game_config_with_loadscreen(force_reload, nullptr, "");

	game_config::load_config(game_config().mandatory_child("game_config"));

	// It's necessary to block the event thread while load_hotkeys() runs, otherwise keyboard input
	// can cause a crash by accessing the list of hotkeys while it's being modified.
	events::call_in_main_thread([this]() {
		const hotkey::scope_changer hk_scope{hotkey::scope_main, false};

		// Load the standard hotkeys, then apply any player customizations.
		hotkey::load_default_hotkeys(game_config());
		prefs::get().load_hotkeys();
	});

	prefs::get().load_advanced_prefs(game_config());

	::init_textdomains(game_config());
	about::set_about(game_config());
	ai::configuration::init(game_config());

	return true;
}

namespace
{
/** returns true if every define in special is also defined in general */
bool map_includes(const preproc_map& general, const preproc_map& special)
{
	return std::all_of(special.begin(), special.end(), [&general](const auto& pair) {
		const auto it = general.find(pair.first);
		return it != general.end() && it->second == pair.second;
	});
}
} // end anonymous namespace

void game_config_manager::load_game_config_with_loadscreen(
	FORCE_RELOAD_CONFIG force_reload, const game_classification* classification, const std::string& scenario_id)
{
	if(!lg::info().dont_log(log_config)) {
		auto out = formatter();
		out << "load_game_config: defines:";

		for(const auto& pair : cache_.get_preproc_map()) {
			out << pair.first << ",";
		}

		out << "\n";
		FORCE_LOG_TO(lg::info(), log_config) << out.str();
	}

	game_config::scoped_preproc_define debug_mode("DEBUG_MODE",
		game_config::debug || game_config::mp_debug);

	bool reload_everything = true;

	// Game_config already holds requested config in memory.
	if(!game_config_.empty()) {
		if(force_reload == NO_FORCE_RELOAD && old_defines_map_ == cache_.get_preproc_map()) {
			reload_everything = false;
		}

		if(force_reload == NO_INCLUDE_RELOAD && map_includes(old_defines_map_, cache_.get_preproc_map())) {
			reload_everything = false;
		}
	}

	LOG_CONFIG << "load_game_config reload everything: " << reload_everything;

	gui2::dialogs::loading_screen::display([this, reload_everything, classification, scenario_id]() {
		load_game_config(reload_everything, classification, scenario_id);
	});
}

void game_config_manager::load_game_config(bool reload_everything, const game_classification* classification, const std::string& scenario_id)
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
		if(reload_everything) {
			gui2::dialogs::loading_screen::progress(loading_stage::verify_cache);
			filesystem::data_tree_checksum();
			gui2::dialogs::loading_screen::progress(loading_stage::create_cache);

			// Start transaction so macros are shared.
			game_config::config_cache_transaction main_transaction;
			achievements_.reload();

			// Load mainline cores definition file.
			config cores_cfg;
			cache_.get_config(game_config::path + "/data/cores.cfg", cores_cfg);

			// Append the $user_campaign_dir/*/cores.cfg files to the cores.
			std::vector<std::string> user_dirs;
			{
				const std::string user_campaign_dir = filesystem::get_addons_dir();
				std::vector<std::string> user_files;
				filesystem::get_files_in_dir(
					user_campaign_dir, &user_files, &user_dirs, filesystem::name_mode::ENTIRE_FILE_PATH);
			}

			for(const std::string& umc : user_dirs) {
				const std::string cores_file = umc + "/cores.cfg";
				if(filesystem::file_exists(cores_file)) {
					config cores;
					cache_.get_config(cores_file, cores);
					cores_cfg.append(cores);
				}
			}

			// Validate every core
			config valid_cores;
			bool current_core_valid = false;
			std::string wml_tree_root;

			for(const config& core : cores_cfg.child_range("core")) {
				const std::string& id = core["id"];
				if(id.empty()) {
					events::call_in_main_thread([&]() {
						gui2::dialogs::wml_error::display(
							_("Error validating data core."),
							_("Found a core without id attribute.")
							+ '\n' +  _("Skipping the core."));
					});
					continue;
				}

				if(valid_cores.find_child("core", "id", id)) {
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
				if(!filesystem::get_wml_location(path)) {
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

				if(id == "default" && !current_core_valid) {
					wml_tree_root = path;
				}
				if(id == prefs::get().core()) {
					current_core_valid = true;
					wml_tree_root = path;
				}

				valid_cores.add_child("core", core);  // append(core);
			}

			if(!current_core_valid) {
				events::call_in_main_thread([&]() {
					gui2::dialogs::wml_error::display(
						_("Error loading core data."),
						_("Core ID: ") + prefs::get().core()
						+ '\n' + _("Error loading the core with named id.")
						+ '\n' + _("Falling back to the default core."));
				});
				prefs::get().set_core("default");
			}

			// check if we have a valid default core which should always be the case.
			if(wml_tree_root.empty()) {
				events::call_in_main_thread([&]() {
					gui2::dialogs::wml_error::display(
						_("Error loading core data."),
						_("Can't locate the default core.")
						+ '\n' + _("The game will now exit."));
				});
				throw;
			}

			// Load the selected core
			std::unique_ptr<schema_validation::schema_validator> validator;
			if(cmdline_opts_.validate_core) {
				validator.reset(new schema_validation::schema_validator(filesystem::get_wml_location("schema/game_config.cfg").value()));
				validator->set_create_exceptions(false); // Don't crash if there's an error, just go ahead anyway
			}

			cache_.get_config(filesystem::get_wml_location(wml_tree_root).value(), game_config_, validator.get());
			game_config_.append(valid_cores);

			main_transaction.lock();

			if(!game_config::no_addons && !cmdline_opts_.noaddons) {
				load_addons_cfg();
			}
		}

		// only after addon configs have been loaded do we check for which addons are needed and whether they exist to be used
		LOG_CONFIG << "active_addons_ has size " << active_addons_.size() << " and contents: " << utils::join(active_addons_);
		if(classification) {
			LOG_CONFIG << "Enabling only some add-ons!";
			std::set<std::string> active_addons = classification->active_addons(scenario_id);
			// IMPORTANT: this is a significant performance optimization, particularly for the worst case example of the batched WML unit tests
			if(!reload_everything && active_addons == active_addons_) {
				LOG_CONFIG << "Configs not reloaded and active add-ons remain the same; returning early.";
				LOG_CONFIG << "active_addons has size " << active_addons_.size() << " and contents: " << utils::join(active_addons);
				return;
			}
			active_addons_ = active_addons;
			set_enabled_addon(active_addons_);
		} else {
			LOG_CONFIG << "Enabling all add-ons!";
			set_enabled_addon_all();
		}

		// Extract the Lua scripts at toplevel.
		game_lua_kernel::extract_preload_scripts(game_config());

		set_unit_data();
		terrain_builder::set_terrain_rules_cfg(game_config());
		tdata_ = std::make_shared<terrain_type_data>(game_config());
		::init_strings(game_config());
		theme::set_known_themes(&game_config());

		set_multiplayer_hashes();

		game_config::add_color_info(game_config());

	} catch(const game::error& e) {
		ERR_CONFIG << "Error loading game configuration files\n" << e.message;

		// Try reloading without add-ons
		if(!game_config::no_addons) {
			game_config::no_addons = true;
			events::call_in_main_thread([&]() {
				gui2::dialogs::wml_error::display(
					_("Error loading custom game configuration files. The game will try without loading add-ons."),
					e.message);
			});
			load_game_config(reload_everything, classification, scenario_id);
		} else if(prefs::get().core() != "default") {
			events::call_in_main_thread([&]() {
				gui2::dialogs::wml_error::display(
					_("Error loading custom game configuration files. The game will fallback to the default core files."),
					e.message);
			});
			prefs::get().set_core("default");
			game_config::no_addons = false;
			load_game_config(reload_everything, classification, scenario_id);
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
static void show_deprecated_warnings(config& umc_cfg)
{
	for(auto& units : umc_cfg.child_range("units")) {
		for(auto& unit_type : units.child_range("unit_type")) {
			for(const auto& advancefrom : unit_type.child_range("advancefrom")) {
				auto symbols = utils::string_map {
					{"lower_level", advancefrom["unit"]},
					{"higher_level", unit_type["id"]}
				};
				auto message = VGETTEXT(
					// TRANSLATORS: For example, 'Cuttle Fish' units will not be able to advance to 'Kraken'.
					// The substituted strings are unit ids, not translated names; hopefully any add-ons
					// that trigger this will be quickly fixed and stop triggering the warning.
					"Error: [advancefrom] no longer works. ‘$lower_level’ units will not be able to advance to ‘$higher_level’; please ask the add-on author to use [modify_unit_type] instead.",
					symbols);
				deprecated_message("[advancefrom]", DEP_LEVEL::REMOVED, {1, 15, 4}, message);
			}
			unit_type.remove_children("advancefrom", [](const config&){return true;});
		}
	}


	// hardcoded list of 1.14 advancement macros, just used for the error mesage below.
	static const std::set<std::string> deprecated_defines {
		"ENABLE_PARAGON",
		"DISABLE_GRAND_MARSHAL",
		"ENABLE_ARMAGEDDON_DRAKE",
		"ENABLE_DWARVISH_ARCANISTER",
		"ENABLE_DWARVISH_RUNESMITH",
		"ENABLE_WOLF_ADVANCEMENT",
		"ENABLE_NIGHTBLADE",
		"ENABLE_TROLL_SHAMAN",
		"ENABLE_ANCIENT_LICH",
		"ENABLE_DEATH_KNIGHT",
		"ENABLE_WOSE_SHAMAN"
	};

	for(auto& campaign : umc_cfg.child_range("campaign")) {
		for(auto str : utils::split(campaign["extra_defines"])) {
			if(deprecated_defines.count(str) > 0) {
				//TODO: we could try to implement a compatibility path by
				//      somehow getting the content of that macro from the
				//      cache_ object, but considering that 1) the breakage
				//      isn't that bad (just one disabled unit) and 2)
				//      it before also didn't work in all cases (see #4402)
				//      i don't think it is worth it.
				deprecated_message(
					"campaign id='" + campaign["id"].str() + "' has extra_defines=" + str,
					DEP_LEVEL::REMOVED,
					{1, 15, 4},
					_("instead, use the macro with the same name in the [campaign] tag")
				);
			}
		}
	}
}

void game_config_manager::load_addons_cfg()
{
	const std::string user_campaign_dir = filesystem::get_addons_dir();

	std::vector<std::string> error_log;
	std::vector<std::string> error_addons;
	std::vector<std::string> user_dirs;
	std::vector<std::string> user_files;

	filesystem::get_files_in_dir(user_campaign_dir, &user_files, &user_dirs, filesystem::name_mode::ENTIRE_FILE_PATH);

	// Warn player about addons using the no-longer-supported single-file format.
	for(const std::string& file : user_files) {

		if(filesystem::is_cfg(file)) {
			ERR_CONFIG << "error reading usermade add-on '" << file << "'";

			error_addons.push_back(file);

			const std::string short_wml_path = filesystem::get_short_wml_path(file);
			const std::string log_msg = formatter()
				<< "The format '"
				<< short_wml_path
				<< "' (for single-file add-ons) is not supported anymore, use '"
				<< short_wml_path.substr(0, short_wml_path.size() - filesystem::wml_extension.size())
				<< "/_main.cfg' instead.";

			error_log.push_back(log_msg);
		}
	}

	loading_screen::spin();

	// Rerun the directory scan using filename only, to get the addon_ids more easily.
	user_files.clear();
	user_dirs.clear();

	filesystem::get_files_in_dir(user_campaign_dir, nullptr, &user_dirs,
		filesystem::name_mode::FILE_NAME_ONLY);

	loading_screen::spin();

	// Load the addons.
	for(const std::string& addon_id : user_dirs) {
		log_scope2(log_config, "Loading add-on '" + addon_id + "'");
		const std::string addon_dir = user_campaign_dir + "/" + addon_id;

		const std::string main_cfg = addon_dir + "/_main.cfg";
		const std::string info_cfg = addon_dir + "/_info.cfg";

		if(!filesystem::file_exists(main_cfg)) {
			continue;
		}

		loading_screen::spin();

		// Try to find this addon's metadata. Author publishing info (_server.pbl) is given
		// precedence over addon sever-generated info (_info.cfg). If neither are found, it
		// probably means the addon was installed manually and certain defaults will be used.
		config metadata;

		if(have_addon_pbl_info(addon_id)) {
			// Publishing info needs to be read from disk.
			try {
				metadata = get_addon_pbl_info(addon_id, false);
			} catch(const invalid_pbl_exception& e) {
				const std::string log_msg = formatter()
				<< "The provided addon has an invalid pbl file"
				<< " for addon "
				<< addon_id;

				error_addons.push_back(e.message);
				error_log.push_back(log_msg);
			}
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
		if(!metadata.empty() && metadata["type"] != "core" && using_core != prefs::get().core()) {
			continue;
		}

		std::string addon_title = metadata["title"].str();
		if(addon_title.empty()) {
			addon_title = addon_id;
		}

		version_info addon_version(metadata["version"]);

		try {
			std::unique_ptr<schema_validation::schema_validator> validator;
			if(cmdline_opts_.validate_addon && *cmdline_opts_.validate_addon == addon_id) {
				validator.reset(new schema_validation::schema_validator(filesystem::get_wml_location("schema/game_config.cfg").value()));
				validator->set_create_exceptions(false); // Don't crash if there's an error, just go ahead anyway
			}

			loading_screen::spin();

			// Load this addon from the cache to a config.
			config umc_cfg;
			cache_.get_config(main_cfg, umc_cfg, validator.get());

			static const std::set<std::string> tags_with_addon_id {
				"era",
				"modification",
				"resource",
				"multiplayer",
				"scenario",
				"campaign",
				"test"
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

			loading_screen::spin();

			show_deprecated_warnings(umc_cfg);

			loading_screen::spin();

			static const std::set<std::string> entry_tags {
				"era",
				"modification",
				"resource",
				"multiplayer",
				"scenario",
				"campaign"
			};

			for(const std::string& tagname : entry_tags) {
				game_config_.append_children_by_move(umc_cfg, tagname);
			}

			loading_screen::spin();

			addon_cfgs_[addon_id] = std::move(umc_cfg);
		} catch(const config::error& err) {
			ERR_CONFIG << "config error reading usermade add-on '" << main_cfg << "'";
			ERR_CONFIG << err.message;
			error_addons.push_back(main_cfg);
			error_log.push_back(err.message);
		} catch(const preproc_config::error& err) {
			ERR_CONFIG << "preprocessor config error reading usermade add-on '" << main_cfg << "'";
			ERR_CONFIG << err.message;
			error_addons.push_back(main_cfg);
			error_log.push_back(err.message);
		} catch(const filesystem::io_exception&) {
			ERR_CONFIG << "filesystem I/O error reading usermade add-on '" << main_cfg << "'";
			error_addons.push_back(main_cfg);
		}
	}

	if(cmdline_opts_.validate_addon) {
		if(!addon_cfgs_.count(*cmdline_opts_.validate_addon)) {
			ERR_CONFIG << "Didn’t find an add-on for --validate-addon - check whether the id has a typo";
			const std::string log_msg = formatter()
				<< "Didn't find an add-on for --validate-addon - check whether the id has a typo";
			error_log.push_back(log_msg);
			throw game::error("Did not find an add-on for --validate-addon");
		}

		WRN_CONFIG << "Note: for --validate-addon to find errors, you have to play (in the GUI) a game that uses the add-on.";
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
	for(const config& ch : game_config().child_range("multiplayer")) {
		hashes[ch["id"].str()] = ch.hash();
	}
}

void game_config_manager::set_unit_data()
{
	gui2::dialogs::loading_screen::progress(loading_stage::load_unit_types);
	unit_types.set_config(game_config().merged_children_view("units"));
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
	sound::flush_cache();
}

void game_config_manager::load_game_config_for_editor()
{
	game_config::scoped_preproc_define editor("EDITOR");
	load_game_config_with_loadscreen(NO_FORCE_RELOAD, nullptr, "");
}

void game_config_manager::load_game_config_for_game(
	const game_classification& classification, const std::string& scenario_id)
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
		classification.is_multiplayer());
	game_config::scoped_preproc_define mptest("MP_TEST", cmdline_opts_.mptest &&
		classification.is_multiplayer());

	//
	// NOTE: these deques aren't used here, but the objects within are utilized as RAII helpers.
	//

	std::deque<game_config::scoped_preproc_define> extra_defines;
	for(const std::string& extra_define : classification.campaign_xtra_defines) {
		extra_defines.emplace_back(extra_define);
	}

	std::deque<game_config::scoped_preproc_define> modification_defines;
	for(const std::string& mod_define : classification.mod_defines) {
		modification_defines.emplace_back(mod_define, !mod_define.empty());
	}

	try {
		load_game_config_with_loadscreen(NO_FORCE_RELOAD, &classification, scenario_id);
	} catch(const game::error&) {
		cache_.clear_defines();

		std::deque<game_config::scoped_preproc_define> previous_defines;
		for(const preproc_map::value_type& preproc : old_defines_map_) {
			previous_defines.emplace_back(preproc.first);
		}

		load_game_config_with_loadscreen(NO_FORCE_RELOAD, nullptr, "");
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
	/** During an mp game the default difficulty define is also defined so better already load it now if we already must reload config cache. */
	game_config::scoped_preproc_define normal(
		DEFAULT_DIFFICULTY, !map_includes(old_defines_map_, cache_.get_preproc_map()));

	try {
		load_game_config_with_loadscreen(NO_INCLUDE_RELOAD, nullptr, "");
	} catch(const game::error&) {
		cache_.clear_defines();

		std::deque<game_config::scoped_preproc_define> previous_defines;
		for (const preproc_map::value_type& preproc : old_defines_map_) {
			previous_defines.emplace_back(preproc.first);
		}

		load_game_config_with_loadscreen(NO_FORCE_RELOAD, nullptr, "");
		throw;
	}
}

void game_config_manager::set_enabled_addon(std::set<std::string> addon_ids)
{
	auto& vec = game_config_view_.data();
	vec.clear();
	vec.push_back(game_config_);

	for(const std::string& id : addon_ids) {
		auto it = addon_cfgs_.find(id);
		if(it != addon_cfgs_.end()) {
			LOG_CONFIG << "Enabling add-on " << id;
			vec.push_back(it->second);
		} else {
			ERR_CONFIG << "Attempted to enable add-on '" << id << "' but its config could not be found";
		}
	}
}

void game_config_manager::set_enabled_addon_all()
{
	active_addons_.clear();
	auto& vec = game_config_view_.data();
	vec.clear();
	vec.push_back(game_config_);

	for(const auto& pair : addon_cfgs_) {
		LOG_CONFIG << "Enabling add-on " << pair.first;
		vec.push_back(pair.second);
		active_addons_.emplace(pair.first);
	}
}
