/* $Id$ */
/*
   Copyright (C) 2003 - 2012 by David White <dave@whitevine.net>
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
 * @file
 * Recruiting, Fighting.
 */

#include "actions.hpp"

#include "attack_prediction.hpp"
#include "game_display.hpp"
#include "game_events.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "hotkeys.hpp"
#include "map_label.hpp"
#include "mouse_handler_base.hpp"
#include "replay.hpp"
#include "resources.hpp"
#include "statistics.hpp"
#include "unit_abilities.hpp"
#include "unit_display.hpp"
#include "wml_exception.hpp"
#include "formula_string_utils.hpp"
#include "tod_manager.hpp"
#include "whiteboard/manager.hpp"

#include <boost/foreach.hpp>

static lg::log_domain log_engine("engine");
#define DBG_NG LOG_STREAM(debug, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)
#define ERR_NG LOG_STREAM(err, log_engine)

static lg::log_domain log_config("config");
#define LOG_CF LOG_STREAM(info, log_config)

static lg::log_domain log_ai_testing("ai/testing");
#define LOG_AI_TESTING LOG_STREAM(info, log_ai_testing)

struct castle_cost_calculator : pathfind::cost_calculator
{
	castle_cost_calculator(const gamemap& map, const team & view_team) :
		map_(map),
		viewer_(view_team),
		use_shroud_(view_team.uses_shroud())
	{}

	virtual double cost(const map_location& loc, const double) const
	{
		if(!map_.is_castle(loc))
			return 10000;

		if ( use_shroud_ && viewer_.shrouded(loc) )
			return 10000;

		return 1;
	}

private:
	const gamemap& map_;
	const team& viewer_;
	const bool use_shroud_; // Allows faster checks when shroud is disabled.
};

void move_unit_spectator::add_seen_friend(const unit_map::const_iterator &u)
{
	seen_friends_.push_back(u);
}


void move_unit_spectator::add_seen_enemy(const unit_map::const_iterator &u)
{
	seen_enemies_.push_back(u);
}


const unit_map::const_iterator& move_unit_spectator::get_ambusher() const
{
	return ambusher_;
}


const unit_map::const_iterator& move_unit_spectator::get_failed_teleport() const
{
	return failed_teleport_;
}


const std::vector<unit_map::const_iterator>& move_unit_spectator::get_seen_enemies() const
{
	return seen_enemies_;
}


const std::vector<unit_map::const_iterator>& move_unit_spectator::get_seen_friends() const
{
	return seen_friends_;
}


const unit_map::const_iterator& move_unit_spectator::get_unit() const
{
	return unit_;
}


move_unit_spectator::move_unit_spectator(const unit_map &units)
	: ambusher_(units.end()),failed_teleport_(units.end()),seen_enemies_(),seen_friends_(),unit_(units.end())
{
}


move_unit_spectator::~move_unit_spectator()
{
}

void move_unit_spectator::reset(const unit_map &units)
{
	ambusher_ = units.end();
	failed_teleport_ = units.end();
	seen_enemies_.clear();
	seen_friends_.clear();
	unit_ = units.end();
}


void move_unit_spectator::set_ambusher(const unit_map::const_iterator &u)
{
	ambusher_ = u;
}


void move_unit_spectator::set_failed_teleport(const unit_map::const_iterator &u)
{
	failed_teleport_ = u;
}


void move_unit_spectator::set_unit(const unit_map::const_iterator &u)
{
	unit_ = u;
}


unit_creator::unit_creator(team &tm, const map_location &start_pos)
  : add_to_recall_(false),discover_(false),get_village_(false),invalidate_(false), rename_side_(false), show_(false), start_pos_(start_pos), team_(tm)
{
}


unit_creator& unit_creator::allow_show(bool b)
{
	show_=b;
	return *this;
}


unit_creator& unit_creator::allow_get_village(bool b)
{
	get_village_=b;
	return *this;
}


unit_creator& unit_creator::allow_rename_side(bool b)
{
	rename_side_=b;
	return *this;
}

unit_creator& unit_creator::allow_invalidate(bool b)
{
	invalidate_=b;
	return *this;
}


unit_creator& unit_creator::allow_discover(bool b)
{
	discover_=b;
	return *this;
}


unit_creator& unit_creator::allow_add_to_recall(bool b)
{
	add_to_recall_=b;
	return *this;
}


map_location unit_creator::find_location(const config &cfg, const unit* pass_check)
{

	DBG_NG << "finding location for unit with id=["<<cfg["id"]<<"] placement=["<<cfg["placement"]<<"] x=["<<cfg["x"]<<"] y=["<<cfg["y"]<<"] for side " << team_.side() << "\n";

	std::vector< std::string > placements = utils::split(cfg["placement"]);

	placements.push_back("map");
	placements.push_back("recall");

	BOOST_FOREACH(std::string place, placements) {
		map_location loc;
		bool pass((place == "leader_passable") || (place == "map_passable"));

		if (place == "recall" ) {
			return map_location::null_location;
		}

		if (place == "leader" || place == "leader_passable") {
			unit_map::const_iterator leader = resources::units->find_leader(team_.side());
			//todo: take 'leader in recall list' possibility into account
			if (leader.valid()) {
				loc = leader->get_location();
			} else {
				loc = start_pos_;
			}
		}

		if (place=="map" || place == "map_passable") {
			loc = map_location(cfg,resources::gamedata);
		}

		if(loc.valid() && resources::game_map->on_board(loc)) {
			if (pass) {
				loc = find_vacant_tile(loc, pathfind::VACANT_ANY, pass_check);
			} else {
				loc = find_vacant_tile(loc, pathfind::VACANT_ANY);
			}
			if(loc.valid() && resources::game_map->on_board(loc)) {
				return loc;
			}
		}
	}

	return map_location::null_location;

}


void unit_creator::add_unit(const config &cfg, const vconfig* vcfg)
{
	config temp_cfg(cfg);
	temp_cfg["side"] = team_.side();
	temp_cfg.remove_attribute("player_id");
	temp_cfg.remove_attribute("faction_from_recruit");

	const std::string& id =(cfg)["id"];
	bool animate = temp_cfg["animate"].to_bool();
	temp_cfg.remove_attribute("animate");

	std::vector<unit> &recall_list = team_.recall_list();
	std::vector<unit>::iterator recall_list_element = find_if_matches_id(recall_list, id);

	if ( recall_list_element == recall_list.end() ) {
		//make a temporary unit
		boost::scoped_ptr<unit> temp_unit(new unit(temp_cfg, true, resources::state_of_game, vcfg));
		map_location loc = find_location(temp_cfg, temp_unit.get());
		if ( loc.valid() ) {
			unit *new_unit = temp_unit.get();
			//add temporary unit to map
			assert( resources::units->find(loc) == resources::units->end() );
			resources::units->add(loc, *new_unit);
			LOG_NG << "inserting unit for side " << new_unit->side() << "\n";
			post_create(loc,*(resources::units->find(loc)),animate);
		}
		else if ( add_to_recall_ ) {
			//add to recall list
			unit *new_unit = temp_unit.get();
			recall_list.push_back(*new_unit);
			DBG_NG << "inserting unit with id=["<<id<<"] on recall list for side " << new_unit->side() << "\n";
			preferences::encountered_units().insert(new_unit->type_id());
		}
	} else {
		//get unit from recall list
		map_location loc = find_location(temp_cfg, &(*recall_list_element));
		if ( loc.valid() ) {
			resources::units->add(loc, *recall_list_element);
			LOG_NG << "inserting unit from recall list for side " << recall_list_element->side()<< " with id="<< id << "\n";
			post_create(loc,*(resources::units->find(loc)),animate);
			//if id is not empty, delete units with this ID from recall list
			erase_if_matches_id(recall_list, id);
		} else {
			LOG_NG << "wanted to insert unit on recall list, but recall list for side " << (cfg)["side"] << "already contains id=" <<id<<"\n";
			return;
		}
	}
}


void unit_creator::post_create(const map_location &loc, const unit &new_unit, bool anim)
{

	if (discover_) {
		preferences::encountered_units().insert(new_unit.type_id());
	}

	bool show = show_ && (resources::screen !=NULL) && !resources::screen->fogged(loc);
	bool animate = show && anim;

	if (get_village_) {
		if (resources::game_map->is_village(loc)) {
			get_village(loc, new_unit.side());
		}
	}

	if (resources::screen!=NULL) {

		if (invalidate_ ) {
			resources::screen->invalidate(loc);
		}

		if (animate) {
			unit_display::unit_recruited(loc);
		} else if (show) {
			resources::screen->draw();
		}
	}
}


/**
 * Checks to see if a leader at @a leader_loc could recruit on @a recruit_loc.
 * This takes into account terrain, shroud (for side @a side), and whether or
 * not there is already a visible unit at recruit_loc.
 * The behavior for an invalid @a side is subject to change for future needs.
 */
bool can_recruit_on(const map_location& leader_loc, const map_location& recruit_loc, int side)
{
	const gamemap& map = *resources::game_map;

	if( !map.is_castle(recruit_loc) )
		return false;

	if( !map.is_keep(leader_loc) )
		return false;

	if ( side < 1  ||  resources::teams == NULL  ||
	     resources::teams->size() < static_cast<size_t>(side) ) {
		// Invalid side specified.
		// Currently this cannot happen, but it could conceivably be used in
		// the future to request that shroud and visibility be ignored. Until
		// that comes to pass, just return.
 		return false;
	}
	const team & view_team = (*resources::teams)[side-1];

	if ( view_team.shrouded(recruit_loc) )
		return false;

	if ( get_visible_unit(recruit_loc, view_team) != NULL )
		return false;

	castle_cost_calculator calc(map, view_team);
	// The limit computed in the third argument is more than enough for
	// any convex castle on the map. Strictly speaking it could be
	// reduced to sqrt(map.w()**2 + map.h()**2).
	pathfind::plain_route rt =
		pathfind::a_star_search(leader_loc, recruit_loc, map.w()+map.h(), &calc,
		                        map.w(), map.h());
	return !rt.steps.empty();
}

const std::set<std::string> get_recruits_for_location(int side, const map_location &recruit_loc)
{
	LOG_NG << "getting recruit list for side " << side << " at location " << recruit_loc << "\n";

	const std::set<std::string>& recruit_list = (*resources::teams)[side -1].recruits();
	std::set<std::string> local_result;
	std::set<std::string> global_result;

	unit_map::const_iterator u = resources::units->begin(),
			u_end = resources::units->end();

	bool leader_in_place = false;
	bool recruit_loc_is_castle = resources::game_map->is_castle(recruit_loc);

	for(; u != u_end; ++u) {
		//Only consider leaders on this side.
		if (!(u->can_recruit() && u->side() == side))
			continue;

		// Check if the leader is on a connected keep.
		if ( can_recruit_on(*u, recruit_loc) ) {
			leader_in_place= true;
			local_result.insert(u->recruits().begin(), u->recruits().end());
		} else
			global_result.insert(u->recruits().begin(), u->recruits().end());
	}

	bool global = !(recruit_loc_is_castle && leader_in_place);

	if (global)
		global_result.insert(recruit_list.begin(),recruit_list.end());
	else if (leader_in_place)
		local_result.insert(recruit_list.begin(),recruit_list.end());

	return global ? global_result : local_result;
}

const std::vector<const unit*> get_recalls_for_location(int side, const map_location &recall_loc) {
	LOG_NG << "getting recall list for side " << side << " at location " << recall_loc << "\n";

	const team& t = (*resources::teams)[side-1];
	const std::vector<unit>& recall_list = t.recall_list();
	std::vector<const unit*> result;

	/*
	 * We have two use cases:
	 * 1. A castle tile is highlighted, we only present the units recallable there.
	 * 2. A non castle tile is highlighted, we present all units in the recall list.
	 */

	bool leader_in_place = false;
	bool recall_loc_is_castle = resources::game_map->is_castle(recall_loc);

	if (recall_loc_is_castle) {

		unit_map::const_iterator u = resources::units->begin(),
				u_end = resources::units->end();
		std::set<size_t> valid_local_recalls;

		for(; u != u_end; ++u) {
			//We only consider leaders on our side.
			if (!(u->can_recruit() && u->side() == side))
				continue;

			// Check if the leader is on a connected keep.
			if ( can_recruit_on(*u, recall_loc) )
				leader_in_place= true;
			else continue;

			BOOST_FOREACH(const unit& recall_unit, recall_list)
			{
				//Only units which match the leaders recall filter are valid.
				scoped_recall_unit this_unit("this_unit", t.save_id(), &recall_unit - &recall_list[0]);
				if (!(recall_unit.matches_filter(vconfig(u->recall_filter()), map_location::null_location)))
					continue;

				//Do not add a unit twice.
				if (valid_local_recalls.find(recall_unit.underlying_id())
						== valid_local_recalls.end()) {
				valid_local_recalls.insert(recall_unit.underlying_id());
				result.push_back(&recall_unit);
				}
			}
		}
	}

	if (!(recall_loc_is_castle && leader_in_place)) {
		BOOST_FOREACH(const unit &recall, recall_list)
		{
			result.push_back(&recall);
		}
	}

	return result;
}

std::string find_recall_location(const int side, map_location& recall_loc, map_location& recall_from, const unit &recall_unit)
{
	LOG_NG << "finding recall location for side " << side << " and unit " << recall_unit.id() << "\n";

	unit_map::const_iterator u = resources::units->begin(),
		u_end = resources::units->end(), leader = u_end, leader_keep = u_end, leader_fit = u_end,
			leader_able = u_end, leader_opt = u_end;

	map_location alternate_location = map_location::null_location;
	map_location alternate_from = map_location::null_location;

	for(; u != u_end; ++u) {
		//quit if it is not a leader on the @side
		if (!(u->can_recruit() && u->side() == side))
			continue;
		leader = u;

		//quit if the leader is not able to recall the @recall_unit
		const team& t = (*resources::teams)[side-1];
		scoped_recall_unit this_unit("this_unit",
			t.save_id(),
			&recall_unit - &t.recall_list()[0]);
		if (!(recall_unit.matches_filter(vconfig(leader->recall_filter()), map_location::null_location)))
			continue;
		leader_able = leader;

		//quit if the leader is not on a keep
		if (!(resources::game_map->is_keep(leader->get_location())))
			continue;
		leader_keep = leader_able;

		//find a place to recall in the leader's keep
		map_location tmp_location = pathfind::find_vacant_castle(*leader_keep);

		//quit if there is no place to recruit on
		if (tmp_location == map_location::null_location)
			continue;
		leader_fit = leader_keep;

		if ( can_recruit_on(*leader_fit, recall_loc) ) {
			leader_opt = leader_fit;
			if (resources::units->count(recall_loc) == 1)
				recall_loc = tmp_location;
				recall_from = leader_opt->get_location();
			break;
		} else {
			alternate_location = tmp_location;
			alternate_from = leader_fit->get_location();
		}
	}

	if (leader == u_end) {
		LOG_NG << "No Leader on side " << side << " when recalling " << recall_unit.id() << '\n';
		return _("You don’t have a leader to recall with.");
	}

	if (leader_able == u_end) {
		LOG_NG << "No Leader able to recall unit: " << recall_unit.id() << " on side " << side << '\n';
		return _("None of your leaders is able to recall that unit.");
	}

	if (leader_keep == u_end) {
		LOG_NG << "No Leader on a keep to recall the unit " << recall_unit.id() << " at " << recall_loc  << '\n';
		return _("You must have a leader on a keep who is able to recall that unit.");
	}

	if (leader_fit == u_end) {
		LOG_NG << "No vacant castle tiles on a keep available to recall the unit " << recall_unit.id() << " at " << recall_loc  << '\n';
		return _("There are no vacant castle tiles in which to recall the unit.");
	}

	if (leader_opt == u_end) {
		recall_loc = alternate_location;
		recall_from = alternate_from;
	}

	return std::string();
}

