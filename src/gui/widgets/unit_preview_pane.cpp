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
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "help/help.hpp"
#include "help/help_impl.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "team.hpp"
#include "units/attack_type.hpp"
#include "units/types.hpp"
#include "units/helper.hpp"
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

void unit_preview_pane::finalize_setup()
{
	// Icons
	icon_type_              = find_widget<image>(this, "type_image", false, false);
	icon_race_              = find_widget<image>(this, "type_race", false, false);
	icon_alignment_         = find_widget<image>(this, "type_alignment", false, false);

	// Labels
	label_name_             = find_widget<label>(this, "type_name", false, false);
	label_level_            = find_widget<label>(this, "type_level", false, false);
	label_race_             = find_widget<label>(this, "type_race_label", false, false);
	label_details_          = find_widget<styled_widget>(this, "type_details", false, false);
	label_details_minimal_  = find_widget<styled_widget>(this, "type_details_minimal", false, false);

	tree_details_           = find_widget<tree_view>(this, "tree_details", false, false);

	// Profile button
	button_profile_ = find_widget<button>(this, "type_profile", false, false);

	if(button_profile_) {
		connect_signal_mouse_left_click(*button_profile_,
			std::bind(&unit_preview_pane::profile_button_callback, this));
	}
}

static inline tree_view_node& add_name_tree_node(tree_view_node& header_node, const std::string& type, const t_string& label, const t_string& tooltip = "")
{
	/* Note: We have to pass data instead of just doing 'child_label.set_label(label)' below
	 * because the tree_view_node::add_child needs to have the correct size of the
	 * node child widgets for its internal size calculations.
	 * Same is true for 'use_markup'
	 */
	auto& child_node = header_node.add_child(type, { { "name",{ { "label", label },{ "use_markup", "true" } } } });
	auto& child_label = find_widget<styled_widget>(&child_node, "name", true);

	child_label.set_tooltip(tooltip);
	return child_node;
}

static inline std::string get_hp_tooltip(const utils::string_map& res, const std::function<int (const std::string&, bool)>& get)
{
	std::ostringstream tooltip;

	std::set<std::string> resistances_table;

	bool att_def_diff = false;
	for(const utils::string_map::value_type &resist : res) {
		std::ostringstream line;
		line << translation::gettext(resist.first.c_str()) << ": ";

		// Some units have different resistances when attacking or defending.
		const int res_att = 100 - get(resist.first, true);
		const int res_def = 100 - get(resist.first, false);

		if(res_att == res_def) {
			line << "<span color='" << unit_helper::resistance_color(res_def) << "'>" << utils::signed_percent(res_def) << "</span>";
		} else {
			line << "<span color='" << unit_helper::resistance_color(res_att) << "'>" << utils::signed_percent(res_att) << "</span>" << "/"
			     << "<span color='" << unit_helper::resistance_color(res_def) << "'>" << utils::signed_percent(res_def) << "</span>";
			att_def_diff = true;
		}

		resistances_table.insert(line.str());
	}

	tooltip << "<big>" << _("Resistances: ") << "</big>";
	if(att_def_diff) {
		tooltip << _("(Att / Def)");
	}

	for(const std::string &line : resistances_table) {
		tooltip << '\n' << font::unicode_bullet << " " << line;
	}

	return tooltip.str();
}

