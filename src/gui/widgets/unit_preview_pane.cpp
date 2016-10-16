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

#include "gui/core/register_widget.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "gui/widgets/tree_view.hpp"
#include "gui/widgets/tree_view_node.hpp"

#include "font/text_formatting.hpp"
#include "formatter.hpp"
#include "formula/string_utils.hpp"
#include "gettext.hpp"
#include "help/help.hpp"
#include "font/marked-up_text.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "team.hpp"
#include "units/attack_type.hpp"
#include "units/types.hpp"
#include "units/unit.hpp"

#include <boost/iterator/zip_iterator.hpp>

#include "utils/functional.hpp"

namespace {
	template <typename... T>
	auto zip(const T&... containers) -> boost::iterator_range<boost::zip_iterator<decltype(boost::make_tuple(std::begin(containers)...))>>
	{
		auto zip_begin = boost::make_zip_iterator(boost::make_tuple(std::begin(containers)...));
		auto zip_end = boost::make_zip_iterator(boost::make_tuple(std::end(containers)...));
		return boost::make_iterator_range(zip_begin, zip_end);
	}
}

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(unit_preview_pane)

void tunit_preview_pane::finalize_setup()
{
	// Icons
	icon_type_              = find_widget<timage>(this, "type_image", false, false);
	icon_race_              = find_widget<timage>(this, "type_race", false, false);
	icon_alignment_         = find_widget<timage>(this, "type_alignment", false, false);

	// Labels
	label_name_             = find_widget<tlabel>(this, "type_name", false, false);
	label_level_            = find_widget<tlabel>(this, "type_level", false, false);
	label_race_             = find_widget<tlabel>(this, "type_race_label", false, false);
	label_details_          = find_widget<tcontrol>(this, "type_details", false, false);
	label_details_minimal_  = find_widget<tcontrol>(this, "type_details_minimal", false, false);

	tree_details_           = find_widget<ttree_view>(this, "tree_details", false, false);

	// Profile button
	button_profile_ = find_widget<tbutton>(this, "type_profile", false, false);

	if(button_profile_) {
		connect_signal_mouse_left_click(*button_profile_,
			std::bind(&tunit_preview_pane::profile_button_callback, this));
	}
}

static inline ttree_view_node& add_name_tree_node(ttree_view_node& header_node, const std::string& type, const t_string& label, const t_string& tooltip = "")
{
	/* Note: We have to pass data instead of just doing 'child_label.set_label(label)' below
	 * because the ttree_view_node::add_child needs to have the correct size of the
	 * node child widgets for its internal size calculations.
	 * Same is true for 'use_markup'
	 */
	auto& child_node = header_node.add_child(type, { { "name",{ { "label", label },{ "use_markup", "true" } } } });
	auto& child_label = find_widget<tcontrol>(&child_node, "name", true);

	child_label.set_tooltip(tooltip);
	return child_node;
}

/*
 * Both unit and unit_type use the same format (vector of attack_types) for their
 * attack data, meaning we can keep this as a helper function.
 */
template<typename T>
void tunit_preview_pane::print_attack_details(T attacks, ttree_view_node& parent_node)
{
	if (attacks.empty()) {
		return;
	}

	auto& header_node = add_name_tree_node(
		parent_node,
		"header",
		"<b>" + _("Attacks") + "</b>"
	);

	for (const auto& a : attacks) {

		auto& subsection = add_name_tree_node(
			header_node,
			"item",
			(formatter() << "<span color='#f5e6c1'>" << a.damage() << font::weapon_numbers_sep << a.num_attacks() << " " << a.name() << "</span>").str()
		);

		add_name_tree_node(
			subsection,
			"item",
			(formatter() << "<span color='#a69275'>" << a.range() << font::weapon_details_sep << a.type() << "</span>").str()
		);

		for (const auto& pair : a.special_tooltips())
		{
			add_name_tree_node(
				subsection,
				"item",
				(formatter() << font::span_color(font::weapon_details_color) << pair.first << "</span>").str(),
				(formatter() << _("Weapon special: ") << "<b>" << pair.first << "</b>" << '\n' << pair.second).str()
			);
		}
	}
}

