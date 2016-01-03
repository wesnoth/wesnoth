/*
   Copyright (C) 2008 - 2016 by Jörg Hinrichs <joerg.hinrichs@alice-dsl.de>
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

#include "formula_string_utils.hpp"
#include "gettext.hpp"
#include "game_config.hpp"
#include "game_preferences.hpp"
#include "game_classification.hpp"
#include "gui/auxiliary/log.hpp"
#include "gui/dialogs/field.hpp"
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
#include "language.hpp"
#include "preferences_display.hpp"
#include "utils/foreach.tpp"

#include <cctype>
#include <boost/bind.hpp>

/* Helper function for determining if the selected save is a replay */
static bool is_replay_save(const config& cfg)
{
	return cfg["replay"].to_bool() && !cfg["snapshot"].to_bool(true);
}

namespace gui2
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
 * -filename & & control & m &
 *         Name of the savegame. $
 *
 * -date & & control & o &
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

tgame_load::tgame_load(const config& cache_config)
	: txtFilter_(register_text("txtFilter", true))
	, chk_change_difficulty_(register_bool("change_difficulty", true))
	, chk_show_replay_(register_bool("show_replay", true))
	, chk_cancel_orders_(register_bool("cancel_orders", true))
	, filename_()
	, change_difficulty_(false)
	, show_replay_(false)
	, cancel_orders_(false)
	, games_()
	, cache_config_(cache_config)
	, last_words_()
{
}

void tgame_load::pre_show(CVideo& /*video*/, twindow& window)
{
	assert(txtFilter_);

	find_widget<tminimap>(&window, "minimap", false).set_config(&cache_config_);

	ttext_box* filter
			= find_widget<ttext_box>(&window, "txtFilter", false, true);
	window.keyboard_capture(filter);
	filter->set_text_changed_callback(
			boost::bind(&tgame_load::filter_text_changed, this, _1, _2));
	window.keyboard_capture(filter);

	tlistbox* list
			= find_widget<tlistbox>(&window, "savegame_list", false, true);

#ifdef GUI2_EXPERIMENTAL_LISTBOX
	connect_signal_notify_modified(*list,
								   boost::bind(&tgame_load::list_item_clicked,
											   *this,
											   boost::ref(window)));
#else
	list->set_callback_value_change(
			dialog_callback<tgame_load, &tgame_load::list_item_clicked>);
#endif

	{
		cursor::setter cur(cursor::WAIT);
		games_ = savegame::get_saves_list();
	}
	fill_game_list(window, games_);

	connect_signal_mouse_left_click(
			find_widget<tbutton>(&window, "delete", false),
			boost::bind(&tgame_load::delete_button_callback,
						this,
						boost::ref(window)));

	display_savegame(window);
}

bool tgame_load::compare_name(unsigned i1, unsigned i2) const
{
	return games_[i1].name() < games_[i2].name();
}

bool tgame_load::compare_date(unsigned i1, unsigned i2) const
{
	return games_[i1].modified() < games_[i2].modified();
}

bool tgame_load::compare_name_rev(unsigned i1, unsigned i2) const
{
	return games_[i1].name() > games_[i2].name();
}

bool tgame_load::compare_date_rev(unsigned i1, unsigned i2) const
{
	return games_[i1].modified() > games_[i2].modified();
}

void tgame_load::fill_game_list(twindow& window,
								std::vector<savegame::save_info>& games)
{
	tlistbox& list = find_widget<tlistbox>(&window, "savegame_list", false);
	list.clear();

	FOREACH(const AUTO & game, games)
	{
		std::map<std::string, string_map> data;
		string_map item;

		item["label"] = game.name();
		data.insert(std::make_pair("filename", item));

		item["label"] = game.format_time_summary();
		data.insert(std::make_pair("date", item));

		list.add_row(data);
	}
	std::vector<tgenerator_::torder_func> order_funcs(2);
	order_funcs[0] = boost::bind(&tgame_load::compare_name, this, _1, _2);
	order_funcs[1] = boost::bind(&tgame_load::compare_name_rev, this, _1, _2);
	list.set_column_order(0, order_funcs);
	order_funcs[0] = boost::bind(&tgame_load::compare_date, this, _1, _2);
	order_funcs[1] = boost::bind(&tgame_load::compare_date_rev, this, _1, _2);
	list.set_column_order(1, order_funcs);
}

void tgame_load::list_item_clicked(twindow& window)
{
	display_savegame(window);
}

