/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "help/help_impl.hpp"

#include "about.hpp"                    // for get_text
#include "display.hpp"                  // for display
#include "display_context.hpp"          // for display_context
#include "game_config.hpp"              // for debug, menu_contract, etc
#include "game_config_manager.hpp"      // for game_config_manager
#include "preferences/game.hpp"         // for encountered_terrains, etc
#include "gettext.hpp"                  // for _, gettext, N_
#include "help/help_topic_generators.hpp"
#include "hotkey/hotkey_command.hpp"    // for is_scope_active, etc
#include "image.hpp"                    // for get_image, locator
#include "log.hpp"                      // for LOG_STREAM, logger, etc
#include "utils/make_enum.hpp"          // for operator<<
#include "map/map.hpp"                  // for gamemap
#include "font/marked-up_text.hpp"      // for is_cjk_char, word_wrap_text
#include "font/standard_colors.hpp"     // for NORMAL_COLOR
#include "units/race.hpp"               // for unit_race, etc
#include "resources.hpp"                // for tod_manager, config_manager
#include "sdl/surface.hpp"                // for surface
#include "serialization/string_utils.hpp"  // for split, quoted_split, etc
#include "serialization/unicode_cast.hpp"  // for unicode_cast
#include "serialization/unicode_types.hpp"  // for char_t, etc
#include "terrain/terrain.hpp"          // for terrain_type
#include "terrain/translation.hpp"      // for operator==, ter_list, etc
#include "terrain/type_data.hpp"        // for terrain_type_data, etc
#include "time_of_day.hpp"              // for time_of_day
#include "tod_manager.hpp"              // for tod_manager
#include "tstring.hpp"                  // for t_string, operator<<
#include "units/types.hpp"              // for unit_type, unit_type_data, etc
#include "serialization/unicode.hpp"    // for iterator
#include "color.hpp"

#include <cassert>                     // for assert
#include <algorithm>                    // for sort, find, transform, etc
#include <iostream>                     // for operator<<, basic_ostream, etc
#include <iterator>                     // for back_insert_iterator, etc
#include <map>                          // for map, etc
#include <set>
#include <SDL.h>

static lg::log_domain log_display("display");
#define WRN_DP LOG_STREAM(warn, log_display)

static lg::log_domain log_help("help");
#define WRN_HP LOG_STREAM(warn, log_help)
#define DBG_HP LOG_STREAM(debug, log_help)

