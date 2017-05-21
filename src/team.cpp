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

/**
 *  @file
 *  Team-management, allies, setup at start of scenario.
 */

#include "team.hpp"

#include "ai/manager.hpp"
#include "font/text_formatting.hpp"
#include "formula/string_utils.hpp"
#include "game_events/pump.hpp"
#include "game_data.hpp"
#include "map/map.hpp"
#include "resources.hpp"
#include "play_controller.hpp"
#include "preferences/game.hpp"
#include "color.hpp"
#include "units/types.hpp"
#include "synced_context.hpp"
#include "whiteboard/side_actions.hpp"
#include "serialization/string_utils.hpp"
#include <boost/dynamic_bitset.hpp>

static lg::log_domain log_engine("engine");
#define DBG_NG LOG_STREAM(debug, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)
#define WRN_NG LOG_STREAM(warn, log_engine)
#define ERR_NG LOG_STREAM(err, log_engine)

static lg::log_domain log_engine_enemies("engine/enemies");
#define DBG_NGE LOG_STREAM(debug, log_engine_enemies)
#define LOG_NGE LOG_STREAM(info, log_engine_enemies)
#define WRN_NGE LOG_STREAM(warn, log_engine_enemies)

//static member initialization
const int team::default_team_gold_ = 100;

// Update this list of attributes if you change what is used to define a side
// (excluding those attributes used to define the side's leader).
const std::set<std::string> team::attributes {"ai_config",
	"carryover_add", "carryover_percentage", "color", "controller", "current_player", "defeat_condition", "flag",
	"flag_icon", "fog", "fog_data", "gold", "hidden", "income",
	"no_leader", "objectives", "objectives_changed", "persistent", "lost",
	"recall_cost", "recruit", "save_id", "scroll_to_leader",
	"share_vision", "share_maps" ,"share_view", "shroud", "shroud_data", "start_gold",
	"suppress_end_turn_confirmation",
	"team_name", "user_team_name", "side_name", "village_gold", "village_support", "is_local",
	// Multiplayer attributes.
	"player_id", "is_host","action_bonus_count", "allow_changes", "allow_player", "color_lock",
	"countdown_time", "disallow_observers", "faction",
	"faction_from_recruit", "faction_name", "gold_lock", "income_lock",
	"leader", "random_leader", "team_lock", "terrain_liked",
	"user_description", "controller_lock", "chose_random",
	"disallow_shuffle", "description"};

team::team_info::team_info() :
	gold(0),
	start_gold(0),
	income(0),
	income_per_village(0),
	support_per_village(1),
	minimum_recruit_price(0),
	recall_cost(0),
	can_recruit(),
	team_name(),
	user_team_name(),
	side_name(),
	faction(),
	faction_name(),
	save_id(),
	current_player(),
	countdown_time(),
	action_bonus_count(0),
	flag(),
	flag_icon(),
	id(),
	scroll_to_leader(true),
	objectives(),
	objectives_changed(false),
	controller(),
	is_local(true),
	defeat_condition(team::DEFEAT_CONDITION::NO_LEADER),
	proxy_controller(team::PROXY_CONTROLLER::PROXY_HUMAN),
	share_vision(team::SHARE_VISION::ALL),
	disallow_observers(false),
	allow_player(false),
	chose_random(false),
	no_leader(true),
	hidden(true),
	no_turn_confirmation(false),
	color(),
	side(1),
	persistent(false),
	lost(false),
	carryover_percentage(game_config::gold_carryover_percentage),
	carryover_add(false),
	carryover_bonus(0),
	carryover_gold(0)
{
}