static inline std::string get_mp_tooltip(int total_movement, std::function<int (t_translation::terrain_code)> get)
{
	std::ostringstream tooltip;
	tooltip << "<big>" << _("Movement Costs:") << "</big>";

	ter_data_cache tdata = help::load_terrain_types_data();

	if(!tdata) {
		return "";
	}

	for(t_translation::terrain_code terrain : preferences::encountered_terrains()) {
		if(terrain == t_translation::FOGGED || terrain == t_translation::VOID_TERRAIN || t_translation::terrain_matches(terrain, t_translation::ALL_OFF_MAP)) {
			continue;
		}

		const terrain_type& info = tdata->get_terrain_info(terrain);
		if(info.union_type().size() == 1 && info.union_type()[0] == info.number() && info.is_nonnull()) {
			const std::string& name = info.name();
			const int moves = get(terrain);

			tooltip << '\n' << font::unicode_bullet << " " << name << ": ";

			// movement  -  range: 1 .. 5, movetype::UNREACHABLE=impassable
			const bool cannot_move = moves > total_movement;

			std::string color;
			if(cannot_move) {
				// cannot move in this terrain
				color = "red";
			} else if(moves > 1) {
				color = "yellow";
			} else {
				color = "white";
			}

			tooltip << "<span color='" << color << "'>";

			// A 5 MP margin; if the movement costs go above the unit's max moves + 5, we replace it with dashes.
			if(cannot_move && (moves > total_movement + 5)) {
				tooltip << font::unicode_figure_dash;
			} else {
				tooltip << moves;
			}

			tooltip << "</span>";
		}
	}

	return tooltip.str();
}

/*
 * Both unit and unit_type use the same format (vector of attack_types) for their
 * attack data, meaning we can keep this as a helper function.
 */
template<typename T>
void unit_preview_pane::print_attack_details(T attacks, tree_view_node& parent_node)
{
	if(attacks.empty()) {
		return;
	}

	auto& header_node = add_name_tree_node(
		parent_node,
		"header",
		"<b>" + _("Attacks") + "</b>"
	);

	for(const auto& a : attacks) {

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

		for(const auto& pair : a.special_tooltips()) {
			add_name_tree_node(
				subsection,
				"item",
				(formatter() << font::span_color(font::weapon_details_color) << pair.first << "</span>").str(),
				(formatter() << "<span size='x-large'>" << pair.first << "</span>" << "\n" << pair.second).str()
			);
		}
	}
}

void unit_preview_pane::set_displayed_type(const unit_type& type)
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

		tree_details_->clear();
		tree_details_->add_node("hp_xp_mp", {
			{ "hp",{
				{ "label", (formatter() << "<small>" << "<span color='#21e100'>" << "<b>" << _("HP: ") << "</b>" << type.hitpoints() << "</span>" << " | </small>").str() },
				{ "use_markup", "true" },
				{ "tooltip", get_hp_tooltip(type.movement_type().get_resistances().damage_table(), [&type](const std::string& dt, bool is_attacker) { return type.resistance_against(dt, is_attacker); }) }
			} },
			{ "xp",{
				{ "label", (formatter() << "<small>" << "<span color='#00a0e1'>" << "<b>" << _("XP: ") << "</b>" << type.experience_needed() << "</span>" << " | </small>").str() },
				{ "use_markup", "true" },
				{ "tooltip", (formatter() << _("Experience Modifier: ") << unit_experience_accelerator::get_acceleration() << '%').str() }
			} },
			{ "mp",{
				{ "label", (formatter() << "<small>" << "<b>" << _("MP: ") << "</b>" << type.movement() << "</small>").str() },
				{ "use_markup", "true" },
				{ "tooltip", get_mp_tooltip(type.movement(), [&type](t_translation::terrain_code terrain) { return type.movement_type().movement_cost(terrain); }) }
			} },
		});

		// Print trait details
		{
			tree_view_node* header_node = nullptr;

			for(const auto& tr : type.possible_traits()) {
				t_string name = tr[type.genders().front() == unit_race::FEMALE ? "female_name" : "male_name"];
				if(tr["availability"] != "musthave" || name.empty()) {
					continue;
				}

				if(header_node == nullptr) {
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
					(formatter() << "<span size='x-large'>" << boost::get<0>(ab) << "</span>\n" << boost::get<1>(ab)).str()
				);
			}
		}

		print_attack_details(type.attacks(), tree_details_->get_root_node());
	}
}

