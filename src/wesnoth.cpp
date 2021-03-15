/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
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
#include "gui/gui.hpp"                  // for init
#include "picture.hpp"                    // for flush_cache, etc
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
#include "serialization/unicode_cast.hpp"
#include "serialization/schema_validator.hpp" // for strict_validation_enabled and schema_validator
#include "sound.hpp"                   // for commit_music_changes, etc
#include "statistics.hpp"              // for fresh_stats
#include <functional>
#include "game_version.hpp"        // for version_info
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

#include <SDL2/SDL.h> // for SDL_Init, SDL_INIT_TIMER

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
#include <optional>

#include <algorithm> // for transform
#include <cerrno>    // for ENOMEM
#include <clocale>   // for setlocale, LC_ALL, etc
#include <cstdio>    // for remove, fprintf, stderr
#include <cstdlib>   // for srand, exit
#include <ctime>     // for time, ctime, std::time_t
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
	} catch(const filesystem::io_exception& e) {
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
	} catch(const filesystem::io_exception& e) {
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
		} catch(const config::error& e) {
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

			for(auto& define_pair : defines_map) {
				define_pair.second.write(writer, define_pair.first);
			}
		} else {
			std::cerr << "couldn't open the file.\n";
		}
	}

	std::cerr << "preprocessing finished. Took " << SDL_GetTicks() - startTime << " ticks.\n";
}