void team::team_info::read(const config &cfg)
{
	gold = cfg["gold"];
	income = cfg["income"];
	team_name = cfg["team_name"].str();
	user_team_name = cfg["user_team_name"];
	side_name = cfg["side_name"];
	faction = cfg["faction"].str();
	faction_name = cfg["faction_name"];
	save_id = cfg["save_id"].str();
	current_player = cfg["current_player"].str();
	countdown_time = cfg["countdown_time"].str();
	action_bonus_count = cfg["action_bonus_count"];
	flag = cfg["flag"].str();
	flag_icon = cfg["flag_icon"].str();
	id = cfg["id"].str();
	scroll_to_leader = cfg["scroll_to_leader"].to_bool(true);
	objectives = cfg["objectives"];
	objectives_changed = cfg["objectives_changed"].to_bool();
	disallow_observers = cfg["disallow_observers"].to_bool();
	allow_player = cfg["allow_player"].to_bool(true);
	chose_random = cfg["chose_random"].to_bool(false);
	no_leader = cfg["no_leader"].to_bool();
	defeat_condition = cfg["defeat_condition"].to_enum<team::DEFEAT_CONDITION>(team::DEFEAT_CONDITION::NO_LEADER);
	lost = cfg["lost"].to_bool(false);
	hidden = cfg["hidden"].to_bool();
	no_turn_confirmation = cfg["suppress_end_turn_confirmation"].to_bool();
	side = cfg["side"].to_int(1);
	carryover_percentage = cfg["carryover_percentage"].to_int(game_config::gold_carryover_percentage);
	carryover_add = cfg["carryover_add"].to_bool(false);
	carryover_bonus = cfg["carryover_bonus"].to_double(1);
	carryover_gold = cfg["carryover_gold"].to_int(0);
	variables = cfg.child_or_empty("variables");
	is_local = cfg["is_local"].to_bool(true);

	if(cfg.has_attribute("color")) {
		color = cfg["color"].str();
	} else {
		color = cfg["side"].str();
	}

	// If arel starting new scenario override settings from [ai] tags
	if (!user_team_name.translatable())
		user_team_name = t_string::from_serialized(user_team_name);

	if(cfg.has_attribute("ai_config")) {
		ai::manager::add_ai_for_side_from_file(side, cfg["ai_config"], true);
	} else {
		ai::manager::add_ai_for_side_from_config(side, cfg, true);
	}

	std::vector<std::string> recruits = utils::split(cfg["recruit"]);
	can_recruit.insert(recruits.begin(), recruits.end());

	// at the start of a scenario "start_gold" is not set, we need to take the
	// value from the gold setting (or fall back to the gold default)
	if (!cfg["start_gold"].empty())
		start_gold = cfg["start_gold"];
	else if (!cfg["gold"].empty())
		start_gold = gold;
	else
		start_gold = default_team_gold_;

	if(team_name.empty()) {
		team_name = cfg["side"].str();
	}

	if(save_id.empty()) {
		save_id = id;
	}

	income_per_village = cfg["village_gold"].to_int(game_config::village_income);
	recall_cost = cfg["recall_cost"].to_int(game_config::recall_cost);

	const std::string& village_support = cfg["village_support"];
	if(village_support.empty())
		support_per_village = game_config::village_support;
	else
		support_per_village = lexical_cast_default<int>(village_support, game_config::village_support);

	controller = team::CONTROLLER::AI;
	controller.parse(cfg["controller"].str());

	//TODO: Why do we read disallow observers differently when controller is empty?
	if (controller == CONTROLLER::EMPTY) {
		disallow_observers = cfg["disallow_observers"].to_bool(true);
	}
	//override persistence flag if it is explicitly defined in the config
	//by default, persistence of a team is set depending on the controller
	persistent = cfg["persistent"].to_bool(this->controller == CONTROLLER::HUMAN);

	//========================================================
	//END OF MESSY CODE

	// Share_view and share_maps can't both be enabled,
	// so share_view overrides share_maps.
	share_vision = cfg["share_vision"].to_enum<team::SHARE_VISION>(team::SHARE_VISION::ALL);
	handle_legacy_share_vision(cfg);
	LOG_NG << "team_info::team_info(...): team_name: " << team_name
	       << ", share_vision: " << share_vision << ".\n";
}

