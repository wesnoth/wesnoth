/*
	Copyright (C) 2003 - 2024
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "addon/manager.hpp"
#include "build_info.hpp"
#include "commandline_argv.hpp"
#include "commandline_options.hpp" // for commandline_options, etc
#include "config.hpp"              // for config, config::error, etc
#include "cursor.hpp"              // for set, CURSOR_TYPE::NORMAL, etc
#include "filesystem.hpp" // for filesystem::file_exists, filesystem::io_exception, etc
#include "floating_label.hpp"
#include "font/error.hpp"          // for error
#include "font/font_config.hpp"    // for load_font_config, etc
#include "formula/formula.hpp"     // for formula_error
#include "game_config.hpp"         // for path, debug, debug_lua, etc
#include "game_config_manager.hpp" // for game_config_manager, etc
#include "game_end_exceptions.hpp"
#include "game_launcher.hpp" // for game_launcher, etc
#include "gettext.hpp"
#include "gui/core/event/handler.hpp" // for tmanager
#include "gui/dialogs/loading_screen.hpp"
#include "gui/dialogs/message.hpp"      // for show_error_message
#include "gui/dialogs/migrate_version_selection.hpp"
#include "gui/dialogs/title_screen.hpp" // for title_screen, etc
#include "gui/gui.hpp"                  // for init
#include "log.hpp"                      // for LOG_STREAM, general, logger, etc
#include "preferences/preferences.hpp"
#include "scripting/application_lua_kernel.hpp"
#include "scripting/plugins/context.hpp"
#include "scripting/plugins/manager.hpp"
#include "sdl/exception.hpp" // for exception
#include "serialization/binary_or_text.hpp" // for config_writer
#include "serialization/parser.hpp"         // for read
#include "serialization/preprocessor.hpp"   // for preproc_define, etc
#include "serialization/schema_validator.hpp" // for strict_validation_enabled and schema_validator
#include "sound.hpp"                   // for commit_music_changes, etc
#include "utils/optimer.hpp"
#include "formula/string_utils.hpp" // VGETTEXT
#include <functional>
#include "game_version.hpp"        // for version_info
#include "video.hpp"          // for video::error and video::quit
#include "wesconfig.h"        // for PACKAGE
#include "widgets/button.hpp" // for button
#include "wml_exception.hpp"  // for wml_exception

#include "utils/spritesheet_generator.hpp"
#ifdef _WIN32
#include "log_windows.hpp"

#include <float.h>
#endif // _WIN32

#ifndef _MSC_VER
#include <fenv.h>
#endif // _MSC_VER

#include <SDL2/SDL.h> // for SDL_Init, SDL_INIT_TIMER

#include <boost/program_options/errors.hpp>     // for error
#include <boost/algorithm/string/predicate.hpp> // for checking cmdline options
#include "utils/optional_fwd.hpp"

#include <cerrno>    // for ENOMEM
#include <clocale>   // for setlocale, LC_ALL, etc
#include <cstdio>    // for remove, fprintf, stderr
#include <cstdlib>   // for srand, exit
#include <ctime>     // for time, ctime, std::time_t
#include <exception> // for exception
#include <vector>
#include <iostream>

//#define NO_CATCH_AT_GAME_END

#ifdef _WIN32

#ifdef INADDR_ANY
#undef INADDR_ANY
#endif

#ifdef INADDR_BROADCAST
#undef INADDR_BROADCAST
#endif

#ifdef INADDR_NONE
#undef INADDR_NONE
#endif

#include <windows.h>

#endif // _WIN32

#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
#include "gui/widgets/debug.hpp"
#endif

// this file acts as a launcher for various gui2 dialogs
// so this reduces some boilerplate.
using namespace gui2::dialogs;

static lg::log_domain log_config("config");
#define LOG_CONFIG LOG_STREAM(info, log_config)

#define LOG_GENERAL LOG_STREAM(info, lg::general())

static lg::log_domain log_preprocessor("preprocessor");
#define LOG_PREPROC LOG_STREAM(info, log_preprocessor)

// this is needed to allow identical functionality with clean refactoring
// play_game only returns on an error, all returns within play_game can
// be replaced with this
static void safe_exit(int res)
{
	LOG_GENERAL << "exiting with code " << res;
	exit(res);
}

static void handle_preprocess_string(const commandline_options& cmdline_opts)
{
	preproc_map defines_map;

	if(cmdline_opts.preprocess_input_macros) {
		std::string file = *cmdline_opts.preprocess_input_macros;
		if(!filesystem::file_exists(file)) {
			PLAIN_LOG << "please specify an existing file. File " << file << " doesn't exist.";
			return;
		}

		PLAIN_LOG << "Reading cached defines from: " << file;

		config cfg;

		try {
			filesystem::scoped_istream stream = filesystem::istream_file(file);
			read(cfg, *stream);
		} catch(const config::error& e) {
			PLAIN_LOG << "Caught a config error while parsing file '" << file << "':\n" << e.message;
		}

		int read = 0;

		// use static preproc_define::read_pair(config) to make a object
		for(const auto [_, cfg] : cfg.all_children_view()) {
			const preproc_map::value_type def = preproc_define::read_pair(cfg);
			defines_map[def.first] = def.second;
			++read;
		}

		PLAIN_LOG << "Read " << read << " defines.";
	}

	if(cmdline_opts.preprocess_defines) {
		// add the specified defines
		for(const std::string& define : *cmdline_opts.preprocess_defines) {
			if(define.empty()) {
				PLAIN_LOG << "empty define supplied";
				continue;
			}

			LOG_PREPROC << "adding define: " << define;
			defines_map.emplace(define, preproc_define(define));
		}
	}

	// add the WESNOTH_VERSION define
	defines_map["WESNOTH_VERSION"] = preproc_define(game_config::wesnoth_version.str());

	// preprocess resource
	PLAIN_LOG << "preprocessing specified string: " << *cmdline_opts.preprocess_source_string;
	const utils::ms_optimer timer(
		[](const auto& timer) { PLAIN_LOG << "preprocessing finished. Took " << timer << " ticks."; });
	std::cout << preprocess_string(*cmdline_opts.preprocess_source_string, &defines_map) << std::endl;
	PLAIN_LOG << "added " << defines_map.size() << " defines.";
}

static void handle_preprocess_command(const commandline_options& cmdline_opts)
{
	preproc_map input_macros;

	if(cmdline_opts.preprocess_input_macros) {
		std::string file = *cmdline_opts.preprocess_input_macros;
		if(filesystem::file_exists(file) == false) {
			PLAIN_LOG << "please specify an existing file. File " << file << " doesn't exist.";
			return;
		}

		PLAIN_LOG << "Reading cached defines from: " << file;

		config cfg;

		try {
			filesystem::scoped_istream stream = filesystem::istream_file(file);
			read(cfg, *stream);
		} catch(const config::error& e) {
			PLAIN_LOG << "Caught a config error while parsing file '" << file << "':\n" << e.message;
		}

		int read = 0;

		// use static preproc_define::read_pair(config) to make a object
		for(const auto [_, cfg] : cfg.all_children_view()) {
			const preproc_map::value_type def = preproc_define::read_pair(cfg);
			input_macros[def.first] = def.second;
			++read;
		}

		PLAIN_LOG << "Read " << read << " defines.";
	}

	const std::string resourceToProcess(*cmdline_opts.preprocess_path);
	const std::string targetDir(*cmdline_opts.preprocess_target);

	const utils::ms_optimer timer(
		[](const auto& timer) { PLAIN_LOG << "preprocessing finished. Took " << timer << " ticks."; });

	// If the users add the SKIP_CORE define we won't preprocess data/core
	bool skipCore = false;
	bool skipTerrainGFX = false;

	// The 'core_defines_map' is the one got from data/core macros
	preproc_map defines_map(input_macros);

	if(cmdline_opts.preprocess_defines) {
		// add the specified defines
		for(const std::string& define : *cmdline_opts.preprocess_defines) {
			if(define.empty()) {
				PLAIN_LOG << "empty define supplied";
				continue;
			}

			LOG_PREPROC << "adding define: " << define;
			defines_map.emplace(define, preproc_define(define));

			if(define == "SKIP_CORE") {
				PLAIN_LOG << "'SKIP_CORE' defined.";
				skipCore = true;
			} else if(define == "NO_TERRAIN_GFX") {
				PLAIN_LOG << "'NO_TERRAIN_GFX' defined.";
				skipTerrainGFX = true;
			}
		}
	}

	// add the WESNOTH_VERSION define
	defines_map["WESNOTH_VERSION"] = preproc_define(game_config::wesnoth_version.str());

	PLAIN_LOG << "added " << defines_map.size() << " defines.";

	// preprocess core macros first if we don't skip the core
	if(skipCore == false) {
		PLAIN_LOG << "preprocessing common macros from 'data/core' ...";

		// process each folder explicitly to gain speed
		preprocess_resource(game_config::path + "/data/core/macros", &defines_map);

		if(skipTerrainGFX == false) {
			preprocess_resource(game_config::path + "/data/core/terrain-graphics", &defines_map);
		}

		PLAIN_LOG << "acquired " << (defines_map.size() - input_macros.size()) << " 'data/core' defines.";
	} else {
		PLAIN_LOG << "skipped 'data/core'";
	}

	// preprocess resource
	PLAIN_LOG << "preprocessing specified resource: " << resourceToProcess << " ...";

	preprocess_resource(resourceToProcess, &defines_map, true, true, targetDir);
	PLAIN_LOG << "acquired " << (defines_map.size() - input_macros.size()) << " total defines.";

	if(cmdline_opts.preprocess_output_macros) {
		std::string outputFileName = "_MACROS_.cfg";
		if(!cmdline_opts.preprocess_output_macros->empty()) {
			outputFileName = *cmdline_opts.preprocess_output_macros;
		}

		std::string outputPath = targetDir + "/" + outputFileName;

		PLAIN_LOG << "writing '" << outputPath << "' with " << defines_map.size() << " defines.";

		filesystem::scoped_ostream out = filesystem::ostream_file(outputPath);
		if(!out->fail()) {
			config_writer writer(*out, false);

			for(auto& define_pair : defines_map) {
				define_pair.second.write(writer, define_pair.first);
			}
		} else {
			PLAIN_LOG << "couldn't open the file.";
		}
	}
}

static int handle_validate_command(const std::string& file, abstract_validator& validator, const std::vector<std::string>& defines) {
	preproc_map defines_map;
	// add the WESNOTH_VERSION define
	defines_map["WESNOTH_VERSION"] = preproc_define(game_config::wesnoth_version.str());
	defines_map["SCHEMA_VALIDATION"] = preproc_define();
	for(const std::string& define : defines) {
		if(define.empty()) {
			PLAIN_LOG << "empty define supplied";
			continue;
		}

		LOG_PREPROC << "adding define: " << define;
		defines_map.emplace(define, preproc_define(define));
	}
	PLAIN_LOG << "Validating " << file << " against schema " << validator.name_;
	lg::set_strict_severity(lg::severity::LG_ERROR);
	filesystem::scoped_istream stream = preprocess_file(file, &defines_map);
	config result;
	read(result, *stream, &validator);
	if(lg::broke_strict()) {
		std::cout << "validation failed\n";
	} else {
		std::cout << "validation succeeded\n";
	}
	return lg::broke_strict();
}

/** Process commandline-arguments */
static int process_command_args(commandline_options& cmdline_opts)
{
	// Options that don't change behavior based on any others should be checked alphabetically below.

	if(cmdline_opts.no_log_sanitize) {
		lg::set_log_sanitize(false);
	}

	if(cmdline_opts.usercache_dir) {
		filesystem::set_cache_dir(*cmdline_opts.usercache_dir);
	}

	if(cmdline_opts.userdata_dir) {
		filesystem::set_user_data_dir(*cmdline_opts.userdata_dir);
	}

	// earliest possible point to ensure the userdata directory is known
	if(!filesystem::is_userdata_initialized()) {
		filesystem::set_user_data_dir(std::string());
	}

	// userdata is initialized, so initialize logging to file if enabled
	// If true, output will be redirected to file, else output be written to console.
	// On Windows, if Wesnoth was not started from a console, one will be allocated.
	if(cmdline_opts.log_to_file
		|| (!cmdline_opts.no_log_to_file
			&& !getenv("WESNOTH_NO_LOG_FILE")
			// command line options that imply not redirecting output to a log file
			&& !cmdline_opts.data_path
			&& !cmdline_opts.userdata_path
			&& !cmdline_opts.usercache_path
			&& !cmdline_opts.version
			&& !cmdline_opts.simple_version
			&& !cmdline_opts.logdomains
			&& !cmdline_opts.help
			&& !cmdline_opts.report
			&& !cmdline_opts.do_diff
			&& !cmdline_opts.do_patch
			&& !cmdline_opts.preprocess
			&& !cmdline_opts.render_image
			&& !cmdline_opts.screenshot
			&& !cmdline_opts.nogui
			&& !cmdline_opts.headless_unit_test
			&& !cmdline_opts.validate_schema
			&& !cmdline_opts.validate_wml
			)
		)
	{
		lg::set_log_to_file();
	}
#ifdef _WIN32
	// This forces a Windows console to be attached to the process even
	// if Wesnoth is an IMAGE_SUBSYSTEM_WINDOWS_GUI executable because it
	// turns Wesnoth into a CLI application. (unless --wnoconsole is given)
	else if(!cmdline_opts.no_console) {
		lg::do_console_redirect();
	}
#endif

	if(cmdline_opts.log) {
		for(const auto& log_pair : *cmdline_opts.log) {
			const std::string log_domain = log_pair.second;
			const lg::severity severity = log_pair.first;
			if(!lg::set_log_domain_severity(log_domain, severity)) {
				PLAIN_LOG << "unknown log domain: " << log_domain;
				return 2;
			}
		}
	}

	if(!cmdline_opts.nobanner) {
		PLAIN_LOG << "Battle for Wesnoth v" << game_config::revision  << " " << game_config::build_arch();
		const std::time_t t = std::time(nullptr);
		PLAIN_LOG << "Started on " << ctime(&t);
	}

	if(cmdline_opts.usercache_path) {
		std::cout << filesystem::get_cache_dir();
		return 0;
	}

	if(cmdline_opts.userdata_path) {
		std::cout << filesystem::get_user_data_dir();
		return 0;
	}

	if(cmdline_opts.data_dir) {
		game_config::path = filesystem::normalize_path(*cmdline_opts.data_dir, true, true);
		if(!cmdline_opts.nobanner) {
			PLAIN_LOG << "Overriding data directory with '" << game_config::path << "'";
		}
	} else {
		// if a pre-defined path does not exist this will empty it
		game_config::path = filesystem::normalize_path(game_config::path, true, true);
		if(game_config::path.empty()) {
			if(std::string exe_dir = filesystem::get_exe_dir(); !exe_dir.empty()) {
				if(std::string auto_dir = filesystem::autodetect_game_data_dir(std::move(exe_dir)); !auto_dir.empty()) {
					if(!cmdline_opts.nobanner) {
						PLAIN_LOG << "Automatically found a possible data directory at: " << auto_dir;
					}
					game_config::path = filesystem::normalize_path(auto_dir, true, true);
				}
			} else {
				PLAIN_LOG << "Cannot find game data directory. Specify one with --data-dir";
				return 1;
			}
		}
	}

	if(!filesystem::is_directory(game_config::path)) {
		PLAIN_LOG << "Could not find game data directory '" << game_config::path << "'";
		return 1;
	}

	if(cmdline_opts.data_path) {
		std::cout << game_config::path;
		return 0;
	}

	if(cmdline_opts.debug_lua) {
		game_config::debug_lua = true;
	}

	if(cmdline_opts.allow_insecure) {
		game_config::allow_insecure = true;
	}

	if(cmdline_opts.addon_server_info) {
		game_config::addon_server_info = true;
	}

	if(cmdline_opts.strict_lua) {
		game_config::strict_lua = true;
	}

	if(cmdline_opts.help) {
		std::cout << cmdline_opts;
		return 0;
	}

	if(cmdline_opts.logdomains) {
		std::cout << lg::list_log_domains(*cmdline_opts.logdomains);
		return 0;
	}

	if(cmdline_opts.log_precise_timestamps) {
		lg::precise_timestamps(true);
	}

	if(cmdline_opts.rng_seed) {
		srand(*cmdline_opts.rng_seed);
	}

	if(cmdline_opts.render_image) {
		SDL_setenv("SDL_VIDEODRIVER", "dummy", 1);
	}

	if(cmdline_opts.strict_validation) {
		strict_validation_enabled = true;
	}

	if(cmdline_opts.version) {
		std::cout << "Battle for Wesnoth" << " " << game_config::wesnoth_version.str() << "\n\n";
		std::cout << "Library versions:\n" << game_config::library_versions_report() << '\n';
		std::cout << "Optional features:\n" << game_config::optional_features_report();

		return 0;
	}

	if(cmdline_opts.simple_version) {
		std::cout << game_config::wesnoth_version.str() << "\n";

		return 0;
	}

	if(cmdline_opts.report) {
		std::cout << "\n========= BUILD INFORMATION =========\n\n" << game_config::full_build_report();
		return 0;
	}

	if(cmdline_opts.validate_schema) {
		schema_validation::schema_self_validator validator;
		validator.set_create_exceptions(false); // Don't crash if there's an error, just go ahead anyway
		return handle_validate_command(*cmdline_opts.validate_schema, validator, {});
	}

	if(cmdline_opts.do_diff) {
		config left, right;
		std::ifstream in_left(cmdline_opts.diff_left);
		std::ifstream in_right(cmdline_opts.diff_right);
		read(left, in_left);
		read(right, in_right);
		std::ostream* os = &std::cout;
		if(cmdline_opts.output_file) {
			os = new std::ofstream(*cmdline_opts.output_file);
		}
		config_writer out(*os, compression::format::none);
		out.write(right.get_diff(left));
		if(os != &std::cout) delete os;
		return 0;
	}

	if(cmdline_opts.do_patch) {
		config base, diff;
		std::ifstream in_base(cmdline_opts.diff_left);
		std::ifstream in_diff(cmdline_opts.diff_right);
		read(base, in_base);
		read(diff, in_diff);
		base.apply_diff(diff);
		std::ostream* os = &std::cout;
		if(cmdline_opts.output_file) {
			os = new std::ofstream(*cmdline_opts.output_file);
		}
		config_writer out(*os, compression::format::none);
		out.write(base);
		if(os != &std::cout) delete os;
		return 0;
	}

	if(cmdline_opts.generate_spritesheet) {
		PLAIN_LOG << "sheet path " << *cmdline_opts.generate_spritesheet;
		image::build_spritesheet_from(*cmdline_opts.generate_spritesheet);
		return 0;
	}

	// Options changing their behavior dependent on some others should be checked below.

	if(cmdline_opts.preprocess) {
		handle_preprocess_command(cmdline_opts);
		return 0;
	}

	if(cmdline_opts.preprocess_source_string.has_value()) {
		handle_preprocess_string(cmdline_opts);
		return 0;
	}

	if(cmdline_opts.validate_wml) {
		std::string schema_path;
		if(cmdline_opts.validate_with) {
			schema_path = *cmdline_opts.validate_with;
			if(!filesystem::file_exists(schema_path)) {
				if(auto check = filesystem::get_wml_location(schema_path)) {
					schema_path = check.value();
				} else {
					PLAIN_LOG << "Could not find schema file: " << schema_path;
				}
			} else {
				schema_path = filesystem::normalize_path(schema_path);
			}
		} else {
			schema_path = filesystem::get_wml_location("schema/game_config.cfg").value();
		}
		schema_validation::schema_validator validator(schema_path);
		validator.set_create_exceptions(false); // Don't crash if there's an error, just go ahead anyway
		return handle_validate_command(*cmdline_opts.validate_wml, validator,
			cmdline_opts.preprocess_defines.value_or<decltype(cmdline_opts.preprocess_defines)::value_type>({}));
	}

	if(cmdline_opts.preprocess_defines || cmdline_opts.preprocess_input_macros || cmdline_opts.preprocess_path) {
		// It would be good if this was supported for running tests too, possibly for other uses.
		// For the moment show an error message instead of leaving the user wondering why it doesn't work.
		PLAIN_LOG << "That --preprocess-* option is only supported when using --preprocess or --validate.";
		return 2;
	}

	// Not the most intuitive solution, but I wanted to leave current semantics for now
	return -1;
}

