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

commandline_options::commandline_options ( int /* argc*/, char** /*argv*/ ) :
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
	with_replay(false)
{

}