static int handle_validate_command(const std::string& file, abstract_validator& validator, const std::vector<std::string>& defines) {
	preproc_map defines_map;
	for(const std::string& define : defines) {
		if(define.empty()) {
			std::cerr << "empty define supplied\n";
			continue;
		}

		LOG_PREPROC << "adding define: " << define << '\n';
		defines_map.emplace(define, preproc_define(define));
	}
	std::cout << "Validating " << file << " against schema " << validator.name_ << std::endl;
	lg::set_strict_severity(0);
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
static int process_command_args(const commandline_options& cmdline_opts)
{
	// Options that don't change behavior based on any others should be checked alphabetically below.

	if(cmdline_opts.log) {
		for(const auto& log_pair : *cmdline_opts.log) {
			const std::string log_domain = log_pair.second;
			const int severity = log_pair.first;
			if(!lg::set_log_domain_severity(log_domain, severity)) {
				std::cerr << "unknown log domain: " << log_domain << '\n';
				return 2;
			}
		}
	}

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
		if(!cmdline_opts.nobanner) std::cerr << "Overriding data directory with " << game_config::path << std::endl;

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

	if(cmdline_opts.strict_lua) {
		game_config::strict_lua = true;
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

	if(cmdline_opts.logdomains) {
		std::cout << lg::list_logdomains(*cmdline_opts.logdomains);
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
		std::cout << "Battle for Wesnoth" << " " << game_config::wesnoth_version.str() << "\n\n";
		std::cout << "Library versions:\n" << game_config::library_versions_report() << '\n';
		std::cout << "Optional features:\n" << game_config::optional_features_report();

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
		config_writer out(*os, compression::format::NONE);
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
		config_writer out(*os, compression::format::NONE);
		out.write(base);
		if(os != &std::cout) delete os;
		return 0;
	}

	// Options changing their behavior dependent on some others should be checked below.

	if(cmdline_opts.preprocess) {
		handle_preprocess_command(cmdline_opts);
		return 0;
	}

	if(cmdline_opts.validate_wml) {
		std::string schema_path;
		if(cmdline_opts.validate_with) {
			schema_path = *cmdline_opts.validate_with;
		} else {
			schema_path = filesystem::get_wml_location("schema/game_config.cfg");
		}
		schema_validation::schema_validator validator(schema_path);
		validator.set_create_exceptions(false); // Don't crash if there's an error, just go ahead anyway
		return handle_validate_command(*cmdline_opts.validate_wml, validator,
			cmdline_opts.preprocess_defines.value_or<decltype(cmdline_opts.preprocess_defines)::value_type>({}));
	}

	if(cmdline_opts.preprocess_defines || cmdline_opts.preprocess_input_macros || cmdline_opts.preprocess_path) {
		// It would be good if this was supported for running tests too, possibly for other uses.
		// For the moment show an error message instead of leaving the user wondering why it doesn't work.
		std::cerr << "That --preprocess-* option is only supported when using --preprocess or --validate-wml.\n";
		// Return an error status other than -1, because in our caller -1 means no error
		return -2;
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

#ifndef _M_AMD64
		uint32_t precision_mode = f_control & _MCW_PC;
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
#endif

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
	srand(std::time(nullptr));

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

	const auto game = std::make_unique<game_launcher>(cmdline_opts);
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

	check_fpu();
	const cursor::manager cursor_manager;
	cursor::set(cursor::WAIT);

#if(defined(_X11) && !defined(__APPLE__)) || defined(_WIN32)
	SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
#endif

	gui2::init();
	const gui2::event::manager gui_event_manager;

	game_config_manager config_manager(cmdline_opts);

	gui2::dialogs::loading_screen::display([&res, &config_manager, &cmdline_opts]() {
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

		if(!game_config::no_addons && !cmdline_opts.noaddons)  {
			gui2::dialogs::loading_screen::progress(loading_stage::refresh_addons);

			refresh_addon_version_info_cache();
		}
	});

	if(res == false) {
		return 1;
	}

	LOG_CONFIG << "time elapsed: " << (SDL_GetTicks() - start_ticks) << " ms\n";

	plugins_manager plugins_man(new application_lua_kernel);

	const plugins_context::reg_vec callbacks {
		{"play_multiplayer", std::bind(&game_launcher::play_multiplayer, game.get(), game_launcher::mp_mode::CONNECT)},
	};

	const plugins_context::areg_vec accessors {
		{"command_line", std::bind(&commandline_options::to_config, &cmdline_opts)},
	};

	plugins_context plugins("titlescreen", callbacks, accessors);

	plugins.set_callback("exit", [](const config& cfg) { safe_exit(cfg["code"].to_int(0)); }, false);

	while(true) {
		statistics::fresh_stats();

		if(!game->has_load_data()) {
			const config& cfg = config_manager.game_config().child("titlescreen_music");
			if(cfg) {
				for(const config& i : cfg.child_range("music")) {
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

		gui2::dialogs::title_screen dlg(*game);

		// Allows re-layout on resize
		while(dlg.get_retval() == gui2::dialogs::title_screen::REDRAW_BACKGROUND) {
			dlg.show();
		}

		switch(dlg.get_retval()) {
		case gui2::dialogs::title_screen::QUIT_GAME:
			LOG_GENERAL << "quitting game...\n";
			return 0;
		case gui2::dialogs::title_screen::MP_CONNECT:
			game_config::set_debug(game_config::mp_debug);
			game->play_multiplayer(game_launcher::mp_mode::CONNECT);
			break;
		case gui2::dialogs::title_screen::MP_HOST:
			game_config::set_debug(game_config::mp_debug);
			game->play_multiplayer(game_launcher::mp_mode::HOST);
			break;
		case gui2::dialogs::title_screen::MP_LOCAL:
			game_config::set_debug(game_config::mp_debug);
			game->play_multiplayer(game_launcher::mp_mode::LOCAL);
			break;
		case gui2::dialogs::title_screen::RELOAD_GAME_DATA:
			gui2::dialogs::loading_screen::display([&config_manager]() {
				config_manager.reload_changed_game_config();
			});
			break;
		case gui2::dialogs::title_screen::MAP_EDITOR:
			game->start_editor();
			break;
		case gui2::dialogs::title_screen::SHOW_ABOUT:
			gui2::dialogs::end_credits::display();
			break;
		case gui2::dialogs::title_screen::LAUNCH_GAME:
			game->launch_game(game_launcher::reload_mode::RELOAD_DATA);
			break;
		case gui2::dialogs::title_screen::REDRAW_BACKGROUND:
			break;
		}
	}
}

#ifndef _WIN32
static void wesnoth_terminate_handler(int)
{
	exit(0);
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

#ifdef __APPLE__
extern "C" int wesnoth_main(int argc, char** argv);
int wesnoth_main(int argc, char** argv)
#else
int main(int argc, char** argv)
#endif
{
	auto args = read_argv(argc, argv);
	assert(!args.empty());
	
	// --nobanner needs to be detected before the main command-line parsing happens
	bool nobanner = false;
	for(const auto& arg : args) {
		if(arg == "--nobanner") {
			nobanner = true;
			break;
		}
	}

#ifdef _WIN32
	bool log_redirect = true, native_console_implied = false;
	// This is optional<bool> instead of tribool because value_or() is exactly the required semantic
	std::optional<bool> native_console_force;
	// Some switches force a Windows console to be attached to the process even
	// if Wesnoth is an IMAGE_SUBSYSTEM_WINDOWS_GUI executable because they
	// turn it into a CLI application. Also, --wconsole in particular attaches
	// a console to a regular GUI game session.
	//
	// It's up to commandline_options later to handle these switches (other
	// --wconsole) later and emit any applicable console output, but right here
	// we need a rudimentary check for the switches in question to set up the
	// console before proceeding any further.
	for(const auto& arg : args) {
		// Switches that don't take arguments
		static const std::set<std::string> wincon_switches = {
			"--wconsole", "-h", "--help", "-v", "--version", "-R", "--report", "--logdomains",
			"--data-path", "--userdata-path", "--userconfig-path",
		};

		// Switches that take arguments, the switch may have the argument past
		// the first = character, or in a subsequent argv entry which we don't
		// care about -- we just want to see if the switch is there.
		static const std::set<std::string> wincon_arg_switches = {
			"-D", "--diff", "-p", "--preprocess", "-P", "--patch", "--render-image",
			 "--screenshot", "-V", "--validate", "--validate-schema",
		};

		auto switch_matches_arg = [&arg](const std::string& sw) {
			const auto pos = arg.find('=');
			return pos == std::string::npos ? arg == sw : arg.substr(0, pos) == sw;
		};

		if(wincon_switches.find(arg) != wincon_switches.end() ||
			std::find_if(wincon_arg_switches.begin(), wincon_arg_switches.end(), switch_matches_arg) != wincon_arg_switches.end()) {
			native_console_implied = true;
		}
		
		if(arg == "--wnoconsole") {
			native_console_force = false;
		} else if(arg == "--wconsole") {
			native_console_force = true;
		} else if(arg == "--wnoredirect") {
			log_redirect = false;
		}
	}

	if(native_console_force.value_or(native_console_implied)) {
		lg::enable_native_console_output();
	}
	lg::early_log_file_setup(!log_redirect);
#endif

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
#endif

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

	try {
		if(!nobanner) {
			std::cerr << "Battle for Wesnoth v" << game_config::revision  << " " << game_config::build_arch() << '\n';
			const std::time_t t = std::time(nullptr);
			std::cerr << "Started on " << ctime(&t) << "\n";
		}

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
				if(!nobanner) std::cerr << "Automatically found a possible data directory at " << filesystem::sanitize_path(auto_dir) << '\n';
				game_config::path = auto_dir;
			}
		}

		const int res = do_gameloop(args);
		safe_exit(res);
	} catch(const boost::program_options::error& e) {
		std::cerr << "Error in command line: " << e.what() << '\n';
		error_exit(1);
	} catch(const CVideo::error& e) {
		std::cerr << "Could not initialize video.\n\n" << e.what() << "\n\nExiting.\n";
		error_exit(1);
	} catch(const font::error& e) {
		std::cerr << "Could not initialize fonts.\n\n" << e.what() << "\n\nExiting.\n";
		error_exit(1);
	} catch(const config::error& e) {
		std::cerr << e.message << "\n";
		error_exit(1);
	} catch(const gui::button::error&) {
		std::cerr << "Could not create button: Image could not be found\n";
		error_exit(1);
	} catch(const CVideo::quit&) {
		// just means the game should quit
	} catch(const return_to_play_side_exception&) {
		std::cerr << "caught return_to_play_side_exception, please report this bug (quitting)\n";
	} catch(const quit_game_exception&) {
		std::cerr << "caught quit_game_exception (quitting)\n";
	} catch(const wml_exception& e) {
		std::cerr << "WML exception:\nUser message: " << e.user_message << "\nDev message: " << e.dev_message << '\n';
		error_exit(1);
	} catch(const wfl::formula_error& e) {
		std::cerr << e.what() << "\n\nGame will be aborted.\n";
		error_exit(1);
	} catch(const sdl::exception& e) {
		std::cerr << e.what();
		error_exit(1);
	} catch(const game::error&) {
		// A message has already been displayed.
		error_exit(1);
	} catch(const std::bad_alloc&) {
		std::cerr << "Ran out of memory. Aborted.\n";
		error_exit(ENOMEM);
#if !defined(NO_CATCH_AT_GAME_END)
	} catch(const std::exception& e) {
		// Try to catch unexpected exceptions.
		std::cerr << "Caught general '" << typeid(e).name() << "' exception:\n" << e.what() << std::endl;
		error_exit(1);
	} catch(const std::string& e) {
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

/**
 * @page GUIToolkitWML GUIToolkitWML
 * @tableofcontents
 *
 * @section State State
 *
 * A state contains the info what to do in a state. At the moment this is rather focussed on the drawing part, might change later. Keys:
 * Key              |Type                                |Default  |Description
 * -----------------|------------------------------------|---------|-------------
 * draw             | @ref guivartype_section "section"  |mandatory|Section with drawing directions for a canvas.
 *
 * @section WindowDefinition Window Definition
 *
 * A window defines how a window looks in the game.
 * Key              |Type                                |Default  |Description
 * -----------------|------------------------------------|---------|-------------
 * id               | @ref guivartype_string "string"    |mandatory|Unique id for this window.
 * description      | @ref guivartype_t_string "t_string"|mandatory|Unique translatable name for this window.
 * resolution       | @ref guivartype_section "section"  |mandatory|The definitions of the window in various resolutions.
 *
 * @section Cell Cell
 *
 * Every grid cell has some cell configuration values and one widget in the grid cell.
 * Here we describe the what is available more information about the usage can be found at @ref GUILayout.
 *
 * Key                 |Type                                    |Default  |Description
 * --------------------|----------------------------------------|---------|-------------
 * id                  | @ref guivartype_string "string"        |""       |A grid is a widget and can have an id. This isn't used that often, but is allowed.
 * linked_group        | @ref guivartype_string "string"        |0        |.
 *
 * @section RowValues Row Values
 *
 * For every row the following variables are available:
 * Key                 |Type                                    |Default  |Description
 * --------------------|----------------------------------------|---------|-------------
 * grow_factor         | @ref guivartype_unsigned "unsigned"    |0        |The grow factor for a row.
 *
 * @section CellValues Cell Values
 *
 * For every column the following variables are available:
 * Key                 |Type                                    |Default  |Description
 * --------------------|----------------------------------------|---------|-------------
 * grow_factor         | @ref guivartype_unsigned "unsigned"    |0        |The grow factor for a column, this value is only read for the first row.
 * border_size         | @ref guivartype_unsigned "unsigned"    |0        |The border size for this grid cell.
 * border              | @ref guivartype_border "border"        |""       |Where to place the border in this grid cell.
 * vertical_alignment  | @ref guivartype_v_align "v_align"      |""       |The vertical alignment of the widget in the grid cell. (This value is ignored if vertical_grow is true.)
 * horizontal_alignment| @ref guivartype_h_align "h_align"      |""       |The horizontal alignment of the widget in the grid cell.(This value is ignored if horizontal_grow is true.)
 * vertical_grow       | @ref guivartype_bool "bool"            |false    |Does the widget grow in vertical direction when the grid cell grows in the vertical direction. This is used if the grid cell is wider as the best width for the widget.
 * horizontal_grow     | @ref guivartype_bool "bool"            |false    |Does the widget grow in horizontal direction when the grid cell grows in the horizontal direction. This is used if the grid cell is higher as the best width for the widget.
 */

/**
 * @page GUILayout GUILayout
 * @tableofcontents
 *
 * @section Abstract Abstract
 *
 * In the widget library the placement and sizes of elements is determined by
 * a grid. Therefore most widgets have no fixed size.
 *
 * @section Theory Theory
 *
 * We have two examples for the addon dialog, the first example the lower
 * buttons are in one grid, that means if the remove button gets wider
 * (due to translations) the connect button (4.1 - 2.2) will be aligned
 * to the left of the remove button. In the second example the connect
 * button will be partial underneath the remove button.
 *
 * A grid exists of x rows and y columns for all rows the number of columns
 * needs to be the same, there is no column (nor row) span. If spanning is
 * required place a nested grid to do so. In the examples every row has 1 column
 * but rows 3, 4 (and in the second 5) have a nested grid to add more elements
 * per row.
 *
 * In the grid every cell needs to have a widget, if no widget is wanted place
 * the special widget @a spacer. This is a non-visible item which normally
 * shouldn't have a size. It is possible to give a spacer a size as well but
 * that is discussed elsewhere.
 *
 * Every row and column has a @a grow_factor, since all columns in a grid are
 * aligned only the columns in the first row need to define their grow factor.
 * The grow factor is used to determine with the extra size available in a
 * dialog. The algorithm determines the extra size work like this:
 *
 * * determine the extra size
 * * determine the sum of the grow factors
 * * if this sum is 0 set the grow factor for every item to 1 and sum to sum of items.
 * * divide the extra size with the sum of grow factors
 * * for every item multiply the grow factor with the division value
 *
 * eg:
 * * extra size 100
 * * grow factors 1, 1, 2, 1
 * * sum 5
 * * division 100 / 5 = 20
 * * extra sizes 20, 20, 40, 20
 *
 * Since we force the factors to 1 if all zero it's not possible to have non
 * growing cells. This can be solved by adding an extra cell with a spacer and a
 * grow factor of 1. This is used for the buttons in the examples.
 *
 * Every cell has a @a border_size and @a border the @a border_size is the
 * number of pixels in the cell which aren't available for the widget. This is
 * used to make sure the items in different cells aren't put side to side. With
 * @a border it can be determined which sides get the border. So a border is
 * either 0 or @a border_size.
 *
 * If the widget doesn't grow when there's more space available the alignment
 * determines where in the cell the widget is placed.
 *
 * @subsection AbstractExample Abstract Example
 *
 *  	|---------------------------------------|
 *  	| 1.1                                   |
 *  	|---------------------------------------|
 *  	| 2.1                                   |
 *  	|---------------------------------------|
 *  	| |-----------------------------------| |
 *  	| | 3.1 - 1.1          | 3.1 - 1.2    | |
 *  	| |-----------------------------------| |
 *  	|---------------------------------------|
 *  	| |-----------------------------------| |
 *  	| | 4.1 - 1.1 | 4.1 - 1.2 | 4.1 - 1.3 | |
 *  	| |-----------------------------------| |
 *  	| | 4.1 - 2.1 | 4.1 - 2.2 | 4.1 - 2.3 | |
 *  	| |-----------------------------------| |
 *  	|---------------------------------------|
 *
 *
 *  	1.1       label : title
 *  	2.1       label : description
 *  	3.1 - 1.1 label : server
 *  	3.1 - 1.2 text box : server to connect to
 *  	4.1 - 1.1 spacer
 *  	4.1 - 1.2 spacer
 *  	4.1 - 1.3 button : remove addon
 *  	4.1 - 2.1 spacer
 *  	4.1 - 2.2 button : connect
 *  	4.1 - 2.3 button : cancel
 *
 *
 *  	|---------------------------------------|
 *  	| 1.1                                   |
 *  	|---------------------------------------|
 *  	| 2.1                                   |
 *  	|---------------------------------------|
 *  	| |-----------------------------------| |
 *  	| | 3.1 - 1.1          | 3.1 - 1.2    | |
 *  	| |-----------------------------------| |
 *  	|---------------------------------------|
 *  	| |-----------------------------------| |
 *  	| | 4.1 - 1.1         | 4.1 - 1.2     | |
 *  	| |-----------------------------------| |
 *  	|---------------------------------------|
 *  	| |-----------------------------------| |
 *  	| | 5.1 - 1.1 | 5.1 - 1.2 | 5.1 - 2.3 | |
 *  	| |-----------------------------------| |
 *  	|---------------------------------------|
 *
 *
 *  	1.1       label : title
 *  	2.1       label : description
 *  	3.1 - 1.1 label : server
 *  	3.1 - 1.2 text box : server to connect to
 *  	4.1 - 1.1 spacer
 *  	4.1 - 1.2 button : remove addon
 *  	5.1 - 1.1 spacer
 *  	5.1 - 1.2 button : connect
 *  	5.1 - 1.3 button : cancel
 *
 * @subsection ConcreteExample Concrete Example
 *
 * This is the code needed to create the skeleton for the structure the extra
 * flags are omitted.
 *
 *  	[grid]
 *  		[row]
 *  			[column]
 *  				[label]
 *  					# 1.1
 *  				[/label]
 *  			[/column]
 *  		[/row]
 *  		[row]
 *  			[column]
 *  				[label]
 *  					# 2.1
 *  				[/label]
 *  			[/column]
 *  		[/row]
 *  		[row]
 *  			[column]
 *  				[grid]
 *  					[row]
 *  						[column]
 *  							[label]
 *  								# 3.1 - 1.1
 *  							[/label]
 *  						[/column]
 *  						[column]
 *  							[text_box]
 *  								# 3.1 - 1.2
 *  							[/text_box]
 *  						[/column]
 *  					[/row]
 *  				[/grid]
 *  			[/column]
 *  		[/row]
 *  		[row]
 *  			[column]
 *  				[grid]
 *  					[row]
 *  						[column]
 *  							[spacer]
 *  								# 4.1 - 1.1
 *  							[/spacer]
 *  						[/column]
 *  						[column]
 *  							[spacer]
 *  								# 4.1 - 1.2
 *  							[/spacer]
 *  						[/column]
 *  						[column]
 *  							[button]
 *  								# 4.1 - 1.3
 *  							[/button]
 *  						[/column]
 *  					[/row]
 *  					[row]
 *  						[column]
 *  							[spacer]
 *  								# 4.1 - 2.1
 *  							[/spacer]
 *  						[/column]
 *  						[column]
 *  							[button]
 *  								# 4.1 - 2.2
 *  							[/button]
 *  						[/column]
 *  						[column]
 *  							[button]
 *  								# 4.1 - 2.3
 *  							[/button]
 *  						[/column]
 *  					[/row]
 *  				[/grid]
 *  			[/column]
 *  		[/row]
 *  	[/grid]
 */

/**
 * @defgroup GUIWidgetWML GUIWidgetWML
 * In various parts of the GUI there are several variables types in use. This section describes them.
 *
 * Below are the simple types which have one value or a short list of options:
 * Variable                                        |description
 * ------------------------------------------------|-----------
 * @anchor guivartype_unsigned unsigned            |Unsigned number (positive whole numbers and zero).
 * @anchor guivartype_f_unsigned f_unsigned        |Unsigned number or formula returning an unsigned number.
 * @anchor guivartype_int int                      |Signed number (whole numbers).
 * @anchor guivartype_f_int f_int                  |Signed number or formula returning an signed number.
 * @anchor guivartype_bool bool                    |A boolean value accepts the normal values as the rest of the game.
 * @anchor guivartype_f_bool f_bool                |Boolean value or a formula returning a boolean value.
 * @anchor guivartype_string string                |A text.
 * @anchor guivartype_t_string t_string            |A translatable string.
 * @anchor guivartype_f_tstring f_tstring          |Formula returning a translatable string.
 * @anchor guivartype_function function            |A string containing a set of function definition for the formula language.
 * @anchor guivartype_color color                  |A string which contains the color, this a group of 4 numbers between 0 and 255 separated by a comma. The numbers are red component, green component, blue component and alpha. A color of 0 is not available. An alpha of 255 is fully transparent. Omitted values are set to 0.
 * @anchor guivartype_font_style font_style        |A string which contains the style of the font:<ul><li>normal</li><li>bold</li><li>italic</li><li>underlined</li></ul>Since SDL has problems combining these styles only one can be picked. Once SDL will allow multiple options, this type will be transformed to a comma separated list. If empty we default to the normal style. Since the render engine is replaced by Pango markup this field will change later on. Note widgets that allow marked up text can use markup to change the font style.
 * @anchor guivartype_v_align v_align              |Vertical alignment; how an item is aligned vertically in the available space. Possible values:<ul><li>top</li><li>bottom</li><li>center</li></ul>When nothing is set or an another value as in the list the item is centered.
 * @anchor guivartype_h_align h_align              |Horizontal alignment; how an item is aligned horizontal in the available space. Possible values:<ul><li>left</li><li>right</li><li>center</li></ul>
 * @anchor guivartype_f_h_align f_h_align          |A horizontal alignment or a formula returning a horizontal alignment.
 * @anchor guivartype_border border                |Comma separated list of borders to use. Possible values:<ul><li>left</li><li>right</li><li>top</li><li>bottom</li><li>all alias for "left, right, top, bottom"</li></ul>
 * @anchor guivartype_scrollbar_mode scrollbar_mode|How to show the scrollbar of a widget. Possible values:<ul><li>always - The scrollbar is always shown, regardless whether it's required or not.</li><li>never - The scrollbar is never shown, even not when needed. (Note when setting this mode dialogs might not properly fit anymore).</li><li>auto - Shows the scrollbar when needed. The widget will reserve space for the scrollbar, but only show when needed.</li><li>initial_auto - Like auto, but when the scrollbar is not needed the space is not reserved.</li></ul>Use auto when the list can be changed dynamically eg the game list in the lobby. For optimization you can also use auto when you really expect a scrollbar, but don't want it to be shown when not needed eg the language list will need a scrollbar on most screens.
 * @anchor guivartype_resize_mode resize_mode      |Determines how an image is resized. Possible values:<ul><li>scale - The image is scaled.</li><li>stretch - The first row or column of pixels is copied over the entire image. (Can only be used to scale resize in one direction, else falls back to scale.)</li><li>tile - The image is placed several times until the entire surface is filled. The last images are truncated.</li></ul>
 * @anchor guivartype_grow_direction grow_direction|Determines how an image is resized. Possible values:<ul><li>scale - The image is scaled.</li><li>stretch - The first row or column of pixels is copied over the entire image. (Can only be used to scale resize in one direction, else falls back to scale.)</li><li>tile - The image is placed several times until the entire surface is filled. The last images are truncated.</li></ul>
 *
 * For more complex parts, there are sections. Sections contain of several lines of WML and can have sub sections. For example a grid has sub sections which contain various widgets. Here's the list of sections:
 * Variable                                        |description
 * ------------------------------------------------|-----------
 * @anchor guivartype_section section              |A generic section. The documentation about the section should describe the section in further detail.
 * @anchor guivartype_grid grid                    |A grid contains several widgets.
 * @anchor guivartype_config config                |.
 *
 * Every widget has some parts in common. First of all, every definition has the following fields:
 * Key          |Type                                |Default  |Description
 * -------------|------------------------------------|---------|-----------
 * id           | @ref guivartype_string "string"    |mandatory|Unique id for this gui (theme).
 * description  | @ref guivartype_t_string "t_string"|mandatory|Unique translatable name for this gui.
 * resolution   | @ref guivartype_section "section"  |mandatory|The definitions of the widget in various resolutions.
 * Inside a grid (which is inside all container widgets) a widget is instantiated. With this instantiation some more variables of a widget can be tuned.
 */

/**
 * @defgroup GUICanvasWML GUICanvasWML
 *
 * A canvas is a blank drawing area on which the user can draw several shapes.
 * The drawing is done by adding WML structures to the canvas.
 *
 * @section PreCommit Pre-commit
 *
 * This section contains the pre commit functions.
 * These functions will be executed before the drawn canvas is applied on top of the normal background.
 * There should only be one pre commit section and its order regarding the other shapes doesn't matter.
 * The function has effect on the entire canvas, it's not possible to affect only a small part of the canvas.
 *
 * @subsection Blur Blur
 *
 * Blurs the background before applying the canvas. This doesn't make sense if the widget isn't semi-transparent.
 *
 * Keys:
 * Key          |Type                                |Default  |Description
 * -------------|------------------------------------|---------|-----------
 * depth        | @ref guivartype_unsigned "unsigned"|0        |The depth to blur.
 */

/**
 * @defgroup GUIWindowDefinitionWML GUIWindowDefinitionWML
 *
 * The window definition define how the windows shown in the dialog look.
 */