std::string find_recruit_location(const int side, map_location& recruit_location, map_location& recruited_from, const std::string& unit_type)
{
	LOG_NG << "finding recruit location for side " << side << "\n";

	unit_map::const_iterator u = resources::units->begin(), u_end = resources::units->end(),
			leader = u_end, leader_keep = u_end, leader_fit = u_end,
			leader_able = u_end, leader_opt = u_end;

	map_location alternate_location = map_location::null_location;
	map_location alternate_from = map_location::null_location;

	const std::set<std::string>& recruit_list = (*resources::teams)[side -1].recruits();
	std::set<std::string>::const_iterator recruit_it = recruit_list.find(unit_type);
	bool is_on_team_list = (recruit_it != recruit_list.end());

	for(; u != u_end; ++u) {
		if (!(u->can_recruit() && u->side() == side))
			continue;
		leader = u;

		bool can_recruit_unit = is_on_team_list;
		if (!can_recruit_unit) {
			BOOST_FOREACH(const std::string &recruitable, leader->recruits()) {
				if (recruitable == unit_type) {
					can_recruit_unit = true;
					break;
				}
			}
		}

		if (!can_recruit_unit)
			continue;
		leader_able = leader;

		if (!(resources::game_map->is_keep(leader_able->get_location())))
			continue;
		leader_keep = leader_able;

		map_location tmp_location = pathfind::find_vacant_castle(*leader_keep);

		if (tmp_location == map_location::null_location)
			continue;
		leader_fit = leader_keep;

		if ( can_recruit_on(*leader_fit, recruit_location) ) {
			leader_opt = leader_fit;
			if (resources::units->count(recruit_location) == 1)
				recruit_location = tmp_location;
				recruited_from = leader_opt->get_location();
			break;
		} else {
			alternate_location = tmp_location;
			alternate_from = leader_fit->get_location();
		}
	}

	if (leader == u_end) {
		LOG_NG << "No Leader on side " << side << " when recruiting " << unit_type << '\n';
		return _("You don’t have a leader to recruit with.");
	}

	if (leader_keep == u_end) {
		LOG_NG << "Leader not on start: leader is on " << leader->get_location() << '\n';
		return _("You must have a leader on a keep who is able to recruit the unit.");
	}

	if (leader_fit == u_end) {
		LOG_NG << "No vacant castle tiles on a keep available to recruit the unit " << unit_type << " at " << recruit_location  << '\n';
		return _("There are no vacant castle tiles in which to recruit the unit.");
	}

	if (leader_opt == u_end) {
		recruit_location = alternate_location;
		recruited_from = alternate_from;
	}

	return std::string();
}

void place_recruit(const unit &u, const map_location &recruit_location, const map_location& recruited_from,
    bool is_recall, bool show, bool fire_event, bool full_movement,
    bool wml_triggered)
{
	LOG_NG << "placing new unit on location " << recruit_location << "\n";

	assert(resources::units->count(recruit_location) == 0);

	unit new_unit = u;
	if (full_movement) {
		new_unit.set_movement(new_unit.total_movement());
	} else {
		new_unit.set_movement(0);
		new_unit.set_attacks(0);
	}
	new_unit.heal_all();
	new_unit.set_hidden(true);

	resources::units->add(recruit_location, new_unit);

	if (is_recall)
	{
		if (fire_event) {
			LOG_NG << "firing prerecall event\n";
			game_events::fire("prerecall",recruit_location, recruited_from);
		}
	}
	else
	{
		LOG_NG << "firing prerecruit event\n";
		game_events::fire("prerecruit",recruit_location, recruited_from);
	}
	const unit_map::iterator new_unit_itor = resources::units->find(recruit_location);
	if (new_unit_itor.valid()) {
		new_unit_itor->set_hidden(false);
		if (resources::game_map->is_village(recruit_location)) {
			get_village(recruit_location,new_unit_itor->side());
		}
		preferences::encountered_units().insert(new_unit_itor->type_id());

	}

	unit_map::iterator leader = resources::units->begin();
	for(; leader != resources::units->end(); ++leader)
		if (leader->can_recruit() &&
		    leader->side() == new_unit.side() &&
		    can_recruit_on(*leader, recruit_location))
			break;
	if (show) {
		if (leader.valid()) {
			unit_display::unit_recruited(recruit_location, leader->get_location());
		} else {
			unit_display::unit_recruited(recruit_location);
		}
	}
	if (is_recall)
	{
		if (fire_event) {
			LOG_NG << "firing recall event\n";
			game_events::fire("recall", recruit_location, recruited_from);
		}
	}
	else
	{
		LOG_NG << "firing recruit event\n";
		game_events::fire("recruit",recruit_location, recruited_from);
	}

	const std::string checksum = get_checksum(new_unit);

	const config* ran_results = get_random_results();
	if(ran_results != NULL) {
		// When recalling from WML there should be no random results, if we use
		// random we might get the replay out of sync.
		assert(!wml_triggered);
		const std::string rc = (*ran_results)["checksum"];
		if(rc != checksum) {
			std::stringstream error_msg;
			error_msg << "SYNC: In recruit " << new_unit.type_id() <<
				": has checksum " << checksum <<
				" while datasource has checksum " <<
				rc << "\n";
			ERR_NG << error_msg.str();

			config cfg_unit1;
			new_unit.write(cfg_unit1);
			DBG_NG << cfg_unit1;
			replay::process_error(error_msg.str());
		}

	} else if(wml_triggered == false) {
		config cfg;
		cfg["checksum"] = checksum;
		set_random_results(cfg);
	}

	resources::whiteboard->on_gamestate_change();
}

map_location under_leadership(const unit_map& units,
		const map_location& loc, int* bonus)
{

	const unit_map::const_iterator un = units.find(loc);
	if(un == units.end()) {
		return map_location::null_location;
	}
	unit_ability_list abil = un->get_abilities("leadership");
	if(bonus) {
		*bonus = abil.highest("value").first;
	}
	return abil.highest("value").second;
}

battle_context::battle_context(const unit_map& units,
		const map_location& attacker_loc, const map_location& defender_loc,
		int attacker_weapon, int defender_weapon, double aggression, const combatant *prev_def, const unit* attacker_ptr)
: attacker_stats_(NULL), defender_stats_(NULL), attacker_combatant_(NULL), defender_combatant_(NULL)
{
	const unit &attacker = attacker_ptr ? *attacker_ptr : *units.find(attacker_loc);
	const unit &defender = *units.find(defender_loc);
	const double harm_weight = 1.0 - aggression;

	if (attacker_weapon == -1 && attacker.attacks().size() == 1 && attacker.attacks()[0].attack_weight() > 0 )
		attacker_weapon = 0;

	if (attacker_weapon == -1) {
		attacker_weapon = choose_attacker_weapon(attacker, defender, units,
			attacker_loc, defender_loc,
				harm_weight, &defender_weapon, prev_def);
	} else if (defender_weapon == -1) {
		defender_weapon = choose_defender_weapon(attacker, defender, attacker_weapon,
			units, attacker_loc, defender_loc, prev_def);
	}

	// If those didn't have to generate statistics, do so now.
	if (!attacker_stats_) {
		const attack_type *adef = NULL;
		const attack_type *ddef = NULL;
		if (attacker_weapon >= 0) {
			VALIDATE(attacker_weapon < static_cast<int>(attacker.attacks().size()),
					_("An invalid attacker weapon got selected."));
			adef = &attacker.attacks()[attacker_weapon];
		}
		if (defender_weapon >= 0) {
			VALIDATE(defender_weapon < static_cast<int>(defender.attacks().size()),
					_("An invalid defender weapon got selected."));
			ddef = &defender.attacks()[defender_weapon];
		}
		assert(!defender_stats_ && !attacker_combatant_ && !defender_combatant_);
		attacker_stats_ = new battle_context_unit_stats(attacker, attacker_loc, attacker_weapon,
				true, defender, defender_loc, ddef, units);
		defender_stats_ = new battle_context_unit_stats(defender, defender_loc, defender_weapon, false,
				attacker, attacker_loc, adef, units);
	}
}

	battle_context::battle_context(const battle_context &other)
: attacker_stats_(NULL), defender_stats_(NULL), attacker_combatant_(NULL), defender_combatant_(NULL)
{
	*this = other;
}

battle_context::battle_context(const battle_context_unit_stats &att, const battle_context_unit_stats &def) :
	attacker_stats_(new battle_context_unit_stats(att)),
	defender_stats_(new battle_context_unit_stats(def)),
	attacker_combatant_(0),
	defender_combatant_(0)
{
}

battle_context::~battle_context()
{
	delete attacker_stats_;
	delete defender_stats_;
	delete attacker_combatant_;
	delete defender_combatant_;
}
battle_context& battle_context::operator=(const battle_context &other)
{
	if (&other != this) {
		delete attacker_stats_;
		delete defender_stats_;
		delete attacker_combatant_;
		delete defender_combatant_;
		attacker_stats_ = new battle_context_unit_stats(*other.attacker_stats_);
		defender_stats_ = new battle_context_unit_stats(*other.defender_stats_);
		attacker_combatant_ = other.attacker_combatant_ ? new combatant(*other.attacker_combatant_, *attacker_stats_) : NULL;
		defender_combatant_ = other.defender_combatant_ ? new combatant(*other.defender_combatant_, *defender_stats_) : NULL;
	}
	return *this;
}

/** @todo FIXME: Hand previous defender unit in here. */
int battle_context::choose_defender_weapon(const unit &attacker, const unit &defender, unsigned attacker_weapon,
	const unit_map& units,
		const map_location& attacker_loc, const map_location& defender_loc,
		const combatant *prev_def)
{
	VALIDATE(attacker_weapon < attacker.attacks().size(),
			_("An invalid attacker weapon got selected."));
	const attack_type &att = attacker.attacks()[attacker_weapon];
	std::vector<unsigned int> choices;

	// What options does defender have?
	unsigned int i;
	for (i = 0; i < defender.attacks().size(); ++i) {
		const attack_type &def = defender.attacks()[i];
		if (def.range() == att.range() && def.defense_weight() > 0) {
			choices.push_back(i);
		}
	}
	if (choices.empty())
		return -1;
	if (choices.size() == 1)
		return choices[0];

	// Multiple options:
	// First pass : get the best weight and the minimum simple rating for this weight.
	// simple rating = number of blows * damage per blows (resistance taken in account) * cth * weight
	// Elligible attacks for defense should have a simple rating greater or equal to this weight.

	double max_weight = 0.0;
	int min_rating = 0;

	for (i = 0; i < choices.size(); ++i) {
		const attack_type &def = defender.attacks()[choices[i]];
		if (def.defense_weight() >= max_weight) {
			max_weight = def.defense_weight();
			const battle_context_unit_stats def_stats(defender, defender_loc,
					choices[i], false, attacker, attacker_loc, &att, units);
			int rating = static_cast<int>(def_stats.num_blows * def_stats.damage *
					def_stats.chance_to_hit * def.defense_weight());
			if (def.defense_weight() > max_weight || rating < min_rating ) {
				min_rating = rating;
			}
		}
	}

	// Multiple options: simulate them, save best.
	for (i = 0; i < choices.size(); ++i) {
		const attack_type &def = defender.attacks()[choices[i]];
		battle_context_unit_stats *att_stats = new battle_context_unit_stats(attacker, attacker_loc, attacker_weapon,
				true, defender, defender_loc, &def, units);
		battle_context_unit_stats *def_stats = new battle_context_unit_stats(defender, defender_loc, choices[i], false,
				attacker, attacker_loc, &att, units);

		combatant *att_comb = new combatant(*att_stats);
		combatant *def_comb = new combatant(*def_stats, prev_def);
		att_comb->fight(*def_comb);

		int simple_rating = static_cast<int>(def_stats->num_blows *
				def_stats->damage * def_stats->chance_to_hit * def.defense_weight());

		if (simple_rating >= min_rating &&
				( !attacker_combatant_ || better_combat(*def_comb, *att_comb, *defender_combatant_, *attacker_combatant_, 1.0) )
		   ) {
			delete attacker_combatant_;
			delete defender_combatant_;
			delete attacker_stats_;
			delete defender_stats_;
			attacker_combatant_ = att_comb;
			defender_combatant_ = def_comb;
			attacker_stats_ = att_stats;
			defender_stats_ = def_stats;
		} else {
			delete att_comb;
			delete def_comb;
			delete att_stats;
			delete def_stats;
		}
	}

	return defender_stats_->attack_num;
}

int battle_context::choose_attacker_weapon(const unit &attacker, const unit &defender,
	const unit_map& units,
		const map_location& attacker_loc, const map_location& defender_loc,
		double harm_weight, int *defender_weapon, const combatant *prev_def)
{
	std::vector<unsigned int> choices;

	// What options does attacker have?
	unsigned int i;
	for (i = 0; i < attacker.attacks().size(); ++i) {
		const attack_type &att = attacker.attacks()[i];
		if (att.attack_weight() > 0) {
			choices.push_back(i);
		}
	}
	if (choices.empty())
		return -1;
	if (choices.size() == 1) {
		*defender_weapon = choose_defender_weapon(attacker, defender, choices[0], units,
			attacker_loc, defender_loc, prev_def);
		return choices[0];
	}

	// Multiple options: simulate them, save best.
	battle_context_unit_stats *best_att_stats = NULL, *best_def_stats = NULL;
	combatant *best_att_comb = NULL, *best_def_comb = NULL;

	for (i = 0; i < choices.size(); ++i) {
		const attack_type &att = attacker.attacks()[choices[i]];
		int def_weapon = choose_defender_weapon(attacker, defender, choices[i], units,
			attacker_loc, defender_loc, prev_def);
		// If that didn't simulate, do so now.
		if (!attacker_combatant_) {
			const attack_type *def = NULL;
			if (def_weapon >= 0) {
				def = &defender.attacks()[def_weapon];
			}
			attacker_stats_ = new battle_context_unit_stats(attacker, attacker_loc, choices[i],
				true, defender, defender_loc, def, units);
			defender_stats_ = new battle_context_unit_stats(defender, defender_loc, def_weapon, false,
				attacker, attacker_loc, &att, units);
			attacker_combatant_ = new combatant(*attacker_stats_);
			defender_combatant_ = new combatant(*defender_stats_, prev_def);
			attacker_combatant_->fight(*defender_combatant_);
		}
		if (!best_att_comb || better_combat(*attacker_combatant_, *defender_combatant_,
					*best_att_comb, *best_def_comb, harm_weight)) {
			delete best_att_comb;
			delete best_def_comb;
			delete best_att_stats;
			delete best_def_stats;
			best_att_comb = attacker_combatant_;
			best_def_comb = defender_combatant_;
			best_att_stats = attacker_stats_;
			best_def_stats = defender_stats_;
		} else {
			delete attacker_combatant_;
			delete defender_combatant_;
			delete attacker_stats_;
			delete defender_stats_;
		}
		attacker_combatant_ = NULL;
		defender_combatant_ = NULL;
		attacker_stats_ = NULL;
		defender_stats_ = NULL;
	}

	attacker_combatant_ = best_att_comb;
	defender_combatant_ = best_def_comb;
	attacker_stats_ = best_att_stats;
	defender_stats_ = best_def_stats;

	*defender_weapon = defender_stats_->attack_num;
	return attacker_stats_->attack_num;
}

// Does combat A give us a better result than combat B?
bool battle_context::better_combat(const combatant &us_a, const combatant &them_a,
		const combatant &us_b, const combatant &them_b,
		double harm_weight)
{
	double a, b;

	// Compare: P(we kill them) - P(they kill us).
	a = them_a.hp_dist[0] - us_a.hp_dist[0] * harm_weight;
	b = them_b.hp_dist[0] - us_b.hp_dist[0] * harm_weight;
	if (a - b < -0.01)
		return false;
	if (a - b > 0.01)
		return true;

	// Add poison to calculations
	double poison_a_us = (us_a.poisoned) * game_config::poison_amount;
	double poison_a_them = (them_a.poisoned) * game_config::poison_amount;
	double poison_b_us = (us_b.poisoned) * game_config::poison_amount;
	double poison_b_them = (them_b.poisoned) * game_config::poison_amount;
	// Compare: damage to them - damage to us (average_hp replaces -damage)
	a = (us_a.average_hp()-poison_a_us)*harm_weight - (them_a.average_hp()-poison_a_them);
	b = (us_b.average_hp()-poison_b_us)*harm_weight - (them_b.average_hp()-poison_b_them);
	if (a - b < -0.01)
		return false;
	if (a - b > 0.01)
		return true;

	// All else equal: go for most damage.
	return them_a.average_hp() < them_b.average_hp();
}

