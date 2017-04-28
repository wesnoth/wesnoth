/*
   Copyright (C) 2009 - 2017 by Mark de Wever <koraq@xs4all.nl>
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

#include "addon/client.hpp"
#include "addon/info.hpp"
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
#include "gui/dialogs/addon/install_dependencies.hpp"
#include "gui/dialogs/addon/manager.hpp"
#include "gui/dialogs/advanced_graphics_options.hpp"
#include "gui/dialogs/attack_predictions.hpp"
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
#include "gui/dialogs/file_dialog.hpp"
#include "gui/dialogs/folder_create.hpp"
#include "gui/dialogs/formula_debugger.hpp"
#include "gui/dialogs/game_cache_options.hpp"
#include "gui/dialogs/game_delete.hpp"
#include "gui/dialogs/game_load.hpp"
#include "gui/dialogs/game_version.hpp"
#include "gui/dialogs/game_save.hpp"
#include "gui/dialogs/game_stats.hpp"
#include "gui/dialogs/gamestate_inspector.hpp"
#include "gui/dialogs/help_browser.hpp"
#include "gui/dialogs/hotkey_bind.hpp"
#include "gui/dialogs/label_settings.hpp"
#include "gui/dialogs/language_selection.hpp"
#include "gui/dialogs/loading_screen.hpp"
#include "gui/dialogs/lobby/lobby.hpp"
#include "gui/dialogs/lobby/player_info.hpp"
#include "gui/dialogs/log_settings.hpp"
#include "gui/dialogs/lua_interpreter.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/multiplayer/faction_select.hpp"
#include "gui/dialogs/multiplayer/mp_alerts_options.hpp"
#include "gui/dialogs/multiplayer/mp_change_control.hpp"
#include "gui/dialogs/multiplayer/mp_cmd_wrapper.hpp"
#include "gui/dialogs/multiplayer/mp_connect.hpp"
#include "gui/dialogs/multiplayer/mp_create_game.hpp"
#include "gui/dialogs/multiplayer/mp_join_game.hpp"
#include "gui/dialogs/multiplayer/mp_join_game_password_prompt.hpp"
#include "gui/dialogs/multiplayer/mp_staging.hpp"
#include "gui/dialogs/outro.hpp"
#include "gui/dialogs/depcheck_confirm_change.hpp"
#include "gui/dialogs/depcheck_select_new.hpp"
#include "gui/dialogs/multiplayer/mp_login.hpp"
#include "gui/dialogs/multiplayer/mp_method_selection.hpp"
#include "gui/dialogs/simple_item_selector.hpp"
#include "gui/dialogs/screenshot_notification.hpp"
#include "gui/dialogs/select_orb_colors.hpp"
#include "gui/dialogs/sp_options_configure.hpp"
#include "gui/dialogs/statistics_dialog.hpp"
#include "gui/dialogs/story_viewer.hpp"
#include "gui/dialogs/theme_list.hpp"
#include "gui/dialogs/terrain_layers.hpp"
#include "gui/dialogs/title_screen.hpp"
#include "gui/dialogs/tooltip.hpp"
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
#include "game_initialization/lobby_data.hpp"
#include "game_initialization/lobby_info.hpp"
#include "tests/utils/fake_display.hpp"
#include "replay.hpp"
#include "saved_game.hpp"
//#include "scripting/lua_kernel_base.hpp"
#include "video.hpp"
#include "wesnothd_connection.hpp"
#include "wml_exception.hpp"

#include "utils/functional.hpp"

#include <memory>

using namespace gui2::dialogs;

namespace gui2 {

std::vector<std::string>& unit_test_registered_window_list()
{
	static std::vector<std::string> result =
			unit_test_access_only::get_registered_window_list();

	return result;
}

namespace dialogs {

std::string unit_test_mark_as_tested(const modal_dialog& dialog)
{
	std::vector<std::string>& list = unit_test_registered_window_list();
	list.erase(
			std::remove(list.begin(), list.end(), dialog.window_id())
			, list.end());
	return dialog.window_id();
}

std::string unit_test_mark_popup_as_tested(const modeless_dialog& dialog)
{
	std::vector<std::string>& list = unit_test_registered_window_list();
	list.erase(
			std::remove(list.begin(), list.end(), dialog.window_id())
			, list.end());
	return dialog.window_id();
}

window* unit_test_window(const modeless_dialog& dialog)
{
	return dialog.window_;
}

class mp_server_list;

modal_dialog* unit_test_mp_server_list()
{
	return mp_connect::mp_server_list_for_unit_test();
}

} // namespace dialogs
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
	struct dialog_tester
	{
		T* create() { return new T(); }
	};

	typedef std::pair<unsigned, unsigned> resolution;
	typedef std::vector<std::pair<unsigned, unsigned> > resolution_list;

	template<class T>
	void test_resolutions(const resolution_list& resolutions)
	{
		for(const resolution& resolution : resolutions) {
			CVideo& video = test_utils::get_fake_display(resolution.first, resolution.second).video();

			dialog_tester<T> ctor;
			const std::unique_ptr<modal_dialog> dlg(ctor.create());
			BOOST_REQUIRE_MESSAGE(dlg.get(), "Failed to create a dialog.");

			const std::string id = unit_test_mark_as_tested(*(dlg.get()));

			std::string exception;
			try {
				dlg->show(video, 1);
			} catch(gui2::layout_exception_width_modified&) {
				exception = "gui2::layout_exception_width_modified";
			} catch(gui2::layout_exception_width_resize_failed&) {
				exception = "gui2::layout_exception_width_resize_failed";
			} catch(gui2::layout_exception_height_resize_failed&) {
				exception = "gui2::layout_exception_height_resize_failed";
			} catch(wml_exception& e) {
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
	void test_popup_resolutions(const resolution_list& resolutions)
	{
		bool interact = false;
		for(int i = 0; i < 2; ++i) {
			for(const resolution& resolution : resolutions) {
				CVideo& video = test_utils::get_fake_display(resolution.first, resolution.second).video();

				dialog_tester<T> ctor;
				const std::unique_ptr<modeless_dialog> dlg(ctor.create());
				BOOST_REQUIRE_MESSAGE(dlg.get(), "Failed to create a dialog.");

				const std::string id = unit_test_mark_popup_as_tested(*(dlg.get()));

				std::string exception;
				try {
					dlg->show(video, interact);
					gui2::window* window = unit_test_window((*dlg.get()));
					BOOST_REQUIRE_NE(window, static_cast<void*>(nullptr));
					window->draw();
				} catch(gui2::layout_exception_width_modified&) {
					exception = "gui2::layout_exception_width_modified";
				} catch(gui2::layout_exception_width_resize_failed&) {
					exception = "gui2::layout_exception_width_resize_failed";
				} catch(gui2::layout_exception_height_resize_failed&) {
					exception = "gui2::layout_exception_height_resize_failed";
				} catch(wml_exception& e) {
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
	void test_tip_resolutions(const resolution_list& resolutions
			, const std::string& id)
	{
		for(const resolution& resolution : resolutions) {
			
			CVideo& video = test_utils::get_fake_display(resolution.first, resolution.second).video();

			std::vector<std::string>& list =
					gui2::unit_test_registered_window_list();
			list.erase(std::remove(list.begin(), list.end(), id), list.end());

			std::string exception;
			try {
				tip::show(video
						, id
						, "Test messsage for a tooltip."
						, gui2::point(0, 0)
						, {0,0,0,0});
			} catch(gui2::layout_exception_width_modified&) {
				exception = "gui2::layout_exception_width_modified";
			} catch(gui2::layout_exception_width_resize_failed&) {
				exception = "gui2::layout_exception_width_resize_failed";
			} catch(gui2::layout_exception_height_resize_failed&) {
				exception = "gui2::layout_exception_height_resize_failed";
			} catch(wml_exception& e) {
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

const resolution_list& get_gui_resolutions()
{
	static resolution_list result {
		{800,  600},
		{1024, 768},
		{1280, 1024},
		{1680, 1050},
	};

	return result;
}

template<class T>
void test()
{
	gui2::new_widgets = false;

//	for(size_t i = 0; i < 2; ++i) {

		test_resolutions<T>(get_gui_resolutions());

//		break; // FIXME: New widgets break
//		gui2::new_widgets = true;
//	}
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

	/* The modal_dialog classes. */
	test<addon_connect>();
	//test<addon_manager>();
	test<advanced_graphics_options>();
	//test<attack_predictions>();
	test<campaign_difficulty>();
	test<campaign_selection>();
	test<campaign_settings>();
	test<chat_log>();
	test<core_selection>();
	test<custom_tod>();
	test<depcheck_confirm_change>();
	test<depcheck_select_new>();
	test<edit_label>();
	test<edit_text>();
	test<editor_edit_label>();
	test<editor_edit_side>();
	test<editor_edit_scenario>();
	test<editor_generate_map>();
	test<editor_new_map>();
	test<editor_resize_map>();
	test<editor_set_starting_position>();
	//test<end_credits>();
	test<faction_select>();
	test<file_dialog>();
	test<folder_create>();
	test<formula_debugger>();
	test<game_cache_options>();
	test<game_delete>();
	test<game_load>();
	test<game_version>();
	test<game_save>();
	test<game_save_message>();
	test<game_save_oos>();
	test<game_stats>();
	test<gamestate_inspector>();
	test<generator_settings>();
	//test<help_browser>();
	test<hotkey_bind>();
	test<install_dependencies>();
	test<language_selection>();
	// test<loading_screen>(); TODO: enable
	test<mp_lobby>();
	test<lobby_player_info>();
	test<log_settings>();
	//test<lua_interpreter>(& lua_kernel_base());
	test<message>();
	test<mp_alerts_options>();
	//test<mp_change_control>();
	test<mp_cmd_wrapper>();
	test<mp_connect>();
	//test<mp_create_game>();
	//test<mp_join_game>();
	test<mp_join_game_password_prompt>();
	test<mp_login>();
	test<mp_method_selection>();
	test<mp_server_list>();
	//test<mp_staging>();
	//test<outro>();
	test<simple_item_selector>();
	test<screenshot_notification>();
	test<select_orb_colors>();
	test<sp_options_configure>();
	test<statistics_dialog>();
	//test<story_viewer>();
	test<theme_list>();
	//test<terrain_layers>();
	//test<title_screen>();
	test<transient_message>();
	//test<unit_advance>();
	//test<unit_attack>();
	test<unit_create>();
	//test<unit_list>();
	//test<unit_recall>();
	//test<unit_recruit>();
	test<wml_error>();
	test<wml_message_left>();
	test<wml_message_right>();
	test<wml_message_double>();

	/* The modeless_dialog classes. */
	test_popup<debug_clock>();

	/* The tooltip classes. */
	test_tip("tooltip_large");
	test_tip("tooltip");

	std::vector<std::string>& list = gui2::unit_test_registered_window_list();
	std::vector<std::string> omitted {
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
		"addon_uninstall_list",
		"addon_manager",
		"loading_screen",
		"network_transmission",
		"synched_choice_wait",
		"drop_down_menu",
		"preferences_dialog",
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
		"terrain_layers",
		"attack_predictions",
		"help_browser",
		"story_viewer",
		"outro",
		"mp_change_control", // Basically useless without a game_board object, so disabling
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
		message dlg("title", "message", true, false);
		dlg.show(video, 1);
	} catch(wml_exception& e) {
		BOOST_CHECK(e.user_message == _("Failed to show a dialog, "
					"which doesn't fit on the screen."));
		return;
	} catch(...) {
	}
	BOOST_ERROR("Didn't catch the wanted exception.");
}

