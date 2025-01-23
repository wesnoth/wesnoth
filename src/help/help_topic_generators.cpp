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

#define GETTEXT_DOMAIN "wesnoth-help"

#include "help/help_topic_generators.hpp"

#include "formula/string_utils.hpp"     // for VNGETTEXT
#include "game_config.hpp"              // for debug, menu_contract, etc
#include "gettext.hpp"                  // for _, gettext, N_
#include "language.hpp"                 // for string_table, symbol_table
#include "log.hpp"                      // for LOG_STREAM, logger, etc
#include "movetype.hpp"                 // for movetype, movetype::effects, etc
#include "preferences/preferences.hpp"  // for encountered_terrains, etc
#include "units/race.hpp"               // for unit_race, etc
#include "serialization/markup.hpp"     // for markup related utilities
#include "terrain/terrain.hpp"          // for terrain_type
#include "terrain/translation.hpp"      // for operator==, ter_list, etc
#include "terrain/type_data.hpp"        // for terrain_type_data, etc
#include "tstring.hpp"                  // for t_string, operator<<
#include "units/helper.hpp"             // for resistance_color
#include "units/types.hpp"              // for unit_type, unit_type_data, etc
#include "utils/optional_fwd.hpp"
#include "video.hpp"                    // fore current_resolution

#include <set>

static lg::log_domain log_help("help");
#define WRN_HP LOG_STREAM(warn, log_help)
#define DBG_HP LOG_STREAM(debug, log_help)

namespace help {

struct terrain_movement_info
{
	const t_string name;
	const std::string id;
	const int defense;
	const int movement_cost;
	const int vision_cost;
	const int jamming_cost;
	const bool defense_cap;

	bool operator<(const terrain_movement_info& other) const
	{
		return translation::icompare(name, other.name) < 0;
	}
};

static std::string best_str(bool best) {
	std::string lang_policy = (best ? _("Best of") : _("Worst of"));
	std::string color_policy = (best ? "green": "red");

	return markup::span_color(color_policy, lang_policy);
}

static std::string format_mp_entry(const int cost, const int max_cost) {
	std::stringstream str_unformatted;
	const bool cannot = cost < max_cost;

	// passing true to select the less saturated red-to-green scale
	color_t color = game_config::red_to_green(100.0 - 25.0 * max_cost, true);

	// A 5 point margin; if the costs go above
	// the unit's mp cost + 5, we replace it with dashes.
	if (cannot && max_cost > cost + 5) {
		str_unformatted << font::unicode_figure_dash;
	} else if(cannot) {
		str_unformatted << "(" << max_cost << ")";
	} else {
		str_unformatted << max_cost;
	}
	if(max_cost != 0) {
		const int hexes_per_turn = cost / max_cost;
		str_unformatted << " ";
		for(int i = 0; i < hexes_per_turn; ++i) {
			// Unicode horizontal black hexagon and Unicode zero width space (to allow a line break)
			str_unformatted << "\u2b23\u200b";
		}
	}

	return markup::span_color(color, str_unformatted.str());
}

typedef t_translation::ter_list::const_iterator ter_iter;
// Gets an english description of a terrain ter_list alias behavior: "Best of cave, hills", "Worst of Swamp, Forest" etc.
static std::string print_behavior_description(const ter_iter& start, const ter_iter& end, const std::shared_ptr<terrain_type_data> & tdata, bool first_level = true, bool begin_best = true)
{

	if (start == end) return "";
	if (*start == t_translation::MINUS || *start == t_translation::PLUS) return print_behavior_description(start+1, end, tdata, first_level, *start == t_translation::PLUS); //absorb any leading mode changes by calling again, with a new default value begin_best.

	utils::optional<ter_iter> last_change_pos;

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
			if (*i == t_translation::BASE) {
				// TRANSLATORS: in a description of an overlay terrain, the terrain that it's placed on
				names.push_back(_("base terrain"));
			} else {
				const terrain_type tt = tdata->get_terrain_info(*i);
				if (!tt.editor_name().empty())
					names.push_back(tt.editor_name());
			}
		}

