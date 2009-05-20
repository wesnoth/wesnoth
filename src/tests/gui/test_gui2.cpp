/* $Id$ */
/*
   Copyright (C) 2009 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-test"

#include "tests/utils/test_support.hpp"

#include "config_cache.hpp"
#include "foreach.hpp"
#include "game_config.hpp"
#include "gui/auxiliary/layout_exception.hpp"
#include "gui/dialogs/addon_connect.hpp"
#include "gui/dialogs/addon_list.hpp"
#include "gui/dialogs/campaign_selection.hpp"
#ifndef DISABLE_EDITOR2
#include "gui/dialogs/editor_generate_map.hpp"
#include "gui/dialogs/editor_new_map.hpp"
#include "gui/dialogs/editor_resize_map.hpp"
#include "gui/dialogs/editor_settings.hpp"
#endif
#include "gui/dialogs/game_save.hpp"
#include "gui/dialogs/language_selection.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/mp_connect.hpp"
#include "gui/dialogs/mp_create_game.hpp"
#include "gui/dialogs/mp_method_selection.hpp"
#include "gui/dialogs/mp_cmd_wrapper.hpp"
#include "gui/dialogs/title_screen.hpp"
#include "gui/dialogs/wml_message.hpp"
#include "gui/widgets/helper.hpp"
#include "gui/widgets/settings.hpp"
#include "video.hpp"
#include "wml_exception.hpp"

namespace {

	typedef std::pair<unsigned, unsigned> tresolution;
	typedef std::vector<std::pair<unsigned, unsigned> > tresolution_list;

	CVideo video(CVideo::FAKE_TEST);

	template<class T>
	void test_resolutions(const tresolution_list& resolutions)
	{
		foreach(const tresolution& resolution, resolutions) {
			video.make_test_fake(resolution.first, resolution.second);

			T dlg;
			std::string exception;
			try {
				dlg.show(video, 1);
			} catch(gui2::tlayout_exception_width_modified&) {
				exception = "gui2::tlayout_exception_width_modified";
			} catch(gui2::tlayout_exception_width_resize_failed&) {
				exception = "gui2::tlayout_exception_width_resize_failed";
			} catch(gui2::tlayout_exception_height_resize_failed&) {
				exception = "gui2::tlayout_exception_height_resize_failed";
			} catch(twml_exception& e) {
				exception = e.dev_message;
			} catch(std::exception& e) {
				exception = e.what();
			} catch(...) {
				exception = "unknown";
			}
			BOOST_CHECK_MESSAGE(exception.empty(),
					"Test for " << typeid(T).name()
					<< " Failed\nnew widgets = " << gui2::new_widgets
					<< " small gui = " << game_config::small_gui
					<< " resolution = " << resolution.first
					<< 'x' << resolution.second
					<< "\nException caught: " << exception << '.');
		}
	}

#ifdef USE_TINY_GUI

const tresolution_list& get_tiny_gui_resolutions()
{
	static tresolution_list result;
	if(result.empty()) {
		result.push_back(std::make_pair(320, 240));
		result.push_back(std::make_pair(640, 480));
	}
	return result;
}

template<class T>
void test()
{
	test_resolutions<T>(get_tiny_gui_resolutions());
}

#else

const tresolution_list& get_small_gui_resolutions()
{
	static tresolution_list result;
	if(result.empty()) {
		result.push_back(std::make_pair(800, 480));
	}
	return result;
}

const tresolution_list& get_gui_resolutions()
{
	static tresolution_list result;
	if(result.empty()) {
		result.push_back(std::make_pair(800, 600));
		result.push_back(std::make_pair(1024, 768));
		result.push_back(std::make_pair(1280, 1024));
		result.push_back(std::make_pair(1680, 1050));
	}
	return result;
}

template<class T>
void test()
{
	gui2::new_widgets = false;

	for(size_t i = 0; i < 2; ++i) {

		game_config::small_gui = true;
		test_resolutions<T>(get_small_gui_resolutions());

		game_config::small_gui = false;
		test_resolutions<T>(get_gui_resolutions());

		gui2::new_widgets = true;
	}
}
#endif

} // namespace

BOOST_AUTO_TEST_CASE(test_gui2)
{
	gui2::init();

	game_config::config_cache& cache = game_config::config_cache::instance();
	config game_config;

	cache.clear_defines();
#ifdef USE_TINY_GUI
	cache.add_define("TINY");
#endif
	cache.get_config(game_config::path +"/data", game_config);

	test<gui2::taddon_connect>();
//	test<gui2::taddon_list>();
//	test<gui2::tcampaign_selection>();
#ifndef DISABLE_EDITOR2
////	test<gui2::teditor_generate_map>();
	test<gui2::teditor_new_map>();
	test<gui2::teditor_resize_map>();
////	test<gui2::teditor_settings>();
#endif
//	test<gui2::tgame_save>();
	test<gui2::tlanguage_selection>();
//	test<gui2::tmessage>();
//	test<gui2::tmp_cmd_wrapper>();
////	test<gui2::tmp_connect>();
//	test<gui2::tmp_create_game>();
	test<gui2::tmp_method_selection>();
////	test<gui2::ttitle_screen>();
//	test<gui2::twml_message_left>();
//	test<gui2::twml_message_right>();

}
