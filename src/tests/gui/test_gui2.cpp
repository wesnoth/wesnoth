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

#include "config_assign.hpp"
#include "config_cache.hpp"
#include "editor/editor_display.hpp" // for dummy display context
#include "filesystem.hpp"
#include "formula/debugger.hpp"
#include "gettext.hpp"
#include "game_classification.hpp"
#include "game_config.hpp"
#include "game_display.hpp"
#include "game_launcher.hpp"
#include "game_events/manager.hpp"
#include "generators/map_create.hpp"
#include "gui/core/layout_exception.hpp"
#include "gui/dialogs/addon/connect.hpp"
#include "gui/dialogs/addon/list.hpp"
#include "gui/dialogs/advanced_graphics_options.hpp"
#include "gui/dialogs/campaign_difficulty.hpp"
#include "gui/dialogs/campaign_selection.hpp"
#include "gui/dialogs/campaign_settings.hpp"
#include "gui/dialogs/chat_log.hpp"
#include "gui/dialogs/core_selection.hpp"
#include "gui/dialogs/debug_clock.hpp"
#include "gui/dialogs/edit_label.hpp"
#include "gui/dialogs/edit_text.hpp"
#include "gui/dialogs/editor/custom_tod.hpp"
#include "gui/dialogs/editor/edit_side.hpp"
#include "gui/dialogs/editor/edit_label.hpp"
#include "gui/dialogs/editor/edit_scenario.hpp"
#include "gui/dialogs/editor/generate_map.hpp"
#include "gui/dialogs/editor/generator_settings.hpp"
#include "gui/dialogs/editor/new_map.hpp"
#include "gui/dialogs/editor/resize_map.hpp"
#include "gui/dialogs/editor/set_starting_position.hpp"
#include "gui/dialogs/end_credits.hpp"
#include "gui/dialogs/folder_create.hpp"
#include "gui/dialogs/formula_debugger.hpp"
#include "gui/dialogs/game_cache_options.hpp"
#include "gui/dialogs/game_delete.hpp"
#include "gui/dialogs/game_load.hpp"
#include "gui/dialogs/game_version.hpp"
#include "gui/dialogs/game_save.hpp"
#include "gui/dialogs/game_stats.hpp"
#include "gui/dialogs/gamestate_inspector.hpp"
#include "gui/dialogs/label_settings.hpp"
#include "gui/dialogs/language_selection.hpp"
#include "gui/dialogs/loadscreen.hpp"
#include "gui/dialogs/lobby/lobby.hpp"
#include "gui/dialogs/lobby/player_info.hpp"
#include "gui/dialogs/logging.hpp"
#include "gui/dialogs/lua_interpreter.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/multiplayer/faction_select.hpp"
#include "gui/dialogs/multiplayer/mp_alerts_options.hpp"
#include "gui/dialogs/multiplayer/mp_change_control.hpp"
#include "gui/dialogs/multiplayer/mp_cmd_wrapper.hpp"
#include "gui/dialogs/multiplayer/mp_connect.hpp"
#include "gui/dialogs/multiplayer/mp_create_game.hpp"
#include "gui/dialogs/multiplayer/mp_create_game_set_password.hpp"
#include "gui/dialogs/multiplayer/mp_join_game.hpp"
#include "gui/dialogs/multiplayer/mp_join_game_password_prompt.hpp"
#include "gui/dialogs/multiplayer/mp_staging.hpp"
#include "gui/dialogs/depcheck_confirm_change.hpp"
#include "gui/dialogs/depcheck_select_new.hpp"
#include "gui/dialogs/multiplayer/mp_login.hpp"
#include "gui/dialogs/multiplayer/mp_method_selection.hpp"
#include "gui/dialogs/simple_item_selector.hpp"
#include "gui/dialogs/screenshot_notification.hpp"
#include "gui/dialogs/select_orb_colors.hpp"
#include "gui/dialogs/sp_options_configure.hpp"
#include "gui/dialogs/theme_list.hpp"
#include "gui/dialogs/title_screen.hpp"
#include "gui/dialogs/tip.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/dialogs/unit_advance.hpp"
#include "gui/dialogs/unit_attack.hpp"
#include "gui/dialogs/unit_create.hpp"
#include "gui/dialogs/unit_list.hpp"
#include "gui/dialogs/unit_recall.hpp"
#include "gui/dialogs/unit_recruit.hpp"
#include "gui/dialogs/wml_error.hpp"
#include "gui/dialogs/wml_message.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "language.hpp"
#include "game_initialization/create_engine.hpp"
#include "tests/utils/fake_display.hpp"
#include "replay.hpp"
#include "saved_game.hpp"
//#include "scripting/lua_kernel_base.hpp"
#include "video.hpp"
#include "wesnothd_connection.hpp"
#include "wml_exception.hpp"

