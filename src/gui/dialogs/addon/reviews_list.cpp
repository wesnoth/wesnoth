/*
   Copyright (C) 2011 - 2014 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "gui/dialogs/addon/reviews_list.hpp"

#include "gui/auxiliary/find_widget.tpp"
#include "gui/widgets/grid.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/settings.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/window.hpp"
#include "utils/foreach.tpp"
#include "gettext.hpp"
#include "addon/info.hpp"
#include <boost/bind.hpp>
#include "gui/widgets/button.hpp"
#include "gui/dialogs/addon/addon_review_write.hpp"

#include <algorithm>

namespace gui2
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 7_addon_reviews_list
 *
 * == Add-on reviews list ==
 *
 * Dialog with a checkbox list for reading reviews of add-ons and voting them up.
 *
 * @begin{table}{dialog_widgets}
 *
 * reviews_list & & listbox & m &
 *     A listbox containing reviews. $
 *
 * -player_agrees & & toggle_button & m &
 *     A toggle button allowing the user to mark which review he likes. $
 *
 * -review_box & & scroll_label & m &
 *     A scroll label where the reviews appear. $
 *
 * -people_who_agree & & label & o &
 *     A label that shows the number of people who voted up that review. $
 *
 * @end{table}
 */

std::string& taddon_reviews_list::cut_into_more_lines(std::string& inserted, int max_length) {
	// This is basically a workaround for the problem that more than one scroll label can't
	// appear in the same window, that results in no division of long strings into lines.
	unsigned int i = 0;
	while (i < inserted.size()) {
		unsigned int j;
		for (j = i + max_length; j >= i; j--) {
			if (j == i) {
				inserted.insert(i + max_length,"\n");
				i += max_length;
				break;
			}
			if (j >= inserted.size()) {
				i = inserted.size();
				break;
			}
			if (inserted[j] == 32) {
				inserted[j] = 10;
				i = j;
				break;
			}
		}
	}
	return inserted;
}

REGISTER_DIALOG(addon_reviews_list)

void taddon_reviews_list::pre_show(CVideo& /*video*/, twindow& window)
{
	tlistbox& list = find_widget<tlistbox>(&window, "reviews_list", false);
	FOREACH(AUTO & entry, addon.reviews)
	{
		entry.sorted = false;
	}

	for (unsigned int i = 0; i < addon.reviews.size(); i++) {
		int most = -1;
		int best = -1;
		for (unsigned int j = 0; j < addon.reviews.size(); j++) {
			if (addon.reviews[j].sorted == false && addon.reviews[j].likes > most) {
				most = addon.reviews[j].likes;
				best = j;
			}
		}
		if (best != -1) {
			std::string review_content;
			if (addon.reviews[best].gameplay.size() > 0) {
				std::string this_part(_("Gameplay: ") + addon.reviews[best].gameplay + "\n");
				review_content += cut_into_more_lines(this_part);
			}
			if (addon.reviews[best].visuals.size() > 0) {
				std::string this_part(_("Visuals: ") + addon.reviews[best].visuals + "\n");
				review_content += cut_into_more_lines(this_part);
			}
			if (addon.reviews[best].story.size() > 0) {
				std::string this_part(_("Story: ") + addon.reviews[best].story + "\n");
				review_content += cut_into_more_lines(this_part);
			}
			if (addon.reviews[best].balance.size() > 0) {
				std::string this_part(_("Balance: ") + addon.reviews[best].balance + "\n");
				review_content += cut_into_more_lines(this_part);
			}
			if (addon.reviews[best].overall.size() > 0) {
				std::string this_part(_("Overall: ") + addon.reviews[best].overall + "\n");
				review_content += cut_into_more_lines(this_part);
			}

			std::map<std::string, string_map> data;
			string_map column;

			column["label"] = review_content;
			data.insert(std::make_pair("review_box", column));

			std::string agreements(str_cast(addon.reviews[best].likes) + _(" people like this review. Do you like it too?"));
			column["label"] = agreements;
			data.insert(std::make_pair("people_who_agree", column));

			list.add_row(data);
			addon.reviews[best].sorted = true;
		}
	}

	connect_signal_mouse_left_click(
				find_widget<tbutton>(&window, "write_a_review", false)
				, boost::bind(
					&gui2::taddon_reviews_list::write_review_button_callback
					, this
					, boost::ref(window)));

}

void taddon_reviews_list::post_show(twindow& window)
{
	tlistbox& list = find_widget<tlistbox>(&window, "reviews_list", false);

	unsigned int i = 0;
	FOREACH(AUTO & entry, addon.reviews)
	{
		tgrid* row = list.get_row_grid(i);
		ttoggle_button& checkbox
				= find_widget<ttoggle_button>(row, "player_agrees", false);
		if (checkbox.get_value()) {
			current_users_rating_.liked_reviews.push_back(entry.id);
		}
		i++;
	}
}

void taddon_reviews_list::write_review_button_callback(twindow& window)
{
	current_users_rating_.submitted_review = gui2::taddon_review_write::execute(current_users_rating_.custom_review,window.video());
	if (current_users_rating_.submitted_review == true && current_users_rating_.custom_review.overall.size() == 0 &&
			current_users_rating_.custom_review.visuals.size() == 0 && current_users_rating_.custom_review.gameplay.size() == 0 &&
			current_users_rating_.custom_review.balance.size() == 0 && current_users_rating_.custom_review.story.size() == 0) {
		current_users_rating_.submitted_review = false;
	}
}


} // namespace gui2
