/*
   Copyright (C) 2009 - 2015 by Ignacio R. Morelle <shadowm2006@gmail.com>
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

#include "gui/dialogs/unit_create.hpp"

#include "gui/auxiliary/find_widget.tpp"
#include "gui/auxiliary/log.hpp"
#include "gui/dialogs/helper.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/settings.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/grid.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/window.hpp"
#include "display.hpp"
#include "marked-up_text.hpp"
#include "help/help.hpp"
#include "game_config.hpp"
#include "gettext.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "team.hpp"
#include "unit_types.hpp"

#include "utils/foreach.tpp"

#include <boost/bind.hpp>

namespace
{
static std::string last_chosen_type_id = "";
static unit_race::GENDER last_gender = unit_race::MALE;

/**
 * Helper function for updating the male/female checkboxes.
 * It's not a private member of class gui2::tunit_create so
 * we don't have to expose a forward-declaration of ttoggle_button
 * in the interface.
 */
void update_male_female_toggles(gui2::ttoggle_button& male,
								gui2::ttoggle_button& female,
								unit_race::GENDER choice)
{
	male.set_value(choice == unit_race::MALE);
	female.set_value(choice == unit_race::FEMALE);
}
}

namespace gui2
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_unit_create
 *
 * == Unit create ==
 *
 * This shows the debug-mode dialog to create new units on the map.
 *
 * @begin{table}{dialog_widgets}
 *
 * male_toggle & & toggle_button & m &
 *         Option button to select the "male" gender for created units. $
 *
 * female_toggle & & toggle_button & m &
 *         Option button to select the "female" gender for created units. $
 *
 * unit_type_list & & listbox & m &
 *         Listbox displaying existing unit types sorted by name and race. $
 *
 * -unit_type & & control & m &
 *         Widget which shows the unit type name label. $
 *
 * -race & & control & m &
 *         Widget which shows the unit race name label. $
 *
 * @end{table}
 */

REGISTER_DIALOG(unit_create)

tunit_create::tunit_create(display* disp)
	: gender_(last_gender)
	, choice_(last_chosen_type_id)
	, last_words_()
	, disp_(disp)
{
}

void tunit_create::pre_show(CVideo& /*video*/, twindow& window)
{
	ttoggle_button& male_toggle
			= find_widget<ttoggle_button>(&window, "male_toggle", false);
	ttoggle_button& female_toggle
			= find_widget<ttoggle_button>(&window, "female_toggle", false);
	tlistbox& list = find_widget<tlistbox>(&window, "unit_type_list", false);

	ttext_box* filter
			= find_widget<ttext_box>(&window, "filter_box", false, true);

	filter->set_text_changed_callback(
			boost::bind(&tunit_create::filter_text_changed, this, _1, _2));

	window.keyboard_capture(filter);

#ifdef GUI2_EXPERIMENTAL_LISTBOX
	connect_signal_notify_modified(*list,
								   boost::bind(&tunit_create::list_item_clicked,
											   *this,
											   boost::ref(window)));
#else
	list.set_callback_value_change(
			dialog_callback<tunit_create, &tunit_create::list_item_clicked>);
#endif

	connect_signal_mouse_left_click(
			find_widget<tbutton>(&window, "type_profile", false),
			boost::bind(&tunit_create::profile_button_callback,
						this,
						boost::ref(window)));

	male_toggle.set_callback_state_change(
			dialog_callback<tunit_create, &tunit_create::gender_toggle_callback>);

	female_toggle.set_callback_state_change(
			dialog_callback<tunit_create, &tunit_create::gender_toggle_callback>);

	update_male_female_toggles(male_toggle, female_toggle, gender_);

	list.clear();

	FOREACH(const AUTO & i, unit_types.types())
	{
		if(i.second.do_not_list())
			continue;

		// Make sure this unit type is built with the data we need.
		unit_types.build_unit_type(i.second, unit_type::FULL);

		units_.push_back(&i.second);

		std::map<std::string, string_map> row_data;
		string_map column;

		column["label"] = units_.back()->race()->plural_name();
		row_data.insert(std::make_pair("race", column));
		column["label"] = units_.back()->type_name();
		row_data.insert(std::make_pair("unit_type", column));

		list.add_row(row_data);

		// Select the previous choice, if any.
		if(choice_.empty() != true && choice_ == i.first) {
			list.select_row(list.get_item_count() - 1);
		}
	}

	if(units_.empty()) {
		ERR_GUI_G << "no unit types found for unit create dialog; not good"
				  << std::endl;
	}

	std::vector<tgenerator_::torder_func> order_funcs(2);
	order_funcs[0] = boost::bind(&tunit_create::compare_race, this, _1, _2);
	order_funcs[1] = boost::bind(&tunit_create::compare_race_rev, this, _1, _2);
	list.set_column_order(0, order_funcs);
	order_funcs[0] = boost::bind(&tunit_create::compare_type, this, _1, _2);
	order_funcs[1] = boost::bind(&tunit_create::compare_type_rev, this, _1, _2);
	list.set_column_order(1, order_funcs);

	list_item_clicked(window);
}