void team::team_info::handle_legacy_share_vision(const config& cfg)
{
	if(cfg.has_attribute("share_view") || cfg.has_attribute("share_maps"))
	{
		if(cfg["share_view"].to_bool()) {
			share_vision = team::SHARE_VISION::ALL;
		}
		else if(cfg["share_maps"].to_bool(true)) {
			share_vision = team::SHARE_VISION::SHROUD;
		}
		else {
			share_vision = team::SHARE_VISION::NONE;
		}
	}
}

void team::team_info::write(config& cfg) const
{
	cfg["gold"] = gold;
	cfg["start_gold"] = start_gold;
	cfg["income"] = income;
	cfg["team_name"] = team_name;
	cfg["user_team_name"] = user_team_name;
	cfg["side_name"] = side_name;
	cfg["faction"] = faction;
	cfg["faction_name"] = faction_name;
	cfg["save_id"] = save_id;
	cfg["current_player"] = current_player;
	cfg["flag"] = flag;
	cfg["flag_icon"] = flag_icon;
	cfg["id"] = id;
	cfg["objectives"] = objectives;
	cfg["objectives_changed"] = objectives_changed;
	cfg["countdown_time"]= countdown_time;
	cfg["action_bonus_count"]= action_bonus_count;
	cfg["village_gold"] = income_per_village;
	cfg["village_support"] = support_per_village;
	cfg["recall_cost"] = recall_cost;
	cfg["disallow_observers"] = disallow_observers;
	cfg["allow_player"] = allow_player;
	cfg["chose_random"] = chose_random;
	cfg["no_leader"] = no_leader;
	cfg["defeat_condition"] = defeat_condition;
	cfg["hidden"] = hidden;
	cfg["suppress_end_turn_confirmation"] = no_turn_confirmation;
	cfg["scroll_to_leader"] = scroll_to_leader;
	cfg["controller"] = controller;
	cfg["recruit"] = utils::join(can_recruit);
	cfg["share_vision"] = share_vision;

	cfg["color"] = color;
	cfg["persistent"] = persistent;
	cfg["lost"] = lost;
	cfg["carryover_percentage"] = carryover_percentage;
	cfg["carryover_add"] = carryover_add;
	cfg["carryover_bonus"] = carryover_bonus;
	cfg["carryover_gold"] = carryover_gold;

	if(!variables.empty()) {
		cfg.add_child("variables", variables);
	}

	cfg.add_child("ai", ai::manager::to_config(side));
}

team::team() :
	gold_(0),
	villages_(),
	shroud_(),
	fog_(),
	fog_clearer_(),
	auto_shroud_updates_(true),
	info_(),
	countdown_time_(0),
	action_bonus_count_(0),
	recall_list_(),
	last_recruit_(),
	enemies_(),
	ally_shroud_(),
	ally_fog_(),
	planned_actions_()
{
}

team::~team()
{
}

void team::build(const config &cfg, const gamemap& map, int gold)
{
	gold_ = gold;
	info_.read(cfg);

	fog_.set_enabled(cfg["fog"].to_bool());
	fog_.read(cfg["fog_data"]);
	shroud_.set_enabled(cfg["shroud"].to_bool());
	shroud_.read(cfg["shroud_data"]);
	auto_shroud_updates_ = cfg["auto_shroud"].to_bool(auto_shroud_updates_);

	LOG_NG << "team::team(...): team_name: " << info_.team_name
	       << ", shroud: " << uses_shroud() << ", fog: " << uses_fog() << ".\n";

	// Load the WML-cleared fog.
	const config &fog_override = cfg.child("fog_override");
	if ( fog_override ) {
		const std::vector<map_location> fog_vector =
			map.parse_location_range(fog_override["x"], fog_override["y"], true);
		fog_clearer_.insert(fog_vector.begin(), fog_vector.end());
	}

	// To ensure some minimum starting gold,
	// gold is the maximum of 'gold' and what is given in the config file
	gold_ = std::max(gold, info_.gold);
	if (gold_ != info_.gold)
		info_.start_gold = gold;
	// Old code was doing:
	// info_.start_gold = std::to_string(gold) + " (" + info_.start_gold + ")";
	// Was it correct?

	// Load in the villages the side controls at the start
	for (const config &v : cfg.child_range("village"))
	{
		map_location loc(v);
		if (map.is_village(loc)) {
			villages_.insert(loc);
		} else {
			WRN_NG << "[side] " << current_player() << " [village] points to a non-village location " << loc << std::endl;
		}
	}

	countdown_time_ = cfg["countdown_time"];
	action_bonus_count_ = cfg["action_bonus_count"];

	planned_actions_.reset(new wb::side_actions());
	planned_actions_->set_team_index(info_.side - 1);
}

