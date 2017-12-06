/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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
#include "commandline_options.hpp" // for commandline_options, etc
#include "config.hpp"              // for config, config::error, etc
#include "cursor.hpp"              // for set, CURSOR_TYPE::NORMAL, etc
#include "editor/editor_main.hpp"
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
#include "gui/dialogs/end_credits.hpp"
#include "gui/dialogs/loading_screen.hpp"
#include "gui/dialogs/message.hpp"      // for show_error_message
#include "gui/dialogs/title_screen.hpp" // for title_screen, etc
#include "gui/widgets/helper.hpp"       // for init
#include "image.hpp"                    // for flush_cache, etc
#include "log.hpp"                      // for LOG_STREAM, general, logger, etc
#include "preferences/general.hpp"      // for core_id, etc
#include "scripting/application_lua_kernel.hpp"
#include "scripting/plugins/context.hpp"
#include "scripting/plugins/manager.hpp"
#include "sdl/exception.hpp" // for exception
#include "sdl/rect.hpp"
#include "serialization/binary_or_text.hpp" // for config_writer
#include "serialization/parser.hpp"         // for read
#include "serialization/preprocessor.hpp"   // for preproc_define, etc
#include "serialization/string_utils.hpp"
#include "serialization/unicode_cast.hpp"
#include "serialization/validator.hpp" // for strict_validation_enabled
#include "sound.hpp"                   // for commit_music_changes, etc
#include "statistics.hpp"              // for fresh_stats
#include "tstring.hpp"                 // for operator==, t_string
#include "utils/functional.hpp"
#include "version.hpp"        // for version_info
#include "video.hpp"          // for CVideo
#include "wesconfig.h"        // for PACKAGE
#include "widgets/button.hpp" // for button
#include "wml_exception.hpp"  // for wml_exception

#ifdef _WIN32
#include "log_windows.hpp"

#include <float.h>
#endif // _WIN32

#ifndef _MSC_VER
#include <fenv.h>
#endif // _MSC_VER

#include <SDL.h> // for SDL_Init, SDL_INIT_TIMER

#include <boost/iostreams/categories.hpp>   // for input, output
#include <boost/iostreams/copy.hpp>         // for copy
#include <boost/iostreams/filter/bzip2.hpp> // for bzip2_compressor, etc

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4456)
#pragma warning(disable : 4458)
#endif

#include <boost/iostreams/filter/gzip.hpp> // for gzip_compressor, etc

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#include <boost/iostreams/filtering_stream.hpp> // for filtering_stream
#include <boost/program_options/errors.hpp>     // for error

#include <algorithm> // for transform
#include <cerrno>    // for ENOMEM
#include <clocale>   // for setlocale, LC_ALL, etc
#include <cstdio>    // for remove, fprintf, stderr
#include <cstdlib>   // for srand, exit
#include <ctime>     // for time, ctime, time_t
#include <exception> // for exception
#include <fstream>   // for operator<<, basic_ostream, etc
#include <iostream>  // for cerr, cout
#include <vector>

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

#if defined(_OPENMP) && _MSC_VER >= 1600
#include <process.h>
#endif

#endif // _WIN32

#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
#include "gui/widgets/debug.hpp"
#endif

class end_level_exception;
namespace game
{
struct error;
}

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
	LOG_GENERAL << "exiting with code " << res << "\n";
	exit(res);
}

// maybe this should go in a util file somewhere?
template<typename filter>
static void encode(const std::string& input_file, const std::string& output_file)
{
	try {
		std::ifstream ifile(input_file.c_str(), std::ios_base::in | std::ios_base::binary);
		ifile.peek(); // We need to touch the stream to set the eof bit

		if(!ifile.good()) {
			std::cerr << "Input file " << input_file
					  << " is not good for reading. Exiting to prevent bzip2 from segfaulting\n";
			safe_exit(1);
		}

		std::ofstream ofile(output_file.c_str(), std::ios_base::out | std::ios_base::binary);

		boost::iostreams::filtering_stream<boost::iostreams::output> stream;
		stream.push(filter());
		stream.push(ofile);

		boost::iostreams::copy(ifile, stream);
		ifile.close();

		safe_exit(remove(input_file.c_str()));
	} catch(filesystem::io_exception& e) {
		std::cerr << "IO error: " << e.what() << "\n";
	}
}

