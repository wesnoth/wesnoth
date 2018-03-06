/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "help/help_topic_generators.hpp"

#include "font/sdl_ttf.hpp"             // for line_width
#include "game_config.hpp"              // for debug, menu_contract, etc
#include "preferences/game.hpp"         // for encountered_terrains, etc
#include "gettext.hpp"                  // for _, gettext, N_
#include "language.hpp"                 // for string_table, symbol_table
#include "log.hpp"                      // for LOG_STREAM, logger, etc
#include "movetype.hpp"                 // for movetype, movetype::effects, etc
#include "units/race.hpp"               // for unit_race, etc
#include "terrain/terrain.hpp"          // for terrain_type
#include "terrain/translation.hpp"      // for operator==, ter_list, etc
#include "terrain/type_data.hpp"        // for terrain_type_data, etc
#include "tstring.hpp"                  // for t_string, operator<<
#include "units/helper.hpp"             // for resistance_color
#include "units/types.hpp"              // for unit_type, unit_type_data, etc
#include "video.hpp"                    // fore current_resolution

#include <boost/optional.hpp>  // for optional
#include <iostream>                     // for operator<<, basic_ostream, etc
#include <map>                          // for map, etc
#include <set>
#include <SDL.h>

static lg::log_domain log_help("help");
#define WRN_HP LOG_STREAM(warn, log_help)
#define DBG_HP LOG_STREAM(debug, log_help)

