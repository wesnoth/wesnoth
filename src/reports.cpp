/*
	Copyright (C) 2003 - 2024
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "actions/attack.hpp"
#include "attack_prediction.hpp"
#include "desktop/battery_info.hpp"
#include "font/pango/escape.hpp"
#include "font/text_formatting.hpp"
#include "formatter.hpp"
#include "formula/string_utils.hpp"
#include "preferences/preferences.hpp"
#include "gettext.hpp"
#include "language.hpp"
#include "map/map.hpp"
#include "mouse_events.hpp"
#include "pathfind/pathfind.hpp"
#include "picture.hpp"
#include "reports.hpp"
#include "color.hpp"
#include "team.hpp"
#include "terrain/movement.hpp"
#include "tod_manager.hpp"
#include "units/unit.hpp"
#include "units/helper.hpp"
#include "units/types.hpp"
#include "units/unit_alignments.hpp"
#include "whiteboard/manager.hpp"

#include <ctime>
#include <iomanip>
#include <boost/format.hpp>

static void add_text(config &report, const std::string &text,
	const std::string &tooltip, const std::string &help = "")
{
	config &element = report.add_child("element");
	element["text"] = text;
	if (!tooltip.empty()) element["tooltip"] = tooltip;
	if (!help.empty()) element["help"] = help;
}

static void add_image(config &report, const std::string &image,
	const std::string &tooltip, const std::string &help = "")
{
	config &element = report.add_child("element");
	element["image"] = image;
	if (!tooltip.empty()) element["tooltip"] = tooltip;
	if (!help.empty()) element["help"] = help;
}

static config text_report(const std::string &text,
	const std::string &tooltip = "", const std::string &help = "")
{
	config r;
	add_text(r, text, tooltip, help);
	return r;
}

static config image_report(const std::string &image,
	const std::string &tooltip = "", const std::string &help = "")
{
	config r;
	add_image(r, image, tooltip, help);
	return r;
}

using font::span_color;

static void add_status(config &r,
	const std::string& path, char const *desc1, char const *desc2)
{
	std::ostringstream s;
	s << translation::gettext(desc1) << translation::gettext(desc2);
	add_image(r, path, s.str());
}

static std::string flush(std::ostringstream &s)
{
	std::string r(s.str());
	s.str(std::string());
	return r;
}

static const time_of_day get_visible_time_of_day_at(const reports::context& rc, const map_location & hex)
{
	const team &viewing_team = rc.teams()[rc.screen().viewing_team()];
	if (viewing_team.shrouded(hex)) {
		// Don't show time on shrouded tiles.
		return rc.tod().get_time_of_day();
	} else if (viewing_team.fogged(hex)) {
		// Don't show illuminated time on fogged tiles.
		return rc.tod().get_time_of_day(hex);
	} else {
		return rc.tod().get_illuminated_time_of_day(rc.units(), rc.map(), hex);
	}
}

typedef std::map<std::string, reports::generator_function> static_report_generators;
static static_report_generators static_generators;

struct report_generator_helper
{
	report_generator_helper(const char *name, reports::generator_function g)
	{
		static_generators.insert(static_report_generators::value_type(name, g));
	}
};

#define REPORT_GENERATOR(n, cn) \
	static config report_##n(const reports::context& cn); \
	static report_generator_helper reg_gen_##n(#n, &report_##n); \
	static config report_##n(const reports::context& cn)

static char const *naps = "</span>";

static const unit *get_visible_unit(const reports::context& rc)
{
	return rc.dc().get_visible_unit(rc.screen().displayed_unit_hex(),
		rc.teams()[rc.screen().viewing_team()],
		rc.screen().show_everything());
}

static const unit *get_selected_unit(const reports::context& rc)
{
	return rc.dc().get_visible_unit(rc.screen().selected_hex(),
		rc.teams()[rc.screen().viewing_team()],
		rc.screen().show_everything());
}

static unit_const_ptr get_selected_unit_ptr(const reports::context& rc)
{
	return rc.dc().get_visible_unit_shared_ptr(rc.screen().selected_hex(),
		rc.teams()[rc.screen().viewing_team()],
		rc.screen().show_everything());
}

static config gray_inactive(const reports::context& rc, const std::string &str, const std::string& tooltip = "")
{
	if ( rc.screen().viewing_side() == rc.screen().playing_side() )
			return text_report(str, tooltip);

	return text_report(span_color(font::GRAY_COLOR) + str + naps, tooltip);
}

static config unit_name(const unit *u)
{
	if (!u) {
		return config();
	}

	/*
	 * The name needs to be escaped, it might be set by the user and using
	 * markup. Also names often contain a forbidden single quote.
	 */
	const std::string& name = font::escape_text(u->name());
	std::ostringstream str, tooltip;
	str << "<b>" << name << "</b>";
	tooltip << _("Name: ") << "<b>" << name << "</b>";
	return text_report(str.str(), tooltip.str());
}

REPORT_GENERATOR(unit_name, rc)
{
	const unit *u = get_visible_unit(rc);
	return unit_name(u);
}
REPORT_GENERATOR(selected_unit_name, rc)
{
	const unit *u = get_selected_unit(rc);
	return unit_name(u);
}

static config unit_type(const unit* u)
{
	if (!u) return config();
	std::string has_variations_prefix = (u->type().show_variations_in_help() ? ".." : "");
	std::ostringstream str, tooltip;
	str << u->type_name();
	tooltip << _("Type: ") << "<b>" << u->type_name() << "</b>\n"
		<< u->unit_description();
	if(const auto& notes = u->unit_special_notes(); !notes.empty()) {
		tooltip << "\n\n" << _("Special Notes:") << '\n';
		for(const auto& note : notes) {
			tooltip << font::unicode_bullet << " " << note << '\n';
		}
	}
	return text_report(str.str(), tooltip.str(), has_variations_prefix + "unit_" + u->type_id());
}
REPORT_GENERATOR(unit_type, rc)
{
	const unit *u = get_visible_unit(rc);
	return unit_type(u);
}
REPORT_GENERATOR(selected_unit_type, rc)
{
	const unit *u = get_selected_unit(rc);
	return unit_type(u);
}

static config unit_race(const unit* u)
{
	if (!u) return config();
	std::ostringstream str, tooltip;
	str << u->race()->name(u->gender());
	tooltip << _("Race: ") << "<b>" << u->race()->name(u->gender()) << "</b>";
	return text_report(str.str(), tooltip.str(), "..race_" + u->race()->id());
}
REPORT_GENERATOR(unit_race, rc)
{
	const unit *u = get_visible_unit(rc);
	return unit_race(u);
}
REPORT_GENERATOR(selected_unit_race, rc)
{
	const unit *u = get_selected_unit(rc);
	return unit_race(u);
}

static std::string side_tooltip(const team& team)
{
	if(team.side_name().empty())
		return "";

	return VGETTEXT("Side: <b>$side_name</b> ($color_name)",
			{{"side_name", team.side_name()},
			 {"color_name", team::get_side_color_name_for_UI(team.side()) }});
}

static config unit_side(const reports::context& rc, const unit* u)
{
	if (!u) return config();

	config report;
	const team &u_team = rc.dc().get_team(u->side());
	std::string flag_icon = u_team.flag_icon();
	std::string old_rgb = game_config::flag_rgb;
	std::string new_rgb = u_team.color();
	std::string mods = "~RC(" + old_rgb + ">" + new_rgb + ")";
	if (flag_icon.empty())
		flag_icon = game_config::images::flag_icon;

	std::stringstream text;
	text << " " << u->side();

	const std::string& tooltip = side_tooltip(u_team);
	add_image(report, flag_icon + mods, tooltip, "");
	add_text(report, text.str(), tooltip, "");
	return report;
}
REPORT_GENERATOR(unit_side, rc)
{
	const unit *u = get_visible_unit(rc);
	return unit_side(rc,u);
}
REPORT_GENERATOR(selected_unit_side, rc)
{
	const unit *u = get_selected_unit(rc);
	return unit_side(rc, u);
}

static config unit_level(const unit* u)
{
	if (!u) return config();
	return text_report(std::to_string(u->level()), unit_helper::unit_level_tooltip(*u));
}
REPORT_GENERATOR(unit_level, rc)
{
	const unit *u = get_visible_unit(rc);
	return unit_level(u);
}
REPORT_GENERATOR(selected_unit_level, rc)
{
	const unit *u = get_selected_unit(rc);
	return unit_level(u);
}

REPORT_GENERATOR(unit_amla, rc)
{
	const unit *u = get_visible_unit(rc);
	if (!u) return config();
	config res;
	typedef std::pair<std::string, std::string> pair_string;
	for (const pair_string &ps : u->amla_icons()) {
		add_image(res, ps.first, ps.second);
	}
	return res;
}

