/*
   Copyright (C) 2009 - 2014 by Yurii Chernyi <terraninfo@terraninfo.net>
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

#include "configuration.hpp"

#include "../filesystem.hpp"
#include "../log.hpp"
#include "../serialization/parser.hpp"
#include "../serialization/preprocessor.hpp"
#include "../team.hpp"
#include "wml_exception.hpp"

#include <boost/foreach.hpp>

#include <vector>

namespace ai {

static lg::log_domain log_ai_configuration("ai/config");
#define DBG_AI_CONFIGURATION LOG_STREAM(debug, log_ai_configuration)
#define LOG_AI_CONFIGURATION LOG_STREAM(info, log_ai_configuration)
#define WRN_AI_CONFIGURATION LOG_STREAM(warn, log_ai_configuration)
#define ERR_AI_CONFIGURATION LOG_STREAM(err, log_ai_configuration)


class well_known_aspect {
public:
	well_known_aspect(const std::string &name, bool attr = true)
		: name_(name),was_an_attribute_(attr)
	{
	}

	virtual ~well_known_aspect() {}

	std::string name_;
	bool was_an_attribute_;
};

std::vector<well_known_aspect> well_known_aspects;

void configuration::init(const config &game_config)
{
	ai_configurations_.clear();
	era_ai_configurations_.clear();
	mod_ai_configurations_.clear();
	well_known_aspects.clear();
	well_known_aspects.push_back(well_known_aspect("advancements"));
	well_known_aspects.push_back(well_known_aspect("aggression"));
	well_known_aspects.push_back(well_known_aspect("attack_depth"));
	well_known_aspects.push_back(well_known_aspect("attacks"));
	well_known_aspects.push_back(well_known_aspect("avoid",false));
	well_known_aspects.push_back(well_known_aspect("caution"));
	well_known_aspects.push_back(well_known_aspect("grouping"));
	well_known_aspects.push_back(well_known_aspect("leader_aggression"));
	well_known_aspects.push_back(well_known_aspect("leader_goal",false));
	well_known_aspects.push_back(well_known_aspect("leader_ignores_keep"));
	well_known_aspects.push_back(well_known_aspect("leader_value"));
	well_known_aspects.push_back(well_known_aspect("number_of_possible_recruits_to_force_recruit"));
	well_known_aspects.push_back(well_known_aspect("passive_leader"));
	well_known_aspects.push_back(well_known_aspect("passive_leader_shares_keep"));
	well_known_aspects.push_back(well_known_aspect("recruitment"));
	well_known_aspects.push_back(well_known_aspect("recruitment_diversity"));
	well_known_aspects.push_back(well_known_aspect("recruitment_ignore_bad_combat"));
	well_known_aspects.push_back(well_known_aspect("recruitment_ignore_bad_movement"));
	well_known_aspects.push_back(well_known_aspect("recruitment_instructions"));
	well_known_aspects.push_back(well_known_aspect("recruitment_more"));
	well_known_aspects.push_back(well_known_aspect("recruitment_pattern"));
	well_known_aspects.push_back(well_known_aspect("recruitment_randomness"));
	well_known_aspects.push_back(well_known_aspect("recruitment_save_gold"));
	well_known_aspects.push_back(well_known_aspect("scout_village_targeting"));
	well_known_aspects.push_back(well_known_aspect("simple_targeting"));
	well_known_aspects.push_back(well_known_aspect("support_villages"));
	well_known_aspects.push_back(well_known_aspect("village_value"));
	well_known_aspects.push_back(well_known_aspect("villages_per_scout"));

	const config &ais = game_config.child("ais");
	default_config_ = ais.child("default_config");
	if (!default_config_) {
		ERR_AI_CONFIGURATION << "Missing AI [default_config]. Therefore, default_config_ set to empty." << std::endl;
		default_config_ = config();
	}


	BOOST_FOREACH(const config &ai_configuration, ais.child_range("ai")) {
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

		ai_configurations_.insert(std::make_pair(id,desc));
		LOG_AI_CONFIGURATION << "loaded AI config: " << ai_configuration["description"] << std::endl;
	}
}

namespace {
void extract_ai_configurations(std::map<std::string, description> &storage, const config &input)
{
	BOOST_FOREACH(const config &ai_configuration, input.child_range("ai")) {
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

		storage.insert(std::make_pair(id,desc));
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
	BOOST_FOREACH(const config &mod, mods) {
		extract_ai_configurations(mod_ai_configurations_, mod);
	}
}

std::vector<description*> configuration::get_available_ais(){
	std::vector<description*> ais_list;
	for(description_map::iterator desc = ai_configurations_.begin(); desc!=ai_configurations_.end(); ++desc) {
		ais_list.push_back(&desc->second);
		DBG_AI_CONFIGURATION << "has ai with config: "<< std::endl << desc->second.cfg<< std::endl;
	}
	for(description_map::iterator desc = era_ai_configurations_.begin(); desc!=era_ai_configurations_.end(); ++desc) {
		ais_list.push_back(&desc->second);
		DBG_AI_CONFIGURATION << "has ai with config: "<< std::endl << desc->second.cfg<< std::endl;
	}
	for(description_map::iterator desc = mod_ai_configurations_.begin(); desc!=mod_ai_configurations_.end(); ++desc) {
		ais_list.push_back(&desc->second);
		DBG_AI_CONFIGURATION << "has ai with config: "<< std::endl << desc->second.cfg<< std::endl;
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
		scoped_istream stream = preprocess_file(get_wml_location(file));
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


bool configuration::upgrade_aspect_config_from_1_07_02_to_1_07_03(side_number side, const config& cfg, config& parsed_cfg, const std::string &id, bool aspect_was_attribute)
{
	config aspect_config;
	aspect_config["id"] = id;

	BOOST_FOREACH(const config &aiparam, cfg.child_range("ai")) {
		const config &_aspect = aiparam.find_child("aspect","id",id);
		if (_aspect) {
			aspect_config.append(_aspect);
		}

		if (aspect_was_attribute && !aiparam.has_attribute(id)) {
			continue;//we are looking for an attribute but it isn't present
		}

		if (!aspect_was_attribute && !aiparam.child(id)) {
			continue;//we are looking for a child but it isn't present
		}

		config facet_config;
		facet_config["engine"] = "cpp";
		facet_config["name"] = "standard_aspect";
		if (aspect_was_attribute) {
			facet_config["value"] = aiparam[id];
		} else {
			BOOST_FOREACH(const config &value, aiparam.child_range(id)) {
				facet_config.add_child("value",value);
			}
		}

		if (const config::attribute_value *v = aiparam.get("turns")) {
			facet_config["turns"] = *v;
		}
		if (const config::attribute_value *v = aiparam.get("time_of_day")) {
			facet_config["time_of_day"] = *v;
		}

		aspect_config.add_child("facet",facet_config);
	}
	DBG_AI_CONFIGURATION << "side "<< side << " aspect[" << id << "] config :\n"<< cfg << "\n";

	parsed_cfg.add_child("aspect",aspect_config);
	return true;
}


bool configuration::parse_side_config(side_number side, const config& original_cfg, config &cfg )
{
	LOG_AI_CONFIGURATION << "side "<< side <<": parsing AI configuration from config" << std::endl;

	//leave only the [ai] children
	cfg = config();
	BOOST_FOREACH(const config &aiparam, original_cfg.child_range("ai")) {
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
		ERR_AI_CONFIGURATION << "side "<< side <<": default configuration is not available, do not applying it" << std::endl;
	}

	//find version
	int version = 10600;
	BOOST_FOREACH(const config &aiparam, cfg.child_range("ai")) {
		if (const config::attribute_value *a = aiparam.get("version")){
			int v = a->to_int(version);
			if (version<v) {
				version = v;
			}
		}
	}

	if (version<10703) {
		if (!upgrade_side_config_from_1_07_02_to_1_07_03(side, cfg)) {
			return false;
		}
		version = 10703;
	}


	if (version<10710) {
		version = 10710;
	}

	//construct new-style integrated config
	LOG_AI_CONFIGURATION << "side "<< side << ": doing final operations on AI config"<< std::endl;
	config parsed_cfg = config();

	LOG_AI_CONFIGURATION << "side "<< side <<": merging AI configurations"<< std::endl;
	BOOST_FOREACH(const config &aiparam, cfg.child_range("ai")) {
		parsed_cfg.append(aiparam);
	}

	LOG_AI_CONFIGURATION << "side "<< side <<": setting config version to "<< version << std::endl;
	parsed_cfg["version"] = version;


	LOG_AI_CONFIGURATION << "side "<< side <<": merging AI aspect with the same id"<< std::endl;
	parsed_cfg.merge_children_by_attribute("aspect","id");

	LOG_AI_CONFIGURATION << "side "<< side <<": removing duplicate [default] tags from aspects"<< std::endl;
	BOOST_FOREACH(config &aspect_cfg, parsed_cfg.child_range("aspect")) {
		if (!aspect_cfg.child("default")) {
			WRN_AI_CONFIGURATION << "side "<< side <<": aspect with id=["<<aspect_cfg["id"]<<"] lacks default config facet!" <<std::endl;
			continue;
		}
		config c = aspect_cfg.child("default");
		aspect_cfg.clear_children("default");
		aspect_cfg.add_child("default",c);
	}

	DBG_AI_CONFIGURATION << "side "<< side <<": done parsing side config, it contains:"<< std::endl << parsed_cfg << std::endl;
	LOG_AI_CONFIGURATION << "side "<< side <<": done parsing side config"<< std::endl;

	cfg = parsed_cfg;
	return true;

}


static void transfer_turns_and_time_of_day_data(const config &src, config &dst)
{
	if (const config::attribute_value *turns = src.get("turns")) {
		dst["turns"] = *turns;
	}
	if (const config::attribute_value *time_of_day = src.get("time_of_day")) {
		dst["time_of_day"] = *time_of_day;
	}
}


bool configuration::upgrade_side_config_from_1_07_02_to_1_07_03(side_number side, config &cfg)
{
	LOG_AI_CONFIGURATION << "side "<< side <<": upgrading ai config version from version 1.7.2 to 1.7.3"<< std::endl;
	config parsed_cfg;

	bool is_idle_ai = false;
	if (cfg["ai_algorithm"]=="idle_ai") {
		is_idle_ai = true;
	} else {
		BOOST_FOREACH(config &aiparam, cfg.child_range("ai")) {
			if (aiparam["ai_algorithm"]=="idle_ai") {
				is_idle_ai = true;
				break;
			}
		}
	}

	if (!is_idle_ai) {
		parsed_cfg = get_ai_config_for("ai_default_rca");
	}

	//get values of all aspects
	upgrade_aspect_configs_from_1_07_02_to_1_07_03(side, cfg.child_range("ai"), parsed_cfg);

	//dump the rest of the config into fallback stage

	config fallback_stage_cfg_ai;

	BOOST_FOREACH(config &aiparam, cfg.child_range("ai")) {
		BOOST_FOREACH(const well_known_aspect &wka, well_known_aspects) {
			if (wka.was_an_attribute_) {
				aiparam.remove_attribute(wka.name_);
			} else {
				aiparam.clear_children(wka.name_);
			}
		}


		BOOST_FOREACH(const config &aitarget, aiparam.child_range("target")) {
			lg::wml_error << deprecate_wml_key_warning("target", "1.13.0") << "\n";
			config aigoal;
			transfer_turns_and_time_of_day_data(aiparam,aigoal);

			if (const config::attribute_value *v = aitarget.get("value")) {
				aigoal["value"] = *v;
			} else {
				aigoal["value"] = 0;
			}

			config &aigoalcriteria = aigoal.add_child("criteria",aitarget);
			aigoalcriteria.remove_attribute("value");

			parsed_cfg.add_child("goal",aigoal);
		}
		aiparam.clear_children("target");


		BOOST_FOREACH(config &ai_protect_unit, aiparam.child_range("protect_unit")) {
			lg::wml_error << deprecate_wml_key_warning("protect_unit", "1.13.0") << "\n";
			transfer_turns_and_time_of_day_data(aiparam,ai_protect_unit);
			upgrade_protect_goal_config_from_1_07_02_to_1_07_03(side,ai_protect_unit,parsed_cfg,true);
		}
		aiparam.clear_children("protect_unit");


		BOOST_FOREACH(config &ai_protect_location, aiparam.child_range("protect_location")) {
			lg::wml_error << deprecate_wml_key_warning("protect_location", "1.13.0") << "\n";
			transfer_turns_and_time_of_day_data(aiparam,ai_protect_location);
			upgrade_protect_goal_config_from_1_07_02_to_1_07_03(side,ai_protect_location,parsed_cfg,false);
		}
		aiparam.clear_children("protect_location");


		if (const config::attribute_value *v = aiparam.get("protect_leader"))
		{
			lg::wml_error << deprecate_wml_key_warning("protect_leader", "1.13.0") << "\n";
			config c;
			c["value"] = *v;
			c["canrecruit"] = true;
			c["side_number"] = side;
			transfer_turns_and_time_of_day_data(aiparam,c);
			if (const config::attribute_value *v = aiparam.get("protect_leader_radius")) {
				lg::wml_error << deprecate_wml_key_warning("protect_leader_radius", "1.13.0") << "\n";
				c["radius"] = *v;
			}

			upgrade_protect_goal_config_from_1_07_02_to_1_07_03(side,c,parsed_cfg,true);
		}

		if (!aiparam.has_attribute("turns") && !aiparam.has_attribute("time_of_day")) {
			fallback_stage_cfg_ai.append(aiparam);
		} else {
			// Move [goal]s to root of the config, adding turns and time_of_day.
			BOOST_FOREACH(const config &aigoal, aiparam.child_range("goal")) {
				config updated_goal_config = aigoal;
				transfer_turns_and_time_of_day_data(aiparam, updated_goal_config);
				parsed_cfg.add_child("goal", updated_goal_config);
			}
                }
	}
	fallback_stage_cfg_ai.clear_children("aspect");

	//move [stage]s to root of the config
	BOOST_FOREACH(const config &aistage, fallback_stage_cfg_ai.child_range("stage")) {
		parsed_cfg.add_child("stage",aistage);
	}
	fallback_stage_cfg_ai.clear_children("stage");

	//move [goal]s to root of the config
	BOOST_FOREACH(const config &aigoal, fallback_stage_cfg_ai.child_range("goal")) {
		parsed_cfg.add_child("goal",aigoal);
	}
	fallback_stage_cfg_ai.clear_children("goal");

	//move [modify_ai]'s to root of the config
	BOOST_FOREACH(const config &aimodifyai, fallback_stage_cfg_ai.child_range("modify_ai")) {
		parsed_cfg.add_child("modify_ai",aimodifyai);
	}
	fallback_stage_cfg_ai.clear_children("modify_ai");
	//nothing useful should be at fallback_stage_cfg_ai at this point

	cfg = config();
	cfg.add_child("ai",parsed_cfg);
	DBG_AI_CONFIGURATION << "side "<< side <<": after upgrade to 1.7.3 syntax, config contains:"<< std::endl << cfg << std::endl;
	return true;
}


void configuration::upgrade_aspect_configs_from_1_07_02_to_1_07_03(side_number side, const config::const_child_itors &ai_parameters, config &parsed_cfg)
{
	config cfg;

	BOOST_FOREACH(const config &aiparam, ai_parameters) {
		cfg.add_child("ai",aiparam);
	}

	DBG_AI_CONFIGURATION << "side "<< side <<": upgrading aspects from syntax of 1.7.2 to 1.7.3, old-style config is:" << std::endl << cfg << std::endl;
	BOOST_FOREACH(const well_known_aspect &wka, well_known_aspects) {
		upgrade_aspect_config_from_1_07_02_to_1_07_03(side, cfg,parsed_cfg,wka.name_,wka.was_an_attribute_);
	}
}


void configuration::upgrade_protect_goal_config_from_1_07_02_to_1_07_03(side_number side, const config &protect_cfg, config &parsed_cfg, bool add_filter)
{
	config aigoal;
	aigoal["name"] = "protect";
	transfer_turns_and_time_of_day_data(protect_cfg,aigoal);

	if (const config::attribute_value *v = protect_cfg.get("value")) {
		aigoal["value"] = *v;
	} else {
		aigoal["value"] = 1; //old default value
	}

	//note: 'radius' attribute is renamed to avoid confusion with SLF's radius
	if (const config::attribute_value *v = protect_cfg.get("radius")) {
		aigoal["protect_radius"] = *v;
	} else {
		aigoal["protect_radius"] = 20; //old default value
	}
	DBG_AI_CONFIGURATION << "side "<< side <<": upgrading protect goal from syntax of 1.7.2 to 1.7.3, old-style config is:" << std::endl << protect_cfg << std::endl;


	if (add_filter) {
		config &aigoal_criteria = aigoal.add_child("criteria",config());
		config &aigoal_criteria_filter = aigoal_criteria.add_child("filter",protect_cfg);
		aigoal_criteria_filter.remove_attribute("value");
		aigoal_criteria_filter.remove_attribute("radius");
	} else {
		config &aigoal_criteria = aigoal.add_child("criteria",protect_cfg);
		aigoal_criteria.remove_attribute("value");
		aigoal_criteria.remove_attribute("radius");
	}


	parsed_cfg.add_child("goal",aigoal);
	DBG_AI_CONFIGURATION << "side "<< side <<": after upgrade of protect goal from syntax of 1.7.2 to 1.7.3, new-style config is:" << std::endl << aigoal << std::endl;
}

} //end of namespace ai
