/*
	Copyright (C) 2011 - 2024
	by Lukasz Dobrogowski <lukasz.dobrogowski@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "commandline_options.hpp"

#include "config.hpp"
#include "formatter.hpp"
#include "lexical_cast.hpp"
#include "serialization/string_utils.hpp"  // for split

#include <boost/any.hpp>                // for any
#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/errors.hpp>  // for validation_error, etc
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/value_semantic.hpp>  // for value, etc
#include <boost/program_options/variables_map.hpp>  // for variables_map, etc

#include <array>

namespace po = boost::program_options;

class two_strings : public std::pair<std::string,std::string> {};

static void validate(boost::any& v, const std::vector<std::string>& values,
              two_strings*, int)
{
	two_strings ret_val;
	if(values.size() != 2) {
		throw po::validation_error(po::validation_error::invalid_option_value);
	}
	ret_val.first = values[0];
	ret_val.second = values[1];
	v = ret_val;
}

bad_commandline_resolution::bad_commandline_resolution(const std::string& resolution)
	: error(formatter() << "Invalid resolution \"" << resolution
						 << "\" (WIDTHxHEIGHT expected)")
{
}

bad_commandline_tuple::bad_commandline_tuple(const std::string& str,
											 const std::string& expected_format)
	: error(formatter() << "Invalid value set \"" << str
						 << "\" (" << expected_format << " expected)")
{
}


#define IMPLY_TERMINAL " Implies --no-log-to-file"


commandline_options::commandline_options(const std::vector<std::string>& args)
	: campaign()
	, campaign_difficulty()
	, campaign_scenario()
	, campaign_skip_story(false)
	, clock(false)
	, core_id()
	, data_path(false)
	, data_dir()
	, debug(false)
	, debug_lua(false)
	, strict_lua(false)
	, allow_insecure(false)
#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
	, debug_dot_domain()
	, debug_dot_level()
#endif
	, editor()
	, fps(false)
	, fullscreen(false)
	, help()
	, language()
	, log()
	, load()
	, logdomains()
	, log_precise_timestamps(false)
	, multiplayer(false)
	, multiplayer_ai_config()
	, multiplayer_algorithm()
	, multiplayer_controller()
	, multiplayer_era()
	, multiplayer_exit_at_end()
	, multiplayer_ignore_map_settings()
	, multiplayer_label()
	, multiplayer_parm()
	, multiplayer_repeat()
	, multiplayer_scenario()
	, multiplayer_side()
	, multiplayer_turns()
	, max_fps()
	, noaddons(false)
	, nocache(false)
	, nodelay(false)
	, nogui(false)
	, nobanner(false)
	, nomusic(false)
	, nosound(false)
	, new_widgets(false)
	, preprocess(false)
	, preprocess_defines()
	, preprocess_input_macros()
	, preprocess_output_macros()
	, preprocess_path()
	, preprocess_target()
	, resolution()
	, rng_seed()
	, server()
	, username()
	, password()
	, render_image()
	, render_image_dst()
	, generate_spritesheet()
	, screenshot(false)
	, screenshot_map_file()
	, screenshot_output_file()
	, script_file()
	, plugin_file()
	, script_unsafe_mode(false)
	, strict_validation(false)
	, test()
	, unit_test()
	, headless_unit_test(false)
	, noreplaycheck(false)
	, mptest(false)
	, usercache_path(false)
	, usercache_dir()
	, userdata_path(false)
	, userdata_dir()
	, validcache(false)
	, validate_core(false)
	, validate_addon()
	, validate_schema()
	, validate_wml()
	, validate_with()
	, do_diff()
	, do_patch()
	, diff_left()
	, diff_right()
	, version(false)
	, simple_version(false)
	, report(false)
	, windowed(false)
	, with_replay(false)
#ifdef _WIN32
	, no_console(false)
#endif
	, no_log_sanitize(false)
	, log_to_file(false)
	, no_log_to_file(false)
	, translation_percent()
	, args_(args.begin() + 1, args.end())
	, args0_(*args.begin())
	, all_()
	, visible_()
	, hidden_()
{
	// When adding items don't forget to update doc/man/wesnoth.6
	// Options are sorted alphabetically by --long-option.
	po::options_description general_opts("General options");
	general_opts.add_options()
		("all-translations", "Show all translations, even incomplete ones.")
		("clock", "Adds the option to show a clock for testing the drawing timer.")
		("core", po::value<std::string>(), "overrides the loaded core with the one whose id is specified.")
		("data-dir", po::value<std::string>(), "overrides the data directory with the one specified.")
		("data-path", "prints the path of the data directory and exits." IMPLY_TERMINAL)
		("debug,d", "enables additional command mode options in-game.")
		("debug-lua", "enables some Lua debugging mechanisms")
		("strict-lua", "disallow deprecated Lua API calls")
		("allow-insecure", "Allows sending a plaintext password over an unencrypted connection. Should only ever be used for local testing.")
#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
		("debug-dot-level", po::value<std::string>(), "sets the level of the debug dot files. <arg> should be a comma separated list of levels. These files are used for debugging the widgets especially the for the layout engine. When enabled the engine will produce dot files which can be converted to images with the dot tool. Available levels: size (generate the size info of the widget), state (generate the state info of the widget).")
		("debug-dot-domain", po::value<std::string>(), "sets the domain of the debug dot files. <arg> should be a comma separated list of domains. See --debug-dot-level for more info. Available domains: show (generate the data when the dialog is about to be shown), layout (generate the data during the layout phase - might result in multiple files). The data can also be generated when the F12 is pressed in a dialog.")
#endif
		("editor,e", po::value<std::string>()->implicit_value(std::string()), "starts the in-game map editor directly. If file <arg> is specified, equivalent to -e --load <arg>.")
		("help,h", "prints this message and exits." IMPLY_TERMINAL)
		("language,L", po::value<std::string>(), "uses language <arg> (symbol) this session. Example: --language ang_GB@latin")
		("load,l", po::value<std::string>(), "loads the save <arg> from the standard save game directory. When launching the map editor via -e, the map <arg> is loaded, relative to the current directory. If it is a directory, the editor will start with a load map dialog opened there.")
		("noaddons", "disables the loading of all add-ons.")
		("nocache", "disables caching of game data.")
		("nodelay", "runs the game without any delays.")
		("nomusic", "runs the game without music.")
		("nosound", "runs the game without sounds and music.")
		("password", po::value<std::string>(), "uses <password> when connecting to a server, ignoring other preferences.")
		("plugin", po::value<std::string>(), "(experimental) load a script which defines a wesnoth plugin. similar to --script below, but Lua file should return a function which will be run as a coroutine and periodically woken up with updates.")
		("render-image", po::value<two_strings>()->multitoken(), "takes two arguments: <image> <output>. Like screenshot, but instead of a map, takes a valid Wesnoth 'image path string' with image path functions, and writes it to a .png file." IMPLY_TERMINAL)
		("generate-spritesheet", po::value<std::string>(), "generates a spritesheet from all png images in the given path, recursively (one sheet per directory)")
    ("report,R", "initializes game directories, prints build information suitable for use in bug reports, and exits." IMPLY_TERMINAL)
		("rng-seed", po::value<unsigned int>(), "seeds the random number generator with number <arg>. Example: --rng-seed 0")
		("screenshot", po::value<two_strings>()->multitoken(), "takes two arguments: <map> <output>. Saves a screenshot of <map> to <output> without initializing a screen. Editor must be compiled in for this to work." IMPLY_TERMINAL)
		("script", po::value<std::string>(), "(experimental) file containing a Lua script to control the client")
		("server,s", po::value<std::string>()->implicit_value(std::string()), "connects to the host <arg> if specified or to the first host in your preferences.")
		("strict-validation", "makes validation errors fatal")
		("translations-over", po::value<unsigned int>(), "Specify the standard for determining whether a translation is complete.")
		("unsafe-scripts", "makes the \'package\' package available to Lua scripts, so that they can load arbitrary packages. Do not do this with untrusted scripts! This action gives ua the same permissions as the Wesnoth executable.")
		("usercache-dir", po::value<std::string>(), "sets the path of the cache directory to $HOME/<arg> or My Documents\\My Games\\<arg> for Windows. You can specify also an absolute path outside the $HOME or My Documents\\My Games directory. Defaults to $HOME/.cache/wesnoth on X11 and to the userdata-dir on other systems.")
		("usercache-path", "prints the path of the cache directory and exits.")
		("userdata-dir", po::value<std::string>(), "sets the path of the userdata directory. You can use ~ to denote $HOME or My Documents\\My Games on Windows.")
		("userdata-path", "prints the path of the userdata directory and exits." IMPLY_TERMINAL)
		("username", po::value<std::string>(), "uses <username> when connecting to a server, ignoring other preferences.")
		("validcache", "assumes that the cache is valid. (dangerous)")
		("version,v", "prints the game's version number and exits." IMPLY_TERMINAL)
		("simple-version", "prints the game's version number and nothing else." IMPLY_TERMINAL)
		("with-replay", "replays the file loaded with the --load option.")
		;

	po::options_description campaign_opts("Campaign options");
	campaign_opts.add_options()
		("campaign,c", po::value<std::string>()->implicit_value(std::string()), "goes directly to the campaign with id <arg>. A selection menu will appear if no id was specified.")
		("campaign-difficulty", po::value<int>(), "The difficulty of the specified campaign (1 to max). If none specified, the campaign difficulty selection widget will appear.")
		("campaign-scenario", po::value<std::string>(),"The id of the scenario from the specified campaign. The default is the first scenario.")
		("campaign-skip-story", "Skip [story] tags of the specified campaign.")
		;

	po::options_description display_opts("Display options");
	display_opts.add_options()
		("fps", "displays the number of frames per second the game is currently running at, in a corner of the screen. Min/avg/max don't take the FPS limiter into account, act does.")
		("fullscreen,f", "runs the game in full screen mode.")
		("max-fps", po::value<int>(), "the maximum fps the game tries to run at. Values should be between 1 and 1000, the default is the display's refresh rate.")
		("new-widgets", "there is a new WIP widget toolkit this switch enables the new toolkit (VERY EXPERIMENTAL don't file bug reports since most are known). Parts of the library are deemed stable and will work without this switch.")
		("resolution,r", po::value<std::string>(), "sets the screen resolution. <arg> should have format XxY. Example: --resolution 800x600")
		("windowed,w", "runs the game in windowed mode.")
		;

	po::options_description logging_opts("Logging options");
	logging_opts.add_options()
		("logdomains", po::value<std::string>()->implicit_value(std::string()), "lists defined log domains (only the ones containing <arg> filter if such is provided) and exits." IMPLY_TERMINAL)
		("log-error", po::value<std::vector<std::string>>()->composing(), "sets the severity level of the specified log domain(s) to 'error'. <arg> should be given as a comma-separated list of domains, wildcards are allowed. Example: --log-error=network,gui/*,engine/enemies")
		("log-warning", po::value<std::vector<std::string>>()->composing(), "sets the severity level of the specified log domain(s) to 'warning'. Similar to --log-error.")
		("log-info", po::value<std::vector<std::string>>()->composing(), "sets the severity level of the specified log domain(s) to 'info'. Similar to --log-error.")
		("log-debug", po::value<std::vector<std::string>>()->composing(), "sets the severity level of the specified log domain(s) to 'debug'. Similar to --log-error.")
		("log-none", po::value<std::vector<std::string>>()->composing(), "sets the severity level of the specified log domain(s) to 'none'. Similar to --log-error.")
		("log-precise", "shows the timestamps in log output with more precision.")
		("no-log-to-file", "log output is written only to standard error rather than to a file. The environment variable WESNOTH_NO_LOG_FILE can also be set as an alternative.")
		("log-to-file", "log output is written to the log file instead of standard error. Cancels the effect of --no-log-to-file whether implicit or explicit.")
		("no-log-sanitize", "disables the anonymization that's normally applied when logging, for example replacing usernames with USER.")
#ifdef _WIN32
		("wnoconsole", "For Windows, when used with --no-log-to-file, results in output being written to cerr/cout instead of CONOUT. Otherwise, does nothing.")
#endif
		;

	po::options_description multiplayer_opts("Multiplayer options");
	multiplayer_opts.add_options()
		("multiplayer,m", "Starts a multiplayer game. There are additional options that can be used as explained below:")
		("ai-config", po::value<std::vector<std::string>>()->composing(), "selects a configuration file to load for this side. <arg> should have format side:value")
		("algorithm", po::value<std::vector<std::string>>()->composing(), "selects a non-standard algorithm to be used by the AI controller for this side. <arg> should have format side:value")
		("controller", po::value<std::vector<std::string>>()->composing(), "selects the controller for this side. <arg> should have format side:value")
		("era", po::value<std::string>(), "selects the era to be played in by its id.")
		("exit-at-end", "exit Wesnoth at the end of the scenario.")
		("ignore-map-settings", "do not use map settings.")
		("label", po::value<std::string>(), "sets the label for AIs.") // TODO: is the description precise? this option was undocumented before.
		("multiplayer-repeat",  po::value<unsigned int>(), "repeats a multiplayer game after it is finished <arg> times.")
		("nogui", "runs the game without the GUI." IMPLY_TERMINAL)
		("parm", po::value<std::vector<std::string>>()->composing(), "sets additional parameters for this side. <arg> should have format side:name:value.")
		("scenario", po::value<std::string>(), "selects a multiplayer scenario. The default scenario is \"multiplayer_The_Freelands\".")
		("side", po::value<std::vector<std::string>>()->composing(), "selects a faction of the current era for this side by id. <arg> should have format side:value.")
		("turns", po::value<std::string>(), "sets the number of turns. By default no turn limit is set.")
		;

	po::options_description testing_opts("Testing options");
	testing_opts.add_options()
		("test,t", po::value<std::string>()->implicit_value(std::string()), "runs the game in a small test scenario. If specified, scenario <arg> will be used instead.")
		("unit,u", po::value<std::vector<std::string>>(), "runs a unit test scenario. The GUI is not shown and the exit code of the program reflects the victory / defeat conditions of the scenario.\n\t0 - PASS\n\t1 - FAIL\n\t3 - FAIL (INVALID REPLAY)\n\t4 - FAIL (ERRORED REPLAY)\n\t5 - FAIL (BROKE STRICT)\n\t6 - FAIL (WML EXCEPTION)\n\tMultiple tests can be run by giving this option multiple times, in this case the test run will stop immediately after any test which doesn't PASS and the return code will be the status of the test that caused the stop." IMPLY_TERMINAL)
		("showgui", "don't run headlessly (for debugging a failing test)")
		("log-strict", po::value<std::string>(), "sets the strict level of the logger. any messages sent to log domains of this level or more severe will cause the unit test to fail regardless of the victory result.")
		("nobanner", "suppress startup banner.")
		("noreplaycheck", "don't try to validate replay of unit test.")
		("mp-test", "load the test mp scenarios.")
		;

	po::options_description parsing_opts("WML parsing options");
	parsing_opts.add_options()
		("use-schema,S", po::value<std::string>(), "specify a schema to validate WML against (defaults to the core schema).")
		("validate,V", po::value<std::string>(), "validate a specified WML file against a schema." IMPLY_TERMINAL)
		("validate-addon", po::value<std::string>()->value_name("addon_id"), "validate the specified addon's WML against the schema. Requires the user to play the campaign (in the GUI) to trigger the validation.")
		("validate-core", "validate the core WML against the schema.")
		("validate-schema", po::value<std::string>(), "validate a specified WML schema." IMPLY_TERMINAL)
		("diff,D", po::value<two_strings>()->multitoken(), "diff two preprocessed WML documents." IMPLY_TERMINAL)
		("output,o", po::value<std::string>(), "output to specified file")
		("patch,P", po::value<two_strings>()->multitoken(), "apply a patch to a preprocessed WML document." IMPLY_TERMINAL)
		("preprocess,p", po::value<two_strings>()->multitoken(), "requires two arguments: <file/folder> <target directory>. Preprocesses a specified file/folder. The preprocessed file(s) will be written in the specified target directory: a plain cfg file and a processed cfg file." IMPLY_TERMINAL)
		("preprocess-defines", po::value<std::string>(), "comma separated list of defines to be used by '--preprocess' command. If 'SKIP_CORE' is in the define list the data/core won't be preprocessed. Example: --preprocess-defines=FOO,BAR")
		("preprocess-input-macros", po::value<std::string>(), "used only by the '--preprocess' command. Specifies source file <arg> that contains [preproc_define]s to be included before preprocessing.")
		("preprocess-output-macros", po::value<std::string>()->implicit_value(std::string()), "used only by the '--preprocess' command. Will output all preprocessed macros in the target file <arg>. If the file is not specified the output will be file '_MACROS_.cfg' in the target directory of preprocess's command.")
		;

	//hidden_.add_options()
	//	("example-hidden-option", "")
	//	;
	visible_.add(general_opts).add(campaign_opts).add(display_opts).add(logging_opts).add(multiplayer_opts).add(testing_opts).add(parsing_opts);

	all_.add(visible_).add(hidden_);

	po::positional_options_description positional;
	positional.add("data-dir",1);

	po::variables_map vm;
	const int parsing_style = po::command_line_style::default_style ^ po::command_line_style::allow_guessing;

	const auto parsed_options = po::command_line_parser(args_)
		.options(all_)
		.positional(positional)
		.style(parsing_style)
		.run();

	po::store(parsed_options, vm);

	if(vm.count("ai-config"))
		multiplayer_ai_config = parse_to_uint_string_tuples_(vm["ai-config"].as<std::vector<std::string>>());
	if(vm.count("algorithm"))
		multiplayer_algorithm = parse_to_uint_string_tuples_(vm["algorithm"].as<std::vector<std::string>>());
	if(vm.count("campaign"))
		campaign = vm["campaign"].as<std::string>();
	if(vm.count("campaign-difficulty"))
		campaign_difficulty = vm["campaign-difficulty"].as<int>();
	if(vm.count("campaign-scenario"))
		campaign_scenario = vm["campaign-scenario"].as<std::string>();
	if(vm.count("campaign-skip-story"))
		campaign_skip_story = true;
	if(vm.count("clock"))
		clock = true;
	if(vm.count("core"))
		core_id = vm["core"].as<std::string>();
	if(vm.count("controller"))
		multiplayer_controller = parse_to_uint_string_tuples_(vm["controller"].as<std::vector<std::string>>());
	if(vm.count("data-dir"))
		data_dir = vm["data-dir"].as<std::string>();
	if(vm.count("data-path"))
		data_path = true;
	if(vm.count("debug"))
		debug = true;
	if(vm.count("debug-lua"))
		debug_lua = true;
	if(vm.count("strict-lua"))
		strict_lua = true;
	if(vm.count("allow-insecure"))
		allow_insecure = true;
#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
	if(vm.count("debug-dot-domain")) {
		debug_dot_domain = vm["debug-dot-domain"].as<std::string>();
	}
	if(vm.count("debug-dot-level")) {
		debug_dot_level = vm["debug-dot-level"].as<std::string>();
	}
#endif
	if(vm.count("editor"))
		editor = vm["editor"].as<std::string>();
	if(vm.count("era"))
		multiplayer_era = vm["era"].as<std::string>();
	if(vm.count("exit-at-end"))
		multiplayer_exit_at_end = true;
	if(vm.count("fps"))
		fps = true;
	if(vm.count("fullscreen"))
		fullscreen = true;
	if(vm.count("help"))
		help = true;
	if(vm.count("ignore-map-settings"))
		multiplayer_ignore_map_settings = true;
	if(vm.count("label"))
		multiplayer_label = vm["label"].as<std::string>();
	if(vm.count("language"))
		language = vm["language"].as<std::string>();
	if(vm.count("load"))
		load = vm["load"].as<std::string>();
	if(vm.count("logdomains"))
		logdomains = vm["logdomains"].as<std::string>();
	if(vm.count("log-precise"))
		log_precise_timestamps = true;
	if(vm.count("log-strict"))
		parse_log_strictness(vm["log-strict"].as<std::string>());
	if(vm.count("max-fps"))
		max_fps = vm["max-fps"].as<int>();
	if(vm.count("mp-test"))
		mptest = true;
	if(vm.count("multiplayer"))
		multiplayer = true;
	if(vm.count("multiplayer-repeat"))
		multiplayer_repeat = vm["multiplayer-repeat"].as<unsigned int>();
	if(vm.count("new-widgets"))
		new_widgets = true;
	if(vm.count("noaddons"))
		noaddons = true;
	if(vm.count("nocache"))
		nocache = true;
	if(vm.count("nodelay"))
		nodelay = true;
	if(vm.count("nomusic"))
		nomusic = true;
	if(vm.count("noreplaycheck"))
		noreplaycheck = true;
	if(vm.count("nosound"))
		nosound = true;
	if(vm.count("nogui"))
		nogui = true;
	if(vm.count("nobanner"))
		nobanner = true;
	if(vm.count("parm"))
		multiplayer_parm = parse_to_uint_string_string_tuples_(vm["parm"].as<std::vector<std::string>>());
	if(vm.count("preprocess"))
	{
		preprocess = true;
		preprocess_path = vm["preprocess"].as<two_strings>().first;
		preprocess_target = vm["preprocess"].as<two_strings>().second;
	}
	if(vm.count("diff"))
	{
		do_diff = true;
		diff_left = vm["diff"].as<two_strings>().first;
		diff_right = vm["diff"].as<two_strings>().second;
	}
	if(vm.count("patch"))
	{
		do_patch = true;
		diff_left = vm["patch"].as<two_strings>().first;
		diff_right = vm["patch"].as<two_strings>().second;
	}
	if(vm.count("output"))
	{
		output_file = vm["output"].as<std::string>();
	}
	if(vm.count("preprocess-defines"))
		preprocess_defines = utils::split(vm["preprocess-defines"].as<std::string>(), ',');
	if(vm.count("preprocess-input-macros"))
		preprocess_input_macros = vm["preprocess-input-macros"].as<std::string>();
	if(vm.count("preprocess-output-macros"))
		preprocess_output_macros = vm["preprocess-output-macros"].as<std::string>();
	if(vm.count("resolution"))
		parse_resolution_(vm["resolution"].as<std::string>());
	if(vm.count("rng-seed"))
		rng_seed = vm["rng-seed"].as<unsigned int>();
	if(vm.count("scenario"))
		multiplayer_scenario = vm["scenario"].as<std::string>();
	if(vm.count("render-image"))
	{
		render_image = vm["render-image"].as<two_strings>().first;
		render_image_dst = vm["render-image"].as<two_strings>().second;
	}
	if(vm.count("generate-spritesheet"))
		generate_spritesheet = vm["generate-spritesheet"].as<std::string>();
	if(vm.count("screenshot"))
	{
		screenshot = true;
		screenshot_map_file = vm["screenshot"].as<two_strings>().first;
		screenshot_output_file = vm["screenshot"].as<two_strings>().second;
	}
	if(vm.count("script"))
		script_file = vm["script"].as<std::string>();
	if(vm.count("unsafe-scripts"))
		script_unsafe_mode = true;
	if(vm.count("plugin"))
		plugin_file = vm["plugin"].as<std::string>();
	if(vm.count("server"))
		server = vm["server"].as<std::string>();
	if(vm.count("username"))
		username = vm["username"].as<std::string>();
	if(vm.count("password"))
		password = vm["password"].as<std::string>();
	if(vm.count("report"))
		report = true;
	if(vm.count("side"))
		multiplayer_side = parse_to_uint_string_tuples_(vm["side"].as<std::vector<std::string>>());
	if(vm.count("test"))
		test = vm["test"].as<std::string>();
	if(vm.count("unit"))
	{
		unit_test = vm["unit"].as<std::vector<std::string>>();
		headless_unit_test = true;
	}
	if(vm.count("showgui"))
		headless_unit_test = false;
	if(vm.count("noreplaycheck"))
		noreplaycheck = true;
	if(vm.count("turns"))
		multiplayer_turns = vm["turns"].as<std::string>();
	if(vm.count("strict-validation"))
		strict_validation = true;
	if(vm.count("usercache-dir"))
		usercache_dir = vm["usercache-dir"].as<std::string>();
	if(vm.count("usercache-path"))
		usercache_path = true;
	if(vm.count("userdata-dir"))
		userdata_dir = vm["userdata-dir"].as<std::string>();
	if(vm.count("userdata-path"))
		userdata_path = true;
	if(vm.count("validcache"))
		validcache = true;
	// If you add a new validate-* option, remember the any_validation_option() function
	if(vm.count("validate"))
		validate_wml = vm["validate"].as<std::string>();
	if(vm.count("validate-core"))
		validate_core = true;
	if(vm.count("validate-addon"))
		validate_addon = vm["validate-addon"].as<std::string>();
	if(vm.count("validate-schema"))
		validate_schema = vm["validate-schema"].as<std::string>();
	// If you add a new validate-* option, remember the any_validation_option() function
	if(vm.count("use-schema"))
		validate_with = vm["use-schema"].as<std::string>();
	if(vm.count("version"))
		version = true;
	if(vm.count("simple-version"))
	{
		simple_version = true;
		nobanner = true;
	}
	if(vm.count("windowed"))
		windowed = true;
	if(vm.count("with-replay"))
		with_replay = true;
#ifdef _WIN32
	if(vm.count("wnoconsole"))
		no_console = true;
#endif
	if(vm.count("no-log-sanitize"))
		no_log_sanitize = true;
	if(vm.count("log-to-file"))
		log_to_file = true;
	if(vm.count("no-log-to-file"))
		no_log_to_file = true;
	if(vm.count("all-translations"))
		translation_percent = 0;
	else if(vm.count("translations-over"))
		translation_percent = std::clamp<unsigned int>(vm["translations-over"].as<unsigned int>(), 0, 100);

	// Parse log domain severity following the command line order.
	for (const auto& option : parsed_options.options) {
		if (!option.value.empty()) {
			if (option.string_key == "log-error") {
				parse_log_domains_(option.value.front(),lg::err().get_severity());
			} else if (option.string_key == "log-warning") {
				parse_log_domains_(option.value.front(),lg::warn().get_severity());
			} else if (option.string_key == "log-info") {
				parse_log_domains_(option.value.front(),lg::info().get_severity());
			} else if (option.string_key == "log-debug") {
				parse_log_domains_(option.value.front(),lg::debug().get_severity());
			} else if (option.string_key == "log-none") {
				parse_log_domains_(option.value.front(),lg::severity::LG_NONE);
			}
		}
	}
}

void commandline_options::parse_log_domains_(const std::string &domains_string, const lg::severity severity)
{
	if(std::vector<std::string> domains = utils::split(domains_string, ','); !domains.empty()) {
		if(!log) {
			log.emplace();
		}
		for(auto&& domain : domains) {
			log->emplace_back(severity, std::move(domain));
		}
	}
}

void commandline_options::parse_log_strictness (const std::string & severity) {
	static const std::array<const lg::logger*, 4> loggers {{&lg::err(), &lg::warn(), &lg::info(), &lg::debug()}};
	for(const lg::logger * l : loggers ) {
		if(severity == l->get_name()) {
			lg::set_strict_severity(*l);
			return;
		}
	}
	PLAIN_LOG << "Unrecognized argument to --log-strict : " << severity << " . \nDisabling strict mode logging.";
	lg::set_strict_severity(lg::severity::LG_NONE);
}

void commandline_options::parse_resolution_ (const std::string& resolution_string)
{
	const std::vector<std::string> tokens = utils::split(resolution_string, 'x');
	if(tokens.size() != 2) {
		throw bad_commandline_resolution(resolution_string);
	}

	int xres, yres;

	try {
		xres = std::stoi(tokens[0]);
		yres = std::stoi(tokens[1]);
	} catch(const std::invalid_argument &) {
		throw bad_commandline_resolution(resolution_string);
	}

	resolution = std::pair(xres, yres);
}

std::vector<std::pair<unsigned int,std::string>> commandline_options::parse_to_uint_string_tuples_(const std::vector<std::string> &strings, char separator)
{
	std::vector<std::pair<unsigned int,std::string>> vec;

	using namespace std::literals;
	const std::string expected_format
			= "UINT"s + separator + "STRING";

	for(const std::string &s : strings) {
		std::vector<std::string> tokens = utils::split(s, separator);
		if(tokens.size() != 2) {
			throw bad_commandline_tuple(s, expected_format);
		}

		unsigned int temp;
		try {
			temp = lexical_cast<unsigned int>(tokens[0]);
		} catch (const bad_lexical_cast &) {
			throw bad_commandline_tuple(s, expected_format);
		}

		vec.emplace_back(temp, std::move(tokens[1]));
	}
	return vec;
}

std::vector<std::tuple<unsigned int,std::string,std::string>> commandline_options::parse_to_uint_string_string_tuples_(const std::vector<std::string> &strings, char separator)
{
	std::vector<std::tuple<unsigned int,std::string,std::string>> vec;

	using namespace std::literals;
	const std::string expected_format
			= "UINT"s + separator + "STRING" + separator + "STRING";

	for(const std::string &s : strings) {
		const std::vector<std::string> tokens = utils::split(s, separator);
		if(tokens.size() != 3) {
			throw bad_commandline_tuple(s, expected_format);
		}

		unsigned int temp;
		try {
			temp = lexical_cast<unsigned int>(tokens[0]);
		} catch (const bad_lexical_cast &) {
			throw bad_commandline_tuple(s, expected_format);
		}

		vec.emplace_back(temp, tokens[1], tokens[2]);
	}
	return vec;
}

std::ostream& operator<<(std::ostream &os, const commandline_options& cmdline_opts)
{
	os << "Usage: " << cmdline_opts.args0_ << " [<options>] [<data-directory>]\n";
	os << cmdline_opts.visible_;
	return os;
}

config commandline_options::to_config() const {
	config ret;
	if(server) {
		ret["server"] = *server;
	}
	if(username) {
		ret["username"] = *username;
	}
	if(password) {
		ret["password"] = *password;
	}
	return ret;
}

bool commandline_options::any_validation_option() const
{
	return validate_addon || validate_core || validate_schema || validate_with || validate_wml;
}
