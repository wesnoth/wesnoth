/* $Id$ */
/*
   Copyright (C) 2003 - 2010 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "actions.hpp"
#include "font.hpp"
#include "foreach.hpp"
#include "game_display.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "language.hpp"
#include "map.hpp"
#include "marked-up_text.hpp"
#include "reports.hpp"
#include "resources.hpp"
#include "team.hpp"
#include "tod_manager.hpp"
#include "unit.hpp"
#include "whiteboard/manager.hpp"

#include <cassert>
#include <ctime>

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

static config report()
{
	return config();
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

using reports::report_data;
using font::span_color;

static void add_status(config &r,
	char const *path, char const *desc1, char const *desc2)
{
	std::ostringstream s;
	s << gettext(desc1) << gettext(desc2);
	add_image(r, path, s.str());
}

static std::string flush(std::ostringstream &s)
{
	std::string r(s.str());
	s.str(std::string());
	return r;
}

typedef config (*generator_function)(const report_data &);

struct static_report_generator
{
	generator_function generator;
	bool for_units;
};

struct dynamic_report_generator
{
	reports::generator *generator;
	bool for_units;
};

typedef std::map<std::string, static_report_generator> static_report_generators;
typedef std::map<std::string, dynamic_report_generator> dynamic_report_generators;
static static_report_generators static_generators;
static dynamic_report_generators dynamic_generators;

struct report_generator_helper
{
	report_generator_helper(const char *name, generator_function g, bool u)
	{
		static_report_generator rg = { g, u };
		static_generators.insert(static_report_generators::value_type(name, rg));
	}
};

#define REPORT_GENERATOR(n, u, data) \
	static config report_##n(const report_data &); \
	static report_generator_helper reg_gen_##n(#n, &report_##n, u); \
	static config report_##n(const report_data &data)

static char const *naps = "</span>";

static unit *get_visible_unit(const report_data &data)
{
	return get_visible_unit(data.displayed_unit_hex,
		(*resources::teams)[data.viewing_side - 1], data.show_everything);
}

static config gray_inactive(const report_data &data, const std::string &str)
{
	if (data.viewing_side == data.active_side)
		return text_report(str);
	return text_report(span_color(font::GRAY_COLOR) + str + naps);
}

REPORT_GENERATOR(unit_name, true, data)
{
	unit *u = get_visible_unit(data);
	if (!u) return report();
	std::ostringstream str, tooltip;
	str << "<b>" << u->name() << "</b>";
	tooltip << _("Name: ") << "<b>" << u->name() << "</b>";
	return text_report(str.str(), tooltip.str());
}

REPORT_GENERATOR(unit_type, true, data)
{
	unit *u = get_visible_unit(data);
	if (!u) return report();
	std::ostringstream str, tooltip;
	str << span_color(font::unit_type_color) << u->type_name() << naps;
	tooltip << _("Type: ") << "<b>" << u->type_name() << "</b>\n"
		<< u->unit_description();
	return text_report(str.str(), tooltip.str(), "unit_" + u->type_id());
}

REPORT_GENERATOR(unit_race, true, data)
{
	unit *u = get_visible_unit(data);
	if (!u) return report();
	std::ostringstream str, tooltip;
	str << span_color(font::race_color) << u->race()->name(u->gender()) << naps;
	tooltip << _("Race: ") << "<b>" << u->race()->name(u->gender()) << "</b>";
	return text_report(str.str(), tooltip.str(), "..race_" + u->race()->id());
}

REPORT_GENERATOR(unit_side, true, data)
{
	unit *u = get_visible_unit(data);
	if (!u) return report();
	const team &u_team = (*resources::teams)[u->side() - 1];
	std::string flag_icon = u_team.flag_icon();
	std::string old_rgb = game_config::flag_rgb;
	std::string new_rgb = team::get_side_color_index(u->side());
	std::string mods = "~RC(" + old_rgb + ">" + new_rgb + ")";
	if (flag_icon.empty())
		flag_icon = game_config::images::flag_icon;
	return image_report(flag_icon + mods, u_team.current_player());
}

REPORT_GENERATOR(unit_level, true, data)
{
	unit *u = get_visible_unit(data);
	if (!u) return report();
	std::ostringstream str, tooltip;
	str << u->level();
	tooltip << _("Level: ") << "<b>" << u->level() << "</b>\n";
	const std::vector<std::string> &adv_to = u->advances_to();
	if (adv_to.empty())
		tooltip << _("No advancement");
	else
		tooltip << _("Advances to:") << "\n<b>\t"
			<< utils::join(adv_to, "\n\t") << "</b>";
	return text_report(str.str(), tooltip.str());
}

REPORT_GENERATOR(unit_amla, true, data)
{
	unit *u = get_visible_unit(data);
	if (!u) return report();
	config res;
	typedef std::pair<std::string, std::string> pair_string;
	foreach(const pair_string &ps, u->amla_icons()) {
		add_image(res, ps.first, ps.second);
	}
	return res;
}

REPORT_GENERATOR(unit_traits, true, data)
{
	unit *u = get_visible_unit(data);
	if (!u) return report();
	config res;
	const std::vector<t_string> &traits = u->trait_names();
	const std::vector<t_string> &descriptions = u->trait_descriptions();
	unsigned nb = traits.size();
	for (unsigned i = 0; i < nb; ++i)
	{
		std::ostringstream str, tooltip;
		str << traits[i];
		if (i != nb - 1 ) str << ", ";
		tooltip << _("Trait: ") << "<b>" << traits[i] << "</b>\n"
			<< descriptions[i];
		add_text(res, str.str(), tooltip.str());
	}
	return res;
}

REPORT_GENERATOR(unit_status, true, data)
{
	unit *u = get_visible_unit(data);
	if (!u) return report();
	config res;
	if (resources::game_map->on_board(data.displayed_unit_hex) &&
	    u->invisible(data.displayed_unit_hex))
	{
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
	return res;
}

REPORT_GENERATOR(unit_alignment, true, data)
{
	unit *u = get_visible_unit(data);
	if (!u) return report();
	std::ostringstream str, tooltip;
	char const *align = unit_type::alignment_description(u->alignment(), u->gender());
	std::string align_id = unit_type::alignment_id(u->alignment());
	int cm = combat_modifier(data.displayed_unit_hex, u->alignment(), u->is_fearless());
	str << align << " (" << utils::signed_percent(cm) << ")";
	tooltip << _("Alignment: ") << "<b>" << align << "</b>\n"
		<< string_table[align_id + "_description"];
	return text_report(str.str(), tooltip.str(), "time_of_day");
}

REPORT_GENERATOR(unit_abilities, true, data)
{
	unit *u = get_visible_unit(data);
	if (!u) return report();
	config res;
	const std::vector<std::string> &abilities = u->ability_tooltips();
	for (std::vector<std::string>::const_iterator i = abilities.begin(),
	     i_end = abilities.end(); i != i_end; ++i)
	{
		std::ostringstream str, tooltip;
		const std::string &name = *i;
		str << gettext(name.c_str());
		if (i + 2 != i_end) str << ", ";
		++i;
		tooltip << _("Ability: ") << *i;
		add_text(res, str.str(), tooltip.str(), "ability_" + name);
	}
	return res;
}

REPORT_GENERATOR(unit_hp, true, data)
{
	unit *u = get_visible_unit(data);
	if (!u) return report();
	std::ostringstream str, tooltip;
	str << span_color(u->hp_color()) << u->hitpoints()
		<< '/' << u->max_hitpoints() << naps;

	std::set<std::string> resistances_table;
	utils::string_map resistances = u->get_base_resistances();

	bool att_def_diff = false;
	foreach (const utils::string_map::value_type &resist, u->get_base_resistances())
	{
		std::ostringstream line;
		line << gettext(resist.first.c_str()) << ": ";
		// Some units have different resistances when attacking or defending.
		int res_att = 100 - u->resistance_against(resist.first, true, data.displayed_unit_hex);
		int res_def = 100 - u->resistance_against(resist.first, false, data.displayed_unit_hex);
		if (res_att == res_def) {
			line << utils::signed_percent(res_def) << "\n";
		} else {
			line << utils::signed_percent(res_att) << " / " << utils::signed_percent(res_def) << '\n';
			att_def_diff = true;
		}
		resistances_table.insert(line.str());
	}

	tooltip << _("Resistances: ");
	if (att_def_diff)
		tooltip << _("(Att / Def)");
	tooltip << '\n';
	foreach (const std::string &line, resistances_table) {
		tooltip << line;
	}
	return text_report(str.str(), tooltip.str());
}

REPORT_GENERATOR(unit_xp, true, data)
{
	unit *u = get_visible_unit(data);
	if (!u) return report();
	std::ostringstream str, tooltip;
	str << span_color(u->xp_color()) << u->experience()
		<< '/' << u->max_experience() << naps;

	std::string exp_mod = data.level["experience_modifier"].str();
	tooltip << _("Experience Modifier: ") << (!exp_mod.empty() ? exp_mod : "100") << '%';
	return text_report(str.str(), tooltip.str());
}

REPORT_GENERATOR(unit_advancement_options, true, data)
{
	unit *u = get_visible_unit(data);
	if (!u) return report();
	config res;
	typedef std::pair<std::string, std::string> pair_string;
	foreach (const pair_string &ps, u->advancement_icons()) {
		add_image(res, ps.first, ps.second);
	}
	return res;
}

REPORT_GENERATOR(unit_defense, true, data)
{
	unit *u = get_visible_unit(data);
	if (!u) return report();
	std::ostringstream str, tooltip;
	const gamemap &map = *resources::game_map;
	const t_translation::t_terrain &terrain = map[data.displayed_unit_hex];
	int def = 100 - u->defense_modifier(terrain);
	SDL_Color color = int_to_color(game_config::red_to_green(def));
	str << span_color(color) << def << "%</span>";
	tooltip << _("Terrain: ") << "<b>" << map.get_terrain_info(terrain).description() << "</b>\n";

	const t_translation::t_list &underlyings = map.underlying_def_terrain(terrain);
	std::vector<int> t_defs;
	bool revert = false;
	if (underlyings.size() != 1 || underlyings.front() != terrain)
	{
		foreach (const t_translation::t_terrain &t, underlyings)
		{
			if (t == t_translation::MINUS) {
				revert = true;
			} else if (t == t_translation::PLUS) {
				revert = false;
			} else {
				int t_def = 100 - u->defense_modifier(t);
				SDL_Color color = int_to_color(game_config::red_to_green(t_def));
				tooltip << '\t' << map.get_terrain_info(t).description() << ": "
					<< span_color(color) << t_def << "%</span> "
					<< (revert ? _("maximum^max.") : _("minimum^min.")) << '\n';
			}
		}
	}

	tooltip << "<b>" << _("Defense: ") << span_color(color)  << def << "%</span></b>";
	return text_report(str.str(), tooltip.str());
}

REPORT_GENERATOR(unit_moves, true, data)
{
	unit *u = get_visible_unit(data);
	if (!u) return report();
	std::ostringstream str;
	double movement_frac = 1.0;
	if (u->side() == data.active_side) {
		movement_frac = double(u->movement_left()) / std::max<int>(1, u->total_movement());
		if (movement_frac > 1.0)
			movement_frac = 1.0;
	}

	int grey = 128 + int((255 - 128) * movement_frac);
	SDL_Color c = create_color(grey, grey, grey);
	str << span_color(c) << u->movement_left() << '/' << u->total_movement() << naps;
	return text_report(str.str());
}

REPORT_GENERATOR(unit_weapons, true, data)
{
	unit *u = get_visible_unit(data);
	if (!u) return report();
	std::ostringstream str, tooltip;
	config res;

	foreach (const attack_type &at, u->attacks())
	{
		at.set_specials_context(data.displayed_unit_hex, map_location(), *u);
		int base_damage = at.damage();
		int damage_multiplier = 100;
		int tod_bonus = combat_modifier(data.displayed_unit_hex, u->alignment(), u->is_fearless());
		damage_multiplier += tod_bonus;
		int leader_bonus = 0;
		if (under_leadership(*resources::units, data.displayed_unit_hex, &leader_bonus).valid())
			damage_multiplier += leader_bonus;

		// Assume no specific resistance.
		damage_multiplier *= 100;
		bool slowed = u->get_state(unit::STATE_SLOWED);
		int damage_divisor = slowed ? 20000 : 10000;
		int damage = round_damage(base_damage, damage_multiplier, damage_divisor);

		int base_nattacks = at.num_attacks();
		int nattacks = base_nattacks;
		unit_ability_list swarm = at.get_specials("swarm");
		if (!swarm.empty())
		{
			int swarm_max_attacks = swarm.highest("swarm_attacks_max", nattacks).first;
			int swarm_min_attacks = swarm.highest("swarm_attacks_min").first;
			int hitp = u->hitpoints();
			int mhitp = u->max_hitpoints();
			nattacks = swarm_min_attacks + (swarm_max_attacks - swarm_min_attacks) * hitp / mhitp;
		}

		SDL_Color dmg_color = font::weapon_color;
		double dmg_bonus = double(damage) / base_damage;
		if (dmg_bonus > 1.0)
			dmg_color = font::good_dmg_color;
		else if (dmg_bonus < 1.0)
			dmg_color = font::bad_dmg_color;

		str << span_color(dmg_color) << damage << naps << span_color(font::weapon_color)
			<< font::weapon_numbers_sep << nattacks << ' ' << at.name()
			<< "</span>\n";
		tooltip << _("Weapon: ") << "<b>" << at.name() << "</b>\n"
			<< _("Damage: ") << "<b>" << damage << "</b>\n";

		if (tod_bonus || leader_bonus || slowed)
		{
			tooltip << '\t' << _("Base damage: ") << base_damage << '\n';
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

		tooltip << _("Attacks: ") << "<b>" << nattacks << "</b>\n";
		if (nattacks != base_nattacks){
			tooltip << '\t' << _("Base attacks: ") << base_nattacks << '\n';
			int hp_ratio = u->hitpoints() * 100 / u->max_hitpoints();
			tooltip << '\t' << _("Swarm: ") << "* "<< hp_ratio << "%\n";
		}

		add_text(res, flush(str), flush(tooltip));

		std::string range = string_table["range_" + at.range()];
		std::string lang_type = string_table["type_" + at.type()];

		str << span_color(font::weapon_details_color) << "  "
			<< range << font::weapon_details_sep
			<< lang_type << "</span>\n";

		tooltip << _("Weapon range: ") << "<b>" << range << "</b>\n"
			<< _("Damage type: ")  << "<b>" << lang_type << "</b>\n"
			<< _("Damage versus: ") << '\n';

		// Show this weapon damage and resistance against all the different units.
		// We want weak resistances (= good damage) first.
		std::map<int, std::set<std::string>, std::greater<int> > resistances;
		std::set<std::string> seen_types;
		const team &unit_team = (*resources::teams)[u->side() - 1];
		const team &viewing_team = (*resources::teams)[data.viewing_side - 1];
		foreach(const unit &enemy, *resources::units)
		{
			if (!unit_team.is_enemy(enemy.side()))
				continue;
			const map_location &loc = enemy.get_location();
			if (viewing_team.fogged(loc) ||
			    (viewing_team.is_enemy(enemy.side()) && enemy.invisible(loc)))
				continue;
			bool new_type = seen_types.insert(enemy.type_id()).second;
			if (new_type) {
				int resistance = enemy.resistance_against(at, false, loc);
				resistances[resistance].insert(enemy.type_name());
			}
		}

		// Get global ToD.
		damage_multiplier = 100;
		tod_bonus = combat_modifier(map_location::null_location, u->alignment(), u->is_fearless());
		damage_multiplier += tod_bonus;

		typedef std::pair<int, std::set<std::string> > resist_units;
		foreach (const resist_units &resist, resistances) {
			int damage = round_damage(base_damage, damage_multiplier * resist.first, damage_divisor);
			tooltip << "<b>" << damage << "</b>  "
				<< "<i>(" << utils::signed_percent(resist.first-100) << ")</i> : "
				<< utils::join(resist.second, ", ") << '\n';
		}
		add_text(res, flush(str), flush(tooltip));

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

		const std::vector<t_string> &specials = at.special_tooltips();
		if (!specials.empty())
		{
			for (std::vector<t_string>::const_iterator sp_it = specials.begin(),
			     sp_end = specials.end(); sp_it != sp_end; ++sp_it)
			{
				str << span_color(font::weapon_details_color)
					<< "  " << *sp_it << "</span>\n";
				std::string help_page = "weaponspecial_" + sp_it->base_str();
				++sp_it;
				//FIXME pull out special's name from description
				tooltip << _("Weapon special: ") << *sp_it << '\n';
				add_text(res, flush(str), flush(tooltip), help_page);
			}
		}
	}
	return res;
}

REPORT_GENERATOR(unit_image, true, data)
{
	unit *u = get_visible_unit(data);
	if (!u) return report();
	return image_report(u->absolute_image() + u->image_mods());
}

REPORT_GENERATOR(unit_profile, true, data)
{
	unit *u = get_visible_unit(data);
	if (!u) return report();
	return image_report(u->small_profile());
}

REPORT_GENERATOR(time_of_day, false, data)
{
	std::ostringstream tooltip;
	time_of_day tod;
	const team &viewing_team = (*resources::teams)[data.viewing_side - 1];
	map_location mouseover_hex = resources::screen->mouseover_hex();
	if (viewing_team.shrouded(mouseover_hex)) {
		// Don't show time on shrouded tiles.
		tod = resources::tod_manager->get_time_of_day();
	} else if (viewing_team.fogged(mouseover_hex)) {
		// Don't show illuminated time on fogged tiles.
		tod = resources::tod_manager->get_time_of_day(mouseover_hex);
	} else {
		tod = resources::tod_manager->time_of_day_at(mouseover_hex);
	}

	int b = tod.lawful_bonus;
	int c = tod.liminal_bonus;
	tooltip << tod.name << '\n'
		<< _("Lawful units: ") << utils::signed_percent(b) << '\n'
		<< _("Neutral units: ") << utils::signed_percent(0) << '\n'
		<< _("Chaotic units: ") << utils::signed_percent(-b);
	if (tod.liminal_present)
		tooltip << '\n' << _("Liminal units: ") << utils::signed_percent(c);

	std::string tod_image = tod.image;
	if (tod.lawful_bonus_modified > 0) tod_image += "~BRIGHTEN()";
	else if (tod.lawful_bonus_modified < 0) tod_image += "~DARKEN()";
	if (preferences::flip_time()) tod_image += "~FL(horiz)";

	return image_report(tod_image, tooltip.str(), "time_of_day");
}

REPORT_GENERATOR(turn, false, /*data*/)
{
	std::ostringstream str;
	str << resources::tod_manager->turn();
	int nb = resources::tod_manager->number_of_turns();
	if (nb != -1) str << '/' << nb;
	return text_report(str.str());
}