void team::write(config& cfg) const
{
	info_.write(cfg);
	cfg["auto_shroud"] = auto_shroud_updates_;
	cfg["shroud"] = uses_shroud();
	cfg["fog"] = uses_fog();
	cfg["gold"] = gold_;

	// Write village locations
	for(std::set<map_location>::const_iterator t = villages_.begin(); t != villages_.end(); ++t) {
		t->write(cfg.add_child("village"));
	}

	cfg["shroud_data"] = shroud_.write();
	cfg["fog_data"] = fog_.write();
	if ( !fog_clearer_.empty() )
		write_location_range(fog_clearer_, cfg.add_child("fog_override"));

	cfg["countdown_time"] = countdown_time_;
	cfg["action_bonus_count"] = action_bonus_count_;
}

bool team::get_village(const map_location& loc, const int owner_side, game_data * gamedata)
{
	villages_.insert(loc);
	bool gamestate_changed = false;
	if(gamedata) {
		config::attribute_value& var = gamedata->get_variable("owner_side");
		const config::attribute_value old_value = var;
		var = owner_side;

		// During team building, game_events pump is not guaranteed to exist yet. (At current revision.) We skip capture events in this case.
		if (resources::game_events) {
			gamestate_changed = resources::game_events->pump().fire("capture",loc);
		}

		if(old_value.blank())
			gamedata->clear_variable("owner_side");
		else
			var = old_value;
	}
	return gamestate_changed;
}

void team::lose_village(const map_location& loc)
{
	const std::set<map_location>::const_iterator vil = villages_.find(loc);
	assert(vil != villages_.end());
	villages_.erase(vil);
}

void team::set_recruits(const std::set<std::string>& recruits)
{
	info_.can_recruit = recruits;
	info_.minimum_recruit_price = 0;
	ai::manager::raise_recruit_list_changed();
}

void team::add_recruit(const std::string &recruit)
{
	info_.can_recruit.insert(recruit);
	info_.minimum_recruit_price = 0;
	ai::manager::raise_recruit_list_changed();
}

int team::minimum_recruit_price() const
{
	if(info_.minimum_recruit_price){
		return info_.minimum_recruit_price;
	}else{
		int min = 20;
		for(std::string recruit : info_.can_recruit) {
			const unit_type *ut = unit_types.find(recruit);
			if(!ut)
				continue;
			else{
				if(ut->cost() < min)
					min = ut->cost();
			}

		}
		info_.minimum_recruit_price = min;
	}
	return info_.minimum_recruit_price;
}

void team::calculate_enemies(size_t index) const
{
	if(!resources::gameboard || index >= resources::gameboard->teams().size()) {
		return;
	}

	while(enemies_.size() <= index) {
		enemies_.push_back(calculate_is_enemy(enemies_.size()));
	}
}

bool team::calculate_is_enemy(size_t index) const
{
	// We're not enemies of ourselves
	if(&resources::gameboard->teams()[index] == this) {
		return false;
	}

	// We are friends with anyone who we share a teamname with
	std::vector<std::string> our_teams = utils::split(info_.team_name),
						   their_teams = utils::split(resources::gameboard->teams()[index].info_.team_name);

	LOG_NGE << "team " << info_.side << " calculates if it has enemy in team "<<index+1 << "; our team_name ["<<info_.team_name<<"], their team_name is ["<<resources::gameboard->teams()[index].info_.team_name<<"]"<< std::endl;
	for(std::vector<std::string>::const_iterator t = our_teams.begin(); t != our_teams.end(); ++t) {
		if(std::find(their_teams.begin(), their_teams.end(), *t) != their_teams.end())
		{
			LOG_NGE << "team " << info_.side << " found same team name [" << *t << "] in team "<< index+1 << std::endl;
			return false;
		} else {
			LOG_NGE << "team " << info_.side << " not found same team name [" << *t << "] in team "<< index+1 << std::endl;
		}
	}
	LOG_NGE << "team " << info_.side << " has enemy in team " << index+1 << std::endl;
	return true;
}