void unit_preview_pane::set_displayed_unit(const unit& u)
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
		tree_details_->clear();
		tree_details_->add_node("hp_xp_mp", {
			{ "hp",{
				{ "label", (formatter() << "<small>" << font::span_color(u.hp_color()) << "<b>" << _("HP: ") << "</b>" << u.hitpoints() << "/" << u.max_hitpoints() << "</span>" << " | </small>").str() },
				{ "use_markup", "true" },
				{ "tooltip", get_hp_tooltip(u.get_base_resistances(), [&u](const std::string& dt, bool is_attacker) { return u.resistance_against(dt, is_attacker, u.get_location()); }) }
			} },
			{ "xp",{
				{ "label", (formatter() << "<small>" << font::span_color(u.xp_color()) << "<b>" << _("XP: ") << "</b>" << u.experience() << "/" << u.max_experience() << "</span>" << " | </small>").str() },
				{ "use_markup", "true" },
				{ "tooltip", (formatter() << _("Experience Modifier: ") << unit_experience_accelerator::get_acceleration() << '%').str() }
			} },
			{ "mp",{
				{ "label", (formatter() << "<small>" << "<b>" << _("MP: ") << "</b>" << u.movement_left() << "/" << u.total_movement() << "</small>").str() },
				{ "use_markup", "true" },
				{ "tooltip", get_mp_tooltip(u.total_movement(), [&u](t_translation::terrain_code terrain) { return u.movement_cost(terrain); }) }
			} },
		});

		if(!u.trait_names().empty()) {
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

		if(!u.get_ability_list().empty()) {
			auto& header_node = add_name_tree_node(
				tree_details_->get_root_node(),
				"header",
				"<b>" + _("Abilities") + "</b>"
			);

			for(const auto& ab : u.ability_tooltips()) {
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

void unit_preview_pane::profile_button_callback()
{
	if(get_window()) {
		const unit_type* ut = unit_types.find(current_type_);
		if(ut != nullptr) {
			help::show_unit_description((*get_window()).video(), *ut);
		}
	}
}

void unit_preview_pane::set_image_mods(const std::string& mods)
{
	image_mods_ = mods;
}

void unit_preview_pane::set_active(const bool /*active*/)
{
	/* DO NOTHING */
}

bool unit_preview_pane::get_active() const
{
	return true;
}

unsigned unit_preview_pane::get_state() const
{
	return ENABLED;
}

const std::string& unit_preview_pane::get_control_type() const
{
	static const std::string type = "unit_preview_pane";
	return type;
}

void unit_preview_pane::set_self_active(const bool /*active*/)
{
	/* DO NOTHING */
}

// }---------- DEFINITION ---------{

unit_preview_pane_definition::unit_preview_pane_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing unit preview pane " << id << '\n';

	load_resolutions<resolution>(cfg);
}

unit_preview_pane_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg), grid()
{
	state.push_back(state_definition(cfg.child("background")));
	state.push_back(state_definition(cfg.child("foreground")));

	const config& child = cfg.child("grid");
	VALIDATE(child, _("No grid defined."));

	grid = std::make_shared<builder_grid>(child);
}

// }---------- BUILDER -----------{

namespace implementation
{

builder_unit_preview_pane::builder_unit_preview_pane(const config& cfg)
	: builder_styled_widget(cfg)
	, image_mods_(cfg["image_mods"])
{
}

widget* builder_unit_preview_pane::build() const
{
	unit_preview_pane* widget = new unit_preview_pane();

	init_control(widget);

	DBG_GUI_G << "Window builder: placed unit preview pane '" << id
			  << "' with definition '" << definition << "'.\n";

	std::shared_ptr<const unit_preview_pane_definition::resolution> conf
		= std::static_pointer_cast<
			const unit_preview_pane_definition::resolution>(widget->config());

	assert(conf);

	widget->init_grid(conf->grid);
	widget->finalize_setup();
	widget->set_image_mods(image_mods_);

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