/** @todo FIXME: better to initialize combatant initially (move into battle_context_unit_stats?), just do fight() when required. */
const combatant &battle_context::get_attacker_combatant(const combatant *prev_def)
{
	// We calculate this lazily, since AI doesn't always need it.
	if (!attacker_combatant_) {
		assert(!defender_combatant_);
		attacker_combatant_ = new combatant(*attacker_stats_);
		defender_combatant_ = new combatant(*defender_stats_, prev_def);
		attacker_combatant_->fight(*defender_combatant_);
	}
	return *attacker_combatant_;
}

const combatant &battle_context::get_defender_combatant(const combatant *prev_def)
{
	// We calculate this lazily, since AI doesn't always need it.
	if (!defender_combatant_) {
		assert(!attacker_combatant_);
		attacker_combatant_ = new combatant(*attacker_stats_);
		defender_combatant_ = new combatant(*defender_stats_, prev_def);
		attacker_combatant_->fight(*defender_combatant_);
	}
	return *defender_combatant_;
}

battle_context_unit_stats::battle_context_unit_stats(const unit &u, const map_location& u_loc,
		int u_attack_num, bool attacking,
		const unit &opp, const map_location& opp_loc,
		const attack_type *opp_weapon,
		const unit_map& units) :
	weapon(0),
	attack_num(u_attack_num),
	is_attacker(attacking),
	is_poisoned(u.get_state(unit::STATE_POISONED)),
	is_slowed(u.get_state(unit::STATE_SLOWED)),
	slows(false),
	drains(false),
	petrifies(false),
	plagues(false),
	poisons(false),
	backstab_pos(false),
	swarm(false),
	firststrike(false),
	experience(u.experience()),
	max_experience(u.max_experience()),
	level(u.level()),
	rounds(1),
	hp(0),
	max_hp(u.max_hitpoints()),
	chance_to_hit(0),
	damage(0),
	slow_damage(0),
	drain_percent(0),
	drain_constant(0),
	num_blows(0),
	swarm_min(0),
	swarm_max(0),
	plague_type()
{
	// Get the current state of the unit.
	if (attack_num >= 0) {
		weapon = &u.attacks()[attack_num];
	}
	if(u.hitpoints() < 0) {
		LOG_CF << "Unit with " << u.hitpoints() << " hitpoints found, set to 0 for damage calculations\n";
		hp = 0;
	} else if(u.hitpoints() > u.max_hitpoints()) {
		// If a unit has more hp as it's maximum the engine will fail
		// with an assertion failure due to accessing the prob_matrix
		// out of bounds.
		hp = u.max_hitpoints();
	} else {
		hp = u.hitpoints();
	}
	const map_location* aloc = &u_loc;
	const map_location* dloc = &opp_loc;

	if (!attacking)
	{
		aloc = &opp_loc;
		dloc = &u_loc;
	}

	// Get the weapon characteristics, if any.
	if (weapon) {
		weapon->set_specials_context(*aloc, *dloc, units, attacking, opp_weapon);
		if (opp_weapon)
			opp_weapon->set_specials_context(*aloc, *dloc, units, !attacking, weapon);
		bool not_living = opp.get_state("not_living");
		slows = weapon->get_special_bool("slow");
		drains = !not_living && weapon->get_special_bool("drains");
		petrifies = weapon->get_special_bool("petrifies");
		poisons = !not_living && weapon->get_special_bool("poison") && !opp.get_state(unit::STATE_POISONED);
		backstab_pos = is_attacker && backstab_check(u_loc, opp_loc, units, *resources::teams);
		rounds = weapon->get_specials("berserk").highest("value", 1).first;
		firststrike = weapon->get_special_bool("firststrike");

		// Handle plague.
		unit_ability_list plague_specials = weapon->get_specials("plague");
		plagues = !not_living && !plague_specials.empty() &&
			strcmp(opp.undead_variation().c_str(), "null") && !resources::game_map->is_village(opp_loc);

		if (plagues) {
			plague_type = (*plague_specials.cfgs.front().first)["type"].str();
			if (plague_type.empty())
				plague_type = u.type_id();
		}

		// Compute chance to hit.
		chance_to_hit = opp.defense_modifier(resources::game_map->get_terrain(opp_loc)) + weapon->accuracy() - (opp_weapon ? opp_weapon->parry() : 0);
		if(chance_to_hit > 100) {
			chance_to_hit = 100;
		}

		unit_ability_list cth_specials = weapon->get_specials("chance_to_hit");
		unit_abilities::effect cth_effects(cth_specials, chance_to_hit, backstab_pos);
		chance_to_hit = cth_effects.get_composite_value();

		// Compute base damage done with the weapon.
		int base_damage = weapon->damage();
		unit_ability_list dmg_specials = weapon->get_specials("damage");
		unit_abilities::effect dmg_effect(dmg_specials, base_damage, backstab_pos);
		base_damage = dmg_effect.get_composite_value();

		// Get the damage multiplier applied to the base damage of the weapon.
		int damage_multiplier = 100;

		// Time of day bonus.
		damage_multiplier += combat_modifier(u_loc, u.alignment(), u.is_fearless());

		// Leadership bonus.
		int leader_bonus = 0;
		if (under_leadership(units, u_loc, &leader_bonus).valid())
			damage_multiplier += leader_bonus;

		// Resistance modifier.
		damage_multiplier *= opp.damage_from(*weapon, !attacking, opp_loc);

		// Compute both the normal and slowed damage.
		damage = round_damage(base_damage, damage_multiplier, 10000);
		slow_damage = round_damage(base_damage, damage_multiplier, 20000);
		if (is_slowed)
			damage = slow_damage;

		// Compute drain amounts only if draining is possible.
		if(drains) {
			unit_ability_list drain_specials = weapon->get_specials("drains");

			// Compute the drain percent (with 50% as the base for backward compatibility)
			unit_abilities::effect drain_percent_effects(drain_specials, 50, backstab_pos);
			drain_percent = drain_percent_effects.get_composite_value();
		}

		// Add heal_on_hit (the drain constant)
		unit_ability_list heal_on_hit_specials = weapon->get_specials("heal_on_hit");
		unit_abilities::effect heal_on_hit_effects(heal_on_hit_specials, 0, backstab_pos);
		drain_constant += heal_on_hit_effects.get_composite_value();

		drains = drain_constant || drain_percent;

		// Compute the number of blows and handle swarm.
		unit_ability_list swarm_specials = weapon->get_specials("swarm");

		if (!swarm_specials.empty()) {
			swarm = true;
			swarm_min = swarm_specials.highest("swarm_attacks_min").first;
			swarm_max = swarm_specials.highest("swarm_attacks_max", weapon->num_attacks()).first;
			num_blows = swarm_min + (swarm_max - swarm_min) * hp / max_hp;
		} else {
			swarm = false;
			num_blows = weapon->num_attacks();
			unit_ability_list attacks_specials = weapon->get_specials("attacks");
			unit_abilities::effect attacks_effect(attacks_specials,num_blows,backstab_pos);
			const int num_blows_with_specials = attacks_effect.get_composite_value();
			if(num_blows_with_specials >= 0) {
				num_blows = num_blows_with_specials;
			}
			else { ERR_NG << "negative number of strikes after applying weapon specials\n"; }
			swarm_min = num_blows;
			swarm_max = num_blows;
		}
	}
}

battle_context_unit_stats::~battle_context_unit_stats()
{
}

void battle_context_unit_stats::dump() const
{
	printf("==================================\n");
	printf("is_attacker:	%d\n", static_cast<int>(is_attacker));
	printf("is_poisoned:	%d\n", static_cast<int>(is_poisoned));
	printf("is_slowed:	%d\n", static_cast<int>(is_slowed));
	printf("slows:		%d\n", static_cast<int>(slows));
	printf("drains:		%d\n", static_cast<int>(drains));
	printf("petrifies:	%d\n", static_cast<int>(petrifies));
	printf("poisons:	%d\n", static_cast<int>(poisons));
	printf("backstab_pos:	%d\n", static_cast<int>(backstab_pos));
	printf("swarm:		%d\n", static_cast<int>(swarm));
	printf("rounds:	%d\n", static_cast<int>(rounds));
	printf("firststrike:	%d\n", static_cast<int>(firststrike));
	printf("\n");
	printf("hp:		%d\n", hp);
	printf("max_hp:		%d\n", max_hp);
	printf("chance_to_hit:	%d\n", chance_to_hit);
	printf("damage:		%d\n", damage);
	printf("slow_damage:	%d\n", slow_damage);
	printf("drain_percent:	%d\n", drain_percent);
	printf("drain_constant:	%d\n", drain_constant);
	printf("num_blows:	%d\n", num_blows);
	printf("swarm_min:	%d\n", swarm_min);
	printf("swarm_max:	%d\n", swarm_max);
	printf("\n");
}

// Given this harm_weight, are we better than this other context?
bool battle_context::better_attack(class battle_context &that, double harm_weight)
{
	return better_combat(get_attacker_combatant(), get_defender_combatant(),
			that.get_attacker_combatant(), that.get_defender_combatant(), harm_weight);
}

/** Helper class for performing an attack. */
class attack
{
	friend void attack_unit(const map_location &, const map_location &,
		int, int, bool);

	attack(const map_location &attacker, const map_location &defender,
		int attack_with, int defend_with, bool update_display = true);
	void perform();
	bool perform_hit(bool, statistics::attack_context &);
	~attack();

	class attack_end_exception {};
	void fire_event(const std::string& n);
	void refresh_bc();

	/** Structure holding unit info used in the attack action. */
	struct unit_info
	{
		const map_location loc_;
		int weapon_;
		unit_map &units_;
		size_t id_; /**< unit.underlying_id() */
		std::string weap_id_;
		int orig_attacks_;
		int n_attacks_; /**< Number of attacks left. */
		int cth_;
		int damage_;
		int xp_;

		unit_info(const map_location &loc, int weapon, unit_map &units);
		unit &get_unit();
		bool valid();

		std::string dump();
	};

	void unit_killed(unit_info &, unit_info &,
		const battle_context_unit_stats *&, const battle_context_unit_stats *&,
		bool);

	battle_context *bc_;
	const battle_context_unit_stats *a_stats_;
	const battle_context_unit_stats *d_stats_;

	int abs_n_attack_, abs_n_defend_;
	// update_att_fog_ is not used, other than making some code simpler.
	bool update_att_fog_, update_def_fog_, update_minimap_;

	unit_info a_, d_;
	unit_map &units_;
	std::ostringstream errbuf_;

	bool update_display_;
	bool OOS_error_;
};

void attack::fire_event(const std::string& n)
{
	LOG_NG << "firing " << n << " event\n";
	//prepare the event data for weapon filtering
	config ev_data;
	config& a_weapon_cfg = ev_data.add_child("first");
	config& d_weapon_cfg = ev_data.add_child("second");
	if(a_stats_->weapon != NULL && a_.valid()) {
		a_weapon_cfg = a_stats_->weapon->get_cfg();
	}
	if(d_stats_->weapon != NULL && d_.valid()) {
		d_weapon_cfg = d_stats_->weapon->get_cfg();
	}
	if(a_weapon_cfg["name"].empty()) {
		a_weapon_cfg["name"] = "none";
	}
	if(d_weapon_cfg["name"].empty()) {
		d_weapon_cfg["name"] = "none";
	}
	if(n == "attack_end") {
		// We want to fire attack_end event in any case! Even if one of units was removed by WML
		game_events::fire(n, a_.loc_, d_.loc_, ev_data);
		return;
	}
	const int defender_side = d_.get_unit().side();
	game_events::fire(n, game_events::entity_location(a_.loc_, a_.id_),
		game_events::entity_location(d_.loc_, d_.id_), ev_data);

	// The event could have killed either the attacker or
	// defender, so we have to make sure they still exist
	refresh_bc();
	if(!a_.valid() || !d_.valid() || !(*resources::teams)[a_.get_unit().side() - 1].is_enemy(d_.get_unit().side())) {
		if (update_display_){
			recalculate_fog(defender_side);
			resources::screen->redraw_minimap();
			resources::screen->draw(true, true);
		}
		fire_event("attack_end");
		throw attack_end_exception();
	}
}

namespace {
	void refresh_weapon_index(int& weap_index, std::string const& weap_id, std::vector<attack_type> const& attacks) {
		if(attacks.empty()) {
			//no attacks to choose from
			weap_index = -1;
			return;
		}
		if(weap_index >= 0 && weap_index < static_cast<int>(attacks.size()) && attacks[weap_index].id() == weap_id) {
			//the currently selected attack fits
			return;
		}
		if(!weap_id.empty()) {
			//lookup the weapon by id
			for(int i=0; i<static_cast<int>(attacks.size()); ++i) {
				if(attacks[i].id() == weap_id) {
					weap_index = i;
					return;
				}
			}
		}
		//lookup has failed
		weap_index = -1;
		return;
	}
} //end anonymous namespace

void attack::refresh_bc()
{
	// Fix index of weapons
	if (a_.valid()) {
		refresh_weapon_index(a_.weapon_, a_.weap_id_, a_.get_unit().attacks());
	}
	if (d_.valid()) {
		refresh_weapon_index(d_.weapon_, d_.weap_id_, d_.get_unit().attacks());
	}
	if(!a_.valid() || !d_.valid()) {
		// Fix pointer to weapons
		const_cast<battle_context_unit_stats*>(a_stats_)->weapon =
			a_.valid() && a_.weapon_ >= 0
				? &a_.get_unit().attacks()[a_.weapon_] : NULL;

		const_cast<battle_context_unit_stats*>(d_stats_)->weapon =
			d_.valid() && d_.weapon_ >= 0
				? &d_.get_unit().attacks()[d_.weapon_] : NULL;

		return;
	}

	*bc_ =	battle_context(units_, a_.loc_, d_.loc_, a_.weapon_, d_.weapon_);
	a_stats_ = &bc_->get_attacker_stats();
	d_stats_ = &bc_->get_defender_stats();
	a_.cth_ = a_stats_->chance_to_hit;
	d_.cth_ = d_stats_->chance_to_hit;
	a_.damage_ = a_stats_->damage;
	d_.damage_ = d_stats_->damage;
}

attack::~attack()
{
	delete bc_;
}

attack::unit_info::unit_info(const map_location& loc, int weapon, unit_map& units) :
	loc_(loc),
	weapon_(weapon),
	units_(units),
	id_(),
	weap_id_(),
	orig_attacks_(0),
	n_attacks_(0),
	cth_(0),
	damage_(0),
	xp_(0)
{
	unit_map::iterator i = units_.find(loc_);
	if (!i.valid()) return;
	id_ = i->underlying_id();
}

unit &attack::unit_info::get_unit()
{
	unit_map::iterator i = units_.find(loc_);
	assert(i.valid() && i->underlying_id() == id_);
	return *i;
}

bool attack::unit_info::valid()
{
	unit_map::iterator i = units_.find(loc_);
	return i.valid() && i->underlying_id() == id_;
}

std::string attack::unit_info::dump()
{
	std::stringstream s;
	s << get_unit().type_id() << " (" << loc_.x + 1 << ',' << loc_.y + 1 << ')';
	return s.str();
}

void attack_unit(const map_location &attacker, const map_location &defender,
	int attack_with, int defend_with, bool update_display)
{
	attack dummy(attacker, defender, attack_with, defend_with, update_display);
	dummy.perform();
}