namespace
{
	class controller_server_choice : public synced_context::server_choice
	{
	public:
		controller_server_choice(team::CONTROLLER new_controller, const team& team)
			: new_controller_(new_controller)
			, team_(team)
		{
		}
		/// We are in a game with no mp server and need to do this choice locally
		virtual config local_choice() const
		{
			return config {"controller", new_controller_, "is_local", true};
		}
		/// the request which is sended to the mp server.
		virtual config request() const
		{
			return config {
				"new_controller", new_controller_,
				"old_controller", team_.controller(),
				"side", team_.side(),
			};
		}
		virtual const char* name() const
		{
			return "change_controller_wml";
		}
	private:
		team::CONTROLLER new_controller_;
		const team& team_;
	};
}

void team::change_controller_by_wml(const std::string& new_controller_string)
{
	CONTROLLER new_controller;
	if(!new_controller.parse(new_controller_string)) {
		WRN_NG << "ignored attempt to change controller to " << new_controller_string << std::endl;
		return;
	}
	if(new_controller == CONTROLLER::EMPTY && resources::controller->current_side() == this->side()) {
		WRN_NG << "ignored attempt to change the currently playing side's controller to 'null'" << std::endl;
		return;
	}
	config choice = synced_context::ask_server_choice(controller_server_choice(new_controller, *this));
	if(!new_controller.parse(choice["controller"])) {
		//TODO: this should be more than a ERR_NG message.
		// GL-2016SEP02 Oh? So why was ERR_NG defined as warning level? Making the call fit the definition.
		WRN_NG << "recieved an invalid controller string from the server" << choice["controller"] << std::endl;
	}
	if(!resources::controller->is_replay()) {
		set_local(choice["is_local"].to_bool());
	}
	change_controller(new_controller);
}

void team::change_team(const std::string &name, const t_string &user_name)
{
	info_.team_name = name;
	if (!user_name.empty())
	{
		info_.user_team_name = user_name;
	}
	else
	{
		info_.user_team_name = name;
	}

	clear_caches();
}

void team::clear_caches(){
	// Reset the cache of allies for all teams
	if(resources::gameboard) {
		for(auto& t : resources::gameboard->teams()) {
			t.enemies_.clear();
			t.ally_shroud_.clear();
			t.ally_fog_.clear();
		}
	}
}

void team::set_objectives(const t_string& new_objectives, bool silently)
{
	info_.objectives = new_objectives;
	if(!silently)
		info_.objectives_changed = true;
}

bool team::shrouded(const map_location& loc) const
{
	if(!resources::gameboard)
		return shroud_.value(loc.wml_x(),loc.wml_y());

	return shroud_.shared_value(ally_shroud(resources::gameboard->teams()),loc.wml_x(),loc.wml_y());
}

bool team::fogged(const map_location& loc) const
{
	if(shrouded(loc)) return true;

	// Check for an override of fog.
	if ( fog_clearer_.count(loc) > 0 )
		return false;

	if(!resources::gameboard)
		return fog_.value(loc.wml_x(),loc.wml_y());

	return fog_.shared_value(ally_fog(resources::gameboard->teams()),loc.wml_x(),loc.wml_y());
}

const std::vector<const team::shroud_map*>& team::ally_shroud(const std::vector<team>& teams) const
{
	if(ally_shroud_.empty()) {
		for(size_t i = 0; i < teams.size(); ++i) {
			if(!is_enemy(i + 1) && (&(teams[i]) == this || teams[i].share_view() || teams[i].share_maps())) {
				ally_shroud_.push_back(&(teams[i].shroud_));
			}
		}
	}

	return ally_shroud_;
}

