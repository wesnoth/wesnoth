/* $Id$ */
/*
   Copyright (C) 2011 by Lukasz Dobrogowski <lukasz.dobrogowski@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "commandline_options.hpp"
#include "foreach.hpp"
#include "serialization/string_utils.hpp"
#include "util.hpp"

namespace po = boost::program_options;

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
		("data-dir", po::value<std::string>(), "overrides the data directory with the one specified.")
		("debug,d", "enables additional command mode options in-game.")
		("help,h", "prints this message and exits.")
		("load,l", po::value<std::string>(), "loads the save <arg> from the standard save game directory.\nWhen launching the map editor via -e, the map <arg> is loaded, relative to the current directory. If it is a directory, the editor will start with a load map dialog opened there.")
		("new-syntax", "enables the new campaign syntax parsing.")
		("nocache", "disables caching of game data.")
		("path", "prints the path to the data directory and exits.")
		("validcache", "assumes that the cache is valid. (dangerous)")
		("version,v", "prints the game's version number and exits.")
		("with-replay", "replays the file loaded with the --load option.")
		;
	po::options_description display_opts("Display options");
	display_opts.add_options()
		("bpp", po::value<int>(), "sets BitsPerPixel value. Example: --bpp 32")
		("fps", "displays the number of frames per second the game is currently running at, in a corner of the screen.")
		("max-fps", "the maximum fps the game tries to run at. Values should be between 1 and 1000, the default is 50.")
		;

	po::options_description multiplayer_opts("Multiplayer options");
	multiplayer_opts.add_options()
		("multiplayer,m", "Starts a multiplayer game. There are additional options that can be used as explained below:")
		("ai-config", po::value<std::vector<std::string> >()->composing(), "arg should have format side:value\nselects a configuration file to load for this side.")
		;

	hidden_.add_options()
		("new_storyscreens", "")
		("new-widgets", "")
		;
	visible_.add(general_opts).add(display_opts).add(multiplayer_opts);
	
	all_.add(visible_).add(hidden_);

	po::variables_map vm;
	po::store(po::parse_command_line(argc_,argv_,all_),vm);

	if (vm.count("ai-config"))
		multiplayer_ai_config = parse_to_int_string_tuples_(vm["ai-config"].as<std::vector<std::string> >());

	if (vm.count("bpp"))
		bpp = vm["bpp"].as<int>();
	if (vm.count("config-dir"))
		config_dir = vm["config-dir"].as<std::string>();
	if (vm.count("data-dir"))
		data_dir = vm["data-dir"].as<std::string>();
	if (vm.count("debug"))
		debug = true;
	if (vm.count("fps"))
		fps = true;
	if (vm.count("help"))
		help = true;
	if (vm.count("load"))
		load = vm["load"].as<std::string>();
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
	if (vm.count("path"))
		path = true;
	if (vm.count("validcache"))
		validcache = true;
	if (vm.count("version"))
		version = true;
	if (vm.count("with-replay"))
		with_replay = true;
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
			 //TODO throw meaningful exception
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