namespace help {

const config *game_cfg = nullptr;
// The default toplevel.
help::section default_toplevel;
// All sections and topics not referenced from the default toplevel.
help::section hidden_sections;

int last_num_encountered_units = -1;
int last_num_encountered_terrains = -1;
bool last_debug_state = game_config::debug;

config dummy_cfg;
std::vector<std::string> empty_string_vector;
const int max_section_level = 15;
const int title_size = font::SIZE_LARGE;
const int title2_size = font::SIZE_15;
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

bool section_is_referenced(const std::string &section_id, const config &cfg)
{
	if (const config &toplevel = cfg.child("toplevel"))
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
	if (const config &toplevel = cfg.child("toplevel"))
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
		std::cerr << "Maximum section depth has been reached. Maybe circular dependency?"
				  << std::endl;
	}
	else if (section_cfg != nullptr) {
		const std::vector<std::string> sections = utils::quoted_split((*section_cfg)["sections"]);
		sec.level = level;
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
			if (const config &child_cfg = help_cfg->find_child("section", "id", *it))
			{
				section child_section;
				parse_config_internal(help_cfg, &child_cfg, child_section, level + 1);
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
		//TODO: harmonize topics/sections sorting
		if ((*section_cfg)["sort_sections"] == "yes") {
			std::sort(sec.sections.begin(),sec.sections.end(), section_less());
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

		std::vector<topic> generated_topics =
		generate_topics(sort_generated,(*section_cfg)["generator"]);

		const std::vector<std::string> topics_id = utils::quoted_split((*section_cfg)["topics"]);
		std::vector<topic> topics;

		// Find all topics in this section.
		for (it = topics_id.begin(); it != topics_id.end(); ++it) {
			if (const config &topic_cfg = help_cfg->find_child("topic", "id", *it))
			{
				std::string text = topic_cfg["text"];
				text += generate_topic_text(topic_cfg["generator"], help_cfg, sec, generated_topics);
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
		config const &toplevel_cfg = cfg->child("toplevel");
		parse_config_internal(cfg, toplevel_cfg ? &toplevel_cfg : nullptr, sec);
	}
	return sec;
}

std::vector<topic> generate_topics(const bool sort_generated,const std::string &generator)
{
	std::vector<topic> res;
	if (generator == "") {
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
			WRN_HP << "Found a topic generator that I didn't recognize: " << generator << "\n";
		}
	}

	return res;
}

void generate_sections(const config *help_cfg, const std::string &generator, section &sec, int level)
{
	if (generator == "races") {
		generate_races_sections(help_cfg, sec, level);
	} else if (generator == "terrains") {
		generate_terrain_sections(help_cfg, sec, level);
	} else if (generator == "eras") {
		DBG_HP << "Generating eras...\n";
		generate_era_sections(help_cfg, sec, level);
	} else 	{
		std::vector<std::string> parts = utils::split(generator, ':', utils::STRIP_SPACES);
		if (parts.size() > 1 && parts[0] == "units") {
			generate_unit_sections(help_cfg, sec, level, true, parts[1]);
		} else if (generator.size() > 0) {
			WRN_HP << "Found a section generator that I didn't recognize: " << generator << "\n";
		}
	}
}

std::string generate_topic_text(const std::string &generator, const config *help_cfg, const section &sec, const std::vector<topic>& generated_topics)
{
	std::string empty_string = "";
	if (generator == "") {
		return empty_string;
	} else if (generator == "about") {
		return generate_about_text();
	} else {
		std::vector<std::string> parts = utils::split(generator, ':');
		if (parts.size() > 1 && parts[0] == "contents") {
			if (parts[1] == "generated") {
				return generate_contents_links(sec, generated_topics);
			} else {
				return generate_contents_links(parts[1], help_cfg);
			}
		}
	}
	return empty_string;
}

topic_text::~topic_text()
{
	if (generator_ && --generator_->count == 0)
		delete generator_;
}

topic_text::topic_text(topic_text const &t): parsed_text_(t.parsed_text_), generator_(t.generator_)
{
	if (generator_)
		++generator_->count;
}

topic_text &topic_text::operator=(topic_generator *g)
{
	if (generator_ && --generator_->count == 0)
		delete generator_;
	generator_ = g;
	return *this;
}

const std::vector<std::string>& topic_text::parsed_text() const
{
	if (generator_) {
		parsed_text_ = parse_text((*generator_)());
		if (--generator_->count == 0)
			delete generator_;
		generator_ = nullptr;
	}
	return parsed_text_;
}

std::vector<topic> generate_time_of_day_topics(const bool /*sort_generated*/)
{
	std::vector<topic> topics;
	std::stringstream toplevel;

	if (! resources::tod_manager) {
		toplevel << N_("Only available during a scenario.");
		topics.emplace_back("Time of Day Schedule", "..schedule", toplevel.str());
		return topics;
	}
	const std::vector<time_of_day>& times = resources::tod_manager->times();
	for (const time_of_day& time : times)
	{
		const std::string id = "time_of_day_" + time.id;
		const std::string image = "<img>src='" + time.image + "'</img>";
		std::stringstream text;

		toplevel << make_link(time.name.str(), id) << jump_to(160) <<
				image << jump(30) << time.lawful_bonus << '\n';

		text << image << '\n' <<
				time.description.str() << '\n' <<
				"Lawful Bonus: " << time.lawful_bonus << '\n' <<
				'\n' << make_link(N_("Schedule"), "..schedule");

		topics.emplace_back(time.name.str(), id, text.str());
	}

	topics.emplace_back("Time of Day Schedule", "..schedule", toplevel.str());
	return topics;
}

std::vector<topic> generate_weapon_special_topics(const bool sort_generated)
{
	std::vector<topic> topics;

	std::map<t_string, std::string> special_description;
	std::map<t_string, std::set<std::string, string_less> > special_units;

	for (const unit_type_data::unit_type_map::value_type &type_mapping : unit_types.types())
	{
		const unit_type &type = type_mapping.second;
		// Only show the weapon special if we find it on a unit that
		// detailed description should be shown about.
		if (description_type(type) != FULL_DESCRIPTION)
		 	continue;

		for (const attack_type& atk : type.attacks()) {

			std::vector<std::pair<t_string, t_string> > specials = atk.special_tooltips();
			for ( size_t i = 0; i != specials.size(); ++i )
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
					std::string link = make_link(type_name, ref_id);
					special_units[specials[i].first].insert(link);
				}
			}
		}

		for(config adv : type.modification_advancements()) {
			for(config effect : adv.child_range("effect")) {
				if(effect["apply_to"] == "new_attack" && effect.has_child("specials")) {
					for(config::any_child spec : effect.child("specials").all_children_range()) {
						if(!spec.cfg["name"].empty()) {
							special_description.emplace(spec.cfg["name"].t_str(), spec.cfg["description"].t_str());
							if(!type.hide_help()) {
								//add a link in the list of units having this special
								std::string type_name = type.type_name();
								//check for variations (walking corpse/soulless etc)
								const std::string section_prefix = type.show_variations_in_help() ? ".." : "";
								std::string ref_id = section_prefix + unit_prefix + type.id();
								//we put the translated name at the beginning of the hyperlink,
								//so the automatic alphabetic sorting of std::set can use it
								std::string link = make_link(type_name, ref_id);
								special_units[spec.cfg["name"]].insert(link);
							}
						}
					}
				} else if(effect["apply_to"] == "attack" && effect.has_child("set_specials")) {
					for(config::any_child spec : effect.child("set_specials").all_children_range()) {
						if(!spec.cfg["name"].empty()) {
							special_description.emplace(spec.cfg["name"].t_str(), spec.cfg["description"].t_str());
							if(!type.hide_help()) {
								//add a link in the list of units having this special
								std::string type_name = type.type_name();
								//check for variations (walking corpse/soulless etc)
								const std::string section_prefix = type.show_variations_in_help() ? ".." : "";
								std::string ref_id = section_prefix + unit_prefix + type.id();
								//we put the translated name at the beginning of the hyperlink,
								//so the automatic alphabetic sorting of std::set can use it
								std::string link = make_link(type_name, ref_id);
								special_units[spec.cfg["name"]].insert(link);
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
	std::map<t_string, std::string> ability_description;
	std::map<t_string, std::set<std::string, string_less> > ability_units;
	// Look through all the unit types, check if a unit of this type
	// should have a full description, if so, add this units abilities
	// for display. We do not want to show abilities that the user has
	// not encountered yet.
	for (const unit_type_data::unit_type_map::value_type &type_mapping : unit_types.types())
	{
		const unit_type &type = type_mapping.second;
		if (description_type(type) == FULL_DESCRIPTION) {

			std::vector<t_string> const* abil_vecs[2];
			abil_vecs[0] = &type.abilities();
			abil_vecs[1] = &type.adv_abilities();

			std::vector<t_string> const* desc_vecs[2];
			desc_vecs[0] = &type.ability_tooltips();
			desc_vecs[1] = &type.adv_ability_tooltips();

			for(int i=0; i<2; ++i) {
				std::vector<t_string> const& abil_vec = *abil_vecs[i];
				std::vector<t_string> const& desc_vec = *desc_vecs[i];
				for(size_t j=0; j < abil_vec.size(); ++j) {
					t_string const& abil_name = abil_vec[j];
					const std::string abil_desc =
						j >= desc_vec.size() ? "" : desc_vec[j].str();

					ability_description.emplace(abil_name, abil_desc);

					if (!type.hide_help()) {
						//add a link in the list of units with this ability
						std::string type_name = type.type_name();
						std::string ref_id = unit_prefix +  type.id();
						//we put the translated name at the beginning of the hyperlink,
						//so the automatic alphabetic sorting of std::set can use it
						std::string link = make_link(type_name, ref_id);
						ability_units[abil_name].insert(link);
					}
				}
			}
		}
	}

	for (std::map<t_string, std::string>::iterator a = ability_description.begin(); a != ability_description.end(); ++a) {
		// we generate topic's id using the untranslated version of the ability's name
		std::string id = "ability_" + a->first.base_str();
		std::stringstream text;
		text << a->second;  //description
		text << "\n\n" << _("<header>text='Units with this ability'</header>") << "\n";
		std::set<std::string, string_less> &units = ability_units[a->first];
		for (std::set<std::string, string_less>::iterator u = units.begin(); u != units.end(); ++u) {
			text << font::unicode_bullet << " " << (*u) << "\n";
		}

		topics.emplace_back(a->first, id, text.str());
	}

	if (sort_generated)
		std::sort(topics.begin(), topics.end(), title_less());
	return topics;
}

std::vector<topic> generate_era_topics(const bool sort_generated, const std::string & era_id)
{
	std::vector<topic> topics;

	const config & era = game_cfg->find_child("era","id", era_id);
	if(era && !era["hide_help"].to_bool()) {
		topics = generate_faction_topics(era, sort_generated);

		std::vector<std::string> faction_links;
		for (const topic & t : topics) {
			faction_links.push_back(make_link(t.title, t.id));
		}

		std::stringstream text;
		text << "<header>text='" << _("Era:") << " " << era["name"] << "'</header>" << "\n";
		text << "\n";
		const config::attribute_value& description = era["description"];
		if (!description.empty()) {
			text << description.t_str() << "\n";
			text << "\n";
		}

		text << "<header>text='" << _("Factions") << "'</header>" << "\n";

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
					races.insert(make_link(r->plural_name(), std::string("..") + race_prefix + r->id()));
				}
				DBG_HP << type.alignment() << " -> " << type.alignment_description(type.alignment(), type.genders().front()) << "\n";
				alignments.insert(make_link(type.alignment_description(type.alignment(), type.genders().front()), "time_of_day"));
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

		text << "<header>text='" << _("Leaders") << "'</header>" << "\n";
		const std::vector<std::string> leaders =
				make_unit_links_list( utils::split(f["leader"]), true );
		for (const std::string &link : leaders) {
			text << font::unicode_bullet << " " << link << "\n";
		}

		text << "\n";

		text << "<header>text='" << _("Recruits") << "'</header>" << "\n";
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
	std::vector<topic> topics;
	std::map<t_string, const config> trait_list;

	for (const config & trait : unit_types.traits()) {
		const std::string trait_id = trait["id"];
		trait_list.emplace(trait_id, trait);
	}


	for (const unit_type_data::unit_type_map::value_type &i : unit_types.types())
	{
		const unit_type &type = i.second;
		if (description_type(type) == FULL_DESCRIPTION) {
			if (config::const_child_itors traits = type.possible_traits()) {
				for (const config & trait : traits) {
					const std::string trait_id = trait["id"];
					trait_list.emplace(trait_id, trait);
				}
			}
			if (const unit_race *r = unit_types.find_race(type.race_id())) {
				for (const config & trait : r->additional_traits()) {
					const std::string trait_id = trait["id"];
					trait_list.emplace(trait_id, trait);
				}
			}
		}
	}

	for (std::map<t_string, const config>::iterator a = trait_list.begin(); a != trait_list.end(); ++a) {
		std::string id = "traits_" + a->first;
		const config trait = a->second;
		std::stringstream text;
		if (trait["help_text"].empty()) {
			text << trait["description"];
		} else {
			text << trait["help_text"];
		}
		text << "\n\n";
		if (trait["availability"] == "musthave") {
			text << _("Availability: ") << _("Must-have") << "\n";
		} else if (trait["availability"] == "none") {
			text << _("Availability: ") << _("Unavailable") << "\n";
		}
		std::string name = trait["male_name"].str();
		if (name.empty()) name = trait["female_name"].str();
		if (name.empty()) name = trait["name"].str();

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
		std::cerr << "Unknown unit type : " << type_id << "\n";
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
		link =  make_link(name, ref_id);
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

void generate_races_sections(const config *help_cfg, section &sec, int level)
{
	std::set<std::string, string_less> races;
	std::set<std::string, string_less> visible_races;

	for (const unit_type_data::unit_type_map::value_type &i : unit_types.types())
	{
		const unit_type &type = i.second;
		UNIT_DESCRIPTION_TYPE desc_type = description_type(type);
		if (desc_type == FULL_DESCRIPTION) {
			races.insert(type.race_id());
			if (!type.hide_help())
				visible_races.insert(type.race_id());
		}
	}

	for(std::set<std::string, string_less>::iterator it = races.begin(); it != races.end(); ++it) {
		section race_section;
		config section_cfg;

		bool hidden = (visible_races.count(*it) == 0);

		section_cfg["id"] = hidden_symbol(hidden) + race_prefix + *it;

		std::string title;
		if (const unit_race *r = unit_types.find_race(*it)) {
			title = r->plural_name();
		} else {
			title = _ ("race^Miscellaneous");
		}
		section_cfg["title"] = title;

		section_cfg["sections_generator"] = "units:" + *it;
		section_cfg["generator"] = "units:" + *it;

		parse_config_internal(help_cfg, &section_cfg, race_section, level+1);
		sec.add_section(race_section);
	}
}

void generate_era_sections(const config* help_cfg, section & sec, int level)
{
	for (const config & era : game_cfg->child_range("era")) {
		if (era["hide_help"].to_bool()) {
			continue;
		}

		DBG_HP << "Adding help section: " << era["id"].str() << "\n";

		section era_section;
		config section_cfg;
		section_cfg["id"] = era_prefix + era["id"].str();
		section_cfg["title"] = era["name"];

		section_cfg["generator"] = "era:" + era["id"].str();

		DBG_HP << section_cfg.debug() << "\n";

		parse_config_internal(help_cfg, &section_cfg, era_section, level+1);
		sec.add_section(era_section);
	}
}

void generate_terrain_sections(const config* /*help_cfg*/, section& sec, int /*level*/)
{
	ter_data_cache tdata = load_terrain_types_data();

	if (!tdata) {
		WRN_HP << "When building terrain help sections, couldn't acquire terrain types data, aborting.\n";
		return;
	}

	std::map<std::string, section> base_map;

	const t_translation::ter_list& t_listi = tdata->list();

	for (const t_translation::terrain_code& t : t_listi) {

		const terrain_type& info = tdata->get_terrain_info(t);

		bool hidden = info.is_combined() || info.hide_help();

		if (preferences::encountered_terrains().find(t)
				== preferences::encountered_terrains().end() && !info.is_overlay())
			hidden = true;

		topic terrain_topic;
		terrain_topic.title = info.editor_name();
		terrain_topic.id    = hidden_symbol(hidden) + terrain_prefix + info.id();
		terrain_topic.text  = new terrain_topic_generator(info);

		t_translation::ter_list base_terrains = tdata->underlying_union_terrain(t);
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

	for (std::map<std::string, section>::const_iterator it = base_map.begin(); it != base_map.end(); ++it) {
		sec.add_section(it->second);
	}
}

void generate_unit_sections(const config* /*help_cfg*/, section& sec, int level, const bool /*sort_generated*/, const std::string& race)
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
			const std::string topic_name = var_type.type_name() + "\n" + var_type.variation_name();
			const std::string var_ref = hidden_symbol(var_type.hide_help()) + variation_prefix + var_type.id() + "_" + variation_id;

			topic var_topic(topic_name, var_ref, "");
			var_topic.text = new unit_topic_generator(var_type, variation_id);
			base_unit.topics.push_back(var_topic);
		}

		const std::string type_name = type.type_name();
		const std::string ref_id = hidden_symbol(type.hide_help()) + unit_prefix +  type.id();

		base_unit.id = ref_id;
		base_unit.title = type_name;
		base_unit.level = level +1;

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

		const std::string type_name = type.type_name();
		const std::string real_prefix = type.show_variations_in_help() ? ".." : "";
		const std::string ref_id = hidden_symbol(type.hide_help()) + real_prefix + unit_prefix +  type.id();
		topic unit_topic(type_name, ref_id, "");
		unit_topic.text = new unit_topic_generator(type);
		topics.push_back(unit_topic);

		if (!type.hide_help()) {
			// we also record an hyperlink of this unit
			// in the list used for the race topic
			std::string link = make_link(type_name, ref_id);
			race_units.insert(link);

			alignments.insert(make_link(type.alignment_description(type.alignment(), type.genders().front()), "time_of_day"));
		}
	}

	//generate the hidden race description topic
	std::string race_id = "..race_"+race;
	std::string race_name;
	std::string race_description;
	if (const unit_race *r = unit_types.find_race(race)) {
		race_name = r->plural_name();
		race_description = r->description();
		// if (description.empty()) description =  _("No description Available");
		for (const config &additional_topic : r->additional_topics())
		  {
		    std::string id = additional_topic["id"];
		    std::string title = additional_topic["title"];
		    std::string text = additional_topic["text"];
		    //topic additional_topic(title, id, text);
		    topics.emplace_back(title,id,text);
			std::string link = make_link(title, id);
			race_topics.insert(link);
		  }
	} else {
		race_name = _ ("race^Miscellaneous");
		// description =  _("Here put the description of the Miscellaneous race");
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

	text << _("<header>text='Units of this race'</header>") << "\n";
	for (std::set<std::string, string_less>::iterator u = race_units.begin(); u != race_units.end(); ++u) {
		text << font::unicode_bullet << " " << (*u) << "\n";
	}

	topics.emplace_back(race_name, race_id, text.str());

	if (sort_generated)
		std::sort(topics.begin(), topics.end(), title_less());

	return topics;
}

UNIT_DESCRIPTION_TYPE description_type(const unit_type &type)
{
	if (game_config::debug || preferences::show_all_units_in_help()	||
			hotkey::is_scope_active(hotkey::SCOPE_EDITOR) ) {
		return FULL_DESCRIPTION;
	}

	const std::set<std::string> &encountered_units = preferences::encountered_units();
	if (encountered_units.find(type.id()) != encountered_units.end()) {
		return FULL_DESCRIPTION;
	}
	return NO_DESCRIPTION;
}

std::string generate_about_text()
{
	/*std::vector<std::string> about_lines = about::get_text();
	std::vector<std::string> res_lines;
	std::transform(about_lines.begin(), about_lines.end(), std::back_inserter(res_lines),
				   about_text_formatter());
	res_lines.erase(std::remove(res_lines.begin(), res_lines.end(), ""), res_lines.end());
	std::string text = utils::join(res_lines, "\n");
	return text;*/
	return "";
}

std::string generate_contents_links(const std::string& section_name, config const *help_cfg)
{
	config const &section_cfg = help_cfg->find_child("section", "id", section_name);
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
			if (config const &topic_cfg = help_cfg->find_child("topic", "id", *t)) {
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
			std::string link = make_link(l->first, l->second);
			res << font::unicode_bullet << " " << link << "\n";
		}

		return res.str();
}

std::string generate_contents_links(const section &sec, const std::vector<topic>& topics)
{
		std::stringstream res;

		section_list::const_iterator s;
		for (s = sec.sections.begin(); s != sec.sections.end(); ++s) {
			if (is_visible_id((*s)->id)) {
				std::string link = make_link((*s)->title, ".."+(*s)->id);
				res << font::unicode_bullet << " " << link << "\n";
			}
		}

		std::vector<topic>::const_iterator t;
		for (t = topics.begin(); t != topics.end(); ++t) {
			if (is_visible_id(t->id)) {
				std::string link = make_link(t->title, t->id);
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

section::~section()
{
	std::for_each(sections.begin(), sections.end(), delete_section());
}

section::section(const section &sec) :
	title(sec.title),
	id(sec.id),
	topics(sec.topics),
	sections(),
	level(sec.level)
{
	std::transform(sec.sections.begin(), sec.sections.end(),
				   std::back_inserter(sections), create_section());
}

section& section::operator=(const section &sec)
{
	title = sec.title;
	id = sec.id;
	level = sec.level;
	topics.insert(topics.end(), sec.topics.begin(), sec.topics.end());
	std::transform(sec.sections.begin(), sec.sections.end(),
				   std::back_inserter(sections), create_section());
	return *this;
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
	sections.push_back(new section(s));
}

void section::clear()
{
	topics.clear();
	std::for_each(sections.begin(), sections.end(), delete_section());
	sections.clear();
}



const topic *find_topic(const section &sec, const std::string &id)
{
	topic_list::const_iterator tit =
		std::find_if(sec.topics.begin(), sec.topics.end(), has_id(id));
	if (tit != sec.topics.end()) {
		return &(*tit);
	}
	section_list::const_iterator sit;
	for (sit = sec.sections.begin(); sit != sec.sections.end(); ++sit) {
		const topic *t = find_topic(*(*sit), id);
		if (t != nullptr) {
			return t;
		}
	}
	return nullptr;
}

const section *find_section(const section &sec, const std::string &id)
{
	section_list::const_iterator sit =
		std::find_if(sec.sections.begin(), sec.sections.end(), has_id(id));
	if (sit != sec.sections.end()) {
		return *sit;
	}
	for (sit = sec.sections.begin(); sit != sec.sections.end(); ++sit) {
		const section *s = find_section(*(*sit), id);
		if (s != nullptr) {
			return s;
		}
	}
	return nullptr;
}

std::vector<std::string> parse_text(const std::string &text)
{
	std::vector<std::string> res;
	bool last_char_escape = false;
	const char escape_char = '\\';
	std::stringstream ss;
	size_t pos;
	enum { ELEMENT_NAME, OTHER } state = OTHER;
	for (pos = 0; pos < text.size(); ++pos) {
		const char c = text[pos];
		if (c == escape_char && !last_char_escape) {
			last_char_escape = true;
		}
		else {
			if (state == OTHER) {
				if (c == '<') {
					if (last_char_escape) {
						ss << c;
					}
					else {
						res.push_back(ss.str());
						ss.str("");
						state = ELEMENT_NAME;
					}
				}
				else {
					ss << c;
				}
			}
			else if (state == ELEMENT_NAME) {
				if (c == '/') {
					std::string msg = "Erroneous / in element name.";
					throw parse_error(msg);
				}
				else if (c == '>') {
					// End of this name.
					std::stringstream s;
					const std::string element_name = ss.str();
					ss.str("");
					s << "</" << element_name << ">";
					const std::string end_element_name = s.str();
					size_t end_pos = text.find(end_element_name, pos);
					if (end_pos == std::string::npos) {
						std::stringstream msg;
						msg << "Unterminated element: " << element_name;
						throw parse_error(msg.str());
					}
					s.str("");
					const std::string contents = text.substr(pos + 1, end_pos - pos - 1);
					const std::string element = convert_to_wml(element_name, contents);
					res.push_back(element);
					pos = end_pos + end_element_name.size() - 1;
					state = OTHER;
				}
				else {
					ss << c;
				}
			}
			last_char_escape = false;
		}
	}
	if (state == ELEMENT_NAME) {
		std::stringstream msg;
		msg << "Element '" << ss.str() << "' continues through end of string.";
		throw parse_error(msg.str());
	}
	if (!ss.str().empty()) {
		// Add the last string.
		res.push_back(ss.str());
	}
	return res;
}

std::string convert_to_wml(const std::string &element_name, const std::string &contents)
{
	std::stringstream ss;
	bool in_quotes = false;
	bool last_char_escape = false;
	const char escape_char = '\\';
	std::vector<std::string> attributes;
	// Find the different attributes.
	// No checks are made for the equal sign or something like that.
	// Attributes are just separated by spaces or newlines.
	// Attributes that contain spaces must be in single quotes.
	for (size_t pos = 0; pos < contents.size(); ++pos) {
		const char c = contents[pos];
		if (c == escape_char && !last_char_escape) {
			last_char_escape = true;
		}
		else {
			if (c == '\'' && !last_char_escape) {
				ss << '"';
				in_quotes = !in_quotes;
			}
			else if ((c == ' ' || c == '\n') && !last_char_escape && !in_quotes) {
				// Space or newline, end of attribute.
				attributes.push_back(ss.str());
				ss.str("");
			}
			else {
				ss << c;
			}
			last_char_escape = false;
		}
	}
	if (in_quotes) {
		std::stringstream msg;
		msg << "Unterminated single quote after: '" << ss.str() << "'";
		throw parse_error(msg.str());
	}
	if (!ss.str().empty()) {
		attributes.push_back(ss.str());
	}
	ss.str("");
	// Create the WML.
	ss << "[" << element_name << "]\n";
	for (std::vector<std::string>::const_iterator it = attributes.begin();
		 it != attributes.end(); ++it) {
		ss << *it << "\n";
	}
	ss << "[/" << element_name << "]\n";
	return ss.str();
}

color_t string_to_color(const std::string &cmp_str)
{
	if (cmp_str == "green") {
		return font::GOOD_COLOR;
	}
	if (cmp_str == "red") {
		return font::BAD_COLOR;
	}
	if (cmp_str == "black") {
		return font::BLACK_COLOR;
	}
	if (cmp_str == "yellow") {
		return font::YELLOW_COLOR;
	}
	if (cmp_str == "white") {
		return font::BIGMAP_COLOR;
	}
	// a #rrggbb color in pango format.
	if (*cmp_str.c_str() == '#' && cmp_str.size() == 7) {
		return color_t::from_argb_bytes(strtoul(cmp_str.c_str() + 1, nullptr, 16));
	}
	return font::NORMAL_COLOR;
}

std::vector<std::string> split_in_width(const std::string &s, const int font_size,
		const unsigned width)
{
	std::vector<std::string> res;
	try {
	const std::string& first_line = font::word_wrap_text(s, font_size, width, -1, 1, true);
	res.push_back(first_line);
	if(s.size() > first_line.size()) {
		res.push_back(s.substr(first_line.size()));
	}
	}
	catch (utf8::invalid_utf8_exception&)
	{
		throw parse_error (_("corrupted original file"));
	}

	return res;
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
	size_t first_word_start = s.find_first_not_of(' ');
	if (first_word_start == std::string::npos) {
		return s;
	}
	size_t first_word_end = s.find_first_of(" \n", first_word_start);
	if( first_word_end == first_word_start ) {
		// This word is '\n'.
		first_word_end = first_word_start+1;
	}

	//if no gap(' ' or '\n') found, test if it is CJK character
	std::string re = s.substr(0, first_word_end);

	utf8::iterator ch(re);
	if (ch == utf8::iterator::end(re))
		return re;

	ucs4::char_t firstchar = *ch;
	if (font::is_cjk_char(firstchar)) {
		re = unicode_cast<utf8::string>(firstchar);
	}
	return re;
}

void generate_contents()
{
	default_toplevel.clear();
	hidden_sections.clear();
	if (game_cfg != nullptr) {
		const config *help_config = &game_cfg->child("help");
		if (!*help_config) {
			help_config = &dummy_cfg;
		}
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
			hidden_cfg.add_child("toplevel", hidden_toplevel);
			hidden_sections = parse_config(&hidden_cfg);
		}
		catch (parse_error& e) {
			std::stringstream msg;
			msg << "Parse error when parsing help text: '" << e.message << "'";
			std::cerr << msg.str() << std::endl;
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

/// Return true if the id is valid for user defined topics and
/// sections. Some IDs are special, such as toplevel and may not be
/// be defined in the config.
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


// Return the width for the image with filename.
unsigned image_width(const std::string &filename)
{
	image::locator loc(filename);
	surface surf(image::get_image(loc));
	if (surf != nullptr) {
		return surf->w;
	}
	return 0;
}

void push_tab_pair(std::vector<std::pair<std::string, unsigned int> > &v, const std::string &s)
{
	v.emplace_back(s, font::line_width(s, normal_font_size));
}

std::string generate_table(const table_spec &tab, const unsigned int spacing)
{
	table_spec::const_iterator row_it;
	std::vector<std::pair<std::string, unsigned> >::const_iterator col_it;
	unsigned int num_cols = 0;
	for (row_it = tab.begin(); row_it != tab.end(); ++row_it) {
		if (row_it->size() > num_cols) {
			num_cols = row_it->size();
		}
	}
	std::vector<unsigned int> col_widths(num_cols, 0);
	// Calculate the width of all columns, including spacing.
	for (row_it = tab.begin(); row_it != tab.end(); ++row_it) {
		unsigned int col = 0;
		for (col_it = row_it->begin(); col_it != row_it->end(); ++col_it) {
			if (col_widths[col] < col_it->second + spacing) {
				col_widths[col] = col_it->second + spacing;
			}
			++col;
		}
	}
	std::vector<unsigned int> col_starts(num_cols);
	// Calculate the starting positions of all columns
	for (unsigned int i = 0; i < num_cols; ++i) {
		unsigned int this_col_start = 0;
		for (unsigned int j = 0; j < i; ++j) {
			this_col_start += col_widths[j];
		}
		col_starts[i] = this_col_start;
	}
	std::stringstream ss;
	for (row_it = tab.begin(); row_it != tab.end(); ++row_it) {
		unsigned int col = 0;
		for (col_it = row_it->begin(); col_it != row_it->end(); ++col_it) {
			ss << jump_to(col_starts[col]) << col_it->first;
			++col;
		}
		ss << "\n";
	}
	return ss.str();
}

/// Prepend all chars with meaning inside attributes with a backslash.
std::string escape(const std::string &s)
{
	return utils::escape(s, "'\\");
}

/// Load the appropriate terrain types data to use
ter_data_cache load_terrain_types_data() {
	if (display::get_singleton()) {
		return display::get_singleton()->get_disp_context().map().tdata();
	} else if (game_config_manager::get()){
		return game_config_manager::get()->terrain_types();
	} else {
		return ter_data_cache();
	}
}


} // end namespace help
