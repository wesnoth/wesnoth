/*
   Copyright (C) 2009 - 2018 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/


/**
 * Managing the AI configuration
 * @file
 */

#include "ai/configuration.hpp"

#include "filesystem.hpp"
#include "log.hpp"
#include "serialization/parser.hpp"
#include "serialization/preprocessor.hpp"
#include "wml_exception.hpp"

#include <vector>
#include <deque>
#include <set>

namespace ai {

static lg::log_domain log_ai_configuration("ai/config");
#define DBG_AI_CONFIGURATION LOG_STREAM(debug, log_ai_configuration)
#define LOG_AI_CONFIGURATION LOG_STREAM(info, log_ai_configuration)
#define WRN_AI_CONFIGURATION LOG_STREAM(warn, log_ai_configuration)
#define ERR_AI_CONFIGURATION LOG_STREAM(err, log_ai_configuration)

void configuration::init(const config &game_config)
{
	ai_configurations_.clear();
	era_ai_configurations_.clear();
	mod_ai_configurations_.clear();

	const config &ais = game_config.child("ais");
	default_config_ = ais.child("default_config");
	if (!default_config_) {
		ERR_AI_CONFIGURATION << "Missing AI [default_config]. Therefore, default_config_ set to empty." << std::endl;
		default_config_.clear();
	}


	for (const config &ai_configuration : ais.child_range("ai")) {
		const std::string &id = ai_configuration["id"];
		if (id.empty()){

			ERR_AI_CONFIGURATION << "skipped AI config due to missing id" << ". Config contains:"<< std::endl << ai_configuration << std::endl;
			continue;
		}
		if (ai_configurations_.count(id)>0){
			ERR_AI_CONFIGURATION << "skipped AI config due to duplicate id [" << id << "]. Config contains:"<< std::endl << ai_configuration << std::endl;
			continue;
		}

		description desc;
		desc.id=id;
		desc.text = ai_configuration["description"].t_str();
		desc.cfg=ai_configuration;

		ai_configurations_.emplace(id, desc);
		LOG_AI_CONFIGURATION << "loaded AI config: " << ai_configuration["description"] << std::endl;
	}
}

namespace {
void extract_ai_configurations(std::map<std::string, description> &storage, const config &input)
{
	for (const config &ai_configuration : input.child_range("ai")) {
		const std::string &id = ai_configuration["id"];
		if (id.empty()){

			ERR_AI_CONFIGURATION << "skipped AI config due to missing id" << ". Config contains:"<< std::endl << ai_configuration << std::endl;
			continue;
		}
		if (storage.count(id)>0){
			ERR_AI_CONFIGURATION << "skipped AI config due to duplicate id [" << id << "]. Config contains:"<< std::endl << ai_configuration << std::endl;
			continue;
		}

		description desc;
		desc.id=id;
		desc.text = ai_configuration["description"].t_str();
		desc.cfg=ai_configuration;

		storage.emplace(id, desc);
		LOG_AI_CONFIGURATION << "loaded AI config: " << ai_configuration["description"] << std::endl;
	}
}
}

void configuration::add_era_ai_from_config(const config &era)
{
	era_ai_configurations_.clear();
	extract_ai_configurations(era_ai_configurations_, era);
}

void configuration::add_mod_ai_from_config(config::const_child_itors mods)
{
	mod_ai_configurations_.clear();
	for (const config &mod : mods) {
		extract_ai_configurations(mod_ai_configurations_, mod);
	}
}

std::vector<description*> configuration::get_available_ais()
{
	std::vector<description*> ais_list;

	const auto add_if_not_hidden = [&ais_list](description* d) {
		const config& cfg = d->cfg;

		if(!cfg["hidden"].to_bool(false)) {
			ais_list.push_back(d);

			DBG_AI_CONFIGURATION << "has ai with config: " << std::endl << cfg << std::endl;
		}
	};

	for(auto& a_config : ai_configurations_) {
		add_if_not_hidden(&a_config.second);
	}

	for(auto& e_config : era_ai_configurations_) {
		add_if_not_hidden(&e_config.second);
	}

	for(auto& m_config : mod_ai_configurations_) {
		add_if_not_hidden(&m_config.second);
	}

	return ais_list;
}

const config& configuration::get_ai_config_for(const std::string &id)
{
	description_map::iterator cfg_it = ai_configurations_.find(id);
	if (cfg_it==ai_configurations_.end()){
		description_map::iterator era_cfg_it = era_ai_configurations_.find(id);
		if (era_cfg_it==era_ai_configurations_.end()){
			description_map::iterator mod_cfg_it = mod_ai_configurations_.find(id);
			if (mod_cfg_it==mod_ai_configurations_.end()) {
				return default_config_;
			} else {
				return mod_cfg_it->second.cfg;
			}
		} else {
			return era_cfg_it->second.cfg;
		}
	}
	return cfg_it->second.cfg;
}


configuration::description_map configuration::ai_configurations_ = configuration::description_map();
configuration::description_map configuration::era_ai_configurations_ = configuration::description_map();
configuration::description_map configuration::mod_ai_configurations_ = configuration::description_map();
config configuration::default_config_ = config();

bool configuration::get_side_config_from_file(const std::string& file, config& cfg ){
	try {
		filesystem::scoped_istream stream = preprocess_file(filesystem::get_wml_location(file));
		read(cfg, *stream);
		LOG_AI_CONFIGURATION << "Reading AI configuration from file '" << file  << "'" << std::endl;
	} catch(config::error &) {
		ERR_AI_CONFIGURATION << "Error while reading AI configuration from file '" << file  << "'" << std::endl;
		return false;
	}
	LOG_AI_CONFIGURATION << "Successfully read AI configuration from file '" << file  << "'" << std::endl;
	return true;
}

const config& configuration::get_default_ai_parameters()
{
	return default_config_;
}


bool configuration::parse_side_config(side_number side, const config& original_cfg, config &cfg )
{
	LOG_AI_CONFIGURATION << "side "<< side <<": parsing AI configuration from config" << std::endl;

	//leave only the [ai] children
	cfg.clear();
	for (const config &aiparam : original_cfg.child_range("ai")) {
		cfg.add_child("ai",aiparam);
	}

	//backward-compatibility hack: put ai_algorithm if it is present
	if (const config::attribute_value *v = original_cfg.get("ai_algorithm")) {
		config ai_a;
		ai_a["ai_algorithm"] = *v;
		cfg.add_child("ai",ai_a);
	}
	DBG_AI_CONFIGURATION << "side " << side << ": config contains:"<< std::endl << cfg << std::endl;

	//insert default config at the beginning
	if (default_config_) {
		DBG_AI_CONFIGURATION << "side "<< side <<": applying default configuration" << std::endl;
		cfg.add_child_at("ai",default_config_,0);
	} else {
		ERR_AI_CONFIGURATION << "side "<< side <<": default configuration is not available, not applying it" << std::endl;
	}

	LOG_AI_CONFIGURATION << "side "<< side << ": expanding simplified aspects into full facets"<< std::endl;
	expand_simplified_aspects(side, cfg);

	//construct new-style integrated config
	LOG_AI_CONFIGURATION << "side "<< side << ": doing final operations on AI config"<< std::endl;
	config parsed_cfg = config();

	LOG_AI_CONFIGURATION << "side "<< side <<": merging AI configurations"<< std::endl;
	for (const config &aiparam : cfg.child_range("ai")) {
		parsed_cfg.append(aiparam);
	}


	LOG_AI_CONFIGURATION << "side "<< side <<": merging AI aspect with the same id"<< std::endl;
	parsed_cfg.merge_children_by_attribute("aspect","id");

	LOG_AI_CONFIGURATION << "side "<< side <<": removing duplicate [default] tags from aspects"<< std::endl;
	for (config &aspect_cfg : parsed_cfg.child_range("aspect")) {
		if (aspect_cfg["name"] != "composite_aspect") {
			// No point in warning about Lua or standard aspects lacking [default]
			continue;
		}
		if (!aspect_cfg.child("default")) {
			WRN_AI_CONFIGURATION << "side "<< side <<": aspect with id=["<<aspect_cfg["id"]<<"] lacks default config facet!" <<std::endl;
			continue;
		}
		aspect_cfg.merge_children("default");
		config& dflt = aspect_cfg.child("default");
		if (dflt.has_child("value")) {
			while (dflt.child_count("value") > 1) {
				dflt.remove_child("value", 0);
			}
		}
	}

	DBG_AI_CONFIGURATION << "side "<< side <<": done parsing side config, it contains:"<< std::endl << parsed_cfg << std::endl;
	LOG_AI_CONFIGURATION << "side "<< side <<": done parsing side config"<< std::endl;

	cfg = parsed_cfg;
	return true;

}

static const std::set<std::string> non_aspect_attributes {"turns", "time_of_day", "engine", "ai_algorithm", "id", "description"};
static const std::set<std::string> just_copy_tags {"engine", "stage", "aspect", "goal", "modify_ai"};
static const std::set<std::string> old_goal_tags {"target", "target_location", "protect_unit", "protect_location"};

void configuration::expand_simplified_aspects(side_number side, config &cfg) {
	std::string algorithm;
	config base_config, parsed_config;
	for (const config &aiparam : cfg.child_range("ai")) {
		std::string turns, time_of_day, engine = "cpp";
		if (aiparam.has_attribute("turns")) {
			turns = aiparam["turns"].str();
		}
		if (aiparam.has_attribute("time_of_day")) {
			time_of_day = aiparam["time_of_day"].str();
		}
		if (aiparam.has_attribute("engine")) {
			engine = aiparam["engine"].str();
		}
		if (aiparam.has_attribute("ai_algorithm")) {
			if (algorithm.empty()) {
				algorithm = aiparam["ai_algorithm"].str();
				base_config = get_ai_config_for(algorithm);
			} else if(algorithm != aiparam["ai_algorithm"]) {
				lg::wml_error() << "side " << side << " has two [ai] tags with contradictory ai_algorithm - the first one will take precedence.\n";
			}
		}
		std::deque<std::pair<std::string, config>> facet_configs;
		for (const config::attribute &attr : aiparam.attribute_range()) {
			if (non_aspect_attributes.count(attr.first)) {
				continue;
			}
			config facet_config;
			facet_config["engine"] = engine;
			facet_config["name"] = "standard_aspect";
			facet_config["turns"] = turns;
			facet_config["time_of_day"] = time_of_day;
			facet_config["value"] = attr.second;
			facet_configs.emplace_back(attr.first, facet_config);
		}
		for (const config::any_child &child : aiparam.all_children_range()) {
			if (just_copy_tags.count(child.key)) {
				// These aren't simplified, so just copy over unchanged.
				parsed_config.add_child(child.key, child.cfg);
				continue;
			} else if(old_goal_tags.count(child.key)) {
				// A simplified goal, mainly kept around just for backwards compatibility.
				config goal_config, criteria_config = child.cfg;
				goal_config["name"] = child.key;
				goal_config["turns"] = turns;
				goal_config["time_of_day"] = time_of_day;
				if(child.key.substr(0,7) == "protect" && criteria_config.has_attribute("protect_radius")) {
					goal_config["protect_radius"] = criteria_config["protect_radius"];
					criteria_config.remove_attribute("protect_radius");
				}
				if(criteria_config.has_attribute("value")) {
					goal_config["value"] = criteria_config["value"];
					criteria_config.remove_attribute("value");
				}
				parsed_config.add_child("goal", std::move(goal_config));
				continue;
			}
			// Now there's two possibilities. If the tag is [attacks] or contains either value= or [value],
			// then it can be copied verbatim as a [facet] tag.
			// Otherwise, it needs to be placed as a [value] within a [facet] tag.
			if (child.key == "attacks" || child.cfg.has_attribute("value") || child.cfg.has_child("value")) {
				facet_configs.emplace_back(child.key, child.cfg);
			} else {
				config facet_config;
				facet_config["engine"] = engine;
				facet_config["name"] = "standard_aspect";
				facet_config["turns"] = turns;
				facet_config["time_of_day"] = time_of_day;
				facet_config.add_child("value", child.cfg);
				if (child.key == "leader_goal" && !child.cfg["id"].empty()) {
					// Use id= attribute (if present) as the facet ID
					const std::string& id = child.cfg["id"];
					if(id != "*" && id.find_first_not_of("0123456789") != std::string::npos) {
						facet_config["id"] = child.cfg["id"];
					}
				}
				facet_configs.emplace_back(child.key, facet_config);
			}
		}
		std::map<std::string, config> aspect_configs;
		while (!facet_configs.empty()) {
			const std::string &aspect = facet_configs.front().first;
			const config &facet_config = facet_configs.front().second;
			aspect_configs[aspect]["id"] = aspect; // Will sometimes be redundant assignment
			aspect_configs[aspect]["name"] = "composite_aspect";
			aspect_configs[aspect].add_child("facet", facet_config);
			facet_configs.pop_front();
		}
		typedef std::map<std::string, config>::value_type aspect_pair;
		for (const aspect_pair& p : aspect_configs) {
			parsed_config.add_child("aspect", p.second);
		}
	}
	// Support old recruitment aspect syntax
	for(auto& child : parsed_config.child_range("aspect")) {
		if(child["id"] == "recruitment") {
			child["id"] = "recruitment_instructions";
		}
	}
	if (algorithm.empty() && !parsed_config.has_child("stage")) {
		base_config = get_ai_config_for("ai_default_rca");
	}
	for (const config::any_child &child : parsed_config.all_children_range()) {
		base_config.add_child(child.key, child.cfg);
	}
	cfg.clear_children("ai");
	cfg.add_child("ai", std::move(base_config));
}

} //end of namespace ai