namespace {

template<>
struct dialog_tester<addon_connect>
{
	std::string host_name = "host_name";
	addon_connect* create()
	{
		return new addon_connect(host_name, true);
	}
};

template<>
struct dialog_tester<addon_manager>
{
	CVideo& video = test_utils::get_fake_display(10, 10).video();
	dialog_tester()
	{
	}
	addon_manager* create()
	{
		addons_client client(video, "localhost:15999");
		return new addon_manager(client);
	}
};

template<>
struct dialog_tester<campaign_difficulty>
{
	campaign_difficulty* create()
	{
		const config items("difficulty");

		return new campaign_difficulty(items);
	}
};

template<>
struct dialog_tester<campaign_selection>
{
	saved_game state;
	ng::create_engine ng;
	dialog_tester() : state(config_of("campaign_type", "scenario")), ng(test_utils::get_fake_display(-1, -1).video(), state)
	{
	}
	campaign_selection* create()
	{
		return new campaign_selection(ng);
	}
};

template<>
struct dialog_tester<campaign_settings>
{
	saved_game state;
	ng::create_engine ng;
	dialog_tester() : state(config_of("campaign_type", "scenario")), ng(test_utils::get_fake_display(-1, -1).video(), state)
	{
	}
	campaign_settings* create()
	{
		return new campaign_settings(ng);
	}
};

template<>
struct dialog_tester<chat_log>
{
	config cfg;
	vconfig vcfg;
	replay_recorder_base rbase;
	replay r;
	dialog_tester() : vcfg(cfg), r(rbase) {}
	chat_log* create()
	{
		return new chat_log(vcfg, r);
	}
};

template<>
struct dialog_tester<core_selection>
{
	std::vector<config> cores;
	dialog_tester()
	{
		cores.resize(1);
	}
	core_selection* create()
	{
		return new core_selection(cores, 0);
	}
};

template<>
struct dialog_tester<custom_tod>
{
	std::vector<time_of_day> times;
	int current_tod = 0;
	dialog_tester()
	{
		times.resize(1);
	}
	custom_tod* create()
	{
		return new custom_tod(times, current_tod);
	}
};

template<>
struct dialog_tester<edit_label>
{
	std::string label = "Label text to modify";
	bool team_only = false;
	edit_label* create()
	{
		return new edit_label(label, team_only);
	}
};

template<>
struct dialog_tester<edit_text>
{
	std::string text = "text to modify";
	edit_text* create()
	{
		return new edit_text("title", "label", text);
	}
};

template<>
struct dialog_tester<editor_edit_label>
{
	std::string label = "Label text to modify";
	std::string category = "test";
	bool immutable = false, fog = false, shroud = false;
	color_t color;
	editor_edit_label* create()
	{
		return new editor_edit_label(label, immutable, fog, shroud, color, category);
	}
};

template<>
struct dialog_tester<editor_edit_scenario>
{
	std::string id, name, descr;
	int turns = 0, xp_mod = 50;
	bool defeat_enemies = false, random_start = false;
	editor_edit_scenario* create()
	{
		return new editor_edit_scenario(id, name, descr, turns, xp_mod, defeat_enemies, random_start);
	}
};

template<>
struct dialog_tester<editor_edit_side>
{
	team t;
	editor::editor_team_info info;
	dialog_tester() : info(t) {}
	editor_edit_side* create()
	{
		return new editor_edit_side(info);
	}
};

template<>
struct dialog_tester<formula_debugger>
{
	wfl::formula_debugger debugger;
	formula_debugger* create()
	{
		return new formula_debugger(debugger);
	}
};

template<>
struct dialog_tester<game_load>
{
	config cfg;
	savegame::load_game_metadata data;
	dialog_tester()
	{
		/** @todo Would be nice to add real data to the config. */
	}
	game_load* create()
	{
		return new game_load(cfg, data);
	}

};

template<>
struct dialog_tester<game_save>
{
	std::string title = "Title";
	std::string filename = "filename";
	game_save* create()
	{
		return new game_save(title, filename);
	}

};

template<>
struct dialog_tester<game_save_message>
{
	std::string title = "Title";
	std::string filename = "filename";
	std::string message = "message";
	game_save_message* create()
	{
		return new game_save_message(title, filename, message);
	}

};

template<>
struct dialog_tester<game_save_oos>
{
	bool ignore_all = false;
	std::string title = "Title";
	std::string filename = "filename";
	std::string message = "message";
	game_save_oos* create()
	{
		return new game_save_oos(ignore_all, title, filename, message);
	}

};

template<>
struct dialog_tester<gamestate_inspector>
{
	config vars;
	game_events::manager events;
	gamestate_inspector* create()
	{
		const display_context* dc = editor::get_dummy_display_context();
		return new gamestate_inspector(vars, events, *dc, "Unit Test");
	}

};

template<>
struct dialog_tester<install_dependencies>
{
	addons_list addons;
	install_dependencies* create()
	{
		return new install_dependencies(addons);
	}
};

template<>
struct dialog_tester<hotkey_bind>
{
	std::string id = "";

