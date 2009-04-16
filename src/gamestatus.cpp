/* $Id$ */
/*
   Copyright (C) 2003 - 2009 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file gamestatus.cpp
 * Maintain status of a game, load&save games.
 */

#include "global.hpp"

#include "foreach.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "game_preferences.hpp"
#include "replay.hpp"
#include "statistics.hpp"
#include "unit_id.hpp"
#include "wesconfig.h"
#include "wml_exception.hpp"
#include "formula_string_utils.hpp"

#ifndef _MSC_VER
#include <sys/time.h>
#endif

#define DBG_NG lg::debug(lg::engine)
#define LOG_NG lg::info(lg::engine)
#define WRN_NG lg::warn(lg::engine)
#define ERR_NG lg::err(lg::engine)

#ifdef _WIN32

static void write_player(const player_info& player, config& cfg);

#endif /* _WIN32 */

player_info* game_state::get_player(const std::string& id) {
	std::map< std::string, player_info >::iterator found = players.find(id);
	if (found == players.end()) {
		LOG_STREAM(warn, engine) << "player " << id << " does not exist.\n";
		return NULL;
	} else
		return &found->second;
}

#ifdef __UNUSED__
std::string generate_game_uuid()
{
	struct timeval ts;
	std::stringstream uuid;
	gettimeofday(&ts, NULL);

	uuid << preferences::login() << "@" << ts.tv_sec << "." << ts.tv_usec;

	return uuid.str();
}
#endif

void gamestatus::add_time_area(const config& cfg)
{
	areas_.push_back(area_time_of_day());
	area_time_of_day &area = areas_.back();
	area.id   = cfg["id"];
	area.xsrc = cfg["x"];
	area.ysrc = cfg["y"];
	std::vector<map_location> const& locs = parse_location_range(area.xsrc, area.ysrc);
	std::copy(locs.begin(), locs.end(), std::inserter(area.hexes, area.hexes.end()));
	time_of_day::parse_times(cfg, area.times);
}

void gamestatus::add_time_area(const std::string& id, const std::set<map_location>& locs,
                               const config& time_cfg)
{
	areas_.push_back(area_time_of_day());
	area_time_of_day& area = areas_.back();
	area.id = id;
	area.hexes = locs;
	time_of_day::parse_times(time_cfg, area.times);
}

void gamestatus::remove_time_area(const std::string& area_id)
{
	if(area_id.empty()) {
		areas_.clear();
	} else {
		// search for all time areas that match the id.
		std::vector<area_time_of_day>::iterator i = areas_.begin();
		while(i != areas_.end()) {
			if((*i).id == area_id) {
				i = areas_.erase(i);
			} else {
				++i;
			}
		}
	}
}

gamestatus::gamestatus(const config& time_cfg, int num_turns, game_state* s_o_g) :
		teams(0),
		times_(),
		areas_(),
		turn_(1),
		numTurns_(num_turns),
		currentTime_(0),
		state_of_game_(s_o_g)
{
	std::string turn_at = time_cfg["turn_at"];
	std::string current_tod = time_cfg["current_tod"];
	std::string random_start_time = time_cfg["random_start_time"];
	if (s_o_g)
	{
	    turn_at = utils::interpolate_variables_into_string(turn_at, *s_o_g);
		current_tod = utils::interpolate_variables_into_string(current_tod, *s_o_g);

	}

	if(turn_at.empty() == false) {
		turn_ = atoi(turn_at.c_str());
	}

	time_of_day::parse_times(time_cfg,times_);

	set_start_ToD(const_cast<config&>(time_cfg),s_o_g);

	foreach (const config &t, time_cfg.child_range("time_area")) {
		this->add_time_area(t);
	}
}

void gamestatus::write(config& cfg) const
{
	std::stringstream buf;
	buf << turn_;
	cfg["turn_at"] = buf.str();
	buf.str(std::string());
	buf << numTurns_;
	cfg["turns"] = buf.str();
	buf.str(std::string());
	buf << currentTime_;
	cfg["current_tod"] = buf.str();

	std::vector<time_of_day>::const_iterator t;
	for(t = times_.begin(); t != times_.end(); ++t) {
		t->write(cfg.add_child("time"));
	}


	for(std::vector<area_time_of_day>::const_iterator i = areas_.begin(); i != areas_.end(); ++i) {
		config& area = cfg.add_child("time_area");
		area["x"] = i->xsrc;
		area["y"] = i->ysrc;
		for(t = i->times.begin(); t != i->times.end(); ++t) {
			t->write(area.add_child("time"));
		}

	}
}

