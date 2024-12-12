/*
	Copyright (C) 2016 - 2024
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

#include "gui/widgets/unit_preview_pane.hpp"


#include "gui/core/register_widget.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/drawing.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/tree_view.hpp"
#include "gui/widgets/tree_view_node.hpp"

#include "formatter.hpp"
#include "formula/string_utils.hpp"
#include "language.hpp"
#include "preferences/preferences.hpp"
#include "gettext.hpp"
#include "help/help.hpp"
#include "help/help_impl.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "serialization/markup.hpp"
#include "team.hpp"
#include "terrain/movement.hpp"
#include "terrain/type_data.hpp"
#include "units/types.hpp"
#include "units/helper.hpp"
#include "units/unit.hpp"
#include "wml_exception.hpp"

#include <functional>

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(unit_preview_pane)

unit_preview_pane::unit_preview_pane(const implementation::builder_unit_preview_pane& builder)
	: container_base(builder, type())
	, current_type_()
	, icon_type_(nullptr)
	, icon_race_(nullptr)
	, icon_alignment_(nullptr)
	, label_name_(nullptr)
	, label_level_(nullptr)
	, label_race_(nullptr)
	, label_details_(nullptr)
	, tree_details_(nullptr)
	, button_profile_(nullptr)
	, image_mods_(builder.image_mods)
{
}

void unit_preview_pane::finalize_setup()
{
	// Icons
	icon_type_              = find_widget<drawing>("type_image", false, false);
	icon_race_              = find_widget<image>("type_race", false, false);
	icon_alignment_         = find_widget<image>("type_alignment", false, false);

	// Labels
	label_name_             = find_widget<label>("type_name", false, false);
	label_level_            = find_widget<label>("type_level", false, false);
	label_race_             = find_widget<label>("type_race_label", false, false);
	label_details_          = find_widget<styled_widget>("type_details_minimal", false, false);

	tree_details_           = find_widget<tree_view>("type_details", false, false);

	// Profile button
	button_profile_ = find_widget<button>("type_profile", false, false);

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
	auto& child_label = child_node.find_widget<styled_widget>("name", true);

	child_label.set_tooltip(tooltip);
	return child_node;
}

static inline std::string get_hp_tooltip(
	const utils::string_map_res& res, const std::function<int(const std::string&, bool)>& get)
{
	std::ostringstream tooltip;

	std::vector<std::string> resistances_table;

	bool att_def_diff = false;
	for(const utils::string_map_res::value_type &resist : res) {
		std::ostringstream line;
		line << translation::dgettext("wesnoth", resist.first.c_str()) << ": ";

		// Some units have different resistances when attacking or defending.
		const int res_att = 100 - get(resist.first, true);
		const int res_def = 100 - get(resist.first, false);

		if(res_att == res_def) {
			line << markup::span_color(unit_helper::resistance_color(res_def), "\t", utils::signed_percent(res_def));
		} else {
			line << markup::span_color(unit_helper::resistance_color(res_att), "\t", utils::signed_percent(res_att))
				 << markup::span_color(unit_helper::resistance_color(res_def), "\t", utils::signed_percent(res_def));
			att_def_diff = true;
		}

		resistances_table.push_back(line.str());
	}

	tooltip << markup::tag("big", _("Resistances: "));
	if(att_def_diff) {
		tooltip << _("(Att / Def)");
	}

	for(const std::string &line : resistances_table) {
		tooltip << '\n' << font::unicode_bullet << " " << line;
	}

	return tooltip.str();
}

static inline std::string get_mp_tooltip(int total_movement, const std::function<int (t_translation::terrain_code)>& get)
{
	std::set<terrain_movement> terrain_moves;
	std::ostringstream tooltip;
	tooltip << markup::tag("big", _("Movement Costs:"));

	std::shared_ptr<terrain_type_data> tdata = help::load_terrain_types_data();

	if(!tdata) {
		return "";
	}

	for(t_translation::terrain_code terrain : prefs::get().encountered_terrains()) {
		if(terrain == t_translation::FOGGED || terrain == t_translation::VOID_TERRAIN || t_translation::terrain_matches(terrain, t_translation::ALL_OFF_MAP)) {
			continue;
		}

		const terrain_type& info = tdata->get_terrain_info(terrain);
		if(info.is_indivisible() && info.is_nonnull()) {
			terrain_moves.emplace(info.name(), get(terrain));
		}
	}

	for(const terrain_movement& tm: terrain_moves)
	{
		tooltip << '\n' << font::unicode_bullet << " " << tm.name << ": ";

		// movement  -  range: 1 .. 5, movetype::UNREACHABLE=impassable
		const bool cannot_move = tm.moves > total_movement;     // cannot move in this terrain
		double movement_red_to_green = 100.0 - 25.0 * tm.moves;

		std::stringstream move_ss;
		// A 5 MP margin; if the movement costs go above the unit's max moves + 5, we replace it with dashes.
		if(cannot_move && (tm.moves > total_movement + 5)) {
			move_ss << font::unicode_figure_dash;
		} else if (cannot_move) {
			move_ss << "(" << tm.moves << ")";
		} else {
			move_ss << tm.moves;
		}
		if(tm.moves != 0) {
			const int movement_hexes_per_turn = total_movement / tm.moves;
			tooltip << " ";
			for(int i = 0; i < movement_hexes_per_turn; ++i) {
				// Unicode horizontal black hexagon and Unicode zero width space (to allow a line break)
				move_ss << "\u2b23\u200b";
			}
		}

		// passing true to select the less saturated red-to-green scale
		tooltip << markup::span_color(game_config::red_to_green(movement_red_to_green, true), move_ss.str());
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


	auto& header_node = add_name_tree_node(parent_node, "header", markup::bold(_("Attacks")));

	for(const auto& a : attacks) {
		const std::string range_png = std::string("icons/profiles/") + a.range() + "_attack.png~SCALE_INTO(16,16)";
		const std::string type_png = std::string("icons/profiles/") + a.type() + ".png~SCALE_INTO(16,16)";
		const bool range_png_exists = ::image::exists(range_png);
		const bool type_png_exists = ::image::exists(type_png);

		const t_string& range = string_table["range_" + a.range()];
		const t_string& type = string_table["type_" + a.type()];

		const std::string label = markup::span_color(
			font::unit_type_color, a.damage(), font::weapon_numbers_sep, a.num_attacks(), " ", a.name());

		auto& subsection = header_node.add_child(
			"item_image",
			{
				{ "image_range", { { "label", range_png } } },
				{ "image_type", { { "label", type_png } } },
				{ "name", { { "label", label }, { "use_markup", "true" } } },
			}
		);

		subsection.find_widget<styled_widget>("image_range", true).set_tooltip(range);
		subsection.find_widget<styled_widget>("image_type", true).set_tooltip(type);

		if(!range_png_exists || !type_png_exists) {
			add_name_tree_node(
				subsection,
				"item",
				markup::span_color(font::weapon_details_color, range, font::weapon_details_sep, type)
			);
		}

		for(const auto& pair : a.special_tooltips()) {
			add_name_tree_node(
				subsection,
				"item",
				markup::span_color(font::weapon_details_color, pair.first),
				markup::span_size("x-large", pair.first) + "\n" + pair.second
			);
		}
	}
}

void unit_preview_pane::set_display_data(const unit_type& type)
{
	// Sets the current type id for the profile button callback to use
	current_type_ = type;

	if(icon_type_) {
		std::string mods;

		if(resources::controller) {
			mods = "~RC(" + type.flag_rgb() + ">" +
				 team::get_side_color_id(resources::controller->current_side())
				 + ")";
		}

		mods += image_mods_;

		icon_type_->set_label((type.icon().empty() ? type.image() : type.icon()) + mods);
	}

	if(label_name_) {
		label_name_->set_label(markup::bold(type.type_name()));
		label_name_->set_use_markup(true);
	}

	if(label_level_) {
		std::string l_str = VGETTEXT("Lvl $lvl", {{"lvl", std::to_string(type.level())}});

		label_level_->set_label(markup::bold(l_str));
		label_level_->set_tooltip(unit_helper::unit_level_tooltip(type));
		label_level_->set_use_markup(true);
	}

	if(label_race_) {
		label_race_ ->set_label(type.race()->name(type.genders().front()));
	}

	if(icon_race_) {
		icon_race_->set_label(type.race()->get_icon_path_stem() + "_30.png");
	}

	if(icon_alignment_) {
		const std::string& alignment_name = unit_alignments::get_string(type.alignment());

		icon_alignment_->set_label("icons/alignments/alignment_" + alignment_name + "_30.png");
		icon_alignment_->set_tooltip(unit_type::alignment_description(
			type.alignment(),
			type.genders().front()));
	}

	if(label_details_) {
		std::stringstream str;

		str << " \n";

		str << markup::span_color(font::unit_type_color, type.type_name()) << "\n";

		std::string l_str = VGETTEXT("Lvl $lvl", {{"lvl", std::to_string(type.level())}});
		str << l_str << "\n";

		str << unit_alignments::get_string(type.alignment()) << "\n";

		str << "\n"; // Leave a blank line where traits would be

		str <<  _("HP: ") << type.hitpoints() << "\n";

		str << _("XP: ") << type.experience_needed(true);

		label_details_->set_label(str.str());
		label_details_->set_use_markup(true);
	}

	if(tree_details_) {

		tree_details_->clear();
		tree_details_->add_node("hp_xp_mp", {
			{ "hp",{
				{ "label", markup::tag("small", markup::span_color(unit::hp_color_max(), markup::bold(_("HP: ")), type.hitpoints()), " | ") },
				{ "use_markup", "true" },
				{ "tooltip", get_hp_tooltip(type.movement_type().get_resistances().damage_table(), [&type](const std::string& dt, bool is_attacker) { return type.resistance_against(dt, is_attacker); }) }
			} },
			{ "xp",{
				{ "label",  markup::tag("small", markup::span_color(unit::xp_color(100, type.can_advance(), true), markup::bold(_("XP: ")), type.experience_needed()), " | ") },
				{ "use_markup", "true" },
				{ "tooltip", (formatter() << _("Experience Modifier: ") << unit_experience_accelerator::get_acceleration() << '%').str() }
			} },
			{ "mp",{
				{ "label", markup::tag("small", markup::bold(_("MP: ")) + std::to_string(type.movement())) },
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
					header_node = &add_name_tree_node(tree_details_->get_root_node(), "header", markup::bold(_("Traits")));
				}

				add_name_tree_node(*header_node, "item", name);
			}
		}

		// Print ability details
		if(!type.abilities_metadata().empty()) {

			auto& header_node = add_name_tree_node(tree_details_->get_root_node(), "header", markup::bold(_("Abilities")));

			for(const auto& ab : type.abilities_metadata()) {
				if(!ab.name.empty()) {
					add_name_tree_node(
						header_node,
						"item",
						ab.name,
						markup::span_size("x-large", ab.name) + "\n" + ab.description
					);
				}
			}
		}

		print_attack_details(type.attacks(), tree_details_->get_root_node());
	}
}

void unit_preview_pane::set_display_data(const unit& u)
{
	// Sets the current type id for the profile button callback to use
	current_type_ = u.type();

	if(icon_type_) {
		std::string mods = u.image_mods();

		if(u.can_recruit()) {
			mods += "~BLIT(" + unit::leader_crown() + ")";
		}

		for(const std::string& overlay : u.overlays()) {
			mods += "~BLIT(" + overlay + ")";
		}

		mods += image_mods_;

		icon_type_->set_label(u.absolute_image() + mods);
	}

	if(label_name_) {
		std::string name;
		if(!u.name().empty()) {
			name = markup::span_size("large", u.name() + "\n") + markup::tag("small", markup::span_color(font::unit_type_color, u.type_name()));
		} else {
			name = markup::span_size("large", u.type_name()) + "\n";
		}

		label_name_->set_label(name);
		label_name_->set_use_markup(true);
	}

	if(label_level_) {
		std::string l_str = VGETTEXT("Lvl $lvl", {{"lvl", std::to_string(u.level())}});

		label_level_->set_label(markup::bold(l_str));
		label_level_->set_tooltip(unit_helper::unit_level_tooltip(u));
		label_level_->set_use_markup(true);
	}

	if(label_race_) {
		label_race_->set_label(u.race()->name(u.gender()));
	}

	if(icon_race_) {
		icon_race_->set_label(u.race()->get_icon_path_stem() + "_30.png");
	}

	if(icon_alignment_) {
		const std::string& alignment_name = unit_alignments::get_string(u.alignment());

		icon_alignment_->set_label("icons/alignments/alignment_" + alignment_name + "_30.png");
		icon_alignment_->set_tooltip(unit_type::alignment_description(
			u.alignment(),
			u.gender()));
	}

	if(label_details_) {
		std::stringstream str;

		const std::string name = markup::span_size("large", (!u.name().empty() ? u.name() : " "));
		str << name << "\n";

		str << markup::span_color(font::unit_type_color, u.type_name()) << "\n";

		std::string l_str = VGETTEXT("Lvl $lvl", {{"lvl", std::to_string(u.level())}});
		str << l_str << "\n";

		str << unit_type::alignment_description(u.alignment(), u.gender()) << "\n";

		str << utils::join(u.trait_names(), ", ") << "\n";

		str << markup::span_color(u.hp_color(), _("HP: "), u.hitpoints(), "/", u.max_hitpoints(), "\n");

		if(u.can_advance()) {
			str << markup::span_color(u.xp_color(), _("XP: "), u.experience(), "/", u.max_experience());
		} else {
			str << markup::span_color(u.xp_color(), _("XP: "), font::unicode_en_dash);
		}

		label_details_->set_label(str.str());
		label_details_->set_use_markup(true);
	}

	if(tree_details_) {
		tree_details_->clear();
		const std::string unit_xp = u.can_advance() ? (formatter() << u.experience() << "/" << u.max_experience()).str() : font::unicode_en_dash;
		tree_details_->add_node("hp_xp_mp", {
			{ "hp",{
				{ "label", markup::tag("small", markup::span_color(u.hp_color(), markup::bold(_("HP: ")), u.hitpoints(), "/", u.max_hitpoints(), " | ")) },
				{ "use_markup", "true" },
				{ "tooltip", get_hp_tooltip(u.get_base_resistances(), [&u](const std::string& dt, bool is_attacker) { return u.resistance_against(dt, is_attacker, u.get_location()); }) }
			} },
			{ "xp",{
				{ "label",  markup::tag("small", markup::span_color(u.xp_color(), markup::bold(_("XP: ")), unit_xp, " | ")) },
				{ "use_markup", "true" },
				{ "tooltip", (formatter() << _("Experience Modifier: ") << unit_experience_accelerator::get_acceleration() << '%').str() }
			} },
			{ "mp",{
				{ "label", markup::tag("small", markup::bold(_("MP: ")), u.movement_left(), "/", u.total_movement()) },
				{ "use_markup", "true" },
				{ "tooltip", get_mp_tooltip(u.total_movement(), [&u](t_translation::terrain_code terrain) { return u.movement_cost(terrain); }) }
			} },
		});

		if(!u.trait_names().empty()) {
			auto& header_node = add_name_tree_node(tree_details_->get_root_node(), "header", markup::bold(_("Traits")));

			assert(u.trait_names().size() == u.trait_descriptions().size());
			for (std::size_t i = 0; i < u.trait_names().size(); ++i) {
				add_name_tree_node(
					header_node,
					"item",
					u.trait_names()[i],
					u.trait_descriptions()[i]
				);
			}
		}

		if(!u.get_ability_list().empty()) {
			auto& header_node = add_name_tree_node(tree_details_->get_root_node(), "header", markup::bold(_("Abilities")));

			for(const auto& ab : u.ability_tooltips()) {
				add_name_tree_node(
					header_node,
					"item",
					std::get<2>(ab),
					std::get<3>(ab)
				);
			}
		}
		print_attack_details(u.attacks(), tree_details_->get_root_node());
	}
}

void unit_preview_pane::profile_button_callback()
{
	if(get_window() && current_type_) {
		help::show_unit_description(*current_type_);
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

void unit_preview_pane::set_self_active(const bool /*active*/)
{
	/* DO NOTHING */
}