void tunit_preview_pane::set_displayed_type(const unit_type& type)
{
	// Sets the current type id for the profile button callback to use
	current_type_ = type.id();

	if(icon_type_) {
		std::string mods;

		if(resources::controller) {
			mods = "~RC(" + type.flag_rgb() + ">" +
				 team::get_side_color_index(resources::controller->current_side())
				 + ")";
		}

		mods += "~SCALE_INTO_SHARP(144,144)" + image_mods_;

		icon_type_->set_label((type.icon().empty() ? type.image() : type.icon()) + mods);
	}

	if(label_name_) {
		label_name_->set_label("<big>" + type.type_name() + "</big>");
		label_name_->set_use_markup(true);
	}

	if(label_level_) {
		std::string l_str = vgettext("Lvl $lvl", {{"lvl", std::to_string(type.level())}});

		label_level_->set_label("<b>" + l_str + "</b>");
		label_level_->set_use_markup(true);
	}

	if(label_race_) {
		label_race_ ->set_label(type.race()->name(type.genders().front()));
	}

	if(icon_race_) {
		icon_race_->set_label("icons/unit-groups/race_" + type.race_id() + "_30.png");
	}

	if(icon_alignment_) {
		const std::string& alignment_name = type.alignment().to_string();

		icon_alignment_->set_label("icons/alignments/alignment_" + alignment_name + "_30.png");
		icon_alignment_->set_tooltip(unit_type::alignment_description(
			type.alignment(),
			type.genders().front()));
	}

	if(tree_details_) {
		std::stringstream str;
		str << "<small>";

		str << "<span color='#21e100'>"
			<< "<b>" << _("HP: ") << "</b>" << type.hitpoints() << "</span>" << " | ";

		str << "<span color='#00a0e1'>"
			<< "<b>" << _("XP: ") << "</b>" << type.experience_needed() << "</span>" << " | ";

		str << "<b>" << _("MP: ") << "</b>"
			<< type.movement();

		str << "</small>";

		tree_details_->clear();

		add_name_tree_node(
			tree_details_->get_root_node(),
			"item",
			str.str()
		);

		// Print trait details
		{
			ttree_view_node* header_node = nullptr;

			for(const auto& tr : type.possible_traits()) {
				t_string name = tr[type.genders().front() == unit_race::FEMALE ? "female_name" : "male_name"];
				if (tr["availability"] != "musthave" || name.empty()) {
					continue;
				}
				if (header_node == nullptr) {
					header_node = &add_name_tree_node(
						tree_details_->get_root_node(),
						"header",
						"<b>" + _("Traits") + "</b>"
					);
				}
				add_name_tree_node(
					*header_node,
					"item",
					name
				);
			}
		}

		// Print ability details
		if(!type.abilities().empty()) {

			auto& header_node = add_name_tree_node(
				tree_details_->get_root_node(),
				"header",
				"<b>" + _("Abilities") + "</b>"
			);

			for(const auto& ab : zip(type.abilities() , type.ability_tooltips())) {
				add_name_tree_node(
					header_node,
					"item",
					boost::get<0>(ab),
					boost::get<1>(ab)
				);
			}
		}

		print_attack_details(type.attacks(), tree_details_->get_root_node());
	}
}