bool tunit_create::compare_type(unsigned i1, unsigned i2) const
{
	return units_[i1]->type_name().str() < units_[i2]->type_name().str();
}

bool tunit_create::compare_race(unsigned i1, unsigned i2) const
{
	return units_[i1]->race()->plural_name().str() < units_[i2]->race()->plural_name().str();
}

bool tunit_create::compare_type_rev(unsigned i1, unsigned i2) const
{
	return units_[i1]->type_name().str() > units_[i2]->type_name().str();
}

bool tunit_create::compare_race_rev(unsigned i1, unsigned i2) const
{
	return units_[i1]->race()->plural_name().str() > units_[i2]->race()->plural_name().str();
}

void tunit_create::post_show(twindow& window)
{
	ttoggle_button& female_toggle
			= find_widget<ttoggle_button>(&window, "female_toggle", false);
	tlistbox& list = find_widget<tlistbox>(&window, "unit_type_list", false);

	choice_ = "";

	if(get_retval() != twindow::OK) {
		return;
	}

	const int selected_row = list.get_selected_row();
	if(selected_row < 0) {
		return;
	} else if(static_cast<size_t>(selected_row) >= units_.size()) {
		// FIXME: maybe assert?
		ERR_GUI_G << "unit create dialog has more list items than known unit "
					 "types; not good\n";
		return;
	}

	last_chosen_type_id = choice_
			= units_[selected_row]->id();
	last_gender = gender_ = female_toggle.get_value() ? unit_race::FEMALE
													  : unit_race::MALE;
}

void tunit_create::print_stats(std::stringstream& str, const int row)
{
	const unit_type* u = units_[row];

	str << "<b>" << _("HP: ") << "</b>"
		<< "<span color='#21e100'>" << u->hitpoints() << "</span> ";

	str << "<b>" << _("XP: ") << "</b>"
		<< "<span color='#00a0e1'>" << u->experience_needed() << "</span> ";

	str << "<b>" << _("MP: ") << "</b>"
		<< u->movement() << "\n";

	str << " \n";

	// Print trait details
	bool has_traits = false;
	std::stringstream t_str;

	BOOST_FOREACH(const config& tr, u->possible_traits())
	{
		if(tr["availability"] != "musthave") continue;

		const std::string gender_string = 
			u->genders().front() == unit_race::FEMALE ? "female_name" : "male_name";

		t_string name = tr[gender_string];
		if(name.empty()) {
			name = tr["name"];
		}

		if(!name.empty()) {
			t_str << "  " << name << "\n";
		}

		has_traits = true;
	}

	if(has_traits) {
		str << "<b>" << "Traits" << "</b>" << "\n";
		str << t_str.str();
		str << " \n";
	}

	// Print ability details
	if(!u->abilities().empty()) {
		str << "<b>" << "Abilities" << "</b>" << "\n";

		BOOST_FOREACH(const std::string& ab, u->abilities())
		{
			str << "  " << ab << "\n";
		}

		str << " \n";
	}

	// Print attack details
	if(!u->attacks().empty()) {
		str << "<b>" << "Attacks" << "</b>" << "\n";

		BOOST_FOREACH(const attack_type& a, u->attacks())
		{
			str << "<span color='#f5e6c1'>" << a.num_attacks() 
				<< font::weapon_numbers_sep << a.damage() << " " << a.name() << "</span>" << "\n";

			str << "<span color='#a69275'>" << "  " << a.range() 
				<< font::weapon_details_sep << a.type() << "</span>" << "\n";

			const std::string special = a.weapon_specials();
			if (!special.empty()) {
				str << "<span color='#a69275'>" << "  " << special << "</span>" << "\n";
			}

			const std::string accuracy_parry = a.accuracy_parry_description();
			if(!accuracy_parry.empty()) {
				str << "<span color='#a69275'>" << "  " << accuracy_parry << "</span>" << "\n";
			}

			str << " \n";
		}
	}
}