REPORT_GENERATOR(gold, false, data)
{
	std::ostringstream str;
	// Suppose the full/"pathfind" unit map is applied.
	int fake_gold = (*resources::teams)[data.viewing_side - 1].gold() -
		resources::whiteboard->get_spent_gold_for(data.viewing_side);
	char const *end = naps;
	if (data.viewing_side != data.active_side)
		str << span_color(font::GRAY_COLOR);
	else if (fake_gold < 0)
		str << span_color(font::BAD_COLOR);
	else
		end = "";
	str << fake_gold << end;
	return text_report(str.str());
}

REPORT_GENERATOR(villages, false, data)
{
	std::ostringstream str;
	const team &viewing_team = (*resources::teams)[data.viewing_side - 1];
	team_data td = calculate_team_data(viewing_team, data.viewing_side);
	str << td.villages << '/';
	if (viewing_team.uses_shroud()) {
		int unshrouded_villages = 0;
		foreach (const map_location &loc, resources::game_map->villages()) {
			if (!viewing_team.shrouded(loc))
				++unshrouded_villages;
		}
		str << unshrouded_villages;
	} else {
		str << resources::game_map->villages().size();
	}
	return gray_inactive(data, str.str());
}

REPORT_GENERATOR(num_units, false, data)
{
	return gray_inactive(data, str_cast(side_units(data.viewing_side)));
}