static config unit_traits(const unit* u)
{
	if (!u) return config();
	config res;
	const std::vector<t_string> &traits = u->trait_names();
	const std::vector<t_string> &descriptions = u->trait_descriptions();
	const std::vector<std::string> &trait_ids = u->trait_nonhidden_ids();
	unsigned nb = traits.size();
	for (unsigned i = 0; i < nb; ++i)
	{
		std::ostringstream str, tooltip;
		str << traits[i];
		if (i != nb - 1 ) str << ", ";
		tooltip << _("Trait: ") << "<b>" << traits[i] << "</b>\n"
			<< descriptions[i];
		add_text(res, str.str(), tooltip.str(), "traits_" + trait_ids[i]);
	}
	return res;
}
REPORT_GENERATOR(unit_traits, rc)
{
	const unit *u = get_visible_unit(rc);
	return unit_traits(u);
}
REPORT_GENERATOR(selected_unit_traits, rc)
{
	const unit *u = get_selected_unit(rc);
	return unit_traits(u);
}

static config unit_status(const reports::context& rc, const unit* u)
{
	if (!u) return config();
	config res;
	map_location displayed_unit_hex = rc.screen().displayed_unit_hex();
	if (rc.map().on_board(displayed_unit_hex) && u->invisible(displayed_unit_hex)) {
		add_status(res, "misc/invisible.png", N_("invisible: "),
			N_("This unit is invisible. It cannot be seen or attacked by enemy units."));
	}
	if (u->get_state(unit::STATE_SLOWED)) {
		add_status(res, "misc/slowed.png", N_("slowed: "),
			N_("This unit has been slowed. It will only deal half its normal damage when attacking and its movement cost is doubled."));
	}
	if (u->get_state(unit::STATE_POISONED)) {
		add_status(res, "misc/poisoned.png", N_("poisoned: "),
			N_("This unit is poisoned. It will lose 8 HP every turn until it can seek a cure to the poison in a village or from a friendly unit with the ‘cures’ ability.\n\nUnits cannot be killed by poison alone. The poison will not reduce it below 1 HP."));
	}
	if (u->get_state(unit::STATE_PETRIFIED)) {
		add_status(res, "misc/petrified.png", N_("petrified: "),
			N_("This unit has been petrified. It may not move or attack."));
	}
	if (u->get_state(unit::STATE_UNHEALABLE)) {
		add_status(res, "misc/unhealable.png", N_("unhealable: "),
			N_("This unit is unhealable. It cannot be healed by healers or villages and doesn’t benefit from resting."));
	}
	if (u->get_state(unit::STATE_INVULNERABLE)) {
		add_status(res, "misc/invulnerable.png", N_("invulnerable: "),
			N_("This unit is invulnerable. It cannot be harmed by any attack."));
	}
	return res;
}
REPORT_GENERATOR(unit_status,rc)
{
	const unit *u = get_visible_unit(rc);
	return unit_status(rc,u);
}
REPORT_GENERATOR(selected_unit_status, rc)
{
	const unit *u = get_selected_unit(rc);
	return unit_status(rc, u);
}

static config unit_alignment(const reports::context& rc, const unit* u, const map_location& hex)
{
	if (!u) return config();
	std::ostringstream str, tooltip;
	const std::string align = unit_type::alignment_description(u->alignment(), u->gender());
	const std::string align_id = unit_alignments::get_string(u->alignment());
	const time_of_day effective_tod = get_visible_time_of_day_at(rc, hex);
	int cm = combat_modifier(effective_tod, u->alignment(), u->is_fearless());

	color_t color = font::weapon_color;
	if (cm != 0)
		color = (cm > 0) ? font::good_dmg_color : font::bad_dmg_color;

	str << align << " (" << span_color(color) << utils::signed_percent(cm)
		<< naps << ")";

	tooltip << _("Alignment: ") << "<b>" << align << "</b>\n"
		<< string_table[align_id + "_description"];

	return text_report(str.str(), tooltip.str(), "time_of_day");
}
REPORT_GENERATOR(unit_alignment, rc)
{
	const unit *u = get_visible_unit(rc);
	const map_location& mouseover_hex = rc.screen().mouseover_hex();
	const map_location& displayed_unit_hex = rc.screen().displayed_unit_hex();
	const map_location& hex = mouseover_hex.valid() ? mouseover_hex : displayed_unit_hex;
	return unit_alignment(rc, u, hex);
}
REPORT_GENERATOR(selected_unit_alignment, rc)
{
	const unit *u = get_selected_unit(rc);
	const map_location& attack_indicator_src = game_display::get_singleton()->get_attack_indicator_src();
	const map_location& hex_to_show_alignment_at =
		attack_indicator_src.valid() ? attack_indicator_src : u->get_location();
	return unit_alignment(rc, u, hex_to_show_alignment_at);
}

static config unit_abilities(const unit* u, const map_location& loc)
{
	if (!u) return config();
	config res;

	boost::dynamic_bitset<> active;
	const auto abilities = u->ability_tooltips(active, loc);

	const std::size_t abilities_size = abilities.size();
	for(std::size_t i = 0; i != abilities_size; ++i) {
		// Aliases for readability:
		const auto& [id, base_name, display_name, description] = abilities[i];

		std::ostringstream str, tooltip;

		if(active[i]) {
			str << display_name;
		} else {
			str << span_color(font::inactive_ability_color) << display_name << naps;
		}

		if(i + 1 != abilities_size) {
			str << ", ";
		}

		tooltip << _("Ability: ") << "<b>" << display_name << "</b>";
		if(!active[i]) {
			tooltip << "<i>" << _(" (inactive)") << "</i>";
		}

		tooltip << '\n' << description;

		add_text(res, str.str(), tooltip.str(), "ability_" + id + base_name.base_str());
	}

	return res;
}
REPORT_GENERATOR(unit_abilities, rc)
{
	const unit *u = get_visible_unit(rc);
	const team &viewing_team = rc.teams()[rc.screen().viewing_team()];
	const map_location& mouseover_hex = rc.screen().mouseover_hex();
	const map_location& displayed_unit_hex = rc.screen().displayed_unit_hex();
	const map_location& hex = (mouseover_hex.valid() && !viewing_team.shrouded(mouseover_hex)) ? mouseover_hex : displayed_unit_hex;

	return unit_abilities(u, hex);
}
REPORT_GENERATOR(selected_unit_abilities, rc)
{
	const unit *u = get_selected_unit(rc);

	const map_location& mouseover_hex = rc.screen().mouseover_hex();
	const unit *visible_unit = get_visible_unit(rc);
	const team &viewing_team = rc.teams()[rc.screen().viewing_team()];

	if (visible_unit && u && visible_unit->id() != u->id() && mouseover_hex.valid() && !viewing_team.shrouded(mouseover_hex))
		return unit_abilities(u, mouseover_hex);
	else
		return unit_abilities(u, u->get_location());
}


static config unit_hp(const reports::context& rc, const unit* u)
{
	if (!u) return config();
	std::ostringstream str, tooltip;
	str << span_color(u->hp_color()) << u->hitpoints()
		<< '/' << u->max_hitpoints() << naps;

	std::vector<std::string> resistances_table;

	bool att_def_diff = false;
	map_location displayed_unit_hex = rc.screen().displayed_unit_hex();
	for (const utils::string_map_res::value_type &resist : u->get_base_resistances())
	{
		std::ostringstream line;
		line << translation::gettext(resist.first.c_str()) << ": ";
		// Some units have different resistances when attacking or defending.
		int res_att = 100 - u->resistance_against(resist.first, true, displayed_unit_hex);
		int res_def = 100 - u->resistance_against(resist.first, false, displayed_unit_hex);
		const std::string def_color = unit_helper::resistance_color(res_def);
		if (res_att == res_def) {
			line << "<span foreground=\"" << def_color << "\">" << utils::signed_percent(res_def)
			<< naps << '\n';
		} else {
			const std::string att_color = unit_helper::resistance_color(res_att);
			line << "<span foreground=\"" << att_color << "\">" << utils::signed_percent(res_att)
			<< naps << "/"
			<< "<span foreground=\"" << def_color << "\">" << utils::signed_percent(res_def)
			<< naps << '\n';
			att_def_diff = true;
		}
		resistances_table.push_back(line.str());
	}

	tooltip << _("Resistances: ");
	if (att_def_diff)
		tooltip << _("(Att / Def)");
	tooltip << '\n';
	for (const std::string &line : resistances_table) {
		tooltip << line;
	}
	return text_report(str.str(), tooltip.str());
}
REPORT_GENERATOR(unit_hp, rc)
{
	const unit *u = get_visible_unit(rc);
	return unit_hp(rc, u);
}
REPORT_GENERATOR(selected_unit_hp, rc)
{
	const unit *u = get_selected_unit(rc);
	return unit_hp(rc, u);
}