void tunit_preview_pane::set_displayed_unit(const unit& u)
{
	// Sets the current type id for the profile button callback to use
	current_type_ = u.type_id();

	if(icon_type_) {
		std::string mods = u.image_mods();

		if(u.can_recruit()) {
			mods += "~BLIT(" + unit::leader_crown() + ")";
		}

		for(const std::string& overlay : u.overlays()) {
			mods += "~BLIT(" + overlay + ")";
		}

		mods += "~SCALE_INTO_SHARP(144,144)" + image_mods_;

		icon_type_->set_label(u.absolute_image() + mods);
	}

	if(label_name_) {
		std::string name;
		if(!u.name().empty()) {
			name = "<span size='large'>" + u.name() + "</span>" + "\n" + "<small><span color='#a69275'>" + u.type_name() + "</span></small>";
		} else {
			name = "<span size='large'>" + u.type_name() + "</span>\n";
		}

		label_name_->set_label(name);
		label_name_->set_use_markup(true);
	}

	if(label_level_) {
		std::string l_str = vgettext("Lvl $lvl", {{"lvl", std::to_string(u.level())}});

		label_level_->set_label("<b>" + l_str + "</b>");
		label_level_->set_use_markup(true);
	}

	if(label_race_) {
		label_race_->set_label(u.race()->name(u.gender()));
	}

	if(icon_race_) {
		icon_race_->set_label("icons/unit-groups/race_" + u.race()->id() + "_30.png");
	}

	if(icon_alignment_) {
		const std::string& alignment_name = u.alignment().to_string();

		icon_alignment_->set_label("icons/alignments/alignment_" + alignment_name + "_30.png");
		icon_alignment_->set_tooltip(unit_type::alignment_description(
			u.alignment(),
			u.gender()));
	}

	if(label_details_minimal_) {
		std::stringstream str;

		const std::string name = "<span size='large'>" + (!u.name().empty() ? u.name() : " ") + "</span>";
		str << name << "\n";

		str << "<span color='#a69275'>" << u.type_name() << "</span>" << "\n";

		str << "Lvl " << u.level() << "\n";

		str << u.alignment() << "\n";

		str << utils::join(u.trait_names(), ", ") << "\n";

		str << font::span_color(u.hp_color())
			<< _("HP: ") << u.hitpoints() << "/" << u.max_hitpoints() << "</span>" << "\n";

		str << font::span_color(u.xp_color())
			<< _("XP: ") << u.experience() << "/" << u.max_experience() << "</span>";

		label_details_minimal_->set_label(str.str());
		label_details_minimal_->set_use_markup(true);
	}

	if(tree_details_) {
		std::stringstream str;
		str << "<small>";

		str << font::span_color(u.hp_color())
			<< "<b>" << _("HP: ") << "</b>" << u.hitpoints() << "/" << u.max_hitpoints() << "</span>" << " | ";

		str << font::span_color(u.xp_color())
			<< "<b>" << _("XP: ") << "</b>" << u.experience() << "/" << u.max_experience() << "</span>" << " | ";

		str << "<b>" << _("MP: ") << "</b>"
			<< u.movement_left() << "/" << u.total_movement();

		str << "</small>";

		tree_details_->clear();

		add_name_tree_node(
			tree_details_->get_root_node(),
			"item",
			str.str()
		);

		if (!u.trait_names().empty()) {
			auto& header_node = add_name_tree_node(
				tree_details_->get_root_node(),
				"header",
				"<b>" + _("Traits") + "</b>"
			);

			assert(u.trait_names().size() == u.trait_descriptions().size());
			for (size_t i = 0; i < u.trait_names().size(); ++i) {
				add_name_tree_node(
					header_node,
					"item",
					u.trait_names()[i],
					u.trait_descriptions()[i]
				);
			}
		}
		if (!u.get_ability_list().empty()) {
			auto& header_node = add_name_tree_node(
				tree_details_->get_root_node(),
				"header",
				"<b>" + _("Abilities") + "</b>"
			);

			for (const auto& ab : u.ability_tooltips()) {
				add_name_tree_node(
					header_node,
					"item",
					std::get<1>(ab),
					std::get<2>(ab)
				);
			}
		}
		print_attack_details(u.attacks(), tree_details_->get_root_node());
	}
}

void tunit_preview_pane::profile_button_callback()
{
	if(get_window()) {
		help::show_unit_help((*get_window()).video(), current_type_);
	}
}

void tunit_preview_pane::set_image_mods(const std::string& mods)
{
	image_mods_ = mods;
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

// }---------- DEFINITION ---------{

tunit_preview_pane_definition::tunit_preview_pane_definition(const config& cfg)
	: tcontrol_definition(cfg)
{
	DBG_GUI_P << "Parsing unit preview pane " << id << '\n';

	load_resolutions<tresolution>(cfg);
}

tunit_preview_pane_definition::tresolution::tresolution(const config& cfg)
	: tresolution_definition_(cfg), grid()
{
	state.push_back(tstate_definition(cfg.child("background")));
	state.push_back(tstate_definition(cfg.child("foreground")));

	const config& child = cfg.child("grid");
	VALIDATE(child, _("No grid defined."));

	grid = std::make_shared<tbuilder_grid>(child);
}

// }---------- BUILDER -----------{

namespace implementation
{

tbuilder_unit_preview_pane::tbuilder_unit_preview_pane(const config& cfg)
	: tbuilder_control(cfg)
	, image_mods_(cfg["image_mods"])
{
}

twidget* tbuilder_unit_preview_pane::build() const
{
	tunit_preview_pane* widget = new tunit_preview_pane();

	init_control(widget);

	DBG_GUI_G << "Window builder: placed unit preview pane '" << id
			  << "' with definition '" << definition << "'.\n";

	std::shared_ptr<const tunit_preview_pane_definition::tresolution> conf
		= std::static_pointer_cast<
			const tunit_preview_pane_definition::tresolution>(widget->config());

	assert(conf);

	widget->init_grid(conf->grid);
	widget->finalize_setup();
	widget->set_image_mods(image_mods_);

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