/**
 * I would prefer to setup locale first so that early error
 * messages can get localized, but we need the game_launcher
 * initialized to have filesystem::get_intl_dir() to work.  Note: setlocale()
 * does not take GUI language setting into account.
 */
static void init_locale()
{
#if defined _WIN32 || defined __APPLE__
	setlocale(LC_ALL, "English");
#else
	std::setlocale(LC_ALL, "C");
#endif

	const std::string& intl_dir = filesystem::get_intl_dir();

	translation::bind_textdomain(PACKAGE, intl_dir.c_str(), "UTF-8");
	translation::bind_textdomain(PACKAGE "-lib", intl_dir.c_str(), "UTF-8");
	translation::set_default_textdomain(PACKAGE);
}

/**
 * Print an alert and instructions to stderr about early initialization errors.
 *
 * This is provided as an aid for users dealing with potential data dir
 * configuration issues. The first code to read core WML *has* the
 * responsibility to call this function in the event of a problem, to inform
 * the user of the most likely possible cause and suggest a course of action
 * to solve the issue.
 */
static void warn_early_init_failure()
{
	// NOTE: wrap output to 80 columns.
	PLAIN_LOG << '\n'
			  << "An error at this point during initialization usually indicates that the data\n"
			  << "directory above was not correctly set or detected. Try passing the correct path\n"
			  << "in the command line with the --data-dir switch or as the only argument.";
}

