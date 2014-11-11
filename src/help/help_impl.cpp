/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "help_impl.hpp"

#include "about.hpp"                    // for get_text
#include "game_config.hpp"              // for debug, menu_contract, etc
#include "game_preferences.hpp"         // for encountered_terrains, etc
#include "gettext.hpp"                  // for _, gettext, N_
#include "hotkey/hotkey_command.hpp"    // for is_scope_active, etc
#include "language.hpp"                 // for string_table, symbol_table
#include "log.hpp"                      // for LOG_STREAM, logger, etc
#include "make_enum.hpp"                // for operator<<
#include "marked-up_text.hpp"           // for is_cjk_char, word_wrap_text
#include "movetype.hpp"                 // for movetype, movetype::effects, etc
#include "race.hpp"                     // for unit_race, etc
#include "resources.hpp"                // for tod_manager
#include "serialization/unicode_cast.hpp"  // for unicode_cast
#include "serialization/unicode_types.hpp"  // for char_t, etc
#include "sound.hpp"                    // for play_UI_sound
#include "terrain.hpp"                  // for terrain_type
#include "terrain_translation.hpp"      // for operator==, t_list, etc
#include "terrain_type_data.hpp"        // for terrain_type_data, etc
#include "time_of_day.hpp"              // for time_of_day
#include "tod_manager.hpp"              // for tod_manager
#include "tstring.hpp"                  // for t_string, operator<<
#include "unit_helper.hpp"              // for resistance_color
#include "unit_types.hpp"               // for unit_type, unit_type_data, etc
#include "serialization/unicode.hpp"  // for iterator
#include "wml_separators.hpp"           // for IMG_TEXT_SEPARATOR, etc

#include <assert.h>                     // for assert
#include <stdio.h>                      // for NULL, snprintf
#include <stdlib.h>                     // for atoi
#include <algorithm>                    // for sort, find, transform, etc
#include <boost/foreach.hpp>            // for auto_any_base, etc
#include <boost/mpl/bool.hpp>           // for bool_
#include <boost/optional/optional.hpp>  // for optional
#include <boost/smart_ptr/shared_ptr.hpp>  // for shared_ptr
#include <iostream>                     // for operator<<, basic_ostream, etc
#include <iterator>                     // for back_insert_iterator, etc
#include <map>                          // for map, etc
#include <SDL.h>

class CVideo;

static lg::log_domain log_display("display");
#define WRN_DP LOG_STREAM(warn, log_display)

static lg::log_domain log_help("help");
#define WRN_HP LOG_STREAM(warn, log_help)
#define DBG_HP LOG_STREAM(debug, log_help)

namespace help {

const config *game_cfg = NULL;
// The default toplevel.
help::section toplevel;
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
const int normal_font_size = font::SIZE_SMALL;
const unsigned max_history = 100;
const std::string topic_img = "help/topic.png";
const std::string closed_section_img = "help/closed_section.png";
const std::string open_section_img = "help/open_section.png";
const std::string indentation_img = "help/indentation.png";
// The topic to open by default when opening the help dialog.
const std::string default_show_topic = "introduction_topic";
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

	BOOST_FOREACH(const config &section, cfg.child_range("section"))
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