attack::attack(const map_location &attacker, const map_location &defender,
		int attack_with,
		int defend_with,
		bool update_display) :
	bc_(0),
	a_stats_(0),
	d_stats_(0),
	abs_n_attack_(0),
	abs_n_defend_(0),
	update_att_fog_(false),
	update_def_fog_(false),
	update_minimap_(false),
	a_(attacker, attack_with, *resources::units),
	d_(defender, defend_with, *resources::units),
	units_(*resources::units),
	errbuf_(),
	update_display_(update_display),
	OOS_error_(false)
{
}

bool attack::perform_hit(bool attacker_turn, statistics::attack_context &stats)
{
	unit_info
		&attacker = *(attacker_turn ? &a_ : &d_),
		&defender = *(attacker_turn ? &d_ : &a_);
	const battle_context_unit_stats
		*&attacker_stats = *(attacker_turn ? &a_stats_ : &d_stats_),
		*&defender_stats = *(attacker_turn ? &d_stats_ : &a_stats_);
	int &abs_n = *(attacker_turn ? &abs_n_attack_ : &abs_n_defend_);
	bool &update_fog = *(attacker_turn ? &update_def_fog_ : &update_att_fog_);

	int ran_num = get_random();
	bool hits = (ran_num % 100) < attacker.cth_;

	int damage = 0;
	if (hits) {
		damage = attacker.damage_;
		resources::gamedata->get_variable("damage_inflicted") = damage;
	}

	// Make sure that if we're serializing a game here,
	// we got the same results as the game did originally.
	const config *ran_results = get_random_results();
	if (ran_results)
	{
		int results_chance = (*ran_results)["chance"];
		bool results_hits = (*ran_results)["hits"].to_bool();
		int results_damage = (*ran_results)["damage"];

		if (results_chance != attacker.cth_)
		{
			errbuf_ << "SYNC: In attack " << a_.dump() << " vs " << d_.dump()
				<< ": chance to hit is inconsistent. Data source: "
				<< results_chance << "; Calculation: " << attacker.cth_
				<< " (over-riding game calculations with data source results)\n";
			attacker.cth_ = results_chance;
			OOS_error_ = true;
		}

		if (results_hits != hits)
		{
			errbuf_ << "SYNC: In attack " << a_.dump() << " vs " << d_.dump()
				<< ": the data source says the hit was "
				<< (results_hits ? "successful" : "unsuccessful")
				<< ", while in-game calculations say the hit was "
				<< (hits ? "successful" : "unsuccessful")
				<< " random number: " << ran_num << " = "
				<< (ran_num % 100) << "/" << results_chance
				<< " (over-riding game calculations with data source results)\n";
			hits = results_hits;
			OOS_error_ = true;
		}

		if (results_damage != damage)
		{
			errbuf_ << "SYNC: In attack " << a_.dump() << " vs " << d_.dump()
				<< ": the data source says the hit did " << results_damage
				<< " damage, while in-game calculations show the hit doing "
				<< damage
				<< " damage (over-riding game calculations with data source results)\n";
			damage = results_damage;
			OOS_error_ = true;
		}
	}

	int damage_done = std::min<int>(defender.get_unit().hitpoints(), attacker.damage_);

	int drains_damage = 0;
	if (hits && attacker_stats->drains) {
		drains_damage = damage_done * attacker_stats->drain_percent / 100 + attacker_stats->drain_constant;
		// don't drain so much that the attacker gets more than his maximum hitpoints
		drains_damage = std::min<int>(drains_damage, attacker.get_unit().max_hitpoints() - attacker.get_unit().hitpoints());
		// if drain is negative, don't allow drain to kill the attacker
		drains_damage = std::max<int>(drains_damage, 1 - attacker.get_unit().hitpoints());
	}

	if (update_display_)
	{
		std::ostringstream float_text;
		if (hits)
		{
			const unit &defender_unit = defender.get_unit();
			if (attacker_stats->poisons && !defender_unit.get_state(unit::STATE_POISONED)) {
				float_text << (defender_unit.gender() == unit_race::FEMALE ?
					_("female^poisoned") : _("poisoned")) << '\n';
			}

			if (attacker_stats->slows && !defender_unit.get_state(unit::STATE_SLOWED)) {
				float_text << (defender_unit.gender() == unit_race::FEMALE ?
					_("female^slowed") : _("slowed")) << '\n';
			}

			if (attacker_stats->petrifies) {
				float_text << (defender_unit.gender() == unit_race::FEMALE ?
					_("female^petrified") : _("petrified")) << '\n';
			}
		}

		unit_display::unit_attack(attacker.loc_, defender.loc_, damage,
			*attacker_stats->weapon, defender_stats->weapon,
			abs_n, float_text.str(), drains_damage, "");
	}

	bool dies = defender.get_unit().take_hit(damage);
	LOG_NG << "defender took " << damage << (dies ? " and died\n" : "\n");
	if (attacker_turn) {
		stats.attack_result(hits
			? (dies ? statistics::attack_context::KILLS : statistics::attack_context::HITS)
			: statistics::attack_context::MISSES, damage_done, drains_damage);
	} else {
		stats.defend_result(hits
			? (dies ? statistics::attack_context::KILLS : statistics::attack_context::HITS)
			: statistics::attack_context::MISSES, damage_done, drains_damage);
	}

	if (!ran_results)
	{
		log_scope2(log_engine, "setting random results");
		config cfg;
		cfg["hits"] = hits;
		cfg["dies"] = dies;
		cfg["unit_hit"] = "defender";
		cfg["damage"] = damage;
		cfg["chance"] = attacker.cth_;

		set_random_results(cfg);
	}
	else
	{
		bool results_dies = (*ran_results)["dies"].to_bool();
		if (results_dies != dies)
		{
			errbuf_ << "SYNC: In attack " << a_.dump() << " vs " << d_.dump()
				<< ": the data source says the "
				<< (attacker_turn ? "defender" : "attacker") << ' '
				<< (results_dies ? "perished" : "survived")
				<< " while in-game calculations show it "
				<< (dies ? "perished" : "survived")
				<< " (over-riding game calculations with data source results)\n";
			dies = results_dies;
			// Set hitpoints to 0 so later checks don't invalidate the death.
			// Maybe set to > 0 for the else case to avoid more errors?
			if (results_dies) defender.get_unit().set_hitpoints(0);
			OOS_error_ = true;
		}
	}

	if (hits)
	{
		try {
			fire_event(attacker_turn ? "attacker_hits" : "defender_hits");
		} catch (attack_end_exception) {
			refresh_bc();
			return false;
		}
	}
	else
	{
		try {
			fire_event(attacker_turn ? "attacker_misses" : "defender_misses");
		} catch (attack_end_exception) {
			refresh_bc();
			return false;
		}
	}
	refresh_bc();

	bool attacker_dies = false;
	if (drains_damage > 0) {
		attacker.get_unit().heal(drains_damage);
	} else if(drains_damage < 0) {
		attacker_dies = attacker.get_unit().take_hit(-drains_damage);
	}

	if (dies) {
		unit_killed(attacker, defender, attacker_stats, defender_stats, false);
		update_fog = true;
	}
	if (attacker_dies) {
		unit_killed(defender, attacker, defender_stats, attacker_stats, true);
		*(attacker_turn ? &update_att_fog_ : &update_def_fog_) = true;
	}

	if(dies) {
		update_minimap_ = true;
		return false;
	}

	if (hits)
	{
		unit &defender_unit = defender.get_unit();
		if (attacker_stats->poisons && !defender_unit.get_state(unit::STATE_POISONED)) {
			defender_unit.set_state(unit::STATE_POISONED, true);
			LOG_NG << "defender poisoned\n";
		}

		if (attacker_stats->slows && !defender_unit.get_state(unit::STATE_SLOWED)) {
			defender_unit.set_state(unit::STATE_SLOWED, true);
			update_fog = true;
			defender.damage_ = defender_stats->slow_damage;
			LOG_NG << "defender slowed\n";
		}

		// If the defender is petrified, the fight stops immediately
		if (attacker_stats->petrifies) {
			defender_unit.set_state(unit::STATE_PETRIFIED, true);
			update_fog = true;
			attacker.n_attacks_ = 0;
			defender.n_attacks_ = -1; // Petrified.
			game_events::fire("petrified", defender.loc_, attacker.loc_);
			refresh_bc();
		}
	}

	// Delay until here so that poison and slow go through
	if(attacker_dies) {
		update_minimap_ = true;
		return false;
	}

	--attacker.n_attacks_;
	return true;
}

void attack::unit_killed(unit_info& attacker, unit_info& defender,
	const battle_context_unit_stats *&attacker_stats, const battle_context_unit_stats *&defender_stats,
	bool drain_killed)
{
	attacker.xp_ = game_config::kill_xp(defender.get_unit().level());
	defender.xp_ = 0;
	resources::screen->invalidate(attacker.loc_);

	game_events::entity_location death_loc(defender.loc_, defender.id_);
	game_events::entity_location attacker_loc(attacker.loc_, attacker.id_);
	std::string undead_variation = defender.get_unit().undead_variation();
	fire_event("attack_end");
	refresh_bc();

	// get weapon info for last_breath and die events
	config dat;
	config a_weapon_cfg = attacker_stats->weapon && attacker.valid() ?
		attacker_stats->weapon->get_cfg() : config();
	config d_weapon_cfg = defender_stats->weapon && defender.valid() ?
		defender_stats->weapon->get_cfg() : config();
	if (a_weapon_cfg["name"].empty())
		a_weapon_cfg["name"] = "none";
	if (d_weapon_cfg["name"].empty())
		d_weapon_cfg["name"] = "none";
	dat.add_child("first",  d_weapon_cfg);
	dat.add_child("second", a_weapon_cfg);

	game_events::fire("last breath", death_loc, attacker_loc, dat);
	refresh_bc();

	if (!defender.valid() || defender.get_unit().hitpoints() > 0) {
		// WML has invalidated the dying unit, abort
		return;
	}

	if (!attacker.valid()) {
		unit_display::unit_die(defender.loc_, defender.get_unit(),
			NULL, defender_stats->weapon);
	} else {
		unit_display::unit_die(defender.loc_, defender.get_unit(),
			attacker_stats->weapon, defender_stats->weapon,
			attacker.loc_, &attacker.get_unit());
	}

	game_events::fire("die", death_loc, attacker_loc, dat);
	refresh_bc();

	if (!defender.valid() || defender.get_unit().hitpoints() > 0) {
		// WML has invalidated the dying unit, abort
		return;
	}

	units_.erase(defender.loc_);

	if (attacker.valid() && attacker_stats->plagues && !drain_killed)
	{
		// plague units make new units on the target hex
		LOG_NG << "trying to reanimate " << attacker_stats->plague_type << '\n';
		const unit_type *reanimator =
			unit_types.find(attacker_stats->plague_type);
		if (reanimator)
		{
			LOG_NG << "found unit type:" << reanimator->id() << '\n';
			unit newunit(reanimator, attacker.get_unit().side(),
				true, unit_race::MALE);
			newunit.set_attacks(0);
			newunit.set_movement(0);
			// Apply variation
			if (undead_variation != "null")
			{
				config mod;
				config &variation = mod.add_child("effect");
				variation["apply_to"] = "variation";
				variation["name"] = undead_variation;
				newunit.add_modification("variation",mod);
				newunit.heal_all();
			}
			units_.add(death_loc, newunit);
			preferences::encountered_units().insert(newunit.type_id());
			if (update_display_) {
				resources::screen->invalidate(death_loc);
			}
		}
	}
	else
	{
		LOG_NG << "unit not reanimated\n";
	}
}

void attack::perform()
{
	// Stop the user from issuing any commands while the units are fighting
	const events::command_disabler disable_commands;

	if(!a_.valid() || !d_.valid()) {
		return;
	}

	// no attack weapon => stop here and don't attack
	if (a_.weapon_ < 0) {
		a_.get_unit().set_attacks(a_.get_unit().attacks_left()-1);
		a_.get_unit().set_movement(-1);
		return;
	}

	a_.get_unit().set_attacks(a_.get_unit().attacks_left()-1);
	VALIDATE(a_.weapon_ < static_cast<int>(a_.get_unit().attacks().size()),
		_("An invalid attacker weapon got selected."));
	a_.get_unit().set_movement(a_.get_unit().movement_left() -
		a_.get_unit().attacks()[a_.weapon_].movement_used());
	a_.get_unit().set_state(unit::STATE_NOT_MOVED,false);
	a_.get_unit().set_resting(false);
	d_.get_unit().set_resting(false);

	// If the attacker was invisible, she isn't anymore!
	a_.get_unit().set_state(unit::STATE_UNCOVERED, true);

	bc_ = new battle_context(units_, a_.loc_, d_.loc_, a_.weapon_, d_.weapon_);
	a_stats_ = &bc_->get_attacker_stats();
	d_stats_ = &bc_->get_defender_stats();
	if(a_stats_->weapon) {
		a_.weap_id_ = a_stats_->weapon->id();
	}
	if(d_stats_->weapon) {
		d_.weap_id_ = d_stats_->weapon->id();
	}

	try {
		fire_event("attack");
	} catch (attack_end_exception) {
		return;
	}
	refresh_bc();

	DBG_NG << "getting attack statistics\n";
	statistics::attack_context attack_stats(a_.get_unit(), d_.get_unit(), a_stats_->chance_to_hit, d_stats_->chance_to_hit);

	{
		// Calculate stats for battle
		combatant attacker(bc_->get_attacker_stats());
		combatant defender(bc_->get_defender_stats());
		attacker.fight(defender,false);
		const double attacker_inflict = static_cast<double>(d_.get_unit().hitpoints()) - defender.average_hp();
		const double defender_inflict = static_cast<double>(a_.get_unit().hitpoints()) - attacker.average_hp();

		attack_stats.attack_expected_damage(attacker_inflict,defender_inflict);
	}

	a_.orig_attacks_ = a_stats_->num_blows;
	d_.orig_attacks_ = d_stats_->num_blows;
	a_.n_attacks_ = a_.orig_attacks_;
	d_.n_attacks_ = d_.orig_attacks_;
	a_.xp_ = d_.get_unit().level();
	d_.xp_ = a_.get_unit().level();

	bool defender_strikes_first = (d_stats_->firststrike && !a_stats_->firststrike);
	unsigned int rounds = std::max<unsigned int>(a_stats_->rounds, d_stats_->rounds) - 1;
	const int defender_side = d_.get_unit().side();

	LOG_NG << "Fight: (" << a_.loc_ << ") vs (" << d_.loc_ << ") ATT: " << a_stats_->weapon->name() << " " << a_stats_->damage << "-" << a_stats_->num_blows << "(" << a_stats_->chance_to_hit << "%) vs DEF: " << (d_stats_->weapon ? d_stats_->weapon->name() : "none") << " " << d_stats_->damage << "-" << d_stats_->num_blows << "(" << d_stats_->chance_to_hit << "%)" << (defender_strikes_first ? " defender first-strike" : "") << "\n";

	// Play the pre-fight animation
	unit_display::unit_draw_weapon(a_.loc_,a_.get_unit(),a_stats_->weapon,d_stats_->weapon,d_.loc_,&d_.get_unit());

	for (;;)
	{
		DBG_NG << "start of attack loop...\n";
		++abs_n_attack_;

		if (a_.n_attacks_ > 0 && !defender_strikes_first) {
			if (!perform_hit(true, attack_stats)) break;
		}

		// If the defender got to strike first, they use it up here.
		defender_strikes_first = false;
		++abs_n_defend_;

		if (d_.n_attacks_ > 0) {
			if (!perform_hit(false, attack_stats)) break;
		}

		// Continue the fight to death; if one of the units got petrified,
		// either n_attacks or n_defends is -1
		if(rounds > 0 && d_.n_attacks_ == 0 && a_.n_attacks_ == 0) {
			a_.n_attacks_ = a_.orig_attacks_;
			d_.n_attacks_ = d_.orig_attacks_;
			--rounds;
			defender_strikes_first = (d_stats_->firststrike && ! a_stats_->firststrike);
		}

		if (a_.n_attacks_ <= 0 && d_.n_attacks_ <= 0) {
			fire_event("attack_end");
			refresh_bc();
			break;
		}
	}

	// TODO: if we knew the viewing team, we could skip some of these display update
	if (update_def_fog_ && (*resources::teams)[defender_side - 1].uses_fog())
	{
		if (update_display_) {
			recalculate_fog(defender_side);
		}
	}

	if (update_minimap_ && update_display_) {
		resources::screen->redraw_minimap();
	}

	if (a_.valid()) {
		unit &u = a_.get_unit();
		u.set_standing();
		u.set_experience(u.experience() + a_.xp_);
	}

	if (d_.valid()) {
		unit &u = d_.get_unit();
		u.set_standing();
		u.set_experience(u.experience() + d_.xp_);
	}

	unit_display::unit_sheath_weapon(a_.loc_,a_.valid()?&a_.get_unit():NULL,a_stats_->weapon,
			d_stats_->weapon,d_.loc_,d_.valid()?&d_.get_unit():NULL);

	if (update_display_){
		resources::screen->invalidate_unit();
		resources::screen->invalidate(a_.loc_);
		resources::screen->invalidate(d_.loc_);
		resources::screen->draw(true, true);
	}

	if(OOS_error_) {
		replay::process_error(errbuf_.str());
	}
}