/**
 * Handles the lua script command line arguments if present.
 * This function will only run once.
 */
static void handle_lua_script_args(game_launcher* game, commandline_options& /*cmdline_opts*/)
{
	static bool first_time = true;

	if(!first_time) {
		return;
	}

	first_time = false;

	if(!game->init_lua_script()) {
		// PLAIN_LOG << "error when loading lua scripts at startup";
		// PLAIN_LOG << "could not load lua script: " << *cmdline_opts.script_file;
	}
}

#ifdef _MSC_VER
static void check_fpu()
{
	uint32_t f_control;

	if(_controlfp_s(&f_control, 0, 0) == 0) {
		uint32_t unused;
		uint32_t rounding_mode = f_control & _MCW_RC;

		if(rounding_mode != _RC_NEAR) {
			PLAIN_LOG << "Floating point rounding mode is currently '"
				<< ((rounding_mode == _RC_CHOP)
					? "chop"
					: (rounding_mode == _RC_UP)
						? "up"
						: (rounding_mode == _RC_DOWN)
							? "down"
							: (rounding_mode == _RC_NEAR) ? "near" : "unknown")
				<< "' setting to 'near'";

			if(_controlfp_s(&unused, _RC_NEAR, _MCW_RC)) {
				PLAIN_LOG << "failed to set floating point rounding type to 'near'";
			}
		}

#ifndef _M_AMD64
		uint32_t precision_mode = f_control & _MCW_PC;
		if(precision_mode != _PC_53) {
			PLAIN_LOG << "Floating point precision mode is currently '"
				<< ((precision_mode == _PC_53)
					? "double"
					: (precision_mode == _PC_24)
						? "single"
						: (precision_mode == _PC_64) ? "double extended" : "unknown")
				<< "' setting to 'double'";

			if(_controlfp_s(&unused, _PC_53, _MCW_PC)) {
				PLAIN_LOG << "failed to set floating point precision type to 'double'";
			}
		}
#endif

	} else {
		PLAIN_LOG << "_controlfp_s failed.";
	}
}
#else
static void check_fpu()
{
	switch(fegetround()) {
	case FE_TONEAREST:
		break;
	case FE_DOWNWARD:
		STREAMING_LOG << "Floating point precision mode is currently 'downward'";
		goto reset_fpu;
	case FE_TOWARDZERO:
		STREAMING_LOG << "Floating point precision mode is currently 'toward-zero'";
		goto reset_fpu;
	case FE_UPWARD:
		STREAMING_LOG << "Floating point precision mode is currently 'upward'";
		goto reset_fpu;
	default:
		STREAMING_LOG << "Floating point precision mode is currently 'unknown'";
		goto reset_fpu;
	reset_fpu:
		STREAMING_LOG << " - setting to 'nearest'\n";
		fesetround(FE_TONEAREST);
		break;
	}
}
#endif