static config unit_xp(const unit* u)
{
	if (!u) return config();
	std::ostringstream str, tooltip;
	str << span_color(u->xp_color());
	if(u->can_advance()) {
		str << u->experience() << '/' << u->max_experience();
	} else {
		str << font::unicode_en_dash;
	}
	str << naps;

	int exp_mod = unit_experience_accelerator::get_acceleration();
	tooltip << _("Experience Modifier: ") << exp_mod << '%';
	return text_report(str.str(), tooltip.str());
}
REPORT_GENERATOR(unit_xp, rc)
{
	const unit *u = get_visible_unit(rc);
	return unit_xp(u);
}
REPORT_GENERATOR(selected_unit_xp, rc)
{
	const unit *u = get_selected_unit(rc);
	return unit_xp(u);
}

static config unit_advancement_options(const unit* u)
{
	if (!u) return config();
	config res;
	for (const auto& ps : u->advancement_icons()) {
		add_image(res, ps.first, ps.second);
	}
	return res;
}
REPORT_GENERATOR(unit_advancement_options, rc)
{
	const unit *u = get_visible_unit(rc);
	return unit_advancement_options(u);
}
REPORT_GENERATOR(selected_unit_advancement_options, rc)
{
	const unit *u = get_selected_unit(rc);
	return unit_advancement_options(u);
}

static config unit_defense(const reports::context& rc, const unit* u, const map_location& displayed_unit_hex)
{
	if(!u) {
		return config();
	}

	std::ostringstream str, tooltip;
	const gamemap &map = rc.map();
	if(!rc.map().on_board(displayed_unit_hex)) {
		return config();
	}

	const t_translation::terrain_code &terrain = map[displayed_unit_hex];
	int def = 100 - u->defense_modifier(terrain);
	color_t color = game_config::red_to_green(def);
	str << span_color(color) << def << '%' << naps;
	tooltip << _("Terrain: ") << "<b>" << map.get_terrain_info(terrain).description() << "</b>\n";

	const t_translation::ter_list &underlyings = map.underlying_def_terrain(terrain);
	if (underlyings.size() != 1 || underlyings.front() != terrain)
	{
		bool revert = false;
		for (const t_translation::terrain_code &t : underlyings)
		{
			if (t == t_translation::MINUS) {
				revert = true;
			} else if (t == t_translation::PLUS) {
				revert = false;
			} else {
				int t_def = 100 - u->defense_modifier(t);
				color_t t_color = game_config::red_to_green(t_def);
				tooltip << '\t' << map.get_terrain_info(t).description() << ": "
					<< span_color(t_color) << t_def << '%' << naps
					<< (revert ? _("maximum^max.") : _("minimum^min.")) << '\n';
			}
		}
	}

	tooltip << "<b>" << _("Defense: ") << span_color(color)  << def << '%' << naps << "</b>";
	const std::string has_variations_prefix = (u->type().show_variations_in_help() ? ".." : "");
	return text_report(str.str(), tooltip.str(), has_variations_prefix + "unit_" + u->type_id());
}
REPORT_GENERATOR(unit_defense,rc)
{
	const unit *u = get_visible_unit(rc);
	const team &viewing_team = rc.teams()[rc.screen().viewing_team()];
	const map_location& mouseover_hex = rc.screen().mouseover_hex();
	const map_location& displayed_unit_hex = rc.screen().displayed_unit_hex();
	const map_location& hex = (mouseover_hex.valid() && !viewing_team.shrouded(mouseover_hex)) ? mouseover_hex : displayed_unit_hex;
	return unit_defense(rc, u, hex);
}
REPORT_GENERATOR(selected_unit_defense, rc)
{
	const unit *u = get_selected_unit(rc);
	const map_location& attack_indicator_src = game_display::get_singleton()->get_attack_indicator_src();
	if(attack_indicator_src.valid())
		return unit_defense(rc, u, attack_indicator_src);

	const map_location& mouseover_hex = rc.screen().mouseover_hex();
	const unit *visible_unit = get_visible_unit(rc);
	if(visible_unit && u && visible_unit->id() != u->id() && mouseover_hex.valid())
		return unit_defense(rc, u, mouseover_hex);
	else
		return unit_defense(rc, u, u->get_location());
}

static config unit_vision(const unit* u)
{
	if (!u) return config();

	// TODO
	std::ostringstream str, tooltip;
	if (u->vision() != u->total_movement()) {
		str << _("vision:") << ' ' << u->vision();
		tooltip << _("vision:") << ' ' << u->vision() << '\n';
	}
	if (u->jamming() != 0) {
		if (static_cast<std::streamoff>(str.tellp()) == 0)
			str << _("jamming:") << ' ' << u->jamming();
		tooltip << _("jamming:") << ' ' << u->jamming() << '\n';
	}
	return text_report(str.str(), tooltip.str());
}
REPORT_GENERATOR(unit_vision, rc)
{
	const unit* u = get_visible_unit(rc);
	return unit_vision(u);
}
REPORT_GENERATOR(selected_unit_vision, rc)
{
	const unit* u = get_selected_unit(rc);
	return unit_vision(u);
}

static config unit_moves(const reports::context& rc, const unit* u, bool is_visible_unit)
{
	if (!u) return config();
	std::ostringstream str, tooltip;
	double movement_frac = 1.0;

	std::set<terrain_movement> terrain_moves;

	if (u->side() == rc.screen().playing_side()) {
		movement_frac = static_cast<double>(u->movement_left()) / std::max<int>(1, u->total_movement());
		if (movement_frac > 1.0)
			movement_frac = 1.0;
	}

	tooltip << _("Movement Costs:") << "\n";
	for (t_translation::terrain_code terrain : prefs::get().encountered_terrains()) {
		if (terrain == t_translation::FOGGED || terrain == t_translation::VOID_TERRAIN || t_translation::terrain_matches(terrain, t_translation::ALL_OFF_MAP))
			continue;

		const terrain_type& info = rc.map().get_terrain_info(terrain);

		if (info.union_type().size() == 1 && info.union_type()[0] == info.number() && info.is_nonnull()) {
			terrain_moves.emplace(info.name(), u->movement_cost(terrain));
		}
	}

	for (const terrain_movement& tm : terrain_moves) {
		tooltip << tm.name << ": ";

		//movement  -  range: 1 .. 5, movetype::UNREACHABLE=impassable
		const bool cannot_move = tm.moves > u->total_movement();		// cannot move in this terrain
		double movement_red_to_green = 100.0 - 25.0 * tm.moves;

		// passing true to select the less saturated red-to-green scale
		std::string color = game_config::red_to_green(movement_red_to_green, true).to_hex_string();
		tooltip << "<span foreground=\"" << color << "\">";
		// A 5 MP margin; if the movement costs go above
		// the unit's max moves + 5, we replace it with dashes.
		if (cannot_move && (tm.moves > u->total_movement() + 5)) {
			tooltip << font::unicode_figure_dash;
		} else if (cannot_move) {
			tooltip << "(" << tm.moves << ")";
		} else {
			tooltip << tm.moves;
		}
		if(tm.moves != 0) {
			const int movement_hexes_per_turn = u->total_movement() / tm.moves;
			tooltip << " ";
			for(int i = 0; i < movement_hexes_per_turn; ++i) {
				// Unicode horizontal black hexagon and Unicode zero width space (to allow a line break)
				tooltip << "\u2b23\u200b";
			}
		}
		tooltip << naps << '\n';
	}

	int grey = 128 + static_cast<int>((255 - 128) * movement_frac);
	color_t c = color_t(grey, grey, grey);
	int numerator = u->movement_left();
	if(is_visible_unit) {
		const pathfind::marked_route& route = game_display::get_singleton()->get_route();
		if(route.steps.size() > 0) {
			numerator -= route.move_cost;
			if(numerator < 0) {
				// Multi-turn move
				// TODO: what to show in this case?
				numerator = 0;
			}

			// If the route captures a village, assume that uses up all remaining MP.
			const auto& end = route.marks.find(route.steps.back());
			if(end != route.marks.end() && end->second.capture) {
				numerator = 0;
			}
		} else {
			// TODO: if the mouseover hex is unreachable (for example, a deep water hex
			// and the current unit is a land unit), what to show?
		}
	}
	str << span_color(c) << numerator << '/' << u->total_movement() << naps;
	return text_report(str.str(), tooltip.str());
}
REPORT_GENERATOR(unit_moves, rc)
{
	const unit *u = get_visible_unit(rc);
	return unit_moves(rc, u, true);
}
REPORT_GENERATOR(selected_unit_moves, rc)
{
	const unit *u = get_selected_unit(rc);
	return unit_moves(rc, u, false);
}

