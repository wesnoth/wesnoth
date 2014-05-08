/*
   Copyright (C) 2011 - 2014 by Lukasz Dobrogowski <lukasz.dobrogowski@gmail.com>
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
#include "serialization/string_utils.hpp"
#include "util.hpp"
#include "lua/llimits.h"
#include "log.hpp"

#include <boost/version.hpp>
#include <boost/foreach.hpp>

namespace po = boost::program_options;

// this class is needed since boost has some templated operators>> declared internally for tuples and we don't want them to interfere. Existence of such operator>> apparently causes program_options to cause the custom class somehow specially... well, the boost::tuple default operator>>  format doesn't suit our needs anyway.
class two_strings : public boost::tuple<std::string,std::string> {};

static void validate(boost::any& v, const std::vector<std::string>& values,
              two_strings*, int)
{
    two_strings ret_val;
	if (values.size() != 2)
#if BOOST_VERSION >= 104200
		throw po::validation_error(po::validation_error::invalid_option_value);
#else
		throw po::validation_error("Invalid number of strings provided to option requiring exactly two of them.");
#endif
    ret_val.get<0>() = values.at(0);
    ret_val.get<1>() = values.at(1);
    v = ret_val;
}

commandline_options::commandline_options ( int argc, char** argv ) :
	bpp(),
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
	proxy(false),
	proxy_address(),
	proxy_password(),
	proxy_port(),
	proxy_user(),
	resolution(),
	rng_seed(),
	server(),
	username(),
	password(),
	screenshot(false),
	screenshot_map_file(),
	screenshot_output_file(),
	strict_validation(false),
	test(),
	unit_test(),
	headless_unit_test(false),
	noreplaycheck(false),
	userconfig_path(false),
	userconfig_dir(),
	userdata_path(false),
	userdata_dir(),
	validcache(false),
	version(false),
	windowed(false),
	with_replay(false),
	argc_(argc),
	argv_(argv),
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
		("nocache", "disables caching of game data.")
		("nodelay", "runs the game without any delays.")
		("nomusic", "runs the game without music.")
		("nosound", "runs the game without sounds and music.")
		("path", "prints the path to the data directory and exits.")
		("rng-seed", po::value<unsigned int>(), "seeds the random number generator with number <arg>. Example: --rng-seed 0")
		("screenshot", po::value<two_strings>()->multitoken(), "takes two arguments: <map> <output>. Saves a screenshot of <map> to <output> without initializing a screen. Editor must be compiled in for this to work.")
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
		;

	po::options_description campaign_opts("Campaign options");
	campaign_opts.add_options()
		("campaign,c", po::value<std::string>()->implicit_value(std::string()), "goes directly to the campaign with id <arg>. A selection menu will appear if no id was specified.")
		("campaign-difficulty", po::value<int>(), "The difficulty of the specified campaign (1 to max). If none specified, the campaign difficulty selection widget will appear.")
		("campaign-scenario", po::value<std::string>(),"The id of the scenario from the specified campaign. The default is the first scenario.")
		;

	po::options_description display_opts("Display options");
	display_opts.add_options()
		("bpp", po::value<int>(), "sets BitsPerPixel value. Example: --bpp 32")
		("fps", "displays the number of frames per second the game is currently running at, in a corner of the screen.")
		("fullscreen,f", "runs the game in full screen mode.")
		("max-fps", po::value<int>(), "the maximum fps the game tries to run at. Values should be between 1 and 1000, the default is 50.")
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
		("log-precise", "shows the timestamps in the logfile with more precision")
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
		("timeout", po::value<unsigned int>(), "sets a timeout (milliseconds) for the unit test. If unused there is no timeout or threading.")
		("noreplaycheck", "don't try to validate replay of unit test")
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
	po::store(po::command_line_parser(argc_,argv_).options(all_).positional(positional).style(parsing_style).run(),vm);

	if (vm.count("ai-config"))
		multiplayer_ai_config = parse_to_uint_string_tuples_(vm["ai-config"].as<std::vector<std::string> >());
	if (vm.count("algorithm"))
		multiplayer_algorithm = parse_to_uint_string_tuples_(vm["algorithm"].as<std::vector<std::string> >());
	if (vm.count("bpp"))
		bpp = vm["bpp"].as<int>();
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
		 parse_log_domains_(vm["log-error"].as<std::string>(),lg::err.get_severity());
	if (vm.count("log-warning"))
		 parse_log_domains_(vm["log-warning"].as<std::string>(),lg::warn.get_severity());
	if (vm.count("log-info"))
		 parse_log_domains_(vm["log-info"].as<std::string>(),lg::info.get_severity());
	if (vm.count("log-debug"))
		 parse_log_domains_(vm["log-debug"].as<std::string>(),lg::debug.get_severity());
	if (vm.count("logdomains"))
		logdomains = vm["logdomains"].as<std::string>();
	if (vm.count("log-precise"))
		log_precise_timestamps = true;
	if (vm.count("max-fps"))
		max_fps = vm["max-fps"].as<int>();
	if (vm.count("multiplayer"))
		multiplayer = true;
	if (vm.count("multiplayer-repeat"))
		multiplayer_repeat = vm["multiplayer-repeat"].as<unsigned int>();
	if (vm.count("new-widgets"))
		new_widgets = true;
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
		preprocess_path = vm["preprocess"].as<two_strings>().get<0>();
		preprocess_target = vm["preprocess"].as<two_strings>().get<1>();
	}
	if (vm.count("preprocess-defines"))
		preprocess_defines = utils::split(vm["preprocess-defines"].as<std::string>(), ',');
	if (vm.count("preprocess-input-macros"))
		preprocess_input_macros = vm["preprocess-input-macros"].as<std::string>();
	if (vm.count("preprocess-output-macros"))
		preprocess_output_macros = vm["preprocess-output-macros"].as<std::string>();
	if (vm.count("proxy"))
		proxy = true;
	if (vm.count("proxy-address"))
		proxy_address = vm["proxy-address"].as<std::string>();
	if (vm.count("proxy-password"))
		proxy_password = vm["proxy-password"].as<std::string>();
	if (vm.count("proxy-port"))
		proxy_port = vm["proxy-port"].as<std::string>();
	if (vm.count("proxy-user"))
		proxy_user = vm["proxy-user"].as<std::string>();
	if (vm.count("resolution"))
		parse_resolution_(vm["resolution"].as<std::string>());
	if (vm.count("rng-seed"))
		rng_seed = vm["rng-seed"].as<unsigned int>();
	if (vm.count("scenario"))
		multiplayer_scenario = vm["scenario"].as<std::string>();
	if (vm.count("screenshot"))
	{
		screenshot = true;
		screenshot_map_file = vm["screenshot"].as<two_strings>().get<0>();
		screenshot_output_file = vm["screenshot"].as<two_strings>().get<1>();
	}
	if (vm.count("server"))
		server = vm["server"].as<std::string>();
	if (vm.count("username"))
		username = vm["username"].as<std::string>();
	if (vm.count("password"))
		password = vm["password"].as<std::string>();
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
	BOOST_FOREACH(const std::string& domain, domains)
	{
		if (!log)
			log = std::vector<boost::tuple<int, std::string> >();
		log->push_back(boost::tuple<int, std::string>(severity,domain));
	}
}

void commandline_options::parse_resolution_ ( const std::string& resolution_string )
{
	const std::vector<std::string> tokens = utils::split(resolution_string, 'x');
	if (tokens.size() != 2)
		{} // TODO throw a meaningful exception
	int xres = lexical_cast<int>(tokens[0]);
	int yres = lexical_cast<int>(tokens[1]);
	resolution = boost::tuple<int,int>(xres,yres);
}

std::vector<boost::tuple<unsigned int,std::string> > commandline_options::parse_to_uint_string_tuples_(const std::vector<std::string> &strings, char separator)
{
	std::vector<boost::tuple<unsigned int,std::string> > vec;
	boost::tuple<unsigned int,std::string> elem;
	BOOST_FOREACH(const std::string &s, strings)
	{
		const std::vector<std::string> tokens = utils::split(s, separator);
		if (tokens.size()!=2)
		{
			 //TODO throw a meaningful exception
		}
		elem.get<0>() = lexical_cast<unsigned int>(tokens[0]);
			//TODO catch exception and pack in meaningful something
		elem.get<1>() = tokens[1];
		vec.push_back(elem);
	}
	return vec;
}

std::vector<boost::tuple<unsigned int,std::string,std::string> > commandline_options::parse_to_uint_string_string_tuples_(const std::vector<std::string> &strings, char separator)
{
	std::vector<boost::tuple<unsigned int,std::string,std::string> > vec;
	boost::tuple<unsigned int,std::string,std::string> elem;
	BOOST_FOREACH(const std::string &s, strings)
	{
		const std::vector<std::string> tokens = utils::split(s, separator);
		if (tokens.size()!=3)
		{
			 //TODO throw a meaningful exception
		}
		elem.get<0>() = lexical_cast<unsigned int>(tokens[0]);
			//TODO catch exception and pack in meaningful something
		elem.get<1>() = tokens[1];
		elem.get<2>() = tokens[2];
		vec.push_back(elem);
	}
	return vec;
}

std::ostream& operator<<(std::ostream &os, const commandline_options& cmdline_opts)
{
	os << "Usage: " << cmdline_opts.argv_[0] << " [<options>] [<data-directory>]\n";
	os << cmdline_opts.visible_;
	return os;
}
