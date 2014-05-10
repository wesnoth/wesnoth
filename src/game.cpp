/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "about.hpp"
#include "addon/manager.hpp"
#include "addon/manager_ui.hpp"
#include "commandline_options.hpp"
#include "game_config_manager.hpp"
#include "game_controller.hpp"
#include "gui/dialogs/core_selection.hpp"
#include "gui/dialogs/title_screen.hpp"
#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
#include "gui/widgets/debug.hpp"
#endif
#include "gui/widgets/window.hpp"
#include "help.hpp"
#include "loadscreen.hpp"
#include "log.hpp"
#include "playcampaign.hpp"
#include "preferences_display.hpp"
#include "replay.hpp"
#include "sdl/exception.hpp"
#include "serialization/binary_or_text.hpp"
#include "serialization/parser.hpp"
#include "serialization/validator.hpp"
#include "statistics.hpp"
#include "version.hpp"
#include "wml_exception.hpp"

#include <cerrno>
#include <clocale>
#include <fstream>
#include <libintl.h>

#include <boost/foreach.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include <SDL.h>
#include <SDL_thread.h>


#ifdef HAVE_VISUAL_LEAK_DETECTOR
#include "vld.h"
#endif

static lg::log_domain log_config("config");
#define LOG_CONFIG LOG_STREAM(info, log_config)

#define LOG_GENERAL LOG_STREAM(info, lg::general)

static lg::log_domain log_preprocessor("preprocessor");
#define LOG_PREPROC LOG_STREAM(info,log_preprocessor)

// this is needed to allow identical functionality with clean refactoring
// play_game only returns on an error, all returns within play_game can
// be replaced with this
static void safe_exit(int res) {

	LOG_GENERAL << "exiting with code " << res << "\n";
	exit(res);
}

// maybe this should go in a util file somewhere?
template <typename filter>
static void encode(const std::string & input_file, const std::string & output_file)
{
	try {
		std::ifstream ifile(input_file.c_str(),
				std::ios_base::in | std::ios_base::binary);
		ifile.peek(); // We need to touch the stream to set the eof bit
		if(!ifile.good()) {
			std::cerr << "Input file " << input_file << " is not good for reading. Exiting to prevent bzip2 from segfaulting\n";
			safe_exit(1);
		}
		std::ofstream ofile(output_file.c_str(), std::ios_base::out
				| std::ios_base::binary);
		boost::iostreams::filtering_stream<boost::iostreams::output> stream;
		stream.push(filter());
		stream.push(ofile);
		boost::iostreams::copy(ifile, stream);
		ifile.close();
		safe_exit(remove(input_file.c_str()));
	}  catch(io_exception& e) {
		std::cerr << "IO error: " << e.what() << "\n";
	}
}

template <typename filter>
static void decode(const std::string & input_file, const std::string & output_file)
{
	try {
		std::ofstream ofile(output_file.c_str(), std::ios_base::out
				| std::ios_base::binary);
		std::ifstream ifile(input_file.c_str(),
				std::ios_base::in | std::ios_base::binary);
		boost::iostreams::filtering_stream<boost::iostreams::input> stream;
		stream.push(filter());
		stream.push(ifile);
		boost::iostreams::copy(stream, ofile);
		ifile.close();
		safe_exit(remove(input_file.c_str()));
	}  catch(io_exception& e) {
		std::cerr << "IO error: " << e.what() << "\n";
	}
}

static void gzip_encode(const std::string & input_file, const std::string & output_file)
{
	encode<boost::iostreams::gzip_compressor>(input_file, output_file);
}

static void gzip_decode(const std::string & input_file, const std::string & output_file)
{
	decode<boost::iostreams::gzip_decompressor>(input_file, output_file);
}

static void bzip2_encode(const std::string & input_file, const std::string & output_file)
{
	encode<boost::iostreams::bzip2_compressor>(input_file, output_file);
}

static void bzip2_decode(const std::string & input_file, const std::string & output_file)
{
	decode<boost::iostreams::bzip2_decompressor>(input_file, output_file);
}