REPORT_GENERATOR(upkeep, false, data)
{
	std::ostringstream str;
	const team &viewing_team = (*resources::teams)[data.viewing_side - 1];
	team_data td = calculate_team_data(viewing_team, data.viewing_side);
	str << td.expenses << " (" << td.upkeep << ")";
	return gray_inactive(data, str.str());
}

REPORT_GENERATOR(expenses, false, data)
{
	const team &viewing_team = (*resources::teams)[data.viewing_side - 1];
	team_data td = calculate_team_data(viewing_team, data.viewing_side);
	return gray_inactive(data, str_cast(td.expenses));
}

REPORT_GENERATOR(income, false, data)
{
	std::ostringstream str;
	const team &viewing_team = (*resources::teams)[data.viewing_side - 1];
	team_data td = calculate_team_data(viewing_team, data.viewing_side);
	char const *end = naps;
	if (data.viewing_side != data.active_side)
		str << span_color(font::GRAY_COLOR);
	else if (td.net_income < 0)
		str << span_color(font::BAD_COLOR);
	else
		end = "";
	str << td.net_income << end;
	return text_report(str.str());
}

REPORT_GENERATOR(terrain, false, data)
{
	gamemap &map = *resources::game_map;
	const team &viewing_team = (*resources::teams)[data.viewing_side - 1];
	map_location mouseover_hex = resources::screen->mouseover_hex();
	if (!map.on_board(mouseover_hex) || viewing_team.shrouded(mouseover_hex))
		return report();

	t_translation::t_terrain terrain = map.get_terrain(mouseover_hex);
	if (terrain == t_translation::OFF_MAP_USER)
		return report();

	std::ostringstream str;
	if (map.is_village(mouseover_hex))
	{
		int owner = village_owner(mouseover_hex, *resources::teams) + 1;
		if (owner == 0 || viewing_team.fogged(mouseover_hex)) {
			str << map.get_terrain_info(terrain).income_description();
		} else if (owner == data.viewing_side) {
			str << map.get_terrain_info(terrain).income_description_own();
		} else if (viewing_team.is_enemy(owner)) {
			str << map.get_terrain_info(terrain).income_description_enemy();
		} else {
			str << map.get_terrain_info(terrain).income_description_ally();
		}
		str << ' ';
	} else {
		str << map.get_terrain_info(terrain).description();
	}

	const t_translation::t_list &underlying = map.underlying_union_terrain(terrain);
	if (underlying.size() != 1 || underlying.front() != terrain)
	{
		str << " (";
		for (t_translation::t_list::const_iterator i = underlying.begin(),
		     i_end = underlying.end(); i != i_end; ++i)
		{
			str << map.get_terrain_info(*i).name();
			if (i + 1 != underlying.end()) {
				str << ", ";
			}
		}
		str << ')';
	}
	return text_report(str.str());
}

