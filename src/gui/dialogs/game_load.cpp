/*
   Copyright (C) 2008 - 2017 by Jörg Hinrichs <joerg.hinrichs@alice-dsl.de>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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

#include "filesystem.hpp"
#include "formula/string_utils.hpp"
#include "gettext.hpp"
#include "game_config.hpp"
#include "preferences/game.hpp"
#include "game_classification.hpp"
#include "gui/auxiliary/field.hpp"
#include "gui/core/log.hpp"
#include "gui/dialogs/game_delete.hpp"
#include "gui/dialogs/helper.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/label.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/minimap.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/window.hpp"
#include "image.hpp"
#include "language.hpp"
#include "serialization/string_utils.hpp"
#include "utils/general.hpp"

#include <cctype>
#include "utils/functional.hpp"

namespace gui2
{
namespace dialogs
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_game_load
 *
 * == Load a game ==
 *
 * This shows the dialog to select and load a savegame file.
 *
 * @begin{table}{dialog_widgets}
 *
 * txtFilter & & text & m &
 *         The filter for the listbox items. $
 *
 * savegame_list & & listbox & m &
 *         List of savegames. $
 *
 * -filename & & styled_widget & m &
 *         Name of the savegame. $
 *
 * -date & & styled_widget & o &
 *         Date the savegame was created. $
 *
 * -minimap & & minimap & m &
 *         Minimap of the selected savegame. $
 *
 * -imgLeader & & image & m &
 *         The image of the leader in the selected savegame. $
 *
 * -lblScenario & & label & m &
 *         The name of the scenario of the selected savegame. $
 *
 * -lblSummary & & label & m &
 *         Summary of the selected savegame. $
 *
 * @end{table}
 */

REGISTER_DIALOG(game_load)

game_load::game_load(const config& cache_config, savegame::load_game_metadata& data)
	: filename_(data.filename)
	, change_difficulty_(register_bool("change_difficulty", true, data.select_difficulty))
	, show_replay_(register_bool("show_replay", true, data.show_replay))
	, cancel_orders_(register_bool("cancel_orders", true, data.cancel_orders))
	, summary_(data.summary)
	, games_({savegame::get_saves_list()})
	, cache_config_(cache_config)
	, last_words_()
{
}

void game_load::pre_show(window& window)
{
	// Allow deleting saves with the Delete key.
	connect_signal_pre_key_press(window, std::bind(&game_load::key_press_callback, this, std::ref(window), _5));

	find_widget<minimap>(&window, "minimap", false).set_config(&cache_config_);

	text_box* filter = find_widget<text_box>(&window, "txtFilter", false, true);

	filter->set_text_changed_callback(
			std::bind(&game_load::filter_text_changed, this, _1, _2));

	listbox& list = find_widget<listbox>(&window, "savegame_list", false);

	connect_signal_notify_modified(list,
			std::bind(&game_load::display_savegame, this, std::ref(window)));

	window.keyboard_capture(filter);
	window.add_to_keyboard_chain(&list);

	list.clear();

	for(const auto& game : games_) {
		std::map<std::string, string_map> data;
		string_map item;

		std::string name = game.name();
		utils::ellipsis_truncate(name, 40);
		item["label"] = name;
		data.emplace("filename", item);

		item["label"] = game.format_time_summary();
		data.emplace("date", item);

		list.add_row(data);
	}

	list.register_sorting_option(0, [this](const int i) { return games_[i].name(); });
	list.register_sorting_option(1, [this](const int i) { return games_[i].modified(); });

	connect_signal_mouse_left_click(
			find_widget<button>(&window, "delete", false),
			std::bind(&game_load::delete_button_callback,
					this, std::ref(window)));

	display_savegame(window);
}