/**
 * Maps resistance <= -60 (resistance value <= -60%) to intense red.
 * Maps resistance >= 60 (resistance value >= 60%) to intense green.
 * Intermediate values are affinely mapped to the red-to-green scale,
 * with 0 (0%) being mapped to yellow.
 * Compare unit_helper::resistance_color().
 */
static inline const color_t attack_info_percent_color(int resistance)
{
	// Passing false to select the more saturated red-to-green scale.
	return game_config::red_to_green(50.0 + resistance * 5.0 / 6.0, false);
}

static int attack_info(const reports::context& rc, const attack_type &at, config &res, const unit &u, const map_location &hex, const unit* sec_u = nullptr, const_attack_ptr sec_u_weapon = nullptr)
{
	std::ostringstream str, tooltip;
	int damage = 0;

	struct string_with_tooltip {
		std::string str;
		std::string tooltip;
	};

	{
		auto ctx = at.specials_context(u.shared_from_this(), hex, u.side() == rc.screen().playing_side());
		int base_damage = at.damage();
		int specials_damage = at.modified_damage();
		int damage_multiplier = 100;
		const_attack_ptr weapon  = at.shared_from_this();
		int tod_bonus = combat_modifier(get_visible_time_of_day_at(rc, hex), u.alignment(), u.is_fearless());
		damage_multiplier += tod_bonus;
		int leader_bonus = under_leadership(u, hex, weapon);
		if (leader_bonus != 0)
			damage_multiplier += leader_bonus;

		bool slowed = u.get_state(unit::STATE_SLOWED);
		int damage_divisor = slowed ? 20000 : 10000;
		// Assume no specific resistance (i.e. multiply by 100).
		damage = round_damage(specials_damage, damage_multiplier * 100, damage_divisor);

		// Hit points are used to calculate swarm, so they need to be bounded.
		unsigned max_hp = u.max_hitpoints();
		unsigned cur_hp = std::min<unsigned>(std::max(0, u.hitpoints()), max_hp);

		unsigned base_attacks = at.num_attacks();
		unsigned min_attacks, max_attacks;
		at.modified_attacks(min_attacks, max_attacks);
		unsigned num_attacks = swarm_blows(min_attacks, max_attacks, cur_hp, max_hp);

		color_t dmg_color = font::weapon_color;
		if ( damage > specials_damage )
			dmg_color = font::good_dmg_color;
		else if ( damage < specials_damage )
			dmg_color = font::bad_dmg_color;

		str << span_color(dmg_color) << "  " << damage << naps << span_color(font::weapon_color)
			<< font::weapon_numbers_sep << num_attacks << ' ' << at.name()
			<< "</span>\n";
		tooltip << _("Weapon: ") << "<b>" << at.name() << "</b>\n"
			<< _("Damage: ") << "<b>" << damage << "</b>\n";

		if ( tod_bonus || leader_bonus || slowed || specials_damage != base_damage )
		{
			tooltip << '\t' << _("Base damage: ") << base_damage << '\n';
			if ( specials_damage != base_damage ) {
				tooltip << '\t' << _("With specials: ") << specials_damage << '\n';
			}
			if (tod_bonus) {
				tooltip << '\t' << _("Time of day: ")
					<< utils::signed_percent(tod_bonus) << '\n';
			}
			if (leader_bonus) {
				tooltip << '\t' << _("Leadership: ")
					<< utils::signed_percent(leader_bonus) << '\n';
			}
			if (slowed) {
				tooltip << '\t' << _("Slowed: ") << "/ 2" << '\n';
			}
		}

		tooltip << _("Attacks: ") << "<b>" << num_attacks << "</b>\n";
		if ( max_attacks != min_attacks  &&  cur_hp != max_hp ) {
			if ( max_attacks < min_attacks ) {
				// "Reverse swarm"
				tooltip << '\t' << _("Max swarm bonus: ") << (min_attacks-max_attacks) << '\n';
				tooltip << '\t' << _("Swarm: ") << "* "<< (100 - cur_hp*100/max_hp) << "%\n";
				tooltip << '\t' << _("Base attacks: ") << '+' << base_attacks << '\n';
				// The specials line will not necessarily match up with how the
				// specials are calculated, but for an unusual case, simple brevity
				// trumps complexities.
				if ( max_attacks != base_attacks ) {
					int attack_diff = static_cast<int>(max_attacks) - static_cast<int>(base_attacks);
					tooltip << '\t' << _("Specials: ") << utils::signed_value(attack_diff) << '\n';
				}
			}
			else {
				// Regular swarm
				tooltip << '\t' << _("Base attacks: ") << base_attacks << '\n';
				if ( max_attacks != base_attacks ) {
					tooltip << '\t' << _("With specials: ") << max_attacks << '\n';
				}
				if ( min_attacks != 0 ) {
					tooltip << '\t' << _("Subject to swarm: ") << (max_attacks-min_attacks) << '\n';
				}
				tooltip << '\t' << _("Swarm: ") << "* "<< (cur_hp*100/max_hp) << "%\n";
			}
		}
		else if ( num_attacks != base_attacks ) {
			tooltip << '\t' << _("Base attacks: ") << base_attacks << '\n';
			tooltip << '\t' << _("With specials: ") << num_attacks << '\n';
		}

		const string_with_tooltip damage_and_num_attacks {flush(str), flush(tooltip)};

		std::string range = string_table["range_" + at.range()];
		std::string type = at.damage_type().first;
		std::set<std::string> alt_types = at.alternative_damage_types();
		std::string lang_type = string_table["type_" + type];
		for(auto alt_t : alt_types){
			lang_type += ", " + string_table["type_" + alt_t];
		}

		// SCALE_INTO() is needed in case the 72x72 images/misc/missing-image.png is substituted.
		const std::string range_png = std::string("icons/profiles/") + at.range() + "_attack.png~SCALE_INTO(16,16)";
		const std::string type_png = std::string("icons/profiles/") + type + ".png~SCALE_INTO(16,16)";
		std::vector<std::string> secondary_types_png;
		for(const auto& alt_t : alt_types) {
			secondary_types_png.push_back(std::string("icons/profiles/") + alt_t + ".png~SCALE_INTO(16,16)");
		}

		// If any of the images is missing, then add a text description too.
		bool all_pngs_exist = image::exists(range_png);
		all_pngs_exist &= image::exists(type_png);
		for(const auto& png : secondary_types_png) {
			all_pngs_exist &= image::exists(png);
		}
		if(!all_pngs_exist) {
			str << span_color(font::weapon_details_color) << "  " << "  "
				<< range << font::weapon_details_sep
				<< lang_type << "</span>\n";
		}

		tooltip << _("Weapon range: ") << "<b>" << range << "</b>\n"
			<< _("Damage type: ")  << "<b>" << lang_type << "</b>\n"
			<< _("Damage versus: ") << '\n';

		// Show this weapon damage and resistance against all the different units.
		// We want weak resistances (= good damage) first.
		std::map<int, std::set<std::string>, std::greater<int>> resistances;
		std::set<std::string> seen_types;
		const team &unit_team = rc.dc().get_team(u.side());
		const team &viewing_team = rc.teams()[rc.screen().viewing_team()];
		for (const unit &enemy : rc.units())
		{
			if (enemy.incapacitated()) //we can't attack statues so don't display them in this tooltip
				continue;
			if (!unit_team.is_enemy(enemy.side()))
				continue;
			const map_location &loc = enemy.get_location();
			const bool see_all = game_config::debug || rc.screen().show_everything();
			if (!enemy.is_visible_to_team(viewing_team, see_all))
				continue;
			bool new_type = seen_types.insert(enemy.type_id()).second;
			if (new_type) {
				int resistance = enemy.resistance_against(at, false, loc);
				resistances[resistance].insert(enemy.type_name());
			}
		}

		for (const auto& resist : resistances) {
			int damage_with_resistance = round_damage(specials_damage, damage_multiplier * resist.first, damage_divisor);
			tooltip << "<b>" << damage_with_resistance << "</b>  "
				<< "<span color='" << attack_info_percent_color(resist.first-100).to_hex_string() << "'>"
				<< "<i>(" << utils::signed_percent(resist.first-100) << ")</i>"
				<< naps
				<< " :  \t" // spaces to align the tab to a multiple of 8
				<< utils::join(resist.second, " " + font::unicode_bullet + " ") << '\n';
		}
		const string_with_tooltip damage_versus {flush(str), flush(tooltip)};

#if 0
		// We wanted to use the attack icon here, but couldn't find a good layout.
		// The default images are 60x60 and have a 2-pixel transparent border. Trim it.
		// The first SCALE() accounts for theoretically possible add-ons attack images larger than 60x60.
		const std::string attack_icon = at.icon() + "~SCALE_INTO_SHARP(60,60)~CROP(2,2,56,56)~SCALE_INTO_SHARP(32,32)";
		add_image(res, attack_icon, at.name());
		add_text(res, " ", "");
#endif

		// The icons are 16x16. We add 5px padding for alignment reasons (placement of the icon in relation to ascender and descender letters).
		const std::string spacer = "misc/blank.png~CROP(0, 0, 16, 21)"; // 21 == 16+5
		add_image(res, spacer + "~BLIT(" + range_png + ",0,5)", damage_versus.tooltip);
		add_image(res, spacer + "~BLIT(" + type_png + ",0,5)", damage_versus.tooltip);
		for(auto sec_exist : secondary_types_png){
			if(image::exists(sec_exist)){
				add_image(res, spacer + "~BLIT(" + sec_exist + ",0,5)", damage_versus.tooltip);
			}
		}
		add_text(res, damage_and_num_attacks.str, damage_and_num_attacks.tooltip);
		add_text(res, damage_versus.str, damage_versus.tooltip); // This string is usually empty

		const std::string &accuracy_parry = at.accuracy_parry_description();
		if (!accuracy_parry.empty())
		{
			str << span_color(font::weapon_details_color)
				<< "  " << accuracy_parry << "</span>\n";
			int accuracy = at.accuracy();
			if (accuracy) {
				tooltip << _("Accuracy:") << "<b>"
					<< utils::signed_percent(accuracy) << "</b>\n";
			}
			int parry = at.parry();
			if (parry) {
				tooltip << _("Parry:") << "<b>"
					<< utils::signed_percent(parry) << "</b>\n";
				}
			add_text(res, flush(str), flush(tooltip));
		}
	}

	{
		//If we have a second unit, do the 2-unit specials_context
		bool attacking = (u.side() == rc.screen().playing_side());
		auto ctx = (sec_u == nullptr) ? at.specials_context_for_listing(attacking) :
						at.specials_context(u.shared_from_this(), sec_u->shared_from_this(), hex, sec_u->get_location(), attacking, sec_u_weapon);

		boost::dynamic_bitset<> active;
		const std::vector<std::pair<t_string, t_string>> &specials = at.special_tooltips(&active);
		const std::size_t specials_size = specials.size();
		for ( std::size_t i = 0; i != specials_size; ++i )
		{
			// Aliases for readability:
			const t_string &name = specials[i].first;
			const t_string &description = specials[i].second;
			const color_t &details_color = active[i] ? font::weapon_details_color :
														 font::inactive_details_color;

			str << span_color(details_color) << "  " << "  " << name << naps << '\n';
			std::string help_page = "weaponspecial_" + name.base_str();
			tooltip << _("Weapon special: ") << "<b>" << name << "</b>";
			if ( !active[i] )
				tooltip << "<i>" << _(" (inactive)") << "</i>";
			tooltip << '\n' << description;

			add_text(res, flush(str), flush(tooltip), help_page);
		}
		if(!specials.empty()) {
			// Add some padding so the end of the specials list
			// isn't too close vertically to the attack icons of
			// the next attack. Also for symmetry with the padding
			// above the list of specials (below the attack icon line).
			const std::string spacer = "misc/blank.png~CROP(0, 0, 1, 5)";
			add_image(res, spacer, "");
			add_text(res, "\n", "");
		}
	}
	return damage;
}

