/*
   Copyright (C) 2016 The Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/widgets/unit_preview_pane.hpp"

#include "gui/auxiliary/find_widget.hpp"
#include "gui/auxiliary/widget_definition/unit_preview_pane.hpp"
#include "gui/auxiliary/window_builder/unit_preview_pane.hpp"

#include "gui/widgets/detail/register.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"

#include "formula_string_utils.hpp"
#include "game_config.hpp"
#include "gettext.hpp"
#include "help/help.hpp"
#include "marked-up_text.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "team.hpp"

#include "utils/foreach.hpp"

#include <boost/bind.hpp>

namespace gui2
{

REGISTER_WIDGET(unit_preview_pane)

void tunit_preview_pane::finalize_setup()
{
	// Icons
	icon_type_      = find_widget<timage>(this, "type_image" , false, false);
	icon_race_      = find_widget<timage>(this, "type_race" , false, false);
	icon_alignment_ = find_widget<timage>(this, "type_alignment", false, false);

	// Labels
	label_name_     = find_widget<tlabel>(this, "type_name" , false, false);
	label_level_    = find_widget<tlabel>(this, "type_level" , false, false);
	label_details_  = find_widget<tlabel>(this, "type_details", false, false);

	// Profile button
	button_profile_ = find_widget<tbutton>(this, "type_profile", false, false);

	if(button_profile_) {
		connect_signal_mouse_left_click(*button_profile_,
			boost::bind(&tunit_preview_pane::profile_button_callback, this));
	}
}

void tunit_preview_pane::set_displayed_type(const unit_type* type)
{
	// Sets the current type id for the profile button callback to use
	current_type_ = type->id();

	if(icon_type_) {
		std::string tc;

		if(resources::controller) {
			tc = "~RC(" + type->flag_rgb() + ">" +
				 team::get_side_color_index(resources::controller->current_side())
				 + ")";
		}

		icon_type_->set_label((type->icon().empty() ? type->image() : type->icon()) + tc);
	}

	if(label_name_) {
		label_name_->set_label("<big>" + type->type_name() + "</big>");
		label_name_->set_use_markup(true);
	}

	if(label_level_) {
		utils::string_map symbols;
		symbols["lvl"] = lexical_cast<std::string>(type->level());

		std::string l_str = vgettext("Lvl $lvl", symbols);

		label_level_->set_label("<b>" + l_str + "</b>");
		label_level_->set_use_markup(true);
	}

	if(icon_race_) {
		icon_race_->set_label("icons/unit-groups/race_" + type->race_id() + "_30.png");
		icon_race_->set_tooltip(type->race()->name(type->genders().front()));
	}

	if(icon_alignment_) {
		const std::string& alignment_name = type->alignment().to_string();

		icon_alignment_->set_label("icons/alignments/alignment_" + alignment_name + "_30.png");
		icon_alignment_->set_tooltip(unit_type::alignment_description(
			type->alignment(),
			type->genders().front()));
	}

	if(label_details_) {
		std::stringstream str;
		str << "<b>" << _("HP: ") << "</b>"
			<< "<span color='#21e100'>" << type->hitpoints() << "</span> ";

		str << "<b>" << _("XP: ") << "</b>"
			<< "<span color='#00a0e1'>" << type->experience_needed() << "</span> ";

		str << "<b>" << _("MP: ") << "</b>"
			<< type->movement() << "\n";

		str << " \n";

		// Print trait details
		bool has_traits = false;
		std::stringstream t_str;

		FOREACH(const AUTO& tr, type->possible_traits())
		{
			if(tr["availability"] != "musthave") continue;

			const std::string gender_string =
				type->genders().front() == unit_race::FEMALE ? "female_name" : "male_name";

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
			str << "<b>" << _("Traits") << "</b>" << "\n";
			str << t_str.str();
			str << " \n";
		}

		// Print ability details
		if(!type->abilities().empty()) {
			str << "<b>" << _("Abilities") << "</b>" << "\n";

			FOREACH(const AUTO& ab, type->abilities())
			{
				str << "  " << ab << "\n";
			}

			str << " \n";
		}

		// Print attack details
		if(!type->attacks().empty()) {
			str << "<b>" << _("Attacks") << "</b>" << "\n";

			FOREACH(const AUTO& a, type->attacks())
			{
				str << "<span color='#f5e6c1'>" << a.damage()
					<< utils::unicode_multiplication_sign << a.num_attacks() << " " << a.name() << "</span>" << "\n";

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
			}
		}

		label_details_->set_label(str.str());
		label_details_->set_use_markup(true);
	}
}

void tunit_preview_pane::profile_button_callback()
{
	if(get_window()) {
		help::show_unit_help((*get_window()).video(), current_type_);
	}
}

void tunit_preview_pane::set_active(const bool /*active*/)
{
	/* DO NOTHING */
}

bool tunit_preview_pane::get_active() const
{
	return true;
}

unsigned tunit_preview_pane::get_state() const
{
	return ENABLED;
}

const std::string& tunit_preview_pane::get_control_type() const
{
	static const std::string type = "unit_preview_pane";
	return type;
}

void tunit_preview_pane::set_self_active(const bool /*active*/)
{
	/* DO NOTHING */
}

} // namespace gui2
