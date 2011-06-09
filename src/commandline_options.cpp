/* $Id$ */
/*
   Copyright (C) 2011 by Lukasz Dobrogowski <lukasz.dobrogowski@gmail.com>
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
#include "foreach.hpp"
#include "serialization/string_utils.hpp"
#include "util.hpp"
#include "lua/llimits.h"

namespace po = boost::program_options;

class two_strings : public boost::tuple<std::string,std::string> {};

void validate(boost::any& v, const std::vector<std::string>& values,
              two_strings*, int)
{
    two_strings ret_val;
	if (values.size() != 2)
		throw po::validation_error(po::validation_error::invalid_option_value);
    ret_val.get<0>() = values.at(0);
    ret_val.get<1>() = values.at(1);
    v = ret_val;
}

commandline_options::commandline_options ( int argc, char** argv ) :
	bpp(),
	campaign(),
	campaign_difficulty(),
	campaign_scenario(),
	clock(false),
	config_path(false),
	config_dir(),
	data_dir(),
	debug(false),
#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
	debug_dot_level(),
	debug_dot_domain(),
#endif
	editor(),
	fps(false),
	fullscreen(false),
	gunzip(),
	gzip(),
	help(),
	log(),
	load(),
	logdomains(),
	multiplayer(false),
	multiplayer_ai_config(),
	multiplayer_algorithm(),
	multiplayer_controller(),
	multiplayer_era(),
	multiplayer_label(),
	multiplayer_parm(),
	multiplayer_side(),
	multiplayer_turns(),
	max_fps(),
	nocache(false),
	nodelay(false),
	nogui(false),
	nomusic(false),
	nosound(false),
	new_storyscreens(false),
	new_syntax(false),
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
	screenshot(false),
	screenshot_map_file(),
	screenshot_output_file(),
	smallgui(false),
	test(false),
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
		("config-dir", po::value<std::string>(), "sets the path of the user config directory to $HOME/<arg> or My Documents\\My Games\\<arg> for Windows. You can specify also an absolute path outside the $HOME or My Documents\\My Games directory.")
		("config-path", "prints the path of the user config directory and exits.")
		("data-dir", po::value<std::string>(), "overrides the data directory with the one specified.")
		("debug,d", "enables additional command mode options in-game.")
		("gunzip", po::value<std::string>(), "decompresses a file (<arg>.gz) in gzip format and stores it without the .gz suffix. <arg>.gz will be removed.")
		("gzip", po::value<std::string>(), "compresses a file (<arg>) in gzip format, stores it as <arg>.gz and removes <arg>.")
		("help,h", "prints this message and exits.")
		("load,l", po::value<std::string>(), "loads the save <arg> from the standard save game directory. When launching the map editor via -e, the map <arg> is loaded, relative to the current directory. If it is a directory, the editor will start with a load map dialog opened there.")
		("new-syntax", "enables the new campaign syntax parsing.")
		("nocache", "disables caching of game data.")
		("path", "prints the path to the data directory and exits.")
		("rng-seed", po::value<unsigned int>(), "seeds the random number generator with number <arg>. Example: --rng-seed 0")
		("screenshot", po::value<two_strings>()->multitoken(), "takes two arguments: <map> <output>. Saves a screenshot of <map> to <output> without initializing a screen. Editor must be compiled in for this to work.")
		("validcache", "assumes that the cache is valid. (dangerous)")
		("version,v", "prints the game's version number and exits.")
		("with-replay", "replays the file loaded with the --load option.")
		;
	po::options_description display_opts("Display options");
	display_opts.add_options()
		("bpp", po::value<int>(), "sets BitsPerPixel value. Example: --bpp 32")
		("fps", "displays the number of frames per second the game is currently running at, in a corner of the screen.")
		("max-fps", po::value<int>(), "the maximum fps the game tries to run at. Values should be between 1 and 1000, the default is 50.")
		("resolution,r", po::value<std::string>(), "sets the screen resolution. <arg> should have format XxY. Example: --resolution 800x600")
		;

	po::options_description logging_opts("Logging options");
	logging_opts.add_options()
		("logdomains", po::value<std::string>()->implicit_value(std::string()), "lists defined log domains (only the ones containing <arg> filter if such is provided) and exits.")
		("log-error", po::value<std::string>(), "sets the severity level of the specified log domain(s) to 'error'. <arg> should be given as comma separated list of domains, wildcards are allowed. Example: --log-error=network,gui/*,engine/enemies")
		("log-warning", po::value<std::string>(), "sets the severity level of the specified log domain(s) to 'warning'. Similar to --log-error.")
		("log-info", po::value<std::string>(), "sets the severity level of the specified log domain(s) to 'info'. Similar to --log-error.")
		("log-debug", po::value<std::string>(), "sets the severity level of the specified log domain(s) to 'debug'. Similar to --log-error.")
		;

	po::options_description multiplayer_opts("Multiplayer options");
	multiplayer_opts.add_options()
		("multiplayer,m", "Starts a multiplayer game. There are additional options that can be used as explained below:")
		("ai-config", po::value<std::vector<std::string> >()->composing(), "arg should have format side:value\nselects a configuration file to load for this side.")
		("nogui", "runs the game without the GUI.")
		;

	po::options_description preprocessor_opts("Preprocessor mode options");
	preprocessor_opts.add_options()
		("preprocess,p", po::value<two_strings>()->multitoken(), "requires two arguments: <file/folder> <target directory>. Preprocesses a specified file/folder. The preprocessed file(s) will be written in the specified target directory: a plain cfg file and a processed cfg file.")
		("preprocess-defines", po::value<std::string>(), "comma separated list of defines to be used by '--preprocess' command. If 'SKIP_CORE' is in the define list the data/core won't be preprocessed. Example: --preprocess-defines=FOO,BAR")
		("preprocess-input-macros", po::value<std::string>(), "used only by the '--preprocess' command. Specifies source file <arg> that contains [preproc_define]s to be included before preprocessing.")
		("preprocess-output-macros", po::value<std::string>()->implicit_value(std::string()), "used only by the '--preprocess' command. Will output all preprocessed macros in the target file <arg>. If the file is not specified the output will be file '_MACROS_.cfg' in the target directory of preprocess's command.")
		;

	hidden_.add_options()
		("new-storyscreens", "")
		("new-widgets", "")
		;
	visible_.add(general_opts).add(display_opts).add(logging_opts).add(multiplayer_opts).add(preprocessor_opts);
	
	all_.add(visible_).add(hidden_);

	po::variables_map vm;
	const int parsing_style = po::command_line_style::default_style ^ po::command_line_style::allow_guessing;
	po::store(po::parse_command_line(argc_,argv_,all_,parsing_style),vm);

	if (vm.count("ai-config"))
		multiplayer_ai_config = parse_to_int_string_tuples_(vm["ai-config"].as<std::vector<std::string> >());
	if (vm.count("bpp"))
		bpp = vm["bpp"].as<int>();
	if (vm.count("config-dir"))
		config_dir = vm["config-dir"].as<std::string>();
	if (vm.count("config-path"))
		config_path = true;
	if (vm.count("data-dir"))
		data_dir = vm["data-dir"].as<std::string>();
	if (vm.count("debug"))
		debug = true;
	if (vm.count("fps"))
		fps = true;
	if (vm.count("gunzip"))
		gunzip = vm["gunzip"].as<std::string>();
	if (vm.count("gzip"))
		gzip = vm["gzip"].as<std::string>();
	if (vm.count("help"))
		help = true;
	if (vm.count("load"))
		load = vm["load"].as<std::string>();
	if (vm.count("log-error"))
		 parse_log_domains_(vm["log-error"].as<std::string>(),0);
	if (vm.count("log-warning"))
		 parse_log_domains_(vm["log-warning"].as<std::string>(),1);
	if (vm.count("log-info"))
		 parse_log_domains_(vm["log-info"].as<std::string>(),2);
	if (vm.count("log-debug"))
		 parse_log_domains_(vm["log-debug"].as<std::string>(),3);
	if (vm.count("logdomains"))
		logdomains = vm["logdomains"].as<std::string>();
	if (vm.count("max-fps"))
		max_fps = vm["max-fps"].as<int>();
	if (vm.count("multiplayer"))
		multiplayer = true;
	if (vm.count("new-storyscreens"))
		new_storyscreens = true;
	if (vm.count("new-syntax"))
		new_syntax = true;
	if (vm.count("new-widgets"))
		new_widgets = true;
	if (vm.count("nocache"))
		nocache = true;
	if (vm.count("nogui"))
		nogui = true;
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
	if (vm.count("resolution"))
		parse_resolution_(vm["resolution"].as<std::string>());
	if (vm.count("rng-seed"))
		rng_seed = vm["rng-seed"].as<unsigned int>();
	if (vm.count("screenshot"))
	{
		screenshot = true;
		screenshot_map_file = vm["screenshot"].as<two_strings>().get<0>();
		screenshot_output_file = vm["screenshot"].as<two_strings>().get<1>();
	}
	if (vm.count("validcache"))
		validcache = true;
	if (vm.count("version"))
		version = true;
	if (vm.count("with-replay"))
		with_replay = true;
}

void commandline_options::parse_log_domains_(const std::string &domains_string, const int severity)
{
	const std::vector<std::string> domains = utils::split(domains_string, ',');
	foreach (const std::string& domain, domains)
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

std::vector<boost::tuple<int,std::string> > commandline_options::parse_to_int_string_tuples_(const std::vector<std::string> &strings)
{
	std::vector<boost::tuple<int,std::string> > vec;
	boost::tuple<int,std::string> elem;
	foreach(const std::string &s, strings)
	{
		const std::vector<std::string> tokens = utils::split(s, ':');
		if (tokens.size()!=2)
		{
			 //TODO throw a meaningful exception
		}
		elem.get<0>() = lexical_cast<int>(tokens[0]);
			//TODO catch exception and pack in meaningful something
		elem.get<1>() = tokens[1];
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