void tunit_create::list_item_clicked(twindow& window)
{
	const int selected_row 
			= find_widget<tlistbox>(&window, "unit_type_list", false).get_selected_row();

	const unit_type* u = units_[selected_row];

	std::stringstream str;
	print_stats(str, selected_row);

	std::string tc;

	if(resources::controller) {
		tc = "~RC(" + u->flag_rgb() + ">" +
			 team::get_side_color_index(resources::controller->current_side())
			 + ")";
	}

	const std::string& alignment_name = unit_type::alignment_description(
		u->alignment(),
		u->genders().front());

	find_widget<timage>(&window, "type_image", false)
			.set_label((u->icon().empty() ? u->image() : u->icon()) + tc);

	tlabel& u_name = find_widget<tlabel>(&window, "type_name", false);

	u_name.set_label("<big>" + u->type_name() + "</big>");
	u_name.set_use_markup(true);

	std::stringstream l_str;
	l_str << "<span size='x-large'>" << "L " << u->level() << "</span>";

	tlabel& l_label = find_widget<tlabel>(&window, "type_level", false);

	l_label.set_label(l_str.str());
	l_label.set_use_markup(true);

	timage& r_icon = find_widget<timage>(&window, "type_race", false);

	r_icon.set_label("icons/unit-groups/race_" + u->race_id() + "_30.png");
	r_icon.set_tooltip(u->race()->name(u->genders().front()));

	timage& a_icon = find_widget<timage>(&window, "type_alignment", false);

	a_icon.set_label("icons/alignments/alignment_" + alignment_name + "_30.png");
	a_icon.set_tooltip(alignment_name);

	tlabel& details = find_widget<tlabel>(&window, "type_details", false);

	details.set_label(str.str());
	details.set_use_markup(true);
}

bool tunit_create::filter_text_changed(ttext_* textbox, const std::string& text)
{
	twindow& window = *textbox->get_window();

	tlistbox& list = find_widget<tlistbox>(&window, "unit_type_list", false);

	const std::vector<std::string> words = utils::split(text, ' ');

	if(words == last_words_)
		return false;
	last_words_ = words;

	std::vector<bool> show_items(list.get_item_count(), true);

	if(!text.empty()) {
		for(unsigned int i = 0; i < list.get_item_count(); i++) {
			tgrid* row = list.get_row_grid(i);

			tgrid::iterator it = row->begin();
			tlabel& type_label
					= find_widget<tlabel>(*it, "unit_type", false);

			bool found = false;
			FOREACH(const AUTO & word, words)
			{
				found = std::search(type_label.label().str().begin(),
									type_label.label().str().end(),
									word.begin(),
									word.end(),
									chars_equal_insensitive)
						!= type_label.label().str().end();

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

void tunit_create::profile_button_callback(twindow& window)
{
	if(!disp_) {
		return;
	}

	const int selected_row
			= find_widget<tlistbox>(&window, "unit_type_list", false).get_selected_row();

	help::show_unit_help(*disp_,
		units_[selected_row]->id(),
		units_[selected_row]->show_variations_in_help(), false);
}

void tunit_create::gender_toggle_callback(twindow& window)
{
	ttoggle_button& male_toggle
			= find_widget<ttoggle_button>(&window, "male_toggle", false);
	ttoggle_button& female_toggle
			= find_widget<ttoggle_button>(&window, "female_toggle", false);

	// TODO Ye olde ugly hack for the lack of radio buttons.

	if(gender_ == unit_race::MALE) {
		gender_ = female_toggle.get_value() ? unit_race::FEMALE
											: unit_race::MALE;
	} else {
		gender_ = male_toggle.get_value() ? unit_race::MALE : unit_race::FEMALE;
	}

	update_male_female_toggles(male_toggle, female_toggle, gender_);
}
}