int village_owner(const map_location& loc, const std::vector<team>& teams)
{
	for(size_t i = 0; i != teams.size(); ++i) {
		if(teams[i].owns_village(loc))
			return i;
	}

	return -1;
}

bool get_village(const map_location& loc, int side, int *action_timebonus)
{
	std::vector<team> &teams = *resources::teams;
	team *t = unsigned(side - 1) < teams.size() ? &teams[side - 1] : NULL;
	if (t && t->owns_village(loc)) {
		return false;
	}

	bool has_leader = resources::units->find_leader(side).valid();
	bool grants_timebonus = false;

	int old_owner_side = 0;
	// We strip the village off all other sides, unless it is held by an ally
	// and we don't have a leader (and thus can't occupy it)
	for(std::vector<team>::iterator i = teams.begin(); i != teams.end(); ++i) {
		int i_side = i - teams.begin() + 1;
		if (!t || has_leader || t->is_enemy(i_side)) {
			if(i->owns_village(loc)) {
				old_owner_side = i_side;
				i->lose_village(loc);
			}
			if (side != i_side && action_timebonus) {
				grants_timebonus = true;
			}
		}
	}

	if (!t) return false;

	if(grants_timebonus) {
		t->set_action_bonus_count(1 + t->action_bonus_count());
		*action_timebonus = 1;
	}

	if(has_leader) {
		if (resources::screen != NULL) {
			resources::screen->invalidate(loc);
		}
		return t->get_village(loc, old_owner_side);
	}

	return false;
}

// Simple algorithm: no maximum number of patients per healer.
void reset_resting(unit_map& units, int side)
{
	BOOST_FOREACH(unit &u, units) {
		if (u.side() == side)
			u.set_resting(true);
	}
}

/* Contains all the data used to display healing */
struct unit_healing_struct {
	unit *healed;
	std::vector<unit *> healers;
	int healing;
};

void calculate_healing(int side, bool update_display)
{
	DBG_NG << "beginning of healing calculations\n";
	unit_map &units = *resources::units;

	std::list<unit_healing_struct> l;

	// We look for all allied units, then we see if our healer is near them.
	BOOST_FOREACH(unit &u, units) {

		if (u.get_state("unhealable") || u.incapacitated())
			continue;

		DBG_NG << "found healable unit at (" << u.get_location() << ")\n";

		unit_map::iterator curer = units.end();
		std::vector<unit *> healers;

		int healing = 0;
		int rest_healing = 0;

		std::string curing;

		unit_ability_list heal = u.get_abilities("heals");

		const bool is_poisoned = u.get_state(unit::STATE_POISONED);
		if(is_poisoned) {
			// Remove the enemies' healers to determine if poison is slowed or cured
			for (std::vector<std::pair<const config *, map_location> >::iterator
					h_it = heal.cfgs.begin(); h_it != heal.cfgs.end();) {

				unit_map::iterator potential_healer = units.find(h_it->second);

				assert(potential_healer != units.end());
				if ((*resources::teams)[potential_healer->side() - 1].is_enemy(side)) {
					h_it = heal.cfgs.erase(h_it);
				} else {
					++h_it;
				}
			}
			for (std::vector<std::pair<const config *, map_location> >::const_iterator
					heal_it = heal.cfgs.begin(); heal_it != heal.cfgs.end(); ++heal_it) {

				if((*heal_it->first)["poison"] == "cured") {
					curer = units.find(heal_it->second);
					// Full curing only occurs on the healer turn (may be changed)
					if(curer->side() == side) {
						curing = "cured";
					} else if(curing != "cured") {
						curing = "slowed";
					}
				} else if(curing != "cured" && (*heal_it->first)["poison"] == "slowed") {
					curer = units.find(heal_it->second);
					curing = "slowed";
				}
			}
		}

		// For heal amounts, only consider healers on side which is starting now.
		// Remove all healers not on this side.
		for (std::vector<std::pair<const config *, map_location> >::iterator h_it =
				heal.cfgs.begin(); h_it != heal.cfgs.end();) {

			unit_map::iterator potential_healer = units.find(h_it->second);
			assert(potential_healer != units.end());
			if(potential_healer->side() != side) {
				h_it = heal.cfgs.erase(h_it);
			} else {
				++h_it;
			}
		}

		unit_abilities::effect heal_effect(heal,0,false);
		healing = heal_effect.get_composite_value();

		for(std::vector<unit_abilities::individual_effect>::const_iterator heal_loc = heal_effect.begin(); heal_loc != heal_effect.end(); ++heal_loc) {
			healers.push_back(&*units.find(heal_loc->loc));
		}

		if (!healers.empty()) {
			DBG_NG << "Unit has " << healers.size() << " potential healers\n";
		}

		if (u.side() == side) {
			unit_ability_list regen = u.get_abilities("regenerate");
			unit_abilities::effect regen_effect(regen,0,false);
			if(regen_effect.get_composite_value() > healing) {
				healing = regen_effect.get_composite_value();
				healers.clear();
			}
			if(!regen.cfgs.empty()) {
				for (std::vector<std::pair<const config *, map_location> >::const_iterator regen_it = regen.cfgs.begin(); regen_it != regen.cfgs.end(); ++regen_it) {
					if((*regen_it->first)["poison"] == "cured") {
						curer = units.end();
						curing = "cured";
					} else if(curing != "cured" && (*regen_it->first)["poison"] == "slowed") {
						curer = units.end();
						curing = "slowed";
					}
				}
			}
			if (int h = resources::game_map->gives_healing(u.get_location())) {
				if (h > healing) {
					healing = h;
					healers.clear();
				}
				/** @todo FIXME */
				curing = "cured";
				curer = units.end();
			}
			if (u.resting() || u.is_healthy()) {
				rest_healing = game_config::rest_heal_amount;
				healing += rest_healing;
			}
		}
		if(is_poisoned) {
			if(curing == "cured") {
				u.set_state(unit::STATE_POISONED, false);
				healing = rest_healing;
				healers.clear();
				if (curer != units.end())
					healers.push_back(&*curer);
			} else if(curing == "slowed") {
				healing = rest_healing;
				healers.clear();
				if (curer != units.end())
					healers.push_back(&*curer);
			} else {
				healers.clear();
				healing = rest_healing;
				if (u.side() == side) {
					healing -= game_config::poison_amount;
				}
			}
		}

		if (curing == "" && healing==0) {
			continue;
		}

		int pos_max = u.max_hitpoints() - u.hitpoints();
		int neg_max = -(u.hitpoints() - 1);
		if(healing > 0 && pos_max <= 0) {
			// Do not try to "heal" if HP >= max HP
			continue;
		}
		if(healing > pos_max) {
			healing = pos_max;
		} else if(healing < neg_max) {
			healing = neg_max;
		}

		if (!healers.empty()) {
			DBG_NG << "Just before healing animations, unit has " << healers.size() << " potential healers\n";
		}


		if (!recorder.is_skipping() && update_display &&
		    !(u.invisible(u.get_location()) &&
		      (*resources::teams)[resources::screen->viewing_team()].is_enemy(side)))
		{
			unit_healing_struct uhs = { &u, healers, healing };
			l.push_front(uhs);
		}
		if (healing > 0)
			u.heal(healing);
		else if (healing < 0)
			u.take_hit(-healing);
		resources::screen->invalidate_unit();
	}

	// Display healing with nearest first algorithm.
	if (!l.empty()) {

		// The first unit to be healed is chosen arbitrarily.
		unit_healing_struct uhs = l.front();
		l.pop_front();

		unit_display::unit_healing(*uhs.healed, uhs.healed->get_location(),
			uhs.healers, uhs.healing);

		/* next unit to be healed is nearest from uhs left in list l */
		while (!l.empty()) {

			std::list<unit_healing_struct>::iterator nearest;
			int min_d = INT_MAX;

			/* for each unit in l, remember nearest */
			for (std::list<unit_healing_struct>::iterator i =
			     l.begin(), i_end = l.end(); i != i_end; ++i)
			{
				int d = distance_between(uhs.healed->get_location(), i->healed->get_location());
				if (d < min_d) {
					min_d = d;
					nearest = i;
				}
			}

			uhs = *nearest;
			l.erase(nearest);

			unit_display::unit_healing(*uhs.healed, uhs.healed->get_location(),
				uhs.healers, uhs.healing);
		}
	}

	DBG_NG << "end of healing calculations\n";
}


unit get_advanced_unit(const unit &u, const std::string& advance_to)
{
	const unit_type *new_type = unit_types.find(advance_to);
	if (!new_type) {
		throw game::game_error("Could not find the unit being advanced"
			" to: " + advance_to);
	}
	unit new_unit(u);
	new_unit.set_experience(new_unit.experience() - new_unit.max_experience());
	new_unit.advance_to(new_type);
	new_unit.set_state(unit::STATE_POISONED, false);
	new_unit.set_state(unit::STATE_SLOWED, false);
	new_unit.set_state(unit::STATE_PETRIFIED, false);
	new_unit.set_user_end_turn(false);
	new_unit.set_hidden(false);
	return new_unit;
}

void advance_unit(map_location loc, const std::string &advance_to, const bool &fire_event)
{
	unit_map::unit_iterator u = resources::units->find(loc);
	if(!u.valid()) {
		return;
	}
	// original_type is not a reference, since the unit may disappear at any moment.
	std::string original_type = u->type_id();

	if(fire_event)
	{
		LOG_NG << "firing advance event at " << loc <<"\n";
		game_events::fire("advance",loc);

		if (!u.valid() || u->experience() < u->max_experience() ||
			u->type_id() != original_type)
		{
			LOG_NG << "WML has invalidated the advancing unit, abort\n";
			return;
		}
	}


	loc = u->get_location();
	unit new_unit = get_advanced_unit(*u, advance_to);
	statistics::advance_unit(new_unit);

	preferences::encountered_units().insert(new_unit.type_id());
	LOG_CF << "Added '" << new_unit.type_id() << "' to encountered units\n";

	resources::units->replace(loc, new_unit);
	if(fire_event)
	{
		LOG_NG << "firing post_advance event at " << loc << "\n";
		game_events::fire("post_advance",loc);
	}

	resources::whiteboard->on_gamestate_change();
}

int combat_modifier(const map_location &loc,
	unit_type::ALIGNMENT alignment, bool is_fearless)
{
	const time_of_day &tod = resources::tod_manager->get_illuminated_time_of_day(loc);

	int bonus;
	int lawful_bonus = tod.lawful_bonus;

	switch(alignment) {
		case unit_type::LAWFUL:
			bonus = lawful_bonus;
			break;
		case unit_type::NEUTRAL:
			bonus = 0;
			break;
		case unit_type::CHAOTIC:
			bonus = -lawful_bonus;
			break;
		case unit_type::LIMINAL:
			bonus = -(abs(lawful_bonus));
			break;
		default:
			bonus = 0;
	}

	if(is_fearless)
		bonus = std::max<int>(bonus, 0);

	return bonus;
}

namespace {

	/**
	 * Clears shroud from a single location.
	 *
	 * In a few cases, this will also clear corner hexes that otherwise would
	 * not normally get cleared.
	 * @param tm               The team whose fog/shroud is affected.
	 * @param loc              The location to clear.
	 * @param viewer           The unit doing the viewing.
	 * @param seen_units       If the location was cleared and contained a visible,
	 *                         non-petrified unit, it gets added to this set.
	 * @param petrified_units  If the location was cleared and contained a visible,
	 *                         petrified unit, it gets added to this set.
	 * @param known_units      These locations are excluded from being added to
	 *                         seen_units and petrified_units.
	 *
	 * @return whether or not information was uncovered (i.e. returns true if
	 *         the specified location was fogged/ shrouded under shared vision/maps).
	 */
	bool clear_shroud_loc(team &tm, const map_location& loc, const unit & viewer,
	                      std::set<map_location>* seen_units = NULL,
	                      std::set<map_location>* petrified_units = NULL,
	                      const std::set<map_location>* known_units = NULL)
	{
		gamemap &map = *resources::game_map;
		// This counts as clearing a tile for the return value if it is on the
		// board and currently fogged under shared vision. (No need to explicitly
		// check for shrouded since shrouded implies fogged.)
		bool was_fogged = tm.fogged(loc);
		bool result = was_fogged && map.on_board(loc);

		// Clear the border as well as the board, so that the half-hexes
		// at the edge can also be cleared of fog/shroud.
		if ( map.on_board_with_border(loc)) {
			// Both functions should be executed so don't use || which
			// uses short-cut evaluation.
			// (This is different than the return value because shared vision does
			// not apply here.)
			if ( tm.clear_shroud(loc) | tm.clear_fog(loc) ) {
				// If we are near a corner, the corner might also need to be cleared.
				// This happens at the lower-left corner and at either the upper- or
				// lower- right corner (depending on the width).

				// Lower-left corner:
				if ( loc.x == 0  &&  loc.y == map.h()-1 ) {
					const map_location corner(-1, map.h());
					tm.clear_shroud(corner);
					tm.clear_fog(corner);
				}
				// Lower-right corner, odd width:
				else if ( is_odd(map.w())  &&  loc.x == map.w()-1  &&  loc.y == map.h()-1 ) {
					const map_location corner(map.w(), map.h());
					tm.clear_shroud(corner);
					tm.clear_fog(corner);
				}
				// Upper-right corner, even width:
				else if ( is_even(map.w())  &&  loc.x == map.w()-1  &&  loc.y == 0) {
					const map_location corner(map.w(), -1);
					tm.clear_shroud(corner);
					tm.clear_fog(corner);
				}
			}
		}

		// Possible screen invalidation.
		if ( was_fogged ) {
			resources::screen->invalidate(loc);
			// Need to also invalidate adjacent hexes to get rid of the
			// "fog edge" graphics.
			map_location adjacent[6];
			get_adjacent_tiles(loc, adjacent);
			for ( int i = 0; i != 6; ++i )
				resources::screen->invalidate(adjacent[i]);
		}

		// Does the caller want a list of discovered units?
		if ( result  &&  (seen_units || petrified_units) ) {
			// Allow known_units to override fogged().
			if ( loc != viewer.get_location()  &&
			     (known_units == NULL  ||  known_units->count(loc) == 0) )
			{
				// Is there a visible unit here?
				const unit_map::const_iterator sighted = resources::units->find(loc);
				if ( sighted.valid() ) {
					if ( !tm.is_enemy(sighted->side()) ||
					     !sighted->invisible(loc) )
					{
						// Add this unit to the appropriate list.
						if ( !sighted->get_state(unit::STATE_PETRIFIED) )
						{
							if ( seen_units != NULL )
								seen_units->insert(loc);
						}
						else if ( petrified_units != NULL )
							petrified_units->insert(loc);
					}
				}
			}
		}

		return result;
	}