time_of_day gamestatus::get_time_of_day_turn(int nturn) const
{
	VALIDATE(times_.size(), _("No time of day has been defined."));

	int time = (currentTime_ + nturn  - turn())% times_.size();

	if (time < 0)
	{
		time += times_.size();
	}

	return times_[time];
}

time_of_day gamestatus::get_time_of_day() const
{
	VALIDATE(times_.size(), _("No time of day has been defined."));

	return times_[currentTime_];
}

time_of_day gamestatus::get_previous_time_of_day() const
{
	return get_time_of_day_turn(turn()-1);
}

time_of_day gamestatus::get_time_of_day(int illuminated, const map_location& loc, int n_turn) const
{
	time_of_day res = get_time_of_day_turn(n_turn);

	if(loc.valid()) {
		for(std::vector<area_time_of_day>::const_iterator i = areas_.begin(); i != areas_.end(); ++i) {
			if(i->hexes.count(loc) == 1) {

				VALIDATE(i->times.size(), _("No time of day has been defined."));

				res = i->times[(n_turn-1)%i->times.size()];
				break;
			}
		}
	}

	if(illuminated) {
		res.bonus_modified=illuminated;
		res.lawful_bonus += illuminated;
	}
	return res;
}

time_of_day gamestatus::get_time_of_day(int illuminated, const map_location& loc) const
{
	return get_time_of_day(illuminated,loc,turn());
}

bool gamestatus::set_time_of_day(int newTime)
{
	// newTime can come from network so have to take run time test
	if( newTime >= static_cast<int>(times_.size())
	  || newTime < 0)
	{
		return false;
	}

	currentTime_ = newTime;

	return true;
}

bool gamestatus::is_start_ToD(const std::string& random_start_time)
{
	return !random_start_time.empty()
			&& utils::string_bool(random_start_time, true);
}

void gamestatus::set_start_ToD(config &level, game_state* s_o_g)
{
	if (!level["current_tod"].empty())
	{
		set_time_of_day(atoi(level["current_tod"].c_str()));
		return;
	}
	std::string random_start_time = level["random_start_time"];
	if (s_o_g)
	{
		random_start_time = utils::interpolate_variables_into_string(random_start_time, *s_o_g);
	}
	if (gamestatus::is_start_ToD(random_start_time))
	{
		std::vector<std::string> start_strings =
			utils::split(random_start_time, ',', utils::STRIP_SPACES | utils::REMOVE_EMPTY);

		if (utils::string_bool(random_start_time,false))
		{
			// We had boolean value
			set_time_of_day(rand()%times_.size());
		}
		else
		{
			set_time_of_day(atoi(start_strings[rand()%start_strings.size()].c_str()) - 1);
		}
	}
	else
	{
		// We have to set right ToD for oldsaves

		set_time_of_day((turn() - 1) % times_.size());
	}
	// Setting ToD to level data

	std::stringstream buf;
	buf << currentTime_;
	level["current_tod"] = buf.str();

}

void gamestatus::next_time_of_day()
{
	VALIDATE(times_.size(), _("No time of day has been defined."));

	currentTime_ = (currentTime_ + 1)%times_.size();
}

size_t gamestatus::turn() const
{
	return turn_;
}

int gamestatus::number_of_turns() const
{
	return numTurns_;
}
void gamestatus::modify_turns(const std::string& mod)
{
	numTurns_ = std::max<int>(utils::apply_modifier(numTurns_,mod,0),-1);
}
void gamestatus::add_turns(int num)
{
	numTurns_ = std::max<int>(numTurns_ + num,-1);
}

void gamestatus::set_turn(unsigned int num)
{
	VALIDATE(times_.size(), _("No time of day has been defined."));
	const unsigned int old_num = turn_;
	// Correct ToD
	currentTime_ = (num  - 1) % times_.size();
	if (currentTime_ < 0) {
		currentTime_ += times_.size();
	}
	if(static_cast<int>(num) > numTurns_ && numTurns_ != -1) {
		this->add_turns(numTurns_ - num);
	}
	turn_ = num;

	LOG_NG << "changed current turn number from " << old_num << " to " << num << '\n';
}