template<typename filter>
static void decode(const std::string& input_file, const std::string& output_file)
{
	try {
		std::ofstream ofile(output_file.c_str(), std::ios_base::out | std::ios_base::binary);
		std::ifstream ifile(input_file.c_str(), std::ios_base::in | std::ios_base::binary);

		boost::iostreams::filtering_stream<boost::iostreams::input> stream;
		stream.push(filter());
		stream.push(ifile);

		boost::iostreams::copy(stream, ofile);
		ifile.close();

		safe_exit(remove(input_file.c_str()));
	} catch(filesystem::io_exception& e) {
		std::cerr << "IO error: " << e.what() << "\n";
	}
}

static void gzip_encode(const std::string& input_file, const std::string& output_file)
{
	encode<boost::iostreams::gzip_compressor>(input_file, output_file);
}

static void gzip_decode(const std::string& input_file, const std::string& output_file)
{
	decode<boost::iostreams::gzip_decompressor>(input_file, output_file);
}

static void bzip2_encode(const std::string& input_file, const std::string& output_file)
{
	encode<boost::iostreams::bzip2_compressor>(input_file, output_file);
}

static void bzip2_decode(const std::string& input_file, const std::string& output_file)
{
	decode<boost::iostreams::bzip2_decompressor>(input_file, output_file);
}

static void handle_preprocess_command(const commandline_options& cmdline_opts)
{
	preproc_map input_macros;

	if(cmdline_opts.preprocess_input_macros) {
		std::string file = *cmdline_opts.preprocess_input_macros;
		if(filesystem::file_exists(file) == false) {
			std::cerr << "please specify an existing file. File " << file << " doesn't exist.\n";
			return;
		}

		std::cerr << SDL_GetTicks() << " Reading cached defines from: " << file << "\n";

		config cfg;

		try {
			filesystem::scoped_istream stream = filesystem::istream_file(file);
			read(cfg, *stream);
		} catch(config::error& e) {
			std::cerr << "Caught a config error while parsing file '" << file << "':\n" << e.message << std::endl;
		}

		int read = 0;

		// use static preproc_define::read_pair(config) to make a object
		for(const config::any_child& value : cfg.all_children_range()) {
			const preproc_map::value_type def = preproc_define::read_pair(value.cfg);
			input_macros[def.first] = def.second;
			++read;
		}

		std::cerr << SDL_GetTicks() << " Read " << read << " defines.\n";
	}

	const std::string resourceToProcess(*cmdline_opts.preprocess_path);
	const std::string targetDir(*cmdline_opts.preprocess_target);

	uint32_t startTime = SDL_GetTicks();

	// If the users add the SKIP_CORE define we won't preprocess data/core
	bool skipCore = false;
	bool skipTerrainGFX = false;

	// The 'core_defines_map' is the one got from data/core macros
	preproc_map defines_map(input_macros);

	if(cmdline_opts.preprocess_defines) {
		// add the specified defines
		for(const std::string& define : *cmdline_opts.preprocess_defines) {
			if(define.empty()) {
				std::cerr << "empty define supplied\n";
				continue;
			}

			LOG_PREPROC << "adding define: " << define << '\n';
			defines_map.emplace(define, preproc_define(define));

			if(define == "SKIP_CORE") {
				std::cerr << "'SKIP_CORE' defined.\n";
				skipCore = true;
			} else if(define == "NO_TERRAIN_GFX") {
				std::cerr << "'NO_TERRAIN_GFX' defined." << std::endl;
				skipTerrainGFX = true;
			}
		}
	}

	// add the WESNOTH_VERSION define
	defines_map["WESNOTH_VERSION"] = preproc_define(game_config::wesnoth_version.str());

	std::cerr << "added " << defines_map.size() << " defines.\n";

	// preprocess core macros first if we don't skip the core
	if(skipCore == false) {
		std::cerr << "preprocessing common macros from 'data/core' ...\n";

		// process each folder explicitly to gain speed
		preprocess_resource(game_config::path + "/data/core/macros", &defines_map);

		if(skipTerrainGFX == false) {
			preprocess_resource(game_config::path + "/data/core/terrain-graphics", &defines_map);
		}

		std::cerr << "acquired " << (defines_map.size() - input_macros.size()) << " 'data/core' defines.\n";
	} else {
		std::cerr << "skipped 'data/core'\n";
	}

	// preprocess resource
	std::cerr << "preprocessing specified resource: " << resourceToProcess << " ...\n";

	preprocess_resource(resourceToProcess, &defines_map, true, true, targetDir);
	std::cerr << "acquired " << (defines_map.size() - input_macros.size()) << " total defines.\n";

	if(cmdline_opts.preprocess_output_macros) {
		std::string outputFileName = "_MACROS_.cfg";
		if(!cmdline_opts.preprocess_output_macros->empty()) {
			outputFileName = *cmdline_opts.preprocess_output_macros;
		}

		std::string outputPath = targetDir + "/" + outputFileName;

		std::cerr << "writing '" << outputPath << "' with " << defines_map.size() << " defines.\n";

		filesystem::scoped_ostream out = filesystem::ostream_file(outputPath);
		if(!out->fail()) {
			config_writer writer(*out, false);

			for(preproc_map::iterator itor = defines_map.begin(); itor != defines_map.end(); ++itor) {
				(*itor).second.write(writer, (*itor).first);
			}
		} else {
			std::cerr << "couldn't open the file.\n";
		}
	}

	std::cerr << "preprocessing finished. Took " << SDL_GetTicks() - startTime << " ticks.\n";
}

