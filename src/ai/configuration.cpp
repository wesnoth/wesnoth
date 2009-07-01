/* $Id$ */
/*
   Copyright (C) 2009 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/


/**
 * Managing the AI configuration
 * @file ai/configuration.cpp
 */

#include "configuration.hpp"

#include "../filesystem.hpp"
#include "../foreach.hpp"
#include "../log.hpp"
#include "../serialization/parser.hpp"
#include "../serialization/preprocessor.hpp"
#include "../team.hpp"

namespace ai {

static lg::log_domain log_ai_configuration("ai/config");
#define DBG_AI_CONFIGURATION LOG_STREAM(debug, log_ai_configuration)
#define LOG_AI_CONFIGURATION LOG_STREAM(info, log_ai_configuration)
#define WRN_AI_CONFIGURATION LOG_STREAM(warn, log_ai_configuration)
#define ERR_AI_CONFIGURATION LOG_STREAM(err, log_ai_configuration)

void configuration::init(const config &game_config)
{
	ai_configurations_.clear();

	const config &ais = game_config.child("ais");
	foreach (const config &ai_configuration, ais.child_range("ai")) {
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
		desc.text=ai_configuration["description"];
		desc.cfg=ai_configuration;

		ai_configurations_.insert(std::make_pair(id,desc));
		LOG_AI_CONFIGURATION << "loaded AI config: " << ai_configuration["description"] << std::endl;
	}
}

std::vector<description*> configuration::get_available_ais(){
	std::vector<description*> ais_list;
	for(description_map::iterator desc = ai_configurations_.begin(); desc!=ai_configurations_.end(); ++desc) {
		ais_list.push_back(&desc->second);	
		DBG_AI_CONFIGURATION << "has ai with config: "<< std::endl << desc->second.cfg<< std::endl;
	}
	return ais_list;
}

const config& configuration::get_ai_config_for(const std::string &id)
{
	description_map::iterator cfg_it = ai_configurations_.find(id);
	if (cfg_it==ai_configurations_.end()){
		return default_config_;
	}
	return cfg_it->second.cfg;
}


configuration::description_map configuration::ai_configurations_ = configuration::description_map();
config configuration::default_config_ = config();


static std::string bind_config_parameter( const std::string& value_from_config,
		const std::string& value_from_global_config,
		const std::string& default_value )
{
	if (value_from_config!=""){
		return value_from_config;
	} else if (value_from_global_config!=""){
		return value_from_global_config;
	}
	return default_value;
}

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


bool configuration::parse_side_config(const config& cfg,
		std::string& ai_algorithm_type,
		config& global_ai_parameters,
		std::vector<config>& ai_parameters,
		const config& default_ai_parameters,
		config& ai_memory,
		config& effective_ai_parameters )
{
	LOG_AI_CONFIGURATION << "Parsing [side] configuration from config" << std::endl;
	DBG_AI_CONFIGURATION << "Config contains:"<< std::endl << cfg << std::endl;

	bool new_syntax=false;//@todo 1.8 remove old way when necessary
	foreach (const config &aiparam, cfg.child_range("ai")) {
		if (aiparam.child("stage")){
			new_syntax=true;
			break;
		}
	}

	if (new_syntax) {
		DBG_AI_CONFIGURATION << "new syntax parsing rules"<< std::endl;
		return parse_side_config_new(cfg,ai_algorithm_type,global_ai_parameters,ai_parameters,default_ai_parameters,ai_memory,effective_ai_parameters);
	} else {
		DBG_AI_CONFIGURATION << "old syntax parsing rules"<< std::endl;
		return parse_side_config_old(cfg,ai_algorithm_type,global_ai_parameters,ai_parameters,default_ai_parameters,ai_memory,effective_ai_parameters);
	}
}

bool configuration::parse_side_config_new(const config &cfg,
		std::string &ai_algorithm_type,
		config &global_ai_parameters,
		std::vector<config> &ai_parameters,
		const config &default_ai_parameters,
		config &ai_memory,
		config &effective_ai_parameters )
{
	//construct new-style integrated config
	ai_parameters.push_back(default_ai_parameters);
	global_ai_parameters.append(default_ai_parameters);
	global_ai_parameters["ai_algorithm"] = "composite_ai";
	DBG_AI_CONFIGURATION << "after taking default values into account, new-style config contains:"<< std::endl << global_ai_parameters << std::endl;

	foreach (const config &aiparam, cfg.child_range("ai"))
	{
		ai_parameters.push_back(aiparam);
		global_ai_parameters.append(aiparam);
	}

	DBG_AI_CONFIGURATION << "finally, new-style config contains:"<< std::endl << global_ai_parameters << std::endl;

	ai_algorithm_type = global_ai_parameters["ai_algorithm"];

	//AI memory
	foreach (const config &aimem, global_ai_parameters.child_range("ai_memory")) {
		ai_memory.append(aimem);
	}

	effective_ai_parameters["number_of_possible_recruits_to_force_recruit"]= global_ai_parameters["number_of_possible_recruits_to_force_recruit"];
	effective_ai_parameters["villages_per_scout"] = global_ai_parameters["villages_per_scout"];
	effective_ai_parameters["leader_value"] = global_ai_parameters["leader_value"];
	effective_ai_parameters["village_value"] = global_ai_parameters["village_value"];
	effective_ai_parameters["aggression"] = global_ai_parameters["aggression"];
	effective_ai_parameters["caution"] = global_ai_parameters["caution"];

	return true;
}

bool configuration::parse_side_config_old(const config& cfg,
		std::string& ai_algorithm_type,
		config& global_ai_parameters,
		std::vector<config>& ai_parameters,
		const config& default_ai_parameters,
		config& ai_memory,
		config& effective_ai_parameters )
{

	foreach (const config &aiparam, cfg.child_range("ai"))
	{
		ai_parameters.push_back(aiparam);
		//those AI parameters, which are always active, are also added to global ai parameters
		if (aiparam["turns"].empty() && aiparam["time_of_day"].empty()) {
			global_ai_parameters.append(aiparam);
		}
	}

	//AI memory
	foreach (const config &aimem, cfg.child_range("ai_memory")) {
		ai_memory.append(aimem);
	}

	//set some default config values.
	//@todo 1.7 later, the entire 'ai parameter/ai memory/ai effective parameter' system should be refactored.
	//@todo 1.7 the following can also be rewritten to use a loop and a better version of bind_config_parameter [for each in default parameters T do bind_config_parameter(T,cfg,global_ai_params,defaults) ]
	ai_algorithm_type = bind_config_parameter(
				cfg["ai_algorithm"],
				global_ai_parameters["ai_algorithm"],
				default_ai_parameters["ai_algorithm"]);

	effective_ai_parameters["number_of_possible_recruits_to_force_recruit"] = bind_config_parameter(
			cfg["number_of_possible_recruits_to_force_recruit"],
			global_ai_parameters["number_of_possible_recruits_to_force_recruit"],
			default_ai_parameters["number_of_possible_recruits_to_force_recruit"]);

	effective_ai_parameters["villages_per_scout"] = bind_config_parameter(
			cfg["villages_per_scout"],
			global_ai_parameters["villages_per_scout"],
			default_ai_parameters["villages_per_scout"]);

	effective_ai_parameters["leader_value"] = bind_config_parameter(
			cfg["leader_value"],
			global_ai_parameters["leader_value"],
			default_ai_parameters["leader_value"]);

	effective_ai_parameters["village_value"] = bind_config_parameter(
			cfg["village_value"],
			global_ai_parameters["village_value"],
			default_ai_parameters["village_value"]);

	effective_ai_parameters["aggression"] = bind_config_parameter(
			cfg["aggression"],
			global_ai_parameters["aggression"],
			default_ai_parameters["aggression"]);

	effective_ai_parameters["caution"] = bind_config_parameter(
			cfg["caution"],
			global_ai_parameters["caution"],
			default_ai_parameters["caution"]);

	return true;
}

//some default values for the AI parameters following the default values listed
//in the wiki at http://www.wesnoth.org/wiki/AiWML
//@todo 1.7 think about reading this from config
const config& configuration::get_default_ai_parameters(){
	static config default_cfg;
	if (!default_cfg["init"].empty()) {
		return default_cfg;
	}

	default_cfg["init"] = "true";

	default_cfg["ai_algorithm"] = "default_ai";

	//A number 0 or higher which determines how many scouts the AI recruits. If 0,
	//the AI doesn't recruit scouts to capture villages.
	default_cfg["villages_per_scout"] = "4";

	//A number 0 or higher which determines how much the AI targets enemy leaders.
	default_cfg["leader_value"] = "3.0";

	//A number 0 or higher which determines how much the AI tries to capture villages.
	default_cfg["village_value"] = "1.0";

	//details see in the [AI] tag explaination in the wiki:
	//http://www.wesnoth.org/wiki/AiWML#the_.5Bai.5D_tag
	default_cfg["aggression"] = "0.5";

	//details see in the [AI] tag explaination in the wiki:
	//http://www.wesnoth.org/wiki/AiWML#the_.5Bai.5D_tag
	default_cfg["caution"] = "0.25";

	//number_of_possible_recruits_to_force_recruit: a number higher than 0.0.
	//Tells AI to force the leader to move to a keep if it has enough gold to
	//recruit the given number of units. If set to 0.0, moving to recruit is disabled.
	default_cfg["number_of_possible_recruits_to_force_recruit"] = "3.1";
	LOG_AI_CONFIGURATION << "AI default configuration is created" << std::endl;
	return default_cfg;
}

} //end of namespace ai