bool gamestatus::next_turn()
{
	next_time_of_day();
	++turn_;
	return numTurns_ == -1 || turn_ <= size_t(numTurns_);
}

static player_info read_player(const config &cfg)
{
	player_info res;

	res.name = cfg["name"];

	res.gold = atoi(cfg["gold"].c_str());
	res.gold_add = utils::string_bool(cfg["gold_add"]);

	foreach (const config &u, cfg.child_range("unit")) {
		res.available_units.push_back(unit(u, false));
	}

	res.can_recruit.clear();

	const std::string &can_recruit_str = cfg["can_recruit"];
	if (!can_recruit_str.empty()) {
		const std::vector<std::string> can_recruit = utils::split(can_recruit_str);
		std::copy(can_recruit.begin(),can_recruit.end(),std::inserter(res.can_recruit,res.can_recruit.end()));
	}

	return res;
}

game_state::game_state()  :
		label(),
		version(),
		campaign_type(),
		campaign_define(),
		campaign_xtra_defines(),
		campaign(),
		history(),
		abbrev(),
		scenario(),
		next_scenario(),
		completion(),
		end_text(),
		end_text_duration(),
		players(),
		scoped_variables(),
		wml_menu_items(),
		difficulty("NORMAL"),
		replay_data(),
		starting_pos(),
		snapshot(),
		last_selected(map_location::null_location),
		rng_(),
		variables(),
		temporaries(),
		generator_setter(&recorder)
		{}

static void write_player(const player_info& player, config& cfg)
{
	cfg["name"] = player.name;

	char buf[50];
	snprintf(buf,sizeof(buf),"%d",player.gold);

	cfg["gold"] = buf;

	cfg["gold_add"] = player.gold_add ? "yes" : "no";

	for(std::vector<unit>::const_iterator i = player.available_units.begin();
	    i != player.available_units.end(); ++i) {
		config new_cfg;
		i->write(new_cfg);
		cfg.add_child("unit",new_cfg);
		DBG_NG << "added unit '" << new_cfg["id"] << "' to player '" << player.name << "'\n";
	}

	std::stringstream can_recruit;
	std::copy(player.can_recruit.begin(),player.can_recruit.end(),std::ostream_iterator<std::string>(can_recruit,","));
	std::string can_recruit_str = can_recruit.str();

	// Remove the trailing comma
	if(can_recruit_str.empty() == false) {
		can_recruit_str.resize(can_recruit_str.size()-1);
	}

	cfg["can_recruit"] = can_recruit_str;
}

void write_players(game_state& gamestate, config& cfg)
{
	// If there is already a player config available it means we are loading
	// from a savegame. Don't do anything then, the information is already there
	config::child_itors player_cfg = cfg.child_range("player");
	if (player_cfg.first != player_cfg.second)
		return;

	for(std::map<std::string, player_info>::const_iterator i=gamestate.players.begin();
		i!=gamestate.players.end(); ++i)
	{
		config new_cfg;
		write_player(i->second, new_cfg);
		new_cfg["save_id"]=i->first;
		cfg.add_child("player", new_cfg);
	}
}