// Conversion routine for both unscathed and damage change percentage.
static std::string format_prob(double prob)
{
	if(prob > 0.9995) {
		return "100%";
	} else if(prob < 0.0005) {
		return "0%";
	}
	std::ostringstream res;
	res << std::setprecision(prob < 0.01 ? 1 : prob < 0.1 ? 2 : 3) << 100.0 * prob << "%";
	return res.str();
}

static std::string format_hp(unsigned hp)
{
	std::ostringstream res;
	res << ' ' << std::setw(3) << hp;
	return res.str();
}

static config unit_weapons(const reports::context& rc, unit_const_ptr attacker, const map_location &attacker_pos, const unit *defender, bool show_attacker)
{
	if (!attacker || !defender) return config();

	const unit* u = show_attacker ? attacker.get() : defender;
	const unit* sec_u = !show_attacker ? attacker.get() : defender;
	const map_location unit_loc = show_attacker ? attacker_pos : defender->get_location();

	std::ostringstream str, tooltip;
	config res;

	std::vector<battle_context> weapons;
	for (unsigned int i = 0; i < attacker->attacks().size(); i++) {
		// skip weapons with attack_weight=0
		if (attacker->attacks()[i].attack_weight() > 0) {
			weapons.emplace_back(rc.units(), attacker_pos, defender->get_location(), i, -1, 0.0, nullptr, attacker);
		}
	}

	for (const battle_context& weapon : weapons) {

		// Predict the battle outcome.
		combatant attacker_combatant(weapon.get_attacker_stats());
		combatant defender_combatant(weapon.get_defender_stats());
		attacker_combatant.fight(defender_combatant);

		const battle_context_unit_stats& context_unit_stats =
				show_attacker ? weapon.get_attacker_stats() : weapon.get_defender_stats();
		const battle_context_unit_stats& other_context_unit_stats =
				!show_attacker ? weapon.get_attacker_stats() : weapon.get_defender_stats();

		int total_damage = 0;
		int base_damage = 0;
		int num_blows = 0;
		int chance_to_hit = 0;
		t_string weapon_name = _("weapon^None");

		color_t dmg_color = font::weapon_color;
		if (context_unit_stats.weapon) {
			base_damage = attack_info(rc, *context_unit_stats.weapon, res, *u, unit_loc, sec_u, other_context_unit_stats.weapon);
			total_damage = context_unit_stats.damage;
			num_blows = context_unit_stats.num_blows;
			chance_to_hit = context_unit_stats.chance_to_hit;
			weapon_name = context_unit_stats.weapon->name();

			if ( total_damage > base_damage )
				dmg_color = font::good_dmg_color;
			else if ( total_damage < base_damage )
				dmg_color = font::bad_dmg_color;
		} else {
			str << span_color(font::weapon_color) << weapon_name << naps << "\n";
			tooltip << _("Weapon: ") << "<b>" << weapon_name << "</b>\n"
				<< _("Damage: ") << "<b>" << "0" << "</b>\n";
		}

		color_t chance_color = game_config::red_to_green(chance_to_hit);

		// Total damage.
		str << "  " << span_color(dmg_color) << total_damage << naps << span_color(font::weapon_color)
			<< font::unicode_en_dash << num_blows
			<< " (" << span_color(chance_color) << chance_to_hit << "%" << naps << ")"
			<< naps << "\n";

		tooltip << _("Weapon: ") << "<b>" << weapon_name << "</b>\n"
				<< _("Total damage") << "<b>" << total_damage << "</b>\n";

		// Create the hitpoints distribution.
		std::vector<std::pair<int, double>> hp_prob_vector;

		// First, we sort the probabilities in ascending order.
		std::vector<std::pair<double, int>> prob_hp_vector;
		int i;

		combatant* c = show_attacker ? &attacker_combatant : &defender_combatant;

		for(i = 0; i < static_cast<int>(c->hp_dist.size()); i++) {
			double prob = c->hp_dist[i];

			// We keep only values above 0.1%.
			if(prob > 0.001)
				prob_hp_vector.emplace_back(prob, i);
		}

		std::sort(prob_hp_vector.begin(), prob_hp_vector.end());

		//TODO fendrin -- make that dynamically
		int max_hp_distrib_rows_ = 10;

		// We store a few of the highest probability hitpoint values.
		int nb_elem = std::min<int>(max_hp_distrib_rows_, prob_hp_vector.size());

		for(i = prob_hp_vector.size() - nb_elem;
				i < static_cast<int>(prob_hp_vector.size()); i++) {

			hp_prob_vector.emplace_back(prob_hp_vector[i].second, prob_hp_vector[i].first);
		}

		// Then, we sort the hitpoint values in ascending order.
		std::sort(hp_prob_vector.begin(), hp_prob_vector.end());
		// And reverse the order. Might be doable in a better manor.
		std::reverse(hp_prob_vector.begin(), hp_prob_vector.end());

		for(i = 0; i < static_cast<int>(hp_prob_vector.size()); i++) {

			int hp = hp_prob_vector[i].first;
			double prob = hp_prob_vector[i].second;
			color_t prob_color = game_config::blue_to_white(prob * 100.0, true);

			str		<< span_color(font::weapon_details_color) << "  " << "  "
					<< span_color(u->hp_color(hp)) << format_hp(hp) << naps
					<< " " << font::weapon_numbers_sep << " "
					<< span_color(prob_color) << format_prob(prob) << naps
					<< naps << "\n";
		}

		add_text(res, flush(str), flush(tooltip));
	}
	return res;
}

