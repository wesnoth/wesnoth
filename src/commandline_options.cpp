/*
   Copyright (C) 2011 - 2017 by Lukasz Dobrogowski <lukasz.dobrogowski@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any lfooater version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "commandline_options.hpp"

#include "config.hpp"
#include "formatter.hpp"
#include "lexical_cast.hpp"
#include "log.hpp"                      // for logger, set_strict_severity, etc
#include "serialization/string_utils.hpp"  // for split

#include <boost/any.hpp>                // for any
#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/errors.hpp>  // for validation_error, etc
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/value_semantic.hpp>  // for value, etc
#include <boost/program_options/variables_map.hpp>  // for variables_map, etc
#include <iostream>                     // for operator<<, basic_ostream, etc

namespace po = boost::program_options;

class two_strings : public std::pair<std::string,std::string> {};

static void validate(boost::any& v, const std::vector<std::string>& values,
              two_strings*, int)
{
	two_strings ret_val;
	if (values.size() != 2) {
		throw po::validation_error(po::validation_error::invalid_option_value);
	}
	ret_val.first = values.at(0);
	ret_val.second = values.at(1);
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

commandline_options::commandline_options (const std::vector<std::string>& args) :
	bunzip2(),
	bzip2(),
	campaign(),
	campaign_difficulty(),
	campaign_scenario(),
	clock(false),
	data_path(false),
	data_dir(),
	debug(false),
	debug_lua(false),
#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
	debug_dot_domain(),
	debug_dot_level(),
#endif
	editor(),
	fps(false),
	fullscreen(false),
	gunzip(),
	gzip(),
	help(),
	language(),
	log(),
	load(),
	logdomains(),
	log_precise_timestamps(false),
	multiplayer(false),
	multiplayer_ai_config(),
	multiplayer_algorithm(),
	multiplayer_controller(),
	multiplayer_era(),
	multiplayer_exit_at_end(),
	multiplayer_ignore_map_settings(),
	multiplayer_label(),
	multiplayer_parm(),
	multiplayer_repeat(),
	multiplayer_scenario(),
	multiplayer_side(),
	multiplayer_turns(),
	max_fps(),
	noaddons(false),
	nocache(false),
	nodelay(false),
	nogui(false),
	nomusic(false),
	nosound(false),
	new_widgets(false),
	path(false),
	preprocess(false),
	preprocess_defines(),
	preprocess_input_macros(),
	preprocess_output_macros(),
	preprocess_path(),
	preprocess_target(),
	resolution(),
	rng_seed(),
	server(),
	username(),
	password(),
	screenshot(false),
	screenshot_map_file(),
	screenshot_output_file(),
	script_unsafe_mode(false),
	strict_validation(false),
	test(),
	unit_test(),
	headless_unit_test(false),
	noreplaycheck(false),
	mptest(false),
	userconfig_path(false),
	userconfig_dir(),
	userdata_path(false),
	userdata_dir(),
	validcache(false),
	version(false),
	report(false),
	windowed(false),
	with_replay(false),
	args_(args.begin() + 1 , args.end()),
	args0_(*args.begin()),
	all_(),
	visible_(),
	hidden_()
{
	// When adding items don't forget to update doc/man/wesnoth.6
	// Options are sorted alphabetically by --long-option.
	po::options_description general_opts("General options");
	general_opts.add_options()
		("bunzip2", po::value<std::string>(), "decompresses a file (<arg>.bz2) in bzip2 format and stores it without the .bz2 suffix. <arg>.bz2 will be removed.")
		("bzip2", po::value<std::string>(), "compresses a file (<arg>) in bzip2 format, stores it as <arg>.bz2 and removes <arg>.")
		("clock", "Adds the option to show a clock for testing the drawing timer.")
		("config-dir", po::value<std::string>(), "sets the path of the userdata directory to $HOME/<arg> or My Documents\\My Games\\<arg> for Windows. You can specify also an absolute path outside the $HOME or My Documents\\My Games directory. DEPRECATED: use userdata-path and userconfig-path instead.")
		("config-path", "prints the path of the userdata directory and exits. DEPRECATED: use userdata-path and userconfig-path instead.")
		("core", po::value<std::string>(), "overrides the loaded core with the one whose id is specified.")
		("data-dir", po::value<std::string>(), "overrides the data directory with the one specified.")
		("data-path", "prints the path of the data directory and exits.")
		("debug,d", "enables additional command mode options in-game.")
		("debug-lua", "enables some Lua debugging mechanisms")
#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
		("debug-dot-level", po::value<std::string>(), "sets the level of the debug dot files. <arg> should be a comma separated list of levels. These files are used for debugging the widgets especially the for the layout engine. When enabled the engine will produce dot files which can be converted to images with the dot tool. Available levels: size (generate the size info of the widget), state (generate the state info of the widget).")
		("debug-dot-domain", po::value<std::string>(), "sets the domain of the debug dot files. <arg> should be a comma separated list of domains. See --debug-dot-level for more info. Available domains: show (generate the data when the dialog is about to be shown), layout (generate the data during the layout phase - might result in multiple files). The data can also be generated when the F12 is pressed in a dialog.")
#endif
		("editor,e", po::value<std::string>()->implicit_value(std::string()), "starts the in-game map editor directly. If file <arg> is specified, equivalent to -e --load <arg>.")
		("gunzip", po::value<std::string>(), "decompresses a file (<arg>.gz) in gzip format and stores it without the .gz suffix. <arg>.gz will be removed.")
		("gzip", po::value<std::string>(), "compresses a file (<arg>) in gzip format, stores it as <arg>.gz and removes <arg>.")
		("help,h", "prints this message and exits.")
		("language,L", po::value<std::string>(), "uses language <arg> (symbol) this session. Example: --language ang_GB@latin")
		("load,l", po::value<std::string>(), "loads the save <arg> from the standard save game directory. When launching the map editor via -e, the map <arg> is loaded, relative to the current directory. If it is a directory, the editor will start with a load map dialog opened there.")
		("noaddons", "disables the loading of all add-ons.")
		("nocache", "disables caching of game data.")
		("nodelay", "runs the game without any delays.")
		("nomusic", "runs the game without music.")
		("nosound", "runs the game without sounds and music.")
		("path", "prints the path to the data directory and exits.")
		("plugin", po::value<std::string>(), "(experimental) load a script which defines a wesnoth plugin. similar to --script below, but lua file should return a function which will be run as a coroutine and periodically woken up with updates.")
		("render-image", po::value<two_strings>()->multitoken(), "takes two arguments: <image> <output>. Like screenshot, but instead of a map, takes a valid wesnoth 'image path string' with image path functions, and outputs to a windows .bmp file."
#ifdef _WIN32
		 " Implies --wconsole."
#endif // _WIN32
		 )
		("report,R", "initializes game directories, prints build information suitable for use in bug reports, and exits.")
		("rng-seed", po::value<unsigned int>(), "seeds the random number generator with number <arg>. Example: --rng-seed 0")
		("screenshot", po::value<two_strings>()->multitoken(), "takes two arguments: <map> <output>. Saves a screenshot of <map> to <output> without initializing a screen. Editor must be compiled in for this to work."
#ifdef _WIN32
		 " Implies --wconsole."
#endif // _WIN32
		 )
		("script", po::value<std::string>(), "(experimental) file containing a lua script to control the client")
		("unsafe-scripts", "makes the \'package\' package available to lua scripts, so that they can load arbitrary packages. Do not do this with untrusted scripts! This action gives lua the same permissions as the wesnoth executable.")
		("server,s", po::value<std::string>()->implicit_value(std::string()), "connects to the host <arg> if specified or to the first host in your preferences.")
		("username", po::value<std::string>(), "uses <username> when connecting to a server, ignoring other preferences.")
		("password", po::value<std::string>(), "uses <password> when connecting to a server, ignoring other preferences.")
		("strict-validation", "makes validation errors fatal")
		("userconfig-dir", po::value<std::string>(), "sets the path of the user config directory to $HOME/<arg> or My Documents\\My Games\\<arg> for Windows. You can specify also an absolute path outside the $HOME or My Documents\\My Games directory. Defaults to $HOME/.config/wesnoth on X11 and to the userdata-dir on other systems.")
		("userconfig-path", "prints the path of the user config directory and exits.")
		("userdata-dir", po::value<std::string>(), "sets the path of the userdata directory to $HOME/<arg> or My Documents\\My Games\\<arg> for Windows. You can specify also an absolute path outside the $HOME or My Documents\\My Games directory.")
		("userdata-path", "prints the path of the userdata directory and exits.")
		("validcache", "assumes that the cache is valid. (dangerous)")
		("version,v", "prints the game's version number and exits.")
		("with-replay", "replays the file loaded with the --load option.")
#ifdef _WIN32
		("wconsole", "attaches a console window on startup (Windows only). Implied by any option that prints something and exits.")
#endif // _WIN32
		;

	po::options_description campaign_opts("Campaign options");
	campaign_opts.add_options()
		("campaign,c", po::value<std::string>()->implicit_value(std::string()), "goes directly to the campaign with id <arg>. A selection menu will appear if no id was specified.")
		("campaign-difficulty", po::value<int>(), "The difficulty of the specified campaign (1 to max). If none specified, the campaign difficulty selection widget will appear.")
		("campaign-scenario", po::value<std::string>(),"The id of the scenario from the specified campaign. The default is the first scenario.")
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
		("logdomains", po::value<std::string>()->implicit_value(std::string()), "lists defined log domains (only the ones containing <arg> filter if such is provided) and exits.")
		("log-error", po::value<std::string>(), "sets the severity level of the specified log domain(s) to 'error'. <arg> should be given as comma separated list of domains, wildcards are allowed. Example: --log-error=network,gui/*,engine/enemies")
		("log-warning", po::value<std::string>(), "sets the severity level of the specified log domain(s) to 'warning'. Similar to --log-error.")
		("log-info", po::value<std::string>(), "sets the severity level of the specified log domain(s) to 'info'. Similar to --log-error.")
		("log-debug", po::value<std::string>(), "sets the severity level of the specified log domain(s) to 'debug'. Similar to --log-error.")
		("log-precise", "shows the timestamps in the logfile with more precision.")
		;

	po::options_description multiplayer_opts("Multiplayer options");
	multiplayer_opts.add_options()
		("multiplayer,m", "Starts a multiplayer game. There are additional options that can be used as explained below:")
		("ai-config", po::value<std::vector<std::string> >()->composing(), "selects a configuration file to load for this side. <arg> should have format side:value")
		("algorithm", po::value<std::vector<std::string> >()->composing(), "selects a non-standard algorithm to be used by the AI controller for this side. <arg> should have format side:value")
		("controller", po::value<std::vector<std::string> >()->composing(), "selects the controller for this side. <arg> should have format side:value")
		("era", po::value<std::string>(), "selects the era to be played in by its id.")
		("exit-at-end", "exit Wesnoth at the end of the scenario.")
		("ignore-map-settings", "do not use map settings.")
		("label", po::value<std::string>(), "sets the label for AIs.") //TODO is the description precise? this option was undocumented before.
		("multiplayer-repeat",  po::value<unsigned int>(), "repeats a multiplayer game after it is finished <arg> times.")
		("nogui", "runs the game without the GUI.")
		("parm", po::value<std::vector<std::string> >()->composing(), "sets additional parameters for this side. <arg> should have format side:name:value.")
		("scenario", po::value<std::string>(), "selects a multiplayer scenario. The default scenario is \"multiplayer_The_Freelands\".")
		("side", po::value<std::vector<std::string> >()->composing(), "selects a faction of the current era for this side by id. <arg> should have format side:value.")
		("turns", po::value<std::string>(), "sets the number of turns. The default is \"50\".")
		;

	po::options_description testing_opts("Testing options");
	testing_opts.add_options()
		("test,t", po::value<std::string>()->implicit_value(std::string()), "runs the game in a small test scenario. If specified, scenario <arg> will be used instead.")
		("unit,u", po::value<std::string>()->implicit_value(std::string()), "runs a unit test scenario. Works like test, except that the exit code of the program reflects the victory / defeat conditions of the scenario.\n\t0 - PASS\n\t1 - FAIL\n\t2 - FAIL (TIMEOUT)\n\t3 - FAIL (INVALID REPLAY)\n\t4 - FAIL (ERRORED REPLAY)")
		("showgui", "don't run headlessly (for debugging a failing test)")
		("timeout", po::value<unsigned int>(), "sets a timeout (milliseconds) for the unit test. (DEPRECATED)")
		("log-strict", po::value<std::string>(), "sets the strict level of the logger. any messages sent to log domains of this level or more severe will cause the unit test to fail regardless of the victory result.")
		("noreplaycheck", "don't try to validate replay of unit test.")
		("mp-test", "load the test mp scenarios.")
		;

	po::options_description preprocessor_opts("Preprocessor mode options");
	preprocessor_opts.add_options()
		("preprocess,p", po::value<two_strings>()->multitoken(), "requires two arguments: <file/folder> <target directory>. Preprocesses a specified file/folder. The preprocessed file(s) will be written in the specified target directory: a plain cfg file and a processed cfg file.")
		("preprocess-defines", po::value<std::string>(), "comma separated list of defines to be used by '--preprocess' command. If 'SKIP_CORE' is in the define list the data/core won't be preprocessed. Example: --preprocess-defines=FOO,BAR")
		("preprocess-input-macros", po::value<std::string>(), "used only by the '--preprocess' command. Specifies source file <arg> that contains [preproc_define]s to be included before preprocessing.")
		("preprocess-output-macros", po::value<std::string>()->implicit_value(std::string()), "used only by the '--preprocess' command. Will output all preprocessed macros in the target file <arg>. If the file is not specified the output will be file '_MACROS_.cfg' in the target directory of preprocess's command.")
		;

	po::options_description proxy_opts("Proxy options");
	proxy_opts.add_options()
		("proxy", "enables usage of proxy for network connections.")
		("proxy-address", po::value<std::string>(), "specifies address of the proxy.")
		("proxy-port", po::value<std::string>(), "specifies port of the proxy.")
		("proxy-user", po::value<std::string>(), "specifies username to log in to the proxy.")
		("proxy-password", po::value<std::string>(), "specifies password to log in to the proxy.")
		;

	//hidden_.add_options()
	//	("example-hidden-option", "")
	//	;
	visible_.add(general_opts).add(campaign_opts).add(display_opts).add(logging_opts).add(multiplayer_opts).add(testing_opts).add(preprocessor_opts).add(proxy_opts);

	all_.add(visible_).add(hidden_);

	po::positional_options_description positional;
	positional.add("data-dir",1);

	po::variables_map vm;
	const int parsing_style = po::command_line_style::default_style ^ po::command_line_style::allow_guessing;
	po::store(po::command_line_parser(args_).options(all_).positional(positional).style(parsing_style).run(),vm);

	if (vm.count("ai-config"))
		multiplayer_ai_config = parse_to_uint_string_tuples_(vm["ai-config"].as<std::vector<std::string> >());
	if (vm.count("algorithm"))
		multiplayer_algorithm = parse_to_uint_string_tuples_(vm["algorithm"].as<std::vector<std::string> >());
	if (vm.count("bunzip2"))
		bunzip2 = vm["bunzip2"].as<std::string>();
	if (vm.count("bzip2"))
		bzip2 = vm["bzip2"].as<std::string>();
	if (vm.count("campaign"))
		campaign = vm["campaign"].as<std::string>();
	if (vm.count("campaign-difficulty"))
		campaign_difficulty = vm["campaign-difficulty"].as<int>();
	if (vm.count("campaign-scenario"))
		campaign_scenario = vm["campaign-scenario"].as<std::string>();
	if (vm.count("clock"))
		clock = true;
	if (vm.count("core"))
		core_id = vm["core"].as<std::string>();
	if (vm.count("config-dir"))
		userdata_dir = vm["config-dir"].as<std::string>(); //TODO: complain and remove
	if (vm.count("config-path"))
		userdata_path = true; //TODO: complain and remove
	if (vm.count("controller"))
		multiplayer_controller = parse_to_uint_string_tuples_(vm["controller"].as<std::vector<std::string> >());
	if (vm.count("data-dir"))
		data_dir = vm["data-dir"].as<std::string>();
	if (vm.count("data-path"))
		data_path = true;
	if (vm.count("debug"))
		debug = true;
	if (vm.count("debug-lua"))
		debug_lua = true;
#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
	if (vm.count("debug-dot-domain")) {
		debug_dot_domain = vm["debug-dot-domain"].as<std::string>();
	}
	if (vm.count("debug-dot-level")) {
		debug_dot_level = vm["debug-dot-level"].as<std::string>();
	}
#endif
	if (vm.count("editor"))
		editor = vm["editor"].as<std::string>();
	if (vm.count("era"))
		multiplayer_era = vm["era"].as<std::string>();
	if (vm.count("exit-at-end"))
		multiplayer_exit_at_end = true;
	if (vm.count("fps"))
		fps = true;
	if (vm.count("fullscreen"))
		fullscreen = true;
	if (vm.count("gunzip"))
		gunzip = vm["gunzip"].as<std::string>();
	if (vm.count("gzip"))
		gzip = vm["gzip"].as<std::string>();
	if (vm.count("help"))
		help = true;
	if (vm.count("ignore-map-settings"))
		multiplayer_ignore_map_settings = true;
	if (vm.count("label"))
		multiplayer_label = vm["label"].as<std::string>();
	if (vm.count("language"))
		language = vm["language"].as<std::string>();
	if (vm.count("load"))
		load = vm["load"].as<std::string>();
	if (vm.count("log-error"))
		 parse_log_domains_(vm["log-error"].as<std::string>(),lg::err().get_severity());
	if (vm.count("log-warning"))
		 parse_log_domains_(vm["log-warning"].as<std::string>(),lg::warn().get_severity());
	if (vm.count("log-info"))
		 parse_log_domains_(vm["log-info"].as<std::string>(),lg::info().get_severity());
	if (vm.count("log-debug"))
		 parse_log_domains_(vm["log-debug"].as<std::string>(),lg::debug().get_severity());
	if (vm.count("logdomains"))
		logdomains = vm["logdomains"].as<std::string>();
	if (vm.count("log-precise"))
		log_precise_timestamps = true;
	if (vm.count("log-strict"))
		parse_log_strictness(vm["log-strict"].as<std::string>());
	if (vm.count("max-fps"))
		max_fps = vm["max-fps"].as<int>();
	if (vm.count("mp-test"))
		mptest = true;
	if (vm.count("multiplayer"))
		multiplayer = true;
	if (vm.count("multiplayer-repeat"))
		multiplayer_repeat = vm["multiplayer-repeat"].as<unsigned int>();
	if (vm.count("new-widgets"))
		new_widgets = true;
	if (vm.count("noaddons"))
		noaddons = true;
	if (vm.count("nocache"))
		nocache = true;
	if (vm.count("nodelay"))
		nodelay = true;
	if (vm.count("nomusic"))
		nomusic = true;
	if (vm.count("noreplaycheck"))
		noreplaycheck = true;
	if (vm.count("nosound"))
		nosound = true;
	if (vm.count("nogui"))
		nogui = true;
	if (vm.count("parm"))
		multiplayer_parm = parse_to_uint_string_string_tuples_(vm["parm"].as<std::vector<std::string> >());
	if (vm.count("path"))
		path = true;
	if (vm.count("preprocess"))
	{
		preprocess = true;
		preprocess_path = vm["preprocess"].as<two_strings>().first;
		preprocess_target = vm["preprocess"].as<two_strings>().second;
	}
	if (vm.count("preprocess-defines"))
		preprocess_defines = utils::split(vm["preprocess-defines"].as<std::string>(), ',');
	if (vm.count("preprocess-input-macros"))
		preprocess_input_macros = vm["preprocess-input-macros"].as<std::string>();
	if (vm.count("preprocess-output-macros"))
		preprocess_output_macros = vm["preprocess-output-macros"].as<std::string>();
	if (vm.count("resolution"))
		parse_resolution_(vm["resolution"].as<std::string>());
	if (vm.count("rng-seed"))
		rng_seed = vm["rng-seed"].as<unsigned int>();
	if (vm.count("scenario"))
		multiplayer_scenario = vm["scenario"].as<std::string>();
	if (vm.count("render-image"))
	{
		render_image = vm["render-image"].as<two_strings>().first;
		render_image_dst = vm["render-image"].as<two_strings>().second;
	}
	if (vm.count("screenshot"))
	{
		screenshot = true;
		screenshot_map_file = vm["screenshot"].as<two_strings>().first;
		screenshot_output_file = vm["screenshot"].as<two_strings>().second;
	}
	if (vm.count("script"))
		script_file = vm["script"].as<std::string>();
	if (vm.count("unsafe-scripts"))
		script_unsafe_mode = true;
	if (vm.count("plugin"))
		plugin_file = vm["plugin"].as<std::string>();
	if (vm.count("server"))
		server = vm["server"].as<std::string>();
	if (vm.count("username"))
		username = vm["username"].as<std::string>();
	if (vm.count("password"))
		password = vm["password"].as<std::string>();
	if (vm.count("report"))
		report = true;
	if (vm.count("side"))
		multiplayer_side = parse_to_uint_string_tuples_(vm["side"].as<std::vector<std::string> >());
	if (vm.count("test"))
		test = vm["test"].as<std::string>();
	if (vm.count("unit"))
	{
		unit_test = vm["unit"].as<std::string>();
		headless_unit_test = true;
	}
	if (vm.count("showgui"))
		headless_unit_test = false;
	if (vm.count("timeout"))
		timeout = vm["timeout"].as<unsigned int>();
	if (vm.count("noreplaycheck"))
		noreplaycheck = true;
	if (vm.count("turns"))
		multiplayer_turns = vm["turns"].as<std::string>();
	if (vm.count("strict-validation"))
		strict_validation = true;
	if (vm.count("userconfig-dir"))
		userconfig_dir = vm["userconfig-dir"].as<std::string>();
	if (vm.count("userconfig-path"))
		userconfig_path = true;
	if (vm.count("userdata-dir"))
		userdata_dir = vm["userdata-dir"].as<std::string>();
	if (vm.count("userdata-path"))
		userdata_path = true;
	if (vm.count("validcache"))
		validcache = true;
	if (vm.count("version"))
		version = true;
	if (vm.count("windowed"))
		windowed = true;
	if (vm.count("with-replay"))
		with_replay = true;
}

void commandline_options::parse_log_domains_(const std::string &domains_string, const int severity)
{
	const std::vector<std::string> domains = utils::split(domains_string, ',');
	for (const std::string& domain : domains)
	{
		if (!log)
			log = std::vector<std::pair<int, std::string> >();
		log->emplace_back(severity, domain);
	}
}

void commandline_options::parse_log_strictness (const std::string & severity ) {
	static lg::logger const *loggers[] { &lg::err(), &lg::warn(), &lg::info(), &lg::debug() };
	for (const lg::logger * l : loggers ) {
		if (severity == l->get_name()) {
			lg::set_strict_severity(*l);
			return ;
		}
	}
	std::cerr << "Unrecognized argument to --log-strict : " << severity << " . \nDisabling strict mode logging." << std::endl;
	lg::set_strict_severity(-1);
}

void commandline_options::parse_resolution_ ( const std::string& resolution_string )
{
	const std::vector<std::string> tokens = utils::split(resolution_string, 'x');
	if (tokens.size() != 2) {
		throw bad_commandline_resolution(resolution_string);
	}

	int xres, yres;

	try {
		xres = std::stoi(tokens[0]);
		yres = std::stoi(tokens[1]);
	} catch(std::invalid_argument &) {
		throw bad_commandline_resolution(resolution_string);
	}

	resolution = std::make_pair(xres, yres);
}

std::vector<std::pair<unsigned int,std::string> > commandline_options::parse_to_uint_string_tuples_(const std::vector<std::string> &strings, char separator)
{
	std::vector<std::pair<unsigned int,std::string> > vec;
	const std::string& expected_format
			= std::string() + "UINT" + separator + "STRING";

	for (const std::string &s : strings)
	{
		const std::vector<std::string> tokens = utils::split(s, separator);
		if(tokens.size() != 2) {
			throw bad_commandline_tuple(s, expected_format);
		}

		unsigned int temp;
		try {
			temp = lexical_cast<unsigned int>(tokens[0]);
		} catch (bad_lexical_cast &) {
			throw bad_commandline_tuple(s, expected_format);
		}

		vec.emplace_back(temp, tokens[1]);
	}
	return vec;
}

std::vector<std::tuple<unsigned int,std::string,std::string> > commandline_options::parse_to_uint_string_string_tuples_(const std::vector<std::string> &strings, char separator)
{
	std::vector<std::tuple<unsigned int,std::string,std::string> > vec;
	const std::string& expected_format
			= std::string() + "UINT" + separator + "STRING" + separator + "STRING";

	for (const std::string &s : strings)
	{
		const std::vector<std::string> tokens = utils::split(s, separator);
		if(tokens.size() != 3) {
			throw bad_commandline_tuple(s, expected_format);
		}

		unsigned int temp;
		try {
			temp = lexical_cast<unsigned int>(tokens[0]);
		} catch (bad_lexical_cast &) {
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
	if (server) {
		ret["server"] = *server;
	}
	if (username) {
		ret["username"] = *username;
	}
	if (password) {
		ret["password"] = *password;
	}
	return ret;
}