	hotkey_bind* create()
	{
		return new hotkey_bind(id);
	}
};

struct wesnothd_connection_init
{
	wesnothd_connection_init(wesnothd_connection& conn)
	{
		//Swallow the 'cannot connect' execption so that the connection object doesn't throw while we test the dialog.
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
struct dialog_tester<mp_lobby>
{
	config game_config;
	wesnothd_connection connection;
	wesnothd_connection_init init;
	std::vector<std::string> installed_addons;
	mp::lobby_info li;
	dialog_tester() : connection("", ""), init(connection), li(game_config, installed_addons)
	{
	}
	mp_lobby* create()
	{
		return new mp_lobby(game_config, li, connection);
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
struct dialog_tester<lobby_player_info>
{
	config c;
	fake_chat_handler ch;
	wesnothd_connection connection;
	wesnothd_connection_init init;
	mp::user_info ui;
	std::vector<std::string> installed_addons;
	mp::lobby_info li;
	dialog_tester()
		: connection("", ""), init(connection)
		, ui(c), li(c, installed_addons)
	{
	}
	lobby_player_info* create()
	{
		return new lobby_player_info(ch, ui, li);
	}
};

template<>
struct dialog_tester<log_settings>
{
	log_settings* create()
	{
		return new log_settings();
	}
};

template<>
struct dialog_tester<message>
{
	message* create()
	{
		return new message("Title", "Message", false, false);
	}
};
#if 0
template<>
struct dialog_tester<mp_change_control>
{
	mp_change_control* create()
	{
		return new mp_change_control(nullptr);
	}
};
#endif
template<>
struct dialog_tester<mp_cmd_wrapper>
{
	mp_cmd_wrapper* create()
	{
		return new mp_cmd_wrapper("foo");
	}
};

template<>
struct dialog_tester<mp_create_game>
{
	saved_game state;
	ng::create_engine engine;
	dialog_tester() : state(config_of("campaign_type", "multiplayer")), engine(test_utils::get_fake_display(-1, -1).video(), state)
	{
	}
	mp_create_game* create()
	{
		return new mp_create_game(main_config, engine);
	}
};

template<>
struct dialog_tester<mp_join_game_password_prompt>
{
	std::string password;
	mp_join_game_password_prompt* create()
	{
		return new mp_join_game_password_prompt(password);
	}
};

static std::vector<std::string> depcheck_mods {"mod_one", "some other", "more"};

template<>
struct dialog_tester<depcheck_confirm_change>
{
	depcheck_confirm_change* create()
	{
		return new depcheck_confirm_change(true, depcheck_mods, "requester");
	}
};

template<>
struct dialog_tester<depcheck_select_new>
{
	depcheck_select_new* create()
	{
		return new depcheck_select_new(ng::depcheck::MODIFICATION, depcheck_mods);
	}
};

template<>
struct dialog_tester<mp_login>
{
	mp_login* create()
	{
		return new mp_login("label", true);
	}
};

template<>
struct dialog_tester<simple_item_selector>
{
	simple_item_selector* create()
	{
		return new simple_item_selector("title", "message", std::vector<std::string>(), false, false);
	}
};

template<>
struct dialog_tester<screenshot_notification>
{
	screenshot_notification* create()
	{
		return new screenshot_notification("path");
	}
};

template<>
struct dialog_tester<theme_list>
{
	static theme_info make_theme(std::string name)
	{
		theme_info ti;
		ti.id = name;
		ti.name = name;
		ti.description = name + " this is a description";
		return ti;
	}
	static std::vector<theme_info> themes;
	theme_list* create()
	{
		return new theme_list(themes, 0);
	}
};
std::vector<theme_info> dialog_tester<theme_list>::themes {make_theme("classic"), make_theme("new"), make_theme("more"), make_theme("themes")};

template<>
struct dialog_tester<editor_generate_map>
{
	editor_generate_map* create()
	{
		editor_generate_map* result = new editor_generate_map();
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
struct dialog_tester<editor_new_map>
{
	int width = 10;
	int height = 10;
	editor_new_map* create()
	{
		return new editor_new_map(width, height);
	}
};

template<>
struct dialog_tester<editor_set_starting_position>
{
	std::vector<map_location> locations;
	editor_set_starting_position* create()
	{
		return new editor_set_starting_position(0, 0, locations);
	}
};

template<>
struct dialog_tester<editor_resize_map>
{
	int width = 0;
	int height = 0;
	editor_resize_map::EXPAND_DIRECTION expand_direction = editor_resize_map::EXPAND_TOP;
	bool copy = false;
	editor_resize_map* create()
	{
		return new editor_resize_map(width, height, expand_direction, copy);
	}
};

template<>
struct dialog_tester<file_dialog>
{
	file_dialog* create()
	{
		return new file_dialog();
	}
};

template<>
struct dialog_tester<folder_create>
{
	std::string folder_name;
	folder_create* create()
	{
		return new folder_create(folder_name);
	}
};

template<>
struct dialog_tester<mp_server_list>
{
	static modal_dialog* create()
	{
		return unit_test_mp_server_list();
	}
};

template<>
struct dialog_tester<transient_message>
{
	transient_message* create()
	{
		return new transient_message("Title", false, "Message", false, "");
	}
};

template<>
struct dialog_tester<title_screen>
{
	std::vector<std::string> args;
	commandline_options opts;
	game_launcher game;
	dialog_tester() : opts(args), game(opts, "unit_tests") {}
	title_screen* create()
	{
		return new title_screen(game);
	}
};

template<>
struct dialog_tester<wml_error>
{
	static std::vector<std::string> files;
	wml_error* create()
	{
		return new wml_error("Summary", "Post summary", files, "Details");
	}
};
std::vector<std::string> dialog_tester<wml_error>::files {"some", "files", "here"};

template<>
struct dialog_tester<wml_message_left>
{
	wml_message_left* create()
	{
		return new wml_message_left("Title", "Message", "", false);
	}
};

template<>
struct dialog_tester<wml_message_right>
{
	wml_message_right* create()
	{
		return new wml_message_right("Title", "Message", "", false);
	}
};

template<>
struct dialog_tester<wml_message_double>
{
	wml_message_double* create()
	{
		return new wml_message_double("Title", "Message", "", false, "", true);
	}
};

template<>
struct dialog_tester<faction_select>
{
	config era_cfg, side_cfg;
	std::vector<const config*> eras;
	ng::flg_manager flg;
	std::string color;
	dialog_tester()
		: era_cfg(), side_cfg(), eras(1, &era_cfg) // TODO: Add an actual era definition
		, flg(eras, side_cfg, false, false, false)
		, color("teal")
	{}
	faction_select* create() {
		return new faction_select(flg, color, 1);
	}
};

template<>
struct dialog_tester<game_stats>
{
	int i = 1;
	game_stats* create()
	{
		const display_context* ctx = editor::get_dummy_display_context();
		return new game_stats(*ctx, 1, i);
	}
};

template<>
struct dialog_tester<generator_settings>
{
	config cfg;
	generator_data data;
	dialog_tester() : data(cfg) {}
	generator_settings* create()
	{
		return new generator_settings(data);
	}
};

template<>
struct dialog_tester<sp_options_configure>
{
	saved_game state;
	ng::create_engine create_eng;
	ng::configure_engine config_eng;
	dialog_tester() : create_eng(test_utils::get_fake_display(-1, -1).video(), state)
		, config_eng(create_eng.get_state()) {}
	sp_options_configure* create()
	{
		return new sp_options_configure(create_eng, config_eng);
	}
};

template<>
struct dialog_tester<statistics_dialog>
{
	team t;
	dialog_tester() : t() {}
	statistics_dialog* create()
	{
		return new statistics_dialog(t);
	}
};

} // namespace