/*
 * Display the attacks of the displayed unit against the unit passed as argument.
 * 'hex' is the location the attacker will be at during combat.
 */
static config unit_weapons(const reports::context& rc, const unit *u, const map_location &hex)
{
	config res = config();
	if ((u != nullptr) && (!u->attacks().empty())) {
		const std::string attack_headline = _n("Attack", "Attacks", u->attacks().size());

		add_text(res,  span_color(font::weapon_details_color)
				+ attack_headline + "</span>" + '\n', "");

		const auto left = u->attacks_left(false), max = u->max_attacks();
		if(max != 1) {
			// TRANSLATORS: This string is shown in the sidebar beneath the word "Attacks" when a unit can attack multiple times per turn
			const std::string line = VGETTEXT("Remaining: $left/$max",
							{{"left", std::to_string(left)},
							 {"max",  std::to_string(max)}});
			add_text(res, "  " + span_color(font::weapon_details_color, line) + "\n",
				_("This unit can attack multiple times per turn."));
		}

		for (const attack_type &at : u->attacks())
		{
			attack_info(rc, at, res, *u, hex);
		}
	}
	return res;
}
REPORT_GENERATOR(unit_weapons, rc)
{
	const unit *u = get_visible_unit(rc);
	const map_location& mouseover_hex = rc.screen().mouseover_hex();
	const map_location& displayed_unit_hex = rc.screen().displayed_unit_hex();
	const map_location& hex = mouseover_hex.valid() ? mouseover_hex : displayed_unit_hex;
	if (!u) return config();

	return unit_weapons(rc, u, hex);
}
REPORT_GENERATOR(highlighted_unit_weapons, rc)
{
	unit_const_ptr u = get_selected_unit_ptr(rc);
	const unit *sec_u = get_visible_unit(rc);

	if (!u) return report_unit_weapons(rc);
	if (!sec_u || u.get() == sec_u) return unit_weapons(rc, sec_u, rc.screen().mouseover_hex());

	map_location highlighted_hex = rc.screen().displayed_unit_hex();
	map_location attack_loc;
	if (rc.mhb())
		attack_loc = rc.mhb()->current_unit_attacks_from(highlighted_hex);

	if (!attack_loc.valid())
		return unit_weapons(rc, sec_u, rc.screen().mouseover_hex());

	//TODO: shouldn't this pass sec_u as secodn parameter ?
	return unit_weapons(rc, u, attack_loc, sec_u, false);
}
REPORT_GENERATOR(selected_unit_weapons, rc)
{
	unit_const_ptr u = get_selected_unit_ptr(rc);
	const unit *sec_u = get_visible_unit(rc);

	if (!u) return config();
	if (!sec_u || u.get() == sec_u) return unit_weapons(rc, u.get(), u->get_location());

	map_location highlighted_hex = rc.screen().displayed_unit_hex();
	map_location attack_loc;
	if (rc.mhb())
		attack_loc = rc.mhb()->current_unit_attacks_from(highlighted_hex);

	if (!attack_loc.valid())
		return unit_weapons(rc, u.get(), u->get_location());

	return unit_weapons(rc, u, attack_loc, sec_u, true);
}

REPORT_GENERATOR(unit_image,rc)
{
	const unit *u = get_visible_unit(rc);
	if (!u) return config();
	return image_report(u->absolute_image() + u->image_mods());
}
REPORT_GENERATOR(selected_unit_image, rc)
{
	const unit *u = get_selected_unit(rc);
	if (!u) return config();
	return image_report(u->absolute_image() + u->image_mods());
}

REPORT_GENERATOR(selected_unit_profile, rc)
{
	const unit *u = get_selected_unit(rc);
	if (!u) return config();
	return image_report(u->small_profile());
}
REPORT_GENERATOR(unit_profile, rc)
{
	const unit *u = get_visible_unit(rc);
	if (!u) return config();
	return image_report(u->small_profile());
}

static config tod_stats_at(const reports::context& rc, const map_location& hex)
{
	std::ostringstream tooltip;
	std::ostringstream text;

	const map_location& tod_schedule_hex = (hex.valid() && !display::get_singleton()->shrouded(hex)) ? hex : map_location::null_location();
	const std::vector<time_of_day>& schedule = rc.tod().times(tod_schedule_hex);

	tooltip << _("Time of day schedule:") << " \n";
	int current = rc.tod().get_current_time(tod_schedule_hex);
	int i = 0;
	for (const time_of_day& tod : schedule) {
		if (i == current) tooltip << "<big><b>";
		tooltip << tod.name << "\n";
		if (i == current) tooltip << "</b></big>";
		i++;
	}

	int times = schedule.size();
	text << current + 1 << "/" << times;

	return text_report(text.str(), tooltip.str(), "..schedule");
}
REPORT_GENERATOR(tod_stats, rc)
{
	map_location mouseover_hex = rc.screen().mouseover_hex();
	if (mouseover_hex.valid()) return tod_stats_at(rc, mouseover_hex);
	return tod_stats_at(rc, rc.screen().selected_hex());
}
REPORT_GENERATOR(selected_tod_stats, rc)
{
	const unit *u = get_selected_unit(rc);
	if(!u) return tod_stats_at(rc, map_location::null_location());
	const map_location& attack_indicator_src = game_display::get_singleton()->get_attack_indicator_src();
	const map_location& hex =
		attack_indicator_src.valid() ? attack_indicator_src : u->get_location();
	return tod_stats_at(rc, hex);
}

static config time_of_day_at(const reports::context& rc, const map_location& mouseover_hex)
{
	std::ostringstream tooltip;
	time_of_day tod = get_visible_time_of_day_at(rc, mouseover_hex);

	int b = tod.lawful_bonus;
	int l = generic_combat_modifier(b, unit_alignments::type::liminal, false, rc.tod().get_max_liminal_bonus());
	std::string  lawful_color("white");
	std::string chaotic_color("white");
	std::string liminal_color("white");

	if (b != 0) {
		lawful_color  = (b > 0) ? "#0f0" : "#f00";
		chaotic_color = (b < 0) ? "#0f0" : "#f00";
	}
	if (l != 0) {
		liminal_color = (l > 0) ? "#0f0" : "#f00";
	}
	tooltip << _("Time of day:") << " <b>" << tod.name << "</b>\n"
		<< _("Lawful units: ") << "<span foreground=\"" << lawful_color  << "\">"
		<< utils::signed_percent(b)  << "</span>\n"
		<< _("Neutral units: ") << utils::signed_percent(0)  << '\n'
		<< _("Chaotic units: ") << "<span foreground=\"" << chaotic_color << "\">"
		<< utils::signed_percent(-b) << "</span>\n"
		<< _("Liminal units: ") << "<span foreground=\"" << liminal_color << "\">"
		<< utils::signed_percent(l) << "</span>\n";

	std::string tod_image = tod.image;
	if(tod.bonus_modified > 0) {
		tod_image += (formatter() << "~BLIT(" << game_config::images::tod_bright << ")").str();
	} else if(tod.bonus_modified < 0) {
		tod_image += (formatter() << "~BLIT(" << game_config::images::tod_dark << ")").str();
	}

	return image_report(tod_image, tooltip.str(), "time_of_day_" + tod.id);
}
REPORT_GENERATOR(time_of_day, rc)
{
	map_location mouseover_hex = rc.screen().mouseover_hex();
	if (mouseover_hex.valid()) return time_of_day_at(rc, mouseover_hex);
	return time_of_day_at(rc, rc.screen().selected_hex());
}
REPORT_GENERATOR(selected_time_of_day, rc)
{
	const unit *u = get_selected_unit(rc);
	if(!u) return time_of_day_at(rc, map_location::null_location());
	const map_location& attack_indicator_src = game_display::get_singleton()->get_attack_indicator_src();
	const map_location& hex =
		attack_indicator_src.valid() ? attack_indicator_src : u->get_location();
	return time_of_day_at(rc, hex);
}