	BOOST_FOREACH(const config &section, cfg.child_range("section"))
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
	else if (section_cfg != NULL) {
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
	if (cfg != NULL) {
		config const &toplevel_cfg = cfg->child("toplevel");
		parse_config_internal(cfg, toplevel_cfg ? &toplevel_cfg : NULL, sec);
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
	} else {
		std::vector<std::string> parts = utils::split(generator, ':', utils::STRIP_SPACES);
		if (parts[0] == "units" && parts.size()>1) {
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
		if (parts[0] == "units" && parts.size()>1) {
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
		if (parts.size()>1 && parts[0] == "contents") {
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
		generator_ = NULL;
	}
	return parsed_text_;
}

std::vector<topic> generate_time_of_day_topics(const bool /*sort_generated*/)
{
	std::vector<topic> topics;
	std::stringstream toplevel;

	if (! resources::tod_manager) {
		toplevel << N_("Only available during a scenario.");
		topics.push_back( topic("Time of Day Schedule", "..schedule", toplevel.str()) );
		return topics;
	}
	const std::vector<time_of_day>& times = resources::tod_manager->times();
	BOOST_FOREACH(const time_of_day& time, times)
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

		topics.push_back( topic(time.name.str(), id, text.str()) );
	}

	topics.push_back( topic("Time of Day Schedule", "..schedule", toplevel.str()) );
	return topics;
}

std::vector<topic> generate_weapon_special_topics(const bool sort_generated)
{
	std::vector<topic> topics;

	std::map<t_string, std::string> special_description;
	std::map<t_string, std::set<std::string, string_less> > special_units;

	BOOST_FOREACH(const unit_type_data::unit_type_map::value_type &i, unit_types.types())
	{
		const unit_type &type = i.second;
		// Only show the weapon special if we find it on a unit that
		// detailed description should be shown about.
		if (description_type(type) != FULL_DESCRIPTION)
		 	continue;

		std::vector<attack_type> attacks = type.attacks();
		for (std::vector<attack_type>::const_iterator it = attacks.begin();
					it != attacks.end(); ++it) {

			std::vector<std::pair<t_string, t_string> > specials = it->special_tooltips();
			for ( size_t i = 0; i != specials.size(); ++i )
			{
				special_description.insert(std::make_pair(specials[i].first, specials[i].second));

				if (!type.hide_help()) {
					//add a link in the list of units having this special
					std::string type_name = type.type_name();
					//check for variations (walking corpse/soulless etc)
					const std::string section_prefix = type.variations().empty() ? "" : "..";
					std::string ref_id = section_prefix + unit_prefix + type.id();
					//we put the translated name at the beginning of the hyperlink,
					//so the automatic alphabetic sorting of std::set can use it
					std::string link = make_link(type_name, ref_id);
					special_units[specials[i].first].insert(link);
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
		text << "\n\n" << _("<header>text='Units having this special attack'</header>") << "\n";
		std::set<std::string, string_less> &units = special_units[s->first];
		for (std::set<std::string, string_less>::iterator u = units.begin(); u != units.end(); ++u) {
			text << (*u) << "\n";
		}

		topics.push_back( topic(s->first, id, text.str()) );
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
	BOOST_FOREACH(const unit_type_data::unit_type_map::value_type &i, unit_types.types())
	{
		const unit_type &type = i.second;
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
					std::string const abil_desc =
						j >= desc_vec.size() ? "" : desc_vec[j].str();

					ability_description.insert(std::make_pair(abil_name, abil_desc));

					if (!type.hide_help()) {
						//add a link in the list of units having this ability
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
		text << "\n\n" << _("<header>text='Units having this ability'</header>") << "\n";
		std::set<std::string, string_less> &units = ability_units[a->first];
		for (std::set<std::string, string_less>::iterator u = units.begin(); u != units.end(); ++u) {
			text << (*u) << "\n";
		}

		topics.push_back( topic(a->first, id, text.str()) );
	}

	if (sort_generated)
		std::sort(topics.begin(), topics.end(), title_less());
	return topics;
}

std::vector<topic> generate_era_topics(const bool sort_generated, const std::string & era_id)
{
	std::vector<topic> topics;

	const config & era = game_cfg->find_child("era","id", era_id);
	if(era) {
		topics = generate_faction_topics(era, sort_generated);

		std::vector<std::string> faction_links;
		BOOST_FOREACH(const topic & t, topics) {
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

		text << "<header>text='" << _("Factions:") << "'</header>" << "\n";

		std::sort(faction_links.begin(), faction_links.end());
		BOOST_FOREACH(const std::string &link, faction_links) {
			text << link << "\n";
		}

		topic era_topic(era["name"], ".." + era_prefix + era["id"].str(), text.str());

		topics.push_back( era_topic );
	}
	return topics;
}

std::vector<topic> generate_faction_topics(const config & era, const bool sort_generated)
{
	std::vector<topic> topics;
	BOOST_FOREACH(const config &f, era.child_range("multiplayer_side")) {
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

		BOOST_FOREACH(const std::string & u_id, recruit_ids) {
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

		if (races.size() > 0) {
			std::set<std::string>::iterator it = races.begin();
			text << _("Races: ") << *(it++);
			while(it != races.end()) {
				text << ", " << *(it++);
			}
			text << "\n\n";
		}

		if (alignments.size() > 0) {
			std::set<std::string>::iterator it = alignments.begin();
			text << _("Alignments: ") << *(it++);
			while(it != alignments.end()) {
				text << ", " << *(it++);
			}
			text << "\n\n";
		}

		text << "<header>text='" << _("Leaders:") << "'</header>" << "\n";
		const std::vector<std::string> leaders =
				make_unit_links_list( utils::split(f["leader"]), true );
		BOOST_FOREACH(const std::string &link, leaders) {
			text << link << "\n";
		}

		text << "\n";

		text << "<header>text='" << _("Recruits:") << "'</header>" << "\n";
		const std::vector<std::string> recruit_links =
				make_unit_links_list( recruit_ids, true );
		BOOST_FOREACH(const std::string &link, recruit_links) {
			text << link << "\n";
		}

		const std::string name = f["name"];
		const std::string ref_id = faction_prefix + id;
		topics.push_back( topic(name, ref_id, text.str()) );
	}
	if (sort_generated)
		std::sort(topics.begin(), topics.end(), title_less());
	return topics;
}

static std::string best_str(bool best) {
	std::string lang_policy = (best ? _("Best of") : _("Worst of"));
	std::string color_policy = (best ? "green": "red");

	return "<format>color='" + color_policy + "' text='" + lang_policy + "'</format>";
}

typedef t_translation::t_list::const_iterator t_it;
// Gets an english desription of a terrain t_list alias behavior: "Best of cave, hills", "Worst of Swamp, Forest" etc.
static std::string print_behavior_description(t_it start, t_it end, const tdata_cache & tdata, bool first_level = true, bool begin_best = true)
{

	if (start == end) return "";
	if (*start == t_translation::MINUS || *start == t_translation::PLUS) return print_behavior_description(start+1, end, tdata, first_level, *start == t_translation::PLUS); //absorb any leading mode changes by calling again, with a new default value begin_best.

	boost::optional<t_it> last_change_pos;

	bool best = begin_best;
	for (t_it i = start; i != end; i++) {
		if ((best && *i == t_translation::MINUS) || (!best && *i == t_translation::PLUS)) {
			best = !best;
			last_change_pos = i;
		}
	}

	std::stringstream ss;

	if (!last_change_pos) {
		std::vector<std::string> names;
		for (t_it i = start; i != end; i++) {
			const terrain_type tt = tdata->get_terrain_info(*i);
			if (!tt.editor_name().empty())
				names.push_back(tt.editor_name());
		}

		if (names.size() == 0) return "";
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
		for (t_it i = *last_change_pos+1; i != end; i++) {
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
		BOOST_FOREACH(const std::string & s, names) {
			ss << ", " << s;
		}
		if (!first_level) ss << " )";
	}
	return ss.str();
}

class terrain_topic_generator: public topic_generator
{
	const terrain_type& type_;


public:
	terrain_topic_generator(const terrain_type& type) : type_(type) {}

	virtual std::string operator()() const {
		std::stringstream ss;

		if (!type_.icon_image().empty())
		ss << "<img>src='images/buttons/icon-base-32.png~RC(magenta>" << type_.id()
				<< ")~BLIT("<< "terrain/" << type_.icon_image() << "_30.png)" << "'</img> ";

		if (!type_.editor_image().empty())
			ss << "<img>src='" << type_.editor_image() << "'</img> ";

		ss << type_.help_topic_text().str() << "\n";

		tdata_cache tdata = load_terrain_types_data();

		if (!tdata) {
			WRN_HP << "When building terrain help topics, we couldn't acquire any terrain types data\n";
			return ss.str();
		}

		if (!(type_.union_type().size() == 1 && type_.union_type()[0] == type_.number() && type_.is_nonnull())) {

			const t_translation::t_list& underlying_mvt_terrains = tdata->underlying_mvt_terrain(type_.number());

			ss << "\n" << N_("Base Terrain: ");

			bool first = true;
			BOOST_FOREACH(const t_translation::t_terrain& underlying_terrain, underlying_mvt_terrains) {
				const terrain_type& mvt_base = tdata->get_terrain_info(underlying_terrain);

				if (mvt_base.editor_name().empty()) continue;
				if (!first) ss << ",";
				else first = false;
				ss << make_link(mvt_base.editor_name(), ".." + terrain_prefix + mvt_base.id());
			}

			ss << "\n";

			ss << "\n" << N_("Movement properties: ");
			ss << print_behavior_description(underlying_mvt_terrains.begin(), underlying_mvt_terrains.end(), tdata) << "\n";

			const t_translation::t_list& underlying_def_terrains = tdata->underlying_def_terrain(type_.number());
			ss << "\n" << N_("Defense properties: ");
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

			const t_translation::t_list& underlying_mvt_terrains = tdata->underlying_mvt_terrain(type_.number());
			ss << "\nDebug Mvt Description String:";
			BOOST_FOREACH(const t_translation::t_terrain & t, underlying_mvt_terrains) {
				ss << " " << t;
			}

			const t_translation::t_list& underlying_def_terrains = tdata->underlying_def_terrain(type_.number());
			ss << "\nDebug Def Description String:";
			BOOST_FOREACH(const t_translation::t_terrain & t, underlying_def_terrains) {
				ss << " " << t;
			}

		}

		return ss.str();
	}

};

//Typedef to help with formatting list of traits
typedef std::pair<std::string, std::string> trait_data;

//Helper function for printing a list of trait data
static void print_trait_list(std::stringstream & ss, const std::vector<trait_data> & l)
{
	size_t i = 0;
	ss << make_link(l[i].first, l[i].second);

	for(i++; i < l.size(); i++) {
		ss << ", " << make_link(l[i].first,l[i].second);
	}
}

class unit_topic_generator: public topic_generator
{
	const unit_type& type_;
	const std::string variation_;
	typedef std::pair< std::string, unsigned > item;
	void push_header(std::vector< item > &row,  const std::string& name) const {
		row.push_back(item(bold(name), font::line_width(name, normal_font_size, TTF_STYLE_BOLD)));
	}
public:
	unit_topic_generator(const unit_type &t, std::string variation="") : type_(t), variation_(variation) {}
	virtual std::string operator()() const {
		// Force the lazy loading to build this unit.
		unit_types.build_unit_type(type_, unit_type::WITHOUT_ANIMATIONS);

		std::stringstream ss;
		std::string clear_stringstream;
		const std::string detailed_description = type_.unit_description();
		const unit_type& female_type = type_.get_gender_unit_type(unit_race::FEMALE);
		const unit_type& male_type = type_.get_gender_unit_type(unit_race::MALE);

		// Show the unit's image and its level.
#ifdef LOW_MEM
		ss << "<img>src='" << male_type.image() << "~XBRZ(2)'</img> ";
#else
		ss << "<img>src='" << male_type.image() << "~RC(" << male_type.flag_rgb() << ">1)~XBRZ(2)" << "'</img> ";
#endif

		if (&female_type != &male_type)
#ifdef LOW_MEM
			ss << "<img>src='" << female_type.image() << "~XBRZ(2)'</img> ";
#else
			ss << "<img>src='" << female_type.image() << "~RC(" << female_type.flag_rgb() << ">1)~XBRZ(2)" << "'</img> ";
#endif


		ss << "<format>font_size=" << font::relative_size(11) << " text=' " << escape(_("level"))
		   << " " << type_.level() << "'</format>";

		const std::string &male_portrait = male_type.small_profile();
		const std::string &female_portrait = female_type.small_profile();

		if (male_portrait.empty() == false && male_portrait != male_type.image()) {
			ss << "<img>src='" << male_portrait << "~BG()' align='right'</img> ";
		}

		if (female_portrait.empty() == false && female_portrait != male_portrait && female_portrait != female_type.image()) {
			ss << "<img>src='" << female_portrait << "~BG()' align='right'</img> ";
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

				BOOST_FOREACH(const std::string &adv, adv_units)
				{
					const unit_type *type = unit_types.find(adv, unit_type::HELP_INDEXED);
					if (!type || type->hide_help()) continue;

					if (first) {
						if (reverse)
							ss << _("Advances from: ");
						else
							ss << _("Advances to: ");
						first = false;
					} else
						ss << ", ";

					std::string lang_unit = type->type_name();
					std::string ref_id;
					if (description_type(*type) == FULL_DESCRIPTION) {
						const std::string section_prefix = type->variations().empty() ? "" : "..";
						ref_id = section_prefix + unit_prefix + type->id();
					} else {
						ref_id = unknown_unit_topic;
						lang_unit += " (?)";
					}
					ss << make_link(lang_unit, ref_id);
				}
				if (!first) ss << "\n";

				reverse = !reverse; //switch direction
			} while(reverse != first_reverse_value); // don't restart
		}

		const unit_type* parent = variation_.empty() ? &type_ :
				unit_types.find(type_.id(), unit_type::HELP_INDEXED);
		if (!variation_.empty()) {
			ss << _("Base unit: ") << make_link(parent->type_name(), ".." + unit_prefix + type_.id()) << "\n";
		} else {
			bool first = true;
			BOOST_FOREACH(const std::string& base_id, utils::split(type_.get_cfg()["base_ids"])) {
				if (first) {
					ss << _("Base units: ");
					first = false;
				}
				const unit_type* base_type = unit_types.find(base_id, unit_type::HELP_INDEXED);
				const std::string section_prefix = base_type->variations().empty() ? "" : "..";
				ss << make_link(base_type->type_name(), section_prefix + unit_prefix + base_id) << "\n";
			}
		}

		bool first = true;
		BOOST_FOREACH(const std::string &var_id, parent->variations()) {
			const unit_type &type = parent->get_variation(var_id);

			if(type.hide_help()) {
				continue;
			}

			if (first) {
				ss << _("Variations: ");
				first = false;
			} else
				ss << ", ";

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

		// Print the race of the unit, cross-reference it to the
		// respective topic.
		const std::string race_id = type_.race_id();
		std::string race_name = type_.race()->plural_name();
		if ( race_name.empty() ) {
			race_name = _ ("race^Miscellaneous");
		}
		ss << _("Race: ");
		ss << make_link(race_name, "..race_" + race_id);
		ss << "\n\n";

		// Print the possible traits of the unit, cross-reference them
		// to their respective topics.
		config::const_child_itors traits = type_.possible_traits();
		if (traits.first != traits.second && type_.num_traits() > 0) {
			std::vector<trait_data> must_have_traits;
			std::vector<trait_data> random_traits;

			BOOST_FOREACH(const config & trait, traits) {
				const std::string trait_name = trait["male_name"];
				std::string lang_trait_name = translation::gettext(trait_name.c_str());
				const std::string ref_id = "traits_"+trait["id"].str();
				((trait["availability"].str() == "musthave") ? must_have_traits : random_traits).push_back(std::make_pair(lang_trait_name, ref_id));
			}

			bool line1 = !must_have_traits.empty();
			bool line2 = !random_traits.empty() && type_.num_traits() - must_have_traits.size() > 0;

			if (line1) {
				std::string traits_label = _("Traits");
				ss << traits_label;
				if (line2) {
					std::stringstream must_have_count;
					must_have_count << " (" << must_have_traits.size() << ") : ";
					std::stringstream random_count;
					random_count << " (" << (type_.num_traits() - must_have_traits.size()) << ") : ";

					int second_line_whitespace = font::line_width(traits_label+must_have_count.str(), normal_font_size) - font::line_width(random_count.str(), normal_font_size); // This ensures that the second line is justified so that the ':' characters are aligned.

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
					ss << _("Traits") << " (" << type_.num_traits() << ") : ";
					print_trait_list(ss, random_traits);
					ss << "\n\n";
				}
			}
		}

		// Print the abilities the units has, cross-reference them
		// to their respective topics. TODO: Update this according to musthave trait effects, similar to movetype below
		if (!type_.abilities().empty()) {
			ss << _("Abilities: ");
			for(std::vector<t_string>::const_iterator ability_it = type_.abilities().begin(),
				 ability_end = type_.abilities().end();
				 ability_it != ability_end; ++ability_it) {
				const std::string ref_id = "ability_" + ability_it->base_str();
				std::string lang_ability = translation::gettext(ability_it->c_str());
				ss << make_link(lang_ability, ref_id);
				if (ability_it + 1 != ability_end)
					ss << ", ";
			}
			ss << "\n\n";
		}

		// Print the extra AMLA upgrade abilities, cross-reference them
		// to their respective topics.
		if (!type_.adv_abilities().empty()) {
			ss << _("Ability Upgrades: ");
			for(std::vector<t_string>::const_iterator ability_it = type_.adv_abilities().begin(),
				 ability_end = type_.adv_abilities().end();
				 ability_it != ability_end; ++ability_it) {
				const std::string ref_id = "ability_" + ability_it->base_str();
				std::string lang_ability = translation::gettext(ability_it->c_str());
				ss << make_link(lang_ability, ref_id);
				if (ability_it + 1 != ability_end)
					ss << ", ";
			}
			ss << "\n\n";
		}

		// Print some basic information such as HP and movement points.
		// TODO: Make this info update according to musthave traits, similar to movetype below.
		ss << _("HP: ") << type_.hitpoints() << jump(30)
		   << _("Moves: ") << type_.movement() << jump(30);
		if (type_.vision() != type_.movement())
			ss << _("Vision: ") << type_.vision() << jump(30);
		if (type_.jamming() > 0)
			ss << _("Jamming: ") << type_.jamming() << jump(30);
		ss << _("Cost: ") << type_.cost() << jump(30)
		   << _("Alignment: ")
		   << make_link(type_.alignment_description(type_.alignment(), type_.genders().front()), "time_of_day")
		   << jump(30);
		if (type_.can_advance())
			ss << _("Required XP: ") << type_.experience_needed();

		// Print the detailed description about the unit.
		ss << "\n\n" << detailed_description;

		// Print the different attacks a unit has, if it has any.
		std::vector<attack_type> attacks = type_.attacks();
		if (!attacks.empty()) {
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
			for(std::vector<attack_type>::const_iterator attack_it = attacks.begin(),
				 attack_end = attacks.end();
				 attack_it != attack_end; ++attack_it) {
				std::string lang_weapon = attack_it->name();
				std::string lang_type = string_table["type_" + attack_it->type()];
				std::vector<item> row;
				std::stringstream attack_ss;
				attack_ss << "<img>src='" << (*attack_it).icon() << "'</img>";
				row.push_back(std::make_pair(attack_ss.str(),
							     image_width(attack_it->icon())));
				push_tab_pair(row, lang_weapon);
				push_tab_pair(row, lang_type);
				attack_ss.str(clear_stringstream);
				attack_ss << attack_it->damage() << utils::unicode_en_dash << attack_it->num_attacks() << " " << attack_it->accuracy_parry_description();
				push_tab_pair(row, attack_ss.str());
				attack_ss.str(clear_stringstream);
				if ((*attack_it).min_range() > 1 || (*attack_it).max_range() > 1)
					attack_ss << (*attack_it).min_range() << "-" << (*attack_it).max_range() << ' ';
				attack_ss << string_table["range_" + (*attack_it).range()];
				push_tab_pair(row, attack_ss.str());
				attack_ss.str(clear_stringstream);
				// Show this attack's special, if it has any. Cross
				// reference it to the section describing the
				// special.
				std::vector<std::pair<t_string, t_string> > specials = attack_it->special_tooltips();
				if(!specials.empty())
				{
					std::string lang_special = "";
					const size_t specials_size = specials.size();
					for ( size_t i = 0; i != specials_size; ++i) {
						const std::string ref_id = std::string("weaponspecial_")
							+ specials[i].first.base_str();
						lang_special = (specials[i].first);
						attack_ss << make_link(lang_special, ref_id);
						if ( i+1 != specials_size )
							attack_ss << ", "; //comma placed before next special
					}
					row.push_back(std::make_pair(attack_ss.str(),
						font::line_width(lang_special, normal_font_size)));
				}
				table.push_back(row);
			}
			ss << generate_table(table);
		}

		// Generate the movement type of the unit, with resistance, defense, movement, jamming and vision data updated according to any 'musthave' traits which always apply
		movetype movement_type = type_.movement_type();
		traits = type_.possible_traits();
		if (traits.first != traits.second && type_.num_traits() > 0) {
			BOOST_FOREACH(const config & t, traits) {
				if (t["availability"].str() == "musthave") {
					BOOST_FOREACH (const config & effect, t.child_range("effect")) {
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
		for(utils::string_map::const_iterator dam_it = dam_tab.begin(), dam_end = dam_tab.end();
			 dam_it != dam_end; ++dam_it) {
			std::vector<item> row;
			int resistance = 100 - atoi((*dam_it).second.c_str());
			char resi[16];
			snprintf(resi,sizeof(resi),"% 4d%%",resistance);	// range: -100% .. +70%
			std::string resist = resi;
			const size_t pos = resist.find('-');
			if (pos != std::string::npos)
				resist.replace(pos, 1, utils::unicode_minus);
			std::string color = unit_helper::resistance_color(resistance);
			std::string lang_weapon = string_table["type_" + dam_it->first];
			push_tab_pair(row, lang_weapon);
			std::stringstream str;
			str << "<format>color=" << color << " text='"<< resist << "'</format>";
			const std::string markup = str.str();
			str.str(clear_stringstream);
			str << resist;
			row.push_back(std::make_pair(markup,
						     font::line_width(str.str(), normal_font_size)));
			resistance_table.push_back(row);
		}
		ss << generate_table(resistance_table);

		if (tdata_cache tdata = load_terrain_types_data()) {
			// Print the terrain modifier table of the unit.
			ss << "\n\n<header>text='" << escape(_("Terrain Modifiers"))
			   << "'</header>\n\n";
			std::vector<item> first_row;
			table_spec table;
			push_header(first_row, _("Terrain"));
			push_header(first_row, _("Defense"));
			push_header(first_row, _("Movement Cost"));

			const bool has_terrain_defense_caps = movement_type.has_terrain_defense_caps(preferences::encountered_terrains());
			if ( has_terrain_defense_caps )
				push_header(first_row, _("Defense Capped"));

			const bool has_vision = type_.movement_type().has_vision_data();
			if ( has_vision )
				push_header(first_row, _("Vision Cost"));
			const bool has_jamming = type_.movement_type().has_jamming_data();
			if ( has_jamming )
				push_header(first_row, _("Jamming Cost"));

			table.push_back(first_row);
			std::set<t_translation::t_terrain>::const_iterator terrain_it =
				preferences::encountered_terrains().begin();


			for (; terrain_it != preferences::encountered_terrains().end();
					++terrain_it) {
				const t_translation::t_terrain terrain = *terrain_it;
				if (terrain == t_translation::FOGGED || terrain == t_translation::VOID_TERRAIN || terrain == t_translation::OFF_MAP_USER)
					continue;
				const terrain_type& info = tdata->get_terrain_info(terrain);

				if (info.union_type().size() == 1 && info.union_type()[0] == info.number() && info.is_nonnull()) {
					std::vector<item> row;
					const std::string& name = info.name();
					const std::string& id = info.id();
					const int moves = movement_type.movement_cost(terrain);
					const int views = movement_type.vision_cost(terrain);
					const int jams  = movement_type.jamming_cost(terrain);

					bool high_res = false;
					const std::string tc_base = high_res ? "images/buttons/icon-base-32.png" : "images/buttons/icon-base-16.png";
					const std::string terrain_image = "icons/terrain/terrain_type_" + id + (high_res ? "_30.png" : ".png");

					const std::string final_image = tc_base + "~RC(magenta>" + id + ")~BLIT(" + terrain_image + ")";

					row.push_back(std::make_pair( "<img>src='" + final_image + "'</img> " +
							make_link(name, "..terrain_" + id),
							font::line_width(name, normal_font_size) + (high_res ? 32 : 16) ));

					//defense  -  range: +10 % .. +70 %
					const int defense =
							100 - movement_type.defense_modifier(terrain);
					std::string color;
					if (defense <= 10)
						color = "red";
					else if (defense <= 30)
						color = "yellow";
					else if (defense <= 50)
						color = "white";
					else
						color = "green";

					std::stringstream str;
					str << "<format>color=" << color << " text='"<< defense << "%'</format>";
					std::string markup = str.str();
					str.str(clear_stringstream);
					str << defense << "%";
					row.push_back(std::make_pair(markup,
							font::line_width(str.str(), normal_font_size)));

					//movement  -  range: 1 .. 5, movetype::UNREACHABLE=impassable
					str.str(clear_stringstream);
					const bool cannot_move = moves > type_.movement();
					if (cannot_move)		// cannot move in this terrain
						color = "red";
					else if (moves > 1)
						color = "yellow";
					else
						color = "white";
					str << "<format>color=" << color << " text='";
					// A 5 MP margin; if the movement costs go above
					// the unit's max moves + 5, we replace it with dashes.
					if(cannot_move && (moves > type_.movement() + 5)) {
						str << utils::unicode_figure_dash;
					} else {
						str << moves;
					}
					str << "'</format>";
					markup = str.str();
					str.str(clear_stringstream);
					str << moves;
					row.push_back(std::make_pair(markup,
							font::line_width(str.str(), normal_font_size)));

					//defense cap
					if ( has_terrain_defense_caps ) {
						str.str(clear_stringstream);
						const bool has_cap = movement_type.get_defense().capped(terrain);
						if (has_cap) {
							str << "<format>color='"<< color <<"' text='" << defense << "%'</format>";
						} else {
							str << "<format>color=white text='" << utils::unicode_figure_dash << "'</format>";
						}
						markup = str.str();
						str.str(clear_stringstream);
						if (has_cap) {
							str << defense << '%';
						} else {
							str << utils::unicode_figure_dash;
						}
						row.push_back(std::make_pair(markup,
							font::line_width(str.str(), normal_font_size)));
					}

					//vision
					if ( has_vision ) {
						str.str(clear_stringstream);
						const bool cannot_view = views > type_.vision();
						if (cannot_view)		// cannot view in this terrain
							color = "red";
						else if ( views > moves )
							color = "yellow";
						else if ( views == moves )
							color = "white";
						else
							color = "green";
						str << "<format>color=" << color << " text='";
						// A 5 MP margin; if the vision costs go above
						// the unit's vision + 5, we replace it with dashes.
						if(cannot_view && (views > type_.vision() + 5)) {
							str << utils::unicode_figure_dash;
						} else {
							str << views;
						}
						str << "'</format>";
						markup = str.str();
						str.str(clear_stringstream);
						str << views;
						row.push_back(std::make_pair(markup,
								font::line_width(str.str(), normal_font_size)));
					}

					//jamming
					if ( has_jamming ) {
						str.str(clear_stringstream);
						const bool cannot_jam = jams > type_.jamming();
						if ( cannot_jam )		// cannot jamm in this terrain
							color = "red";
						else if ( jams > views )
							color = "yellow";
						else if ( jams == views )
							color = "white";
						else
							color = "green";
						str << "<format>color=" << color << " text='";
						// A 5 MP margin; if the jamming costs go above
						// the unit's jamming + 5, we replace it with dashes.
						if ( cannot_jam  &&  jams > type_.jamming() + 5 ) {
							str << utils::unicode_figure_dash;
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
};

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
			const std::string section_prefix = type->variations().empty() ? "" : "..";
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
	BOOST_FOREACH(const std::string &type_id, type_id_list) {
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

	BOOST_FOREACH(const unit_type_data::unit_type_map::value_type &i, unit_types.types())
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
	BOOST_FOREACH(const config & era, game_cfg->child_range("era")) {
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
	tdata_cache tdata = load_terrain_types_data();

	if (!tdata) {
		WRN_HP << "When building terrain help sections, couldn't acquire terrain types data, aborting.\n";
		return;
	}

	std::map<std::string, section> base_map;

	const t_translation::t_list& t_listi = tdata->list();

	BOOST_FOREACH(const t_translation::t_terrain& t, t_listi) {

		const terrain_type& info = tdata->get_terrain_info(t);

		bool hidden = info.is_combined() || info.hide_help();

		if (preferences::encountered_terrains().find(t)
				== preferences::encountered_terrains().end() && !info.is_overlay())
			hidden = true;

		topic terrain_topic;
		terrain_topic.title = info.editor_name();
		terrain_topic.id    = hidden_symbol(hidden) + terrain_prefix + info.id();
		terrain_topic.text  = new terrain_topic_generator(info);

		t_translation::t_list base_terrains = tdata->underlying_union_terrain(t);
		BOOST_FOREACH(const t_translation::t_terrain& base, base_terrains) {

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
	BOOST_FOREACH(const unit_type_data::unit_type_map::value_type &i, unit_types.types()) {
		const unit_type &type = i.second;

		if (type.race_id() != race)
			continue;

		if (type.variations().empty())
			continue;

		section base_unit;
		BOOST_FOREACH(const std::string &variation_id, type.variations()) {
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

	BOOST_FOREACH(const unit_type_data::unit_type_map::value_type &i, unit_types.types())
	{
		const unit_type &type = i.second;

		if (type.race_id() != race)
			continue;

		UNIT_DESCRIPTION_TYPE desc_type = description_type(type);
		if (desc_type != FULL_DESCRIPTION)
			continue;

		const std::string type_name = type.type_name();
		const std::string real_prefix = type.variations().empty() ? "" : "..";
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
		BOOST_FOREACH(const config &additional_topic, r->additional_topics())
		  {
		    std::string id = additional_topic["id"];
		    std::string title = additional_topic["title"];
		    std::string text = additional_topic["text"];
		    //topic additional_topic(title, id, text);
		    topics.push_back(topic(title,id,text));
			std::string link = make_link(title, id);
			race_topics.insert(link);
		  }
	} else {
		race_name = _ ("race^Miscellaneous");
		// description =  _("Here put the description of the Miscellaneous race");
	}

	std::stringstream text;
	text << race_description << "\n\n";

	if (alignments.size() > 0) {
		std::set<std::string>::iterator it = alignments.begin();
		text << (alignments.size() > 1 ? _("Alignments: ") : _("Alignment: ")) << *(it++);
		while(it != alignments.end()) {
			text << ", " << *(it++);
		}
		text << "\n\n";
	}

	text << _("<header>text='Units of this race'</header>") << "\n";
	for (std::set<std::string, string_less>::iterator u = race_units.begin(); u != race_units.end(); ++u) {
		text << (*u) << "\n";
	}

	topics.push_back(topic(race_name, race_id, text.str()) );

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
	std::vector<std::string> about_lines = about::get_text();
	std::vector<std::string> res_lines;
	std::transform(about_lines.begin(), about_lines.end(), std::back_inserter(res_lines),
				   about_text_formatter());
	res_lines.erase(std::remove(res_lines.begin(), res_lines.end(), ""), res_lines.end());
	std::string text = utils::join(res_lines, "\n");
	return text;
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
					topics_links.push_back(link(topic_cfg["title"], id));
			}
		}

		if (section_cfg["sort_topics"] == "yes") {
			std::sort(topics_links.begin(),topics_links.end());
		}

		std::vector<link>::iterator l;
		for (l = topics_links.begin(); l != topics_links.end(); ++l) {
			std::string link = make_link(l->first, l->second);
			res << link << "\n";
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
				res << link << "\n";
			}
		}

		std::vector<topic>::const_iterator t;
		for (t = topics.begin(); t != topics.end(); ++t) {
			if (is_visible_id(t->id)) {
				std::string link = make_link(t->title, t->id);
				res << link << "\n";
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

help_menu::help_menu(CVideo &video, section const &toplevel, int max_height) :
	gui::menu(video, empty_string_vector, true, max_height, -1, NULL, &gui::menu::bluebg_style),
	visible_items_(),
	toplevel_(toplevel),
	expanded_(),
	restorer_(),
	chosen_topic_(NULL),
	selected_item_(&toplevel, "")
{
	silent_ = true; //silence the default menu sounds
	update_visible_items(toplevel_);
	display_visible_items();
	if (!visible_items_.empty())
		selected_item_ = visible_items_.front();
}

bool help_menu::expanded(const section &sec)
{
	return expanded_.find(&sec) != expanded_.end();
}

void help_menu::expand(const section &sec)
{
	if (sec.id != "toplevel" && expanded_.insert(&sec).second) {
		sound::play_UI_sound(game_config::sounds::menu_expand);
	}
}

void help_menu::contract(const section &sec)
{
	if (expanded_.erase(&sec)) {
		sound::play_UI_sound(game_config::sounds::menu_contract);
	}
}

void help_menu::update_visible_items(const section &sec, unsigned level)
{
	if (level == 0) {
		// Clear if this is the top level, otherwise append items.
		visible_items_.clear();
	}
	section_list::const_iterator sec_it;
	for (sec_it = sec.sections.begin(); sec_it != sec.sections.end(); ++sec_it) {
		if (is_visible_id((*sec_it)->id)) {
			const std::string vis_string = get_string_to_show(*(*sec_it), level + 1);
			visible_items_.push_back(visible_item(*sec_it, vis_string));
			if (expanded(*(*sec_it))) {
				update_visible_items(*(*sec_it), level + 1);
			}
		}
	}
	topic_list::const_iterator topic_it;
	for (topic_it = sec.topics.begin(); topic_it != sec.topics.end(); ++topic_it) {
		if (is_visible_id(topic_it->id)) {
			const std::string vis_string = get_string_to_show(*topic_it, level + 1);
			visible_items_.push_back(visible_item(&(*topic_it), vis_string));
		}
	}
}

std::string help_menu::indented_icon(const std::string& icon, const unsigned level) {
	std::stringstream to_show;
	for (unsigned i = 1; i < level; ++i) 	{
		to_show << IMAGE_PREFIX << indentation_img << IMG_TEXT_SEPARATOR;
	}

	to_show << IMAGE_PREFIX << icon;
	return to_show.str();
}

std::string help_menu::get_string_to_show(const section &sec, const unsigned level)
{
	std::stringstream to_show;
	to_show << indented_icon(expanded(sec) ? open_section_img : closed_section_img, level)
		 << IMG_TEXT_SEPARATOR << sec.title;
	return to_show.str();
}

std::string help_menu::get_string_to_show(const topic &topic, const unsigned level)
{
	std::stringstream to_show;
	to_show <<  indented_icon(topic_img, level)
		<< IMG_TEXT_SEPARATOR << topic.title;
	return to_show.str();
}

bool help_menu::select_topic_internal(const topic &t, const section &sec)
{
	topic_list::const_iterator tit =
		std::find(sec.topics.begin(), sec.topics.end(), t);
	if (tit != sec.topics.end()) {
		// topic starting with ".." are assumed as rooted in the parent section
		// and so only expand the parent when selected
		if (t.id.size()<2 || t.id[0] != '.' || t.id[1] != '.')
			expand(sec);
		return true;
	}
	section_list::const_iterator sit;
	for (sit = sec.sections.begin(); sit != sec.sections.end(); ++sit) {
		if (select_topic_internal(t, *(*sit))) {
			expand(sec);
			return true;
		}
	}
	return false;
}

void help_menu::select_topic(const topic &t)
{
	if (selected_item_ == t) {
		// The requested topic is already selected.
		return;
	}
	if (select_topic_internal(t, toplevel_)) {
		update_visible_items(toplevel_);
		for (std::vector<visible_item>::const_iterator it = visible_items_.begin();
			 it != visible_items_.end(); ++it) {
			if (*it == t) {
				selected_item_ = *it;
				break;
			}
		}
		display_visible_items();
	}
}

int help_menu::process()
{
	int res = menu::process();
	int mousex, mousey;
	SDL_GetMouseState(&mousex,&mousey);

	if (!visible_items_.empty() &&
            static_cast<size_t>(res) < visible_items_.size()) {

		selected_item_ = visible_items_[res];
		const section* sec = selected_item_.sec;
		if (sec != NULL) {
			// Check how we click on the section
			int x = mousex - menu::location().x;

			const std::string icon_img = expanded(*sec) ? open_section_img : closed_section_img;
			// we remove the right thickness (ne present between icon and text)
			int text_start = style_->item_size(indented_icon(icon_img, sec->level)).w - style_->get_thickness();

			// NOTE: if you want to forbid click to the left of the icon
			// also check x >= text_start-image_width(icon_img)
			if (menu::double_clicked() || x < text_start) {
				// Open or close a section if we double-click on it
				// or do simple click on the icon.
				expanded(*sec) ? contract(*sec) : expand(*sec);
				update_visible_items(toplevel_);
				display_visible_items();
			} else if (x >= text_start){
				// click on title open the topic associated to this section
				chosen_topic_ = find_topic(toplevel, ".."+sec->id );
			}
		} else if (selected_item_.t != NULL) {
			/// Choose a topic if it is clicked.
			chosen_topic_ = selected_item_.t;
		}
	}
	return res;
}

const topic *help_menu::chosen_topic()
{
	const topic *ret = chosen_topic_;
	chosen_topic_ = NULL;
	return ret;
}

void help_menu::display_visible_items()
{
	std::vector<std::string> menu_items;
	for(std::vector<visible_item>::const_iterator items_it = visible_items_.begin(),
		 end = visible_items_.end(); items_it != end; ++items_it) {
		std::string to_show = items_it->visible_string;
		if (selected_item_ == *items_it)
			to_show = std::string("*") + to_show;
		menu_items.push_back(to_show);
	}
	set_items(menu_items, false, true);
}

help_menu::visible_item::visible_item(const section *_sec, const std::string &vis_string) :
	t(NULL), sec(_sec), visible_string(vis_string) {}

help_menu::visible_item::visible_item(const topic *_t, const std::string &vis_string) :
	t(_t), sec(NULL), visible_string(vis_string) {}

bool help_menu::visible_item::operator==(const section &_sec) const
{
	return sec != NULL && *sec == _sec;
}

bool help_menu::visible_item::operator==(const topic &_t) const
{
	return t != NULL && *t == _t;
}

bool help_menu::visible_item::operator==(const visible_item &vis_item) const
{
	return t == vis_item.t && sec == vis_item.sec;
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
		if (t != NULL) {
			return t;
		}
	}
	return NULL;
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
		if (s != NULL) {
			return s;
		}
	}
	return NULL;
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
	if (ss.str() != "") {
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
	if (ss.str() != "") {
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

SDL_Color string_to_color(const std::string &cmp_str)
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
	toplevel.clear();
	hidden_sections.clear();
	if (game_cfg != NULL) {
		const config *help_config = &game_cfg->child("help");
		if (!*help_config) {
			help_config = &dummy_cfg;
		}
		try {
			toplevel = parse_config(help_config);
			// Create a config object that contains everything that is
			// not referenced from the toplevel element. Read this
			// config and save these sections and topics so that they
			// can be referenced later on when showing help about
			// specified things, but that should not be shown when
			// opening the help browser in the default manner.
			config hidden_toplevel;
			std::stringstream ss;
			BOOST_FOREACH(const config &section, help_config->child_range("section"))
			{
				const std::string id = section["id"];
				if (find_section(toplevel, id) == NULL) {
					// This section does not exist referenced from the
					// toplevel. Hence, add it to the hidden ones if it
					// is not referenced from another section.
					if (!section_is_referenced(id, *help_config)) {
						if (ss.str() != "") {
							ss << ",";
						}
						ss << id;
					}
				}
			}
			hidden_toplevel["sections"] = ss.str();
			ss.str("");
			BOOST_FOREACH(const config &topic, help_config->child_range("topic"))
			{
				const std::string id = topic["id"];
				if (find_topic(toplevel, id) == NULL) {
					if (!topic_is_referenced(id, *help_config)) {
						if (ss.str() != "") {
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

} // end namespace help