	/**
	 * Clears shroud (and fog) around the provided location for @a view_team as
	 * if a unit with @a viewer's sight range was standing there.
	 * (This uses a team parameter instead of a side since it is assumed that
	 * the caller already checked for fog or shroud being in use. Hence the
	 * caller has the team readily available.)
	 *
	 * @a seen_units will return new units that have been seen by this unit.
	 *
	 * @return whether or not information was uncovered (i.e. returns true if any
	 *         locations in visual range were fogged/shrouded under shared vision/maps).
	 */
	bool clear_shroud_unit(const map_location &view_loc, const unit &viewer,
	                       team &view_team, const std::map<map_location, int>& jamming_map,
	                       const std::set<map_location>* known_units = NULL,
	                       std::set<map_location>* seen_units = NULL,
	                       std::set<map_location>* petrified_units = NULL)
	{
		bool cleared_something = false;

		// Clear the fog.
		pathfind::vision_path sight(*resources::game_map, viewer, view_loc, jamming_map);
		BOOST_FOREACH(const pathfind::paths::step &dest, sight.destinations) {
			if ( clear_shroud_loc(view_team, dest.curr, viewer, seen_units,
			                      petrified_units, known_units) )
				cleared_something = true;
		}
		//TODO guard with game_config option
		BOOST_FOREACH(const map_location &dest, sight.edges) {
			if ( clear_shroud_loc(view_team, dest, viewer, seen_units,
			                      petrified_units, known_units) )
				cleared_something = true;
		}

		return cleared_something;
	}

	/**
	 * Wrapper for the invalidations that should occur after fog or
	 * shroud is cleared. (Needed in multiple places, so this makes
	 * sure the same things are called each time.) This would be
	 * called after one is done calling clear_shroud_unit().
	 */
	void invalidate_after_clearing_shroud()
	{
		resources::screen->invalidate_game_status();
		resources::screen->recalculate_minimap();
		resources::screen->labels().recalculate_shroud();
		// The tiles are invalidated as they are cleared, so no need
		// to invalidate them here.
	}


	void calculate_jamming(int side, std::map<map_location, int>& jamming_map)
	{
		team& viewer_tm = (*resources::teams)[side - 1];

		BOOST_FOREACH(const unit &u, *resources::units)
		{
			if (!viewer_tm.is_enemy(u.side())) continue;
			if (u.jamming() < 1) continue;

			int current = jamming_map[u.get_location()];
			if (current < u.jamming()) jamming_map[u.get_location()] = u.jamming();

			pathfind::jamming_path jamming(*resources::game_map, u, u.get_location());
			BOOST_FOREACH(const pathfind::paths::step& st, jamming.destinations) {
				current = jamming_map[st.curr];
				if (current < st.move_left) jamming_map[st.curr] = st.move_left;
			}

		}
	}

}

/**
 * Function that recalculates the fog of war.
 *
 * This is used at the end of a turn and for the defender at the end of
 * combat. As a back-up, it is also called when clearing shroud at the
 * beginning of a turn.
 * This function does nothing if the indicated side does not use fog.
 * The display is invalidated as needed.
 *
 * @param[in] side The side whose fog will be recalculated.
 */
void recalculate_fog(int side)
{
	team &tm = (*resources::teams)[side - 1];

	if (!tm.uses_fog())
		return;

	// The following lines will be useful at some point, but not yet.
	// So they are commented out for now.
	//std::set<map_location> visible_locs;
	//// Loop through all units, looking for those that are visible.
	//BOOST_FOREACH(const unit &u, *resources::units) {
	//	const map_location & u_location = u.get_location();
	//
	//	if ( !tm.fogged(u_location) )
	//		visible_locs.insert(u_location);
	//}

	tm.refog();
	// Invalidate the screen before clearing the shroud.
	// This speeds up the invalidations within clear_shroud_unit().
	resources::screen->invalidate_all();

	std::map<map_location, int> jamming_map;
	calculate_jamming(side, jamming_map);
	BOOST_FOREACH(const unit &u, *resources::units)
	{
		if (u.side() == side) {
			clear_shroud_unit(u.get_location(), u, tm, jamming_map);
		}
	}

	//FIXME: This pump don't catch any sighted events (they are not fired by
	// clear_shroud_unit) and if it caches another old event, maybe the caller
	// don't want to pump it here
	game_events::pump();

	// Update the screen.
	invalidate_after_clearing_shroud();
}

/**
 * Function that will clear shroud (and fog) based on current unit positions.
 *
 * This will not re-fog hexes unless reset_fog is set to true.
 * This function will do nothing if the side uses neither shroud nor fog.
 * The display is invalidated as needed.
 *
 * @param[in] side      The side whose shroud (and fog) will be cleared.
 * @param[in] reset_fog If set to true, the fog will also be recalculated
 *                      (refogging hexes that can no longer be seen).
 * @returns true if some shroud/fog is actually cleared away.
 */
bool clear_shroud(int side, bool reset_fog)
{
	team &tm = (*resources::teams)[side - 1];
	if (!tm.uses_shroud() && !tm.uses_fog())
		return false;

	bool result = false;

	std::map<map_location, int> jamming_map;
	calculate_jamming(side, jamming_map);
	BOOST_FOREACH(const unit &u, *resources::units)
	{
		if (u.side() == side) {
			result |= clear_shroud_unit(u.get_location(), u, tm, jamming_map);
		}
	}

	//FIXME: This pump don't catch any sighted events (they are not fired by
	// clear_shroud_unit) and if it caches another old event, maybe the caller
	// don't want to pump it here
	game_events::pump();

	if ( reset_fog ) {
		// Note: This will not reveal any new tiles, so result is not affected.
		// Note: This will call invalidate_after_clearing_shroud().
		recalculate_fog(side);
	}
	else if ( result ) {
		// Update the screen.
		invalidate_after_clearing_shroud();
	}

	return result;
}


namespace { // Private helpers for move_unit()

	/// Helper class for move_unit().
	class unit_mover : public boost::noncopyable {
		typedef std::vector<map_location>::const_iterator route_iterator;

	public:
		unit_mover(const std::vector<map_location> & route,
		           move_unit_spectator *move_spectator,
		           bool skip_sightings, const map_location *replay_dest);
		~unit_mover();

		/// Determines how far along the route the unit can expect to move this turn.
		bool check_expected_movement();
		/// Attempts to move the unit along the expected path.
		void try_actual_movement(bool show);
		/// Does some bookkeeping and event firing, for use after movement.
		void post_move(undo_list *undo_stack);
		/// Shows the various on-screen messages, for use after movement.
		void feedback() const;

		/// After checking expected movement, this is the expected path.
		std::vector<map_location> expected_path() const
		{ return std::vector<map_location>(begin_, expected_end_); }
		/// After moving, this is the final hex reached.
		const map_location & final_hex() const
		{ return *move_loc_; }
		/// After moving, this indicates if any units were seen.
		/// This includes seen units that would not interrupt movement.
		bool saw_units() const  { return !seen_units_.empty(); }
		/// The number of hexes actually entered.
		size_t steps_travelled() const
		{ return move_loc_ - begin_; }
		/// After moving, use this to detect if movement was less than expected.
		bool stopped_early() const  { return expected_end_ != real_end_; }
		/// After moving, use this to detect if something happened that would
		/// interrupt movement (even if movement ended for a different reason).
		bool interrupted(bool include_end_of_move_events=true) const
		{
			return ambushed_ || blocked_ || sighted_ || teleport_failed_ ||
			       (include_end_of_move_events ? event_mutated_ : event_mutated_mid_move_ ) ||
			       !move_it_.valid();
		}

	private: // functions
		/// Checks the expected route for hidden units.
		void cache_hidden_units(const route_iterator & start,
		                        const route_iterator & stop);
		/// Fires the enter_hex or exit_hex event and updates our data as needed.
		bool fire_hex_event(const std::string & event_name,
		                    const route_iterator & current,
	                        const route_iterator & other);
		/// Checks how far it appears we can move this turn.
		route_iterator plot_turn(const route_iterator & start,
		                         const route_iterator & stop);
		/// Updates our stored info after a WML event might have changed something.
		bool post_wml(const route_iterator & step);
		bool post_wml() { return post_wml(full_end_); }
		/// Fires the sighted events that were raised earlier.
		bool pump_sighted(const route_iterator & from);
		/// If ambush_string_ is empty, set it to the "alert" for the given unit.
		void read_ambush_string(const unit_map::const_iterator & ambush_it);

		/// Returns whether or not undoing this move should be blocked.
		bool undo_blocked() const
		{ return ambushed_ || blocked_ || event_mutated_ || fog_changed_ ||
		         saw_units() || teleport_failed_; } // Should not be necessary to check saw_units(), but why assume?

		// The remaining private functions are suggested to be inlined because
		// each is used in only one place. (They are separate functions to ease
		// code reading.)

		/// Checks for ambushers around @a hex, setting flags as appropriate.
		inline void check_for_ambushers(const map_location & hex);
		/// Makes sure the path is not obstructed by a unit.
		inline bool check_for_obstructing_unit(const map_location & hex,
		                                       const map_location & prev_hex);
		/// Moves the unit the next step.
		inline void do_move(const route_iterator & step_from,
		                    const route_iterator & step_to,
		                    unit_display::unit_mover & animator);
		/// Clears fog/shroud and handles units being sighted.
		inline void handle_fog(const map_location & hex, bool ally_interrupts);
		inline bool is_reasonable_stop(const map_location & hex) const;
		/// Reveals the units stored in to_reveal_.
		inline void reveal_ambushers() const;
		/// Makes sure the units in to_reveal_ still exist.
		inline void validate_ambushers();

	private: // data
		// (The order of the fields is somewhat important for the constructor.)

		// Movement parameters (these decrease the number of parameters needed
		// for individual functions).
		move_unit_spectator * const spectator_;
		const bool is_replay_;
		const map_location & replay_dest_;
		const bool skip_sighting_;
		// Currently needed to interface with the animation class.
		const std::vector<map_location> & route_;

		// The route to traverse.
		const route_iterator begin_;
		const route_iterator full_end_;	// The end of the plotted route.
		route_iterator expected_end_;	// The end of this turn's portion of the plotted route.
		route_iterator ambush_limit_;	// How far we can go before encountering hidden units, ignoring allied units.
		route_iterator obstructed_;	// Points to either full_end_ or a hex we cannot enter. This is used so that exit_hex can fire before we decide we cannot enter this hex.
		route_iterator real_end_;	// How far we actually can move this turn.

		// The unit that is moving.
		unit_map::iterator move_it_;

		// This data stores the state from before the move started.
		const int orig_side_;
		const int orig_moves_;
		const map_location::DIRECTION orig_dir_;
		const map_location goto_;

		// This data tracks the current state as the move is in proress.
		int current_side_;
		team * current_team_;	// Will default to the original team if the moving unit becomes invalid.
		bool current_uses_fog_;
		route_iterator move_loc_; // Will point to the last moved-to location (in case the moving unit disappears).
		size_t do_move_track_;	// Tracks whether or not do_move() needs to update the displayed (fake) unit. Should only be touched by do_move() and the constructor.

		// Data accumulated while making the move.
		map_location zoc_stop_;
		map_location ambush_stop_; // Could be inaccurate if ambushed_ is false.
		bool ambushed_;
		bool blocked_; // Blocked by an enemy (non-ambusher) unit
		bool event_mutated_;
		bool event_mutated_mid_move_; // Cache of event_mutated_ from just before the end-of-move handling.
		bool fog_changed_;
		bool sighted_;	// Records if sightings were made that could interrupt movement.
		bool sighted_stop_;	// Records if sightings were made that did interrupt movement (the same as sighted_ unless movement ended for another reason).
		bool teleport_failed_;
		bool report_extra_hex_;
		std::string ambush_string_;
		std::map<map_location, int> jamming_;
		std::deque<int> moves_left_;	// The front value is what the moving unit's remaining moves should be set to after the next step through the route.
		std::set<map_location> seen_units_;
		std::vector<map_location> to_reveal_;
	};


	/// This constructor assumes @a route is not empty, and it will assert() that
	/// there is a unit at route.front().
	/// Iterators into @a route must remain valid for the life of this object.
	unit_mover::unit_mover(const std::vector<map_location> & route,
	                       move_unit_spectator *move_spectator,
	                       bool skip_sightings, const map_location *replay_dest) :
		spectator_(move_spectator),
		is_replay_(replay_dest != NULL),
		replay_dest_(is_replay_ ? *replay_dest : route.back()),
		skip_sighting_(is_replay_ || skip_sightings),
		route_(route),
		begin_(route.begin()),
		full_end_(route.end()),
		expected_end_(begin_),
		ambush_limit_(begin_),
		obstructed_(full_end_),
		real_end_(begin_),
		// Unit information:
		move_it_(resources::units->find(*begin_)),
		orig_side_(( assert(move_it_ != resources::units->end()),
		             move_it_->side() )),
		orig_moves_(move_it_->movement_left()),
		orig_dir_(move_it_->facing()),
		goto_(spectator_ == NULL ? route.back() : move_it_->get_goto()),
		current_side_(orig_side_),
		current_team_(&(*resources::teams)[current_side_-1]),
		current_uses_fog_(current_team_->fog_or_shroud()  &&
		                  current_team_->auto_shroud_updates()),
		move_loc_(begin_),
		do_move_track_(game_events::wml_tracking()),
		// The remaining fields are set to some sort of "zero state".
		zoc_stop_(map_location::null_location),
		ambush_stop_(map_location::null_location),
		ambushed_(false),
		blocked_(false),
		event_mutated_(false),
		event_mutated_mid_move_(false),
		fog_changed_(false),
		sighted_(false),
		sighted_stop_(false),
		teleport_failed_(false),
		report_extra_hex_(false),
		ambush_string_(),
		jamming_(),
		moves_left_(),
		seen_units_(),
		to_reveal_()
	{
		// Clear the "goto" instruction during movement (so that the orb is not red).
		move_it_->set_goto(map_location::null_location);
	}


	unit_mover::~unit_mover()
	{
		// Set the "goto" order? (Not if WML set it during movement.)
		if ( move_it_.valid()  &&  move_it_->get_goto() == map_location::null_location )
		{
			if ( spectator_ != NULL )
				// When a spectator is supplied, it should be as if we never changed
				// the goto.
				// NOTE: Currently (July 2012), spectator_ != NULL implies that
				// this is an AI move.
				move_it_->set_goto(goto_);
			else
				// Only set the goto if movement was not complete and was not
				// interrupted.
				if ( real_end_ != full_end_  &&  !interrupted(false) ) // End-of-move-events do not cancel a goto. (Use case: tutorial S2.)
						move_it_->set_goto(goto_);
		}
	}


	// Private inlines:

	/**
	 * Checks for ambushers around @a hex, setting flags as appropriate.
	 */
	inline void unit_mover::check_for_ambushers(const map_location & hex)
	{
		const unit_map &units = *resources::units;

		// Need to check each adjacent hex for hidden enemies.
		map_location adjacent[6];
		get_adjacent_tiles(hex, adjacent);
		for ( int i = 0; i != 6; ++i )
		{
			const unit_map::const_iterator neighbor_it = units.find(adjacent[i]);

			if ( neighbor_it != units.end()  &&
			     current_team_->is_enemy(neighbor_it->side())  &&
			     neighbor_it->invisible(adjacent[i]) )
			{
				// Ambushed!
				ambushed_ = true;
				ambush_stop_ = hex;
				to_reveal_.push_back(adjacent[i]);

				// Get a feedback message (first one available).
				if ( ambush_string_.empty() )
					read_ambush_string(neighbor_it);
			}
		}
	}