static config unit_box_at(const reports::context& rc, const map_location& mouseover_hex)
{
	std::ostringstream tooltip;
	time_of_day global_tod = rc.tod().get_time_of_day();
	time_of_day local_tod = get_visible_time_of_day_at(rc, mouseover_hex);

	int bonus = local_tod.lawful_bonus;
	int l = generic_combat_modifier(bonus, unit_alignments::type::liminal, false, rc.tod().get_max_liminal_bonus());

	std::string  lawful_color("white");
	std::string chaotic_color("white");
	std::string liminal_color("white");

	if (bonus != 0) {
		lawful_color  = (bonus > 0) ? "green" : "red";
		chaotic_color = (bonus < 0) ? "green" : "red";
	}
	if (l != 0) {
		liminal_color = (l > 0) ? "green" : "red";
	}
	tooltip << local_tod.name << '\n'
		<< _("Lawful units: ") << "<span foreground=\"" << lawful_color  << "\">"
		<< utils::signed_percent(bonus)  << "</span>\n"
		<< _("Neutral units: ") << utils::signed_percent(0)  << '\n'
		<< _("Chaotic units: ") << "<span foreground=\"" << chaotic_color << "\">"
		<< utils::signed_percent(-bonus) << "</span>\n"
		<< _("Liminal units: ") << "<span foreground=\"" << liminal_color << "\">"
		<< utils::signed_percent(l) << "</span>\n";

	std::string local_tod_image  = "themes/classic/" + local_tod.image;
	std::string global_tod_image = "themes/classic/" + global_tod.image;
	if(local_tod.bonus_modified != 0) {
		local_tod_image += "~BLIT(";
		if (local_tod.bonus_modified > 0) local_tod_image += game_config::images::tod_bright;
		else if (local_tod.bonus_modified < 0) local_tod_image += game_config::images::tod_dark;
		local_tod_image += ")";
	}

	const gamemap &map = rc.map();
	t_translation::terrain_code terrain = map.get_terrain(mouseover_hex);

	//if (t_translation::terrain_matches(terrain, t_translation::ALL_OFF_MAP))
	//	return config();

	//if (map.is_keep(mouseover_hex)) {
	//	add_image(cfg, "icons/terrain/terrain_type_keep.png", "");
	//}

	const t_translation::ter_list& underlying_terrains = map.underlying_union_terrain(terrain);

	std::string bg_terrain_image;

	for (const t_translation::terrain_code& underlying_terrain : underlying_terrains) {
		const std::string& terrain_id = map.get_terrain_info(underlying_terrain).id();
		bg_terrain_image = "~BLIT(unit_env/terrain/terrain-" + terrain_id + ".png)" + bg_terrain_image;
	}

	std::stringstream color;
	color << local_tod.color;

	bg_terrain_image = bg_terrain_image + "~CS(" + color.str() + ")";

	const unit *u = get_visible_unit(rc);
	std::string unit_image;
	if (u)
		unit_image = "~BLIT(" + u->absolute_image() + u->image_mods() + ",35,22)";

	std::string tod_image = global_tod_image + "~BLIT(" + local_tod_image  + ")";

	return image_report(tod_image + bg_terrain_image + unit_image, tooltip.str(), "time_of_day");
}
REPORT_GENERATOR(unit_box, rc)
{
	map_location mouseover_hex = rc.screen().mouseover_hex();
	return unit_box_at(rc, mouseover_hex);
}


REPORT_GENERATOR(turn, rc)
{
	std::ostringstream str, tooltip;
	str << rc.tod().turn();
	int nb = rc.tod().number_of_turns();
	if (nb != -1) str << '/' << nb;

	tooltip << _("Turn Number");
	if(nb != -1) {
		tooltip << "\n\n" << _("When the game exceeds the number of turns indicated by the second number, it will end.");
	}
	return text_report(str.str(), tooltip.str());
}

REPORT_GENERATOR(gold, rc)
{
	std::ostringstream str;
	int viewing_side = rc.screen().viewing_side();
	// Suppose the full unit map is applied.
	int fake_gold = rc.dc().get_team(viewing_side).gold();

	if (rc.wb())
		fake_gold -= rc.wb()->get_spent_gold_for(viewing_side);
	char const *end = naps;
	if (viewing_side != rc.screen().playing_side()) {
		str << span_color(font::GRAY_COLOR);
	}
	else if (fake_gold < 0) {
		str << span_color(font::BAD_COLOR);
	}
	else {
		end = "";
	}
	str << utils::half_signed_value(fake_gold) << end;
	return text_report(str.str(), _("Gold") + "\n\n" + _("The amount of gold currently available to recruit and maintain your army."));
}

REPORT_GENERATOR(villages, rc)
{
	std::ostringstream str;
	int viewing_side = rc.screen().viewing_side();
	const team &viewing_team = rc.dc().get_team(viewing_side);
	str << viewing_team.villages().size() << '/';
	if (viewing_team.uses_shroud()) {
		int unshrouded_villages = 0;
		for (const map_location &loc : rc.map().villages()) {
			if (!viewing_team.shrouded(loc))
				++unshrouded_villages;
		}
		str << unshrouded_villages;
	} else {
		str << rc.map().villages().size();
	}
	return gray_inactive(rc,str.str(), _("Villages") + "\n\n" + _("The fraction of known villages that your side has captured."));
}

REPORT_GENERATOR(num_units, rc)
{
	return gray_inactive(rc, std::to_string(rc.dc().side_units(rc.screen().viewing_side())), _("Units") + "\n\n" + _("The total number of units on your side."));
}

REPORT_GENERATOR(upkeep, rc)
{
	std::ostringstream str;
	int viewing_side = rc.screen().viewing_side();
	const team &viewing_team = rc.dc().get_team(viewing_side);
	team_data td(rc.dc(), viewing_team);
	str << td.expenses << " (" << td.upkeep << ")";
	return gray_inactive(rc,str.str(), _("Upkeep") + "\n\n" + _("The expenses incurred at the end of every turn to maintain your army. The first number is the amount of gold that will be deducted. It is equal to the number of unit levels not supported by villages. The second is the total cost of upkeep, including that covered by villages — in other words, the amount of gold that would be deducted if you lost all villages."));
}

REPORT_GENERATOR(expenses, rc)
{
	int viewing_side = rc.screen().viewing_side();
	const team &viewing_team = rc.dc().get_team(viewing_side);
	team_data td(rc.dc(), viewing_team);
	return gray_inactive(rc,std::to_string(td.expenses));
}

REPORT_GENERATOR(income, rc)
{
	std::ostringstream str;
	int viewing_side = rc.screen().viewing_side();
	const team &viewing_team = rc.dc().get_team(viewing_side);
	team_data td(rc.dc(), viewing_team);
	char const *end = naps;
	if (viewing_side != rc.screen().playing_side()) {
		if (td.net_income < 0) {
			td.net_income = - td.net_income;
			str << span_color(font::GRAY_COLOR);
			str << font::unicode_minus;
		}
		else {
			str << span_color(font::GRAY_COLOR);
		}
	}
	else if (td.net_income < 0) {
		td.net_income = - td.net_income;
		str << span_color(font::BAD_COLOR);
		str << font::unicode_minus;
	}
	else {
		end = "";
	}
	str << td.net_income << end;
	return text_report(str.str(), _("Net Income") + "\n\n" + _("The net amount of gold you gain or lose each turn, taking into account income from controlled villages and payment of upkeep."));
}

namespace {
void blit_tced_icon(config &cfg, const std::string &terrain_id, const std::string &icon_image, bool high_res,
	const std::string &terrain_name) {
	const std::string tc_base = high_res ? "images/buttons/icon-base-32.png" : "images/buttons/icon-base-16.png";
	const std::string terrain_image = "terrain/" + icon_image + (high_res ? "_30.png" : ".png");
	add_image(cfg, tc_base + "~RC(magenta>" + terrain_id + ")~BLIT(" + terrain_image + ")", terrain_name);
}
}

