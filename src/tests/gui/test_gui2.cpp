/*
   Copyright (C) 2009 - 2016 by Mark de Wever <koraq@xs4all.nl>
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
#include "formula/debugger.hpp"
#include "gettext.hpp"
#include "game_classification.hpp"
#include "game_config.hpp"
#include "game_display.hpp"
#include "generators/map_create.hpp"
#include "gui/core/layout_exception.hpp"
#include "gui/dialogs/addon/connect.hpp"
#include "gui/dialogs/addon/list.hpp"
#include "gui/dialogs/advanced_graphics_options.hpp"
#include "gui/dialogs/campaign_difficulty.hpp"
#include "gui/dialogs/campaign_selection.hpp"
#include "gui/dialogs/campaign_settings.hpp"
#include "gui/dialogs/chat_log.hpp"
#include "gui/dialogs/data_manage.hpp"
#include "gui/dialogs/debug_clock.hpp"
#include "gui/dialogs/edit_label.hpp"
#include "gui/dialogs/edit_text.hpp"
#include "gui/dialogs/editor/generate_map.hpp"
#include "gui/dialogs/editor/new_map.hpp"
#include "gui/dialogs/editor/resize_map.hpp"
#include "gui/dialogs/editor/set_starting_position.hpp"
#include "gui/dialogs/folder_create.hpp"
#include "gui/dialogs/formula_debugger.hpp"
#include "gui/dialogs/game_cache_options.hpp"
#include "gui/dialogs/game_delete.hpp"
#include "gui/dialogs/game_load.hpp"
#include "gui/dialogs/game_version.hpp"
#include "gui/dialogs/game_save.hpp"
#include "gui/dialogs/gamestate_inspector.hpp"
#include "gui/dialogs/label_settings.hpp"
#include "gui/dialogs/language_selection.hpp"
#include "gui/dialogs/lobby_main.hpp"
#include "gui/dialogs/lobby_player_info.hpp"
#include "gui/dialogs/lua_interpreter.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/multiplayer/mp_alerts_options.hpp"
#include "gui/dialogs/multiplayer/mp_change_control.hpp"
#include "gui/dialogs/multiplayer/mp_cmd_wrapper.hpp"
#include "gui/dialogs/multiplayer/mp_connect.hpp"
#include "gui/dialogs/multiplayer/mp_create_game.hpp"
#include "gui/dialogs/multiplayer/mp_create_game_set_password.hpp"
#include "gui/dialogs/multiplayer/mp_join_game_password_prompt.hpp"
#include "gui/dialogs/depcheck_confirm_change.hpp"
#include "gui/dialogs/depcheck_select_new.hpp"
#include "gui/dialogs/multiplayer/mp_login.hpp"
#include "gui/dialogs/multiplayer/mp_method_selection.hpp"
#include "gui/dialogs/simple_item_selector.hpp"
#include "gui/dialogs/screenshot_notification.hpp"
#include "gui/dialogs/select_orb_colors.hpp"
#include "gui/dialogs/theme_list.hpp"
#include "gui/dialogs/title_screen.hpp"
#include "gui/dialogs/tip.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/dialogs/unit_attack.hpp"
#include "gui/dialogs/unit_create.hpp"
#include "gui/dialogs/unit_recruit.hpp"
#include "gui/dialogs/wml_error.hpp"
#include "gui/dialogs/wml_message.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "language.hpp"
#include "game_initialization/create_engine.hpp"
#include "tests/utils/fake_display.hpp"
#include "saved_game.hpp"
//#include "scripting/lua_kernel_base.hpp"
#include "video.hpp"
#include "wml_exception.hpp"

#include <boost/assign/list_of.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#include <memory>

namespace gui2 {

std::vector<std::string>& unit_test_registered_window_list()
{
	static std::vector<std::string> result =
			tunit_test_access_only::get_registered_window_list();

	return result;
}

std::string unit_test_mark_as_tested(const tdialog& dialog)
{
	std::vector<std::string>& list = unit_test_registered_window_list();
	list.erase(
			std::remove(list.begin(), list.end(), dialog.window_id())
			, list.end());
	return dialog.window_id();
}

std::string unit_test_mark_popup_as_tested(const tpopup& dialog)
{
	std::vector<std::string>& list = unit_test_registered_window_list();
	list.erase(
			std::remove(list.begin(), list.end(), dialog.window_id())
			, list.end());
	return dialog.window_id();
}

twindow* unit_test_window(const tpopup& dialog)
{
	return dialog.window_;
}

class tmp_server_list;

tdialog* unit_test_mp_server_list()
{
	return tmp_connect::mp_server_list_for_unit_test();
}

} // namespace gui2

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

	template<class T>
	void test_resolutions(const tresolution_list& resolutions)
	{
		BOOST_FOREACH(const tresolution& resolution, resolutions) {
			CVideo& video = test_utils::get_fake_display(resolution.first, resolution.second).video();

			boost::scoped_ptr<gui2::tdialog> dlg(twrapper<T>::create());
			BOOST_REQUIRE_MESSAGE(dlg.get(), "Failed to create a dialog.");

			const std::string id = gui2::unit_test_mark_as_tested(*(dlg.get()));

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
					"Test for '" << id
					<< "' Failed\nnew widgets = " << gui2::new_widgets
					<< " resolution = " << resolution.first
					<< 'x' << resolution.second
					<< "\nException caught: " << exception << '.');
		}
	}

	template<class T>
	void test_popup_resolutions(const tresolution_list& resolutions)
	{
		bool interact = false;
		for(int i = 0; i < 2; ++i) {
			BOOST_FOREACH(const tresolution& resolution, resolutions) {
				CVideo& video = test_utils::get_fake_display(resolution.first, resolution.second).video();

				boost::scoped_ptr<gui2::tpopup> dlg(twrapper<T>::create());
				BOOST_REQUIRE_MESSAGE(dlg.get(), "Failed to create a dialog.");

				const std::string id = gui2::unit_test_mark_popup_as_tested(*(dlg.get()));

				std::string exception;
				try {
					dlg->show(video, interact);
					gui2::twindow* window = gui2::unit_test_window((*dlg.get()));
					BOOST_REQUIRE_NE(window, static_cast<void*>(NULL));
					window->draw();
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
						"Test for '" << id
						<< "' Failed\nnew widgets = " << gui2::new_widgets
						<< " resolution = " << resolution.first
						<< 'x' << resolution.second
						<< "\nException caught: " << exception << '.');
			}

			interact = true;
		}
	}

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4702)
#endif
	void test_tip_resolutions(const tresolution_list& resolutions
			, const std::string& id)
	{
		BOOST_FOREACH(const tresolution& resolution, resolutions) {
			
			//CVideo& video = test_utils::get_fake_display(resolution.first, resolution.second).video();

			std::vector<std::string>& list =
					gui2::unit_test_registered_window_list();
			list.erase(std::remove(list.begin(), list.end(), id), list.end());

			std::string exception;
			try {
/**
 * @todo The code crashes for some unknown reason when this code is disabled.
 * The backtrace however doesn't show this path, in fact the crash occurs
 * before this code is used. So not entirely sure whether it's a compiler bug
 * or a part of the static initialization fiasco. Need to test with different
 * compilers and try to find the cause.
 */
