/* $Id$ */
/*
   Copyright (C) 2008 - 2009 by Jörg Hinrichs <joerg.hinrichs@alice-dsl.de>
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
#include "gui/dialogs/helper.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/minimap.hpp"
#include "gui/widgets/window.hpp"
#include "language.hpp"
#include "marked-up_text.hpp"

namespace gui2 {

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_game_load
 *
 * == Load a game ==
 *
 * This shows the dialog to select and load a savegame file.
 *
 * @start_table = container
 *     txtFilename_ (text_box)            The name of the savefile.
 * @end_table
 */

	tgame_load::tgame_load(const config& cache_config)
		: filename_(),
		cache_config_(cache_config)
{
}

twindow* tgame_load::build_window(CVideo& video)
{
	return build(video, get_id(GAME_LOAD));
}

void tgame_load::pre_show(CVideo& /*video*/, twindow& window)
{
	tminimap* minimap = dynamic_cast<tminimap*>(window.find_widget("minimap", false));
	VALIDATE(minimap, missing_widget("minimap"));
	minimap->set_config(&cache_config_);

	tlistbox* list = dynamic_cast<tlistbox*>(window.find_widget("savegame_list", false));
	VALIDATE(list, missing_widget("savegame_list"));
	window.keyboard_capture(list);
	list->set_callback_value_change(dialog_callback<tgame_load, &tgame_load::list_item_clicked>);

	
	{
		cursor::setter cur(cursor::WAIT);
		games_ = savegame_manager::get_saves_list();
	}

	foreach(const save_info game, games_) {
		std::map<std::string, string_map> data;
		string_map item;

		item["label"] = game.name;
		data.insert(std::make_pair("filename", item));

		item["label"] = format_time_summary(game.time_modified);
		data.insert(std::make_pair("date", item));

		list->add_row(data);
	}
}

void tgame_load::list_item_clicked(twindow& window){
	tlistbox* list = dynamic_cast<tlistbox*>(window.find_widget("savegame_list", false));
	VALIDATE(list, missing_widget("savegame_list"));

	config summary;
	std::string dummy;
	save_info game = games_[list->get_selected_row()];

	try {
		savegame_manager::load_summary(game.name, summary, &dummy);
	} catch(game::load_game_failed&) {
		summary["corrupt"] = "yes";
	}

	tminimap* minimap = dynamic_cast<tminimap*>(window.find_widget("minimap", false));
	VALIDATE(minimap, missing_widget("minimap"));
	minimap->set_map_data(summary["map_data"]);

	tlabel* scenario = dynamic_cast<tlabel*>(window.find_widget("lblScenario", false));
	scenario->set_label(game.name);

	std::stringstream str;

	char time_buf[256] = {0};
	tm* tm_l = localtime(&game.time_modified);
	if (tm_l) {
		const size_t res = strftime(time_buf,sizeof(time_buf),_("%a %b %d %H:%M %Y"),tm_l);
		if(res == 0) {
			time_buf[0] = 0;
		}
	} else {
		LOG_GUI_G << "localtime() returned null for time " << game.time_modified << ", save " << game.name;
	}

	str << time_buf;

	const std::string& campaign_type = summary["campaign_type"];
	if(utils::string_bool(summary["corrupt"], false)) {
		str << "\n" << _("#(Invalid)");
	} else if (!campaign_type.empty()) {
		str << "\n";

		if(campaign_type == "scenario") {
			const std::string campaign_id = summary["campaign"];
			const config *campaign = NULL;
			if (!campaign_id.empty()) {
				if (const config &c = cache_config_.find_child("campaign", "id", campaign_id))
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

		if(utils::string_bool(summary["replay"], false) && !utils::string_bool(summary["snapshot"], true)) {
			str << _("replay");
		} else if (!summary["turn"].empty()) {
			str << _("Turn") << " " << summary["turn"];
		} else {
			str << _("Scenario Start");
		}

		str << "\n" << _("Difficulty: ") << string_table[summary["difficulty"]];
		if(!summary["version"].empty()) {
			str << "\n" << _("Version: ") << summary["version"];
		}
	}

	tlabel* lblSummary = dynamic_cast<tlabel*>(window.find_widget("lblSummary", false));
	lblSummary->set_label(str.str());

	// FIXME: Find a better way to change the label width
	window.invalidate_layout();
}

void tgame_load::post_show(twindow& window)
{
	//filename_ = txtFilename_->get_widget_value(window);
}

std::string tgame_load::format_time_summary(time_t t)
{
	time_t curtime = time(NULL);
	const struct tm* timeptr = localtime(&curtime);
	if(timeptr == NULL) {
		return "";
	}

	const struct tm current_time = *timeptr;

	timeptr = localtime(&t);
	if(timeptr == NULL) {
		return "";
	}

	const struct tm save_time = *timeptr;

	const char* format_string = _("%b %d %y");

	if(current_time.tm_year == save_time.tm_year) {
		const int days_apart = current_time.tm_yday - save_time.tm_yday;
		if(days_apart == 0) {
			// save is from today
			format_string = _("%H:%M");
		} else if(days_apart > 0 && days_apart <= current_time.tm_wday) {
			// save is from this week
			format_string = _("%A, %H:%M");
		} else {
			// save is from current year
			format_string = _("%b %d");
		}
	} else {
		// save is from a different year
		format_string = _("%b %d %y");
	}

	char buf[40];
	const size_t res = strftime(buf,sizeof(buf),format_string,&save_time);
	if(res == 0) {
		buf[0] = 0;
	}

	return buf;
}

} // namespace gui2