REPORT_GENERATOR(terrain_info, rc)
{
	const gamemap& map = rc.map();
	map_location mouseover_hex = rc.screen().mouseover_hex();

	if(!map.on_board(mouseover_hex)) {
		mouseover_hex = rc.screen().selected_hex();
	}

	if(!map.on_board(mouseover_hex)) {
		return config();
	}

	t_translation::terrain_code terrain = map.get_terrain(mouseover_hex);
	if(t_translation::terrain_matches(terrain, t_translation::ALL_OFF_MAP)) {
		return config();
	}

	config cfg;

	bool high_res = false;

	if(display::get_singleton()->shrouded(mouseover_hex)) {
		return cfg;
	}
	//TODO
//	if (display::get_singleton()->fogged(mouseover_hex)) {
//		blit_tced_icon(cfg, "fog", high_res);
//	}
//
//	if (map.is_keep(mouseover_hex)) {
//		blit_tced_icon(cfg, "keep", high_res);
//	}

	const t_translation::ter_list& underlying_terrains = map.underlying_union_terrain(terrain);
	for(const t_translation::terrain_code& underlying_terrain : underlying_terrains) {
		if(t_translation::terrain_matches(underlying_terrain, t_translation::ALL_OFF_MAP)) {
			continue;
		}
		const std::string& terrain_id = map.get_terrain_info(underlying_terrain).id();
		const std::string& terrain_name = map.get_terrain_string(underlying_terrain);
		const std::string& terrain_icon = map.get_terrain_info(underlying_terrain).icon_image();
		if(terrain_icon.empty()) {
			continue;
		}
		blit_tced_icon(cfg, terrain_id, terrain_icon, high_res, terrain_name);
	}

	if(map.is_village(mouseover_hex)) {
		int owner = rc.dc().village_owner(mouseover_hex);
		// This report is used in both game and editor. get_team(viewing_side) would throw in the editor's
		// terrain-only mode, but if the village already has an owner then we're not in that mode.
		if(owner != 0) {
			int viewing_side = rc.screen().viewing_side();
			const team& viewing_team = rc.dc().get_team(viewing_side);

			if(!viewing_team.fogged(mouseover_hex)) {
				const team& owner_team = rc.dc().get_team(owner);

				std::string flag_icon = owner_team.flag_icon();
				std::string old_rgb = game_config::flag_rgb;
				std::string new_rgb = team::get_side_color_id(owner_team.side());
				std::string mods = "~RC(" + old_rgb + ">" + new_rgb + ")";
				if(flag_icon.empty()) {
					flag_icon = game_config::images::flag_icon;
				}
				std::string tooltip = side_tooltip(owner_team);
				std::string side = std::to_string(owner_team.side());

				add_image(cfg, flag_icon + mods, tooltip);
				add_text(cfg, side, tooltip);
			}
		}
	}

	return cfg;
}

REPORT_GENERATOR(terrain, rc)
{
	const gamemap &map = rc.map();
	int viewing_side = rc.screen().viewing_side();
	const team &viewing_team = rc.dc().get_team(viewing_side);
	map_location mouseover_hex = rc.screen().mouseover_hex();
	if (!map.on_board(mouseover_hex) || viewing_team.shrouded(mouseover_hex))
		return config();

	t_translation::terrain_code terrain = map.get_terrain(mouseover_hex);
	if (t_translation::terrain_matches(terrain, t_translation::ALL_OFF_MAP))
		return config();

	std::ostringstream str;
	if (map.is_village(mouseover_hex))
	{
		int owner = rc.dc().village_owner(mouseover_hex);
		if (owner == 0 || viewing_team.fogged(mouseover_hex)) {
			str << map.get_terrain_info(terrain).income_description();
		} else if (owner == viewing_side) {
			str << map.get_terrain_info(terrain).income_description_own();
		} else if (viewing_team.is_enemy(owner)) {
			str << map.get_terrain_info(terrain).income_description_enemy();
		} else {
			str << map.get_terrain_info(terrain).income_description_ally();
		}

		const std::string& underlying_desc = map.get_underlying_terrain_string(terrain);
		if(!underlying_desc.empty()) {
			str << underlying_desc;
		}
	} else {
		str << map.get_terrain_string(terrain);
	}

	return text_report(str.str());
}

REPORT_GENERATOR(zoom_level, rc)
{
	std::ostringstream text;
	std::ostringstream tooltip;
	std::ostringstream help;

	text << static_cast<int>(rc.screen().get_zoom_factor() * 100) << "%";

	return text_report(text.str(), tooltip.str(), help.str());
}

REPORT_GENERATOR(position, rc)
{
	const gamemap &map = rc.map();
	map_location mouseover_hex = rc.screen().mouseover_hex(),
		displayed_unit_hex = rc.screen().displayed_unit_hex(),
		selected_hex = rc.screen().selected_hex();

	if (!map.on_board(mouseover_hex)) {
		if (!map.on_board(selected_hex))
			return config();
		else {
			mouseover_hex = selected_hex;
		}
	}

	t_translation::terrain_code terrain = map[mouseover_hex];
	if (t_translation::terrain_matches(terrain, t_translation::ALL_OFF_MAP))
		return config();

	std::ostringstream str;
	str << mouseover_hex;

	const unit *u = get_visible_unit(rc);
	const team &viewing_team = rc.teams()[rc.screen().viewing_team()];
	if (!u ||
	    (displayed_unit_hex != mouseover_hex &&
	     displayed_unit_hex != rc.screen().selected_hex()) ||
	    viewing_team.shrouded(mouseover_hex))
		return text_report(str.str());

	int move_cost = u->movement_cost(terrain);
	int defense = 100 - u->defense_modifier(terrain);
	if (move_cost < movetype::UNREACHABLE) {
		str << " " << defense << "%," << move_cost;
	} else if (mouseover_hex == displayed_unit_hex) {
		str << " " << defense << "%,‒";
	} else {
		str << " ‒";
	}
	return text_report(str.str());
}

REPORT_GENERATOR(side_playing, rc)
{
	const team &active_team = rc.teams()[rc.screen().playing_team()];
	std::string flag_icon = active_team.flag_icon();
	std::string old_rgb = game_config::flag_rgb;
	std::string new_rgb = team::get_side_color_id(rc.screen().playing_side());
	std::string mods = "~RC(" + old_rgb + ">" + new_rgb + ")";
	if (flag_icon.empty())
		flag_icon = game_config::images::flag_icon;
	return image_report(flag_icon + mods, side_tooltip(active_team));
}

REPORT_GENERATOR(observers, rc)
{
	const std::set<std::string> &observers = rc.screen().observers();
	if (observers.empty())
		return config();

	std::ostringstream str;
	str << _("Observers:") << '\n';
	for (const std::string &obs : observers) {
		str << obs << '\n';
	}
	return image_report(game_config::images::observer, str.str());
}

REPORT_GENERATOR(report_clock, /*rc*/)
{
	config report;
	add_image(report, game_config::images::time_icon, "");

	std::ostringstream ss;

	const char* format = prefs::get().use_twelve_hour_clock_format()
		? "%I:%M %p"
		: "%H:%M";

	std::time_t t = std::time(nullptr);
	ss << std::put_time(std::localtime(&t), format);
	add_text(report, ss.str(), _("Clock"));

	return report;
}


REPORT_GENERATOR(battery, /*rc*/)
{
	config report;

	add_image(report, game_config::images::battery_icon, "");
	add_text(report, (boost::format("%.0f %%") % desktop::battery_info::get_battery_percentage()).str(), _("Battery"));

	return report;
}

REPORT_GENERATOR(report_countdown, rc)
{
	int viewing_side = rc.screen().viewing_side();
	const team &viewing_team = rc.dc().get_team(viewing_side);
	int min, sec;
	if (viewing_team.countdown_time() == 0)
		return report_report_clock(rc);
	std::ostringstream str;
	sec = viewing_team.countdown_time() / 1000;
	char const *end = naps;
	if (viewing_side != rc.screen().playing_side())
		str << span_color(font::GRAY_COLOR);
	else if (sec < 60)
		str << "<span foreground=\"#c80000\">";
	else if (sec < 120)
		str << "<span foreground=\"#c8c800\">";
	else
		end = "";
	min = sec / 60;
	str << min << ':';
	sec = sec % 60;
	if (sec < 10) str << '0';
	str << sec << end;

	config report;
	add_image(report, game_config::images::time_icon, "");
	add_text(report, str.str(), _("Turn Countdown") + "\n\n" + _("Countdown until your turn automatically ends."));

	return report;
}

void reports::register_generator(const std::string &name, reports::generator *g)
{
	dynamic_generators_[name].reset(g);
}

config reports::generate_report(const std::string &name, const reports::context& rc, bool only_static)
{
	if (!only_static) {
		dynamic_report_generators::const_iterator i = dynamic_generators_.find(name);
		if (i != dynamic_generators_.end())
			return i->second->generate(rc);
	}
	static_report_generators::const_iterator j = static_generators.find(name);
	if (j != static_generators.end())
		return j->second(rc);
	return config();
}

const std::set<std::string> &reports::report_list()
{
	if (!all_reports_.empty()) return all_reports_;
	for (const static_report_generators::value_type &v : static_generators) {
		all_reports_.insert(v.first);
	}
	for (const dynamic_report_generators::value_type &v : dynamic_generators_) {
		all_reports_.insert(v.first);
	}
	return all_reports_;
}