	/**
	 * Makes sure the path is not obstructed by a unit.
	 * @param  hex       The hex to check.
	 * @param  prev_hex  The previous hex in the route (used to detect a teleport).
	 * @return true if @a hex is obstructed.
	 */
	inline bool unit_mover::check_for_obstructing_unit(const map_location & hex,
	                                                   const map_location & prev_hex)
	{
		const unit_map::const_iterator blocking_unit = resources::units->find(hex);

		// If no unit, then the path is not obstructed.
		if ( blocking_unit == resources::units->end() )
			return false;

		if ( !tiles_adjacent(hex, prev_hex) ) {
			// Cannot teleport to an occupied hex.
			teleport_failed_ = true;
			return true;
		}

		if ( current_team_->is_enemy(blocking_unit->side()) ) {
			// Trying to go through an enemy.
			blocked_ = true;
			to_reveal_.push_back(hex);
			return true;
		}

		// If we get here, the unit does not interfere with movement.
		return false;
	}


	/**
	 * Moves the unit the next step.
	 * @a step_to is the hex being moved to.
	 * @a step_from is the hex before that in the route.
	 * (The unit is actually at *move_loc_.)
	 * @a animator is the unit_display::unit_mover being used.
	 */
	inline void unit_mover::do_move(const route_iterator & step_from,
	                                const route_iterator & step_to,
	                                unit_display::unit_mover & animator)
	{
		game_display &disp = *resources::screen;

		// Adjust the movement even if we cannot move yet.
		// We will eventually be able to move if nothing unexpected
		// happens, and if something does happen, this movement is the
		// cost to discover it.
		move_it_->set_movement(moves_left_.front());
		moves_left_.pop_front();

		// Attempt actually moving.
		// (Fails if *step_to is occupied).
		std::pair<unit_map::iterator, bool> move_result =
			resources::units->move(*move_loc_, *step_to);
		if ( move_result.second )
		{
			// Update the moving unit.
			move_it_ = move_result.first;
			move_it_->set_facing(step_from->get_relative_dir(*step_to));
			move_it_->set_standing(false);
			disp.invalidate_unit_after_move(*move_loc_, *step_to);
			disp.invalidate(*move_loc_);
			disp.invalidate(*step_to);
			move_loc_ = step_to;

			// Show this move.
			const size_t current_tracking = game_events::wml_tracking();
			animator.proceed_to(*move_it_, step_to - begin_,
			                    current_tracking != do_move_track_);
			do_move_track_ = current_tracking;
			disp.redraw_minimap();
		}
	}


	/**
	 * Clears fog/shroud and raises events for units being sighted.
	 * Only call this if the current team uses fog or shroud.
	 * @a hex is both the center of fog clearing and the assumed location of
	 * the moving unit when the sighted events will be fired.
	 */
	inline void unit_mover::handle_fog(const map_location & hex, bool ally_interrupts)
	{
		static const std::string sighted_str("sighted");

		const unit_map & units = *resources::units;

		std::set<map_location> newly_seen_units;
		std::set<map_location> petrified_units;

		// Clear the fog.
		if ( clear_shroud_unit(hex, *move_it_, *current_team_, jamming_,
		                       NULL, &newly_seen_units, &petrified_units) )
		{
			invalidate_after_clearing_shroud();
			fog_changed_ = true;
		}

		// Raise sighted events.
		const game_events::entity_location viewer(*move_it_, hex);
		BOOST_FOREACH(const map_location &here, newly_seen_units) {
			game_events::raise(sighted_str, here, viewer);
		}
		BOOST_FOREACH(const map_location &here, petrified_units) {
			game_events::raise(sighted_str, here, viewer);
		}

		// Check for sighted units?
		if ( !skip_sighting_ && !sighted_ ) {
			if ( ally_interrupts )
				// Interrupt if any unit was sighted.
				sighted_ = !newly_seen_units.empty();
			else {
				// Check whether any sighted unit is an enemy.
				BOOST_FOREACH(const map_location &here, newly_seen_units) {
					if ( current_team_->is_enemy(units.find(here)->side()) ) {
						sighted_ = true;
						break;
					}
				}
			}
		}

		// Merge the seen units.
		seen_units_.insert(newly_seen_units.begin(), newly_seen_units.end());
	}


	/**
	 * @return true if an unscheduled stop at @a hex is not likely to negatively
	 * impact the player's plans.
	 * (E.g. it would not kill movement by making an unintended village capture.)
	 */
	inline bool unit_mover::is_reasonable_stop(const map_location & hex) const
	{
		// We cannot reasonably stop if move_it_ could not be moved to this
		// hex (the hex was occupied by someone else).
		if ( *move_loc_ != hex )
			return false;

		// We can reasonably stop if the hex is not an unowned village.
		return !resources::game_map->is_village(hex) ||
		       current_team_->owns_village(hex);
	}


	/**
	 * Reveals the units stored in to_reveal_.
	 * Only call this if appropriate; this function does not itself check
	 * ambushed_ or blocked_.
	 */
	inline void unit_mover::reveal_ambushers() const
	{
		// Some convenient aliases:
		game_display &disp = *resources::screen;
		unit_map &units = *resources::units;

		BOOST_FOREACH(const map_location & reveal, to_reveal_) {
			unit_map::iterator ambusher = units.find(reveal);
			if ( ambusher != units.end() ) {
				ambusher->set_state(unit::STATE_UNCOVERED, true);  // (Needed in case we backtracked.)
				if ( spectator_ )
					spectator_->set_ambusher(ambusher);
			}
			disp.invalidate(reveal);
		}

		disp.draw();
	}


	/**
	 * Makes sure the units in to_reveal_ still exist.
	 * Also updates ambush_string_ based on those that do still exist.
	 */
	inline void unit_mover::validate_ambushers()
	{
		const unit_map &units = *resources::units;

		ambush_string_.clear();

		// Loop through the previously-detected ambushers.
		size_t i = 0;
		while ( i != to_reveal_.size() ) {
			const unit_map::const_iterator ambush_it = units.find(to_reveal_[i]);
			if ( ambush_it == units.end() )
				// Ambusher is gone.
				to_reveal_.erase(to_reveal_.begin() + i);
			else {
				// Update ambush_string_ and proceed to the next ambusher.
				read_ambush_string(ambush_it);
				++i;
			}
		}
	}


	// Private utilities:

	/**
	 * Checks the expected route for hidden units.
	 * This basically handles all the checks for surprises that can be done
	 * without visibly notifying a player. Thus this can be called at the
	 * start of movement and immediately after events, rather than tie up
	 * CPU time in the middle of animating a move.
	 *
	 * @param[in]  start           The beginning of the path to check.
	 * @param[in]  stop            The end of the path to check.
	 */
	void unit_mover::cache_hidden_units(const route_iterator & start,
	                                    const route_iterator & stop)
	{
		// Clear the old cache.
		obstructed_ = full_end_;
		blocked_ = false;
		teleport_failed_ = false;
		jamming_.clear();
		// The ambush cache needs special treatment since we cannot re-detect
		// an ambush if we are already at the ambushed location.
		ambushed_ =  ambushed_  &&  ambush_stop_ == *start;
		if ( ambushed_ ) {
			validate_ambushers();
			ambushed_ = !to_reveal_.empty();
		}
		if ( !ambushed_ ) {
			ambush_stop_ = map_location::null_location;
			ambush_string_.clear();
			to_reveal_.clear();
		}

		// Calculate the jamming map?
		if ( current_uses_fog_ )
			calculate_jamming(current_side_, jamming_);


		// Abort for null routes.
		if ( start == stop ) {
			ambush_limit_ = start;
			return;
		}

		// This loop will end with ambush_limit_ pointing one element beyond
		// where the unit would be forced to stop by a hidden unit.
		for ( ambush_limit_ = start+1; ambush_limit_ != stop; ++ambush_limit_ ) {
			// Check if we need to stop in the previous hex.
			if ( ambushed_ )
				if ( !is_replay_  ||  *(ambush_limit_-1) == replay_dest_ )
					break;

			// Check for being unable to enter this hex.
			if ( check_for_obstructing_unit(*ambush_limit_, *(ambush_limit_-1)) ) {
				// No replay check here? Makes some sense, I guess.
				obstructed_ = ambush_limit_++; // The limit needs to be after obstructed_ in order for the latter to do anything.
				break;
			}

			// We can enter this hex.
			// See if we are stopped in this hex.
			check_for_ambushers(*ambush_limit_);
		}
	}


	/**
	 * Fires the enter_hex or exit_hex event and updates our data as needed.
	 *
	 * @param[in]  event_name  The name of the event ("enter_hex" or "exit_hex").
	 * @param[in]  current     The currently occupied hex.
	 * @param[in]  other       The secondary hex to provide to the event.
	 *
	 * @return true if this event should interrupt movement.
	 * (This is also stored in event_mutated_.)
	 */
	bool unit_mover::fire_hex_event(const std::string & event_name,
		                            const route_iterator & current,
	                                const route_iterator & other)
	{
		const size_t track = game_events::wml_tracking();
		bool valid = true;

		const game_events::entity_location mover(*move_it_, *current);
		const bool event = game_events::fire(event_name, mover, *other);

		if ( track != game_events::wml_tracking() )
			// Some WML fired, so update our status.
			valid = post_wml(current);

		if ( event || !valid )
			event_mutated_ = true;

		return event || !valid;
	}


	/**
	 * Checks how far it appears we can move this turn.
	 *
	 * @param[in] start         The beginning of the plotted path.
	 * @param[in] stop          The end of the plotted path.
	 *
	 * @return An end iterator for the path that can be traversed this turn.
	 */
	unit_mover::route_iterator unit_mover::plot_turn(const route_iterator & start,
	                                                 const route_iterator & stop)
	{
		const gamemap &map = *resources::game_map;

		// Handle null routes.
		if ( start == stop )
			return start;


		int remaining_moves = move_it_->movement_left();
		zoc_stop_ = map_location::null_location;
		moves_left_.clear();

		if ( start != begin_ ) {
			// Check for being unable to leave the current hex.
			if ( !move_it_->get_ability_bool("skirmisher", *start)  &&
			      pathfind::enemy_zoc(*current_team_, *start, *current_team_) )
				zoc_stop_ = *start;
		}

		// This loop will end with end pointing one element beyond where the
		// unit thinks it will stop (the usual notion of "end" for iterators).
		route_iterator end = start + 1;
		for ( ; end != stop; ++end )
		{
			// Break out of the loop if we cannot leave the previous hex.
			if ( zoc_stop_ != map_location::null_location  &&  !is_replay_ )
				break;
			remaining_moves -= move_it_->movement_cost(map[*end]);
			if ( remaining_moves < 0 ) {
				if ( is_replay_ )
					remaining_moves = 0;
				else
					break;
			}

			// We can enter this hex. Record the cost.
			moves_left_.push_back(remaining_moves);

			// Check for being unable to leave this hex.
			if ( !move_it_->get_ability_bool("skirmisher", *end)  &&
			      pathfind::enemy_zoc(*current_team_, *end, *current_team_) )
				zoc_stop_ = *end;
		}

		if ( !is_replay_ ) {
			// Avoiding stopping on a (known) unit.
			route_iterator min_end =  start == begin_ ? start : start + 1;
			while ( end != min_end  &&  get_visible_unit(*(end-1), *current_team_) )
				// Backtrack.
				--end;
		}

		return end;
	}


	/**
	 * Updates our stored info after a WML event might have changed something.
	 *
	 * @param step      Indicates the position in the path where we might need to start recalculating movement.
	 *                  Set this to full_end_ (or do not supply it) to skip such recalculations (because movement has finished).
	 *
	 * @returns false if continuing is impossible (i.e. we lost the moving unit).
	 */
	bool unit_mover::post_wml(const route_iterator & step)
	{
		// Re-find the moving unit.
		move_it_ = resources::units->find(*move_loc_);
		const bool found = move_it_ != resources::units->end();

		// Update the current unit data.
		current_side_ = found ? move_it_->side() : orig_side_;
		current_team_ = &(*resources::teams)[current_side_-1];
		current_uses_fog_ = current_team_->fog_or_shroud()  &&
		                    ( current_side_ != orig_side_  ||
		                      current_team_->auto_shroud_updates() );

		// Update the path.
		if ( found  &&  step != full_end_ ) {
			const route_iterator new_limit = plot_turn(step, expected_end_);
			cache_hidden_units(step, new_limit);
			// Just in case: length 0 paths become length 1 paths.
			if ( ambush_limit_ == step )
				++ambush_limit_;
		}

		return found;
	}


	/**
	 * Fires the sighted events that were raised earlier.
	 *
	 * @param[in]  from  Points to the hex the sighting unit currently occupies.
	 *
	 * @return true if this event should interrupt movement.
	 */
	bool unit_mover::pump_sighted(const route_iterator & from)
	{
		const size_t track = game_events::wml_tracking();
		bool valid = true;

		const bool event = game_events::pump();

		if ( track != game_events::wml_tracking() )
			// Some WML fired, so update our status.
			valid = post_wml(from);

		if ( event || !valid )
			event_mutated_ = true;

		return event || !valid;
	}


	/**
	 * If ambush_string_ is empty, set it to the "alert" for the given unit.
	 */
	void unit_mover::read_ambush_string(const unit_map::const_iterator & ambush_it)
	{
		const unit_ability_list hides = ambush_it->get_abilities("hides");
		std::vector<std::pair<const config *, map_location> >::const_iterator hide_it;
		for ( hide_it = hides.cfgs.begin();
		      hide_it != hides.cfgs.end()  &&  ambush_string_.empty();
		      ++hide_it )
		{
			ambush_string_ = (*hide_it->first)["alert"].str();
		}
	}


	// Public interface:

	/**
	 * Determines how far along the route the unit can expect to move this turn.
	 * This is based solely on data known to the player, and will not plot a move
	 * that ends on another (known) unit.
	 * (For example, this prevents a player from plotting a multi-turn move that
	 * has this turn's movement ending on a (slower) unit, and thereby clearing
	 * fog as if the moving unit actually made it on top of that other unit.)
	 *
	 * @returns false if the expectation is to not move at all.
	 */
	bool unit_mover::check_expected_movement()
	{
		expected_end_ = plot_turn(begin_, full_end_);
		return expected_end_ != begin_;
	}


	/**
	 * Attempts to move the unit along the expected path.
	 * (This will do nothing unless check_expected_movement() was called first.)
	 *
	 * @param[in]  show            Set to false to suppress animations.
	 */
	void unit_mover::try_actual_movement(bool show)
	{
		static const std::string enter_hex_str("enter_hex");
		static const std::string exit_hex_str("exit_hex");

		const bool ally_interrupts = preferences::interrupt_when_ally_sighted();

		bool obstructed_stop = false;


		// Check for hidden units along the expected path before we start
		// animating and firing events.
		cache_hidden_units(begin_, expected_end_);

		if ( begin_ != ambush_limit_ ) {
			// Prepare to animate.
			unit_display::unit_mover animator(route_, show);
			animator.start(*move_it_);

			// Traverse the route to the hex where we need to stop.
			// Each iteration performs the move from real_end_-1 to real_end_.
			for ( real_end_ = begin_+1; real_end_ != ambush_limit_; ++real_end_ ) {
				const route_iterator step_from = real_end_ - 1;
				const bool can_break = !is_replay_  ||  replay_dest_ == *step_from;

				// See if we can leave *step_from.
				// Already accounted for: ambusher
				if ( event_mutated_  &&  can_break )
					break;
				if ( sighted_ && can_break && is_reasonable_stop(*step_from) )
				{
					sighted_stop_ = true;
					break;
				}
				// Already accounted for: ZoC
				// Already accounted for: movement cost
				if ( fire_hex_event(exit_hex_str, step_from, real_end_) ) {
					if ( can_break ) {
						report_extra_hex_ = true;
						break;
					}
				}
				if ( real_end_ == obstructed_ ) {
					// We did not check for being a replay when checking for an
					// obstructed hex, so we do not check can_break here.
					report_extra_hex_ = true;
					obstructed_stop = true;
					break;
				}
				if ( is_replay_  &&  replay_dest_ == *step_from )
					// Preserve the replay.
					break;

				// We can leave *step_from. Make the move to *real_end_.
				do_move(step_from, real_end_, animator);

				// Update the fog.
				if ( current_uses_fog_ )
					handle_fog(*real_end_, ally_interrupts);

				// Fire the events for this move.
				pump_sighted(real_end_); // Ignore the return value, as there are more events to try to fire, and real_end_ still needs to be incremented.
				fire_hex_event(enter_hex_str, real_end_, step_from); // Ignore the return value, as real_end_ still needs to be incremented.
			}//for
			if ( move_it_.valid() )
				animator.finish(*move_it_);
		}//if

		// Some flags were set to indicate why we might stop.
		// Update those to reflect whether or not we got to them.
		ambushed_ = ambushed_ && real_end_ == ambush_limit_;
		blocked_  = blocked_  && obstructed_stop;
		teleport_failed_ = teleport_failed_ && obstructed_stop;
		// event_mutated_ does not get unset, regardless of other reasons
		// for stopping, but we do save its current value.
		event_mutated_mid_move_ = event_mutated_;

		// Need the default ambush message?
		if ( ambushed_  &&  ambush_string_.empty() )
			ambush_string_ = _("Ambushed!");
	}


