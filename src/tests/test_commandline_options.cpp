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
#define GETTEXT_DOMAIN "wesnoth-test"

#include "commandline_options.hpp"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE( cmdline_opts )

BOOST_AUTO_TEST_CASE (test_empty_options)
{
	int argc = 1;
	const char *argv[] = {"wesnoth"};
	commandline_options co(argc,const_cast<char**>(argv));

	BOOST_CHECK(!co.bpp);
	BOOST_CHECK(!co.campaign);
	BOOST_CHECK(!co.campaign_difficulty);
	BOOST_CHECK(!co.campaign_scenario);
	BOOST_CHECK(!co.clock);
	BOOST_CHECK(!co.config_path);
	BOOST_CHECK(!co.config_dir);
	BOOST_CHECK(!co.data_dir);
	BOOST_CHECK(!co.debug);
#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
	BOOST_CHECK(!co.debug_dot_level);
	BOOST_CHECK(!co.debug_dot_domain);
#endif
	BOOST_CHECK(!co.editor);
	BOOST_CHECK(!co.fps);
	BOOST_CHECK(!co.fullscreen);
	BOOST_CHECK(!co.gunzip);
	BOOST_CHECK(!co.gzip);
	BOOST_CHECK(!co.help);
	BOOST_CHECK(!co.log);
	BOOST_CHECK(!co.load);
	BOOST_CHECK(!co.logdomains);
	BOOST_CHECK(!co.multiplayer);
	BOOST_CHECK(!co.multiplayer_ai_config);
	BOOST_CHECK(!co.multiplayer_algorithm);
	BOOST_CHECK(!co.multiplayer_controller);
	BOOST_CHECK(!co.multiplayer_era);
	BOOST_CHECK(!co.multiplayer_label);
	BOOST_CHECK(!co.multiplayer_parm);
	BOOST_CHECK(!co.multiplayer_side);
	BOOST_CHECK(!co.multiplayer_turns);
	BOOST_CHECK(!co.max_fps);
	BOOST_CHECK(!co.nocache);
	BOOST_CHECK(!co.nodelay);
	BOOST_CHECK(!co.nogui);
	BOOST_CHECK(!co.nomusic);
	BOOST_CHECK(!co.nosound);
	BOOST_CHECK(!co.new_storyscreens);
	BOOST_CHECK(!co.new_syntax);
	BOOST_CHECK(!co.new_widgets);
	BOOST_CHECK(!co.path);
	BOOST_CHECK(!co.preprocess);
	BOOST_CHECK(!co.preprocess_defines);
	BOOST_CHECK(!co.preprocess_input_macros);
	BOOST_CHECK(!co.preprocess_output_macros);
	BOOST_CHECK(!co.preprocess_path);
	BOOST_CHECK(!co.preprocess_target);
	BOOST_CHECK(!co.proxy);
	BOOST_CHECK(!co.proxy_address);
	BOOST_CHECK(!co.proxy_password);
	BOOST_CHECK(!co.proxy_port);
	BOOST_CHECK(!co.proxy_user);
	BOOST_CHECK(!co.resolution);
	BOOST_CHECK(!co.rng_seed);
	BOOST_CHECK(!co.server);
	BOOST_CHECK(!co.screenshot);
	BOOST_CHECK(!co.screenshot_map_file);
	BOOST_CHECK(!co.screenshot_output_file);
	BOOST_CHECK(!co.smallgui);
	BOOST_CHECK(!co.test);
	BOOST_CHECK(!co.validcache);
	BOOST_CHECK(!co.version);
	BOOST_CHECK(!co.windowed);
	BOOST_CHECK(!co.with_replay);

}

BOOST_AUTO_TEST_SUITE_END()