REPORT_GENERATOR(position, false, data)
{
	gamemap &map = *resources::game_map;
	map_location mouseover_hex = resources::screen->mouseover_hex();
	if (!map.on_board(mouseover_hex))
		return report();
	t_translation::t_terrain terrain = map[mouseover_hex];
	if (terrain == t_translation::OFF_MAP_USER)
		return report();

	std::ostringstream str;
	str << mouseover_hex;

	const unit *u = get_visible_unit(data);
	const team &viewing_team = (*resources::teams)[data.viewing_side - 1];
	if (!u ||
	    (data.displayed_unit_hex != mouseover_hex &&
	     data.displayed_unit_hex != resources::screen->selected_hex()) ||
	    viewing_team.shrouded(mouseover_hex))
		return text_report(str.str());

	int move_cost = u->movement_cost(terrain);
	int defense = 100 - u->defense_modifier(terrain);
	if (move_cost < unit_movement_type::UNREACHABLE) {
		str << " (" << defense << "%," << move_cost << ')';
	} else if (mouseover_hex == data.displayed_unit_hex) {
		str << " (" << defense << "%,-)";
	} else {
		str << " (-)";
	}
	return text_report(str.str());
}

REPORT_GENERATOR(side_playing, false, data)
{
	const team &active_team = (*resources::teams)[data.active_side - 1];
	std::string flag_icon = active_team.flag_icon();
	std::string old_rgb = game_config::flag_rgb;
	std::string new_rgb = team::get_side_color_index(data.active_side);
	std::string mods = "~RC(" + old_rgb + ">" + new_rgb + ")";
	if (flag_icon.empty())
		flag_icon = game_config::images::flag_icon;
	return image_report(flag_icon + mods, active_team.current_player());
}