namespace help {

static std::string best_str(bool best) {
	std::string lang_policy = (best ? _("Best of") : _("Worst of"));
	std::string color_policy = (best ? "green": "red");

	return "<format>color='" + color_policy + "' text='" + lang_policy + "'</format>";
}

typedef t_translation::ter_list::const_iterator ter_iter;
// Gets an english description of a terrain ter_list alias behavior: "Best of cave, hills", "Worst of Swamp, Forest" etc.
static std::string print_behavior_description(ter_iter start, ter_iter end, const ter_data_cache & tdata, bool first_level = true, bool begin_best = true)
{

	if (start == end) return "";
	if (*start == t_translation::MINUS || *start == t_translation::PLUS) return print_behavior_description(start+1, end, tdata, first_level, *start == t_translation::PLUS); //absorb any leading mode changes by calling again, with a new default value begin_best.

	boost::optional<ter_iter> last_change_pos;

	bool best = begin_best;
	for (ter_iter i = start; i != end; ++i) {
		if ((best && *i == t_translation::MINUS) || (!best && *i == t_translation::PLUS)) {
			best = !best;
			last_change_pos = i;
		}
	}

	std::stringstream ss;

	if (!last_change_pos) {
		std::vector<std::string> names;
		for (ter_iter i = start; i != end; ++i) {
			const terrain_type tt = tdata->get_terrain_info(*i);
			if (!tt.editor_name().empty())
				names.push_back(tt.editor_name());
		}

		if (names.empty()) return "";
		if (names.size() == 1) return names.at(0);

		ss << best_str(best) << " ";
		if (!first_level) ss << "( ";
		ss << names.at(0);

		for (size_t i = 1; i < names.size(); i++) {
			ss << ", " << names.at(i);
		}

		if (!first_level) ss << " )";
	} else {
		std::vector<std::string> names;
		for (ter_iter i = *last_change_pos+1; i != end; ++i) {
			const terrain_type tt = tdata->get_terrain_info(*i);
			if (!tt.editor_name().empty())
				names.push_back(tt.editor_name());
		}

		if (names.empty()) { //This alias list is apparently padded with junk at the end, so truncate it without adding more parens
			return print_behavior_description(start, *last_change_pos, tdata, first_level, begin_best);
		}

		ss << best_str(best) << " ";
		if (!first_level) ss << "( ";
		ss << print_behavior_description(start, *last_change_pos-1, tdata, false, begin_best);
		// Printed the (parenthesized) leading part from before the change, now print the remaining names in this group.
		for (const std::string & s : names) {
			ss << ", " << s;
		}
		if (!first_level) ss << " )";
	}
	return ss.str();
}

std::string terrain_topic_generator::operator()() const {
	std::stringstream ss;

	if (!type_.icon_image().empty())
		ss << "<img>src='images/buttons/icon-base-32.png~RC(magenta>" << type_.id()
			<< ")~BLIT("<< "terrain/" << type_.icon_image() << "_30.png)" << "'</img> ";

	if (!type_.editor_image().empty())
		ss << "<img>src='" << type_.editor_image() << "'</img> ";

	if (!type_.help_topic_text().empty())
		ss << "\n\n" << type_.help_topic_text().str() << "\n";
	else
		ss << "\n";

	ter_data_cache tdata = load_terrain_types_data();

	if (!tdata) {
		WRN_HP << "When building terrain help topics, we couldn't acquire any terrain types data\n";
		return ss.str();
	}

	if (!(type_.union_type().size() == 1 && type_.union_type()[0] == type_.number() && type_.is_nonnull())) {

		const t_translation::ter_list& underlying_mvt_terrains = tdata->underlying_mvt_terrain(type_.number());

		ss << "\n" << _("Base Terrain: ");

		bool first = true;
		for (const t_translation::terrain_code& underlying_terrain : underlying_mvt_terrains) {
			const terrain_type& mvt_base = tdata->get_terrain_info(underlying_terrain);

			if (mvt_base.editor_name().empty()) continue;

			if (!first) {
				ss << ", ";
			} else {
				first = false;
			}

			ss << make_link(mvt_base.editor_name(), ".." + terrain_prefix + mvt_base.id());
		}

		ss << "\n";

		ss << "\n" << _("Movement properties: ");
		ss << print_behavior_description(underlying_mvt_terrains.begin(), underlying_mvt_terrains.end(), tdata) << "\n";

		const t_translation::ter_list& underlying_def_terrains = tdata->underlying_def_terrain(type_.number());
		ss << "\n" << _("Defense properties: ");
		ss << print_behavior_description(underlying_def_terrains.begin(), underlying_def_terrains.end(), tdata) << "\n";
	}

	if (game_config::debug) {

		ss << "\n";
		ss << "ID: "          << type_.id() << "\n";

		ss << "Village: "     << (type_.is_village()   ? "Yes" : "No") << "\n";
		ss << "Gives Healing: " << type_.gives_healing() << "\n";

		ss << "Keep: "        << (type_.is_keep()      ? "Yes" : "No") << "\n";
		ss << "Castle: "      << (type_.is_castle()    ? "Yes" : "No") << "\n";

		ss << "Overlay: "     << (type_.is_overlay()   ? "Yes" : "No") << "\n";
		ss << "Combined: "    << (type_.is_combined()  ? "Yes" : "No") << "\n";
		ss << "Nonnull: "     << (type_.is_nonnull()   ? "Yes" : "No") << "\n";

		ss << "Terrain string:"  << type_.number() << "\n";

		ss << "Hide in Editor: " << (type_.hide_in_editor() ? "Yes" : "No") << "\n";
		ss << "Hide Help: "      << (type_.hide_help() ? "Yes" : "No") << "\n";
		ss << "Editor Group: "   << type_.editor_group() << "\n";

		ss << "Light Bonus: "   << type_.light_bonus(0) << "\n";

		ss << type_.income_description();

		if (type_.editor_image().empty()) { // Note: this is purely temporary to help make a different help entry
			ss << "\nEditor Image: Empty\n";
		} else {
			ss << "\nEditor Image: " << type_.editor_image() << "\n";
		}

		const t_translation::ter_list& underlying_mvt_terrains = tdata->underlying_mvt_terrain(type_.number());
		ss << "\nDebug Mvt Description String:";
		for (const t_translation::terrain_code & t : underlying_mvt_terrains) {
			ss << " " << t;
		}

		const t_translation::ter_list& underlying_def_terrains = tdata->underlying_def_terrain(type_.number());
		ss << "\nDebug Def Description String:";
		for (const t_translation::terrain_code & t : underlying_def_terrains) {
			ss << " " << t;
		}

	}

	return ss.str();
}


//Typedef to help with formatting list of traits
typedef std::pair<std::string, std::string> trait_data;

//Helper function for printing a list of trait data
static void print_trait_list(std::stringstream & ss, const std::vector<trait_data> & l)
{
	size_t i = 0;
	ss << make_link(l[i].first, l[i].second);

	// This doesn't skip traits with empty names
	for(i++; i < l.size(); i++) {
		ss << ", " << make_link(l[i].first,l[i].second);
	}
}

std::string unit_topic_generator::operator()() const {
	// Force the lazy loading to build this unit.
	unit_types.build_unit_type(type_, unit_type::FULL);

	std::stringstream ss;
	std::string clear_stringstream;
	const std::string detailed_description = type_.unit_description();
	const unit_type& female_type = type_.get_gender_unit_type(unit_race::FEMALE);
	const unit_type& male_type = type_.get_gender_unit_type(unit_race::MALE);

	const int screen_width = CVideo::get_singleton().get_width();

	ss << "Level " << type_.level();
	ss << "\n\n";

	ss << "<img>src='" << male_type.image();
	ss << "~RC(" << male_type.flag_rgb() << ">red)";
	if (screen_width >= 1200) ss << "~XBRZ(2)";
	ss << "' box='no'</img> ";


	if (&female_type != &male_type) {
		ss << "<img>src='" << female_type.image();
		ss << "~RC(" << female_type.flag_rgb() << ">red)";
		if (screen_width >= 1200) ss << "~XBRZ(2)";
		ss << "' box='no'</img> ";
	}

	const std::string &male_portrait = male_type.small_profile().empty() ?
		male_type.big_profile() : male_type.small_profile();
	const std::string &female_portrait = female_type.small_profile().empty() ?
		female_type.big_profile() : female_type.small_profile();

	const bool has_male_portrait = !male_portrait.empty() && male_portrait != male_type.image() && male_portrait != "unit_image";
	const bool has_female_portrait = !female_portrait.empty() && female_portrait != male_portrait && female_portrait != female_type.image() && female_portrait != "unit_image";

	int sz = (has_male_portrait && has_female_portrait ? 300 : 400);
	if (screen_width <= 1366) {
		sz = (has_male_portrait && has_female_portrait ? 200 : 300);
	} else if (screen_width >= 1920) {
		sz = 400;
	}

	// TODO: figure out why the second checks don't match but the last does
	if (has_male_portrait) {
		ss << "<img>src='" << male_portrait << "~FL(horiz)~SCALE_INTO(" << sz << ',' << sz << ")' box='no' align='right' float='yes'</img> ";
	}


	if (has_female_portrait) {
		ss << "<img>src='" << female_portrait << "~FL(horiz)~SCALE_INTO(" << sz << ',' << sz << ")' box='no' align='right' float='yes'</img> ";
	}

	ss << "\n\n\n";

	// Print cross-references to units that this unit advances from/to.
	// Cross reference to the topics containing information about those units.
	const bool first_reverse_value = true;
	bool reverse = first_reverse_value;
	if (variation_.empty()) {
		do {
			std::vector<std::string> adv_units =
				reverse ? type_.advances_from() : type_.advances_to();
			bool first = true;

			for (const std::string &adv : adv_units) {
				const unit_type *type = unit_types.find(adv, unit_type::HELP_INDEXED);
				if (!type || type->hide_help()) {
					continue;
				}

				if (first) {
					if (reverse) {
						ss << _("Advances from: ");
					} else {
						ss << _("Advances to: ");
					}
					first = false;
				} else {
					ss << ", ";
				}

				std::string lang_unit = type->type_name();
				std::string ref_id;
				if (description_type(*type) == FULL_DESCRIPTION) {
					const std::string section_prefix = type->show_variations_in_help() ? ".." : "";
					ref_id = section_prefix + unit_prefix + type->id();
				} else {
					ref_id = unknown_unit_topic;
					lang_unit += " (?)";
				}
				ss << make_link(lang_unit, ref_id);
			}
			if (!first) {
				ss << "\n";
			}

			reverse = !reverse; //switch direction
		} while(reverse != first_reverse_value); // don't restart
	}

	const unit_type* parent = variation_.empty() ? &type_ :
		unit_types.find(type_.id(), unit_type::HELP_INDEXED);
	if (!variation_.empty()) {
		ss << _("Base unit: ") << make_link(parent->type_name(), ".." + unit_prefix + type_.id()) << "\n";
	} else {
		bool first = true;
		for (const std::string& base_id : utils::split(type_.get_cfg()["base_ids"])) {
			if (first) {
				ss << _("Base units: ");
				first = false;
			}
			const unit_type* base_type = unit_types.find(base_id, unit_type::HELP_INDEXED);
			const std::string section_prefix = base_type->show_variations_in_help() ? ".." : "";
			ss << make_link(base_type->type_name(), section_prefix + unit_prefix + base_id) << "\n";
		}
	}

	bool first = true;
	for (const std::string &var_id : parent->variations()) {
		const unit_type &type = parent->get_variation(var_id);

		if(type.hide_help()) {
			continue;
		}

		if (first) {
			ss << _("Variations: ");
			first = false;
		} else {
			ss << ", ";
		}

		std::string ref_id;

		std::string var_name = type.variation_name();
		if (description_type(type) == FULL_DESCRIPTION) {
			ref_id = variation_prefix + type.id() + "_" + var_id;
		} else {
			ref_id = unknown_unit_topic;
			var_name += " (?)";
		}

		ss << make_link(var_name, ref_id);
	}
	ss << "\n"; //added even if empty, to avoid shifting

	// Print the race of the unit, cross-reference it to the respective topic.
	const std::string race_id = type_.race_id();
	std::string race_name = type_.race()->plural_name();
	if (race_name.empty()) {
		race_name = _ ("race^Miscellaneous");
	}
	ss << _("Race: ");
	ss << make_link(race_name, "..race_" + race_id);
	ss << "\n\n";

	// Print the possible traits of the unit, cross-reference them
	// to their respective topics.
	if (config::const_child_itors traits = type_.possible_traits()) {
		std::vector<trait_data> must_have_traits;
		std::vector<trait_data> random_traits;
		int must_have_nameless_traits = 0;

		for (const config & trait : traits) {
			const std::string trait_name = trait["male_name"];
			std::string lang_trait_name = translation::gettext(trait_name.c_str());
			if (lang_trait_name.empty() && trait["availability"].str() == "musthave") {
				++must_have_nameless_traits;
				continue;
			}
			const std::string ref_id = "traits_"+trait["id"].str();
			((trait["availability"].str() == "musthave") ? must_have_traits : random_traits).emplace_back(lang_trait_name, ref_id);
		}

		bool line1 = !must_have_traits.empty();
		bool line2 = !random_traits.empty() && type_.num_traits() > must_have_traits.size();

		if (line1) {
			std::string traits_label = _("Traits");
			ss << traits_label;
			if (line2) {
				std::stringstream must_have_count;
				must_have_count << " (" << must_have_traits.size() << ") : ";
				std::stringstream random_count;
				random_count << " (" << (type_.num_traits() - must_have_traits.size() - must_have_nameless_traits) << ") : ";

				int second_line_whitespace = font::line_width(traits_label+must_have_count.str(), normal_font_size)
					- font::line_width(random_count.str(), normal_font_size);
				// This ensures that the second line is justified so that the ':' characters are aligned.

				ss << must_have_count.str();
				print_trait_list(ss, must_have_traits);
				ss << "\n" << jump(second_line_whitespace) << random_count.str();
				print_trait_list(ss, random_traits);
			} else {
				ss << ": ";
				print_trait_list(ss, must_have_traits);
			}
			ss << "\n\n";
		} else {
			if (line2) {
				ss << _("Traits") << " (" << (type_.num_traits() - must_have_nameless_traits) << ") : ";
				print_trait_list(ss, random_traits);
				ss << "\n\n";
			}
		}
	}

	// Print the abilities the units has, cross-reference them
	// to their respective topics. TODO: Update this according to musthave trait effects, similar to movetype below
	if(!type_.abilities_metadata().empty()) {
		ss << _("Abilities: ");

		for(auto iter = type_.abilities_metadata().begin(); iter != type_.abilities_metadata().end(); ++iter) {
			const std::string ref_id = ability_prefix + iter->id + iter->name.base_str();

			std::string lang_ability = translation::gettext(iter->name.c_str());
			ss << make_link(lang_ability, ref_id);

			if(std::next(iter) != type_.abilities_metadata().end()) {
				ss << ", ";
			}
		}

		ss << "\n\n";
	}

	// Print the extra AMLA upgrade abilities, cross-reference them to their respective topics.
	if(!type_.adv_abilities_metadata().empty()) {
		ss << _("Ability Upgrades: ");

		for(auto iter = type_.adv_abilities_metadata().begin(); iter != type_.adv_abilities_metadata().end(); ++iter) {
			const std::string ref_id = ability_prefix + iter->id + iter->name.base_str();

			std::string lang_ability = translation::gettext(iter->name.c_str());
			ss << make_link(lang_ability, ref_id);

			if(std::next(iter) != type_.adv_abilities_metadata().end()) {
				ss << ", ";
			}
		}

		ss << "\n\n";
	}

	// Print some basic information such as HP and movement points.
	// TODO: Make this info update according to musthave traits, similar to movetype below.
	ss << _("HP: ") << type_.hitpoints() << jump(30)
		<< _("Moves: ") << type_.movement() << jump(30);
	if (type_.vision() != type_.movement()) {
		ss << _("Vision: ") << type_.vision() << jump(30);
	}
	if (type_.jamming() > 0) {
		ss << _("Jamming: ") << type_.jamming() << jump(30);
	}
	ss << _("Cost: ") << type_.cost() << jump(30)
		<< _("Alignment: ")
		<< make_link(type_.alignment_description(type_.alignment(), type_.genders().front()), "time_of_day")
		<< jump(30);
	if (type_.can_advance()) {
		ss << _("Required XP: ") << type_.experience_needed();
	}

	// Print the detailed description about the unit.
		ss << "\n\n" << detailed_description;

	// Print the different attacks a unit has, if it has any.
	if (!type_.attacks().empty()) {
		// Print headers for the table.
		ss << "\n\n<header>text='" << escape(_("unit help^Attacks"))
			<< "'</header>\n\n";
		table_spec table;

		std::vector<item> first_row;
		// Dummy element, icons are below.
		first_row.push_back(item("", 0));
		push_header(first_row, _("unit help^Name"));
		push_header(first_row, _("Type"));
		push_header(first_row, _("Strikes"));
		push_header(first_row, _("Range"));
		push_header(first_row, _("Special"));
		table.push_back(first_row);
		// Print information about every attack.
		for(const attack_type& attack : type_.attacks()) {
			std::string lang_weapon = attack.name();
			std::string lang_type = string_table["type_" + attack.type()];
			std::vector<item> row;
			std::stringstream attack_ss;
			attack_ss << "<img>src='" << attack.icon() << "'</img>";
			row.emplace_back(attack_ss.str(),image_width(attack.icon()));
			push_tab_pair(row, lang_weapon);
			push_tab_pair(row, lang_type);
			attack_ss.str(clear_stringstream);
			attack_ss << attack.damage() << font::unicode_en_dash << attack.num_attacks()
				<< " " << attack.accuracy_parry_description();
			push_tab_pair(row, attack_ss.str());
			attack_ss.str(clear_stringstream);
			if (attack.min_range() > 1 || attack.max_range() > 1) {
				attack_ss << attack.min_range() << "-" << attack.max_range() << ' ';
			}
			attack_ss << string_table["range_" + attack.range()];
			push_tab_pair(row, attack_ss.str());
			attack_ss.str(clear_stringstream);
			// Show this attack's special, if it has any. Cross
			// reference it to the section describing the special.
			std::vector<std::pair<t_string, t_string>> specials = attack.special_tooltips();
			if (!specials.empty()) {
				std::string lang_special = "";
				const size_t specials_size = specials.size();
				for (size_t i = 0; i != specials_size; ++i) {
					const std::string ref_id = std::string("weaponspecial_")
						+ specials[i].first.base_str();
					lang_special = (specials[i].first);
					attack_ss << make_link(lang_special, ref_id);
					if (i+1 != specials_size) {
						attack_ss << ", "; //comma placed before next special
					}
				}
				row.emplace_back(attack_ss.str(), font::line_width(lang_special, normal_font_size));
			}
			table.push_back(row);
		}
		ss << generate_table(table);
	}

	// Generate the movement type of the unit, with resistance, defense, movement, jamming and vision data updated according to any 'musthave' traits which always apply
	movetype movement_type = type_.movement_type();
	config::const_child_itors traits = type_.possible_traits();
	if (!traits.empty() && type_.num_traits() > 0) {
		for (const config & t : traits) {
			if (t["availability"].str() == "musthave") {
				for (const config & effect : t.child_range("effect")) {
					if (!effect.child("filter") // If this is musthave but has a unit filter, it might not always apply, so don't apply it in the help.
							&& movetype::effects.find(effect["apply_to"].str()) != movetype::effects.end()) {
						movement_type.merge(effect, effect["replace"].to_bool());
					}
				}
			}
		}
	}

	// Print the resistance table of the unit.
	ss << "\n\n<header>text='" << escape(_("Resistances"))
		<< "'</header>\n\n";
	table_spec resistance_table;
	std::vector<item> first_res_row;
	push_header(first_res_row, _("Attack Type"));
	push_header(first_res_row, _("Resistance"));
	resistance_table.push_back(first_res_row);
	utils::string_map dam_tab = movement_type.damage_table();
	for(std::pair<std::string, std::string> dam_it : dam_tab) {
		std::vector<item> row;
		int resistance = 100;
		try {
			resistance -= std::stoi(dam_it.second);
		} catch(std::invalid_argument&) {}
		std::string resist = std::to_string(resistance) + '%';
		const size_t pos = resist.find('-');
		if (pos != std::string::npos) {
			resist.replace(pos, 1, font::unicode_minus);
		}
		std::string color = unit_helper::resistance_color(resistance);
		std::string lang_weapon = string_table["type_" + dam_it.first];
		push_tab_pair(row, lang_weapon);
		std::stringstream str;
		str << "<format>color=\"" << color << "\" text='"<< resist << "'</format>";
		const std::string markup = str.str();
		str.str(clear_stringstream);
		str << resist;
		row.emplace_back(markup, font::line_width(str.str(), normal_font_size));
		resistance_table.push_back(row);
	}
	ss << generate_table(resistance_table);

	if (ter_data_cache tdata = load_terrain_types_data()) {
		// Print the terrain modifier table of the unit.
		ss << "\n\n<header>text='" << escape(_("Terrain Modifiers"))
			<< "'</header>\n\n";
		std::vector<item> first_row;
		table_spec table;
		push_header(first_row, _("Terrain"));
		push_header(first_row, _("Defense"));
		push_header(first_row, _("Movement Cost"));

		const bool has_terrain_defense_caps = movement_type.has_terrain_defense_caps(preferences::encountered_terrains());
		if (has_terrain_defense_caps) {
			push_header(first_row, _("Defense Cap"));
		}

		const bool has_vision = type_.movement_type().has_vision_data();
		if (has_vision) {
			push_header(first_row, _("Vision Cost"));
		}
		const bool has_jamming = type_.movement_type().has_jamming_data();
		if (has_jamming) {
			push_header(first_row, _("Jamming Cost"));
		}

		table.push_back(first_row);
		std::set<t_translation::terrain_code>::const_iterator terrain_it =
			preferences::encountered_terrains().begin();

		for (; terrain_it != preferences::encountered_terrains().end();
			 	++terrain_it) {
			const t_translation::terrain_code terrain = *terrain_it;
			if (terrain == t_translation::FOGGED || terrain == t_translation::VOID_TERRAIN || t_translation::terrain_matches(terrain, t_translation::ALL_OFF_MAP)) {
				continue;
			}
			const terrain_type& info = tdata->get_terrain_info(terrain);
			const int moves = movement_type.movement_cost(terrain);
			const bool cannot_move = moves > type_.movement();
			if (cannot_move && info.hide_if_impassable()) {
				continue;
			}

			if (info.union_type().size() == 1 && info.union_type()[0] == info.number() && info.is_nonnull()) {
				std::vector<item> row;
				const std::string& name = info.name();
				const std::string& id = info.id();
				const int views = movement_type.vision_cost(terrain);
				const int jams  = movement_type.jamming_cost(terrain);

				bool high_res = false;
				const std::string tc_base = high_res ? "images/buttons/icon-base-32.png" : "images/buttons/icon-base-16.png";
				const std::string terrain_image = "icons/terrain/terrain_type_" + id + (high_res ? "_30.png" : ".png");

				const std::string final_image = tc_base + "~RC(magenta>" + id + ")~BLIT(" + terrain_image + ")";

				row.emplace_back("<img>src='" + final_image + "'</img> " +
						make_link(name, "..terrain_" + id),
					font::line_width(name, normal_font_size) + (high_res ? 32 : 16) );

				//defense  -  range: +10 % .. +70 %
				const int defense = 100 - movement_type.defense_modifier(terrain);
				std::string color;
				if (defense <= 10) {
					color = "red";
				} else if (defense <= 30) {
					color = "yellow";
				} else if (defense <= 50) {
					color = "white";
				} else {
					color = "green";
				}

				std::stringstream str;
				str << "<format>color=" << color << " text='"<< defense << "%'</format>";
				std::string markup = str.str();
				str.str(clear_stringstream);
				str << defense << "%";
				row.emplace_back(markup, font::line_width(str.str(), normal_font_size));

				//movement  -  range: 1 .. 5, movetype::UNREACHABLE=impassable
				str.str(clear_stringstream);
				if (cannot_move) {		// cannot move in this terrain
					color = "red";
				} else if (moves > 1) {
					color = "yellow";
				} else {
					color = "white";
				}
				str << "<format>color=" << color << " text='";
				// A 5 MP margin; if the movement costs go above
				// the unit's max moves + 5, we replace it with dashes.
				if(cannot_move && (moves > type_.movement() + 5)) {
					str << font::unicode_figure_dash;
				} else {
					str << moves;
				}
				str << "'</format>";
				markup = str.str();
				str.str(clear_stringstream);
				str << moves;
				row.emplace_back(markup, font::line_width(str.str(), normal_font_size));

				//defense cap
				if (has_terrain_defense_caps) {
					str.str(clear_stringstream);
					const bool has_cap = movement_type.get_defense().capped(terrain);
					if (has_cap) {
						str << "<format>color='"<< color <<"' text='" << defense << "%'</format>";
					} else {
						str << "<format>color=white text='" << font::unicode_figure_dash << "'</format>";
					}
					markup = str.str();
					str.str(clear_stringstream);
					if (has_cap) {
						str << defense << '%';
					} else {
						str << font::unicode_figure_dash;
					}
					row.emplace_back(markup, font::line_width(str.str(), normal_font_size));
				}

				//vision
					if (has_vision) {
						str.str(clear_stringstream);
						const bool cannot_view = views > type_.vision();
						if (cannot_view) {		// cannot view in this terrain
							color = "red";
						} else if (views > moves) {
							color = "yellow";
						} else if (views == moves) {
							color = "white";
						} else {
							color = "green";
						}
						str << "<format>color=" << color << " text='";
						// A 5 MP margin; if the vision costs go above
						// the unit's vision + 5, we replace it with dashes.
						if(cannot_view && (views > type_.vision() + 5)) {
							str << font::unicode_figure_dash;
						} else {
							str << views;
						}
						str << "'</format>";
						markup = str.str();
						str.str(clear_stringstream);
						str << views;
						row.emplace_back(markup, font::line_width(str.str(), normal_font_size));
					}

				//jamming
				if (has_jamming) {
					str.str(clear_stringstream);
					const bool cannot_jam = jams > type_.jamming();
					if (cannot_jam) {		// cannot jamm in this terrain
						color = "red";
					} else if (jams > views) {
						color = "yellow";
					} else if (jams == views) {
						color = "white";
					} else {
						color = "green";
					}
					str << "<format>color=" << color << " text='";
					// A 5 MP margin; if the jamming costs go above
					// the unit's jamming + 5, we replace it with dashes.
					if ( cannot_jam  &&  jams > type_.jamming() + 5 ) {
						str << font::unicode_figure_dash;
					} else {
						str << jams;
					}
					str << "'</format>";

					push_tab_pair(row, str.str());
				}

				table.push_back(row);
			}
		}

		ss << generate_table(table);
	} else {
		WRN_HP << "When building unit help topics, the display object was null and we couldn't get the terrain info we need.\n";
	}
	return ss.str();
}

void unit_topic_generator::push_header(std::vector< item > &row,  const std::string& name) const {
	row.emplace_back(bold(name), font::line_width(name, normal_font_size, TTF_STYLE_BOLD));
}

} // end namespace help
