/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include "filesystem.hpp"
#include "game_config.hpp"
#include "gamestatus.hpp"
#include "language.hpp"
#include "log.hpp"

#include <algorithm>
#include <cstdio>
#include <sstream>

time_of_day::time_of_day(config& cfg)
                 : lawful_bonus(atoi(cfg.values["lawful_bonus"].c_str())),
                   image(cfg.values["image"]), name(cfg.values["name"]),
                   red(atoi(cfg.values["red"].c_str())),
                   green(atoi(cfg.values["green"].c_str())),
                   blue(atoi(cfg.values["blue"].c_str()))
{
	const std::string& lang_name = string_table[cfg.values["id"]];
	if(lang_name.empty() == false)
		name = lang_name;
}

gamestatus::gamestatus(config& time_cfg, int num_turns) :
                 turn_(1),numTurns_(num_turns)
{
	std::vector<config*>& times = time_cfg.children["time"];
	std::vector<config*>::iterator t;
	for(t = times.begin(); t != times.end(); ++t) {
		times_.push_back(time_of_day(**t));
	}

	std::vector<config*>& times_illum = time_cfg.children["illuminated_time"];
	std::vector<config*>& illum = times_illum.empty() ? times : times_illum;

	for(t = illum.begin(); t != illum.end(); ++t) {
		illuminatedTimes_.push_back(time_of_day(**t));
	}
}

const time_of_day& gamestatus::get_time_of_day(bool illuminated) const
{
	if(illuminated && illuminatedTimes_.empty() == false) {
		return illuminatedTimes_[(turn()-1)%illuminatedTimes_.size()];
	} else if(times_.empty() == false) {
		return times_[(turn()-1)%times_.size()];
	}

	config dummy_cfg;
	const static time_of_day default_time(dummy_cfg);
	return default_time;
}

size_t gamestatus::turn() const
{
	return turn_;
}

size_t gamestatus::number_of_turns() const
{
	return numTurns_;
}

bool gamestatus::next_turn()
{
	++turn_;
	return turn_ <= numTurns_;
}

game_state read_game(game_data& data, config* cfg)
{
	log_scope("read_game");
	game_state res;
	res.label = (*cfg)["label"];
	res.version = (*cfg)["version"];
	res.gold = atoi((*cfg)["gold"].c_str());
	res.scenario = (*cfg)["scenario"];

	res.difficulty = (*cfg)["difficulty"];
	if(res.difficulty.empty())
		res.difficulty = "NORMAL";

	res.campaign_type = (*cfg)["campaign_type"];
	if(res.campaign_type.empty())
		res.campaign_type = "scenario";

	std::vector<config*>& units = cfg->children["unit"];
	for(std::vector<config*>::iterator i = units.begin();
	    i != units.end(); ++i) {
		log_scope("loading unit");
		res.available_units.push_back(unit(data,**i));
	}

	std::vector<config*>& vars = cfg->children["variables"];
	if(vars.empty() == false) {
		res.variables = vars[0]->values;
	}

	std::vector<config*>& replays = cfg->children["replay"];
	if(replays.empty() == false) {
		res.replay_data = *replays[0];
	}

	std::vector<config*>& starts = cfg->children["start"];
	if(starts.empty() == false) {
		res.starting_pos = *starts[0];
	}

	return res;
}

void write_game(const game_state& game, config& cfg)
{
	cfg["label"] = game.label;
	cfg["version"] = game_config::version;

	char buf[50];
	sprintf(buf,"%d",game.gold);
	cfg["gold"] = buf;

	cfg["scenario"] = game.scenario;

	cfg["campaign_type"] = game.campaign_type;

	cfg["difficulty"] = game.difficulty;

	config* vars = new config();
	vars->values = game.variables;
	cfg.children["variables"].push_back(vars);

	for(std::vector<unit>::const_iterator i = game.available_units.begin();
	    i != game.available_units.end(); ++i) {
		config* const new_cfg = new config;
		i->write(*new_cfg);
		cfg.children["unit"].push_back(new_cfg);
	}

	if(game.replay_data.children.empty() == false) {
		cfg.children["replay"].push_back(new config(game.replay_data));
	}

	cfg.children["start"].push_back(new config(game.starting_pos));
}

std::vector<std::string> get_saves_list()
{
	std::vector<std::string> res;
	get_files_in_dir(get_saves_dir(),&res);
	for(std::vector<std::string>::iterator i = res.begin(); i != res.end(); ++i)
		std::replace(i->begin(),i->end(),'_',' ');
	return res;
}


bool save_game_exists(const std::string & name)
{
	std::string fname = name;
	std::replace(fname.begin(),fname.end(),' ','_');
	
	return file_exists(get_saves_dir() + "/" + fname);
}

void load_game(game_data& data, const std::string& name, game_state& state)
{
	log_scope("load_game");
	std::string modified_name = name;
	std::replace(modified_name.begin(),modified_name.end(),' ','_');

	//try reading the file both with and without underscores
	std::string file_data = read_file(get_saves_dir() + "/" + modified_name);
	if(file_data.empty()) {
		file_data = read_file(get_saves_dir() + "/" + name);
	}

	config cfg(file_data);
	state = read_game(data,&cfg);
}

void save_game(const game_state& state)
{
	std::string name = state.label;
	std::replace(name.begin(),name.end(),' ','_');

	config cfg;
	write_game(state,cfg);
	write_file(get_saves_dir() + "/" + name,cfg.write());
}