game_state::game_state(const config& cfg, bool show_replay) :
		label(cfg["label"]),
		version(cfg["version"]),
		campaign_type(cfg["campaign_type"]),
		campaign_define(cfg["campaign_define"]),
		campaign_xtra_defines(utils::split(cfg["campaign_extra_defines"])),
		campaign(cfg["campaign"]),
		history(cfg["history"]),
		abbrev(cfg["abbrev"]),
		scenario(cfg["scenario"]),
		next_scenario(cfg["next_scenario"]),
		completion(cfg["completion"]),
		end_text(cfg["end_text"]),
		end_text_duration(lexical_cast_default<unsigned int>(cfg["end_text_duration"])),
		players(),
		scoped_variables(),
		wml_menu_items(),
		difficulty(cfg["difficulty"]),
		replay_data(),
		starting_pos(),
		snapshot(),
		last_selected(map_location::null_location),
		rng_(cfg),
		variables(),
		temporaries(),
		generator_setter(&recorder)
{
	n_unit::id_manager::instance().set_save_id(lexical_cast_default<size_t>(cfg["next_underlying_unit_id"],0));
	log_scope("read_game");

	const config &snapshot = cfg.child("snapshot");
	const config &replay_start = cfg.child("replay_start");

	// We have to load era id for MP games so they can load correct era.


	if (snapshot && !snapshot.empty() && !show_replay) {

		this->snapshot = snapshot;

		rng_.seed_random(lexical_cast_default<unsigned>(snapshot["random_calls"]));

		// Midgame saves have the recall list stored in the snapshot.
		load_recall_list(snapshot.child_range("player"));

	} else {
		assert(replay_start != NULL);
		
		// The player information should no longer be saved to the root of the config.
		// The game now looks for the info in just the snapshot or the starting position.
		// Check if we find some player information in the starting position
		config::const_child_itors cfg_players = replay_start.child_range("player");

		if (cfg_players.first != cfg_players.second)
			load_recall_list(cfg_players);
	}

	LOG_NG << "scenario: '" << scenario << "'\n";
	LOG_NG << "next_scenario: '" << next_scenario << "'\n";

	if(difficulty.empty()) {
		difficulty = "NORMAL";
	}

	if(campaign_type.empty()) {
		campaign_type = "scenario";
	}

	if (const config &vars = cfg.child("variables")) {
		set_variables(vars);
	}
	set_menu_items(cfg.child_range("menu_item"));

	if (const config &replay = cfg.child("replay")) {
		replay_data = replay;
	}

	if (replay_start) {
		starting_pos = replay_start;
		//This is a quick hack to make replays for campaigns work again:
		//The [player] information needs to be stored somewhere within the gamestate,
		//because we need it later on when creating the replay savegame.
		//We therefore put it inside the starting_pos, so it doesn't get lost.
		//See also playcampaign::play_game, where after finishing the scenario the replay
		//will be saved.
		if(!starting_pos.empty()) {
			foreach (const config &p, cfg.child_range("player")) {
				config& cfg_player = starting_pos.add_child("player");
				cfg_player.merge_with(p);
			}
		}
	}

	if (const config &stats = cfg.child("statistics")) {
		statistics::fresh_stats();
		statistics::read_stats(stats);
	}
}

void game_state::write_snapshot(config& cfg) const
{
	log_scope("write_game");
	cfg["label"] = label;
	cfg["history"] = history;
	cfg["abbrev"] = abbrev;
	cfg["version"] = game_config::version;

	cfg["scenario"] = scenario;
	cfg["next_scenario"] = next_scenario;

	cfg["completion"] = completion;

	cfg["campaign"] = campaign;
	cfg["campaign_type"] = campaign_type;
	cfg["difficulty"] = difficulty;

	cfg["campaign_define"] = campaign_define;
	cfg["campaign_extra_defines"] = utils::join(campaign_xtra_defines);
	cfg["next_underlying_unit_id"] = lexical_cast<std::string>(n_unit::id_manager::instance().get_save_id());

	cfg["random_seed"] = lexical_cast<std::string>(rng_.get_random_seed());
	cfg["random_calls"] = lexical_cast<std::string>(rng_.get_random_calls());

	cfg["end_text"] = end_text;
	cfg["end_text_duration"] = str_cast<unsigned int>(end_text_duration);

	cfg.add_child("variables", variables);

	for(std::map<std::string, wml_menu_item *>::const_iterator j=wml_menu_items.begin();
	    j!=wml_menu_items.end(); ++j) {
		config new_cfg;
		new_cfg["id"]=j->first;
		new_cfg["image"]=j->second->image;
		new_cfg["description"]=j->second->description;
		new_cfg["needs_select"]= (j->second->needs_select) ? "yes" : "no";
		if(!j->second->show_if.empty())
			new_cfg.add_child("show_if", j->second->show_if);
		if(!j->second->filter_location.empty())
			new_cfg.add_child("filter_location", j->second->filter_location);
		if(!j->second->command.empty())
			new_cfg.add_child("command", j->second->command);
		cfg.add_child("menu_item", new_cfg);
	}

	for(std::map<std::string, player_info>::const_iterator i=players.begin();
	    i!=players.end(); ++i) {
		config new_cfg;
		::write_player(i->second, new_cfg);
		new_cfg["save_id"]=i->first;
		cfg.add_child("player", new_cfg);
	}
}