static void handle_preprocess_command(const commandline_options& cmdline_opts)
{
	preproc_map input_macros;

	if( cmdline_opts.preprocess_input_macros ) {
		std::string file = *cmdline_opts.preprocess_input_macros;
		if ( file_exists( file ) == false )
		{
			std::cerr << "please specify an existing file. File "<< file <<" doesn't exist.\n";
			return;
		}

		std::cerr << SDL_GetTicks() << " Reading cached defines from: " << file << "\n";

		config cfg;
		scoped_istream stream = istream_file( file );
		read( cfg, *stream );

		int read = 0;

		// use static preproc_define::read_pair(config) to make a object
		BOOST_FOREACH( const config::any_child &value, cfg.all_children_range() ) {
			const preproc_map::value_type def = preproc_define::read_pair( value.cfg );
			input_macros[def.first] = def.second;
			++read;
		}
		std::cerr << SDL_GetTicks() << " Read " << read << " defines.\n";
	}

	const std::string resourceToProcess(*cmdline_opts.preprocess_path);
	const std::string targetDir(*cmdline_opts.preprocess_target);

	Uint32 startTime = SDL_GetTicks();
	// if the users add the SKIP_CORE define we won't preprocess data/core
	bool skipCore = false;
	bool skipTerrainGFX = false;
	// the 'core_defines_map' is the one got from data/core macros
	preproc_map defines_map( input_macros );

	if ( cmdline_opts.preprocess_defines ) {

		// add the specified defines
		BOOST_FOREACH( const std::string &define, *cmdline_opts.preprocess_defines ) {
			if (define.empty()){
				std::cerr << "empty define supplied\n";
				continue;
			}

			LOG_PREPROC << "adding define: " << define << '\n';
			defines_map.insert(std::make_pair(define, preproc_define(define)));

			if (define == "SKIP_CORE")
			{
				std::cerr << "'SKIP_CORE' defined.\n";
				skipCore = true;
			}
			else if (define == "NO_TERRAIN_GFX")
			{
				std::cerr << "'NO_TERRAIN_GFX' defined.\n";
				skipTerrainGFX = true;
			}
		}
	}

	// add the WESNOTH_VERSION define
	defines_map["WESNOTH_VERSION"] = preproc_define(game_config::wesnoth_version.str());

	std::cerr << "added " << defines_map.size() << " defines.\n";

	// preprocess core macros first if we don't skip the core
	if (skipCore == false) {
		std::cerr << "preprocessing common macros from 'data/core' ...\n";

		// process each folder explicitly to gain speed
		preprocess_resource(game_config::path + "/data/core/macros",&defines_map);
		if (skipTerrainGFX == false)
			preprocess_resource(game_config::path + "/data/core/terrain-graphics",&defines_map);

		std::cerr << "acquired " << (defines_map.size() - input_macros.size())
			<< " 'data/core' defines.\n";
	}
	else
		std::cerr << "skipped 'data/core'\n";

	// preprocess resource
	std::cerr << "preprocessing specified resource: "
		<< resourceToProcess << " ...\n";
	preprocess_resource(resourceToProcess, &defines_map, true,true, targetDir);
	std::cerr << "acquired " << (defines_map.size() - input_macros.size())
		<< " total defines.\n";

	if ( cmdline_opts.preprocess_output_macros )
	{
		std::string outputFileName = "_MACROS_.cfg";
		if (!cmdline_opts.preprocess_output_macros->empty()) {
			outputFileName = *cmdline_opts.preprocess_output_macros;
		}

		std::string outputPath = targetDir + "/" + outputFileName;

		std::cerr << "writing '" << outputPath << "' with "
			<< defines_map.size() << " defines.\n";

		scoped_ostream out = ostream_file(outputPath);
		if (!out->fail())
		{
			config_writer writer(*out,false);

			for(preproc_map::iterator itor = defines_map.begin();
				itor != defines_map.end(); ++itor)
			{
				(*itor).second.write(writer, (*itor).first);
			}
		}
		else
			std::cerr << "couldn't open the file.\n";
	}

	std::cerr << "preprocessing finished. Took "<< SDL_GetTicks() - startTime << " ticks.\n";
}