// }---------- DEFINITION ---------{

unit_preview_pane_definition::unit_preview_pane_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing unit preview pane " << id;

	load_resolutions<resolution>(cfg);
}

unit_preview_pane_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg), grid()
{
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "background", missing_mandatory_wml_tag("unit_preview_pane_definition][resolution", "background")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "foreground", missing_mandatory_wml_tag("unit_preview_pane_definition][resolution", "foreground")));

	auto child = VALIDATE_WML_CHILD(cfg, "grid", missing_mandatory_wml_tag("unit_preview_pane_definition][resolution", "grid"));
	grid = std::make_shared<builder_grid>(child);
}

// }---------- BUILDER -----------{

namespace implementation
{

builder_unit_preview_pane::builder_unit_preview_pane(const config& cfg)
	: builder_styled_widget(cfg)
	, image_mods(cfg["image_mods"])
{
}

std::unique_ptr<widget> builder_unit_preview_pane::build() const
{
	auto widget = std::make_unique<unit_preview_pane>(*this);

	DBG_GUI_G << "Window builder: placed unit preview pane '" << id
			  << "' with definition '" << definition << "'.";

	const auto conf = widget->cast_config_to<unit_preview_pane_definition>();
	assert(conf);

	widget->init_grid(*conf->grid);
	widget->finalize_setup();

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
