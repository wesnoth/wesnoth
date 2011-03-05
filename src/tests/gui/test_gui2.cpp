/* $Id$ */
/*
   Copyright (C) 2009 - 2011 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

// In this domain since it compares a shared string from this domain.
#define GETTEXT_DOMAIN "wesnoth-lib"

#include <boost/test/unit_test.hpp>

#include "config_cache.hpp"
#include "filesystem.hpp"
#include "foreach.hpp"
#include "formula_debugger.hpp"
#include "gettext.hpp"
#include "game_config.hpp"
#include "game_display.hpp"
#include "gui/auxiliary/layout_exception.hpp"
#include "gui/dialogs/addon_connect.hpp"
#include "gui/dialogs/addon_list.hpp"
#include "gui/dialogs/campaign_selection.hpp"
#include "gui/dialogs/data_manage.hpp"
#include "gui/dialogs/debug_clock.hpp"
#include "gui/dialogs/edit_label.hpp"
#include "gui/dialogs/editor_generate_map.hpp"
#include "gui/dialogs/editor_new_map.hpp"
#include "gui/dialogs/editor_resize_map.hpp"
#include "gui/dialogs/editor_settings.hpp"
#include "gui/dialogs/formula_debugger.hpp"
#include "gui/dialogs/game_delete.hpp"
#include "gui/dialogs/game_load.hpp"
#include "gui/dialogs/game_save.hpp"
#include "gui/dialogs/gamestate_inspector.hpp"
#include "gui/dialogs/language_selection.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/mp_cmd_wrapper.hpp"
#include "gui/dialogs/mp_connect.hpp"
#include "gui/dialogs/mp_create_game.hpp"
#include "gui/dialogs/mp_method_selection.hpp"
#include "gui/dialogs/simple_item_selector.hpp"
#include "gui/dialogs/title_screen.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/dialogs/unit_attack.hpp"
#include "gui/dialogs/unit_create.hpp"
#include "gui/dialogs/image_message/image_message.hpp"
//TODO enable when safe
//#include "gui/dialogs/image_message/input_message.hpp"
//#include "gui/dialogs/image_message/option_message.hpp"
//#include "gui/dialogs/image_message/unit_message.hpp"
#include "gui/widgets/settings.hpp"
#include "language.hpp"
#include "map_create.hpp"
#include "tests/utils/fake_display.hpp"
#include "video.hpp"
#include "wml_exception.hpp"

#include <boost/bind.hpp>

#include <memory>

namespace gui2 {

std::vector<std::string>& unit_test_registered_window_list()
{
	static std::vector<std::string> result =
			tunit_test_access_only::get_registered_window_list();

	return result;
}

void unit_test_mark_as_tested(const tdialog& dialog)
{
	std::vector<std::string>& list = unit_test_registered_window_list();
	list.erase(
			std::remove(list.begin(), list.end(), dialog.window_id())
			, list.end());
}

}// namespace gui2

namespace {

	/** The main config, which contains the entire WML tree. */
	config main_config;

	/**
	 * Helper class to generate a dialog.
	 *
	 * This class makes sure the dialog is properly created and initialized.
	 * The specialized versions are at the end of this file.
	 */
	template<class T>
	struct twrapper
	{
		static T* create() { return new T(); }
	};

	typedef std::pair<unsigned, unsigned> tresolution;
	typedef std::vector<std::pair<unsigned, unsigned> > tresolution_list;

	CVideo video(CVideo::FAKE_TEST);

	template<class T>
	void test_resolutions(const tresolution_list& resolutions)
	{
		foreach(const tresolution& resolution, resolutions) {
			video.make_test_fake(resolution.first, resolution.second);

			std::auto_ptr<T> dlg(twrapper<T>::create());
			BOOST_REQUIRE_MESSAGE(dlg.get(), "Failed to create a dialog.");

			gui2::unit_test_mark_as_tested(
					dynamic_cast<gui2::tdialog&>(*(dlg.get())));

			std::string exception;
			try {
				dlg->show(video, 1);
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

} // namespace

BOOST_AUTO_TEST_CASE(test_gui2)
{
	/**** Initialize the environment. *****/
	game_config::config_cache& cache = game_config::config_cache::instance();

	cache.clear_defines();
	cache.add_define("EDITOR");
	cache.add_define("MULTIPLAYER");
	cache.get_config(game_config::path +"/data", main_config);

	const binary_paths_manager bin_paths_manager(main_config);

	load_language_list();
	game_config::load_config(main_config.child("game_config"));

	/**** Run the tests. *****/
	test<gui2::taddon_connect>();
	test<gui2::taddon_list>();
	test<gui2::tcampaign_selection>();
	test<gui2::tdata_manage>();
	test<gui2::tedit_label>();
	test<gui2::teditor_generate_map>();
	test<gui2::teditor_new_map>();
	test<gui2::teditor_resize_map>();
	test<gui2::teditor_settings>();
	test<gui2::tformula_debugger>();
	test<gui2::tgame_delete>();
	test<gui2::tgame_load>();
	test<gui2::tgame_save>();
	test<gui2::tgame_save_message>();
	test<gui2::tgame_save_oos>();
//	test<gui2::tgamestate_inspector>(); /** @todo ENABLE */
	test<gui2::tlanguage_selection>();
	test<gui2::tmessage>();
	test<gui2::tsimple_item_selector>();
	test<gui2::tmp_cmd_wrapper>();
	test<gui2::tmp_connect>();
	test<gui2::tmp_create_game>();
	test<gui2::tmp_login>();
	test<gui2::tmp_method_selection>();
//	test<gui2::tmp_server_list>(); /** @todo ENABLE */
	test<gui2::ttitle_screen>();
	test<gui2::ttransient_message>();
//	test<gui2::tunit_attack>(); /** @todo ENABLE */
	test<gui2::tunit_create>();
	test<gui2::timage_message_left>();
	test<gui2::timage_message_right>();
//TODO enable when safe
//	test<gui2::tinput_message_left>();
//	test<gui2::tinput_message_right>();
//	test<gui2::toption_message_left>();
//	test<gui2::toption_message_right>();
//	test<gui2::tunit_message_left>();
//	test<gui2::tunit_message_right>();

	std::vector<std::string>& list = gui2::unit_test_registered_window_list();

	/**
	 * @todo Clear this list of removals.
	 *
	 * This list marks some dialogs as tested that are not tested.
	 * This list should be removed so all dialogs are properly tested.
	 */
//	list.erase(
//			std::remove(list.begin(), list.end(), "timage_message_left")
//			, list.end());
//	list.erase(
//			std::remove(list.begin(), list.end(), "timage_message_right")
//			, list.end());
	list.erase(
			std::remove(list.begin(), list.end(), "toption_message_left")
			, list.end());
	list.erase(
			std::remove(list.begin(), list.end(), "toption_message_right")
			, list.end());
	list.erase(
			std::remove(list.begin(), list.end(), "tinput_message_left")
			, list.end());
	list.erase(
			std::remove(list.begin(), list.end(), "tinput_message_right")
			, list.end());
	list.erase(
			std::remove(list.begin(), list.end(), "tunit_message_left")
			, list.end());
	list.erase(
			std::remove(list.begin(), list.end(), "tunit_message_right")
			, list.end());
	list.erase(
			std::remove(list.begin(), list.end(), "trecruit_message_left")
			, list.end());
	list.erase(
			std::remove(list.begin(), list.end(), "trecruit_message_right")
			, list.end());
	list.erase(
			std::remove(list.begin(), list.end(), "debug_clock")
			, list.end());
	list.erase(
			std::remove(list.begin(), list.end(), "gamestate_inspector")
			, list.end());
	list.erase(
			std::remove(list.begin(), list.end(), "unit_attack")
			, list.end());
	list.erase(
			std::remove(list.begin(), list.end(), "mp_server_list")
			, list.end());
	list.erase(
			std::remove(list.begin(), list.end(), "tooltip_large")
			, list.end());

	// Test size() instead of empty() to get the number of offenders
	BOOST_CHECK_EQUAL(list.size(), 0);
	foreach(const std::string& id, list) {
		std::cerr << "Window '" << id << "' registerd but not tested.\n";
	}
}

BOOST_AUTO_TEST_CASE(test_make_test_fake)
{
	video.make_test_fake(10, 10);

	try {
		gui2::tmessage dlg("title", "message", true);
		dlg.show(video, 1);
	} catch(twml_exception& e) {
		BOOST_CHECK(e.user_message == _("Failed to show a dialog, "
					"which doesn't fit on the screen."));
		return;
	} catch(...) {
	}
	BOOST_ERROR("Didn't catch the wanted exception.");
}

namespace {

template<>
struct twrapper<gui2::taddon_list>
{
	static gui2::taddon_list* create()
	{
		/** @todo Would nice to add one or more dummy addons in the list. */
		static config cfg;
		return new gui2::taddon_list(cfg);
	}
};

template<>
struct twrapper<gui2::tcampaign_selection>
{
	static gui2::tcampaign_selection* create()
	{
		const config::const_child_itors &ci =
				main_config.child_range("campaign");
		static std::vector<config> campaigns(ci.first, ci.second);

		return new gui2::tcampaign_selection(campaigns);
	}
};

template<>
struct twrapper<gui2::tdata_manage>
{
	static gui2::tdata_manage* create()
	{
		/** @todo Would be nice to add real data to the config. */
		return new gui2::tdata_manage(config());
	}
};

template<>
struct twrapper<gui2::tedit_label>
{
	static gui2::tedit_label* create()
	{
		return new gui2::tedit_label("Label text to modify", false);
	}
};

template<>
struct twrapper<gui2::tformula_debugger>
{
	static gui2::tformula_debugger* create()
	{
		static game_logic::formula_debugger debugger;
		return new gui2::tformula_debugger(debugger);
	}
};

template<>
struct twrapper<gui2::tgame_load>
{
	static gui2::tgame_load* create()
	{
		/** @todo Would be nice to add real data to the config. */
		return new gui2::tgame_load(config());
	}

};

template<>
struct twrapper<gui2::tgame_save>
{
	static gui2::tgame_save* create()
	{
		return new gui2::tgame_save("Title", "filename");
	}

};

template<>
struct twrapper<gui2::tgame_save_message>
{
	static gui2::tgame_save_message* create()
	{
		return new gui2::tgame_save_message("Title", "filename", "message");
	}

};

template<>
struct twrapper<gui2::tgame_save_oos>
{
	static gui2::tgame_save_oos* create()
	{
		return new gui2::tgame_save_oos("Title", "filename", "message");
	}

};

#if 0
template<>
struct twrapper<gui2::tgamestate_inspector>
{
	static gui2::tgamestate_inspector* create()
	{
		/** @todo Would be nice to add real data to the vconfig. */
		return new gui2::tgamestate_inspector(vconfig());
	}

};
#endif

template<>
struct twrapper<gui2::tmessage>
{
	static gui2::tmessage* create()
	{
		return new gui2::tmessage("Title", "Message", false);
	}
};

template<>
struct twrapper<gui2::tmp_cmd_wrapper>
{
	static gui2::tmp_cmd_wrapper* create()
	{
		return new gui2::tmp_cmd_wrapper("foo");
	}
};

template<>
struct twrapper<gui2::tmp_create_game>
{
	static gui2::tmp_create_game* create()
	{
		return new gui2::tmp_create_game(main_config);
	}
};

template<>
struct twrapper<gui2::tmp_login>
{
	static gui2::tmp_login* create()
	{
		return new gui2::tmp_login("label", true);
	}
};

template<>
struct twrapper<gui2::tsimple_item_selector>
{
	static gui2::tsimple_item_selector* create()
	{
		return new gui2::tsimple_item_selector("title"
				, "message"
				, std::vector<std::string>()
				, false
				, false);
	}
};

template<>
struct twrapper<gui2::teditor_generate_map>
{
	static gui2::teditor_generate_map* create()
	{
		gui2::teditor_generate_map* result = new gui2::teditor_generate_map();
		BOOST_REQUIRE_MESSAGE(result, "Failed to create a dialog.");

		std::vector<map_generator*> map_generators;
		foreach (const config &i, main_config.child_range("multiplayer")) {
			if(i["map_generation"] == "default") {
				const config &generator_cfg = i.child("generator");
				if (generator_cfg) {
					map_generators.push_back(
							create_map_generator("", generator_cfg));
				}
			}
		}
		result->set_map_generators(map_generators);

		result->set_gui(
				static_cast<display*>(&test_utils::get_fake_display(-1, -1)));

		return result;
	}
};

template<>
struct twrapper<gui2::teditor_new_map>
{
	static gui2::teditor_new_map* create()
	{
		static int width;
		static int height;
		return new gui2::teditor_new_map(width, height);
	}
};

template<>
struct twrapper<gui2::teditor_settings>
{
	static void dummy_callback(int, int, int) {}

	static gui2::teditor_settings* create()
	{
		gui2::teditor_settings* result = new gui2::teditor_settings();
		BOOST_REQUIRE_MESSAGE(result, "Failed to create a dialog.");

		const config &cfg = main_config.child("editor_times");
		BOOST_REQUIRE_MESSAGE(cfg, "No editor time-of-day defined");

		std::vector<time_of_day> tods;
		foreach (const config &i, cfg.child_range("time")) {
			tods.push_back(time_of_day(i));
		}
		result->set_tods(tods);

		result->set_redraw_callback(boost::bind(dummy_callback, _1, _2, _3));

		return result;
	}
};

template<>
struct twrapper<gui2::ttransient_message>
{
	static gui2::ttransient_message* create()
	{
		return new gui2::ttransient_message("Title", false, "Message", false, "");
	}
};

template<>
struct twrapper<gui2::timage_message_left>
{
	static gui2::timage_message_left* create()
	{
		return new gui2::timage_message_left("Title", "Message", "", false);
	}
};

template<>
struct twrapper<gui2::timage_message_right>
{
	static gui2::timage_message_right* create()
	{
		return new gui2::timage_message_right("Title", "Message", "", false);
	}
};

//TODO enable again when safe
//template<>
//struct twrapper<gui2::tinput_message_left>
//{
//	static gui2::tinput_message_left* create()
//	{
//		return new gui2::tinput_message_left("Title", "Message", "", false);
//	}
//};
//
//template<>
//struct twrapper<gui2::tinput_message_right>
//{
//	static gui2::tinput_message_right* create()
//	{
//		return new gui2::tinput_message_right("Title", "Message", "", false);
//	}
//};
//
//template<>
//struct twrapper<gui2::option_message_left>
//{
//	static gui2::toption_message_left* create()
//	{
//		return new gui2::toption_message_left("Title", "Message", "", false);
//	}
//};
//
//template<>
//struct twrapper<gui2::toption_message_right>
//{
//	static gui2::toption_message_right* create()
//	{
//		return new gui2::toption_message_right("Title", "Message", "", false);
//	}
//};


} // namespace

namespace editor {
	std::string selected_terrain, left_button_function;
}