		if (names.empty()) return "";
		if (names.size() == 1) return names.at(0);

		ss << best_str(best) << " ";
		if (!first_level) ss << "( ";
		ss << names.at(0);

		for (std::size_t i = 1; i < names.size(); i++) {
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

	if (!type_.icon_image().empty()) {
		ss << markup::img(formatter()
			<< "images/buttons/icon-base-32.png~RC(magenta>" << type_.id()
			<< ")~BLIT(" << "terrain/" << type_.icon_image() << "_30.png)");
	}

	if (!type_.editor_image().empty()) {
		ss << markup::img(type_.editor_image()) << markup::br;
	}

	if (!type_.help_topic_text().empty()) {
		ss << "\n" << type_.help_topic_text().str() << "\n";
	} else {
		ss << "\n";
	}

	std::shared_ptr<terrain_type_data> tdata = load_terrain_types_data();

	if (!tdata) {
		WRN_HP << "When building terrain help topics, we couldn't acquire any terrain types data";
		return ss.str();
	}

	// Special notes are generated from the terrain's properties - at the moment there's no way for WML authors
	// to add their own via a [special_note] tag.
	std::vector<std::string> special_notes;

	if(type_.is_village()) {
		special_notes.push_back(_("Villages allow any unit stationed therein to heal, or to be cured of poison."));
	} else if(type_.gives_healing() > 0) {
		auto symbols = utils::string_map{{"amount", std::to_string(type_.gives_healing())}};
		// TRANSLATORS: special note for terrains such as the oasis; the only terrain in core with this property heals 8 hp just like a village.
		// For the single-hitpoint variant, the wording is different because I assume the player will be more interested in the curing-poison part than the minimal healing.
		auto message = VNGETTEXT("This terrain allows units to be cured of poison, or to heal a single hitpoint.",
			"This terrain allows units to heal $amount hitpoints, or to be cured of poison, as if stationed in a village.",
			type_.gives_healing(), symbols);
		special_notes.push_back(std::move(message));
	}

	if(type_.is_castle()) {
		special_notes.push_back(_("This terrain is a castle — units can be recruited onto it from a connected keep."));
	}
	if(type_.is_keep() && type_.is_castle()) {
		// TRANSLATORS: The "this terrain is a castle" note will also be shown directly above this one.
		special_notes.push_back(_("This terrain is a keep — a leader can recruit from this hex onto connected castle hexes."));
	} else if(type_.is_keep() && !type_.is_castle()) {
		// TRANSLATORS: Special note for a terrain, but none of the terrains in mainline do this.
		special_notes.push_back(_("This unusual keep allows a leader to recruit while standing on it, but does not allow a leader on a connected keep to recruit onto this hex."));
	}

	if(!special_notes.empty()) {
		ss << "\n\n" << markup::tag("header", _("Special Notes")) << "\n\n";
		for(const auto& note : special_notes) {
			ss << font::unicode_bullet << " " << note << '\n';
		}
	}

	// Almost all terrains will show the data in this conditional block. The ones that don't are the
	// archetypes used in [movetype]'s subtags such as [movement_costs].
	if (!type_.is_indivisible()) {
		std::vector<t_string> underlying;
		for (const auto& underlying_terrain : type_.union_type()) {
			const terrain_type& base = tdata->get_terrain_info(underlying_terrain);
			if (!base.editor_name().empty()) {
				underlying.push_back(markup::make_link(base.editor_name(), ".." + terrain_prefix + base.id()));
			}
		}
		utils::string_map symbols;
		symbols["types"] = utils::format_conjunct_list("", underlying);
		// TRANSLATORS: $types is a conjunct list, typical values will be "Castle" or "Flat and Shallow Water".
		// The terrain names will be hypertext links to the help page of the corresponding terrain type.
		// There will always be at least 1 item in the list, but unlikely to be more than 3.
		ss << "\n" << VNGETTEXT("Basic terrain type: $types", "Basic terrain types: $types", underlying.size(), symbols);

		if (type_.has_default_base()) {
			const terrain_type& base = tdata->get_terrain_info(type_.default_base());

			symbols.clear();
			if (base.is_indivisible()) {
				symbols["type"] = markup::make_link(base.editor_name(), ".." + terrain_prefix + base.id());
			} else {
				symbols["type"] = markup::make_link(base.editor_name(), terrain_prefix + base.id());
			}
			// TRANSLATORS: In the help for a terrain type, for example Dwarven Village is often placed on Cave Floor
			ss << "\n" << VGETTEXT("Typical base terrain: $type", symbols);
		}

		ss << "\n";

		const t_translation::ter_list& underlying_mvt_terrains = type_.mvt_type();
		ss << "\n" << _("Movement properties: ");
		ss << print_behavior_description(underlying_mvt_terrains.begin(), underlying_mvt_terrains.end(), tdata) << "\n";

		const t_translation::ter_list& underlying_def_terrains = type_.def_type();
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

		ss << "Terrain string: " << type_.number() << "\n";

		ss << "Hide in Editor: " << (type_.hide_in_editor() ? "Yes" : "No") << "\n";
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
// Maps localized trait name to trait help topic ID
typedef std::pair<std::string, std::string> trait_data;

//Helper function for printing a list of trait data
static void print_trait_list(std::stringstream & ss, const std::vector<trait_data> & l)
{
	std::size_t i = 0;
	ss << markup::make_link(l[i].first, l[i].second);

	// This doesn't skip traits with empty names
	for(i++; i < l.size(); i++) {
		ss << ", " << markup::make_link(l[i].first,l[i].second);
	}
}

std::string unit_topic_generator::operator()() const {
	// Force the lazy loading to build this unit.
	unit_types.build_unit_type(type_, unit_type::FULL);

	std::stringstream ss;
	const std::string detailed_description = type_.unit_description();
	const unit_type& female_type = type_.get_gender_unit_type(unit_race::FEMALE);
	const unit_type& male_type = type_.get_gender_unit_type(unit_race::MALE);

	const int screen_width = video::game_canvas_size().x;

	ss << _("Level") << " " << type_.level();

	// Portraits
	const std::string &male_portrait = male_type.small_profile().empty() ?
		male_type.big_profile() : male_type.small_profile();
	const std::string &female_portrait = female_type.small_profile().empty() ?
		female_type.big_profile() : female_type.small_profile();

	const bool has_male_portrait = !male_portrait.empty() && male_portrait != male_type.image() && male_portrait != "unit_image";
	const bool has_female_portrait = !female_portrait.empty() && female_portrait != male_portrait && female_portrait != female_type.image() && female_portrait != "unit_image";

	// TODO: figure out why the second checks don't match but the last does
	if (has_male_portrait) {
		ss << markup::img(male_portrait + "~FL(horiz)", "right", true);
	}

	if (has_female_portrait) {
		ss << markup::img(female_portrait + "~FL(horiz)", "right", true);
	}

	// Unit Images
	ss << markup::img(formatter()
		<< male_type.image() << "~RC(" << male_type.flag_rgb() << ">red)"
		<< (screen_width >= 1200 ? "~SCALE_SHARP(200%,200%)" : ""));

	if (female_type.image() != male_type.image()) {
		ss << markup::img(formatter()
			<< female_type.image() << "~RC(" << female_type.flag_rgb() << ">red)"
			<< (screen_width >= 1200 ? "~SCALE_SHARP(200%,200%)" : ""));
	}

	ss << "\n";

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
				ss << markup::make_link(lang_unit, ref_id);
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
		ss << _("Base unit: ") << markup::make_link(parent->type_name(), ".." + unit_prefix + type_.id()) << "\n";
	} else {
		bool first = true;
		for (const std::string& base_id : utils::split(type_.get_cfg()["base_ids"])) {
			if (first) {
				ss << _("Base units: ");
				first = false;
			}
			const unit_type* base_type = unit_types.find(base_id, unit_type::HELP_INDEXED);
			const std::string section_prefix = base_type->show_variations_in_help() ? ".." : "";
			ss << markup::make_link(base_type->type_name(), section_prefix + unit_prefix + base_id) << "\n";
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

		ss << markup::make_link(var_name, ref_id);
	}

	// Print the race of the unit, cross-reference it to the respective topic.
	const std::string race_id = type_.race_id();
	std::string race_name = type_.race()->plural_name();
	if (race_name.empty()) {
		race_name = _ ("race^Miscellaneous");
	}
	ss << _("Race: ");
	ss << markup::make_link(race_name, "..race_" + race_id);
	ss << "\n";

	// Print the possible traits of the unit, cross-reference them
	// to their respective topics.
	if (config::const_child_itors traits = type_.possible_traits()) {
		std::vector<trait_data> must_have_traits;
		std::vector<trait_data> random_traits;
		int must_have_nameless_traits = 0;

		for(const config& trait : traits) {
			const std::string& male_name = trait["male_name"].str();
			const std::string& female_name = trait["female_name"].str();
			std::string trait_name;
			if (type_.has_gender_variation(unit_race::MALE) && ! male_name.empty())
				trait_name = male_name;
			else if (type_.has_gender_variation(unit_race::FEMALE) && ! female_name.empty())
				trait_name = female_name;
			else if (! trait["name"].str().empty())
				trait_name = trait["name"].str();
			else
				continue; // Hidden trait

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
				must_have_count << "\n (" << must_have_traits.size() << ") : ";
				std::stringstream random_count;
				random_count << " (" << (type_.num_traits() - must_have_traits.size() - must_have_nameless_traits) << ") : ";
				ss << must_have_count.str();
				print_trait_list(ss, must_have_traits);
				ss << "\n" << random_count.str();
				print_trait_list(ss, random_traits);
			} else {
				ss << ": ";
				print_trait_list(ss, must_have_traits);
			}
			ss << "\n";
		} else {
			if (line2) {
				ss << _("Traits") << " (" << (type_.num_traits() - must_have_nameless_traits) << ") : ";
				print_trait_list(ss, random_traits);
				ss << "\n";
			}
		}
	}

	// Print the abilities the units has, cross-reference them
	// to their respective topics. TODO: Update this according to musthave trait effects, similar to movetype below
	if(!type_.abilities_metadata().empty()) {
		ss << _("Abilities: ");

		bool start = true;

		for(auto iter = type_.abilities_metadata().begin(); iter != type_.abilities_metadata().end(); ++iter) {
			const std::string ref_id = ability_prefix + iter->id + iter->name.base_str();

			if(iter->name.empty()) {
				continue;
			}

			if(!start) {
				ss << ", ";
			} else {
				start = false;
			}

			std::string lang_ability = translation::gettext(iter->name.c_str());
			ss << markup::make_link(lang_ability, ref_id);
		}

		ss << "\n\n";
	}

	// Print the extra AMLA upgrade abilities, cross-reference them to their respective topics.
	if(!type_.adv_abilities_metadata().empty()) {
		ss << _("Ability Upgrades: ");

		bool start = true;

		for(auto iter = type_.adv_abilities_metadata().begin(); iter != type_.adv_abilities_metadata().end(); ++iter) {
			const std::string ref_id = ability_prefix + iter->id + iter->name.base_str();

			if(iter->name.empty()) {
				continue;
			}

			if(!start) {
				ss << ", ";
			} else {
				start = false;
			}

			std::string lang_ability = translation::gettext(iter->name.c_str());
			ss << markup::make_link(lang_ability, ref_id);
		}

		ss << "\n\n";
	}

	// Print some basic information such as HP and movement points.
	// TODO: Make this info update according to musthave traits, similar to movetype below.

	// TRANSLATORS: This string is used in the help page of a single unit.  If the translation
	// uses spaces, use non-breaking spaces as appropriate for the target language to prevent
	// unpleasant line breaks (issue #3256).
	ss << _("HP:") << font::nbsp << type_.hitpoints() << "  "
		// TRANSLATORS: This string is used in the help page of a single unit.  If the translation
		// uses spaces, use non-breaking spaces as appropriate for the target language to prevent
		// unpleasant line breaks (issue #3256).
		<< _("Moves:") << font::nbsp << type_.movement() << "  ";
	if (type_.vision() != type_.movement()) {
		// TRANSLATORS: This string is used in the help page of a single unit.  If the translation
		// uses spaces, use non-breaking spaces as appropriate for the target language to prevent
		// unpleasant line breaks (issue #3256).
		ss << _("Vision:") << font::nbsp << type_.vision() << "  ";
	}
	if (type_.jamming() > 0) {
		// TRANSLATORS: This string is used in the help page of a single unit.  If the translation
		// uses spaces, use non-breaking spaces as appropriate for the target language to prevent
		// unpleasant line breaks (issue #3256).
		ss << _("Jamming:") << font::nbsp << type_.jamming() << "  ";
	}
	// TRANSLATORS: This string is used in the help page of a single unit.  If the translation
	// uses spaces, use non-breaking spaces as appropriate for the target language to prevent
	// unpleasant line breaks (issue #3256).
	ss << _("Cost:") << font::nbsp << type_.cost() << "  "
		// TRANSLATORS: This string is used in the help page of a single unit.  If the translation
		// uses spaces, use non-breaking spaces as appropriate for the target language to prevent
		// unpleasant line breaks (issue #3256).
		<< _("Alignment:") << font::nbsp
		<< markup::make_link(type_.alignment_description(type_.alignment(), type_.genders().front()), "time_of_day")
		<< "  ";
	if (type_.can_advance() || type_.modification_advancements()) {
		// TRANSLATORS: This string is used in the help page of a single unit.  It uses
		// non-breaking spaces to prevent unpleasant line breaks (issue #3256).  In the
		// translation use non-breaking spaces as appropriate for the target language.
		ss << _("Required\u00a0XP:") << font::nbsp << type_.experience_needed();
	}

	// Print the detailed description about the unit.
	ss << "\n" << detailed_description;

	if(const auto notes = type_.special_notes(); !notes.empty()) {
		ss << "\n" << markup::tag("header", _("Special Notes")) << "\n";
		for(const auto& note : notes) {
			ss << font::unicode_bullet << " " << markup::italic(note) << '\n';
		}
	}

	std::stringstream table_ss;

	//
	// Attacks table
	//
	ss << "\n" << markup::tag("header", _("Attacks"));

	if (!type_.attacks().empty()) {
		// Print headers for the table.
		table_ss << markup::tag("row",
			markup::tag("col", markup::bold(_("Icon"))),
			markup::tag("col", markup::bold(_("Name"))),
			markup::tag("col", markup::bold(_("Strikes"))),
			markup::tag("col", markup::bold(_("Range"))),
			markup::tag("col", markup::bold(_("Type"))),
			markup::tag("col", markup::bold(_("Special"))));

		// Print information about every attack.
		for(const attack_type& attack : type_.attacks()) {
			std::stringstream attack_ss;

			std::string lang_weapon = attack.name();
			std::string lang_type = string_table["type_" + attack.type()];

			// Attack icon
			attack_ss << markup::tag("col", markup::img(attack.icon()));

			// attack name
			attack_ss << markup::tag("col", lang_weapon);

			// damage x strikes
			attack_ss << markup::tag("col",
				attack.damage(), font::weapon_numbers_sep, attack.num_attacks(),
				" ", attack.accuracy_parry_description());

			// range
			const std::string range_icon = "icons/profiles/" + attack.range() + "_attack.png~SCALE_INTO(16,16)";
			if (attack.min_range() > 1 || attack.max_range() > 1) {
				attack_ss << markup::tag("col",
					markup::img(range_icon),
					attack.min_range(), "-", attack.max_range(), ' ',
					string_table["range_" + attack.range()]);
			} else {
				attack_ss << markup::tag("col",
					markup::img(range_icon),
					string_table["range_" + attack.range()]);
			}

			// type
			const std::string type_icon = "icons/profiles/" + attack.type() + ".png~SCALE_INTO(16,16)";
			attack_ss << markup::tag("col", markup::img(type_icon), lang_type);

			// special
			std::vector<std::pair<t_string, t_string>> specials = attack.special_tooltips();
			if (!specials.empty()) {
				std::stringstream specials_ss;
				std::string lang_special = "";
				const std::size_t specials_size = specials.size();
				for (std::size_t i = 0; i != specials_size; ++i) {
					const std::string ref_id = std::string("weaponspecial_")
						+ specials[i].first.base_str();
					lang_special = (specials[i].first);
					specials_ss << markup::make_link(lang_special, ref_id);
					if (i+1 != specials_size) {
						specials_ss << ", "; //comma placed before next special
					}
				}
				attack_ss << markup::tag("col", specials_ss.str());
			} else {
				attack_ss << markup::tag("col", "none");
			}

			table_ss << markup::tag("row", attack_ss.str());
		}

		ss << markup::tag("table", table_ss.str());
	}

	// Generate the movement type of the unit,
	// with resistance, defense, movement, jamming and vision data
	// updated according to any 'musthave' traits which always apply.
	movetype movement_type = type_.movement_type();
	config::const_child_itors traits = type_.possible_traits();
	if (!traits.empty() && type_.num_traits() > 0) {
		for (const config & t : traits) {
			if (t["availability"].str() == "musthave") {
				for (const config & effect : t.child_range("effect")) {
					if (!effect.has_child("filter") // If this is musthave but has a unit filter, it might not always apply, so don't apply it in the help.
							&& movetype::effects.find(effect["apply_to"].str()) != movetype::effects.end()) {
						movement_type.merge(effect, effect["replace"].to_bool());
					}
				}
			}
		}
	}

	const bool has_terrain_defense_caps = movement_type.has_terrain_defense_caps(prefs::get().encountered_terrains());
	const bool has_vision = type_.movement_type().has_vision_data();
	const bool has_jamming = type_.movement_type().has_jamming_data();

	//
	// Resistances table
	//
	ss << "\n" << markup::tag("header", _("Resistances"));

	std::stringstream().swap(table_ss);
	table_ss << markup::tag("row",
		markup::tag("col", markup::bold(_("Attack Type"))),
		markup::tag("col", markup::bold(_("Resistance"))));

	utils::string_map_res dam_tab = movement_type.damage_table();
	for(std::pair<std::string, std::string> dam_it : dam_tab) {
		int resistance = 100;
		try {
			resistance -= std::stoi(dam_it.second);
		} catch(std::invalid_argument&) {}
		std::string resist = std::to_string(resistance) + '%';
		const std::size_t pos = resist.find('-');
		if (pos != std::string::npos) {
			resist.replace(pos, 1, font::unicode_minus);
		}
		std::string color = unit_helper::resistance_color(resistance);
		const std::string lang_type = string_table["type_" + dam_it.first];
		const std::string type_icon = "icons/profiles/" + dam_it.first + ".png~SCALE_INTO(16,16)";
		table_ss << markup::tag("row",
			markup::tag("col", markup::img(type_icon), lang_type),
			markup::tag("col", markup::span_color(color, resist)));
	}
	ss << markup::tag("table", table_ss.str());

	//
	// Terrain Modifiers table
	//
	std::stringstream().swap(table_ss);
	if (std::shared_ptr<terrain_type_data> tdata = load_terrain_types_data()) {
		// Print the terrain modifier table of the unit.
		ss << "\n" << markup::tag("header", _("Terrain Modifiers"));

		// Header row
		std::stringstream row_ss;
		row_ss << markup::tag("col", markup::bold(_("Terrain")));
		row_ss << markup::tag("col", markup::bold(_("Defense")));
		row_ss << markup::tag("col", markup::bold(_("Movement Cost")));
		if (has_terrain_defense_caps) { row_ss << markup::tag("col", markup::bold(_("Defense Cap"))); }
		if (has_vision)				  { row_ss << markup::tag("col", markup::bold(_("Vision Cost"))); }
		if (has_jamming)			  { row_ss << markup::tag("col", markup::bold(_("Jamming Cost"))); }
		table_ss << markup::tag("row", row_ss.str());

		// Organize terrain movetype data
		std::set<terrain_movement_info> terrain_moves;
		for (t_translation::terrain_code terrain : prefs::get().encountered_terrains()) {
			if (terrain == t_translation::FOGGED || terrain == t_translation::VOID_TERRAIN || t_translation::terrain_matches(terrain, t_translation::ALL_OFF_MAP)) {
				continue;
			}
			const terrain_type& info = tdata->get_terrain_info(terrain);
			const int moves = movement_type.movement_cost(terrain);
			const bool cannot_move = moves > type_.movement();
			if (cannot_move && info.hide_if_impassable()) {
				continue;
			}

			if (info.is_indivisible() && info.is_nonnull()) {
				terrain_movement_info movement_info =
				{
					info.name(),
					info.id(),
					100 - movement_type.defense_modifier(terrain),
					moves,
					movement_type.vision_cost(terrain),
					movement_type.jamming_cost(terrain),
					movement_type.get_defense().capped(terrain)
				};

				terrain_moves.insert(movement_info);
			}
		}

		// Add movement table rows
		for(const terrain_movement_info &m : terrain_moves)
		{
			std::stringstream().swap(row_ss);
			bool high_res = false;
			const std::string tc_base = high_res ? "images/buttons/icon-base-32.png" : "images/buttons/icon-base-16.png";
			const std::string terrain_image = "icons/terrain/terrain_type_" + m.id + (high_res ? "_30.png" : ".png");
			const std::string final_image = tc_base + "~RC(magenta>" + m.id + ")~BLIT(" + terrain_image + ")";

			row_ss << markup::tag("col", markup::img(final_image), markup::make_link(m.name, "..terrain_" + m.id));

			// Defense  -  range: +10 % .. +70 %
			// passing false to select the more saturated red-to-green scale
			color_t def_color = game_config::red_to_green(m.defense, false);
			row_ss << markup::tag("col", markup::span_color(def_color, m.defense, "%"));

			// Movement  -  range: 1 .. 5, movetype::UNREACHABLE=impassable
			row_ss << markup::tag("col", format_mp_entry(type_.movement(), m.movement_cost));

			// Defense cap
			if (has_terrain_defense_caps) {
				if (m.defense_cap) {
					row_ss << markup::tag("col", markup::span_color(def_color, m.defense, "%"));
				} else {
					row_ss << markup::tag("col", markup::span_color("white", font::unicode_figure_dash));
				}
			}

			// Vision
			// uses same formatting as MP
			if (has_vision) {
				row_ss << markup::tag("col", format_mp_entry(type_.vision(), m.vision_cost));
			}

			// Jamming
			// uses same formatting as MP
			if (has_jamming) {
				row_ss << markup::tag("col", format_mp_entry(type_.jamming(), m.jamming_cost));
			}

			table_ss << markup::tag("row", row_ss.str());
		}

		ss << markup::tag("table", table_ss.str());

	} else {
		WRN_HP << "When building unit help topics, we couldn't get the terrain info we need.";
	}

	return ss.str();
}

} // end namespace help