bool tgame_load::filter_text_changed(ttext_* textbox, const std::string& text)
{
	twindow& window = *textbox->get_window();

	tlistbox& list = find_widget<tlistbox>(&window, "savegame_list", false);

	const std::vector<std::string> words = utils::split(text, ' ');

	if(words == last_words_)
		return false;
	last_words_ = words;

	std::vector<bool> show_items(list.get_item_count(), true);

	if(!text.empty()) {
		for(unsigned int i = 0; i < list.get_item_count(); i++) {
			tgrid* row = list.get_row_grid(i);

			tgrid::iterator it = row->begin();
			tlabel& filename_label
					= find_widget<tlabel>(*it, "filename", false);

			bool found = false;
			FOREACH(const AUTO & word, words)
			{
				found = std::search(filename_label.label().str().begin(),
									filename_label.label().str().end(),
									word.begin(),
									word.end(),
									chars_equal_insensitive)
						!= filename_label.label().str().end();

				if(!found) {
					// one word doesn't match, we don't reach words.end()
					break;
				}
			}

			show_items[i] = found;
		}
	}

	list.set_row_shown(show_items);

	return false;
}

void tgame_load::post_show(twindow& window)
{
	change_difficulty_ = chk_change_difficulty_->get_widget_value(window);
	show_replay_ = chk_show_replay_->get_widget_value(window);
	cancel_orders_ = chk_cancel_orders_->get_widget_value(window);
}

void tgame_load::display_savegame(twindow& window)
{
	const int selected_row
			= find_widget<tlistbox>(&window, "savegame_list", false)
					  .get_selected_row();

	if(selected_row == -1) {
		return;
	}

	savegame::save_info& game = games_[selected_row];
	filename_ = game.name();

	const config& summary = game.summary();

	find_widget<timage>(&window, "imgLeader", false)
				.set_label(summary["leader_image"]);

	find_widget<tminimap>(&window, "minimap", false)
				.set_map_data(summary["map_data"]);

	find_widget<tlabel>(&window, "lblScenario", false)
				.set_label(game.name());

	std::stringstream str;
	str << game.format_time_local();
	evaluate_summary_string(str, summary);

	// Always toggle show_replay on if the save is a replay
	ttoggle_button& replay_toggle =
			find_widget<ttoggle_button>(&window, "show_replay", false);
	// cancel orders doesnt make sense on replay saves or start-of-scenario saves.
	ttoggle_button& cancel_orders_toggle =
			find_widget<ttoggle_button>(&window, "cancel_orders", false);

	const bool is_replay = is_replay_save(summary);
	const bool is_scenario_start = summary["turn"].empty();

	replay_toggle.set_value(is_replay);
	replay_toggle.set_active(!is_replay && !is_scenario_start);
	cancel_orders_toggle.set_active(!is_replay && !is_scenario_start);
	find_widget<tlabel>(&window, "lblSummary", false).set_label(str.str());

	// TODO: Find a better way to change the label width
	// window.invalidate_layout();
}

void tgame_load::evaluate_summary_string(std::stringstream& str,
										 const config& cfg_summary)
{

	const std::string& campaign_type = cfg_summary["campaign_type"];
	if(cfg_summary["corrupt"].to_bool()) {
		str << "\n" << _("#(Invalid)");
	} else {
		str << "\n";

		try
		{
			game_classification::CAMPAIGN_TYPE ct
					= lexical_cast<game_classification::CAMPAIGN_TYPE>(
							campaign_type);

			switch(ct.v) {
				case game_classification::CAMPAIGN_TYPE::SCENARIO: {
					const std::string campaign_id = cfg_summary["campaign"];
					const config* campaign = NULL;
					if(!campaign_id.empty()) {
						if(const config& c = cache_config_.find_child(
								   "campaign", "id", campaign_id)) {

							campaign = &c;
						}
					}
					utils::string_map symbols;
					if(campaign != NULL) {
						symbols["campaign_name"] = (*campaign)["name"];
					} else {
						// Fallback to nontranslatable campaign id.
						symbols["campaign_name"] = "(" + campaign_id + ")";
					}
					str << vgettext("Campaign: $campaign_name", symbols);

					// Display internal id for debug purposes if we didn't above
					if(game_config::debug && (campaign != NULL)) {
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
		}
		catch(bad_lexical_cast&)
		{
			str << campaign_type;
		}

		str << "\n";

		if(is_replay_save(cfg_summary)) {
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
}

void tgame_load::delete_button_callback(twindow& window)
{
	tlistbox& list = find_widget<tlistbox>(&window, "savegame_list", false);

	const size_t index = size_t(list.get_selected_row());
	if(index < games_.size()) {

		// See if we should ask the user for deletion confirmation
		if(preferences::ask_delete_saves()) {
			if(!gui2::tgame_delete::execute(window.video())) {
				return;
			}
		}

		// Delete the file
		savegame::delete_game(games_[index].name());

		// Remove it from the list of saves
		games_.erase(games_.begin() + index);
		list.remove_row(index);

		display_savegame(window);
	}
}

} // namespace gui2
