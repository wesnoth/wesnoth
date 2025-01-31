/*
	Copyright (C) 2009 - 2024
	by Mark de Wever <koraq@xs4all.nl>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

#include "addon/client.hpp"
#include "addon/info.hpp"
#include "config_cache.hpp"
#include "filesystem.hpp"
#include "formula/debugger.hpp"
#include "game_config.hpp"
#include "game_config_manager.hpp"
#include "game_config_view.hpp"
#include "game_display.hpp"
#include "game_events/manager.hpp"
#include "game_initialization/create_engine.hpp"
#include "game_initialization/lobby_data.hpp"
#include "game_initialization/lobby_info.hpp"
#include "game_launcher.hpp"
#include "generators/map_create.hpp"
#include "gettext.hpp"
#include "gui/core/layout_exception.hpp"
#include "gui/dialogs/addon/addon_auth.hpp"
#include "gui/dialogs/addon/addon_server_info.hpp"
#include "gui/dialogs/addon/connect.hpp"
#include "gui/dialogs/addon/install_dependencies.hpp"
#include "gui/dialogs/addon/license_prompt.hpp"
#include "gui/dialogs/addon/manager.hpp"
#include "gui/dialogs/achievements_dialog.hpp"
#include "gui/dialogs/attack_predictions.hpp"
#include "gui/dialogs/campaign_difficulty.hpp"
#include "gui/dialogs/campaign_selection.hpp"
#include "gui/dialogs/chat_log.hpp"
#include "gui/dialogs/core_selection.hpp"
#include "gui/dialogs/debug_clock.hpp"
#include "gui/dialogs/depcheck_confirm_change.hpp"
#include "gui/dialogs/depcheck_select_new.hpp"
#include "gui/dialogs/edit_label.hpp"
#include "gui/dialogs/edit_text.hpp"
#include "gui/dialogs/editor/choose_addon.hpp"
#include "gui/dialogs/editor/custom_tod.hpp"
#include "gui/dialogs/editor/edit_label.hpp"
#include "gui/dialogs/editor/edit_pbl.hpp"
#include "gui/dialogs/editor/edit_pbl_translation.hpp"
#include "gui/dialogs/editor/edit_scenario.hpp"
#include "gui/dialogs/editor/edit_side.hpp"
#include "gui/dialogs/editor/edit_unit.hpp"
#include "gui/dialogs/editor/generate_map.hpp"
#include "gui/dialogs/editor/generator_settings.hpp"
#include "gui/dialogs/editor/new_map.hpp"
#include "gui/dialogs/editor/resize_map.hpp"
#include "gui/dialogs/editor/tod_new_schedule.hpp"
#include "gui/dialogs/end_credits.hpp"
#include "gui/dialogs/file_dialog.hpp"
#include "gui/dialogs/folder_create.hpp"
#include "gui/dialogs/formula_debugger.hpp"
#include "gui/dialogs/game_cache_options.hpp"
#include "gui/dialogs/game_delete.hpp"
#include "gui/dialogs/game_load.hpp"
#include "gui/dialogs/game_save.hpp"
#include "gui/dialogs/game_stats.hpp"
#include "gui/dialogs/game_version_dialog.hpp"
#include "gui/dialogs/gamestate_inspector.hpp"
#include "gui/dialogs/gui_test_dialog.hpp"
#include "gui/dialogs/help_browser.hpp"
#include "gui/dialogs/hotkey_bind.hpp"
#include "gui/dialogs/label_settings.hpp"
#include "gui/dialogs/language_selection.hpp"
#include "gui/dialogs/loading_screen.hpp"
#include "gui/dialogs/log_settings.hpp"
#include "gui/dialogs/lua_interpreter.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/migrate_version_selection.hpp"
#include "gui/dialogs/multiplayer/faction_select.hpp"
#include "gui/dialogs/multiplayer/lobby.hpp"
#include "gui/dialogs/multiplayer/mp_alerts_options.hpp"
#include "gui/dialogs/multiplayer/mp_change_control.hpp"
#include "gui/dialogs/multiplayer/mp_connect.hpp"
#include "gui/dialogs/multiplayer/mp_create_game.hpp"
#include "gui/dialogs/multiplayer/mp_join_game.hpp"
#include "gui/dialogs/multiplayer/mp_join_game_password_prompt.hpp"
#include "gui/dialogs/multiplayer/mp_login.hpp"
#include "gui/dialogs/multiplayer/match_history.hpp"
#include "gui/dialogs/multiplayer/mp_method_selection.hpp"
#include "gui/dialogs/multiplayer/mp_report.hpp"
#include "gui/dialogs/multiplayer/mp_staging.hpp"
#include "gui/dialogs/multiplayer/player_info.hpp"
#include "gui/dialogs/outro.hpp"
#include "gui/dialogs/prompt.hpp"
#include "gui/dialogs/screenshot_notification.hpp"
#include "gui/dialogs/select_orb_colors.hpp"
#include "gui/dialogs/simple_item_selector.hpp"
#include "gui/dialogs/sp_options_configure.hpp"
#include "gui/dialogs/statistics_dialog.hpp"
#include "gui/dialogs/story_viewer.hpp"
#include "gui/dialogs/surrender_quit.hpp"
#include "gui/dialogs/terrain_layers.hpp"
#include "gui/dialogs/theme_list.hpp"
#include "gui/dialogs/title_screen.hpp"
#include "gui/dialogs/tooltip.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/dialogs/unit_advance.hpp"
#include "gui/dialogs/unit_attack.hpp"
#include "gui/dialogs/units_dialog.hpp"
#include "gui/dialogs/wml_error.hpp"
#include "gui/dialogs/wml_message.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "language.hpp"
#include "map/map.hpp"
#include "replay.hpp"
#include "save_index.hpp"
#include "saved_game.hpp"
#include "serialization/string_utils.hpp"
#include "terrain/type_data.hpp"
#include "tests/utils/fake_display.hpp"
#include "utils/general.hpp"
#include "wesnothd_connection.hpp"
#include "wml_exception.hpp"

#include <boost/test/unit_test.hpp>

#include <functional>
#include <memory>

using namespace gui2::dialogs;

struct test_gui2_fixture {
	test_gui2_fixture()
	: config_manager()
	, dummy_args({"wesnoth", "--noaddons"})
	{
		/** The main config, which contains the entire WML tree. */
		game_config_view game_config_view_ = game_config_view::wrap(main_config);
		config_manager.reset(new game_config_manager(dummy_args));

		game_config::config_cache& cache = game_config::config_cache::instance();

		cache.clear_defines();
		cache.add_define("EDITOR");
		cache.add_define("MULTIPLAYER");
		cache.get_config(game_config::path +"/data", main_config);

		const filesystem::binary_paths_manager bin_paths_manager(game_config_view_);

		load_language_list();
		game_config::load_config(main_config.mandatory_child("game_config"));
	}
	~test_gui2_fixture()
	{
	}
	static config main_config;
	static const std::string widgets_file;
	std::unique_ptr<game_config_manager> config_manager;
	std::vector<std::string> dummy_args;
};
config test_gui2_fixture::main_config;
const std::string test_gui2_fixture::widgets_file = "widgets_tested.log";