void extract_summary_from_config(config& cfg_save, config& cfg_summary)
{
	const config &cfg_snapshot = cfg_save.child("snapshot");
	const config &cfg_replay_start = cfg_save.child("replay_start");

	const config &cfg_replay = cfg_save.child("replay");
	const bool has_replay = cfg_replay && !cfg_replay.empty();
	const bool has_snapshot = cfg_snapshot && cfg_snapshot.child("side");

	cfg_summary["replay"] = has_replay ? "yes" : "no";
	cfg_summary["snapshot"] = has_snapshot ? "yes" : "no";

	cfg_summary["label"] = cfg_save["label"];
	cfg_summary["campaign_type"] = cfg_save["campaign_type"];
	cfg_summary["scenario"] = cfg_save["scenario"];
	cfg_summary["campaign"] = cfg_save["campaign"];
	cfg_summary["difficulty"] = cfg_save["difficulty"];
	cfg_summary["version"] = cfg_save["version"];
	cfg_summary["corrupt"] = "";

	if(has_snapshot) {
		cfg_summary["turn"] = cfg_snapshot["turn_at"];
		if (cfg_snapshot["turns"] != "-1") {
			cfg_summary["turn"] = cfg_summary["turn"].str() + "/" + cfg_snapshot["turns"].str();
		}
	}

	// Find the first human leader so we can display their icon in the load menu.

	/** @todo Ideally we should grab all leaders if there's more than 1 human player? */
	std::string leader;

	foreach (const config &p, cfg_save.child_range("player"))
	{
		if (utils::string_bool(p["canrecruit"], false)) {
			leader = p["save_id"];
		}
	}

	bool shrouded = false;

	if (!leader.empty())
	{
		if (const config &snapshot = *(has_snapshot ? &cfg_snapshot : &cfg_replay_start))
		{
			foreach (const config &side, snapshot.child_range("side"))
			{
				if (side["controller"] != "human") {
					continue;
				}

				if (utils::string_bool(side["shroud"])) {
					shrouded = true;
				}

				foreach (const config &u, side.child_range("unit"))
				{
					if (utils::string_bool(u["canrecruit"], false)) {
						leader = u["id"];
						break;
					}
				}
			}
		}
	}

	cfg_summary["leader"] = leader;
	cfg_summary["map_data"] = "";

	if(!shrouded) {
		if(has_snapshot) {
			if (!cfg_snapshot.find_child("side", "shroud", "yes")) {
				cfg_summary["map_data"] = cfg_snapshot["map_data"];
			}
		} else if(has_replay) {
			if (!cfg_replay_start.find_child("side","shroud","yes")) {
				cfg_summary["map_data"] = cfg_replay_start["map_data"];
			}
		}
	}
}

t_string& game_state::get_variable(const std::string& key)
{
	return variable_info(key, true, variable_info::TYPE_SCALAR).as_scalar();
}

const t_string& game_state::get_variable_const(const std::string& key) const
{
	variable_info to_get(key, false, variable_info::TYPE_SCALAR);
	if(!to_get.is_valid) return temporaries[key];
	return to_get.as_scalar();
}

config& game_state::get_variable_cfg(const std::string& key)
{
	return variable_info(key, true, variable_info::TYPE_CONTAINER).as_container();
}

variable_info::array_range game_state::get_variable_cfgs(const std::string& key)
{
	return variable_info(key, true, variable_info::TYPE_ARRAY).as_array();
}

void game_state::set_variable(const std::string& key, const t_string& value)
{
	get_variable(key) = value;
}

config& game_state::add_variable_cfg(const std::string& key, const config& value)
{
	variable_info to_add(key, true, variable_info::TYPE_ARRAY);
	return to_add.vars->add_child(to_add.key, value);
}

void game_state::clear_variable_cfg(const std::string& varname)
{
	variable_info to_clear(varname, false, variable_info::TYPE_CONTAINER);
	if(!to_clear.is_valid) return;
	if(to_clear.explicit_index) {
		to_clear.vars->remove_child(to_clear.key, to_clear.index);
	} else {
		to_clear.vars->clear_children(to_clear.key);
	}
}

void game_state::clear_variable(const std::string& varname)
{
	variable_info to_clear(varname, false);
	if(!to_clear.is_valid) return;
	if(to_clear.explicit_index) {
		to_clear.vars->remove_child(to_clear.key, to_clear.index);
	} else {
		to_clear.vars->clear_children(to_clear.key);
		to_clear.vars->remove_attribute(to_clear.key);
	}
}

