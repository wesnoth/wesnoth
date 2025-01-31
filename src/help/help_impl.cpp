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

#include "help/help_impl.hpp"

#include "actions/attack.hpp"           // for time_of_day bonus
#include "color.hpp"
#include "formula/string_utils.hpp"     // for VGETTEXT
#include "font/standard_colors.hpp"     // for NORMAL_COLOR
#include "game_config.hpp"              // for debug, menu_contract, etc
#include "game_config_manager.hpp"      // for game_config_manager
#include "gettext.hpp"                  // for _, gettext, N_
#include "help/help_topic_generators.hpp"
#include "hotkey/hotkey_command.hpp"    // for is_scope_active, etc
#include "log.hpp"                      // for LOG_STREAM, logger, etc
#include "map/map.hpp"                  // for gamemap
#include "picture.hpp"                  // for get_image, locator
#include "preferences/preferences.hpp"  // for encountered_terrains, etc
#include "resources.hpp"                // for tod_manager, config_manager
#include "serialization/markup.hpp"     // for markup utility functions
#include "serialization/parser.hpp"
#include "serialization/string_utils.hpp"  // for split, quoted_split, etc
#include "serialization/unicode.hpp"    // for iterator
#include "serialization/utf8_exception.hpp"  // for char_t, etc
#include "terrain/terrain.hpp"          // for terrain_type
#include "terrain/translation.hpp"      // for operator==, ter_list, etc
#include "terrain/type_data.hpp"        // for terrain_type_data, etc
#include "time_of_day.hpp"              // for time_of_day
#include "tod_manager.hpp"              // for tod_manager
#include "tstring.hpp"                  // for t_string, operator<<
#include "units/race.hpp"               // for unit_race, etc
#include "units/types.hpp"              // for unit_type, unit_type_data, etc
#include "utils/general.hpp"            // for contains

#include <algorithm>                    // for sort, find, transform, etc
#include <boost/algorithm/string.hpp>
#include <cassert>                     // for assert
#include <iterator>                     // for back_insert_iterator, etc
#include <map>                          // for map, etc
#include <set>
#include <utility>

static lg::log_domain log_help("help");
#define WRN_HP LOG_STREAM(warn, log_help)
#define DBG_HP LOG_STREAM(debug, log_help)