/** Process commandline-arguments */
static int process_command_args(const commandline_options& cmdline_opts) {

	// Options that don't change behavior based on any others should be checked alphabetically below.

	if(cmdline_opts.userconfig_dir) {
		set_user_config_dir(*cmdline_opts.userconfig_dir);
	}
	if(cmdline_opts.userconfig_path) {
		std::cout << get_user_config_dir() << '\n';
		return 0;
	}
	if(cmdline_opts.userdata_dir) {
		set_user_data_dir(*cmdline_opts.userdata_dir);
	}
	if(cmdline_opts.userdata_path) {
		std::cout << get_user_data_dir() << '\n';
		return 0;
	}
	if(cmdline_opts.data_dir) {
		const std::string datadir = *cmdline_opts.data_dir;
		std::cerr << "Overriding data directory with " << datadir << std::endl;
#ifdef _WIN32
		// use c_str to ensure that index 1 points to valid element since c_str() returns null-terminated string
		if(datadir.c_str()[1] == ':') {
#else
		if(datadir[0] == '/') {
#endif
			game_config::path = datadir;
		} else {
			game_config::path = get_cwd() + '/' + datadir;
		}

		if(!is_directory(game_config::path)) {
			std::cerr << "Could not find directory '" << game_config::path << "'\n";
			throw config::error("directory not found");
		}
	// don't update font as we already updating it in game ctor
	//font_manager_.update_font_path();
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
		if(!is_gzip_file(input_file)) {
			std::cerr << "file '" << input_file << "'isn't a .gz file\n";
			return 2;
		}
		const std::string output_file(
			input_file, 0, input_file.length() - 3);
		gzip_decode(input_file, output_file);
	}
	if(cmdline_opts.bunzip2) {
		const std::string input_file(*cmdline_opts.bunzip2);
		if(!is_bzip2_file(input_file)) {
			std::cerr << "file '" << input_file << "'isn't a .bz2 file\n";
			return 2;
		}
		const std::string output_file(
			input_file, 0, input_file.length() - 4);
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
		for(std::vector<boost::tuple<int, std::string> >::const_iterator it=cmdline_opts.log->begin(); it!=cmdline_opts.log->end(); ++it)
		{
			const std::string log_domain = it->get<1>();
			const int severity = it->get<0>();
			if (!lg::set_log_domain_severity(log_domain, severity))
			{
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
		std::cout <<  game_config::path << "\n";
		return 0;
	}
	if(cmdline_opts.log_precise_timestamps) {
		lg::precise_timestamps(true);
	}
	if(cmdline_opts.rng_seed) {
		srand(*cmdline_opts.rng_seed);
	}
	if(cmdline_opts.screenshot) {
#if SDL_VERSION_ATLEAST(2, 0, 0)
		SDL_setenv("SDL_VIDEODRIVER", "dummy", 1);
#else
		static char opt[] = "SDL_VIDEODRIVER=dummy";
		SDL_putenv(opt);
#endif
	}
	if(cmdline_opts.strict_validation) {
		strict_validation_enabled = true;
	}
	if(cmdline_opts.version) {
		std::cout << "Battle for Wesnoth" << " " << game_config::version << "\n";
		return 0;
	}

	// Options changing their behavior dependent on some others should be checked below.

	if ( cmdline_opts.preprocess ) {
		handle_preprocess_command(cmdline_opts);
		return 0;
	}

	// Not the most intuitive solution, but I wanted to leave current semantics for now
	return -1;
}

/**
 * I would prefer to setup locale first so that early error
 * messages can get localized, but we need the game_controller
 * initialized to have get_intl_dir() to work.  Note: setlocale()
 * does not take GUI language setting into account.
 */
static void init_locale() {
	#if defined _WIN32 || defined __APPLE__
	    setlocale(LC_ALL, "English");
	#else
		std::setlocale(LC_ALL, "C");
		std::setlocale(LC_MESSAGES, "");
	#endif
	const std::string& intl_dir = get_intl_dir();
	bindtextdomain (PACKAGE, intl_dir.c_str());
	bind_textdomain_codeset (PACKAGE, "UTF-8");
	bindtextdomain (PACKAGE "-lib", intl_dir.c_str());
	bind_textdomain_codeset (PACKAGE "-lib", "UTF-8");
	textdomain (PACKAGE);
}

/**
 * SDL Semaphore which the main thread waits on, while the worker does it's job.
 * We mainly only need this so we can use the SDL_SemWaitTimeout function to do the timer stuff for us.
 */
static SDL_sem * worker_sem;

/**
 * Structure used to pass data to the worker thread. The int* is used to pass the worker result back out.
 * This is not, strictly speaking, good practice with threads, but it is fine for a single thread with
 * a timeout, that only sets the variable once.
 * 
 * If for some reason you want to run many worker threads at once then don't use this code. 
 */
typedef std::pair<boost::shared_ptr<game_controller>, int*> thread_data;

/**
 * Function used by worker thread to perform unit test with timeout.
 */
static int run_unit_test (void * data){
	thread_data * mydata = static_cast<thread_data *>(data);
	if (SDL_SemWait(worker_sem) == -1) {
		std::cerr << "Worker failed to lock worker semaphore!" << std::endl;
	}
	int ret_val = mydata->first->unit_test();
	*mydata->second = ret_val;

	if (SDL_SemPost(worker_sem) == -1) {
		std::cerr << "Worker failed to unlock worker semaphore after working!" << std::endl;
	}
	return ret_val;
}

/**
 * Setups the game environment and enters
 * the titlescreen or game loops.
 */
static int do_gameloop(int argc, char** argv)
{
	srand(time(NULL));

	commandline_options cmdline_opts = commandline_options(argc,argv);
	game_config::wesnoth_program_dir = directory_name(argv[0]);
	int finished = process_command_args(cmdline_opts);
	if(finished != -1) {
		return finished;
	}

	boost::shared_ptr<game_controller> game(
		new game_controller(cmdline_opts,argv[0]));
	const int start_ticks = SDL_GetTicks();

	init_locale();

	bool res;

	// do initialize fonts before reading the game config, to have game
	// config error messages displayed. fonts will be re-initialized later
	// when the language is read from the game config.
	res = font::load_font_config();
	if(res == false) {
		std::cerr << "could not initialize fonts\n";
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

	if(preferences::joystick_support_enabled()) {
		res = game->init_joystick();
		if(res == false) {
			std::cerr << "could not initialize joystick\n";
		}
	}

	const cursor::manager cursor_manager;
	cursor::set(cursor::WAIT);

#if (defined(_X11) && !defined(__APPLE__)) || defined(_WIN32)
	SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
#endif

	loadscreen::global_loadscreen_manager loadscreen_manager(game->disp().video());

	loadscreen::start_stage("init gui");
	gui2::init();
	const gui2::event::tmanager gui_event_manager;

	game_config_manager config_manager(cmdline_opts, game->disp(),
	    game->jump_to_editor());

	loadscreen::start_stage("load config");
	res = config_manager.init_game_config(game_config_manager::NO_FORCE_RELOAD);
	if(res == false) {
		std::cerr << "could not initialize game config\n";
		return 1;
	}
	loadscreen::start_stage("init fonts");

	res = font::load_font_config();
	if(res == false) {
		std::cerr << "could not re-initialize fonts for the current language\n";
		return 1;
	}

	loadscreen::start_stage("refresh addons");
	refresh_addon_version_info_cache();

	config tips_of_day;

	loadscreen::start_stage("titlescreen");

	LOG_CONFIG << "time elapsed: "<<  (SDL_GetTicks() - start_ticks) << " ms\n";

	for (;;)
	{
		// reset the TC, since a game can modify it, and it may be used
		// by images in add-ons or campaigns dialogs
		image::set_team_colors();

		statistics::fresh_stats();

		if (!game->is_loading()) {
			const config &cfg =
			    config_manager.game_config().child("titlescreen_music");
			if (cfg) {
				sound::play_music_repeatedly(game_config::title_music);
				BOOST_FOREACH(const config &i, cfg.child_range("music")) {
					sound::play_music_config(i);
				}
				sound::commit_music_changes();
			} else {
				sound::empty_playlist();
				sound::stop_music();
			}
		}

		loadscreen_manager.reset();

		if(cmdline_opts.unit_test) {
#if SDL_VERSION_ATLEAST(2,0,0) && !SDL_VERSION_ATLEAST(2,0,2)
			if(cmdline_opts.timeout) {
				std::cerr << "SDL Version number: " << SDL_COMPILEDVERSION << std::endl;
				std::cerr << "Your SDL version is between 2.0.0 and 2.0.2..." << std::endl
					<< " I don't know how to handle timedout threads. Disabling timeout." << std::endl;
				*cmdline_opts.timeout = 0;
			}
#endif
			if(cmdline_opts.timeout && *cmdline_opts.timeout > 0) {
				int worker_result = 2; //Default timeout return value if worker fails to return
				worker_sem = SDL_CreateSemaphore(1);
				if (worker_sem == NULL) {
					std::cerr << "Failed to create a semaphore for timeout worker thread!" << std::endl;
					std::cerr << "FAIL TEST (TIMEOUT): " << *cmdline_opts.unit_test << std::endl;
					return 2;
				}

				thread_data data(game, &worker_result);
#if SDL_VERSION_ATLEAST(2,0,0)
				SDL_Thread *worker = SDL_CreateThread(&run_unit_test, "worker", &data);
#else
				SDL_Thread *worker = SDL_CreateThread(&run_unit_test, &data);
#endif

				std::cerr << "Setting timer for " << *cmdline_opts.timeout << " ms." << std::endl;
				int wait_result = SDL_SemWaitTimeout(worker_sem, *cmdline_opts.timeout);
				if (wait_result == 0) {
					SDL_SemPost(worker_sem);
					SDL_DestroySemaphore(worker_sem);
					std::cerr << ((worker_result == 0) ? "PASS TEST " : "FAIL TEST ")
						<< ((worker_result == 3) ? "(INVALID REPLAY)" : "")
						<< ((worker_result == 4) ? "(ERRORED REPLAY)" : "")
						<< ": "<<*cmdline_opts.unit_test << std::endl;
					return worker_result;
				} else {
#if SDL_VERSION_ATLEAST(2,0,2) 
					SDL_DetachThread(worker);
#else
#if !SDL_VERSION_ATLEAST(2,0,0)
					SDL_KillThread(worker);
#endif
#endif
					if (wait_result == -1) {
						std::cerr << "Error in SemWaitTimeout!" << std::endl;
					} else {
						std::cerr << "Test timed out!" << std::endl;
					}
					std::cerr << ("FAIL TEST (TIMEOUT): ") << *cmdline_opts.unit_test << std::endl;
					return 2;
				}
			} else {
				int worker_result = game->unit_test();
				std::cerr << ((worker_result == 0) ? "PASS TEST " : "FAIL TEST ")
					<< ((worker_result == 3) ? "(INVALID REPLAY)" : "")
					<< ((worker_result == 4) ? "(ERRORED REPLAY)" : "")
					<< ": "<<*cmdline_opts.unit_test << std::endl;
				return worker_result;
			}
		}

		if(game->play_test() == false) {
			return 0;
		}

		if(game->play_screenshot_mode() == false) {
			return 0;
		}

		recorder.clear();

		//Start directly a campaign
		if(game->goto_campaign() == false){
			if (game->jump_to_campaign_id().empty())
				continue; //Go to main menu
			else
				return 1; //we got an error starting the campaign from command line
		}

		//Start directly a multiplayer
		//Eventually with a specified server
		if(game->goto_multiplayer() == false){
			continue; //Go to main menu
		}

		//Start directly a commandline multiplayer game
		if(game->play_multiplayer_commandline() == false) {
			return 0;
		}

		if (game->goto_editor() == false) {
			return 0;
		}

		gui2::ttitle_screen::tresult res = game->is_loading()
				? gui2::ttitle_screen::LOAD_GAME
				: gui2::ttitle_screen::NOTHING;

		const preferences::display_manager disp_manager(&game->disp());

		const font::floating_label_context label_manager;

		cursor::set(cursor::NORMAL);
		if(res == gui2::ttitle_screen::NOTHING) {
			const hotkey::basic_handler key_handler(&game->disp());
			gui2::ttitle_screen dlg;
			dlg.show(game->disp().video());

			res = static_cast<gui2::ttitle_screen::tresult>(dlg.get_retval());
		}

		game_controller::RELOAD_GAME_DATA should_reload =
			game_controller::RELOAD_DATA;

		if(res == gui2::ttitle_screen::QUIT_GAME) {
			LOG_GENERAL << "quitting game...\n";
			return 0;
		} else if(res == gui2::ttitle_screen::LOAD_GAME) {
			if(game->load_game() == false) {
				game->clear_loaded_game();
				res = gui2::ttitle_screen::NOTHING;
				continue;
			}
			should_reload = game_controller::NO_RELOAD_DATA;
		} else if(res == gui2::ttitle_screen::TUTORIAL) {
			game->set_tutorial();
		} else if(res == gui2::ttitle_screen::NEW_CAMPAIGN) {
			if(game->new_campaign() == false) {
				continue;
			}
		} else if(res == gui2::ttitle_screen::MULTIPLAYER) {
			game_config::debug = game_config::mp_debug;
			if(game->play_multiplayer() == false) {
				continue;
			}
		} else if(res == gui2::ttitle_screen::CHANGE_LANGUAGE) {
			if (game->change_language()) {
				tips_of_day.clear();
				t_string::reset_translations();
				image::flush_cache();
			}
			continue;
		} else if(res == gui2::ttitle_screen::EDIT_PREFERENCES) {
			game->show_preferences();
			continue;
		} else if(res == gui2::ttitle_screen::SHOW_ABOUT) {
			about::show_about(game->disp());
			continue;
		} else if(res == gui2::ttitle_screen::SHOW_HELP) {
			help::help_manager help_manager(&config_manager.game_config());
			help::show_help(game->disp());
			continue;
		} else if(res == gui2::ttitle_screen::GET_ADDONS) {
			// NOTE: we need the help_manager to get access to the Add-ons
			// section in the game help!
			help::help_manager help_manager(&config_manager.game_config());
			if(manage_addons(game->disp())) {
				config_manager.reload_changed_game_config();
			}
			continue;
		} else if(res == gui2::ttitle_screen::CORES) {

			int current = 0;
			std::vector<config> cores;
			BOOST_FOREACH(const config& core,
					resources::config_manager->game_config().child_range("core")) {
				cores.push_back(core);
				if (core["id"] == preferences::core_id())
					current = cores.size() -1;
			}

			gui2::tcore_selection core_dlg(cores, current);
			if (core_dlg.show(game->disp().video())) {
				int core_index = core_dlg.get_choice();
				const std::string& wml_tree_root = cores[core_index]["path"];
				const std::string& core_id = cores[core_index]["id"];
				preferences::set_wml_tree_root(wml_tree_root);
				preferences::set_core_id(core_id);
				config_manager.reload_changed_game_config();
			}
			continue;
		} else if(res == gui2::ttitle_screen::RELOAD_GAME_DATA) {
			loadscreen::global_loadscreen_manager loadscreen(game->disp().video());
			config_manager.reload_changed_game_config();
			image::flush_cache();
			continue;
		} else if(res == gui2::ttitle_screen::START_MAP_EDITOR) {
			///@todo editor can ask the game to quit completely
			if (game->start_editor() == editor::EXIT_QUIT_TO_DESKTOP) {
				return 0;
			}
			continue;
		}

		if (recorder.at_end()){
			game->launch_game(should_reload);
		}
		else{
			game->play_replay();
		}
	}
}


#ifdef __native_client__
int wesnoth_main(int argc, char** argv)
#else
int main(int argc, char** argv)
#endif
{

#ifdef HAVE_VISUAL_LEAK_DETECTOR
	VLDEnable();
#endif

#if defined(_OPENMP) && !defined(_WIN32) && !defined(__APPLE__)
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
	if (!getenv("OMP_WAIT_POLICY")) {
		setenv("OMP_WAIT_POLICY", "PASSIVE", 1);
		execv(argv[0], argv);
	}
#endif

	if(SDL_Init(SDL_INIT_TIMER) < 0) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		return(1);
	}

	try {
		std::cerr << "Battle for Wesnoth v" << game_config::revision << '\n';
		const time_t t = time(NULL);
		std::cerr << "Started on " << ctime(&t) << "\n";

		const std::string exe_dir = get_exe_dir();
		if(!exe_dir.empty() && file_exists(exe_dir + "/data/_main.cfg")) {
			std::cerr << "Automatically found a possible data directory at "
			          << exe_dir << '\n';
			game_config::path = exe_dir;
		}

		const int res = do_gameloop(argc,argv);
		safe_exit(res);
	} catch(boost::program_options::error& e) {
		std::cerr << "Error in command line: " << e.what() << '\n';
		return 1;
	} catch(CVideo::error&) {
		std::cerr << "Could not initialize video. Exiting.\n";
		return 1;
	} catch(font::manager::error&) {
		std::cerr << "Could not initialize fonts. Exiting.\n";
		return 1;
	} catch(config::error& e) {
		std::cerr << e.message << "\n";
		return 1;
	} catch(gui::button::error&) {
		std::cerr << "Could not create button: Image could not be found\n";
		return 1;
	} catch(CVideo::quit&) {
		//just means the game should quit
	} catch(end_level_exception&) {
		std::cerr << "caught end_level_exception (quitting)\n";
	} catch(twml_exception& e) {
		std::cerr << "WML exception:\nUser message: "
			<< e.user_message << "\nDev message: " << e.dev_message << '\n';
		return 1;
	} catch(game_logic::formula_error& e) {
		std::cerr << e.what()
			<< "\n\nGame will be aborted.\n";
		return 1;
	} catch(const sdl::texception& e) {
		std::cerr << e.what();
		return 1;
	} catch(game::error &) {
		// A message has already been displayed.
		return 1;
	} catch(std::ios::failure &) {
		// This is required mainly for when compiling with Microsoft Visual C++ Compiler
		// so that the game will not crash with a Runtime Error
		return 1;
	} catch(std::bad_alloc&) {
		std::cerr << "Ran out of memory. Aborted.\n";
		return ENOMEM;
	}

	return 0;
} // end main
