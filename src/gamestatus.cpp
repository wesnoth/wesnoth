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
#include <iterator>
#include <sstream>

time_of_day::time_of_day(const config& cfg)
                 : lawful_bonus(atoi(cfg["lawful_bonus"].c_str())),
                   image(cfg["image"]), name(cfg["name"]), id(cfg["id"]),
                   red(atoi(cfg["red"].c_str())),
                   green(atoi(cfg["green"].c_str())),
                   blue(atoi(cfg["blue"].c_str()))
{
	const std::string& lang_name = string_table[cfg["id"]];
	if(lang_name.empty() == false)
		name = lang_name;
}

gamestatus::gamestatus(config& time_cfg, int num_turns) :
                 turn_(1),numTurns_(num_turns)
{
	const config::child_list& times = time_cfg.get_children("time");
	config::child_list::const_iterator t;
	for(t = times.begin(); t != times.end(); ++t) {
		times_.push_back(time_of_day(**t));
	}

	const config::child_list& times_illum = time_cfg.get_children("illuminated_time");
	const config::child_list& illum = times_illum.empty() ? times : times_illum;

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

	const config::child_list& units = cfg->get_children("unit");
	for(config::child_list::const_iterator i = units.begin(); i != units.end(); ++i) {
		res.available_units.push_back(unit(data,**i));
	}

	const config* const vars = cfg->child("variables");
	if(vars != NULL) {
		res.variables = vars->values;
	}

	const config* const replay = cfg->child("replay");
	if(replay != NULL) {
		res.replay_data = *replay;
	}

	const config* starting_pos = cfg->child("start");
	if(starting_pos != NULL) {
		res.starting_pos = *starting_pos;
	}

	res.can_recruit.clear();

	const std::string& can_recruit_str = (*cfg)["can_recruit"];
	if(can_recruit_str != "") {
		const std::vector<std::string> can_recruit = config::split(can_recruit_str);
		std::copy(can_recruit.begin(),can_recruit.end(),std::inserter(res.can_recruit,res.can_recruit.end()));
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

	cfg.add_child("variables").values = game.variables;

	for(std::vector<unit>::const_iterator i = game.available_units.begin();
	    i != game.available_units.end(); ++i) {
		config new_cfg;
		i->write(new_cfg);
		cfg.add_child("unit",new_cfg);
	}

	if(game.replay_data.children.empty() == false) {
		cfg.add_child("replay",game.replay_data);
	}

	cfg.add_child("start",game.starting_pos);

	std::stringstream can_recruit;
	std::copy(game.can_recruit.begin(),game.can_recruit.end(),std::ostream_iterator<std::string>(can_recruit,","));
	std::string can_recruit_str = can_recruit.str();

	//remove the trailing comma
	if(can_recruit_str.size() > 0)
		can_recruit_str.resize(can_recruit_str.size()-1);

	cfg["can_recruit"] = can_recruit_str;
}

//a structure for comparing to save_info objects based on their modified time.
//if the times are equal, will order based on the name
struct save_info_less_time {
	bool operator()(const save_info& a, const save_info& b) const {
		return a.time_modified > b.time_modified ||
		       a.time_modified == b.time_modified && a.name > b.name;
	}
};

std::vector<save_info> get_saves_list()
{
	const std::string& saves_dir = get_saves_dir();

	std::vector<std::string> saves;
	get_files_in_dir(saves_dir,&saves);

	std::vector<save_info> res;
	for(std::vector<std::string>::iterator i = saves.begin(); i != saves.end(); ++i) {
		const time_t modified = file_create_time(saves_dir + "/" + *i);

		std::replace(i->begin(),i->end(),'_',' ');
		res.push_back(save_info(*i,modified));
	}

	std::sort(res.begin(),res.end(),save_info_less_time());

	return res;
}

bool save_game_exists(const std::string& name)
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
