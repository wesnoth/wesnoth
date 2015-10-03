/* $Id$ */
/*
   Copyright (C) 2008 - 2011 by Jörg Hinrichs <joerg.hinrichs@alice-dsl.de>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/game_load.hpp"

#include "foreach.hpp"
#include "formula_string_utils.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/log.hpp"
#include "gui/dialogs/field.hpp"
#include "gui/dialogs/game_delete.hpp"
#include "gui/dialogs/helper.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/minimap.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/window.hpp"
#include "language.hpp"
#include "preferences_display.hpp"

#include <cctype>
#include <boost/bind.hpp>

namespace gui2 {

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_game_load
 *
 * == Load a game ==
 *
 * This shows the dialog to select and load a savegame file.
 *
 * @start_table = grid
 *     (txtFilter) (text) ()      The filter for the listbox items.
 *
 *     (savegame_list) (listbox) ()
 *                                List of savegames.
 *     -(filename)                The name of the savegame.
 *     -[date]                    The date the savegame was created.
 *
 *     (preview_pane) (widget) () Container widget or grid that contains the
 *                                items for a preview. The visible status of
 *                                this container depends on whether or not
 *                                something is selected.
 *     -(minimap) (minimap) ()    Minimap of the selected savegame.
 *     -(imgLeader) (image) ()    The image of the leader in the selected
 *                                savegame.
 *     -(lblScenario) (label) ()  The name of the scenario of the selected
 *                                savegame.
 *     -(lblSummary) (label) ()   Summary of the selected savegame.
 * @end_table
 */

tgame_load::tgame_load(const config& cache_config)
	: txtFilter_(register_text("txtFilter", false))
	, chk_show_replay_(register_bool("show_replay"))
	, chk_cancel_orders_(register_bool("cancel_orders"))
	, filename_()
	, show_replay_(false)
	, cancel_orders_(false)
	, games_()
	, cache_config_(cache_config)
	, last_words_()
{
}

twindow* tgame_load::build_window(CVideo& video)
{
	return build(video, get_id(GAME_LOAD));
}

void tgame_load::pre_show(CVideo& /*video*/, twindow& window)
{
	assert(txtFilter_);

	find_widget<tminimap>(&window, "minimap", false).set_config(&cache_config_);

	ttext_box* filter = find_widget<ttext_box>(
			&window, "txtFilter", false, true);
	window.keyboard_capture(filter);
	filter->set_text_changed_callback(boost::bind(
			&tgame_load::filter_text_changed, this, _1, _2));

	tlistbox* list = find_widget<tlistbox>(
			&window, "savegame_list", false, true);
	window.keyboard_capture(list);
	list->set_callback_value_change(
			dialog_callback<tgame_load, &tgame_load::list_item_clicked>);

	{
		cursor::setter cur(cursor::WAIT);
		games_ = savegame::manager::get_saves_list();
	}
	fill_game_list(window, games_);

	GUI2_EASY_BUTTON_CALLBACK(delete, tgame_load);

	display_savegame(window);
}

