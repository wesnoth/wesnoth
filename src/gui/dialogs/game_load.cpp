/*
	Copyright (C) 2008 - 2024
	by Jörg Hinrichs <joerg.hinrichs@alice-dsl.de>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/game_load.hpp"

#include "desktop/open.hpp"
#include "filesystem.hpp"
#include "formatter.hpp"
#include "formula/string_utils.hpp"
#include "game_config.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/field.hpp"
#include "gui/dialogs/game_delete.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/minimap.hpp"
#include "gui/widgets/scroll_label.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/window.hpp"
#include "language.hpp"
#include "picture.hpp"
#include "preferences/preferences.hpp"
#include "serialization/string_utils.hpp"
#include "utils/general.hpp"
#include <functional>
#include "game_config_view.hpp"


static lg::log_domain log_gameloaddlg{"gui/dialogs/game_load_dialog"};
#define ERR_GAMELOADDLG   LOG_STREAM(err,   log_gameloaddlg)
#define WRN_GAMELOADDLG   LOG_STREAM(warn,  log_gameloaddlg)
#define LOG_GAMELOADDLG   LOG_STREAM(info,  log_gameloaddlg)
#define DBG_GAMELOADDLG   LOG_STREAM(debug, log_gameloaddlg)

namespace gui2::dialogs
{

REGISTER_DIALOG(game_load)

bool game_load::execute(const game_config_view& cache_config, savegame::load_game_metadata& data)
{
	if(savegame::save_index_class::default_saves_dir()->get_saves_list().empty()) {
		bool found_files = false;
		for(const auto& dir : filesystem::find_other_version_saves_dirs()) {
			if(!found_files) {
				// this needs to be a shared_ptr because get_saves_list() uses shared_from_this
				auto index = std::make_shared<savegame::save_index_class>(dir.path);
				found_files = !index->get_saves_list().empty();
			}
		}

		if(!found_files) {
			gui2::show_transient_message(_("No Saved Games"), _("There are no saved games to load."));
			return false;
		}
	}

	return game_load(cache_config, data).show();
}

game_load::game_load(const game_config_view& cache_config, savegame::load_game_metadata& data)
	: modal_dialog(window_id())
	, filename_(data.filename)
	, save_index_manager_(data.manager)
	, change_difficulty_(register_bool("change_difficulty", true, data.select_difficulty))
	, show_replay_(register_bool("show_replay", true, data.show_replay))
	, cancel_orders_(register_bool("cancel_orders", true, data.cancel_orders))
	, summary_(data.summary)
	, games_()
	, cache_config_(cache_config)
	, last_words_()
{
}

void game_load::pre_show(window& window)
{
	// Allow deleting saves with the Delete key.
	connect_signal_pre_key_press(window, std::bind(&game_load::key_press_callback, this, std::placeholders::_5));

	text_box* filter = find_widget<text_box>(&window, "txtFilter", false, true);

	filter->set_text_changed_callback(std::bind(&game_load::filter_text_changed, this, std::placeholders::_2));

	listbox& list = find_widget<listbox>(&window, "savegame_list", false);

	connect_signal_notify_modified(list, std::bind(&game_load::display_savegame, this));

	window.keyboard_capture(filter);
	window.add_to_keyboard_chain(&list);

	list.register_sorting_option(0, [this](const int i) { return games_[i].name(); });
	list.register_sorting_option(1, [this](const int i) { return games_[i].modified(); });

	populate_game_list();

	connect_signal_mouse_left_click(find_widget<button>(&window, "delete", false),
			std::bind(&game_load::delete_button_callback, this));

	connect_signal_mouse_left_click(find_widget<button>(&window, "browse_saves_folder", false),
			std::bind(&game_load::browse_button_callback, this));

	menu_button& dir_list = find_widget<menu_button>(&window, "dirList", false);

	dir_list.set_use_markup(true);
	set_save_dir_list(dir_list);

	connect_signal_notify_modified(dir_list, std::bind(&game_load::handle_dir_select, this));

	display_savegame();
}

void game_load::set_save_dir_list(menu_button& dir_list)
{
	const auto other_dirs = filesystem::find_other_version_saves_dirs();
	if(other_dirs.empty()) {
		dir_list.set_visible(widget::visibility::invisible);
		return;
	}

	std::vector<config> options;

	// The first option in the list is the current version's save dir
	options.emplace_back("label",  _("game_version^Current Version"), "path", "");

	for(const auto& known_dir : other_dirs) {
		options.emplace_back(
			"label", VGETTEXT("game_version^Wesnoth $version", utils::string_map{{"version", known_dir.version}}),
			"path", known_dir.path
		);
	}

	dir_list.set_values(options);
}

void game_load::populate_game_list()
{
	listbox& list = find_widget<listbox>(get_window(), "savegame_list", false);

	list.clear();

	games_ = save_index_manager_->get_saves_list();

	for(const auto& game : games_) {
		widget_data data;
		widget_item item;

		std::string name = game.name();
		utils::ellipsis_truncate(name, 40);
		item["label"] = name;
		data.emplace("filename", item);

		item["label"] = game.format_time_summary();
		data.emplace("date", item);

		list.add_row(data);
	}

	find_widget<button>(get_window(), "delete", false).set_active(!save_index_manager_->read_only());
}

void game_load::display_savegame_internal(const savegame::save_info& game)
{
	filename_ = game.name();
	summary_  = game.summary();

	find_widget<minimap>(get_window(), "minimap", false)
			.set_map_data(summary_["map_data"]);

	find_widget<label>(get_window(), "lblScenario", false)
			.set_label(summary_["label"]);

	listbox& leader_list = find_widget<listbox>(get_window(), "leader_list", false);

	leader_list.clear();

	const std::string sprite_scale_mod = (formatter() << "~SCALE_INTO(" << game_config::tile_size << ',' << game_config::tile_size << ')').str();

	unsigned li = 0;
	for(const auto& leader : summary_.child_range("leader")) {
		widget_data data;
		widget_item item;

		// First, we evaluate whether the leader image as provided exists.
		// If not, we try getting a binary path-independent path. If that still doesn't
		// work, we fallback on unknown-unit.png.
		std::string leader_image = leader["leader_image"].str();
		if(!::image::exists(leader_image)) {
			auto indep_path = filesystem::get_independent_binary_file_path("images", leader_image);

			// The leader TC modifier isn't appending if the independent image path can't
			// be resolved during save_index entry creation, so we need to add it here.
			if(indep_path) {
				leader_image = indep_path.value() + leader["leader_image_tc_modifier"].str();
			}
		}

		if(leader_image.empty()) {
			leader_image = "units/unknown-unit.png" + leader["leader_image_tc_modifier"].str();
		} else {
			// Scale down any sprites larger than 72x72
			leader_image += sprite_scale_mod + "~FL(horiz)";
		}

		item["label"] = leader_image;
		data.emplace("imgLeader", item);

		item["label"] = leader["leader_name"];
		data.emplace("leader_name", item);

		item["label"] = leader["gold"];
		data.emplace("leader_gold", item);

		// TRANSLATORS: "reserve" refers to units on the recall list
		item["label"] = VGETTEXT("$active active, $reserve reserve", {{"active", leader["units"]}, {"reserve", leader["recall_units"]}});
		data.emplace("leader_troops", item);

		leader_list.add_row(data);

		// FIXME: hack. In order to use the listbox in view-only mode, you also need to
		// disable the max number of "selected items", since in this mode, "selected" is
		// synonymous with "visible". This basically just flags all rows as visible. Need
		// a better solution at some point
		leader_list.select_row(li++, true);
	}

	std::stringstream str;
	str << game.format_time_local() << "\n";
	evaluate_summary_string(str, summary_);

	// The new label value may have more or less lines than the previous value, so invalidate the layout.
	find_widget<styled_widget>(get_window(), "slblSummary", false).set_label(str.str());
	//get_window()->invalidate_layout();

	toggle_button& replay_toggle            = dynamic_cast<toggle_button&>(*show_replay_->get_widget());
	toggle_button& cancel_orders_toggle     = dynamic_cast<toggle_button&>(*cancel_orders_->get_widget());
	toggle_button& change_difficulty_toggle = dynamic_cast<toggle_button&>(*change_difficulty_->get_widget());

	const bool is_replay = savegame::loadgame::is_replay_save(summary_);
	const bool is_scenario_start = summary_["turn"].empty();

	// Always toggle show_replay on if the save is a replay
	replay_toggle.set_value(is_replay);
	replay_toggle.set_active(!is_replay && !is_scenario_start);

	// Cancel orders doesn't make sense on replay saves or start-of-scenario saves
	cancel_orders_toggle.set_active(!is_replay && !is_scenario_start);

	// Changing difficulty doesn't make sense on non-start-of-scenario saves
	change_difficulty_toggle.set_active(!is_replay && is_scenario_start);
}

// This is a wrapper that prevents a corrupted save file (if it happens to be
// the first in the list) from making the dialog fail to open.
void game_load::display_savegame()
{
	bool successfully_displayed_a_game = false;

	try {
		const int selected_row = find_widget<listbox>(get_window(), "savegame_list", false).get_selected_row();
		if(selected_row < 0) {
			find_widget<button>(get_window(), "delete", false).set_active(false);
		} else {
			find_widget<button>(get_window(), "delete", false).set_active(!save_index_manager_->read_only());
			game_load::display_savegame_internal(games_[selected_row]);
			successfully_displayed_a_game = true;
		}
	} catch(const config::error& e) {
		// Clear the UI widgets, show an error message.
		const std::string preamble = _("The selected file is corrupt: ");
		const std::string message = e.message.empty() ? "(no details)" : e.message;
		ERR_GAMELOADDLG << preamble << message;
	}

	if(!successfully_displayed_a_game) {
		find_widget<minimap>(get_window(), "minimap", false).set_map_data("");
		find_widget<label>(get_window(), "lblScenario", false)
			.set_label("");
		find_widget<styled_widget>(get_window(), "slblSummary", false)
			.set_label("");

		listbox& leader_list = find_widget<listbox>(get_window(), "leader_list", false);
		leader_list.clear();

		toggle_button& replay_toggle            = dynamic_cast<toggle_button&>(*show_replay_->get_widget());
		toggle_button& cancel_orders_toggle     = dynamic_cast<toggle_button&>(*cancel_orders_->get_widget());
		toggle_button& change_difficulty_toggle = dynamic_cast<toggle_button&>(*change_difficulty_->get_widget());

		replay_toggle.set_active(false);
		cancel_orders_toggle.set_active(false);
		change_difficulty_toggle.set_active(false);
	}

	// Disable Load button if nothing is selected or if the currently selected file can't be loaded
	find_widget<button>(get_window(), "ok", false).set_active(successfully_displayed_a_game);

	// Disable 'Enter' loading in the same circumstance
	get_window()->set_enter_disabled(!successfully_displayed_a_game);
}

void game_load::filter_text_changed(const std::string& text)
{
	apply_filter_text(text, false);
}

void game_load::apply_filter_text(const std::string& text, bool force)
{
	listbox& list = find_widget<listbox>(get_window(), "savegame_list", false);

	const std::vector<std::string> words = utils::split(text, ' ');

	if(words == last_words_ && !force)
		return;
	last_words_ = words;

	boost::dynamic_bitset<> show_items;
	show_items.resize(list.get_item_count(), true);

	if(!text.empty()) {
		for(unsigned int i = 0; i < list.get_item_count() && i < games_.size(); i++) {
			bool found = false;
			for(const auto & word : words)
			{
				found = std::search(games_[i].name().begin(),
									games_[i].name().end(),
									word.begin(),
									word.end(),
									utils::chars_equal_insensitive)
						!= games_[i].name().end();

				if(!found) {
					// one word doesn't match, we don't reach words.end()
					break;
				}
			}

			show_items[i] = found;
		}
	}

	list.set_row_shown(show_items);
}

void game_load::evaluate_summary_string(std::stringstream& str, const config& cfg_summary)
{
	if(cfg_summary["corrupt"].to_bool()) {
		str << "\n<span color='#f00'>" << _("(Invalid)") << "</span>";
		// \todo: this skips the catch() statement in display_savegame. Low priority, as the
		// dialog's state is reasonable; the "load" button is inactive, the "delete" button is
		// active, and (cosmetic bug) it leaves the "change difficulty" toggle active. Can be
		// triggered by creating an empty file in the save directory.
		return;
	}

	const std::string& campaign_type = cfg_summary["campaign_type"];
	const std::string campaign_id = cfg_summary["campaign"];
	auto campaign_type_enum = campaign_type::get_enum(campaign_type);

	if(campaign_type_enum) {
		switch(*campaign_type_enum) {
			case campaign_type::type::scenario: {
				const config* campaign = nullptr;
				if(!campaign_id.empty()) {
					if(auto c = cache_config_.find_child("campaign", "id", campaign_id)) {
						campaign = c.ptr();
					}
				}

				utils::string_map symbols;
				if(campaign != nullptr) {
					symbols["campaign_name"] = (*campaign)["name"];
				} else {
					// Fallback to nontranslatable campaign id.
					symbols["campaign_name"] = "(" + campaign_id + ")";
				}

				str << VGETTEXT("Campaign: $campaign_name", symbols);

				// Display internal id for debug purposes if we didn't above
				if(game_config::debug && (campaign != nullptr)) {
					str << '\n' << "(" << campaign_id << ")";
				}
				break;
			}
			case campaign_type::type::multiplayer:
				str << _("Multiplayer");
				break;
			case campaign_type::type::tutorial:
				str << _("Tutorial");
				break;
			case campaign_type::type::test:
				str << _("Test scenario");
				break;
		}
	} else {
		str << campaign_type;
	}

	str << "\n";

	if(savegame::loadgame::is_replay_save(cfg_summary)) {
		str << _("Replay");
	} else if(!cfg_summary["turn"].empty()) {
		str << _("Turn") << " " << cfg_summary["turn"];
	} else {
		str << _("Scenario start");
	}

	if(campaign_type_enum) {
		switch (*campaign_type_enum) {
		case campaign_type::type::scenario:
		case campaign_type::type::multiplayer: {
			const config* campaign = nullptr;
			if (!campaign_id.empty()) {
				if (auto c = cache_config_.find_child("campaign", "id", campaign_id)) {
					campaign = c.ptr();
				}
			}

			// 'SCENARIO' or SP should only ever be campaigns
			// 'MULTIPLAYER' may be a campaign with difficulty or single scenario without difficulty
			// For the latter do not show the difficulty - even though it will be listed as
			// NORMAL -> Medium in the save file it should not be considered valid (GitHub Issue #5321)
			if (campaign != nullptr) {
				str << "\n" << _("Difficulty: ");
				try {
					const config& difficulty = campaign->find_mandatory_child("difficulty", "define", cfg_summary["difficulty"]);
					std::ostringstream ss;
					ss << difficulty["label"] << " (" << difficulty["description"] << ")";
					str << ss.str();
				}
				catch (const config::error&) {
					// fall back to standard difficulty string in case of exception
					str << string_table[cfg_summary["difficulty"]];
				}
			}

			break;
		}
		case campaign_type::type::tutorial:
		case campaign_type::type::test:
			break;
		}
	} else {
	}

	if(!cfg_summary["version"].empty()) {
		str << "\n" << _("Version: ") << cfg_summary["version"];
	}

	const std::vector<std::string>& active_mods = utils::split(cfg_summary["active_mods"]);
	if(!active_mods.empty()) {
		str << "\n" << _("Modifications: ");
		for(const auto& mod_id : active_mods) {
			std::string mod_name;
			try {
				mod_name = cache_config_.find_mandatory_child("modification", "id", mod_id)["name"].str();
			} catch(const config::error&) {
				// Fallback to nontranslatable mod id.
				mod_name = "(" + mod_id + ")";
			}

			str << "\n" << font::unicode_bullet << " " << mod_name;
		}
	}
}
void game_load::browse_button_callback()
{
	desktop::open_object(save_index_manager_->dir());
}

void game_load::delete_button_callback()
{
	listbox& list = find_widget<listbox>(get_window(), "savegame_list", false);

	const std::size_t index = std::size_t(list.get_selected_row());
	if(index < games_.size()) {

		// See if we should ask the user for deletion confirmation
		if(prefs::get().ask_delete()) {
			if(!gui2::dialogs::game_delete::execute()) {
				return;
			}
		}

		// Delete the file
		save_index_manager_->delete_game(games_[index].name());

		// Remove it from the list of saves
		games_.erase(games_.begin() + index);

		list.remove_row(index);

		display_savegame();
	}
}

void game_load::key_press_callback(const SDL_Keycode key)
{
	//
	// Don't delete games when we're typing in the textbox!
	//
	// I'm not sure if this check was necessary when I first added this feature
	// (I didn't check at the time), but regardless, it's needed now. If it turns
	// out I screwed something up in my refactoring, I'll remove this.
	//
	// - vultraz, 2017-08-28
	//
	if(find_widget<text_box>(get_window(), "txtFilter", false).get_state() == text_box_base::FOCUSED) {
		return;
	}

	if(key == SDLK_DELETE) {
		delete_button_callback();
	}
}

void game_load::handle_dir_select()
{
	menu_button& dir_list = find_widget<menu_button>(get_window(), "dirList", false);

	const auto& path = dir_list.get_value_config()["path"].str();
	if(path.empty()) {
		save_index_manager_ = savegame::save_index_class::default_saves_dir();
	} else {
		save_index_manager_ = std::make_shared<savegame::save_index_class>(path);
	}

	populate_game_list();
	if(auto* filter = find_widget<text_box>(get_window(), "txtFilter", false, true)) {
		apply_filter_text(filter->get_value(), true);
	}
	display_savegame();
}

} // namespace dialogs