#include "utils/functional.hpp"

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
		T* create() { return new T(); }
	};

	typedef std::pair<unsigned, unsigned> tresolution;
	typedef std::vector<std::pair<unsigned, unsigned> > tresolution_list;

	template<class T>
	void test_resolutions(const tresolution_list& resolutions)
	{
		for(const tresolution& resolution : resolutions) {
			CVideo& video = test_utils::get_fake_display(resolution.first, resolution.second).video();

			twrapper<T> ctor;
			const std::unique_ptr<gui2::tdialog> dlg(ctor.create());
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
			for(const tresolution& resolution : resolutions) {
				CVideo& video = test_utils::get_fake_display(resolution.first, resolution.second).video();

				twrapper<T> ctor;
				const std::unique_ptr<gui2::tpopup> dlg(ctor.create());
				BOOST_REQUIRE_MESSAGE(dlg.get(), "Failed to create a dialog.");

				const std::string id = gui2::unit_test_mark_popup_as_tested(*(dlg.get()));

				std::string exception;
				try {
					dlg->show(video, interact);
					gui2::twindow* window = gui2::unit_test_window((*dlg.get()));
					BOOST_REQUIRE_NE(window, static_cast<void*>(nullptr));
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
		for(const tresolution& resolution : resolutions) {
			
			CVideo& video = test_utils::get_fake_display(resolution.first, resolution.second).video();

			std::vector<std::string>& list =
					gui2::unit_test_registered_window_list();
			list.erase(std::remove(list.begin(), list.end(), id), list.end());

			std::string exception;
			try {
				gui2::tip::show(video
						, id
						, "Test messsage for a tooltip."
						, gui2::tpoint(0, 0));
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

		test_popup_resolutions<T>(get_gui_resolutions());

		gui2::new_widgets = true;
	}
}

void test_tip(const std::string& id)
{
	gui2::new_widgets = false;

	for(size_t i = 0; i < 2; ++i) {

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
	test<gui2::tchat_log>();
	test<gui2::tedit_label>();
	test<gui2::tedit_text>();
	test<gui2::teditor_generate_map>();
	test<gui2::teditor_new_map>();
	test<gui2::teditor_resize_map>();
	test<gui2::teditor_set_starting_position>();
	test<gui2::tfaction_select>();
	test<gui2::tfolder_create>();
	test<gui2::tformula_debugger>();
	test<gui2::tgame_cache_options>();
	test<gui2::tgame_delete>();
	test<gui2::tgame_load>();
	test<gui2::tgame_version>();
	test<gui2::tgame_save>();
	test<gui2::tgame_save_message>();
	test<gui2::tgame_save_oos>();
	test<gui2::tgame_stats>();
	test<gui2::tgamestate_inspector>();
	test<gui2::tgenerator_settings>();
	test<gui2::tlanguage_selection>();
	// test<gui2::tloadscreen>(); TODO: enable
	test<gui2::tlobby_main>();
	test<gui2::tlobby_player_info>();
	test<gui2::tlogging>();
	test<gui2::tmessage>();
	test<gui2::tmp_change_control>();
	test<gui2::tmp_cmd_wrapper>();
	test<gui2::tmp_connect>();
	//test<gui2::tmp_create_game>();
	test<gui2::tmp_create_game_set_password>();
	//test<gui2::tmp_join_game>();
	test<gui2::tmp_join_game_password_prompt>();
	test<gui2::tdepcheck_confirm_change>();
	test<gui2::tdepcheck_select_new>();
	test<gui2::tmp_login>();
	test<gui2::tmp_method_selection>();
	test<gui2::tmp_server_list>();
	//test<gui2::tmp_staging>();
	test<gui2::tsimple_item_selector>();
	test<gui2::tscreenshot_notification>();
	test<gui2::tselect_orb_colors>();
	test<gui2::tsp_options_configure>();
	test<gui2::ttheme_list>();
	//test<gui2::ttitle_screen>();
	test<gui2::ttransient_message>();
	//test<gui2::tunit_advance>();
	//test<gui2::tunit_attack>();
	test<gui2::tunit_create>();
	//test<gui2::tunit_list>();
	//test<gui2::tunit_recall>();
	//test<gui2::tunit_recruit>();
	test<gui2::twml_error>();
	test<gui2::twml_message_left>();
	test<gui2::twml_message_right>();
	test<gui2::twml_message_double>();
	test<gui2::tmp_alerts_options>();
	test<gui2::tadvanced_graphics_options>();
	test<gui2::tcustom_tod>();
	test<gui2::teditor_edit_label>();
	test<gui2::teditor_edit_side>();
	test<gui2::teditor_edit_scenario>();
	//test<gui2::tend_credits>();
	test<gui2::tcore_selection>();
	//test<gui2::tlua_interpreter>(& lua_kernel_base());

	/* The tpopup classes. */
	test_popup<gui2::tdebug_clock>();

	/* The tooltip classes. */
	test_tip("tooltip_large");
	test_tip("tooltip");

	std::vector<std::string>& list = gui2::unit_test_registered_window_list();
	std::vector<std::string> omitted = {
		/*
		 * The unit attack unit test are disabled for now, they calling parameters
		 * don't allow 'nullptr's needs to be fixed.
		 */
		"unit_attack",
		// No test for this right now, not sure how to use the test system
		// for dialog with no default constructor
		"lua_interpreter",
		/*
		 * Disable label settings dialog test because we need a display_context
		 * object, which we don't have, and it's a lot of work to produce a dummy
		 * one.
		 */
		"label_settings",
		"addon_description",
		"addon_filter_options",
		"addon_uninstall_list",
		"addon_list",
		"loadscreen",
		"network_transmission",
		"synced_choice_wait",
		"drop_down_list",
		"preferences",
		"unit_recruit",
		"unit_recall",
		"unit_list",
		"unit_advance",
		"mp_host_game_prompt",
		"mp_create_game",
		// The title screen appears to be throwing a bad_alloc on Travis, so disable it for now
		"title_screen",
		"end_credits",
		"mp_staging",
		"mp_join_game",
	};
	std::sort(list.begin(), list.end());
	std::sort(omitted.begin(), omitted.end());
	std::vector<std::string> missing;
	std::set_difference(list.begin(), list.end(), omitted.begin(), omitted.end(), std::back_inserter(missing));

	// Test size() instead of empty() to get the number of offenders
	BOOST_CHECK_EQUAL(missing.size(), 0);
	for(const std::string& id : missing) {
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
	std::string host_name = "host_name";
	gui2::taddon_connect* create()
	{
		return new gui2::taddon_connect(host_name, true);
	}
};

template<>
struct twrapper<gui2::taddon_list>
{
	config cfg;
	twrapper()
	{
		/** @todo Would nice to add one or more dummy addons in the list. */
	}
	gui2::taddon_list* create()
	{
		return new gui2::taddon_list(cfg);
	}
};

template<>
struct twrapper<gui2::tcampaign_difficulty>
{
	gui2::tcampaign_difficulty* create()
	{
		const config items("difficulty");

		return new gui2::tcampaign_difficulty(items);
	}
};

template<>
struct twrapper<gui2::tcampaign_selection>
{
	saved_game state;
	ng::create_engine ng;
	twrapper() : state(config_of("campaign_type", "scenario")), ng(test_utils::get_fake_display(-1, -1).video(), state)
	{
	}
	gui2::tcampaign_selection* create()
	{
		return new gui2::tcampaign_selection(ng);
	}
};

template<>
struct twrapper<gui2::tcampaign_settings>
{
	saved_game state;
	ng::create_engine ng;
	twrapper() : state(config_of("campaign_type", "scenario")), ng(test_utils::get_fake_display(-1, -1).video(), state)
	{
	}
	gui2::tcampaign_settings* create()
	{
		return new gui2::tcampaign_settings(ng);
	}
};

template<>
struct twrapper<gui2::tchat_log>
{
	config cfg;
	vconfig vcfg;
	replay_recorder_base rbase;
	replay r;
	twrapper() : vcfg(cfg), r(rbase) {}
	gui2::tchat_log* create()
	{
		return new gui2::tchat_log(vcfg, r);
	}
};

template<>
struct twrapper<gui2::tcore_selection>
{
	std::vector<config> cores;
	twrapper()
	{
		cores.resize(1);
	}
	gui2::tcore_selection* create()
	{
		return new gui2::tcore_selection(cores, 0);
	}
};

template<>
struct twrapper<gui2::tcustom_tod>
{
	std::vector<time_of_day> times;
	twrapper()
	{
		times.resize(1);
	}
	gui2::tcustom_tod* create()
	{
		return new gui2::tcustom_tod(test_utils::get_fake_display(-1, -1), times);
	}
};

template<>
struct twrapper<gui2::tedit_label>
{
	std::string label = "Label text to modify";
	bool team_only = false;
	gui2::tedit_label* create()
	{
		return new gui2::tedit_label(label, team_only);
	}
};

template<>
struct twrapper<gui2::tedit_text>
{
	std::string text = "text to modify";
	gui2::tedit_text* create()
	{
		return new gui2::tedit_text("title", "label", text);
	}
};

template<>
struct twrapper<gui2::teditor_edit_label>
{
	std::string label = "Label text to modify";
	std::string category = "test";
	bool immutable = false, fog = false, shroud = false;
	SDL_Color color;
	gui2::teditor_edit_label* create()
	{
		return new gui2::teditor_edit_label(label, immutable, fog, shroud, color, category);
	}
};

template<>
struct twrapper<gui2::teditor_edit_scenario>
{
	std::string id, name, descr;
	int turns, xp_mod;
	bool defeat_enemies, random_start;
	gui2::teditor_edit_scenario* create()
	{
		return new gui2::teditor_edit_scenario(id, name, descr, turns, xp_mod, defeat_enemies, random_start);
	}
};

template<>
struct twrapper<gui2::teditor_edit_side>
{
	team t;
	editor::editor_team_info info;
	twrapper() : info(t) {}
	gui2::teditor_edit_side* create()
	{
		return new gui2::teditor_edit_side(info);
	}
};

template<>
struct twrapper<gui2::tformula_debugger>
{
	game_logic::formula_debugger debugger;
	gui2::tformula_debugger* create()
	{
		return new gui2::tformula_debugger(debugger);
	}
};

template<>
struct twrapper<gui2::tgame_load>
{
	config cfg;
	savegame::load_game_metadata data;
	twrapper()
	{
		/** @todo Would be nice to add real data to the config. */
	}
	gui2::tgame_load* create()
	{
		return new gui2::tgame_load(cfg, data);
	}

};

template<>
struct twrapper<gui2::tgame_save>
{
	std::string title = "Title";
	std::string filename = "filename";
	gui2::tgame_save* create()
	{
		return new gui2::tgame_save(title, filename);
	}

};

template<>
struct twrapper<gui2::tgame_save_message>
{
	std::string title = "Title";
	std::string filename = "filename";
	std::string message = "message";
	gui2::tgame_save_message* create()
	{
		return new gui2::tgame_save_message(title, filename, message);
	}

};

template<>
struct twrapper<gui2::tgame_save_oos>
{
	bool ignore_all = false;
	std::string title = "Title";
	std::string filename = "filename";
	std::string message = "message";
	gui2::tgame_save_oos* create()
	{
		return new gui2::tgame_save_oos(ignore_all, title, filename, message);
	}

};

template<>
struct twrapper<gui2::tgamestate_inspector>
{
	config vars;
	game_events::manager events;
	gui2::tgamestate_inspector* create()
	{
		const display_context* dc = editor::get_dummy_display_context();
		return new gui2::tgamestate_inspector(vars, events, *dc, "Unit Test");
	}

};

struct twesnothd_connection_init
{
	twesnothd_connection_init(twesnothd_connection& conn)
	{
		//Swallow the 'cannot connect' execption so that the wesnothd_connection object doesn't throw while we test the dialog.
		try 
		{
			while (true) {
				conn.poll();
			}
		}
		catch (...) 
		{

		}
	}
};

template<>
struct twrapper<gui2::tlobby_main>
{
	config game_config;
	twesnothd_connection wesnothd_connection;
	twesnothd_connection_init wesnothd_connection_init;
	std::vector<std::string> installed_addons;
	lobby_info li;
	twrapper() : wesnothd_connection("", ""), wesnothd_connection_init(wesnothd_connection), li(game_config, installed_addons)
	{
	}
	gui2::tlobby_main* create()
	{
		return new gui2::tlobby_main(game_config, li, wesnothd_connection);
	}
};

class fake_chat_handler : public events::chat_handler {
	void add_chat_message(const time_t&,
		const std::string&, int, const std::string&,
		MESSAGE_TYPE) {}
	void send_chat_message(const std::string&, bool) {}
	void send_to_server(const config&) {}
};

template<>
struct twrapper<gui2::tlobby_player_info>
{
	config c;
	fake_chat_handler ch;
	twesnothd_connection wesnothd_connection;
	twesnothd_connection_init wesnothd_connection_init;
	user_info ui;
	std::vector<std::string> installed_addons;
	lobby_info li;
	twrapper()
		: wesnothd_connection("", ""), wesnothd_connection_init(wesnothd_connection)
		, ui(c), li(c, installed_addons)
	{
	}
	gui2::tlobby_player_info* create()
	{
		return new gui2::tlobby_player_info(ch, ui, li);
	}
};

template<>
struct twrapper<gui2::tlogging>
{
	gui2::tlogging* create()
	{
		return new gui2::tlogging();
	}
};

template<>
struct twrapper<gui2::tmessage>
{
	gui2::tmessage* create()
	{
		return new gui2::tmessage("Title", "Message", false, false);
	}
};

template<>
struct twrapper<gui2::tmp_change_control>
{
	gui2::tmp_change_control* create()
	{
		return new gui2::tmp_change_control(nullptr);
	}
};

template<>
struct twrapper<gui2::tmp_cmd_wrapper>
{
	gui2::tmp_cmd_wrapper* create()
	{
		return new gui2::tmp_cmd_wrapper("foo");
	}
};

template<>
struct twrapper<gui2::tmp_create_game>
{
	saved_game state;
	ng::create_engine engine;
	twrapper() : state(config_of("campaign_type", "multiplayer")), engine(test_utils::get_fake_display(-1, -1).video(), state)
	{
	}
	gui2::tmp_create_game* create()
	{
		return new gui2::tmp_create_game(main_config, engine);
	}
};

template<>
struct twrapper<gui2::tmp_create_game_set_password>
{
	std::string password;
	gui2::tmp_create_game_set_password* create()
	{
		return new gui2::tmp_create_game_set_password(password);
	}
};

template<>
struct twrapper<gui2::tmp_join_game_password_prompt>
{
	std::string password;
	gui2::tmp_join_game_password_prompt* create()
	{
		return new gui2::tmp_join_game_password_prompt(password);
	}
};

template<>
struct twrapper<gui2::tdepcheck_confirm_change>
{
	std::vector<std::string> mods = {"mod_one", "some other", "more"};
	gui2::tdepcheck_confirm_change* create()
	{
		return new gui2::tdepcheck_confirm_change(true, mods, "requester");
	}
};

template<>
struct twrapper<gui2::tdepcheck_select_new>
{
	std::vector<std::string> mods = {"mod_one", "some other", "more"};
	gui2::tdepcheck_select_new* create()
	{
		return new gui2::tdepcheck_select_new(ng::depcheck::MODIFICATION, mods);
	}
};

template<>
struct twrapper<gui2::tmp_login>
{
	gui2::tmp_login* create()
	{
		return new gui2::tmp_login("label", true);
	}
};

template<>
struct twrapper<gui2::tsimple_item_selector>
{
	gui2::tsimple_item_selector* create()
	{
		return new gui2::tsimple_item_selector("title", "message", std::vector<std::string>(), false, false);
	}
};

template<>
struct twrapper<gui2::tscreenshot_notification>
{
	gui2::tscreenshot_notification* create()
	{
		return new gui2::tscreenshot_notification("path");
	}
};

template<>
struct twrapper<gui2::ttheme_list>
{
	theme_info make_theme(std::string name)
	{
		theme_info ti;
		ti.id = name;
		ti.name = name;
		ti.description = name + " this is a description";
		return ti;
	}
	std::vector<theme_info> themes = {make_theme("classic"), make_theme("new"), make_theme("more"), make_theme("themes")};
	gui2::ttheme_list* create()
	{
		return new gui2::ttheme_list(themes, 0);
	}
};

template<>
struct twrapper<gui2::teditor_generate_map>
{
	gui2::teditor_generate_map* create()
	{
		gui2::teditor_generate_map* result = new gui2::teditor_generate_map();
		BOOST_REQUIRE_MESSAGE(result, "Failed to create a dialog.");

		std::vector<map_generator*> map_generators;
		for(const config &i : main_config.child_range("multiplayer")) {
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
	int width;
	int height;
	gui2::teditor_new_map* create()
	{
		return new gui2::teditor_new_map(width, height);
	}
};

template<>
struct twrapper<gui2::teditor_set_starting_position>
{
	std::vector<map_location> locations;
	gui2::teditor_set_starting_position* create()
	{
		return new gui2::teditor_set_starting_position(0, 0, locations);
	}
};

template<>
struct twrapper<gui2::teditor_resize_map>
{
	int width = 0;
	int height = 0;
	gui2::teditor_resize_map::EXPAND_DIRECTION expand_direction = gui2::teditor_resize_map::EXPAND_TOP;
	bool copy = false;
	gui2::teditor_resize_map* create()
	{
		return new gui2::teditor_resize_map(width, height, expand_direction, copy);
	}
};

template<>
struct twrapper<gui2::tfolder_create>
{
	std::string folder_name;
	gui2::tfolder_create* create()
	{
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
	gui2::ttransient_message* create()
	{
		return new gui2::ttransient_message("Title", false, "Message", false, "");
	}
};

template<>
struct twrapper<gui2::ttitle_screen>
{
	std::vector<std::string> args;
	commandline_options opts;
	game_launcher game;
	twrapper() : opts(args), game(opts, "unit_tests") {}
	gui2::ttitle_screen* create()
	{
		return new gui2::ttitle_screen(game);
	}
};

template<>
struct twrapper<gui2::twml_error>
{
	std::vector<std::string> files = {"some", "files", "here"};
	gui2::twml_error* create()
	{
		return new gui2::twml_error("Summary", "Post summary", files, "Details");
	}
};

template<>
struct twrapper<gui2::twml_message_left>
{
	gui2::twml_message_left* create()
	{
		return new gui2::twml_message_left("Title", "Message", "", false);
	}
};

template<>
struct twrapper<gui2::twml_message_right>
{
	gui2::twml_message_right* create()
	{
		return new gui2::twml_message_right("Title", "Message", "", false);
	}
};

template<>
struct twrapper<gui2::twml_message_double>
{
	gui2::twml_message_double* create()
	{
		return new gui2::twml_message_double("Title", "Message", "", false, "", true);
	}
};

template<>
struct twrapper<gui2::tfaction_select>
{
	config era_cfg, side_cfg;
	std::vector<const config*> eras;
	ng::flg_manager flg;
	std::string color;
	twrapper()
		: era_cfg(), side_cfg(), eras(1, &era_cfg) // TODO: Add an actual era definition
		, flg(eras, side_cfg, false, false, false)
		, color("teal")
	{}
	gui2::tfaction_select* create() {
		return new gui2::tfaction_select(flg, color, 1);
	}
};

template<>
struct twrapper<gui2::tgame_stats>
{
	int i;
	gui2::tgame_stats* create()
	{
		const display_context* ctx = editor::get_dummy_display_context();
		return new gui2::tgame_stats(*ctx, 1, i);
	}
};

template<>
struct twrapper<gui2::tgenerator_settings>
{
	config cfg;
	generator_data data;
	twrapper() : data(cfg) {}
	gui2::tgenerator_settings* create()
	{
		return new gui2::tgenerator_settings(data);
	}
};

template<>
struct twrapper<gui2::tsp_options_configure>
{
	saved_game state;
	ng::create_engine engine;
	twrapper() : engine(test_utils::get_fake_display(-1, -1).video(), state) {}
	gui2::tsp_options_configure* create()
	{
		return new gui2::tsp_options_configure(engine);
	}
};

} // namespace