void tgame_load::fill_game_list(twindow& window
		, std::vector<savegame::save_info>& games)
{
	tlistbox& list = find_widget<tlistbox>(&window, "savegame_list", false);
	list.clear();

	BOOST_FOREACH(const savegame::save_info game, games) {
		std::map<std::string, string_map> data;
		string_map item;

		item["label"] = game.name;
		data.insert(std::make_pair("filename", item));

		item["label"] = game.format_time_summary();
		data.insert(std::make_pair("date", item));

		list.add_row(data);
	}
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

	if (words == last_words_)
		return false;
	last_words_ = words;

	std::vector<bool> show_items(list.get_item_count(), true);

	if(!text.empty()) {
		for(unsigned int i = 0; i < list.get_item_count(); i++) {
			tgrid* row = list.get_row_grid(i);

			tgrid::iterator it = row->begin();
			tlabel& filename_label =
				find_widget<tlabel>(*it, "filename", false);

			bool found = false;
			BOOST_FOREACH (const std::string& word, words){
				found = std::search(filename_label.label().str().begin()
						, filename_label.label().str().end()
						, word.begin(), word.end()
						, chars_equal_insensitive)
					!= filename_label.label().str().end();

				if (! found) {
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
	show_replay_ = chk_show_replay_->get_widget_value(window);
	cancel_orders_ = chk_cancel_orders_->get_widget_value(window);
}

void tgame_load::display_savegame(twindow& window)
{
	const int selected_row =
			find_widget<tlistbox>(&window, "savegame_list", false)
				.get_selected_row();

	twidget& preview_pane =
			find_widget<twidget>(&window, "preview_pane", false);

	if(selected_row == -1) {
		preview_pane.set_visible(twidget::HIDDEN);
	} else {
		preview_pane.set_visible(twidget::VISIBLE);

		savegame::save_info& game = games_[selected_row];
		filename_ = game.name;

		config cfg_summary;
		std::string dummy;

		try {
			savegame::manager::load_summary(game.name, cfg_summary, &dummy);
		} catch(game::load_game_failed&) {
			cfg_summary["corrupt"] = "yes";
		}

		find_widget<timage>(&window, "imgLeader", false).
				set_label(cfg_summary["leader_image"]);

		find_widget<tminimap>(&window, "minimap", false).
				set_map_data(cfg_summary["map_data"]);

		find_widget<tlabel>(&window, "lblScenario", false).set_label(game.name);

		std::stringstream str;
		str << game.format_time_local();
		evaluate_summary_string(str, cfg_summary);

		find_widget<tlabel>(&window, "lblSummary", false).set_label(str.str());

		// FIXME: Find a better way to change the label width
		window.invalidate_layout();
	}
}

void tgame_load::evaluate_summary_string(std::stringstream& str
		, const config& cfg_summary){

	const std::string& campaign_type = cfg_summary["campaign_type"];
	if(utils::string_bool(cfg_summary["corrupt"], false)) {
		str << "\n" << _("#(Invalid)");
	} else if (!campaign_type.empty()) {
		str << "\n";

		if(campaign_type == "scenario") {
			const std::string campaign_id = cfg_summary["campaign"];
			const config *campaign = NULL;
			if (!campaign_id.empty()) {
				if (const config &c = cache_config_.find_child(
						"campaign", "id", campaign_id))

					campaign = &c;
			}
			utils::string_map symbols;
			if (campaign != NULL) {
				symbols["campaign_name"] = (*campaign)["name"];
			} else {
				// Fallback to nontranslatable campaign id.
				symbols["campaign_name"] = "(" + campaign_id + ")";
			}
			str << vgettext("Campaign: $campaign_name", symbols);

			// Display internal id for debug purposes if we didn't above
			if (game_config::debug && (campaign != NULL)) {
				str << '\n' << "(" << campaign_id << ")";
			}
		} else if(campaign_type == "multiplayer") {
			str << _("Multiplayer");
		} else if(campaign_type == "tutorial") {
			str << _("Tutorial");
		} else {
			str << campaign_type;
		}

		str << "\n";

		if(utils::string_bool(cfg_summary["replay"], false)
				&& !utils::string_bool(cfg_summary["snapshot"], true)) {

			str << _("replay");
		} else if (!cfg_summary["turn"].empty()) {
			str << _("Turn") << " " << cfg_summary["turn"];
		} else {
			str << _("Scenario Start");
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
			gui2::tgame_delete dlg_delete;
			dlg_delete.show(window.video(), 0);
			int res = dlg_delete.get_retval();

			if (res == twindow::CANCEL)
				return;

			if (dlg_delete.dont_ask_again()) {
				preferences::set_ask_delete_saves(false);
			}
		}

		// Delete the file
		savegame::manager::delete_game(games_[index].name);

		// Remove it from the list of saves
		games_.erase(games_.begin() + index);
		list.remove_row(index);

		display_savegame(window);
	}
}

} // namespace gui2
