/*
   Copyright (C) 2011 - 2016 by Lukasz Dobrogowski <lukasz.dobrogowski@gmail.com>
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
#include <boost/assign.hpp>

BOOST_AUTO_TEST_SUITE( cmdline_opts )

BOOST_AUTO_TEST_CASE (test_empty_options)
{

	std::vector<std::string> args = {"wesnoth"};
	commandline_options co(args);

	BOOST_CHECK(!co.campaign);
	BOOST_CHECK(!co.campaign_difficulty);
	BOOST_CHECK(!co.campaign_scenario);
	BOOST_CHECK(!co.clock);
	BOOST_CHECK(!co.data_dir);
	BOOST_CHECK(!co.data_path);
	BOOST_CHECK(!co.debug);
#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
	BOOST_CHECK(!co.debug_dot_domain);
	BOOST_CHECK(!co.debug_dot_level);
#endif
	BOOST_CHECK(!co.editor);
	BOOST_CHECK(!co.fps);
	BOOST_CHECK(!co.fullscreen);
	BOOST_CHECK(!co.gunzip);
	BOOST_CHECK(!co.gzip);
	BOOST_CHECK(!co.help);
	BOOST_CHECK(!co.load);
	BOOST_CHECK(!co.log);
	BOOST_CHECK(!co.logdomains);
	BOOST_CHECK(!co.multiplayer);
	BOOST_CHECK(!co.multiplayer_ai_config);
	BOOST_CHECK(!co.multiplayer_algorithm);
	BOOST_CHECK(!co.multiplayer_controller);
	BOOST_CHECK(!co.multiplayer_era);
	BOOST_CHECK(!co.multiplayer_exit_at_end);
	BOOST_CHECK(!co.multiplayer_ignore_map_settings);
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
	BOOST_CHECK(!co.multiplayer_scenario);
	BOOST_CHECK(!co.server);
	BOOST_CHECK(!co.screenshot);
	BOOST_CHECK(!co.screenshot_map_file);
	BOOST_CHECK(!co.screenshot_output_file);
	BOOST_CHECK(!co.test);
	BOOST_CHECK(!co.userconfig_dir);
	BOOST_CHECK(!co.userconfig_path);
	BOOST_CHECK(!co.userdata_dir);
	BOOST_CHECK(!co.userdata_path);
	BOOST_CHECK(!co.validcache);
	BOOST_CHECK(!co.version);
	BOOST_CHECK(!co.windowed);
	BOOST_CHECK(!co.with_replay);
}

BOOST_AUTO_TEST_CASE (test_default_options)
{
	std::vector<std::string> args = {
		"wesnoth",
		"--campaign",
		"--editor",
		"--logdomains",
		"--preprocess-output-macros",
		"--server",
		"--test"};

	commandline_options co(args);
	BOOST_CHECK(co.campaign && co.campaign->empty());
	BOOST_CHECK(!co.campaign_difficulty);
	BOOST_CHECK(!co.campaign_scenario);
	BOOST_CHECK(!co.clock);
	BOOST_CHECK(!co.data_dir);
	BOOST_CHECK(!co.data_path);
	BOOST_CHECK(!co.debug);
#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
	BOOST_CHECK(!co.debug_dot_domain);
	BOOST_CHECK(!co.debug_dot_level);
#endif
	BOOST_CHECK(co.editor && co.editor->empty());
	BOOST_CHECK(!co.fps);
	BOOST_CHECK(!co.fullscreen);
	BOOST_CHECK(!co.gunzip);
	BOOST_CHECK(!co.gzip);
	BOOST_CHECK(!co.help);
	BOOST_CHECK(!co.load);
	BOOST_CHECK(!co.log);
	BOOST_CHECK(co.logdomains && co.logdomains->empty());
	BOOST_CHECK(!co.multiplayer);
	BOOST_CHECK(!co.multiplayer_ai_config);
	BOOST_CHECK(!co.multiplayer_algorithm);
	BOOST_CHECK(!co.multiplayer_controller);
	BOOST_CHECK(!co.multiplayer_era);
	BOOST_CHECK(!co.multiplayer_exit_at_end);
	BOOST_CHECK(!co.multiplayer_ignore_map_settings);
	BOOST_CHECK(!co.multiplayer_label);
	BOOST_CHECK(!co.multiplayer_parm);
	BOOST_CHECK(!co.multiplayer_scenario);
	BOOST_CHECK(!co.multiplayer_side);
	BOOST_CHECK(!co.multiplayer_turns);
	BOOST_CHECK(!co.max_fps);
	BOOST_CHECK(!co.nocache);
	BOOST_CHECK(!co.nodelay);
	BOOST_CHECK(!co.nogui);
	BOOST_CHECK(!co.nomusic);
	BOOST_CHECK(!co.nosound);
	BOOST_CHECK(!co.new_widgets);
	BOOST_CHECK(!co.path);
	BOOST_CHECK(!co.preprocess);
	BOOST_CHECK(!co.preprocess_defines);
	BOOST_CHECK(!co.preprocess_input_macros);
	BOOST_CHECK(co.preprocess_output_macros && co.preprocess_output_macros->empty());
	BOOST_CHECK(!co.preprocess_path);
	BOOST_CHECK(!co.preprocess_target);
	BOOST_CHECK(!co.proxy);
	BOOST_CHECK(!co.proxy_address);
	BOOST_CHECK(!co.proxy_password);
	BOOST_CHECK(!co.proxy_port);
	BOOST_CHECK(!co.proxy_user);
	BOOST_CHECK(!co.resolution);
	BOOST_CHECK(!co.rng_seed);
	BOOST_CHECK(co.server && co.server->empty());
	BOOST_CHECK(!co.screenshot);
	BOOST_CHECK(!co.screenshot_map_file);
	BOOST_CHECK(!co.screenshot_output_file);
	BOOST_CHECK(co.test && co.test->empty());
	BOOST_CHECK(!co.userconfig_dir);
	BOOST_CHECK(!co.userconfig_path);
	BOOST_CHECK(!co.userdata_dir);
	BOOST_CHECK(!co.userdata_path);
	BOOST_CHECK(!co.validcache);
	BOOST_CHECK(!co.version);
	BOOST_CHECK(!co.windowed);
	BOOST_CHECK(!co.with_replay);
}

BOOST_AUTO_TEST_CASE (test_full_options)
{
	std::vector<std::string> args = {
		"wesnoth",
		"--ai-config=1:aifoo",
		"--ai-config=2:aibar",
		"--algorithm=3:algfoo",
		"--algorithm=4:algbar",
		"--campaign=campfoo",
		"--campaign-difficulty=16",
		"--campaign-scenario=scenfoo",
		"--clock",
		"--controller=5:confoo",
		"--controller=6:conbar",
		"--data-dir=datadirfoo",
		"--data-path",
		"--debug",
#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
		"--debug-dot-domain=ddfoo",
		"--debug-dot-level=dlfoo",
#endif
		"--editor=editfoo",
		"--era=erafoo",
		"--exit-at-end",
		"--fps",
		"--fullscreen",
		"--gunzip=gunzipfoo.gz",
		"--gzip=gzipfoo",
		"--help",
		"--ignore-map-settings",
		"--label=labelfoo",
		"--load=loadfoo",
		"--log-error=errfoo,errbar/*",
		"--log-warning=warnfoo,warnfoo/bar",
		"--log-info=infofoo",
		"--log-debug=dbgfoo,dbgbar,dbg/foo/bar/baz",
		"--logdomains=filterfoo",
		"--max-fps=100",
		"--multiplayer",
		"--new-widgets",
		"--nocache",
		"--nodelay",
		"--nomusic",
		"--nosound",
		"--nogui",
		"--parm=7:parmfoo:valfoo",
		"--parm=8:parmbar:valbar",
		"--path",
		"--preprocess", "preppathfoo", "preptargfoo",
		"--preprocess-defines=DEFFOO,DEFBAR",
		"--preprocess-input-macros=inmfoo",
		"--preprocess-output-macros=outmfoo",
		"--proxy",
		"--proxy-address=addressfoo",
		"--proxy-password=passfoo",
		"--proxy-port=portfoo",
		"--proxy-user=userfoo",
		"--resolution=800x600",
		"--rng-seed=1234",
		"--scenario=scenfoo",
		"--screenshot", "mapfoo", "outssfoo",
		"--side=9:sidefoo",
		"--side=10:sidebar",
		"--server=servfoo",
		"--test=testfoo",
		"--turns=42",
		"--userconfig-dir=userconfigdirfoo",
		"--userconfig-path",
		"--userdata-dir=userdatadirfoo",
		"--userdata-path",
		"--validcache",
		"--version",
		"--windowed",
		"--with-replay"};

	commandline_options co(args);

	BOOST_CHECK(co.campaign && *co.campaign == "campfoo");
	BOOST_CHECK(co.campaign_difficulty && *co.campaign_difficulty == 16);
	BOOST_CHECK(co.campaign_scenario && *co.campaign_scenario == "scenfoo");
	BOOST_CHECK(co.clock);
	BOOST_CHECK(co.data_dir && *co.data_dir == "datadirfoo");
	BOOST_CHECK(co.data_path);
	BOOST_CHECK(co.debug);
#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
	BOOST_CHECK(co.debug_dot_domain && *co.debug_dot_domain == "ddfoo");
	BOOST_CHECK(co.debug_dot_level && *co.debug_dot_level == "dlfoo");
#endif
	BOOST_CHECK(co.editor && *co.editor == "editfoo");
	BOOST_CHECK(co.fps);
	BOOST_CHECK(co.fullscreen);
	BOOST_CHECK(co.gunzip && *co.gunzip == "gunzipfoo.gz");
	BOOST_CHECK(co.gzip && *co.gzip == "gzipfoo");
	BOOST_CHECK(co.help);
	BOOST_CHECK(co.load && *co.load == "loadfoo");
	BOOST_CHECK(co.log);
	BOOST_CHECK(co.log->size()==8);
	BOOST_CHECK(co.log->at(0).get<0>() == 0 && co.log->at(1).get<0>() == 0);
	BOOST_CHECK(co.log->at(0).get<1>() == "errfoo" && co.log->at(1).get<1>() == "errbar/*");
	BOOST_CHECK(co.log->at(2).get<0>() == 1 && co.log->at(3).get<0>() == 1);
	BOOST_CHECK(co.log->at(2).get<1>() == "warnfoo" && co.log->at(3).get<1>() == "warnfoo/bar");
	BOOST_CHECK(co.log->at(4).get<0>() == 2);
	BOOST_CHECK(co.log->at(4).get<1>() == "infofoo");
	BOOST_CHECK(co.log->at(5).get<0>() == 3 && co.log->at(6).get<0>() == 3 && co.log->at(7).get<0>() == 3);
	BOOST_CHECK(co.log->at(5).get<1>() == "dbgfoo" && co.log->at(6).get<1>() == "dbgbar" && co.log->at(7).get<1>() == "dbg/foo/bar/baz");
	BOOST_CHECK(co.logdomains && *co.logdomains == "filterfoo");
	BOOST_CHECK(co.multiplayer);
	BOOST_CHECK(co.multiplayer_ai_config);
	BOOST_CHECK(co.multiplayer_ai_config->size() == 2);
	BOOST_CHECK(co.multiplayer_ai_config->at(0).get<0>() == 1 && co.multiplayer_ai_config->at(0).get<1>() == "aifoo");
	BOOST_CHECK(co.multiplayer_ai_config->at(1).get<0>() == 2 && co.multiplayer_ai_config->at(1).get<1>() == "aibar");
	BOOST_CHECK(co.multiplayer_algorithm);
	BOOST_CHECK(co.multiplayer_algorithm->at(0).get<0>() == 3 && co.multiplayer_algorithm->at(0).get<1>() == "algfoo");
	BOOST_CHECK(co.multiplayer_algorithm->at(1).get<0>() == 4 && co.multiplayer_algorithm->at(1).get<1>() == "algbar");
	BOOST_CHECK(co.multiplayer_controller);
	BOOST_CHECK(co.multiplayer_controller->at(0).get<0>() == 5 && co.multiplayer_controller->at(0).get<1>() == "confoo");
	BOOST_CHECK(co.multiplayer_controller->at(1).get<0>() == 6 && co.multiplayer_controller->at(1).get<1>() == "conbar");
	BOOST_CHECK(co.multiplayer_era && *co.multiplayer_era == "erafoo");
	BOOST_CHECK(co.multiplayer_exit_at_end);
	BOOST_CHECK(co.multiplayer_ignore_map_settings);
	BOOST_CHECK(co.multiplayer_label && *co.multiplayer_label == "labelfoo");
	BOOST_CHECK(co.multiplayer_parm);
	BOOST_CHECK(co.multiplayer_parm->at(0).get<0>() == 7 && co.multiplayer_parm->at(0).get<1>() == "parmfoo" && co.multiplayer_parm->at(0).get<2>() == "valfoo");
	BOOST_CHECK(co.multiplayer_parm->at(1).get<0>() == 8 && co.multiplayer_parm->at(1).get<1>() == "parmbar" && co.multiplayer_parm->at(1).get<2>() == "valbar");
	BOOST_CHECK(co.multiplayer_scenario && *co.multiplayer_scenario == "scenfoo");
	BOOST_CHECK(co.multiplayer_side);
	BOOST_CHECK(co.multiplayer_side->size() == 2);
	BOOST_CHECK(co.multiplayer_side->at(0).get<0>() == 9 && co.multiplayer_side->at(0).get<1>() == "sidefoo");
	BOOST_CHECK(co.multiplayer_side->at(1).get<0>() == 10 && co.multiplayer_side->at(1).get<1>() == "sidebar");
	BOOST_CHECK(co.multiplayer_turns && *co.multiplayer_turns == "42");
	BOOST_CHECK(co.max_fps && *co.max_fps == 100);
	BOOST_CHECK(co.nocache);
	BOOST_CHECK(co.nodelay);
	BOOST_CHECK(co.nogui);
	BOOST_CHECK(co.nomusic);
	BOOST_CHECK(co.nosound);
	BOOST_CHECK(co.new_widgets);
	BOOST_CHECK(co.path);
	BOOST_CHECK(co.preprocess && co.preprocess_path && co.preprocess_target);
	BOOST_CHECK(*co.preprocess_path == "preppathfoo" && *co.preprocess_target == "preptargfoo");
	BOOST_CHECK(co.preprocess_defines && co.preprocess_defines->size() == 2);
	BOOST_CHECK(co.preprocess_defines->at(0) == "DEFFOO" && co.preprocess_defines->at(1) == "DEFBAR");
	BOOST_CHECK(co.preprocess_input_macros && *co.preprocess_input_macros == "inmfoo");
	BOOST_CHECK(co.preprocess_output_macros && *co.preprocess_output_macros == "outmfoo");
	BOOST_CHECK(co.proxy);
	BOOST_CHECK(co.proxy_address && *co.proxy_address == "addressfoo");
	BOOST_CHECK(co.proxy_password && *co.proxy_password == "passfoo");
	BOOST_CHECK(co.proxy_port && *co.proxy_port == "portfoo");
	BOOST_CHECK(co.proxy_user && *co.proxy_user == "userfoo");
	BOOST_CHECK(co.resolution);
	BOOST_CHECK(co.resolution->get<0>() == 800 && co.resolution->get<1>() == 600);
	BOOST_CHECK(co.rng_seed && *co.rng_seed == 1234);
	BOOST_CHECK(co.server && *co.server == "servfoo");
	BOOST_CHECK(co.screenshot && co.screenshot_map_file && co.screenshot_output_file);
	BOOST_CHECK(*co.screenshot_map_file == "mapfoo" && *co.screenshot_output_file == "outssfoo");
	BOOST_CHECK(co.test && *co.test == "testfoo");
	BOOST_CHECK(co.userconfig_dir && *co.userconfig_dir == "userconfigdirfoo");
	BOOST_CHECK(co.userconfig_path);
	BOOST_CHECK(co.userdata_dir && *co.userdata_dir == "userdatadirfoo");
	BOOST_CHECK(co.userdata_path);
	BOOST_CHECK(co.validcache);
	BOOST_CHECK(co.version);
	BOOST_CHECK(co.windowed);
	BOOST_CHECK(co.with_replay);
}

BOOST_AUTO_TEST_CASE (test_positional_options)
{
	std::vector<std::string> args = {
		"wesnoth",
		"datadirfoo"};

	commandline_options co(args);

	BOOST_CHECK(!co.campaign);
	BOOST_CHECK(!co.campaign_difficulty);
	BOOST_CHECK(!co.campaign_scenario);
	BOOST_CHECK(!co.clock);
	BOOST_CHECK(co.data_dir && *co.data_dir == "datadirfoo");
	BOOST_CHECK(!co.data_path);
	BOOST_CHECK(!co.debug);
#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
	BOOST_CHECK(!co.debug_dot_domain);
	BOOST_CHECK(!co.debug_dot_level);
#endif
	BOOST_CHECK(!co.editor);
	BOOST_CHECK(!co.fps);
	BOOST_CHECK(!co.fullscreen);
	BOOST_CHECK(!co.gunzip);
	BOOST_CHECK(!co.gzip);
	BOOST_CHECK(!co.help);
	BOOST_CHECK(!co.load);
	BOOST_CHECK(!co.log);
	BOOST_CHECK(!co.logdomains);
	BOOST_CHECK(!co.multiplayer);
	BOOST_CHECK(!co.multiplayer_ai_config);
	BOOST_CHECK(!co.multiplayer_algorithm);
	BOOST_CHECK(!co.multiplayer_controller);
	BOOST_CHECK(!co.multiplayer_era);
	BOOST_CHECK(!co.multiplayer_exit_at_end);
	BOOST_CHECK(!co.multiplayer_ignore_map_settings);
	BOOST_CHECK(!co.multiplayer_label);
	BOOST_CHECK(!co.multiplayer_parm);
	BOOST_CHECK(!co.multiplayer_scenario);
	BOOST_CHECK(!co.multiplayer_side);
	BOOST_CHECK(!co.multiplayer_turns);
	BOOST_CHECK(!co.max_fps);
	BOOST_CHECK(!co.nocache);
	BOOST_CHECK(!co.nodelay);
	BOOST_CHECK(!co.nogui);
	BOOST_CHECK(!co.nomusic);
	BOOST_CHECK(!co.nosound);
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
	BOOST_CHECK(!co.test);
	BOOST_CHECK(!co.userconfig_dir);
	BOOST_CHECK(!co.userconfig_path);
	BOOST_CHECK(!co.userdata_dir);
	BOOST_CHECK(!co.userdata_path);
	BOOST_CHECK(!co.validcache);
	BOOST_CHECK(!co.version);
	BOOST_CHECK(!co.windowed);
	BOOST_CHECK(!co.with_replay);
}

BOOST_AUTO_TEST_SUITE_END()
