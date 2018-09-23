/*
   Copyright (C) 2018 by the Battle for Wesnoth Project https://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "help/section.hpp"

#include "config.hpp"
#include "formatter.hpp"
#include "gettext.hpp"
#include "help/constants.hpp"
#include "help/manager.hpp"
#include "help/section_generators.hpp"
#include "help/topic_generators.hpp"
#include "help/utils.hpp"
#include "log.hpp"
#include "serialization/string_utils.hpp"

#include <iterator>

static lg::log_domain log_help("help");
#define WRN_HP LOG_STREAM(warn, log_help)
#define DBG_HP LOG_STREAM(debug, log_help)

namespace help
{
section::section(const config& section_cfg, const help_manager* manager)
	: id("toplevel")
	, title()
	, topics_()
	, sections_()
	, recursion_level_(0)
	, manager_(manager)
{
	assert(manager_ && "Y U NO provide help manager!");
	initialize(section_cfg);
}

section::section(const config& section_cfg, const section& parent)
	: id(section_cfg["id"])
	, title(section_cfg["title"])
	, topics_()
	, sections_()
	, recursion_level_(parent.recursion_level_ + 1)
	, manager_(parent.manager_)
{
	assert(manager_);
	initialize(section_cfg);
}

void section::initialize(const config& section_cfg)
{
	// If we're in too deep, exit.
	if(recursion_level_ > max_section_recursion_level) {
		throw max_recursion_reached("Maximum section depth has been reached. Possible circular dependency?");
	}

	if(section_cfg.empty()) {
		return; // TODO: throw something?
	}

	//
	// Parse static sub-sections.
	//
	for(const std::string& section_id : utils::quoted_split(section_cfg["sections"])) {
		if(const config& child_cfg = manager_->get_section_config(section_id)) {
			if(!is_valid_id(section_id)) {
				throw parse_error(formatter() << "Invalid ID, used for internal purposes: '" << id << "'");
			}

			add_section(child_cfg);
		} else {
			throw parse_error(formatter()
				<< "Help section '" << section_id << "' referenced from '" << id << "' but could not be found.");
		}
	}

	//
	// Generate dynamic sub-sections.
	//
	generate_sections(section_cfg["sections_generator"].str());

	if(section_cfg["sort_sections"] == "yes") {
		std::sort(sections_.begin(), sections_.end(), [](const section_ptr& lhs, const section_ptr& rhs) {
			return translation::compare(lhs->title, rhs->title) < 0; }
		);
	}

	const config::attribute_value& st = section_cfg["sort_topics"];

	bool sort_topics = false;
	bool sort_generated = false;

	if(st == "yes") {
		sort_topics = true;
		sort_generated = false;
	} else if(st == "no") {
		sort_topics = false;
		sort_generated = false;
	} else if(st == "generated") {
		sort_topics = false;
		sort_generated = true;
	} else if(!st.empty()) {
		throw parse_error(formatter() << "Invalid sort option: '" << st << "'");
	}

	//
	// Parse static section topics
	//
	for(const std::string& topic_id : utils::quoted_split(section_cfg["topics"])) {
		if(const config& topic_cfg = manager_->get_topic_config(topic_id)) {
			if(!is_valid_id(topic_id)) {
				throw parse_error(formatter() << "Invalid ID, used for internal purposes: '" << id << "'");
			}

			std::ostringstream text;
			text << topic_cfg["text"];
			text << generate_table_of_contents(topic_cfg["generator"]);

			// We don't need to use add_topic here since we don't need a special text generator.
			topics_.emplace_back(topic_id, topic_cfg["title"], text.str());
		} else {
			throw parse_error(formatter()
				<< "Help-topic '" << topic_id << "' referenced from '" << id << "' but could not be found.");
		}
	}

	//
	// Generate dynamic topics.
	//
	generate_topics(section_cfg["generator"], sort_generated);

	if(sort_topics) {
		this->sort_topics();
	}
}

void section::generate_sections(const std::string& generator_type)
{
	if(generator_type == "races") {
		generate_races_sections(*this);
	} else if(generator_type == "terrains") {
		generate_terrain_sections(*this);
	} else if(generator_type == "eras") {
		DBG_HP << "Generating eras...\n";
		generate_era_sections(*this);
	} else {
		std::vector<std::string> parts = utils::split(generator_type, ':', utils::STRIP_SPACES);

		if(parts.size() > 1 && parts[0] == "units") {
			generate_unit_sections(*this, parts[1]);
		} else if(!generator_type.empty()) {
			WRN_HP << "Unknown section generator: " << generator_type << "\n";
		}
	}
}

void section::generate_topics(const std::string& generator_type, const bool sort_generated)
{
	if(generator_type.empty()) {
		return;
	}

	topic_list res;

	if(generator_type == "abilities") {
		res = generate_ability_topics(sort_generated);
	} else if(generator_type == "weapon_specials") {
		res = generate_weapon_special_topics(sort_generated);
	} else if(generator_type == "time_of_days") {
		res = generate_time_of_day_topics(sort_generated);
	} else if(generator_type == "traits") {
		res = generate_trait_topics(sort_generated);
	} else {
		std::vector<std::string> parts = utils::split(generator_type, ':', utils::STRIP_SPACES);

		if(parts.size() > 1 && parts[0] == "units") {
			res = generate_unit_topics(sort_generated, parts[1]);
		} else if (parts[0] == "era" && parts.size()>1) {
			res = generate_era_topics(sort_generated, parts[1]);
		} else {
			WRN_HP << "Unknown topic generator: " << generator_type << "\n";
		}
	}

	// Add the generated topics to this section.
	topics_.splice(topics_.end(), res);
}

std::string section::generate_table_of_contents(const std::string& generator)
{
	if(generator.empty()) {
		return "";
	}

	std::vector<std::string> parts = utils::split(generator, ':');

	if(parts.size() > 1 && parts[0] == "contents") {
		if(parts[1] == "generated") {
			return print_table_of_contents();
		} else {
			return print_table_of_contents_for(parts[1]);
		}
	}

	return "";
}

std::string section::print_table_of_contents_for(const std::string& section_id) const
{
	const config& section_cfg = manager_->get_section_config(section_id);
	if(!section_cfg) {
		return "";
	}

	// We use an intermediate structure to allow a conditional sorting
	std::vector<std::pair<std::string, std::string>> topics_links;

	// Find all topics in this section.
	for(const std::string& topic_id : utils::quoted_split(section_cfg["topics"])) {
		if(const config& topic_cfg = manager_->get_topic_config(topic_id)) {
			if(is_visible_id(topic_id)) {
				topics_links.emplace_back(topic_cfg["title"], topic_id);
			}
		}
	}

	if(section_cfg["sort_topics"].to_bool()) {
		std::sort(topics_links.begin(), topics_links.end());
	}

	std::ostringstream res;

	for(const auto& link : topics_links) {
		res << font::unicode_bullet << ' ' << make_link(link.first, link.second) << '\n';
	}

	return res.str();
}

std::string section::print_table_of_contents() const
{
	std::ostringstream res;

	for(const section_ptr& s : sections_) {
		if(is_visible_id(s->id)) {
			res << font::unicode_bullet << ' ' << make_link(s->title, ".." + s->id) << '\n';
		}
	}

	for(const topic& t : topics_) {
		if(is_visible_id(t.id)) {
			res << font::unicode_bullet << ' ' << make_link(t.title, t.id) << '\n';
		}
	}

	return res.str();
}

section* section::add_section(const config& new_section_cfg)
{
	try {
		sections_.emplace_back(std::make_unique<section>(new_section_cfg, *this));
		return sections_.back().get();
	} catch(const parse_error& e) {
		std::cerr << "Error while parsing help text: " << e.message << std::endl;
	} catch(const max_recursion_reached& e) {
		std::cerr << e.message << std::endl;
	}

	return nullptr;
}

const section* section::find_section(const std::string& s_id) const
{
	// First, check this section's sub-sections...
	auto sec_iter = std::find_if(sections_.begin(), sections_.end(),
		[&s_id](const section_ptr& s) { return s && s_id == s->id; });

	if(sec_iter != sections_.end()) {
		return sec_iter->get();
	}

	// ...then recursively check this section's sub-section's sub-sections.
	for(const section_ptr& s : sections_) {
		if(const section* ss = s->find_section(s_id)) {
			return ss;
		}
	}

	return nullptr;
}

const topic* section::find_topic(const std::string& t_id) const
{
	// First, check this section's topics...
	auto topic_iter = std::find_if(topics_.begin(), topics_.end(),
		[&t_id](const topic& t) { return t_id == t.id; });

	if(topic_iter != topics_.end()) {
		return &(*topic_iter);
	}

	// ...then recursively check this section's sub-section's topics.
	for(const section::section_ptr& s : sections_) {
		if(const topic* t = s->find_topic(t_id)) {
			return t;
		}
	}

	return nullptr;
}

} // namespace help