const std::vector<const team::shroud_map*>& team::ally_fog(const std::vector<team>& teams) const
{
	if(ally_fog_.empty()) {
		for(size_t i = 0; i < teams.size(); ++i) {
			if(!is_enemy(i + 1) && (&(teams[i]) == this || teams[i].share_view())) {
				ally_fog_.push_back(&(teams[i].fog_));
			}
		}
	}

	return ally_fog_;
}

bool team::knows_about_team(size_t index) const
{
	const team& t = resources::gameboard->teams()[index];

	// We know about our own team
	if(this == &t) return true;

	// If we aren't using shroud or fog, then we know about everyone
	if(!uses_shroud() && !uses_fog()) return true;

	// We don't know about enemies
	if(is_enemy(index+1)) return false;

	// We know our human allies.
	if (t.is_human()) return true;

	// We know about allies we're sharing maps with
	if(share_maps() && t.uses_shroud()) return true;

	// We know about allies we're sharing view with
	if(share_view() && (t.uses_fog() || t.uses_shroud())) return true;

	return false;
}

/**
 * Removes the record of hexes that were cleared of fog via WML.
 * @param[in] hexes	The hexes to no longer keep clear.
 */
void team::remove_fog_override(const std::set<map_location> &hexes)
{
	// Take a set difference.
	std::vector<map_location> result(fog_clearer_.size());
	std::vector<map_location>::iterator result_end =
		std::set_difference(fog_clearer_.begin(), fog_clearer_.end(),
				    hexes.begin(), hexes.end(), result.begin());

	// Put the result into fog_clearer_.
	fog_clearer_.clear();
	fog_clearer_.insert(result.begin(), result_end);
}

void validate_side(int side)
{
	if(!resources::gameboard) {
		return;
	}

	if(side < 1 || side > int(resources::gameboard->teams().size())) {
		throw game::game_error("invalid side(" + std::to_string(side) + ") found in unit definition");
	}
}

bool team::shroud_map::clear(int x, int y)
{
	if(enabled_ == false || x < 0 || y < 0)
		return false;

	if(x >= static_cast<int>(data_.size()))
		data_.resize(x+1);

	if(y >= static_cast<int>(data_[x].size()))
		data_[x].resize(y+1);

	if(data_[x][y] == false) {
		data_[x][y] = true;
		return true;
	} else {
		return false;
	}
}

void team::shroud_map::place(int x, int y)
{
	if(enabled_ == false || x < 0 || y < 0)
		return;

	if (x >= static_cast<int>(data_.size())) {
		DBG_NG << "Couldn't place shroud on invalid x coordinate: ("
			<< x << ", " << y << ") - max x: " << data_.size() - 1 << "\n";
	} else if (y >= static_cast<int>(data_[x].size())) {
		DBG_NG << "Couldn't place shroud on invalid y coordinate: ("
			<< x << ", " << y << ") - max y: " << data_[x].size() - 1 << "\n";
	} else {
		data_[x][y] = false;
	}
}

void team::shroud_map::reset()
{
	if(enabled_ == false)
		return;

	for(std::vector<std::vector<bool> >::iterator i = data_.begin(); i != data_.end(); ++i) {
		std::fill(i->begin(),i->end(),false);
	}
}

bool team::shroud_map::value(int x, int y) const
{
	if ( !enabled_ )
		return false;

	// Locations for which we have no data are assumed to still be covered.
	if ( x < 0  ||  x >= static_cast<int>(data_.size()) )
		return true;
	if ( y < 0  ||  y >= static_cast<int>(data_[x].size()) )
		return true;

	// data_ stores whether or not a location has been cleared, while
	// we want to return whether or not a location is covered.
	return !data_[x][y];
}