namespace gui2 {

namespace dialogs {

std::string get_modal_dialog_id(const modal_dialog& dialog)
{
	return dialog.window_id();
}

std::string get_modeless_dialog_id(const modeless_dialog& dialog)
{
	return dialog.window_id();
}

} // namespace dialogs
} // namespace gui2

namespace {

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
	typedef std::vector<std::pair<unsigned, unsigned>> resolution_list;

	template<class T>
	void test_resolutions(const resolution_list& resolutions)
	{
		for(const resolution& resolution : resolutions) {
			test_utils::get_fake_display(resolution.first, resolution.second);

			dialog_tester<T> ctor;
			const std::unique_ptr<modal_dialog> dlg(ctor.create());
			BOOST_REQUIRE_MESSAGE(dlg.get(), "Failed to create a dialog.");

			std::string id = get_modal_dialog_id(*dlg.get());
			filesystem::write_file(test_gui2_fixture::widgets_file, ","+id, std::ios_base::app);

			std::string exception;
			try {
				dlg->show(1);
			} catch(const gui2::layout_exception_width_modified&) {
				exception = "gui2::layout_exception_width_modified";
			} catch(const gui2::layout_exception_width_resize_failed&) {
				exception = "gui2::layout_exception_width_resize_failed";
			} catch(const gui2::layout_exception_height_resize_failed&) {
				exception = "gui2::layout_exception_height_resize_failed";
			} catch(const wml_exception& e) {
				exception = e.dev_message;
			} catch(const std::exception& e) {
				exception = e.what();
			} catch(...) {
				exception = utils::get_unknown_exception_type();
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
				// debug clock doesn't work at 800x600
				if(resolution.first == 800 && resolution.second == 600) {
					continue;
				}
				test_utils::get_fake_display(resolution.first, resolution.second);

				dialog_tester<T> ctor;
				const std::unique_ptr<modeless_dialog> dlg(ctor.create());
				BOOST_REQUIRE_MESSAGE(dlg.get(), "Failed to create a dialog.");

				std::string id = get_modeless_dialog_id(*dlg.get());
				filesystem::write_file(test_gui2_fixture::widgets_file, ","+id, std::ios_base::app);

				std::string exception;
				try {
					dlg->show(interact);
					gui2::window* window = dlg.get();
					BOOST_REQUIRE_NE(window, static_cast<void*>(nullptr));
					window->draw();
				} catch(const gui2::layout_exception_width_modified&) {
					exception = "gui2::layout_exception_width_modified";
				} catch(const gui2::layout_exception_width_resize_failed&) {
					exception = "gui2::layout_exception_width_resize_failed";
				} catch(const gui2::layout_exception_height_resize_failed&) {
					exception = "gui2::layout_exception_height_resize_failed";
				} catch(const wml_exception& e) {
					exception = e.dev_message;
				} catch(const std::exception& e) {
					exception = e.what();
				} catch(...) {
					exception = utils::get_unknown_exception_type();
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
		for(const auto& resolution : resolutions) {
			test_utils::get_fake_display(resolution.first, resolution.second);

			filesystem::write_file(test_gui2_fixture::widgets_file, ","+id, std::ios_base::app);

			std::string exception;
			try {
				tip::show(id
						, "Test message for a tooltip."
						, point(0, 0)
						, {0,0,0,0});
				tip::remove();
			} catch(const gui2::layout_exception_width_modified&) {
				exception = "gui2::layout_exception_width_modified";
			} catch(const gui2::layout_exception_width_resize_failed&) {
				exception = "gui2::layout_exception_width_resize_failed";
			} catch(const gui2::layout_exception_height_resize_failed&) {
				exception = "gui2::layout_exception_height_resize_failed";
			} catch(const wml_exception& e) {
				exception = e.dev_message;
			} catch(const std::exception& e) {
				exception = e.what();
			} catch(...) {
				exception = utils::get_unknown_exception_type();
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

//	for(std::size_t i = 0; i < 2; ++i) {

		test_resolutions<T>(get_gui_resolutions());

//		break; // FIXME: New widgets break
//		gui2::new_widgets = true;
//	}
}

template<class T>
void test_popup()
{
	gui2::new_widgets = false;

	for(std::size_t i = 0; i < 2; ++i) {

		test_popup_resolutions<T>(get_gui_resolutions());

		gui2::new_widgets = true;
	}
}

void test_tip(const std::string& id)
{
	gui2::new_widgets = false;

	for(std::size_t i = 0; i < 2; ++i) {

		test_tip_resolutions(get_gui_resolutions(), id);

		gui2::new_widgets = true;
	}
}

} // namespace

BOOST_FIXTURE_TEST_SUITE( test_gui2, test_gui2_fixture )

BOOST_AUTO_TEST_CASE(modal_dialog_test_addon_auth)
{
	test<addon_auth>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_addon_connect)
{
	test<addon_connect>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_addon_license_prompt)
{
	test<addon_license_prompt>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_campaign_difficulty)
{
	test<campaign_difficulty>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_chat_log)
{
	test<chat_log>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_editor_choose_addon)
{
	test<editor_choose_addon>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_prompt)
{
	test<prompt>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_core_selection)
{
	test<core_selection>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_custom_tod)
{
	test<custom_tod>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_depcheck_confirm_change)
{
	test<depcheck_confirm_change>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_depcheck_select_new)
{
	test<depcheck_select_new>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_edit_label)
{
	test<edit_label>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_editor_edit_pbl)
{
	test<editor_edit_pbl>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_editor_edit_pbl_translation)
{
	test<editor_edit_pbl_translation>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_edit_text)
{
	test<edit_text>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_editor_edit_label)
{
	test<editor_edit_label>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_editor_edit_side)
{
	test<editor_edit_side>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_editor_edit_scenario)
{
	test<editor_edit_scenario>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_editor_generate_map)
{
	test<editor_generate_map>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_editor_new_map)
{
	test<editor_new_map>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_editor_resize_map)
{
	test<editor_resize_map>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_faction_select)
{
	test<faction_select>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_file_dialog)
{
	test<file_dialog>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_folder_create)
{
	test<folder_create>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_formula_debugger)
{
	test<formula_debugger>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_game_cache_options)
{
	test<game_cache_options>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_game_delete)
{
	test<game_delete>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_game_version)
{
	test<game_version>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_game_save)
{
	test<game_save>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_game_save_message)
{
	test<game_save_message>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_game_save_oos)
{
	test<game_save_oos>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_generator_settings)
{
	test<generator_settings>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_gui_test_dialog)
{
	test<gui_test_dialog>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_hotkey_bind)
{
	test<hotkey_bind>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_install_dependencies)
{
	test<install_dependencies>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_language_selection)
{
	test<language_selection>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_mp_lobby)
{
	test<mp_lobby>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_lobby_player_info)
{
	test<lobby_player_info>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_log_settings)
{
	test<log_settings>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_message)
{
	test<message>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_mp_alerts_options)
{
	test<mp_alerts_options>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_mp_connect)
{
	test<mp_connect>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_mp_join_game_password_prompt)
{
	test<mp_join_game_password_prompt>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_mp_login)
{
	test<mp_login>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_mp_method_selection)
{
	test<mp_method_selection>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_mp_report)
{
	test<mp_report>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_simple_item_selector)
{
	test<simple_item_selector>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_screenshot_notification)
{
	test<screenshot_notification>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_select_orb_colors)
{
	test<select_orb_colors>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_statistics_dialog)
{
	test<statistics_dialog>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_surrender_quit)
{
	test<surrender_quit>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_theme_list)
{
	test<theme_list>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_transient_message)
{
	test<transient_message>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_wml_error)
{
	test<wml_error>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_wml_message_left)
{
	test<wml_message_left>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_wml_message_right)
{
	test<wml_message_right>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_wml_message_double)
{
	test<wml_message_double>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_achievements_dialog)
{
	test<achievements_dialog>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_addon_server_info)
{
	test<addon_server_info>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_mp_match_history_dialog)
{
	test<mp_match_history>();
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_migrate_version_selection_dialog)
{
	test<gui2::dialogs::migrate_version_selection>();
}
BOOST_AUTO_TEST_CASE(modeless_dialog_test_debug_clock)
{
	test_popup<debug_clock>();
}
BOOST_AUTO_TEST_CASE(tooltip_test_tooltip_large)
{
	test_tip("tooltip_large");
}
BOOST_AUTO_TEST_CASE(tooltip_test_tooltip)
{
	test_tip("tooltip");
}
BOOST_AUTO_TEST_CASE(modal_dialog_test_tod_new_schedule)
{
	test<tod_new_schedule>();
}

BOOST_AUTO_TEST_CASE(modal_dialog_test_editor_edit_unit)
{
	test<editor_edit_unit>();
}

// execute last - checks that there aren't any unaccounted for GUIs
BOOST_AUTO_TEST_CASE(test_last)
{
	std::set<std::string> widget_list = gui2::registered_window_types();
	std::vector<std::string> widgets_tested = utils::split(filesystem::read_file(test_gui2_fixture::widgets_file));
	std::set<std::string> omitted {
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
		"units_dialog",
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
		"game_stats", // segfault with LTO
		"gamestate_inspector", // segfault with LTO
		"server_info",
		"sp_options_configure",// segfault with LTO
		"campaign_selection",// segfault with LTO
		"game_load",// segfault after disabling the above tests
		"file_progress",
	};
	filesystem::delete_file(test_gui2_fixture::widgets_file);

	for(const std::string& item : widgets_tested)
	{
		widget_list.erase(item);
		PLAIN_LOG << "Checking widget " << item;
		BOOST_CHECK_EQUAL(omitted.count(item), 0);
	}
	for(const std::string& item : omitted)
	{
		widget_list.erase(item);
	}

	// Test size() instead of empty() to get the number of offenders
	BOOST_CHECK_EQUAL(widget_list.size(), 0);
	for(const std::string& id : widget_list) {
		PLAIN_LOG << "Window '" << id << "' registered but not tested.";
	}
}

BOOST_AUTO_TEST_CASE(test_make_test_fake)
{
	test_utils::get_fake_display(10, 10);

	try {
		message dlg("title", "message", true, false, false);
		dlg.show(1);
	} catch(const wml_exception& e) {
		BOOST_CHECK(e.user_message == _("Failed to show a dialog, which doesn’t fit on the screen."));
		return;
	} catch(...) {
		BOOST_ERROR("Didn't catch the wanted exception, instead caught " << utils::get_unknown_exception_type() << ".");
	}
	BOOST_ERROR("Didn't catch the wanted exception, instead caught nothing.");
}

BOOST_AUTO_TEST_SUITE_END()

namespace {

template<>
struct dialog_tester<addon_server_info>
{
	std::string s = "";
	bool b = false;
	addon_server_info* create()
	{
		addons_client client("localhost:15999");
		return new addon_server_info(client, s, b);
	}
};

template<>
struct dialog_tester<addon_auth>
{
	config cfg;
	addon_auth* create()
	{
		return new addon_auth(cfg);
	}
};

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
struct dialog_tester<addon_license_prompt>
{
	std::string license_terms = R"(Lorem ipsum dolor sit amet, consectetur adipiscing elit. Duis ante nibh, dignissim ullamcorper tristique eget, condimentum sit amet enim. Aenean dictum pulvinar lacinia. Etiam eleifend, leo sed efficitur consectetur, augue nulla ornare lectus, vitae molestie lacus risus vitae libero. Quisque odio nunc, porttitor eget fermentum sit amet, faucibus eu risus. Praesent sit amet lacus tortor. Suspendisse volutpat quam vitae ipsum fermentum, in vulputate metus egestas. Nulla id consequat ex. Nulla ac dignissim nisl, nec euismod lectus. Duis vitae dolor ornare, convallis justo in, porta dui.

Sed faucibus nibh sit amet ligula porta, non malesuada nibh tristique. Maecenas aliquam diam non eros convallis mattis. Proin rhoncus condimentum leo, sed condimentum magna. Phasellus cursus condimentum lacus, sed sodales lacus. Sed pharetra dictum metus, eget dictum nibh lobortis imperdiet. Nunc tempus sollicitudin bibendum. In porttitor interdum orci. Curabitur vitae nibh vestibulum, condimentum lectus quis, condimentum dui. In quis cursus nisl. Maecenas semper neque eu ipsum aliquam, id porta ligula lacinia. Integer sed blandit ex, eu accumsan magna.)";
	addon_license_prompt* create()
	{
		return new addon_license_prompt(license_terms);
	}
};

template<>
struct dialog_tester<addon_manager>
{
	dialog_tester()
	{
		test_utils::get_fake_display(10, 10);
	}
	addon_manager* create()
	{
		addons_client client("localhost:15999");
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
	dialog_tester() : state(config {"campaign_type", "scenario"}), ng(state)
	{
	}
	campaign_selection* create()
	{
		return new campaign_selection(ng);
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
struct dialog_tester<editor_choose_addon>
{
	std::string temp;
	editor_choose_addon* create()
	{
		return new editor_choose_addon(temp);
	}
};

template<>
struct dialog_tester<prompt>
{
	std::string temp;
	prompt* create()
	{
		return new prompt(temp);
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
struct dialog_tester<editor_edit_pbl>
{
	std::string temp;
	std::string temp1;
	editor_edit_pbl* create()
	{
		return new editor_edit_pbl(temp, temp1);
	}
};

template<>
struct dialog_tester<editor_edit_pbl_translation>
{
	std::string temp1;
	std::string temp2;
	std::string temp3;
	editor_edit_pbl_translation* create()
	{
		return new editor_edit_pbl_translation(temp1, temp2, temp3);
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
	game_config_view view;
	// It would be good to have a test directory instead of using the same directory as the player,
	// however this code will support that - default_saves_dir() will respect --userdata-dir.
	savegame::load_game_metadata data{savegame::save_index_class::default_saves_dir()};
	dialog_tester()
	{
		/** @todo Would be nice to add real data to the config. */
	}
	game_load* create()
	{
		view = game_config_view::wrap(cfg);
		return new game_load(view, data);
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

template<>
struct dialog_tester<mp_lobby>
{
	config game_config;
	wesnothd_connection connection;
	mp::lobby_info li;
	int selected_game;
	dialog_tester() : connection("", ""), li()
	{
	}
	mp_lobby* create()
	{
		return new mp_lobby(li, connection, selected_game);
	}
};

template<>
struct dialog_tester<mp_match_history>
{
	wesnothd_connection connection;
	dialog_tester() : connection("", "")
	{
	}
	mp_match_history* create()
	{
		return new mp_match_history("", connection, false);
	}
};

template<>
struct dialog_tester<gui2::dialogs::migrate_version_selection>
{
	gui2::dialogs::migrate_version_selection* create()
	{
		return new gui2::dialogs::migrate_version_selection();
	}
};

class fake_chat_handler : public events::chat_handler {
	void add_chat_message(const std::time_t&,
		const std::string&, int, const std::string&,
		MESSAGE_TYPE) {}
	void send_chat_message(const std::string&, bool) {}
	void send_to_server(const config&) {}
	void clear_messages() {}
};

template<>
struct dialog_tester<lobby_player_info>
{
	config c;
	fake_chat_handler ch;
	wesnothd_connection connection;
	mp::user_info ui;
	mp::lobby_info li;
	dialog_tester()
		: connection("", "")
		, ui(c), li()
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
		return new message("Title", "Message", false, false, false);
	}
};

template<>
struct dialog_tester<mp_create_game>
{
	saved_game state;
	dialog_tester() : state(config {"campaign_type", "multiplayer"})
	{
	}
	mp_create_game* create()
	{
		return new mp_create_game(state, true);
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

template<>
struct dialog_tester<mp_report>
{
	std::string report_text;
	mp_report* create()
	{
		return new mp_report(report_text);
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
		return new mp_login("wesnoth.org", "label", true);
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
		return new screenshot_notification("path", nullptr);
	}
};

template<>
struct dialog_tester<theme_list>
{
	static theme_info make_theme(const std::string& name)
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
	std::vector<std::unique_ptr<map_generator>> map_generators;
	editor_generate_map* create()
	{
		for(const config &i : test_gui2_fixture::main_config.child_range("multiplayer")) {
			if(i["scenario_generation"] == "default") {
				auto generator_cfg = i.optional_child("generator");
				if (generator_cfg) {
					map_generators.emplace_back(create_map_generator("", *generator_cfg));
				}
			}
		}

		editor_generate_map* result = new editor_generate_map(map_generators);
		BOOST_REQUIRE_MESSAGE(result, "Failed to create a dialog.");

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
		return new editor_new_map("Test", width, height);
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
	dialog_tester() : opts(args), game(opts) {}
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
	dialog_tester() : create_eng(state)
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
	statistics_record::campaign_stats_t stats_record;
	statistics_t stats;
	dialog_tester() : t() , stats_record(), stats(stats_record) {}
	statistics_dialog* create()
	{
		return new statistics_dialog(stats, t);
	}
};

template<>
struct dialog_tester<surrender_quit>
{
	dialog_tester() {}
	surrender_quit* create()
	{
		return new surrender_quit();
	}
};

template<>
struct dialog_tester<tod_new_schedule>
{
	std::string id = "id";
	t_string name = "name";
	dialog_tester() {}
	tod_new_schedule* create()
	{
		return new tod_new_schedule(id, name);
	}
};

template<>
struct dialog_tester<editor_edit_unit>
{
	config cfg;
	game_config_view view;

	dialog_tester() {}
	editor_edit_unit* create()
	{
		config& units = cfg.add_child("units");
		cfg.add_child("race");
		config& movetype = units.add_child("movetype");
		movetype["name"] = "Test Movetype";
		movetype.add_child("defense");
		movetype.add_child("resistance");
		movetype.add_child("movement_costs");
		view = game_config_view::wrap(cfg);
		return new editor_edit_unit(view, "test_addon");
	}
};

template<>
struct dialog_tester<gui_test_dialog>
{
	dialog_tester() {}
	gui_test_dialog* create()
	{
		return new gui_test_dialog();
	}
};

} // namespace