/**
 * Setups the game environment and enters
 * the titlescreen or game loops.
 */
static int do_gameloop(commandline_options& cmdline_opts)
{
	srand(std::time(nullptr));

	const auto game = std::make_unique<game_launcher>(cmdline_opts);

	init_locale();

	bool res;

	// Do initialize fonts before reading the game config, to have game
	// config error messages displayed. fonts will be re-initialized later
	// when the language is read from the game config.
	res = font::load_font_config();
	if(res == false) {
		PLAIN_LOG << "could not initialize fonts";
		// The most common symptom of a bogus data dir path -- warn the user.
		warn_early_init_failure();
		return 1;
	}

	res = game->init_language();
	if(res == false) {
		PLAIN_LOG << "could not initialize the language";
		return 1;
	}

	res = game->init_video();
	if(res == false) {
		PLAIN_LOG << "could not initialize display";
		return 1;
	}

	check_fpu();
	const cursor::manager cursor_manager;
	cursor::set(cursor::WAIT);

#if(defined(_X11) && !defined(__APPLE__)) || defined(_WIN32)
	SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
#endif

	gui2::init();
	gui2::switch_theme(prefs::get().gui2_theme());
	const gui2::event::manager gui_event_manager;

	// if the log directory is not writable, then this is the error condition so show the error message.
	// if the log directory is writable, then there's no issue.
	// if the optional isn't set, then logging to file has been disabled, so there's no issue.
	if(!lg::log_dir_writable().value_or(true)) {
		utils::string_map symbols;
		symbols["logdir"] = filesystem::get_logs_dir();
		std::string msg = VGETTEXT("Unable to create log files in directory $logdir. This is often caused by incorrect folder permissions, anti-virus software restricting folder access, or using OneDrive to manage your My Documents folder.", symbols);
		gui2::show_message(_("Logging Failure"), msg, message::ok_button);
	}

	game_config_manager config_manager(cmdline_opts);

	if(game_config::check_migration) {
		game_config::check_migration = false;
		migrate_version_selection::execute();
	}

	loading_screen::display([&res, &config_manager, &cmdline_opts]() {
		loading_screen::progress(loading_stage::load_config);
		res = config_manager.init_game_config(game_config_manager::NO_FORCE_RELOAD);

		if(res == false) {
			PLAIN_LOG << "could not initialize game config";
			return;
		}

		loading_screen::progress(loading_stage::init_fonts);

		res = font::load_font_config();
		if(res == false) {
			PLAIN_LOG << "could not re-initialize fonts for the current language";
			return;
		}

		if(!game_config::no_addons && !cmdline_opts.noaddons)  {
			loading_screen::progress(loading_stage::refresh_addons);

			refresh_addon_version_info_cache();
		}
	});

	if(res == false) {
		return 1;
	}

	plugins_manager plugins_man(new application_lua_kernel);

	const plugins_context::reg_vec callbacks {
		{"play_multiplayer", std::bind(&game_launcher::play_multiplayer, game.get(), game_launcher::mp_mode::CONNECT)},
		{"play_local", std::bind(&game_launcher::play_multiplayer, game.get(), game_launcher::mp_mode::LOCAL)},
		{"play_campaign", std::bind(&game_launcher::play_campaign, game.get())},
	};

	const plugins_context::areg_vec accessors {
		{"command_line", std::bind(&commandline_options::to_config, &cmdline_opts)},
	};

	plugins_context plugins("titlescreen", callbacks, accessors);

	plugins.set_callback("exit", [](const config& cfg) { safe_exit(cfg["code"].to_int(0)); }, false);

	while(true) {
		if(!game->has_load_data()) {
			auto cfg = config_manager.game_config().optional_child("titlescreen_music");
			if(cfg) {
				for(const config& i : cfg->child_range("music")) {
					sound::play_music_config(i);
				}

				config title_music_config;
				title_music_config["name"] = game_config::title_music;
				title_music_config["append"] = true;
				title_music_config["immediate"] = true;
				sound::play_music_config(title_music_config);
			} else {
				sound::empty_playlist();
				sound::stop_music();
			}
		}

		handle_lua_script_args(&*game, cmdline_opts);

		plugins.play_slice();
		plugins.play_slice();

		if(!cmdline_opts.unit_test.empty()) {
			return static_cast<int>(game->unit_test());
		}

		if(game->play_test() == false) {
			return 0;
		}

		if(game->play_screenshot_mode() == false) {
			return 0;
		}

		if(game->play_render_image_mode() == false) {
			return 0;
		}

		// Start directly a campaign
		if(game->goto_campaign() == false) {
			if(game->jump_to_campaign_id().empty())
				continue; // Go to main menu
			else
				return 1; // we got an error starting the campaign from command line
		}

		// Start directly a multiplayer
		// Eventually with a specified server
		if(game->goto_multiplayer() == false) {
			continue; // Go to main menu
		}

		// Start directly a commandline multiplayer game
		if(game->play_multiplayer_commandline() == false) {
			return 0;
		}

		if(game->goto_editor() == false) {
			return 0;
		}

		const font::floating_label_context label_manager;

		cursor::set(cursor::NORMAL);

		// If loading a game, skip the titlescreen entirely
		if(game->has_load_data() && game->load_game()) {
			game->launch_game(game_launcher::reload_mode::RELOAD_DATA);
			continue;
		}

		int retval;
		{ // scope to not keep the title screen alive all game
			title_screen dlg(*game);

			// Allows re-layout on resize.
			// Since RELOAD_UI is not checked here, it causes
			// the dialog to be closed and reshown with changes.
			while(dlg.get_retval() == title_screen::REDRAW_BACKGROUND) {
				dlg.show();
			}
			retval = dlg.get_retval();
		}

		switch(retval) {
		case title_screen::QUIT_GAME:
			LOG_GENERAL << "quitting game...";
			return 0;
		case title_screen::MP_CONNECT:
			game->play_multiplayer(game_launcher::mp_mode::CONNECT);
			break;
		case title_screen::MP_HOST:
			game->play_multiplayer(game_launcher::mp_mode::HOST);
			break;
		case title_screen::MP_LOCAL:
			game->play_multiplayer(game_launcher::mp_mode::LOCAL);
			break;
		case title_screen::RELOAD_GAME_DATA:
			loading_screen::display([&config_manager]() {
				config_manager.reload_changed_game_config();
				gui2::init();
				gui2::switch_theme(prefs::get().gui2_theme());
			});
			break;
		case title_screen::MAP_EDITOR:
			game->start_editor();
			break;
		case title_screen::LAUNCH_GAME:
			game->launch_game(game_launcher::reload_mode::RELOAD_DATA);
			break;
		case title_screen::REDRAW_BACKGROUND:
			break;
		case title_screen::RELOAD_UI:
			gui2::switch_theme(prefs::get().gui2_theme());
			break;
		}
	}
}

