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

#include <cstdio>
#include <sstream>

gamestatus::gamestatus(int num_turns) :
                 timeofday_(DAWN),turn_(1),numTurns_(num_turns)
{}

const std::string& gamestatus::timeofdayDescription(gamestatus::TIME t)
{
	static const std::string times[] = { "Dawn", "Morning", "Afternoon",
	                                     "Dusk", "FirstWatch", "SecondWatch" };
	return times[t];
}

gamestatus::TIME gamestatus::timeofday() const
{
	return timeofday_;
}

int gamestatus::turn() const
{
	return turn_;
}

int gamestatus::number_of_turns() const
{
	return numTurns_;
}

bool gamestatus::next_turn()
{
	timeofday_ = static_cast<TIME>(static_cast<int>(timeofday_)+1);
	if(timeofday_ == NUM_TIMES)
		timeofday_ = DAWN;
	++turn_;
	return turn_ <= numTurns_;
}

game_state read_game(game_data& data, config* cfg)
{
	game_state res;
	res.label = cfg->values["label"];
	res.version = cfg->values["version"];
	res.gold = atoi(cfg->values["gold"].c_str());
	res.scenario = atoi(cfg->values["scenario"].c_str());

	res.difficulty = cfg->values["difficulty"];
	if(res.difficulty.empty())
		res.difficulty = "NORMAL";

	res.campaign_type = cfg->values["campaign_type"];
	if(res.campaign_type.empty())
		res.campaign_type = "scenario";

	std::vector<config*>& units = cfg->children["unit"];
	for(std::vector<config*>::iterator i = units.begin();
	    i != units.end(); ++i) {
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
	cfg.values["label"] = game.label;
	cfg.values["version"] = game_config::version;

	char buf[50];
	sprintf(buf,"%d",game.gold);
	cfg.values["gold"] = buf;

	sprintf(buf,"%d",game.scenario);
	cfg.values["scenario"] = buf;

	cfg.values["campaign_type"] = game.campaign_type;

	cfg.values["difficulty"] = game.difficulty;

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
	return res;
}

void load_game(game_data& data, const std::string& name, game_state& state)
{
	config cfg(read_file(get_saves_dir() + "/" + name));
	state = read_game(data,&cfg);
}

void save_game(const game_state& state)
{
	const std::string& name = state.label;

	config cfg;
	write_game(state,cfg);
	write_file(get_saves_dir() + "/" + name,cfg.write());
}