REPORT_GENERATOR(observers, false, data)
{
	if (data.observers.empty())
		return report();

	std::ostringstream str;
	str << _("Observers:") << '\n';
	foreach (const std::string &obs, data.observers) {
		str << obs << '\n';
	}
	return image_report(game_config::images::observer, str.str());
}

#ifdef DISABLE_EDITOR
REPORT_GENERATOR(selected_terrain, false, data)
{
	return report();
}

REPORT_GENERATOR(edit_left_button_function, false, data)
{
	return report();
}
#else
namespace editor {
extern std::string selected_terrain, left_button_function;
}

REPORT_GENERATOR(selected_terrain, false, /*data*/)
{
	if (editor::selected_terrain.empty())
		return report();
	else
		return text_report(editor::selected_terrain);
}

REPORT_GENERATOR(edit_left_button_function, false, /*data*/)
{
	if (editor::left_button_function.empty())
		return report();
	else
		return text_report(editor::left_button_function);
}
#endif

REPORT_GENERATOR(editor_tool_hint, false, /*data*/)
{
	return report();
}

REPORT_GENERATOR(report_clock, false, /*data*/)
{
	time_t t = std::time(NULL);
	struct tm *lt = std::localtime(&t);
	if (!lt) return report();
	char temp[10];
	size_t s = std::strftime(temp, 10, preferences::clock_format().c_str(), lt);
	if (!s) return report();
	return text_report(temp);
}