#ifdef _WIN32
#define error_exit(res)                                                                                                \
	do {                                                                                                               \
		if(lg::using_own_console()) {                                                                                  \
			std::cerr << "Press enter to continue..." << std::endl;                                                    \
			std::cin.get();                                                                                            \
		}                                                                                                              \
		return res;                                                                                                    \
	} while(false)
#else
#define error_exit(res) return res
#endif

#ifdef __APPLE__
extern "C" int wesnoth_main(int argc, char** argv);
int wesnoth_main(int argc, char** argv)
#else
int main(int argc, char** argv)
#endif
{
	auto args = read_argv(argc, argv);
	assert(!args.empty());

#ifdef _WIN32
	_putenv("PANGOCAIRO_BACKEND=fontconfig");
	_putenv("FONTCONFIG_PATH=fonts");
#endif
#ifdef __APPLE__
	// Using setenv with overwrite disabled so we can override this in the
	// original process environment for research/testing purposes.
	setenv("PANGOCAIRO_BACKEND", "fontconfig", 0);
#endif

	try {
		commandline_options cmdline_opts = commandline_options(args);
		int finished = process_command_args(cmdline_opts);

		if(finished != -1) {
#ifdef _WIN32
			if(lg::using_own_console()) {
				std::cerr << "Press enter to continue..." << std::endl;
				std::cin.get();
			}
#endif
			safe_exit(finished);
		}

		SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");
		// Is there a reason not to just use SDL_INIT_EVERYTHING?
		if(SDL_Init(SDL_INIT_TIMER) < 0) {
			PLAIN_LOG << "Couldn't initialize SDL: " << SDL_GetError();
			return (1);
		}
		atexit(SDL_Quit);

		// Mac's touchpad generates touch events too.
		// Ignore them until Macs have a touchscreen: https://forums.libsdl.org/viewtopic.php?p=45758
#if defined(__APPLE__) && !defined(__IPHONEOS__)
		SDL_EventState(SDL_FINGERMOTION, SDL_DISABLE);
		SDL_EventState(SDL_FINGERDOWN, SDL_DISABLE);
		SDL_EventState(SDL_FINGERUP, SDL_DISABLE);
#endif

		// declare this here so that it will always be at the front of the event queue.
		events::event_context global_context;

		SDL_StartTextInput();

		const int res = do_gameloop(cmdline_opts);
		safe_exit(res);
	} catch(const boost::program_options::error& e) {
		// logging hasn't been initialized by this point
		std::cerr << "Error in command line: " << e.what() << std::endl;
		std::string error = "Error parsing command line arguments: ";
		error += e.what();
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", error.c_str(), nullptr);
		error_exit(2);
	} catch(const video::error& e) {
		PLAIN_LOG << "Video system error: " << e.what();
		error_exit(1);
	} catch(const font::error& e) {
		PLAIN_LOG << "Could not initialize fonts.\n\n" << e.what() << "\n\nExiting.";
		error_exit(1);
	} catch(const config::error& e) {
		PLAIN_LOG << e.message;
		error_exit(1);
	} catch(const video::quit&) {
		// just means the game should quit
	} catch(const return_to_play_side_exception&) {
		PLAIN_LOG << "caught return_to_play_side_exception, please report this bug (quitting)";
	} catch(const quit_game_exception&) {
		PLAIN_LOG << "caught quit_game_exception (quitting)";
	} catch(const wml_exception& e) {
		PLAIN_LOG << "WML exception:\nUser message: " << e.user_message << "\nDev message: " << e.dev_message;
		error_exit(1);
	} catch(const wfl::formula_error& e) {
		PLAIN_LOG << e.what() << "\n\nGame will be aborted.";
		error_exit(1);
	} catch(const sdl::exception& e) {
		PLAIN_LOG << e.what();
		error_exit(1);
	} catch(const game::error& e) {
		PLAIN_LOG << "Game error: " << e.what();
		error_exit(1);
	} catch(const std::bad_alloc&) {
		PLAIN_LOG << "Ran out of memory. Aborted.";
		error_exit(ENOMEM);
#if !defined(NO_CATCH_AT_GAME_END)
	} catch(const std::exception& e) {
		// Try to catch unexpected exceptions.
		PLAIN_LOG << "Caught general '" << typeid(e).name() << "' exception:\n" << e.what();
		error_exit(1);
	} catch(const std::string& e) {
		PLAIN_LOG << "Caught a string thrown as an exception:\n" << e;
		error_exit(1);
	} catch(const char* e) {
		PLAIN_LOG << "Caught a string thrown as an exception:\n" << e;
		error_exit(1);
	} catch(...) {
		// Ensure that even when we terminate with `throw 42`, the exception
		// is caught and all destructors are actually called. (Apparently,
		// some compilers will simply terminate without calling destructors if
		// the exception isn't caught.)
		PLAIN_LOG << "Caught general exception " << utils::get_unknown_exception_type() << ". Terminating.";
		error_exit(1);
#endif
	}

	return 0;
} // end main