void game_state::load_recall_list(const config::const_child_itors &players)
{
	if (players.first == players.second) return;

	foreach (const config &p, players)
	{
		const std::string &save_id = p["save_id"];

		if (save_id.empty()) {
			ERR_NG << "Corrupted player entry: NULL save_id" << std::endl;
		} else {
			player_info player = read_player(p);
			this->players.insert(std::make_pair(save_id, player));
		}
	}
}

static void clear_wmi(std::map<std::string, wml_menu_item*>& gs_wmi) {
	std::map<std::string, wml_menu_item*>::iterator itor = gs_wmi.begin();
	for(itor = gs_wmi.begin(); itor != gs_wmi.end(); ++itor) {
		delete itor->second;
	}
	gs_wmi.clear();
}

game_state::game_state(const game_state& state) :
	/* default construct everything to silence compiler warnings. */
	variable_set(),
	label(),
	version(),
	campaign_type(),
	campaign_define(),
	campaign_xtra_defines(),
	campaign(),
	history(),
	abbrev(),
	scenario(),
	next_scenario(),
	completion(),
	end_text(),
	end_text_duration(),
	players(),
	scoped_variables(),
	wml_menu_items(),
	difficulty(),
	replay_data(),
	starting_pos(),
	snapshot(),
	last_selected(),
	rng_(),
	variables(),
	temporaries(),
	generator_setter(&recorder)
{
	*this = state;
}

game_state& game_state::operator=(const game_state& state)
{
	if(this == &state) {
		return *this;
	}

	history = state.history;
	abbrev = state.abbrev;
	label = state.label;
	version = state.version;
	campaign_type = state.campaign_type;
	campaign_define = state.campaign_define;
	campaign_xtra_defines = state.campaign_xtra_defines;
	campaign = state.campaign;
	scenario = state.scenario;
	completion = state.completion;
	end_text = state.end_text;
	end_text_duration = state.end_text_duration;
	rng_ = state.rng_;
	players = state.players;
	scoped_variables = state.scoped_variables;

	clear_wmi(wml_menu_items);
	std::map<std::string, wml_menu_item*>::const_iterator itor;
	for (itor = state.wml_menu_items.begin(); itor != state.wml_menu_items.end(); ++itor) {
		wml_menu_item*& mref = wml_menu_items[itor->first];
		mref = new wml_menu_item(*(itor->second));
	}

	difficulty = state.difficulty;
	replay_data = state.replay_data;
	starting_pos = state.starting_pos;
	snapshot = state.snapshot;
	last_selected = state.last_selected;
	set_variables(state.get_variables());

	return *this;
}

game_state::~game_state() {
	clear_wmi(wml_menu_items);
}

void game_state::set_variables(const config& vars) {
	if(!variables.empty()) {
		WRN_NG << "clobbering the game_state variables\n";
		WRN_NG << variables;
	}
	variables = vars;
}

void game_state::set_menu_items(const config::const_child_itors &menu_items)
{
	clear_wmi(wml_menu_items);
	foreach (const config &item, menu_items)
	{
		const std::string &id = item["id"].base_str();
		wml_menu_item*& mref = wml_menu_items[id];
		if(mref == NULL) {
			mref = new wml_menu_item(id, &item);
		} else {
			WRN_NG << "duplicate menu item (" << id << ") while loading gamestate\n";
		}
	}
}

void player_info::debug(){
	LOG_NG << "Debugging player\n";
	LOG_NG << "\tName: " << name << "\n";
	LOG_NG << "\tGold: " << gold << "\n";
	LOG_NG << "\tAvailable units:\n";
	for (std::vector<unit>::const_iterator u = available_units.begin(); u != available_units.end(); u++){
		LOG_NG << "\t\t" + u->name() + "\n";
	}
	LOG_NG << "\tEnd available units\n";
}

wml_menu_item::wml_menu_item(const std::string& id, const config* cfg) :
		name(),
		image(),
		description(),
		needs_select(false),
		show_if(),
		filter_location(),
		command()

{
	std::stringstream temp;
	temp << "menu item";
	if(!id.empty()) {
		temp << ' ' << id;
	}
	name = temp.str();
	if(cfg != NULL) {
		image = (*cfg)["image"];
		description = (*cfg)["description"];
		needs_select = utils::string_bool((*cfg)["needs_select"], false);
		if (const config &c = cfg->child("show_if")) show_if = c;
		if (const config &c = cfg->child("filter_location")) filter_location = c;
		if (const config &c = cfg->child("command")) command = c;
	}
}