/** Process commandline-arguments */
static int process_command_args(const commandline_options& cmdline_opts)
{
	// Options that don't change behavior based on any others should be checked alphabetically below.

	if(cmdline_opts.userconfig_dir) {
		filesystem::set_user_config_dir(*cmdline_opts.userconfig_dir);
	}

	if(cmdline_opts.userconfig_path) {
		std::cout << filesystem::get_user_config_dir() << '\n';
		return 0;
	}

	if(cmdline_opts.userdata_dir) {
		filesystem::set_user_data_dir(*cmdline_opts.userdata_dir);
	}

	if(cmdline_opts.userdata_path) {
		std::cout << filesystem::get_user_data_dir() << '\n';
		return 0;
	}

	if(cmdline_opts.data_dir) {
		const std::string datadir = *cmdline_opts.data_dir;
#ifdef _WIN32
		// use c_str to ensure that index 1 points to valid element since c_str() returns null-terminated string
		if(datadir.c_str()[1] == ':') {
#else
		if(datadir[0] == '/') {
#endif
			game_config::path = datadir;
		} else {
			game_config::path = filesystem::get_cwd() + '/' + datadir;
		}

		game_config::path = filesystem::normalize_path(game_config::path, true, true);
		std::cerr << "Overriding data directory with " << game_config::path << std::endl;

		if(!filesystem::is_directory(game_config::path)) {
			std::cerr << "Could not find directory '" << game_config::path << "'\n";
			throw config::error("directory not found");
		}

		// don't update font as we already updating it in game ctor
		// font_manager_.update_font_path();
	}

	if(cmdline_opts.data_path) {
		std::cout << game_config::path << '\n';
		return 0;
	}

	if(cmdline_opts.debug_lua) {
		game_config::debug_lua = true;
	}

	if(cmdline_opts.gunzip) {
		const std::string input_file(*cmdline_opts.gunzip);
		if(!filesystem::is_gzip_file(input_file)) {
			std::cerr << "file '" << input_file << "'isn't a .gz file\n";
			return 2;
		}

		const std::string output_file(input_file, 0, input_file.length() - 3);
		gzip_decode(input_file, output_file);
	}

	if(cmdline_opts.bunzip2) {
		const std::string input_file(*cmdline_opts.bunzip2);
		if(!filesystem::is_bzip2_file(input_file)) {
			std::cerr << "file '" << input_file << "'isn't a .bz2 file\n";
			return 2;
		}

		const std::string output_file(input_file, 0, input_file.length() - 4);
		bzip2_decode(input_file, output_file);
	}

	if(cmdline_opts.gzip) {
		const std::string input_file(*cmdline_opts.gzip);
		const std::string output_file(*cmdline_opts.gzip + ".gz");
		gzip_encode(input_file, output_file);
	}

	if(cmdline_opts.bzip2) {
		const std::string input_file(*cmdline_opts.bzip2);
		const std::string output_file(*cmdline_opts.bzip2 + ".bz2");
		bzip2_encode(input_file, output_file);
	}

	if(cmdline_opts.help) {
		std::cout << cmdline_opts;
		return 0;
	}

	if(cmdline_opts.log) {
		for(std::vector<std::pair<int, std::string>>::const_iterator it = cmdline_opts.log->begin();
				it != cmdline_opts.log->end(); ++it) {
			const std::string log_domain = it->second;
			const int severity = it->first;
			if(!lg::set_log_domain_severity(log_domain, severity)) {
				std::cerr << "unknown log domain: " << log_domain << '\n';
				return 2;
			}
		}
	}

	if(cmdline_opts.logdomains) {
		std::cout << lg::list_logdomains(*cmdline_opts.logdomains);
		return 0;
	}

	if(cmdline_opts.path) {
		std::cout << game_config::path << "\n";
		return 0;
	}

	if(cmdline_opts.log_precise_timestamps) {
		lg::precise_timestamps(true);
	}

	if(cmdline_opts.rng_seed) {
		srand(*cmdline_opts.rng_seed);
	}

	if(cmdline_opts.screenshot || cmdline_opts.render_image) {
		SDL_setenv("SDL_VIDEODRIVER", "dummy", 1);
	}

	if(cmdline_opts.strict_validation) {
		strict_validation_enabled = true;
	}

	if(cmdline_opts.version) {
		std::cout << "Battle for Wesnoth" << " " << game_config::version << "\n\n";
		std::cout << "Library versions:\n" << game_config::library_versions_report() << '\n';
		std::cout << "Optional features:\n" << game_config::optional_features_report();

		return 0;
	}

	if(cmdline_opts.report) {
		std::cout << "\n========= BUILD INFORMATION =========\n\n" << game_config::full_build_report();
		return 0;
	}

	// Options changing their behavior dependent on some others should be checked below.

	if(cmdline_opts.preprocess) {
		handle_preprocess_command(cmdline_opts);
		return 0;
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
	translation::init();
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
	std::cerr << '\n'
			  << "An error at this point during initialization usually indicates that the data\n"
			  << "directory above was not correctly set or detected. Try passing the correct path\n"
			  << "in the command line with the --data-dir switch or as the only argument.\n";
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
		// std::cerr << "error when loading lua scripts at startup\n";
		// std::cerr << "could not load lua script: " << *cmdline_opts.script_file << std::endl;
	}
}

#ifdef _MSC_VER
static void check_fpu()
{
	uint32_t f_control;

	if(_controlfp_s(&f_control, 0, 0) == 0) {
		uint32_t unused;
		uint32_t rounding_mode = f_control & _MCW_RC;
		uint32_t precision_mode = f_control & _MCW_PC;

		if(rounding_mode != _RC_NEAR) {
			std::cerr << "Floating point rounding mode is currently '"
				<< ((rounding_mode == _RC_CHOP)
					? "chop"
					: (rounding_mode == _RC_UP)
						? "up"
						: (rounding_mode == _RC_DOWN)
							? "down"
							: (rounding_mode == _RC_NEAR) ? "near" : "unknown")
				<< "' setting to 'near'\n";

			if(_controlfp_s(&unused, _RC_NEAR, _MCW_RC)) {
				std::cerr << "failed to set floating point rounding type to 'near'\n";
			}
		}

		if(precision_mode != _PC_53) {
			std::cerr << "Floating point precision mode is currently '"
				<< ((precision_mode == _PC_53)
					? "double"
					: (precision_mode == _PC_24)
						? "single"
						: (precision_mode == _PC_64) ? "double extended" : "unknown")
				<< "' setting to 'double'\n";

			if(_controlfp_s(&unused, _PC_53, _MCW_PC)) {
				std::cerr << "failed to set floating point precision type to 'double'\n";
			}
		}
	} else {
		std::cerr << "_controlfp_s failed.\n";
	}
}
#else
static void check_fpu()
{
	switch(fegetround()) {
	case FE_TONEAREST:
		break;
	case FE_DOWNWARD:
		std::cerr << "Floating point precision mode is currently 'downward'";
		goto reset_fpu;
	case FE_TOWARDZERO:
		std::cerr << "Floating point precision mode is currently 'toward-zero'";
		goto reset_fpu;
	case FE_UPWARD:
		std::cerr << "Floating point precision mode is currently 'upward'";
		goto reset_fpu;
	default:
		std::cerr << "Floating point precision mode is currently 'unknown'";
		goto reset_fpu;
	reset_fpu:
		std::cerr << "setting to 'nearest'";
		fesetround(FE_TONEAREST);
		break;
	}
}
#endif

/**
 * Setups the game environment and enters
 * the titlescreen or game loops.
 */
static int do_gameloop(const std::vector<std::string>& args)
{
	srand(time(nullptr));

	commandline_options cmdline_opts = commandline_options(args);
	game_config::wesnoth_program_dir = filesystem::directory_name(args[0]);

	int finished = process_command_args(cmdline_opts);
	if(finished != -1) {
#ifdef _WIN32
		if(lg::using_own_console()) {
			std::cerr << "Press enter to continue..." << std::endl;
			std::cin.get();
		}
#endif

		return finished;
	}

	const std::unique_ptr<game_launcher> game(new game_launcher(cmdline_opts, args[0].c_str()));
	const int start_ticks = SDL_GetTicks();

	init_locale();

	bool res;

	// Do initialize fonts before reading the game config, to have game
	// config error messages displayed. fonts will be re-initialized later
	// when the language is read from the game config.
	res = font::load_font_config();
	if(res == false) {
		std::cerr << "could not initialize fonts\n";
		// The most common symptom of a bogus data dir path -- warn the user.
		warn_early_init_failure();
		return 1;
	}

	res = game->init_language();
	if(res == false) {
		std::cerr << "could not initialize the language\n";
		return 1;
	}

	res = game->init_video();
	if(res == false) {
		std::cerr << "could not initialize display\n";
		return 1;
	}

	res = image::update_from_preferences();
	if(res == false) {
		std::cerr << "could not initialize image preferences\n";
		return 1;
	}

	if(preferences::joystick_support_enabled()) {
		res = game->init_joystick();
		if(res == false) {
			std::cerr << "could not initialize joystick\n";
		}
	}

	check_fpu();
	const cursor::manager cursor_manager;
	cursor::set(cursor::WAIT);

#if(defined(_X11) && !defined(__APPLE__)) || defined(_WIN32)
	SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
#endif

	gui2::init();
	const gui2::event::manager gui_event_manager;

	game_config_manager config_manager(cmdline_opts, game->jump_to_editor());

	gui2::dialogs::loading_screen::display([&res, &config_manager]() {
		gui2::dialogs::loading_screen::progress(loading_stage::load_config);
		res = config_manager.init_game_config(game_config_manager::NO_FORCE_RELOAD);

		if(res == false) {
			std::cerr << "could not initialize game config\n";
			return;
		}

		gui2::dialogs::loading_screen::progress(loading_stage::init_fonts);

		res = font::load_font_config();
		if(res == false) {
			std::cerr << "could not re-initialize fonts for the current language\n";
			return;
		}

		gui2::dialogs::loading_screen::progress(loading_stage::refresh_addons);

		refresh_addon_version_info_cache();
	});

	if(res == false) {
		return 1;
	}

	LOG_CONFIG << "time elapsed: " << (SDL_GetTicks() - start_ticks) << " ms\n";

	plugins_manager plugins_man(new application_lua_kernel);

	plugins_context::Reg const callbacks[] {
		{"play_multiplayer", std::bind(&game_launcher::play_multiplayer, game.get(), game_launcher::MP_CONNECT)},
	};

	plugins_context::aReg const accessors[] {
		{"command_line", std::bind(&commandline_options::to_config, &cmdline_opts)},
	};

	plugins_context plugins("titlescreen", callbacks, accessors);

	plugins.set_callback("exit", [](const config& cfg) { safe_exit(cfg["code"].to_int(0)); }, false);

	for(;;) {
		// reset the TC, since a game can modify it, and it may be used
		// by images in add-ons or campaigns dialogs
		image::set_team_colors();

		statistics::fresh_stats();

		if(!game->is_loading()) {
			const config& cfg = config_manager.game_config().child("titlescreen_music");
			if(cfg) {
				sound::play_music_repeatedly(game_config::title_music);

				for(const config& i : cfg.child_range("music")) {
					sound::play_music_config(i);
				}

				sound::commit_music_changes();
			} else {
				sound::empty_playlist();
				sound::stop_music();
			}
		}

		handle_lua_script_args(&*game, cmdline_opts);

		plugins.play_slice();
		plugins.play_slice();

		if(cmdline_opts.unit_test) {
			if(cmdline_opts.timeout) {
				std::cerr << "The wesnoth built-in timeout feature has been removed.\n" << std::endl;
				std::cerr << "Please use a platform-specific script which will kill the overtime process instead.\n"
						  << std::endl;
				std::cerr << "For examples in bash, or in windows cmd, see the forums, or the wesnoth repository."
						  << std::endl;
				std::cerr
						<< "The bash script is called `run_wml_tests`, the windows script is part of the VC project.\n"
						<< std::endl;
			}

			int worker_result = game->unit_test();
			std::cerr << ((worker_result == 0) ? "PASS TEST " : "FAIL TEST ")
					  << ((worker_result == 3) ? "(INVALID REPLAY)" : "")
					  << ((worker_result == 4) ? "(ERRORED REPLAY)" : "") << ": " << *cmdline_opts.unit_test
					  << std::endl;
			return worker_result;
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

		preferences::load_hotkeys();

		const font::floating_label_context label_manager;

		cursor::set(cursor::NORMAL);

		game_launcher::RELOAD_GAME_DATA should_reload = game_launcher::RELOAD_DATA;

		// If loading a game, skip the titlescreen entirely
		if(game->is_loading()) {
			if(!game->load_game()) {
				game->clear_loaded_game();
			}

			game->launch_game(should_reload);
			continue;
		}

		gui2::dialogs::title_screen dlg(*game);

		/*
		 * Quick explanation of the titlscreen loop:
		 *
		 * The dialog's redraw_background_ flag is initialized as true in the constructor, so the dialog will always
		 * display at least once when this loop is executed. Each time it's opened, the aforementioned flag is set to
		 * false, and any selection that results in leaving the dialog simply sets the window's retval and proceeds to
		 * the appropriate action.
		 *
		 * Certain actions (such as window resizing) set the flag to true, which allows the dialog to reopen with any
		 * layout changes such as those dictated by window resolution.
		 */
		while(dlg.get_retval() == gui2::dialogs::title_screen::REDRAW_BACKGROUND) {
			dlg.show();
		}

		switch(dlg.get_retval()) {
		case gui2::dialogs::title_screen::QUIT_GAME:
			LOG_GENERAL << "quitting game...\n";
			return 0;
		case gui2::dialogs::title_screen::MP_CONNECT:
			game_config::debug = game_config::mp_debug;
			if(!game->play_multiplayer(game_launcher::MP_CONNECT)) {
				continue;
			}
			break;
		case gui2::dialogs::title_screen::MP_HOST:
			game_config::debug = game_config::mp_debug;
			if(!game->play_multiplayer(game_launcher::MP_HOST)) {
				continue;
			}
			break;
		case gui2::dialogs::title_screen::MP_LOCAL:
			game_config::debug = game_config::mp_debug;
			if(!game->play_multiplayer(game_launcher::MP_LOCAL)) {
				continue;
			}
			break;
		case gui2::dialogs::title_screen::RELOAD_GAME_DATA:
			gui2::dialogs::loading_screen::display([&config_manager]() {
				config_manager.reload_changed_game_config();
				image::flush_cache();
			});
			break;
		case gui2::dialogs::title_screen::MAP_EDITOR:
			game->start_editor();
			break;
		case gui2::dialogs::title_screen::SHOW_ABOUT:
			gui2::dialogs::end_credits::display();
			break;
		case gui2::dialogs::title_screen::LAUNCH_GAME:
			game->launch_game(should_reload);
			break;
		case gui2::dialogs::title_screen::REDRAW_BACKGROUND:
			break;
		}
	}
}

#ifdef _WIN32
static bool parse_commandline_argument(const char*& next, const char* end, std::string& res)
{
	// strip leading whitespace
	while(next != end && *next == ' ') {
		++next;
	}

	if(next == end) {
		return false;
	}

	bool is_excaped = false;

	for(; next != end; ++next) {
		if(*next == ' ' && !is_excaped) {
			break;
		} else if(*next == '"' && !is_excaped) {
			is_excaped = true;
			continue;
		} else if(*next == '"' && is_excaped && next + 1 != end && *(next + 1) == '"') {
			res.push_back('"');
			++next;
			continue;
		} else if(*next == '"' && is_excaped) {
			is_excaped = false;
			continue;
		} else {
			res.push_back(*next);
		}
	}

	return true;
}

static std::vector<std::string> parse_commandline_arguments(std::string input)
{
	const char* start = &input[0];
	const char* end = start + input.size();

	std::string buffer;
	std::vector<std::string> res;

	while(parse_commandline_argument(start, end, buffer)) {
		res.emplace_back();
		res.back().swap(buffer);
	}

	return res;
}
#endif

#ifndef _WIN32
static void wesnoth_terminate_handler(int)
{
	exit(0);
}
#endif

#if defined(_OPENMP) && _MSC_VER >= 1600
static void restart_process()
{
	wchar_t process_path[MAX_PATH];
	SetLastError(ERROR_SUCCESS);
	GetModuleFileNameW(nullptr, process_path, MAX_PATH);

	if(GetLastError() != ERROR_SUCCESS) {
		throw std::runtime_error("Failed to retrieve the process path");
	}

	std::wstring commandline_str(GetCommandLineW());

	// CreateProcessW is allowed to modify the passed command line.
	// Therefore we need to copy it.
	wchar_t* commandline_c_str = new wchar_t[commandline_str.length() + 1];
	commandline_str.copy(commandline_c_str, commandline_str.length());
	commandline_c_str[commandline_str.length()] = L'\0';

	STARTUPINFOW startup_info;
	ZeroMemory(&startup_info, sizeof(startup_info));
	startup_info.cb = sizeof(startup_info);

	PROCESS_INFORMATION process_info;
	ZeroMemory(&process_info, sizeof(process_info));

	CreateProcessW(
		process_path, commandline_c_str, nullptr, nullptr, false, 0u, nullptr, nullptr, &startup_info, &process_info);

	CloseHandle(process_info.hProcess);
	CloseHandle(process_info.hThread);

	std::exit(EXIT_SUCCESS);
}
#endif

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

int main(int argc, char** argv)
{
#ifdef _WIN32
	(void)argc;
	(void)argv;

	// windows argv is ansi encoded by default
	std::vector<std::string> args =
		parse_commandline_arguments(unicode_cast<std::string>(std::wstring(GetCommandLineW())));

	// HACK: we don't parse command line arguments using program_options until
	//       the startup banner is printed. We need to get a console up and
	//       running before then if requested, so just perform a trivial search
	//       here and let program_options ignore the switch later.
	for(size_t k = 0; k < args.size(); ++k) {
		if(args[k] == "--wconsole" ||
		   args[k] == "--help" ||
		   args[k] == "--logdomains" ||
		   args[k] == "--path" ||
		   args[k] == "--render-image" ||
		   args[k] == "--screenshot" ||
		   args[k] == "--data-path" ||
		   args[k] == "--userdata-path" ||
		   args[k] == "--userconfig-path" ||
		   args[k] == "--version"
		) {
			lg::enable_native_console_output();
			break;
		}
	}

	lg::early_log_file_setup();
#else
	std::vector<std::string> args;
	for(int i = 0; i < argc; ++i) {
		args.push_back(std::string(argv[i]));
	}
#endif

	assert(!args.empty());

#ifdef _OPENMP
	// Wesnoth is a special case for OMP
	// OMP wait strategy is to have threads busy-loop for 100ms
	// if there is nothing to do, they then go to sleep.
	// this avoids the scheduler putting the thread to sleep when work
	// is about to be available
	//
	// However Wesnoth has a lot of very small jobs that need to be done
	// at each redraw => 50fps every 2ms.
	// All the threads are thus busy-waiting all the time, hogging the CPU
	// To avoid that problem, we need to set the OMP_WAIT_POLICY env var
	// but that var is read by OMP at library loading time (before main)
	// thus the relaunching of ourselves after setting the variable.
#if !defined(_WIN32) && !defined(__APPLE__)
	if(!getenv("OMP_WAIT_POLICY")) {
		setenv("OMP_WAIT_POLICY", "PASSIVE", 1);
		execv(argv[0], argv);
	}
#elif _MSC_VER >= 1600
	if(!getenv("OMP_WAIT_POLICY")) {
		_putenv_s("OMP_WAIT_POLICY", "PASSIVE");
		restart_process();
	}
#endif
#endif //_OPENMP

	filesystem::init();

	if(SDL_Init(SDL_INIT_TIMER) < 0) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		return (1);
	}

#ifndef _WIN32
	struct sigaction terminate_handler;
	terminate_handler.sa_handler = wesnoth_terminate_handler;
	terminate_handler.sa_flags = 0;

	sigemptyset(&terminate_handler.sa_mask);
	sigaction(SIGTERM, &terminate_handler, nullptr);
	sigaction(SIGINT, &terminate_handler, nullptr);

	// Explicitly ignore SIGCHLD. This allows us to launch child processes without waiting
	// for them to exit. See the manpage for wait(2).
	terminate_handler.sa_handler = SIG_IGN;
	sigaction(SIGCHLD, &terminate_handler, nullptr);
#endif

	// declare this here so that it will always be at the front of the event queue.
	events::event_context global_context;

	SDL_StartTextInput();

	try {
		std::cerr << "Battle for Wesnoth v" << game_config::revision << '\n';
		const time_t t = time(nullptr);
		std::cerr << "Started on " << ctime(&t) << "\n";

		const std::string& exe_dir = filesystem::get_exe_dir();
		if(!exe_dir.empty()) {
			// Try to autodetect the location of the game data dir. Note that
			// the root of the source tree currently doubles as the data dir.
			std::string auto_dir;

			// scons leaves the resulting binaries at the root of the source
			// tree by default.
			if(filesystem::file_exists(exe_dir + "/data/_main.cfg")) {
				auto_dir = exe_dir;
			}
			// cmake encourages creating a subdir at the root of the source
			// tree for the build, and the resulting binaries are found in it.
			else if(filesystem::file_exists(exe_dir + "/../data/_main.cfg")) {
				auto_dir = filesystem::normalize_path(exe_dir + "/..");
			}
			// In Windows debug builds, the EXE is placed away from the game data dir
			// (in projectfiles\VCx\Debug), but the working directory is set to the
			// game data dir. Thus, check if the working dir is the game data dir.
			else if(filesystem::file_exists(filesystem::get_cwd() + "/data/_main.cfg")) {
				auto_dir = filesystem::get_cwd();
			}

			if(!auto_dir.empty()) {
				std::cerr << "Automatically found a possible data directory at " << auto_dir << '\n';
				game_config::path = auto_dir;
			}
		}

		const int res = do_gameloop(args);
		safe_exit(res);
	} catch(boost::program_options::error& e) {
		std::cerr << "Error in command line: " << e.what() << '\n';
		error_exit(1);
	} catch(CVideo::error& e) {
		std::cerr << "Could not initialize video.\n\n" << e.what() << "\n\nExiting.\n";
		error_exit(1);
	} catch(font::error& e) {
		std::cerr << "Could not initialize fonts.\n\n" << e.what() << "\n\nExiting.\n";
		error_exit(1);
	} catch(config::error& e) {
		std::cerr << e.message << "\n";
		error_exit(1);
	} catch(gui::button::error&) {
		std::cerr << "Could not create button: Image could not be found\n";
		error_exit(1);
	} catch(CVideo::quit&) {
		// just means the game should quit
	} catch(return_to_play_side_exception&) {
		std::cerr << "caught return_to_play_side_exception, please report this bug (quitting)\n";
	} catch(quit_game_exception&) {
		std::cerr << "caught quit_game_exception (quitting)\n";
	} catch(wml_exception& e) {
		std::cerr << "WML exception:\nUser message: " << e.user_message << "\nDev message: " << e.dev_message << '\n';
		error_exit(1);
	} catch(wfl::formula_error& e) {
		std::cerr << e.what() << "\n\nGame will be aborted.\n";
		error_exit(1);
	} catch(const sdl::exception& e) {
		std::cerr << e.what();
		error_exit(1);
	} catch(game::error&) {
		// A message has already been displayed.
		error_exit(1);
	} catch(std::bad_alloc&) {
		std::cerr << "Ran out of memory. Aborted.\n";
		error_exit(ENOMEM);
#if !defined(NO_CATCH_AT_GAME_END)
	} catch(const std::exception& e) {
		// Try to catch unexpected exceptions.
		std::cerr << "Caught general '" << typeid(e).name() << "' exception:\n" << e.what() << std::endl;
		error_exit(1);
	} catch(std::string& e) {
		std::cerr << "Caught a string thrown as an exception:\n" << e << std::endl;
		error_exit(1);
	} catch(const char* e) {
		std::cerr << "Caught a string thrown as an exception:\n" << e << std::endl;
		error_exit(1);
	} catch(...) {
		// Ensure that even when we terminate with `throw 42`, the exception
		// is caught and all destructors are actually called. (Apparently,
		// some compilers will simply terminate without calling destructors if
		// the exception isn't caught.)
		std::cerr << "Caught unspecified general exception. Terminating." << std::endl;
		error_exit(1);
#endif
	}

	return 0;
} // end main