#if 0
				gui2::tip::show(video
						, id
						, "Test messsage for a tooltip."
						, gui2::tpoint(0, 0));
#endif
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
					"Test for tip '" << id
					<< "' Failed\nnew widgets = " << gui2::new_widgets
					<< " resolution = " << resolution.first
					<< 'x' << resolution.second
					<< "\nException caught: " << exception << '.');
		}
	}
#ifdef _MSC_VER
#pragma warning(pop)
#endif

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

		test_resolutions<T>(get_small_gui_resolutions());
		test_resolutions<T>(get_gui_resolutions());

		break; // FIXME: New widgets break
		gui2::new_widgets = true;
	}
}

template<class T>
void test_popup()
{
	gui2::new_widgets = false;

	for(size_t i = 0; i < 2; ++i) {

		test_popup_resolutions<T>(get_small_gui_resolutions());
		test_popup_resolutions<T>(get_gui_resolutions());

		gui2::new_widgets = true;
	}
}

void test_tip(const std::string& id)
{
	gui2::new_widgets = false;

	for(size_t i = 0; i < 2; ++i) {

		test_tip_resolutions(get_small_gui_resolutions(), id);
		test_tip_resolutions(get_gui_resolutions(), id);

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

	const filesystem::binary_paths_manager bin_paths_manager(main_config);

	load_language_list();
	game_config::load_config(main_config.child("game_config"));

	/**** Run the tests. *****/

	/* The tdialog classes. */
	test<gui2::taddon_connect>();
	//test<gui2::taddon_list>();
	test<gui2::tcampaign_difficulty>();
	test<gui2::tcampaign_selection>();
	test<gui2::tcampaign_settings>();
//	test<gui2::tchat_log>(); /** @todo ENABLE */
	test<gui2::tdata_manage>();
	test<gui2::tedit_label>();
	test<gui2::tedit_text>();
	test<gui2::teditor_generate_map>();
	test<gui2::teditor_new_map>();
	test<gui2::teditor_resize_map>();
	test<gui2::teditor_set_starting_position>();
	test<gui2::tfolder_create>();
	test<gui2::tformula_debugger>();
	test<gui2::tgame_cache_options>();
	test<gui2::tgame_delete>();
	test<gui2::tgame_load>();
	test<gui2::tgame_version>();
	test<gui2::tgame_save>();
	test<gui2::tgame_save_message>();
	test<gui2::tgame_save_oos>();
	test<gui2::tgamestate_inspector>();
	test<gui2::tlanguage_selection>();
	test<gui2::tlobby_main>();
	test<gui2::tlobby_player_info>();
	test<gui2::tmessage>();
	test<gui2::tmp_change_control>();
	test<gui2::tmp_cmd_wrapper>();
	test<gui2::tmp_connect>();
	test<gui2::tmp_create_game>();
	test<gui2::tmp_create_game_set_password>();
	test<gui2::tmp_join_game_password_prompt>();
	test<gui2::tdepcheck_confirm_change>();
	test<gui2::tdepcheck_select_new>();
	test<gui2::tmp_login>();
	test<gui2::tmp_method_selection>();
	test<gui2::tmp_server_list>();
	test<gui2::tsimple_item_selector>();
	test<gui2::tscreenshot_notification>();
	test<gui2::tselect_orb_colors>();
	test<gui2::ttheme_list>();
	test<gui2::ttitle_screen>();
	test<gui2::ttransient_message>();
	//test<gui2::tunit_attack>();
	test<gui2::tunit_create>();
	//test<gui2::tunit_recruit>();
	test<gui2::twml_error>();
	test<gui2::twml_message_left>();
	test<gui2::twml_message_right>();
	test<gui2::tmp_alerts_options>();
	test<gui2::tadvanced_graphics_options>();

	//test<gui2::tlua_interpreter>(& lua_kernel_base());

	/* The tpopup classes. */
	test_popup<gui2::tdebug_clock>();

	/* The tooltip classes. */
	test_tip("tooltip_large");

	std::vector<std::string>& list = gui2::unit_test_registered_window_list();

	/*
	 * The unit attack unit test are disabled for now, they calling parameters
	 * don't allow 'NULL's needs to be fixed.
	 */
	list.erase(
			std::remove(list.begin(), list.end(), "unit_attack")
			, list.end());
	/*
	 * The chat log unit test are disabled for now, they calling parameters
	 * don't allow 'NULL's needs to be fixed.
	 */
	list.erase(
			std::remove(list.begin(), list.end(), "chat_log")
			, list.end());

	// No test for this right now, not sure how to use the test system
	// for dialog with no default constructor
	list.erase(
			std::remove(list.begin(), list.end(), "lua_interpreter")
			, list.end());

	/*
	 * Disable label settings dialog test because we need a display_context
	 * object, which we don't have, and it's a lot of work to produce a dummy
	 * one.
	 */
	list.erase(
			std::remove(list.begin(), list.end(), "label_settings")
			, list.end());

	//Window 'addon_description' registered but not tested.
	//Window 'addon_filter_options' registered but not tested.
	//Window 'addon_uninstall_list' registered but not tested.
	//Window 'network_transmission' registered but not tested.
	list.erase(std::remove(list.begin(), list.end(), "addon_description"), list.end());
	list.erase(std::remove(list.begin(), list.end(), "addon_filter_options"), list.end());
	list.erase(std::remove(list.begin(), list.end(), "addon_uninstall_list"), list.end());
	list.erase(std::remove(list.begin(), list.end(), "addon_list"), list.end());
	list.erase(std::remove(list.begin(), list.end(), "network_transmission"), list.end());
	list.erase(std::remove(list.begin(), list.end(), "synced_choice_wait"), list.end());
	list.erase(std::remove(list.begin(), list.end(), "drop_down_list"), list.end());
	list.erase(std::remove(list.begin(), list.end(), "preferences"), list.end());
	list.erase(std::remove(list.begin(), list.end(), "unit_recruit"), list.end());

	// Test size() instead of empty() to get the number of offenders
	BOOST_CHECK_EQUAL(list.size(), 0);
	BOOST_FOREACH(const std::string& id, list) {
		std::cerr << "Window '" << id << "' registered but not tested.\n";
	}
}

BOOST_AUTO_TEST_CASE(test_make_test_fake)
{
	CVideo& video = test_utils::get_fake_display(10, 10).video();

	try {
		gui2::tmessage dlg("title", "message", true, false);
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
struct twrapper<gui2::taddon_connect>
{
	static gui2::taddon_connect* create()
	{
		static std::string host_name = "host_name";
		return new gui2::taddon_connect(host_name, true);
	}
};

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
struct twrapper<gui2::tcampaign_difficulty>
{
	static gui2::tcampaign_difficulty* create()
	{
		static const config items;

		return new gui2::tcampaign_difficulty(items);
	}
};

template<>
struct twrapper<gui2::tcampaign_selection>
{
	static gui2::tcampaign_selection* create()
	{
		static saved_game state;
		state.classification().campaign_type = game_classification::CAMPAIGN_TYPE::SCENARIO;
		static ng::create_engine ng(test_utils::get_fake_display(-1, -1).video(), state);
		return new gui2::tcampaign_selection(ng);
	}
};

template<>
struct twrapper<gui2::tcampaign_settings>
{
	static gui2::tcampaign_settings* create()
	{
		static saved_game state;
		state.classification().campaign_type = game_classification::CAMPAIGN_TYPE::SCENARIO;
		static ng::create_engine ng(test_utils::get_fake_display(-1, -1).video(), state);
		return new gui2::tcampaign_settings(ng);
	}
};

template<>
struct twrapper<gui2::tchat_log>
{
	static gui2::tchat_log* create()
	{
		static config cfg;
		static vconfig vcfg(cfg);

		return new gui2::tchat_log(vcfg, NULL);
	}
};

template<>
struct twrapper<gui2::tdata_manage>
{
	static gui2::tdata_manage* create()
	{
		return new gui2::tdata_manage();
	}
};

template<>
struct twrapper<gui2::tedit_label>
{
	static gui2::tedit_label* create()
	{
		static std::string label = "Label text to modify";
		static bool team_only = false;
		return new gui2::tedit_label(label, team_only);
	}
};

template<>
struct twrapper<gui2::tedit_text>
{
	static gui2::tedit_text* create()
	{
		static std::string text = "text to modify";
		return new gui2::tedit_text("title", "label", text);
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
		static config cfg;
		return new gui2::tgame_load(cfg);
	}

};

template<>
struct twrapper<gui2::tgame_version>
{
	static gui2::tgame_version* create()
	{
		return new gui2::tgame_version();
	}

};

template<>
struct twrapper<gui2::tgame_save>
{
	static gui2::tgame_save* create()
	{
		static std::string title = "Title";
		static std::string filename = "filename";
		return new gui2::tgame_save(title, filename);
	}

};

template<>
struct twrapper<gui2::tgame_save_message>
{
	static gui2::tgame_save_message* create()
	{
		static std::string title = "Title";
		static std::string filename = "filename";
		static std::string message = "message";
		return new gui2::tgame_save_message(title, filename, message);
	}

};

template<>
struct twrapper<gui2::tgame_save_oos>
{
	static gui2::tgame_save_oos* create()
	{
		static bool ignore_all = false;
		static std::string title = "Title";
		static std::string filename = "filename";
		static std::string message = "message";
		return new gui2::tgame_save_oos(ignore_all, title, filename, message);
	}

};

template<>
struct twrapper<gui2::tgamestate_inspector>
{
	static gui2::tgamestate_inspector* create()
	{
		/**
		 * @todo Would be nice to add real data to the vconfig.
		 * It would also involve adding real data to the resources.
		 */
		static config cfg;
		static vconfig vcfg(cfg);
		return new gui2::tgamestate_inspector(vcfg);
	}

};

template<>
struct twrapper<gui2::tlobby_main>
{
	static gui2::tlobby_main* create()
	{
		static config game_config;
		static lobby_info li(game_config);
		return new gui2::tlobby_main(game_config, li,
			static_cast<display*>(&test_utils::get_fake_display(-1, -1))->video());
	}
};

class fake_chat_handler : public events::chat_handler {
	void add_chat_message(const time_t&,
		const std::string&, int, const std::string&,
		MESSAGE_TYPE) {}
	void send_chat_message(const std::string&, bool) {}
};

template<>
struct twrapper<gui2::tlobby_player_info>
{
	static gui2::tlobby_player_info* create()
	{
		static config c;
		static fake_chat_handler ch;
		static user_info ui(c);
		static lobby_info li(c);
		return new gui2::tlobby_player_info(ch, ui, li);
	}
};

template<>
struct twrapper<gui2::tmessage>
{
	static gui2::tmessage* create()
	{
		return new gui2::tmessage("Title", "Message", false, false);
	}
};

template<>
struct twrapper<gui2::tmp_change_control>
{
	static gui2::tmp_change_control* create()
	{
		return new gui2::tmp_change_control(NULL);
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
struct twrapper<gui2::tmp_create_game_set_password>
{
	static gui2::tmp_create_game_set_password* create()
	{
		static std::string password;
		return new gui2::tmp_create_game_set_password(password);
	}
};

template<>
struct twrapper<gui2::tmp_join_game_password_prompt>
{
	static gui2::tmp_join_game_password_prompt* create()
	{
		static std::string password;
		return new gui2::tmp_join_game_password_prompt(password);
	}
};

template<>
struct twrapper<gui2::tdepcheck_confirm_change>
{
	static gui2::tdepcheck_confirm_change* create()
	{
		static std::vector<std::string> mods = boost::assign::list_of("mod_one")("some other")("more");
		return new gui2::tdepcheck_confirm_change(true, mods, "requester");
	}
};

template<>
struct twrapper<gui2::tdepcheck_select_new>
{
	static gui2::tdepcheck_select_new* create()
	{
		static std::vector<std::string> mods = boost::assign::list_of("mod_one")("some other")("more");
		return new gui2::tdepcheck_select_new(ng::depcheck::MODIFICATION, mods);
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
struct twrapper<gui2::tscreenshot_notification>
{
	static gui2::tscreenshot_notification* create()
	{
		return new gui2::tscreenshot_notification("path");
	}
};

template<>
struct twrapper<gui2::tselect_orb_colors>
{
	static gui2::tselect_orb_colors* create()
	{
		return new gui2::tselect_orb_colors();
	}
};

template<>
struct twrapper<gui2::ttheme_list>
{
	static theme_info make_theme(std::string name)
	{
		theme_info ti;
		ti.id = name;
		ti.name = name;
		ti.description = name + " this is a description";
		return ti;
	}
	static gui2::ttheme_list* create()
	{
		static std::vector<theme_info> themes = boost::assign::list_of(make_theme("classic"))
		(make_theme("new"))(make_theme("more"))(make_theme("themes"));
		return new gui2::ttheme_list(themes, 0);
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
		BOOST_FOREACH(const config &i, main_config.child_range("multiplayer")) {
			if(i["scenario_generation"] == "default") {
				const config &generator_cfg = i.child("generator");
				if (generator_cfg) {
					map_generators.push_back(
							create_map_generator("", generator_cfg));
				}
			}
		}
		result->set_map_generators(map_generators);

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
struct twrapper<gui2::teditor_set_starting_position>
{
	static gui2::teditor_set_starting_position* create()
	{
		static std::vector<map_location> locations;

		return new gui2::teditor_set_starting_position(0, 0, locations);
	}
};

template<>
struct twrapper<gui2::teditor_resize_map>
{
	static gui2::teditor_resize_map* create()
	{
		static int width = 0;
		static int height = 0;
		static gui2::teditor_resize_map::EXPAND_DIRECTION expand_direction =
				gui2::teditor_resize_map::EXPAND_TOP;
		static bool copy = false;
		return new gui2::teditor_resize_map(
				  width
				, height
				, expand_direction
				, copy);
	}
};

template<>
struct twrapper<gui2::tfolder_create>
{
	static gui2::tfolder_create* create()
	{
		static std::string folder_name;
		return new gui2::tfolder_create(folder_name);
	}
};

template<>
struct twrapper<gui2::tmp_server_list>
{
	static gui2::tdialog* create()
	{
		return gui2::unit_test_mp_server_list();
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
struct twrapper<gui2::twml_error>
{
	static gui2::twml_error* create()
	{
		static std::vector<std::string> files = boost::assign::list_of("some")("files")("here");
		return new gui2::twml_error("Summary", "Post summary", files, "Details");
	}
};

template<>
struct twrapper<gui2::twml_message_left>
{
	static gui2::twml_message_left* create()
	{
		return new gui2::twml_message_left("Title", "Message", "", false);
	}
};

template<>
struct twrapper<gui2::twml_message_right>
{
	static gui2::twml_message_right* create()
	{
		return new gui2::twml_message_right("Title", "Message", "", false);
	}
};

} // namespace