namespace help {

const game_config_view *game_cfg = nullptr;
// The default toplevel.
help::section default_toplevel;
// All sections and topics not referenced from the default toplevel.
help::section hidden_sections;

int last_num_encountered_units = -1;
int last_num_encountered_terrains = -1;
boost::tribool last_debug_state = boost::indeterminate;

std::vector<std::string> empty_string_vector;
const int max_section_level = 15;
const int title_size = font::SIZE_LARGE;
const int title2_size = font::SIZE_PLUS;
const int box_width = 2;
const int normal_font_size = font::SIZE_NORMAL;
const unsigned max_history = 100;
const std::string topic_img = "help/topic.png";
const std::string closed_section_img = "help/closed_section.png";
const std::string open_section_img = "help/open_section.png";
// The topic to open by default when opening the help dialog.
const std::string default_show_topic = "..introduction";
const std::string unknown_unit_topic = ".unknown_unit";
const std::string unit_prefix = "unit_";
const std::string terrain_prefix = "terrain_";
const std::string race_prefix = "race_";
const std::string faction_prefix = "faction_";
const std::string era_prefix = "era_";
const std::string variation_prefix = "variation_";
const std::string ability_prefix = "ability_";

static bool is_cjk_char(const char32_t ch)
{
	/**
	 * You can check these range at http://unicode.org/charts/
	 * see the "East Asian Scripts" part.
	 * Notice that not all characters in that part is still in use today, so don't list them all here.
	 * Below are characters that I guess may be used in wesnoth translations.
	 */

	//FIXME add range from Japanese-specific and Korean-specific section if you know the characters are used today.

	if (ch < 0x2e80) return false; // shortcut for common non-CJK

	return
		//Han Ideographs: all except Supplement
		(ch >= 0x4e00 && ch < 0x9fcf) ||
		(ch >= 0x3400 && ch < 0x4dbf) ||
		(ch >= 0x20000 && ch < 0x2a6df) ||
		(ch >= 0xf900 && ch < 0xfaff) ||
		(ch >= 0x3190 && ch < 0x319f) ||

		//Radicals: all except Ideographic Description
		(ch >= 0x2e80 && ch < 0x2eff) ||
		(ch >= 0x2f00 && ch < 0x2fdf) ||
		(ch >= 0x31c0 && ch < 0x31ef) ||

		//Chinese-specific: Bopomofo and Bopomofo Extended
		(ch >= 0x3104 && ch < 0x312e) ||
		(ch >= 0x31a0 && ch < 0x31bb) ||

		//Yi-specific: Yi Radicals, Yi Syllables
		(ch >= 0xa490 && ch < 0xa4c7) ||
		(ch >= 0xa000 && ch < 0xa48d) ||

		//Japanese-specific: Hiragana, Katakana, Kana Supplement
		(ch >= 0x3040 && ch <= 0x309f) ||
		(ch >= 0x30a0 && ch <= 0x30ff) ||
		(ch >= 0x1b000 && ch <= 0x1b001) ||

		//Ainu-specific: Katakana Phonetic Extensions
		(ch >= 0x31f0 && ch <= 0x31ff) ||

		//Korean-specific: Hangul Syllables, Hangul Jamo, Hangul Jamo Extended-A, Hangul Jamo Extended-B
		(ch >= 0xac00 && ch < 0xd7af) ||
		(ch >= 0x1100 && ch <= 0x11ff) ||
		(ch >= 0xa960 && ch <= 0xa97c) ||
		(ch >= 0xd7b0 && ch <= 0xd7fb) ||

		//CJK Symbols and Punctuation
		(ch >= 0x3000 && ch < 0x303f) ||

		//Halfwidth and Fullwidth Forms
		(ch >= 0xff00 && ch < 0xffef);
}

bool section_is_referenced(const std::string &section_id, const config &cfg)
{
	if (auto toplevel = cfg.optional_child("toplevel"))
	{
		const std::vector<std::string> toplevel_refs
			= utils::quoted_split(toplevel["sections"]);
		if (std::find(toplevel_refs.begin(), toplevel_refs.end(), section_id)
			!= toplevel_refs.end()) {
			return true;
		}
	}

	for (const config &section : cfg.child_range("section"))
	{
		const std::vector<std::string> sections_refd
			= utils::quoted_split(section["sections"]);
		if (std::find(sections_refd.begin(), sections_refd.end(), section_id)
			!= sections_refd.end()) {
			return true;
		}
	}
	return false;
}

bool topic_is_referenced(const std::string &topic_id, const config &cfg)
{
	if (auto toplevel = cfg.optional_child("toplevel"))
	{
		const std::vector<std::string> toplevel_refs
			= utils::quoted_split(toplevel["topics"]);
		if (std::find(toplevel_refs.begin(), toplevel_refs.end(), topic_id)
			!= toplevel_refs.end()) {
			return true;
		}
	}

	for (const config &section : cfg.child_range("section"))
	{
		const std::vector<std::string> topics_refd
			= utils::quoted_split(section["topics"]);
		if (std::find(topics_refd.begin(), topics_refd.end(), topic_id)
			!= topics_refd.end()) {
			return true;
		}
	}
	return false;
}

void parse_config_internal(const config *help_cfg, const config *section_cfg,
						   section &sec, int level)
{
	if (level > max_section_level) {
		PLAIN_LOG << "Maximum section depth has been reached. Maybe circular dependency?";
	}
	else if (section_cfg != nullptr) {
		const std::vector<std::string> sections = utils::quoted_split((*section_cfg)["sections"]);
		std::string id = level == 0 ? "toplevel" : (*section_cfg)["id"].str();
		if (level != 0) {
			if (!is_valid_id(id)) {
				std::stringstream ss;
				ss << "Invalid ID, used for internal purpose: '" << id << "'";
				throw parse_error(ss.str());
			}
		}
		std::string title = level == 0 ? "" : (*section_cfg)["title"].str();
		sec.id = id;
		sec.title = title;
		std::vector<std::string>::const_iterator it;
		// Find all child sections.
		for (it = sections.begin(); it != sections.end(); ++it) {
			if (auto child_cfg = help_cfg->find_child("section", "id", *it))
			{
				section child_section;
				parse_config_internal(help_cfg, child_cfg.ptr(), child_section, level + 1);
				sec.add_section(child_section);
			}
			else {
				std::stringstream ss;
				ss << "Help-section '" << *it << "' referenced from '"
				   << id << "' but could not be found.";
				throw parse_error(ss.str());
			}
		}

		generate_sections(help_cfg, (*section_cfg)["sections_generator"], sec, level);
		if ((*section_cfg)["sort_sections"] == "yes") {
			sec.sections.sort(section_less());
		}

		bool sort_topics = false;
		bool sort_generated = true;

		if ((*section_cfg)["sort_topics"] == "yes") {
		  sort_topics = true;
		  sort_generated = false;
		} else if ((*section_cfg)["sort_topics"] == "no") {
		  sort_topics = false;
		  sort_generated = false;
		} else if ((*section_cfg)["sort_topics"] == "generated") {
		  sort_topics = false;
		  sort_generated = true;
		} else if (!(*section_cfg)["sort_topics"].empty()) {
		  std::stringstream ss;
		  ss << "Invalid sort option: '" << (*section_cfg)["sort_topics"] << "'";
		  throw parse_error(ss.str());
		}

		std::vector<topic> generated_topics = generate_topics(sort_generated,(*section_cfg)["generator"]);

		const std::vector<std::string> topics_id = utils::quoted_split((*section_cfg)["topics"]);
		std::vector<topic> topics;

		// Find all topics in this section.
		for (it = topics_id.begin(); it != topics_id.end(); ++it) {
			if (auto topic_cfg = help_cfg->find_child("topic", "id", *it))
			{
				std::string text = topic_cfg["text"];
				text += generate_topic_text(topic_cfg["generator"], help_cfg, sec);
				topic child_topic(topic_cfg["title"], topic_cfg["id"], text);
				if (!is_valid_id(child_topic.id)) {
					std::stringstream ss;
					ss << "Invalid ID, used for internal purpose: '" << id << "'";
					throw parse_error(ss.str());
				}
				topics.push_back(child_topic);
			}
			else {
				std::stringstream ss;
				ss << "Help-topic '" << *it << "' referenced from '" << id
				   << "' but could not be found." << std::endl;
				throw parse_error(ss.str());
			}
		}

		if (sort_topics) {
			std::sort(topics.begin(),topics.end(), title_less());
			std::sort(generated_topics.begin(),
			  generated_topics.end(), title_less());
			std::merge(generated_topics.begin(),
			  generated_topics.end(),topics.begin(),topics.end()
			  ,std::back_inserter(sec.topics),title_less());
		}
		else {
			sec.topics.insert(sec.topics.end(),
				topics.begin(), topics.end());
			sec.topics.insert(sec.topics.end(),
				generated_topics.begin(), generated_topics.end());
		}
	}
}

section parse_config(const config *cfg)
{
	section sec;
	if (cfg != nullptr) {
		auto toplevel_cfg = cfg->optional_child("toplevel");
		parse_config_internal(cfg, toplevel_cfg.ptr(), sec);
	}
	return sec;
}

std::vector<topic> generate_topics(const bool sort_generated,const std::string &generator)
{
	std::vector<topic> res;
	if (generator.empty()) {
		return res;
	}

	if (generator == "abilities") {
		res = generate_ability_topics(sort_generated);
	} else if (generator == "weapon_specials") {
		res = generate_weapon_special_topics(sort_generated);
	} else if (generator == "time_of_days") {
		res = generate_time_of_day_topics(sort_generated);
	} else if (generator == "traits") {
		res = generate_trait_topics(sort_generated);
	} else {
		std::vector<std::string> parts = utils::split(generator, ':', utils::STRIP_SPACES);
		if (parts.size() > 1 && parts[0] == "units") {
			res = generate_unit_topics(sort_generated, parts[1]);
		} else if (parts[0] == "era" && parts.size()>1) {
			res = generate_era_topics(sort_generated, parts[1]);
		} else {
			WRN_HP << "Found a topic generator that I didn't recognize: " << generator;
		}
	}

	return res;
}

void generate_sections(const config *help_cfg, const std::string &generator, section &sec, int level)
{
	if (generator == "races") {
		generate_races_sections(help_cfg, sec, level);
	} else if (generator == "terrains") {
		generate_terrain_sections(sec, level);
	} else if (generator == "eras") {
		DBG_HP << "Generating eras...";
		generate_era_sections(help_cfg, sec, level);
	} else 	{
		std::vector<std::string> parts = utils::split(generator, ':', utils::STRIP_SPACES);
		if (parts.size() > 1 && parts[0] == "units") {
			generate_unit_sections(help_cfg, sec, level, true, parts[1]);
		} else if (generator.size() > 0) {
			WRN_HP << "Found a section generator that I didn't recognize: " << generator;
		}
	}
}

std::string generate_topic_text(const std::string &generator, const config *help_cfg, const section &sec)
{
	std::string empty_string = "";
	if (generator.empty()) {
		return empty_string;
	} else {
		std::vector<std::string> parts = utils::split(generator, ':');
		if (parts.size() > 1 && parts[0] == "contents") {
			if (parts[1] == "generated") {
				return generate_contents_links(sec);
			} else {
				return generate_contents_links(parts[1], help_cfg);
			}
		}
	}
	return empty_string;
}

topic_text& topic_text::operator=(std::shared_ptr<topic_generator> g)
{
	generator_ = std::move(g);
	return *this;
}

const config& topic_text::parsed_text() const
{
	if (generator_) {
		parsed_text_ = markup::parse_text((*generator_)());
		// This caches the result, so doesn't need the generator any more
		generator_.reset();
	}
	return parsed_text_;
}

static std::string time_of_day_bonus_colored(const int time_of_day_bonus)
{
	return markup::span_color((time_of_day_bonus > 0 ? "green" : (time_of_day_bonus < 0 ? "red" : "white")), time_of_day_bonus);
}

std::vector<topic> generate_time_of_day_topics(const bool /*sort_generated*/)
{
	std::vector<topic> topics;
	std::stringstream toplevel;

	if (!resources::tod_manager) {
		toplevel << _("Only available during a scenario.");
		topics.emplace_back(_("Time of Day Schedule"), "..schedule", toplevel.str());
		return topics;
	}

	const std::vector<time_of_day>& times = resources::tod_manager->times();
	for (const time_of_day& time : times)
	{
		const std::string id = "time_of_day_" + time.id;
		const std::string image = markup::img(time.image);
		const std::string image_lawful = markup::img("icons/alignments/alignment_lawful_30.png");
		const std::string image_neutral = markup::img("icons/alignments/alignment_neutral_30.png");
		const std::string image_chaotic = markup::img("icons/alignments/alignment_chaotic_30.png");
		const std::string image_liminal = markup::img("icons/alignments/alignment_liminal_30.png");
		std::stringstream text, row_ss;

		const int lawful_bonus = generic_combat_modifier(time.lawful_bonus, unit_alignments::type::lawful, false, resources::tod_manager->get_max_liminal_bonus());
		const int neutral_bonus = generic_combat_modifier(time.lawful_bonus, unit_alignments::type::neutral, false, resources::tod_manager->get_max_liminal_bonus());
		const int chaotic_bonus = generic_combat_modifier(time.lawful_bonus, unit_alignments::type::chaotic, false, resources::tod_manager->get_max_liminal_bonus());
		const int liminal_bonus = generic_combat_modifier(time.lawful_bonus, unit_alignments::type::liminal, false, resources::tod_manager->get_max_liminal_bonus());

		row_ss << markup::tag("col", markup::make_link(time.name.str(), id))
			   << markup::tag("col", image)
			   << markup::tag("col", image_lawful, time_of_day_bonus_colored(lawful_bonus))
			   << markup::tag("col", image_neutral, time_of_day_bonus_colored(neutral_bonus))
			   << markup::tag("col", image_chaotic, time_of_day_bonus_colored(chaotic_bonus))
			   << markup::tag("col", image_liminal, time_of_day_bonus_colored(liminal_bonus));
		toplevel << markup::tag("row", row_ss.str());

		text << image << '\n'
			 << time.description.str() << '\n'
			 << image_lawful << _("Lawful Bonus:") << ' ' << time_of_day_bonus_colored(lawful_bonus) << '\n'
			 << image_neutral << _("Neutral Bonus:") << ' ' << time_of_day_bonus_colored(neutral_bonus) << '\n'
			 << image_chaotic << _("Chaotic Bonus:") << ' ' << time_of_day_bonus_colored(chaotic_bonus) << '\n'
			 << image_liminal << _("Liminal Bonus:") << ' ' << time_of_day_bonus_colored(liminal_bonus) << '\n' << '\n'
			 << markup::make_link(_("Schedule"), "..schedule");

		topics.emplace_back(time.name.str(), id, text.str());
	}

	topics.emplace_back(_("Time of Day Schedule"), "..schedule", markup::tag("table", toplevel.str()));
	return topics;
}

std::vector<topic> generate_weapon_special_topics(const bool sort_generated)
{
	std::vector<topic> topics;

	std::map<t_string, std::string> special_description;
	std::map<t_string, std::set<std::string, string_less>> special_units;

	for (const unit_type_data::unit_type_map::value_type &type_mapping : unit_types.types())
	{
		const unit_type &type = type_mapping.second;
		// Only show the weapon special if we find it on a unit that
		// detailed description should be shown about.
		if (description_type(type) != FULL_DESCRIPTION)
			continue;

		for (const attack_type& atk : type.attacks()) {

			std::vector<std::pair<t_string, t_string>> specials = atk.special_tooltips();
			for ( std::size_t i = 0; i != specials.size(); ++i )
			{
				special_description.emplace(specials[i].first, specials[i].second);

				if (!type.hide_help()) {
					//add a link in the list of units having this special
					std::string type_name = type.type_name();
					//check for variations (walking corpse/soulless etc)
					const std::string section_prefix = type.show_variations_in_help() ? ".." : "";
					std::string ref_id = section_prefix + unit_prefix + type.id();
					//we put the translated name at the beginning of the hyperlink,
					//so the automatic alphabetic sorting of std::set can use it
					std::string link = markup::make_link(type_name, ref_id);
					special_units[specials[i].first].insert(link);
				}
			}
		}

		for(config adv : type.modification_advancements()) {
			for(config effect : adv.child_range("effect")) {
				if(effect["apply_to"] == "new_attack" && effect.has_child("specials")) {
					for(const auto [key, special] : effect.mandatory_child("specials").all_children_view()) {
						if(!special["name"].empty()) {
							special_description.emplace(special["name"].t_str(), special["description"].t_str());
							if(!type.hide_help()) {
								//add a link in the list of units having this special
								std::string type_name = type.type_name();
								//check for variations (walking corpse/soulless etc)
								const std::string section_prefix = type.show_variations_in_help() ? ".." : "";
								std::string ref_id = section_prefix + unit_prefix + type.id();
								//we put the translated name at the beginning of the hyperlink,
								//so the automatic alphabetic sorting of std::set can use it
								std::string link = markup::make_link(type_name, ref_id);
								special_units[special["name"]].insert(link);
							}
						}
					}
				} else if(effect["apply_to"] == "attack" && effect.has_child("set_specials")) {
					for(const auto [key, special] : effect.mandatory_child("set_specials").all_children_view()) {
						if(!special["name"].empty()) {
							special_description.emplace(special["name"].t_str(), special["description"].t_str());
							if(!type.hide_help()) {
								//add a link in the list of units having this special
								std::string type_name = type.type_name();
								//check for variations (walking corpse/soulless etc)
								const std::string section_prefix = type.show_variations_in_help() ? ".." : "";
								std::string ref_id = section_prefix + unit_prefix + type.id();
								//we put the translated name at the beginning of the hyperlink,
								//so the automatic alphabetic sorting of std::set can use it
								std::string link = markup::make_link(type_name, ref_id);
								special_units[special["name"]].insert(link);
							}
						}
					}
				}
			}
		}
	}

	for (std::map<t_string, std::string>::iterator s = special_description.begin();
			s != special_description.end(); ++s) {
		// use untranslated name to have universal topic id
		std::string id = "weaponspecial_" + s->first.base_str();
		std::stringstream text;
		text << s->second;
		text << "\n\n" << _("<header>text='Units with this special attack'</header>") << "\n";
		std::set<std::string, string_less> &units = special_units[s->first];
		for (std::set<std::string, string_less>::iterator u = units.begin(); u != units.end(); ++u) {
			text << font::unicode_bullet << " " << (*u) << "\n";
		}

		topics.emplace_back(s->first, id, text.str());
	}

	if (sort_generated)
		std::sort(topics.begin(), topics.end(), title_less());
	return topics;
}

std::vector<topic> generate_ability_topics(const bool sort_generated)
{
	std::vector<topic> topics;

	std::map<std::string, const unit_type::ability_metadata*> ability_topic_data;
	std::map<std::string, std::set<std::string, string_less>> ability_units;

	const auto parse = [&](const unit_type& type, const unit_type::ability_metadata& ability) {
		// NOTE: neither ability names nor ability ids are necessarily unique. Creating
		// topics for either each unique name or each unique id means certain abilities
		// will be excluded from help. So... the ability topic ref id is a combination
		// of id and (untranslated) name. It's rather ugly, but it works.
		const std::string topic_ref = ability.id + ability.name.base_str();

		ability_topic_data.emplace(topic_ref, &ability);

		if(!type.hide_help()) {
			// Add a link in the list of units with this ability
			// We put the translated name at the beginning of the hyperlink,
			// so the automatic alphabetic sorting of std::set can use it
			const std::string link = markup::make_link(type.type_name(), unit_prefix + type.id());
			ability_units[topic_ref].insert(link);
		}
	};

	// Look through all the unit types. If a unit of that type would have a full
	// description, add its abilities to the potential topic list. We don't want
	// to show abilities that the user has not encountered yet.
	for(const auto& type_mapping : unit_types.types()) {
		const unit_type& type = type_mapping.second;

		if(description_type(type) != FULL_DESCRIPTION) {
			continue;
		}

		for(const unit_type::ability_metadata& ability : type.abilities_metadata()) {
			parse(type, ability);
		}

		for(const unit_type::ability_metadata& ability : type.adv_abilities_metadata()) {
			parse(type, ability);
		}
	}

	for(const auto& a : ability_topic_data) {
		if (a.second->name.empty()) {
			continue;
		}
		std::ostringstream text;
		text << a.second->description;
		text << "\n\n" << _("<header>text='Units with this ability'</header>") << "\n";

		for(const auto& u : ability_units[a.first]) { // first is the topic ref id
			text << font::unicode_bullet << " " << u << "\n";
		}

		topics.emplace_back(a.second->name, ability_prefix + a.first, text.str());
	}

	if(sort_generated) {
		std::sort(topics.begin(), topics.end(), title_less());
	}

	return topics;
}

std::vector<topic> generate_era_topics(const bool sort_generated, const std::string & era_id)
{
	std::vector<topic> topics;

	auto era = game_cfg->find_child("era","id", era_id);
	if(era && !era["hide_help"].to_bool()) {
		topics = generate_faction_topics(*era, sort_generated);

		std::vector<std::string> faction_links;
		for (const topic & t : topics) {
			faction_links.push_back(markup::make_link(t.title, t.id));
		}

		std::stringstream text;
		text << markup::tag("header", _("Era:"), " ", era["name"]) << "\n";
		text << "\n";
		const config::attribute_value& description = era["description"];
		if (!description.empty()) {
			text << description.t_str() << "\n";
			text << "\n";
		}

		text << markup::tag("header", _("Factions")) << "\n";

		std::sort(faction_links.begin(), faction_links.end());
		for (const std::string &link : faction_links) {
			text << font::unicode_bullet << " " << link << "\n";
		}

		topic era_topic(era["name"], ".." + era_prefix + era["id"].str(), text.str());

		topics.push_back( era_topic );
	}
	return topics;
}

std::vector<topic> generate_faction_topics(const config & era, const bool sort_generated)
{
	std::vector<topic> topics;
	for (const config &f : era.child_range("multiplayer_side")) {
		const std::string& id = f["id"];
		if (id == "Random")
			continue;

		std::stringstream text;

		const config::attribute_value& description = f["description"];
		if (!description.empty()) {
			text << description.t_str() << "\n";
			text << "\n";
		}

		const std::vector<std::string> recruit_ids = utils::split(f["recruit"]);
		std::set<std::string> races;
		std::set<std::string> alignments;

		for (const std::string & u_id : recruit_ids) {
			if (const unit_type * t = unit_types.find(u_id, unit_type::HELP_INDEXED)) {
				assert(t);
				const unit_type & type = *t;

				if (const unit_race *r = unit_types.find_race(type.race_id())) {
					races.insert(markup::make_link(r->plural_name(), std::string("..") + race_prefix + r->id()));
				}
				alignments.insert(markup::make_link(type.alignment_description(type.alignment(), type.genders().front()), "time_of_day"));
			}
		}

		if (!races.empty()) {
			std::set<std::string>::iterator it = races.begin();
			text << _("Races: ") << *(it++);
			while(it != races.end()) {
				text << ", " << *(it++);
			}
			text << "\n\n";
		}

		if (!alignments.empty()) {
			std::set<std::string>::iterator it = alignments.begin();
			text << _("Alignments: ") << *(it++);
			while(it != alignments.end()) {
				text << ", " << *(it++);
			}
			text << "\n\n";
		}

		text << markup::tag("header", _("Leaders")) << "\n";
		const std::vector<std::string> leaders =
				make_unit_links_list( utils::split(f["leader"]), true );
		for (const std::string &link : leaders) {
			text << font::unicode_bullet << " " << link << "\n";
		}

		text << "\n";

		text << markup::tag("header", _("Recruits")) << "\n";
		const std::vector<std::string> recruit_links =
				make_unit_links_list( recruit_ids, true );
		for (const std::string &link : recruit_links) {
			text << font::unicode_bullet << " " << link << "\n";
		}

		const std::string name = f["name"];
		const std::string ref_id = faction_prefix + era["id"] + "_" + id;
		topics.emplace_back(name, ref_id, text.str());
	}
	if (sort_generated)
		std::sort(topics.begin(), topics.end(), title_less());
	return topics;
}

std::vector<topic> generate_trait_topics(const bool sort_generated)
{
	// All traits that could be assigned to at least one discovered or HIDDEN_BUT_SHOW_MACROS unit.
	// This is collected from the [units][trait], [race][traits], and [unit_type][traits] tags. If
	// there are duplicates with the same id, it takes the first one encountered.
	std::map<std::string, const config> trait_list;

	// The global traits that are direct children of a [units] tag
	for (const config & trait : unit_types.traits()) {
		trait_list.emplace(trait["id"], trait);
	}

	// Search for discovered unit types
	std::set<std::string> races;
	for(const auto& i : unit_types.types()) {
		const unit_type& type = i.second;
		UNIT_DESCRIPTION_TYPE desc_type = description_type(type);

		// Remember which races have been discovered.
		//
		// For unit types, unit_type::possible_traits() usually includes racial traits; however it's
		// possible that all discovered units of a race have ignore_race_traits=yes, and so we still
		// need to loop over the [race] tags looking for more traits.
		if(desc_type == FULL_DESCRIPTION) {
			races.insert(type.race_id());
		}

		// Handle [unit_type][trait]s.
		//
		// It would be better if we only looked at the traits that are specific to the unit_type,
		// but that unmerged unit_type_data.traits() isn't available. We're forced to use
		// possible_traits() instead which returns all of the traits, including the ones that units
		// with ignore_race_traits=no have inherited from their [race] tag.
		if (desc_type == FULL_DESCRIPTION || desc_type == HIDDEN_BUT_SHOW_MACROS) {
			for (const config& trait : type.possible_traits()) {
				trait_list.emplace(trait["id"], trait);
			}
		}
	}

	// Race traits, even those that duplicate a global trait (which will be dropped by emplace()).
	//
	// For traits, assume we don't discover additional races via the [race]help_taxonomy= links. The
	// traits themselves don't propagate down those links, so if the trait is interesting w.r.t. the
	// discovered units then their own race will already include it.
	for(const auto& race_id : races) {
		if(const unit_race *r = unit_types.find_race(race_id)) {
			for(const config & trait : r->additional_traits()) {
				trait_list.emplace(trait["id"], trait);
			}
		}
	}

	std::vector<topic> topics;
	for(auto& a : trait_list) {
		std::string id = "traits_" + a.first;
		const config& trait = a.second;

		std::string name = trait["male_name"].str();
		if (name.empty()) name = trait["female_name"].str();
		if (name.empty()) name = trait["name"].str();
		if (name.empty()) continue; // Hidden trait

		std::stringstream text;
		if (!trait["help_text"].empty()) {
			text << trait["help_text"];
		} else if (!trait["description"].empty()) {
			text << trait["description"];
		} else {
			text << _("No description available.");
		}
		text << "\n\n";

		topics.emplace_back(name, id, text.str());
	}

	if (sort_generated)
		std::sort(topics.begin(), topics.end(), title_less());
	return topics;
}


std::string make_unit_link(const std::string& type_id)
{
	std::string link;

	const unit_type *type = unit_types.find(type_id, unit_type::HELP_INDEXED);
	if (!type) {
		PLAIN_LOG << "Unknown unit type : " << type_id;
		// don't return an hyperlink (no page)
		// instead show the id (as hint)
		link = type_id;
	} else if (!type->hide_help()) {
		std::string name = type->type_name();
		std::string ref_id;
		if (description_type(*type) == FULL_DESCRIPTION) {
			const std::string section_prefix = type->show_variations_in_help() ? ".." : "";
			ref_id = section_prefix + unit_prefix + type->id();
		} else {
			ref_id = unknown_unit_topic;
			name += " (?)";
		}
		link =  markup::make_link(name, ref_id);
	} // if hide_help then link is an empty string

	return link;
}

std::vector<std::string> make_unit_links_list(const std::vector<std::string>& type_id_list, bool ordered)
{
	std::vector<std::string> links_list;
	for (const std::string &type_id : type_id_list) {
		std::string unit_link = make_unit_link(type_id);
		if (!unit_link.empty())
			links_list.push_back(unit_link);
	}

	if (ordered)
		std::sort(links_list.begin(), links_list.end());

	return links_list;
}

void generate_races_sections(const config* help_cfg, section& sec, int level)
{
	std::set<std::string, string_less> races;
	std::set<std::string, string_less> visible_races;

	// Calculate which races have been discovered, from the list of discovered unit types.
	for(const auto& i : unit_types.types()) {
		const unit_type& type = i.second;
		UNIT_DESCRIPTION_TYPE desc_type = description_type(type);
		if(desc_type == FULL_DESCRIPTION) {
			races.insert(type.race_id());
			if(!type.hide_help())
				visible_races.insert(type.race_id());
		}
	}

	// Propagate visibility up the help_taxonomy tree.
	std::set<std::string, string_less> last_sweep = visible_races;
	while(!last_sweep.empty()) {
		std::set<std::string, string_less> current_sweep;
		for(const auto& race_id : last_sweep) {
			if(const unit_race* r = unit_types.find_race(race_id)) {
				const auto& help_taxonomy = r->help_taxonomy();
				if(!help_taxonomy.empty() && !visible_races.count(help_taxonomy) && unit_types.find_race(help_taxonomy)) {
					current_sweep.insert(help_taxonomy);
					races.insert(help_taxonomy);
					visible_races.insert(help_taxonomy);
				}
			}
		}
		last_sweep = std::move(current_sweep);
	}

	struct taxonomy_queue_type
	{
		std::string parent_id;
		section content;
	};
	std::vector<taxonomy_queue_type> taxonomy_queue;

	// Add all races without a [race]help_taxonomy= to the documentation section, and queue the others.
	// This avoids a race condition dependency on the order that races are encountered in help_cfg.
	for(const auto& race_id : races) {
		section race_section;
		config section_cfg;

		bool hidden = (visible_races.count(race_id) == 0);

		section_cfg["id"] = hidden_symbol(hidden) + race_prefix + race_id;

		std::string title;
		std::string help_taxonomy;
		if(const unit_race* r = unit_types.find_race(race_id)) {
			title = r->plural_name();
			help_taxonomy = r->help_taxonomy();
		} else {
			title = _("race^Miscellaneous");
			// leave help_taxonomy empty
		}
		section_cfg["title"] = title;

		section_cfg["sections_generator"] = "units:" + race_id;
		section_cfg["generator"] = "units:" + race_id;

		parse_config_internal(help_cfg, &section_cfg, race_section, level + 1);

		if(help_taxonomy.empty()) {
			sec.add_section(race_section);
		} else {
			bool parent_hidden = (visible_races.count(help_taxonomy) == 0);
			auto parent_id = hidden_symbol(parent_hidden) + race_prefix + help_taxonomy;
			taxonomy_queue.push_back({std::move(parent_id), std::move(race_section)});
		}
	}

	// Each run through this loop handles one level of nesting of [race]help_taxonomy=
	bool process_queue_again = true;
	while(process_queue_again && !taxonomy_queue.empty()) {
		process_queue_again = false;
		auto to_process = std::exchange(taxonomy_queue, {});

		for(auto& x : to_process) {
			auto parent = find_section(sec, x.parent_id);
			if(parent) {
				parent->add_section(x.content);
				process_queue_again = true;
			} else {
				taxonomy_queue.push_back(std::move(x));
			}
		}
	}

	// Fallback to adding the new race at the top level, as if it had help_taxonomy.empty().
	for(auto& x : taxonomy_queue) {
		sec.add_section(x.content);
	}
}

void generate_era_sections(const config* help_cfg, section & sec, int level)
{
	for (const config & era : game_cfg->child_range("era")) {
		if (era["hide_help"].to_bool()) {
			continue;
		}

		DBG_HP << "Adding help section: " << era["id"].str();

		section era_section;
		config section_cfg;
		section_cfg["id"] = era_prefix + era["id"].str();
		section_cfg["title"] = era["name"];

		section_cfg["generator"] = "era:" + era["id"].str();

		DBG_HP << section_cfg.debug();

		parse_config_internal(help_cfg, &section_cfg, era_section, level+1);
		sec.add_section(era_section);
	}
}

void generate_terrain_sections(section& sec, int /*level*/)
{
	std::shared_ptr<terrain_type_data> tdata = load_terrain_types_data();

	if (!tdata) {
		WRN_HP << "When building terrain help sections, couldn't acquire terrain types data, aborting.";
		return;
	}

	std::map<std::string, section> base_map;

	const t_translation::ter_list& t_listi = tdata->list();

	for (const t_translation::terrain_code& t : t_listi) {

		const terrain_type& info = tdata->get_terrain_info(t);

		bool hidden = info.hide_help();

		if (prefs::get().encountered_terrains().find(t)
				== prefs::get().encountered_terrains().end() && !info.is_overlay())
			hidden = true;

		topic terrain_topic;
		terrain_topic.title = info.editor_name();
		terrain_topic.id    = hidden_symbol(hidden) + terrain_prefix + info.id();
		terrain_topic.text  = std::make_shared<terrain_topic_generator>(info);

		t_translation::ter_list base_terrains = tdata->underlying_union_terrain(t);
		if (info.has_default_base()) {
			for (const auto base : tdata->underlying_union_terrain(info.default_base())) {
				if (!utils::contains(base_terrains, base)) {
					base_terrains.emplace_back(base);
				}
			}
		}
		for (const t_translation::terrain_code& base : base_terrains) {

			const terrain_type& base_info = tdata->get_terrain_info(base);

			if (!base_info.is_nonnull() || base_info.hide_help())
				continue;

			section& base_section = base_map[base_info.id()];

			base_section.id = terrain_prefix + base_info.id();
			base_section.title = base_info.editor_name();

			if (base_info.id() == info.id())
				terrain_topic.id = ".." + terrain_prefix + info.id();
			base_section.topics.push_back(terrain_topic);
		}
	}

	for (const auto& base : base_map) {
		sec.add_section(base.second);
	}
}

void generate_unit_sections(const config* /*help_cfg*/, section& sec, int /*level*/, const bool /*sort_generated*/, const std::string& race)
{
	for (const unit_type_data::unit_type_map::value_type &i : unit_types.types()) {
		const unit_type &type = i.second;

		if (type.race_id() != race)
			continue;

		if (!type.show_variations_in_help())
			continue;

		section base_unit;
		for (const std::string &variation_id : type.variations()) {
			// TODO: Do we apply encountered stuff to variations?
			const unit_type &var_type = type.get_variation(variation_id);
			const std::string topic_name = var_type.variation_name();
			const std::string var_ref = hidden_symbol(var_type.hide_help()) + variation_prefix + var_type.id() + "_" + variation_id;

			topic var_topic(topic_name, var_ref, "");
			var_topic.text = std::make_shared<unit_topic_generator>(var_type, variation_id);
			base_unit.topics.push_back(var_topic);
		}

		const std::string type_name = type.type_name();
		const std::string ref_id = hidden_symbol(type.hide_help()) + unit_prefix +  type.id();

		base_unit.id = ref_id;
		base_unit.title = type_name;

		sec.add_section(base_unit);
	}
}

std::vector<topic> generate_unit_topics(const bool sort_generated, const std::string& race)
{
	std::vector<topic> topics;
	std::set<std::string, string_less> race_units;
	std::set<std::string, string_less> race_topics;
	std::set<std::string> alignments;

	for (const unit_type_data::unit_type_map::value_type &i : unit_types.types())
	{
		const unit_type &type = i.second;

		if (type.race_id() != race)
			continue;

		UNIT_DESCRIPTION_TYPE desc_type = description_type(type);
		if (desc_type != FULL_DESCRIPTION)
			continue;

		const std::string debug_suffix = (game_config::debug ? " (" + type.id() + ")" : "");
		const std::string type_name = type.type_name() + (type.id() == type.type_name().str() ? "" : debug_suffix);
		const std::string real_prefix = type.show_variations_in_help() ? ".." : "";
		const std::string ref_id = hidden_symbol(type.hide_help()) + real_prefix + unit_prefix +  type.id();
		topic unit_topic(type_name, ref_id, "");
		unit_topic.text = std::make_shared<unit_topic_generator>(type);
		topics.push_back(unit_topic);

		if (!type.hide_help()) {
			// we also record an hyperlink of this unit
			// in the list used for the race topic
			std::string link = markup::make_link(type_name, ref_id);
			race_units.insert(link);

			alignments.insert(markup::make_link(type.alignment_description(type.alignment(), type.genders().front()), "time_of_day"));
		}
	}

	//generate the hidden race description topic
	std::string race_id = "..race_"+race;
	std::string race_name;
	std::string race_description;
	std::string race_help_taxonomy;
	if (const unit_race *r = unit_types.find_race(race)) {
		race_name = r->plural_name();
		race_description = r->description();
		race_help_taxonomy = r->help_taxonomy();
		// if (description.empty()) description =  _("No description Available");
		for (const config &additional_topic : r->additional_topics())
		  {
		    std::string id = additional_topic["id"];
		    std::string title = additional_topic["title"];
		    std::string text = additional_topic["text"];
		    //topic additional_topic(title, id, text);
		    topics.emplace_back(title,id,text);
			std::string link = markup::make_link(title, id);
			race_topics.insert(link);
		  }
	} else {
		race_name = _ ("race^Miscellaneous");
		// description =  _("Here put the description of the Miscellaneous race");
	}

	// Find any other races whose [race]help_taxonomy points to the current race
	std::map<std::string, t_string> subgroups;
	for (const auto &r : unit_types.races()) {
		if (r.second.help_taxonomy() == race) {
			if (!r.second.plural_name().empty())
				subgroups[r.first] = r.second.plural_name();
			else
				subgroups[r.first] = r.first;
		}
	}

	std::stringstream text;

	if (!race_description.empty()) {
		text << race_description << "\n\n";
	}

	if (!alignments.empty()) {
		std::set<std::string>::iterator it = alignments.begin();
		text << (alignments.size() > 1 ? _("Alignments: ") : _("Alignment: ")) << *(it++);
		while(it != alignments.end()) {
			text << ", " << *(it++);
		}
		text << "\n\n";
	}

	if (!race_help_taxonomy.empty()) {
		utils::string_map symbols;
		symbols["topic_id"] = "..race_"+race_help_taxonomy;
		if (const unit_race *r = unit_types.find_race(race_help_taxonomy)) {
			symbols["help_taxonomy"] = r->plural_name();
		} else {
			// Fall back to using showing the race id for the race that we couldn't find.
			// Not great, but probably useful if UMC has a broken link.
			symbols["help_taxonomy"] = race_help_taxonomy;
		}
		// TRANSLATORS: this is expected to say "[Dunefolk are] a group of units, all of whom are Humans",
		// or "[Quenoth Elves are] a group of units, all of whom are Elves".
		text << VGETTEXT("This is a group of units, all of whom are <ref>dst='$topic_id' text='$help_taxonomy'</ref>.", symbols) << "\n\n";
	}

	if (!subgroups.empty()) {
		if (!race_help_taxonomy.empty()) {
			text << _("<header>text='Subgroups of units within this group'</header>") << "\n";
		} else {
			text << _("<header>text='Groups of units within this race'</header>") << "\n";
		}
		for (const auto &sg : subgroups) {
			text << font::unicode_bullet << " " << markup::make_link(sg.second, "..race_" + sg.first) << "\n";
		}
		text << "\n";
	}

	if (!race_help_taxonomy.empty()) {
		text << _("<header>text='Units of this group'</header>") << "\n";
	} else {
		text << _("<header>text='Units of this race'</header>") << "\n";
	}
	for (const auto &u : race_units) {
		text << font::unicode_bullet << " " << u << "\n";
	}

	topics.emplace_back(race_name, race_id, text.str());

	if (sort_generated)
		std::sort(topics.begin(), topics.end(), title_less());

	return topics;
}

UNIT_DESCRIPTION_TYPE description_type(const unit_type &type)
{
	if (game_config::debug || prefs::get().show_all_units_in_help()	||
			hotkey::is_scope_active(hotkey::SCOPE_EDITOR) ) {
		return FULL_DESCRIPTION;
	}

	const std::set<std::string> &encountered_units = prefs::get().encountered_units();
	if (encountered_units.find(type.id()) != encountered_units.end()) {
		return FULL_DESCRIPTION;
	}

	// See the docs of HIDDEN_BUT_SHOW_MACROS
	if (type.id() == "Fog Clearer") {
		return HIDDEN_BUT_SHOW_MACROS;
	}

	return NO_DESCRIPTION;
}

std::string generate_contents_links(const std::string& section_name, config const *help_cfg)
{
	auto section_cfg = help_cfg->find_child("section", "id", section_name);
	if (!section_cfg) {
		return std::string();
	}

	std::ostringstream res;

		std::vector<std::string> topics = utils::quoted_split(section_cfg["topics"]);

		// we use an intermediate structure to allow a conditional sorting
		typedef std::pair<std::string,std::string> link;
		std::vector<link> topics_links;

		std::vector<std::string>::iterator t;
		// Find all topics in this section.
		for (t = topics.begin(); t != topics.end(); ++t) {
			if (auto topic_cfg = help_cfg->find_child("topic", "id", *t)) {
				std::string id = topic_cfg["id"];
				if (is_visible_id(id))
					topics_links.emplace_back(topic_cfg["title"], id);
			}
		}

		if (section_cfg["sort_topics"] == "yes") {
			std::sort(topics_links.begin(),topics_links.end());
		}

		std::vector<link>::iterator l;
		for (l = topics_links.begin(); l != topics_links.end(); ++l) {
			std::string link = markup::make_link(l->first, l->second);
			res << font::unicode_bullet << " " << link << "\n";
		}

		return res.str();
}

std::string generate_contents_links(const section &sec)
{
		std::stringstream res;

		for (auto &s : sec.sections) {
			if (is_visible_id(s.id)) {
				std::string link = markup::make_link(s.title, ".."+s.id);
				res << font::unicode_bullet << " " << link << "\n";
			}
		}

		for(const topic& t : sec.topics) {
			if (is_visible_id(t.id)) {
				std::string link = markup::make_link(t.title, t.id);
				res << font::unicode_bullet << " " << link << "\n";
			}
		}
		return res.str();
}

bool topic::operator==(const topic &t) const
{
	return t.id == id;
}

bool topic::operator<(const topic &t) const
{
	return id < t.id;
}

bool section::operator==(const section &sec) const
{
	return sec.id == id;
}

bool section::operator<(const section &sec) const
{
	return id < sec.id;
}

void section::add_section(const section &s)
{
	sections.emplace_back(s);
}

void section::clear()
{
	topics.clear();
	sections.clear();
}

const topic *find_topic(const section &sec, const std::string &id)
{
	topic_list::const_iterator tit =
		std::find_if(sec.topics.begin(), sec.topics.end(), has_id(id));
	if (tit != sec.topics.end()) {
		return &(*tit);
	}
	for (const auto &s : sec.sections) {
		const auto t = find_topic(s, id);
		if (t != nullptr) {
			return t;
		}
	}
	return nullptr;
}

const section *find_section(const section &sec, const std::string &id)
{
	const auto sit =
		std::find_if(sec.sections.begin(), sec.sections.end(), has_id(id));
	if (sit != sec.sections.end()) {
		return &*sit;
	}
	for (const auto &subsection : sec.sections) {
		const auto s = find_section(subsection, id);
		if (s != nullptr) {
			return s;
		}
	}
	return nullptr;
}

section *find_section(section &sec, const std::string &id)
{
	return const_cast<section *>(find_section(const_cast<const section &>(sec), id));
}

std::string remove_first_space(const std::string& text)
{
	if (text.length() > 0 && text[0] == ' ') {
		return text.substr(1);
	}
	return text;
}

std::string get_first_word(const std::string &s)
{
	std::size_t first_word_start = s.find_first_not_of(' ');
	if (first_word_start == std::string::npos) {
		return s;
	}
	std::size_t first_word_end = s.find_first_of(" \n", first_word_start);
	if( first_word_end == first_word_start ) {
		// This word is '\n'.
		first_word_end = first_word_start+1;
	}

	//if no gap(' ' or '\n') found, test if it is CJK character
	std::string re = s.substr(0, first_word_end);

	utf8::iterator ch(re);
	if (ch == utf8::iterator::end(re))
		return re;

	char32_t firstchar = *ch;
	if (is_cjk_char(firstchar)) {
		re = unicode_cast<std::string>(firstchar);
	}
	return re;
}

void generate_contents()
{
	default_toplevel.clear();
	hidden_sections.clear();
	if (game_cfg != nullptr) {
		const config *help_config = &game_cfg->child_or_empty("help");
		try {
			default_toplevel = parse_config(help_config);
			// Create a config object that contains everything that is
			// not referenced from the toplevel element. Read this
			// config and save these sections and topics so that they
			// can be referenced later on when showing help about
			// specified things, but that should not be shown when
			// opening the help browser in the default manner.
			config hidden_toplevel;
			std::stringstream ss;
			for (const config &section : help_config->child_range("section"))
			{
				const std::string id = section["id"];
				if (find_section(default_toplevel, id) == nullptr) {
					// This section does not exist referenced from the
					// toplevel. Hence, add it to the hidden ones if it
					// is not referenced from another section.
					if (!section_is_referenced(id, *help_config)) {
						if (!ss.str().empty()) {
							ss << ",";
						}
						ss << id;
					}
				}
			}
			hidden_toplevel["sections"] = ss.str();
			ss.str("");
			for (const config &topic : help_config->child_range("topic"))
			{
				const std::string id = topic["id"];
				if (find_topic(default_toplevel, id) == nullptr) {
					if (!topic_is_referenced(id, *help_config)) {
						if (!ss.str().empty()) {
							ss << ",";
						}
						ss << id;
					}
				}
			}
			hidden_toplevel["topics"] = ss.str();
			config hidden_cfg = *help_config;
			// Change the toplevel to our new, custom built one.
			hidden_cfg.clear_children("toplevel");
			hidden_cfg.add_child("toplevel", std::move(hidden_toplevel));
			hidden_sections = parse_config(&hidden_cfg);
		}
		catch (parse_error& e) {
			std::stringstream msg;
			msg << "Parse error when parsing help text: '" << e.message << "'";
			PLAIN_LOG << msg.str();
		}
	}
}

// id starting with '.' are hidden
std::string hidden_symbol(bool hidden) {
	return (hidden ? "." : "");
}

bool is_visible_id(const std::string &id) {
	return (id.empty() || id[0] != '.');
}

/**
 * Return true if the id is valid for user defined topics and
 * sections. Some IDs are special, such as toplevel and may not be
 * be defined in the config.
 */
bool is_valid_id(const std::string &id) {
	if (id == "toplevel") {
		return false;
	}
	if (id.compare(0, unit_prefix.length(), unit_prefix) == 0 || id.compare(hidden_symbol().length(), unit_prefix.length(), unit_prefix) == 0) {
		return false;
	}
	if (id.compare(0, 8, "ability_") == 0) {
		return false;
	}
	if (id.compare(0, 14, "weaponspecial_") == 0) {
		return false;
	}
	if (id == "hidden") {
		return false;
	}
	return true;
}

/** Load the appropriate terrain types data to use */
std::shared_ptr<terrain_type_data> load_terrain_types_data()
{
	if (game_config_manager::get()){
		return game_config_manager::get()->terrain_types();
	} else {
		return {};
	}
}


} // end namespace help