REPORT_GENERATOR(report_countdown, false, data)
{
	const team &viewing_team = (*resources::teams)[data.viewing_side - 1];
	int min, sec;
	if (viewing_team.countdown_time() == 0)
		return report_report_clock(data);
	std::ostringstream str;
	sec = viewing_team.countdown_time() / 1000;
	char const *end = naps;
	if (data.viewing_side != data.active_side)
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
	return text_report(str.str());
}

void reports::reset_generators()
{
	foreach (dynamic_report_generators::value_type &rg, dynamic_generators) {
		delete rg.second.generator;
	}
	dynamic_generators.clear();
}

void reports::register_generator(const std::string &name, reports::generator *g, bool for_units)
{
	dynamic_report_generator rg = { g, for_units };
	std::pair<dynamic_report_generators::iterator, bool> ib =
		dynamic_generators.insert(std::make_pair(name, rg));
	if (!ib.second) {
		delete ib.first->second.generator;
		ib.first->second = rg;
	}
}

config reports::generate_report(const std::string &name, const report_data &data)
{
	dynamic_report_generators::const_iterator i = dynamic_generators.find(name);
	if (i != dynamic_generators.end())
		return i->second.generator->generate(data);
	static_report_generators::const_iterator j = static_generators.find(name);
	if (j != static_generators.end())
		return j->second.generator(data);
	return report();
}

static std::set<std::string> unit_reports, status_reports;

const std::set<std::string> &reports::report_list(bool for_units)
{
	std::set<std::string> &l = *(for_units ? &unit_reports : &status_reports);
	if (!l.empty()) return l;
	foreach (const static_report_generators::value_type &v, static_generators) {
		if (v.second.for_units == for_units) l.insert(v.first);
	}
	foreach (const dynamic_report_generators::value_type &v, dynamic_generators) {
		if (v.second.for_units == for_units) l.insert(v.first);
	}
	return l;
}
