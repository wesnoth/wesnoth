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
	po::options_description general("General options");
	general.add_options()
		("data-dir", po::value<std::string>(), "overrides the data directory with the one specified.")
		("new-syntax", "enables the new campaign syntax parsing.")
		("help,h", "prints this message and exits.")
		;
	
	hidden_.add_options()
		("new-widgets", "")
		("new_storyscreens", "")
		;
	
	visible_.add(general);
	
	all_.add(visible_).add(hidden_);
	
	po::variables_map vm;
	po::store(po::parse_command_line(argc_,argv_,all_),vm);

	if (vm.count("help"))
		help = true;
	if (vm.count("new-widgets"))
		new_widgets = true;
	if (vm.count("new-storyscreens"))
		new_storyscreens = true;
}

std::ostream& operator<<(std::ostream &os, const commandline_options& cmdline_opts)
{
	os << "Usage:\n";
	os << cmdline_opts.visible_;
	return os;
}