	/**
	 * Does some bookkeeping and event firing, for use after movement.
	 * This includes village capturing and the undo stack.
	 */
	void unit_mover::post_move(undo_list *undo_stack)
	{
		const map_location & final_loc = final_hex();

		int orig_village_owner = -1;
		int action_time_bonus = 0;

		// Reveal ambushers?
		if ( ambushed_ || blocked_ )
			reveal_ambushers();
		else if ( teleport_failed_ && spectator_ )
			spectator_->set_failed_teleport(resources::units->find(*obstructed_));
		unit::clear_status_caches();

		if ( move_it_.valid() ) {
			// Update the moving unit.
			move_it_->set_interrupted_move(
				sighted_stop_ && !resources::whiteboard->is_executing_actions() ?
					*(full_end_-1) :
					map_location::null_location);
			if ( ambushed_ || final_loc == zoc_stop_ )
				move_it_->set_movement(0);

			// Village capturing.
			if ( resources::game_map->is_village(final_loc) ) {
				// Is this a capture?
				orig_village_owner = village_owner(final_loc, *resources::teams);
				if ( orig_village_owner != current_side_-1 ) {
					// Captured. Zap movement and take over the village.
					move_it_->set_movement(0);
					event_mutated_ |= get_village(final_loc, current_side_, &action_time_bonus);
					post_wml();
				}
			}
		}

		// Finally, the moveto event.
		event_mutated_ |= game_events::fire("moveto", final_loc, *begin_);
		post_wml();

		// Record keeping.
		if ( spectator_ )
			spectator_->set_unit(move_it_);
		if ( undo_stack ) {
			const bool mover_valid = move_it_.valid();

			if ( mover_valid ) {
				std::vector<map_location> steps(begin_, real_end_);
				// MP_COUNTDOWN: added param
				undo_stack->push_back(
					undo_action(*move_it_, steps, orig_moves_,
						action_time_bonus, orig_village_owner, orig_dir_));
			}

			if ( !mover_valid  ||  undo_blocked()  ||
			    (resources::whiteboard->is_active() && resources::whiteboard->should_clear_undo()) )
			{
				apply_shroud_changes(*undo_stack, orig_side_);
				undo_stack->clear();
			}
		}

		// Update the screen.
		resources::screen->redraw_minimap();
		resources::screen->draw();
	}


	/**
	 * Shows the various on-screen messages, for use after movement.
	 */
	void unit_mover::feedback() const
	{
		// Alias some resources.
		const unit_map &units = *resources::units;
		game_display &disp = *resources::screen;

		bool redraw = false;
		bool playing_team_is_viewing = disp.playing_team() == disp.viewing_team()
		                               ||  disp.show_everything();

		// Multiple messages may be displayed simultaneously
		// this variable is used to keep them from overlapping
		std::string message_prefix = "";

		// Ambush feedback?
		if ( ambushed_ ) {
			// Suppress the message for observers if the ambusher(s) cannot be seen.
			bool show_message = playing_team_is_viewing;
			BOOST_FOREACH(const map_location &ambush, to_reveal_) {
				if ( !disp.fogged(ambush) )
					show_message = true;
			}
			if ( show_message ) {
				disp.announce(message_prefix + ambush_string_, font::BAD_COLOR);
				message_prefix += " \n";
				redraw = true;
			}
		}

		// Failed teleport feedback?
		if ( playing_team_is_viewing  &&  teleport_failed_ ) {
			std::string teleport_string = _("Failed teleport! Exit not empty");
			disp.announce(message_prefix + teleport_string, font::BAD_COLOR);
			message_prefix += " \n";
			redraw = true;
		}

		// Sighted units feedback?
		if ( playing_team_is_viewing  &&  saw_units() ) {
			// Count the number of allies and enemies sighted.
			int friends = 0, enemies = 0;
			BOOST_FOREACH(const map_location &loc, seen_units_) {
				DBG_NG << "Processing unit at " << loc << "...\n";
				const unit_map::const_iterator seen_it = units.find(loc);

				// Unit may have been removed by an event.
				if ( seen_it == units.end() ) {
					DBG_NG << "...was removed.\n";
					continue;
				}

				if ( current_team_->is_enemy(seen_it->side()) ) {
					++enemies;
					if ( spectator_ )
						spectator_->add_seen_enemy(seen_it);
				} else {
					++friends;
					if ( spectator_ )
						spectator_->add_seen_friend(seen_it);
				}
				DBG_NG << "...processed.\n";
			}

			// Create the message to display (depends on whether firends,
			// enemies, or both were sighted, and on how many of each).
			utils::string_map symbols;
			symbols["friends"] = lexical_cast<std::string>(friends);
			symbols["enemies"] = lexical_cast<std::string>(enemies);
			std::string message;
			SDL_Color msg_color;
			if ( friends > 0  &&  enemies > 0 ) {
				// Both friends and enemies sighted -- neutral message.
				symbols["friendphrase"] = vngettext("Part of 'Units sighted! (...)' sentence^1 friendly", "$friends friendly", friends, symbols);
				symbols["enemyphrase"] = vngettext("Part of 'Units sighted! (...)' sentence^1 enemy", "$enemies enemy", enemies, symbols);
				message = vgettext("Units sighted! ($friendphrase, $enemyphrase)", symbols);
				msg_color = font::NORMAL_COLOR;
			} else if ( friends > 0 ) {
				// Only friends sighted -- good message.
				message = vngettext("Friendly unit sighted", "$friends friendly units sighted", friends, symbols);
				msg_color = font::GOOD_COLOR;
			} else if ( enemies > 0 ) {
				// Only enemies sighted -- bad message.
				message = vngettext("Enemy unit sighted!", "$enemies enemy units sighted!", enemies, symbols);
				msg_color = font::BAD_COLOR;
			}

			disp.announce(message_prefix + message, msg_color);
			message_prefix += " \n";
			redraw = true;
		}

		// Suggest "continue move"?
		if ( playing_team_is_viewing && sighted_stop_ && !resources::whiteboard->is_executing_actions() ) {
			// See if the "Continue Move" action has an associated hotkey
			const hotkey::hotkey_item& hk = hotkey::get_hotkey(hotkey::HOTKEY_CONTINUE_MOVE);
			if ( !hk.null() ) {
				utils::string_map symbols;
				symbols["hotkey"] = hk.get_name();
				std::string message = vgettext("(press $hotkey to keep moving)", symbols);
				disp.announce(message_prefix + message, font::NORMAL_COLOR);
				message_prefix += " \n";
				redraw = true;
			}
		}

		// Update the screen.
		if ( redraw )
			disp.draw();
	}

}//end anonymous namespace

/**
 * Moves a unit across the board.
 *
 * This function handles actual movement, checking terrain costs as well as
 * things that might interrupt movement (e.g. ambushes). If the full path
 * cannot be reached this turn, the remainder is stored as the unit's "goto"
 * instruction. (The unit itself is whatever unit is at the beginning of the
 * supplied path.)
 *
 * @param[in]  steps                The route to be travelled. The unit to be moved is at the beginning of this route.
 * @param[out] move_recorder        Will be given the route actually travelled (which might be shorter than the route specified) so it can be stored in the replay.
 * @param      undo_stack           If supplied, then either this movement will be added to the stack or the stack will be cleared.
 * @param[in]  continued_move       If set to true, this is a continuation of an earlier move (movement is not interrupted should units be spotted).
 * @param[in]  show_move            Controls whether or not the movement is animated for the player.
 * @param[out] interrupted          If supplied, then this is set to true if information was uncovered that warrants interrupting a chain of actions (and set to false otherwise).
 * @param[out] move_spectator       If supplied, this will be given the information uncovered by the move (and the unit's "goto" instruction will be preserved).
 * @param[in]  replay_dest          If not NULL, then this move is assumed to be a replay that expects the unit to be moved to here. Several normal considerations are ignored in a replay.
 *
 * @returns The number of hexes entered. This can safely be used as an index
 *          into @a steps to get the location where movement ended, provided
 *          @a steps is not empty (the return value is guaranteed to be less
 *          than steps.size() ).
 */
size_t move_unit(const std::vector<map_location> &steps,
                 replay* move_recorder, undo_list* undo_stack,
                 bool continued_move, bool show_move,
                 bool* interrupted,
                 move_unit_spectator* move_spectator,
                 const map_location* replay_dest)
{
	const events::command_disabler disable_commands;

	// Default return value.
	if ( interrupted )
		*interrupted = false;

	// Avoid some silliness.
	if ( steps.size() < 2  ||  (steps.size() == 2 && steps.front() == steps.back()) ) {
		DBG_NG << "Ignoring a unit trying to jump on its hex at " <<
		          ( steps.empty() ? map_location::null_location : steps.front() ) << ".\n";
		return 0;
	}

	// Evaluate this move.
	unit_mover mover(steps, move_spectator, continued_move, replay_dest);
	if ( !mover.check_expected_movement() )
		return 0;
	if ( move_recorder )
		// Record the expected movement, so that replays trigger the same events.
		// (Recorded here in case an exception occurs during movement.)
		move_recorder->add_movement(mover.expected_path());

	// Attempt moving.
	mover.try_actual_movement(show_move);
	if ( move_recorder  &&  mover.stopped_early() )
		// Record the early stop.
		move_recorder->limit_movement(mover.final_hex());

	// Bookkeeping, etc.
	mover.post_move(undo_stack);
	if ( show_move )
		mover.feedback();

	// Set return value.
	if ( interrupted )
		*interrupted = mover.interrupted();

	return mover.steps_travelled();
}


bool unit_can_move(const unit &u)
{
	const team &current_team = (*resources::teams)[u.side() - 1];

	if(!u.attacks_left() && u.movement_left()==0)
		return false;

	// Units with goto commands that have already done their gotos this turn
	// (i.e. don't have full movement left) should have red globes.
	if(u.has_moved() && u.has_goto()) {
		return false;
	}

	map_location locs[6];
	get_adjacent_tiles(u.get_location(), locs);
	for(int n = 0; n != 6; ++n) {
		if (resources::game_map->on_board(locs[n])) {
			const unit_map::const_iterator i = resources::units->find(locs[n]);
			if (i.valid() && !i->incapacitated() &&
			    current_team.is_enemy(i->side())) {
				return true;
			}

			if (u.movement_cost((*resources::game_map)[locs[n]]) <= u.movement_left()) {
				return true;
			}
		}
	}

	return false;
}

void apply_shroud_changes(undo_list &undos, int side)
{
	team &tm = (*resources::teams)[side - 1];
	// No need to do this if the team isn't using fog or shroud.
	if (!tm.uses_shroud() && !tm.uses_fog())
		return;

	game_display &disp = *resources::screen;
	unit_map &units = *resources::units;

	/*
	   This function works thusly:
	   1. run through the list of undo_actions
	   2. for each one, play back the unit's move
	   3. for each location along the route, clear any "shrouded" hexes that the unit can see
	      and record sighted events
	   4. render shroud/fog cleared.
	   5. pump all events
	   6. call clear_shroud to update the fog of war for each unit
	   7. fix up associated display stuff (done in a similar way to turn_info::undo())
	*/

	bool cleared_shroud = false;  // for further optimization
	bool sighted_event = false;

	for(undo_list::iterator un = undos.begin(); un != undos.end(); ++un) {
		LOG_NG << "Turning an undo...\n";
		//NOTE: for the moment shroud cleared during recall seems never delayed
		//Shroud update during recall can be delayed, during recruit as well
		//if we have a non-random recruit (e.g. undead)
		//if(un->is_recall() || un->is_recruit()) continue;

		// Make a temporary unit move in map and hide the original
		const unit_map::const_unit_iterator unit_itor = units.find(un->affected_unit.underlying_id());
		// check if the unit is still existing (maybe killed by an event)
		// FIXME: A wml-killed unit will not update the shroud explored before its death
		if(unit_itor == units.end())
			continue;

		// Cache the unit's current actual location for raising the sighted events.
		const map_location actual_location = unit_itor->get_location();

		std::vector<map_location> route(un->route.begin(), un->route.end());
		if ( un->recall_loc.valid() )
			route.push_back(un->recall_loc);
		std::vector<map_location>::const_iterator step;
		for(step = route.begin(); step != route.end(); ++step) {
			// Clear the shroud, and collect new seen_units
			std::set<map_location> seen_units;
			std::set<map_location> petrified_units;
			std::map<map_location, int> jamming_map;
			calculate_jamming(side, jamming_map);
			cleared_shroud |= clear_shroud_unit(*step, *unit_itor, tm, jamming_map,
			                                    NULL, &seen_units, &petrified_units);

			// Fire sighted events
			// Try to keep same order (petrified units after normal units)
			// as with move_unit for replay
			for (std::set<map_location>::iterator sight_it = seen_units.begin();
				sight_it != seen_units.end(); ++sight_it)
			{
				unit_map::const_iterator new_unit = units.find(*sight_it);
				assert(new_unit != units.end());

				game_events::raise("sighted", *sight_it, actual_location);
				sighted_event = true;
			}
			for (std::set<map_location>::iterator sight_it = petrified_units.begin();
				sight_it != petrified_units.end(); ++sight_it)
			{
				unit_map::const_iterator new_unit = units.find(*sight_it);
				assert(new_unit != units.end());

				game_events::raise("sighted", *sight_it, actual_location);
				sighted_event = true;
			}
		}
	}

	// Optimization: if nothing was cleared and there are no sighted events,
	// then there is nothing to redraw. (Technically, "nothing was cleared"
	// implies "no sighted events", but checking both is cheap.)
	if ( cleared_shroud  || sighted_event ) {
		// Update the display before pumping events.
		invalidate_after_clearing_shroud();
		disp.draw();

		if ( sighted_event  &&  game_events::pump() ) {
			// Updates in case WML changed stuff.
			disp.invalidate_unit();
			clear_shroud(side);
			disp.draw();
		}
	}
}

bool backstab_check(const map_location& attacker_loc,
		const map_location& defender_loc,
		const unit_map& units, const std::vector<team>& teams)
{
	const unit_map::const_iterator defender =
		units.find(defender_loc);
	if(defender == units.end()) return false; // No defender

	map_location adj[6];
	get_adjacent_tiles(defender_loc, adj);
	int i;
	for(i = 0; i != 6; ++i) {
		if(adj[i] == attacker_loc)
			break;
	}
	if(i >= 6) return false;  // Attack not from adjacent location

	const unit_map::const_iterator opp =
		units.find(adj[(i+3)%6]);
	if(opp == units.end()) return false; // No opposite unit
	if (opp->incapacitated()) return false;
	if (size_t(defender->side() - 1) >= teams.size() || size_t(opp->side() - 1) >= teams.size())
		return true; // If sides aren't valid teams, then they are enemies
	if (teams[defender->side() - 1].is_enemy(opp->side()))
		return true; // Defender and opposite are enemies
	return false; // Defender and opposite are friends
}