void game_load::display_savegame(window& window)
{
	const int selected_row =
		find_widget<listbox>(&window, "savegame_list", false).get_selected_row();

	if(selected_row == -1) {
		return;
	}

	savegame::save_info& game = games_[selected_row];
	filename_ = game.name();
	summary_  = game.summary();

	find_widget<minimap>(&window, "minimap", false)
			.set_map_data(summary_["map_data"]);

	find_widget<label>(&window, "lblScenario", false)
			.set_label(summary_["label"]);

	listbox& leader_list = find_widget<listbox>(&window, "leader_list", false);

	leader_list.clear();

	for(const auto& leader : summary_.child_range("leader")) {
		std::map<std::string, string_map> data;
		string_map item;

		// First, we evaluate whether the leader image as provided exists.
		// If not, we try getting a binary-path independent path. If that still doesn't
		// work, we fallback on unknown-unit.png.
		std::string leader_image = leader["leader_image"].str();
		if(!::image::exists(leader_image)) {
			leader_image = filesystem::get_independent_image_path(leader_image);
		}

		if(leader_image.empty()) {
			leader_image = "units/unknown-unit.png" + leader["leader_image_tc_modifier"].str();
		}
		else {
			leader_image+= leader["leader_image_tc_modifier"].str();
		}
		item["label"] = leader_image;
		data.emplace("imgLeader", item);

		item["label"] = leader["leader_name"];
		data.emplace("leader_name", item);

		item["label"] = leader["gold"];
		data.emplace("leader_gold", item);

		item["label"] = leader["units"];
		data.emplace("leader_troops", item);

		item["label"] = leader["recall_units"];
		data.emplace("leader_reserves", item);

		leader_list.add_row(data);
	}

	std::stringstream str;
	str << game.format_time_local() << "\n";
	evaluate_summary_string(str, summary_);

	find_widget<label>(&window, "lblSummary", false).set_label(str.str());

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

void game_load::filter_text_changed(text_box_base* textbox, const std::string& text)
{
	window& window = *textbox->get_window();

	listbox& list = find_widget<listbox>(&window, "savegame_list", false);

	const std::vector<std::string> words = utils::split(text, ' ');

	if(words == last_words_)
		return;
	last_words_ = words;

	boost::dynamic_bitset<> show_items;
	show_items.resize(list.get_item_count(), true);

	if(!text.empty()) {
		for(unsigned int i = 0; i < list.get_item_count(); i++) {
			grid* row = list.get_row_grid(i);

			grid::iterator it = row->begin();
			label& filename_label = find_widget<label>(*it, "filename", false);

			bool found = false;
			for(const auto & word : words)
			{
				found = std::search(filename_label.get_label().str().begin(),
									filename_label.get_label().str().end(),
									word.begin(),
									word.end(),
									chars_equal_insensitive)
						!= filename_label.get_label().str().end();

				if(!found) {
					// one word doesn't match, we don't reach words.end()
					break;
				}
			}

			show_items[i] = found;
		}
	}

	list.set_row_shown(show_items);

	const bool any_shown = list.any_rows_shown();

	// Disable Load button if no games are available
	find_widget<button>(&window, "ok", false).set_active(any_shown);

	// Diable 'Enter' loading if no games are available
	window.set_enter_disabled(!any_shown);
}

void game_load::evaluate_summary_string(std::stringstream& str, const config& cfg_summary)
{
	if(cfg_summary["corrupt"].to_bool()) {
		str << "\n<span color='#f00'>" << _("(Invalid)") << "</span>";

		return;
	}

	const std::string& campaign_type = cfg_summary["campaign_type"];

	try {
		switch(game_classification::CAMPAIGN_TYPE::string_to_enum(campaign_type).v) {
			case game_classification::CAMPAIGN_TYPE::SCENARIO: {
				const std::string campaign_id = cfg_summary["campaign"];

				const config* campaign = nullptr;
				if(!campaign_id.empty()) {
					if(const config& c = cache_config_.find_child("campaign", "id", campaign_id)) {
						campaign = &c;
					}
				}

				utils::string_map symbols;
				if(campaign != nullptr) {
					symbols["campaign_name"] = (*campaign)["name"];
				} else {
					// Fallback to nontranslatable campaign id.
					symbols["campaign_name"] = "(" + campaign_id + ")";
				}

				str << vgettext("Campaign: $campaign_name", symbols);

				// Display internal id for debug purposes if we didn't above
				if(game_config::debug && (campaign != nullptr)) {
					str << '\n' << "(" << campaign_id << ")";
				}
				break;
			}
			case game_classification::CAMPAIGN_TYPE::MULTIPLAYER:
				str << _("Multiplayer");
				break;
			case game_classification::CAMPAIGN_TYPE::TUTORIAL:
				str << _("Tutorial");
				break;
			case game_classification::CAMPAIGN_TYPE::TEST:
				str << _("Test scenario");
				break;
		}
	} catch(bad_enum_cast&) {
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

	str << "\n" << _("Difficulty: ")
		<< string_table[cfg_summary["difficulty"]];

	if(!cfg_summary["version"].empty()) {
		str << "\n" << _("Version: ") << cfg_summary["version"];
	}
}

void game_load::delete_button_callback(window& window)
{
	listbox& list = find_widget<listbox>(&window, "savegame_list", false);

	const size_t index = size_t(list.get_selected_row());
	if(index < games_.size()) {

		// See if we should ask the user for deletion confirmation
		if(preferences::ask_delete_saves()) {
			if(!gui2::dialogs::game_delete::execute()) {
				return;
			}
		}

		// Delete the file
		savegame::delete_game(games_[index].name());

		// Remove it from the list of saves
		games_.erase(games_.begin() + index);

		list.remove_row(index);

		// Close the dialog if there are no more saves
		if(list.get_item_count() == 0) {
			window.set_retval(window::CANCEL);
		}

		display_savegame(window);
	}
}

void game_load::key_press_callback(window& window, const SDL_Keycode key)
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
	if(find_widget<text_box>(&window, "txtFilter", false).get_state() == text_box_base::FOCUSED) {
		return;
	}

	if(key == SDLK_DELETE) {
		delete_button_callback(window);
	}
}

} // namespace dialogs
} // namespace gui2