bool team::shroud_map::shared_value(const std::vector<const shroud_map*>& maps, int x, int y) const
{
	if ( !enabled_ )
		return false;

	// A quick abort:
	if ( x < 0  ||  y < 0 )
		return true;

	// A tile is uncovered if it is uncovered on any shared map.
	for (const shroud_map * const shared_map : maps) {
		if ( shared_map->enabled_  &&  !shared_map->value(x,y) )
			return false;
	}
	return true;
}

std::string team::shroud_map::write() const
{
	std::stringstream shroud_str;
	for(std::vector<std::vector<bool> >::const_iterator sh = data_.begin(); sh != data_.end(); ++sh) {
		shroud_str << '|';

		for(std::vector<bool>::const_iterator i = sh->begin(); i != sh->end(); ++i) {
			shroud_str << (*i ? '1' : '0');
		}

		shroud_str << '\n';
	}

	return shroud_str.str();
}

void team::shroud_map::read(const std::string& str)
{
	data_.clear();
	for(std::string::const_iterator sh = str.begin(); sh != str.end(); ++sh) {
		if(*sh == '|')
			data_.resize(data_.size()+1);

		if(data_.empty() == false) {
			if(*sh == '1')
				data_.back().push_back(true);
			else if(*sh == '0')
				data_.back().push_back(false);
		}
	}
}

void team::shroud_map::merge(const std::string& str)
{
	int x=0, y=0;
	for(std::string::const_iterator sh = str.begin(); sh != str.end(); ++sh) {
		if(*sh == '|' && sh != str.begin()) {
			y=0;
			x++;
		} else if(*sh == '1') {
			clear(x,y);
			y++;
		} else if(*sh == '0') {
			y++;
		}
	}
}

bool team::shroud_map::copy_from(const std::vector<const shroud_map*>& maps)
{
	if(enabled_ == false)
		return false;

	bool cleared = false;
	for(std::vector<const shroud_map*>::const_iterator i = maps.begin(); i != maps.end(); ++i) {
		if((*i)->enabled_ == false)
			continue;

		const std::vector<std::vector<bool> >& v = (*i)->data_;
		for(size_t x = 0; x != v.size(); ++x) {
			for(size_t y = 0; y != v[x].size(); ++y) {
				if(v[x][y]) {
					cleared |= clear(x,y);
				}
			}
		}
	}
	return cleared;
}

const color_range team::get_side_color_range(int side)
{
	std::string index = get_side_color_index(side);
	std::map<std::string, color_range>::iterator gp=game_config::team_rgb_range.find(index);

	if(gp != game_config::team_rgb_range.end()){
		return(gp->second);
	}

	return color_range({255, 0, 0}, {255, 255, 255}, {0, 0, 0}, {255, 0, 0});
}

color_t team::get_side_color(int side)
{
	return get_side_color_range(side).mid();
}

color_t team::get_minimap_color(int side)
{
	// Note: use mid() instead of rep() unless
	// high contrast is needed over a map or minimap!
	return get_side_color_range(side).rep();
}

std::string team::get_side_color_index(int side)
{
	size_t index = size_t(side-1);

	if(resources::gameboard && index < resources::gameboard->teams().size()) {
		const std::string side_map = resources::gameboard->teams()[index].color();
		if(!side_map.empty()) {
			return side_map;
		}
	}
	return std::to_string(side);
}

std::string team::get_side_highlight_pango(int side)
{
	return get_side_color_range(side + 1).mid().to_hex_string();
}

void team::log_recruitable() const {
	LOG_NG << "Adding recruitable units: \n";
	for (std::set<std::string>::const_iterator it = info_.can_recruit.begin();
		 it != info_.can_recruit.end(); ++it) {
		LOG_NG << *it << std::endl;
	}
	LOG_NG << "Added all recruitable units\n";
}

config team::to_config() const
{
	config cfg;
	config& result = cfg.add_child("side");
	write(result);
	return result;
}

std::string team::allied_human_teams() const
{
	std::vector<int> res;
	for(const team& t : resources::gameboard->teams())
	{
		if(!t.is_enemy(this->side()) && t.is_human()) {
			res.push_back(t.side());
		}
	}
	return utils::join(res);
}
