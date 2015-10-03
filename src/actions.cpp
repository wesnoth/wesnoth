/* $Id$ */
/*
   Copyright (C) 2003 - 2011 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file src/actions.cpp
 * Recruiting, Fighting.
 */

#include "attack_prediction.hpp"
#include "foreach.hpp"
#include "game_display.hpp"
#include "game_end_exceptions.hpp"
#include "game_events.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "hotkeys.hpp"
#include "log.hpp"
#include "map.hpp"
#include "map_label.hpp"
#include "mouse_handler_base.hpp"
#include "pathfind/pathfind.hpp"
#include "replay.hpp"
#include "resources.hpp"
#include "statistics.hpp"
#include "unit_abilities.hpp"
#include "unit_display.hpp"
#include "wml_exception.hpp"
#include "formula_string_utils.hpp"
#include "tod_manager.hpp"


#include <boost/bind.hpp>
#include <boost/scoped_ptr.hpp>
#include <list>


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
	castle_cost_calculator(const gamemap& map) : map_(map)
	{}

	virtual double cost(const map_location& loc, const double) const
	{
		if(!map_.is_castle(loc))
			return 10000;

		return 1;
	}

private:
	const gamemap& map_;
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


map_location unit_creator::find_location(const config &cfg)
{

	DBG_NG << "finding location for unit with id=["<<cfg["id"]<<"] placement=["<<cfg["placement"]<<"] x=["<<cfg["x"]<<"] y=["<<cfg["y"]<<"] for side " << team_.side() << "\n";

	std::vector< std::string > placements = utils::split(cfg["placement"]);

	placements.push_back("map");
	placements.push_back("recall");

	BOOST_FOREACH(std::string place, placements) {
		map_location loc;

		if (place == "recall" ) {
			return map_location::null_location;
		}

		if (place == "leader" ) {
			unit_map::const_iterator leader = resources::units->find_leader(team_.side());
			//todo: take 'leader in recall list' possibility into account
			if (leader.valid()) {
				loc = leader->first;
			} else {
				loc = start_pos_;
			}
		}

		if (place=="map") {
			loc = map_location(cfg,resources::state_of_game);
		}

		if(loc.valid() && resources::game_map->on_board(loc)) {
		  loc = find_vacant_tile(*resources::game_map, *resources::units, loc, pathfind::VACANT_ANY);
			if(loc.valid() && resources::game_map->on_board(loc)) {
				return loc;
			}
		}
	}

	return map_location::null_location;

}


void unit_creator::add_unit(const config &cfg)
{
	config temp_cfg(cfg);
	map_location loc = find_location(temp_cfg);
	temp_cfg["side"] = str_cast(team_.side());
	temp_cfg.remove_attribute("placement");
	temp_cfg.remove_attribute("player_id");
	temp_cfg.remove_attribute("faction_from_recruit");

	const std::string& id =(cfg)["id"];

	std::vector<unit>::iterator recall_list_element = std::find_if(team_.recall_list().begin(), team_.recall_list().end(), boost::bind(&unit::matches_id, _1, id));

	if(!loc.valid()) {
		if(add_to_recall_) {
			if (recall_list_element==team_.recall_list().end()) {
				//add to recall list
				unit new_unit(resources::units, temp_cfg, true,resources::state_of_game);
				team_.recall_list().push_back(new_unit);
				DBG_NG << "inserting unit with id=["<<id<<"] on recall list for side " << new_unit.side() << "\n";
			} else {
				LOG_NG << "wanted to insert unit on recall list, but recall list for side " << (cfg)["side"] << "already contains id="<<id<<"\n";
			}
			return;
		}
	} else {
		assert( resources::units->find(loc) == resources::units->end() );
		bool animate = utils::string_bool(temp_cfg["animate"],false);
		temp_cfg.remove_attribute("animate");
		if (recall_list_element==team_.recall_list().end()) {
			//new unit
			unit new_unit(resources::units, temp_cfg, true, resources::state_of_game);
			resources::units->add(loc, new_unit);
			LOG_NG << "inserting unit for side " << new_unit.side() << "\n";
			post_create(loc,new_unit,animate);
		} else {
			//get unit from recall list
			unit recalled_unit = *recall_list_element;
			recalled_unit.set_game_context(resources::units);
			team_.recall_list().erase(recall_list_element);
			resources::units->add(loc, recalled_unit);
			LOG_NG << "inserting unit from recall list for side " << recalled_unit.side()<< " with id="<< id << "\n";
			post_create(loc,recalled_unit,animate);
		}
	}

	//if id is not empty, delete units with this ID from recall list
	if (!id.empty()) {
		std::vector<unit> &r = team_.recall_list();
		r.erase(std::remove_if(r.begin(),r.end(),boost::bind(&unit::matches_id, _1, id)), r.end());
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


bool can_recruit_on(const gamemap& map, const map_location& leader, const map_location& loc)
{
	if(!map.on_board(loc))
		return false;

	if(!map.is_castle(loc))
		return false;

	if(!map.is_keep(leader))
		return false;

	castle_cost_calculator calc(map);
	// The limit computed in the third argument is more than enough for
	// any convex castle on the map. Strictly speaking it could be
	// reduced to sqrt(map.w()**2 + map.h()**2).
	pathfind::plain_route rt = pathfind::a_star_search(leader, loc, map.w()+map.h(), &calc, map.w(), map.h());
	return !rt.steps.empty();
}

std::string find_recruit_location(int side, map_location &recruit_loc, bool need_castle)
{
	LOG_NG << "finding recruit location for side " << side << "\n";

	unit_map::const_iterator u = resources::units->begin(),
		u_end = resources::units->end(), leader = u_end, leader_keep = u_end;

	for(; u != u_end; ++u) {
		if(u->second.can_recruit() &&
				static_cast<int>(u->second.side()) == side) {
			leader = u;
			if (!need_castle || resources::game_map->is_keep(leader->first)) {
				leader_keep = leader;
				if (can_recruit_on(*resources::game_map, leader_keep->first,
						recruit_loc))
					break;
			}
		}
	}

	if (leader == u_end && (need_castle || !resources::game_map->on_board(recruit_loc))) {
		return _("You don't have a leader to recruit with.");
	}

	assert(leader != u_end || !need_castle);

	if (need_castle && leader_keep == u_end) {
		LOG_NG << "Leader not on start: leader is on " << leader->first << '\n';
		return _("You must have your leader on a keep to recruit or recall units.");
	}

	if(need_castle) {
		if (resources::units->find(recruit_loc) != resources::units->end() ||
		    !can_recruit_on(*resources::game_map, leader_keep->first, recruit_loc))
		{
			recruit_loc = map_location();
		}
	}

	if (!resources::game_map->on_board(recruit_loc)) {
		recruit_loc = find_vacant_tile(*resources::game_map, *resources::units, leader_keep->first,
					       need_castle ? pathfind::VACANT_CASTLE : pathfind::VACANT_ANY);
	} else if (resources::units->count(recruit_loc) == 1) {
	  recruit_loc = find_vacant_tile(*resources::game_map, *resources::units, recruit_loc, pathfind::VACANT_ANY);
	}

	if (!resources::game_map->on_board(recruit_loc)) {
		return _("There are no vacant castle tiles in which to recruit a unit.");
	}

	return std::string();
}

void place_recruit(const unit &u, const map_location &recruit_location,
    bool is_recall, bool show, bool full_movement,
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
		LOG_NG << "firing prerecall event\n";
		game_events::fire("prerecall",recruit_location);
	}
	else
	{
		LOG_NG << "firing prerecruit event\n";
		game_events::fire("prerecruit",recruit_location);
	}
	const unit_map::iterator new_unit_itor = resources::units->find(recruit_location);
	if (new_unit_itor != resources::units->end()) new_unit_itor->second.set_hidden(false);
	unit_map::iterator leader = resources::units->begin();
	for(; leader != resources::units->end(); ++leader)
		if(leader->second.can_recruit()
				&& static_cast<int>(leader->second.side()) == new_unit.side()
				&& resources::game_map->is_keep(leader->first)
				&& can_recruit_on(*resources::game_map, leader->first, recruit_location))
			break;
	if (show) {
		if (leader.valid()) {
			unit_display::unit_recruited(recruit_location,leader->first);
		} else {
			unit_display::unit_recruited(recruit_location);
		}
	}
	if (is_recall)
	{
		LOG_NG << "firing recall event\n";
		game_events::fire("recall",recruit_location);
	}
	else
	{
		LOG_NG << "firing recruit event\n";
		game_events::fire("recruit",recruit_location);
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
}

map_location under_leadership(const unit_map& units,
		const map_location& loc, int* bonus)
{

	const unit_map::const_iterator un = units.find(loc);
	if(un == units.end()) {
		return map_location::null_location;
	}
	unit_ability_list abil = un->second.get_abilities("leadership");
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
	const unit& attacker = attacker_ptr ? *attacker_ptr : units.find(attacker_loc)->second;
	const unit& defender = units.find(defender_loc)->second;
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
		attacker_stats_ = new unit_stats(attacker, attacker_loc, attacker_weapon,
				true, defender, defender_loc, ddef, units);
		defender_stats_ = new unit_stats(defender, defender_loc, defender_weapon, false,
				attacker, attacker_loc, adef, units);
	}
}

	battle_context::battle_context(const battle_context &other)
: attacker_stats_(NULL), defender_stats_(NULL), attacker_combatant_(NULL), defender_combatant_(NULL)
{
	*this = other;
}

battle_context::battle_context(const unit_stats &att, const unit_stats &def) :
	attacker_stats_(new unit_stats(att)),
	defender_stats_(new unit_stats(def)),
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
		attacker_stats_ = new unit_stats(*other.attacker_stats_);
		defender_stats_ = new unit_stats(*other.defender_stats_);
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
		if (def.defense_weight() > max_weight) {
			max_weight = def.defense_weight();
			unit_stats *def_stats = new unit_stats(defender, defender_loc, choices[i], false,
					attacker, attacker_loc, &att, units);
			min_rating = static_cast<int>(def_stats->num_blows * def_stats->damage *
					def_stats->chance_to_hit * def.defense_weight());

			delete def_stats;
		}
		else if (def.defense_weight() == max_weight) {
			unit_stats *def_stats = new unit_stats(defender, defender_loc, choices[i], false,
					attacker, attacker_loc, &att, units);
			int simple_rating = static_cast<int>(def_stats->num_blows * def_stats->damage *
					def_stats->chance_to_hit * def.defense_weight());

			if (simple_rating < min_rating )
				min_rating = simple_rating;
			delete def_stats;
		}
	}

	// Multiple options: simulate them, save best.
	for (i = 0; i < choices.size(); ++i) {
		const attack_type &def = defender.attacks()[choices[i]];
		unit_stats *att_stats = new unit_stats(attacker, attacker_loc, attacker_weapon,
				true, defender, defender_loc, &def, units);
		unit_stats *def_stats = new unit_stats(defender, defender_loc, choices[i], false,
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
	unit_stats *best_att_stats = NULL, *best_def_stats = NULL;
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
			attacker_stats_ = new unit_stats(attacker, attacker_loc, choices[i],
				true, defender, defender_loc, def, units);
			defender_stats_ = new unit_stats(defender, defender_loc, def_weapon, false,
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

/** @todo FIXME: better to initialize combatant initially (move into unit_stats?), just do fight() when required. */
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

battle_context::unit_stats::unit_stats(const unit &u, const map_location& u_loc,
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
			plague_type = (*plague_specials.cfgs.front().first)["type"];
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
		damage_multiplier += combat_modifier(units, u_loc, u.alignment(), u.is_fearless());

		// Leadership bonus.
		int leader_bonus = 0;
		if (under_leadership(units, u_loc, &leader_bonus).valid())
			damage_multiplier += leader_bonus;

		// Resistance modifier.
		damage_multiplier *= opp.damage_from(*weapon, !attacking, opp_loc);

		// Compute both the normal and slowed damage. For the record,
		// drain = normal damage / 2 and slow_drain = slow_damage / 2.
		damage = round_damage(base_damage, damage_multiplier, 10000);
		slow_damage = round_damage(base_damage, damage_multiplier, 20000);
		if (is_slowed)
			damage = slow_damage;

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
			num_blows = attacks_effect.get_composite_value();
			swarm_min = num_blows;
			swarm_max = num_blows;
		}
	}
}

battle_context::unit_stats::~unit_stats()
{
}

void battle_context::unit_stats::dump() const
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

	battle_context *bc_;
	const battle_context::unit_stats *a_stats_;
	const battle_context::unit_stats *d_stats_;

	int abs_n_attack_, abs_n_defend_;
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
	const int attacker_side = a_.get_unit().side();
	game_events::fire(n, game_events::entity_location(a_.loc_, a_.id_),
		game_events::entity_location(d_.loc_, d_.id_), ev_data);

	// The event could have killed either the attacker or
	// defender, so we have to make sure they still exist
	refresh_bc();
	if(!a_.valid() || !d_.valid()) {
		if (update_display_){
			recalculate_fog(attacker_side);
			recalculate_fog(defender_side);
			resources::screen->recalculate_minimap();
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
		const_cast<battle_context::unit_stats*>(a_stats_)->weapon =
			a_.valid() && a_.weapon_ >= 0
				? &a_.get_unit().attacks()[a_.weapon_] : NULL;

		const_cast<battle_context::unit_stats*>(d_stats_)->weapon =
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
	id_ = i->second.underlying_id();
}

unit &attack::unit_info::get_unit()
{
	unit_map::iterator i = units_.find(loc_);
	assert(i.valid() && i->second.underlying_id() == id_);
	return i->second;
}

bool attack::unit_info::valid()
{
	unit_map::iterator i = units_.find(loc_);
	return i.valid() && i->second.underlying_id() == id_;
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
	const battle_context::unit_stats
		*&attacker_stats = *(attacker_turn ? &a_stats_ : &d_stats_),
		*&defender_stats = *(attacker_turn ? &d_stats_ : &a_stats_);
	int &abs_n = *(attacker_turn ? &abs_n_attack_ : &abs_n_defend_);
	bool &update_fog = *(attacker_turn ? &update_def_fog_ : &update_att_fog_);

	int ran_num = get_random();
	bool hits = (ran_num % 100) < attacker.cth_;

	int damage = 0;
	if (hits) {
		damage = attacker.damage_;
		resources::state_of_game->set_variable("damage_inflicted", str_cast<int>(damage));
	}

	// Make sure that if we're serializing a game here,
	// we got the same results as the game did originally.
	const config *ran_results = get_random_results();
	if (ran_results)
	{
		int results_chance = atoi((*ran_results)["chance"].c_str());
		bool results_hits = (*ran_results)["hits"] == "yes";
		int results_damage = atoi((*ran_results)["damage"].c_str());

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
			abs_n, float_text.str(), attacker_stats->drains, "");
	}

	int drains_damage = 0;
	if (attacker_stats->drains) {
		// don't drain so much that the attacker gets more than his maximum hitpoints
		drains_damage = std::min<int>(damage / 2, attacker.get_unit().max_hitpoints() - attacker.get_unit().hitpoints());
		// don't drain more than the defenders remaining hitpoints
		drains_damage = std::min<int>(drains_damage, defender.get_unit().hitpoints() / 2);
	}

	int damage_done = std::min<int>(defender.get_unit().hitpoints(), attacker.damage_);
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
		cfg["hits"] = hits ? "yes" : "no";
		cfg["dies"] = dies ? "yes" : "no";
		cfg["unit_hit"] = "defender";
		cfg["damage"] = lexical_cast<std::string>(damage);
		cfg["chance"] = lexical_cast<std::string>(attacker.cth_);

		set_random_results(cfg);
	}
	else
	{
		bool results_dies = (*ran_results)["dies"] == "yes";
		if (results_dies != dies)
		{
			errbuf_ << "SYNC: In attack " << a_.dump() << " vs " << d_.dump()
				<< ": the data source says the "
				<< (attacker_turn ? "defender" : "attacker")
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

	if (drains_damage > 0) {
		attacker.get_unit().heal(drains_damage);
	}

	if (dies)
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
			return false;
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
			return false;
		}

		units_.erase(defender.loc_);

		if (attacker.valid() && attacker_stats->plagues)
		{
			// plague units make new units on the target hex
			LOG_NG << "trying to reanimate " << attacker_stats->plague_type << '\n';
			const unit_type *reanimator =
				unit_types.find(attacker_stats->plague_type);
			if (reanimator)
			{
				LOG_NG << "found unit type:" << reanimator->id() << '\n';
				unit newunit(&units_, reanimator,
					attacker.get_unit().side(), true, unit_race::MALE);
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

		update_fog = true;
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

	--attacker.n_attacks_;
	return true;
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
	const int attacker_side = a_.get_unit().side();
	const int defender_side = d_.get_unit().side();

	static const std::string poison_string("poison");

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
	if (update_att_fog_ && (*resources::teams)[attacker_side - 1].uses_fog())
	{
		recalculate_fog(attacker_side);
		if (update_display_) {
			resources::screen->invalidate_all();
			resources::screen->recalculate_minimap();
		}
	}
	if (update_def_fog_ && (*resources::teams)[defender_side - 1].uses_fog())
	{
		recalculate_fog(defender_side);
		if (update_display_) {
			resources::screen->invalidate_all();
			resources::screen->recalculate_minimap();
		}
	}

	if (update_minimap_ && update_display_) {
		resources::screen->recalculate_minimap();
	}

	if(a_.valid()) {
		a_.get_unit().set_standing();
		if(a_.xp_)
			a_.get_unit().get_experience(a_.xp_);
	}

	if(d_.valid()) {
		d_.get_unit().set_standing();
		if(d_.xp_)
			d_.get_unit().get_experience(d_.xp_);
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

	// We strip the village off all other sides, unless it is held by an ally
	// and we don't have a leader (and thus can't occupy it)
	for(std::vector<team>::iterator i = teams.begin(); i != teams.end(); ++i) {
		int i_side = i - teams.begin() + 1;
		if (!t || has_leader || t->is_enemy(i_side)) {
			i->lose_village(loc);
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
		return t->get_village(loc);
	}

	return false;
}

// Simple algorithm: no maximum number of patients per healer.
void reset_resting(unit_map& units, int side)
{
	for (unit_map::iterator i = units.begin(); i != units.end(); ++i) {
		if (i->second.side() == side)
			i->second.set_resting(true);
	}
}

/* Contains all the data used to display healing */
struct unit_healing_struct {
  unit *healed;
  map_location *healed_loc;
  std::vector<unit_map::iterator> healers;
  int healing;
};

void calculate_healing(int side, bool update_display)
{
	DBG_NG << "beginning of healing calculations\n";
	unit_map &units = *resources::units;

        /* list used to display healing after having computed it */
        std::list<struct unit_healing_struct> l;

	// We look for all allied units, then we see if our healer is near them.
	for (unit_map::iterator i = units.begin(); i != units.end(); ++i) {

		if (i->second.get_state("not_healable") || i->second.incapacitated())
			continue;

		DBG_NG << "found healable unit at (" << i->first << ")\n";

		unit_map::iterator curer = units.end();
		std::vector<unit_map::iterator> healers;

		int healing = 0;
		int rest_healing = 0;

		std::string curing;

		unit_ability_list heal = i->second.get_abilities("heals");

		const bool is_poisoned = i->second.get_state(unit::STATE_POISONED);
		if(is_poisoned) {
			// Remove the enemies' healers to determine if poison is slowed or cured
			for (std::vector<std::pair<const config *, map_location> >::iterator
					h_it = heal.cfgs.begin(); h_it != heal.cfgs.end();) {

				unit_map::iterator potential_healer = units.find(h_it->second);

				assert(potential_healer != units.end());
				if ((*resources::teams)[potential_healer->second.side() - 1].is_enemy(side)) {
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
					if(curer->second.side() == side) {
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
			if(potential_healer->second.side() != side) {
				h_it = heal.cfgs.erase(h_it);
			} else {
				++h_it;
			}
		}

		unit_abilities::effect heal_effect(heal,0,false);
		healing = heal_effect.get_composite_value();

		for(std::vector<unit_abilities::individual_effect>::const_iterator heal_loc = heal_effect.begin(); heal_loc != heal_effect.end(); ++heal_loc) {
			healers.push_back(units.find(heal_loc->loc));
		}

		if (!healers.empty()) {
			DBG_NG << "Unit has " << healers.size() << " potential healers\n";
		}

		if(i->second.side() == side) {
			unit_ability_list regen = i->second.get_abilities("regenerate");
			unit_abilities::effect regen_effect(regen,0,false);
			if(regen_effect.get_composite_value() > healing) {
				healing = regen_effect.get_composite_value();
				healers.clear();
			}
			if(regen.cfgs.size()) {
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
			if (int h = resources::game_map->gives_healing(i->first)) {
				if (h > healing) {
					healing = h;
					healers.clear();
				}
				/** @todo FIXME */
				curing = "cured";
				curer = units.end();
			}
			if(i->second.resting() || i->second.is_healthy()) {
				rest_healing = game_config::rest_heal_amount;
				healing += rest_healing;
			}
		}
		if(is_poisoned) {
			if(curing == "cured") {
				i->second.set_state(unit::STATE_POISONED,false);
				healing = rest_healing;
				healers.clear();
				if (curer != units.end())
					healers.push_back(curer);
			} else if(curing == "slowed") {
				healing = rest_healing;
				healers.clear();
				if (curer != units.end())
					healers.push_back(curer);
			} else {
				healers.clear();
				healing = rest_healing;
				if(i->second.side() == side) {
					healing -= game_config::poison_amount;
				}
			}
		}

		if (curing == "" && healing==0) {
			continue;
		}

		int pos_max = i->second.max_hitpoints() - i->second.hitpoints();
		int neg_max = -(i->second.hitpoints() - 1);
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
		    !(i->second.invisible(i->first, units, *resources::teams) &&
		      (*resources::teams)[resources::screen->viewing_team()].is_enemy(side)))
		{
                  struct unit_healing_struct uhs = {
                    &i->second,
                    &i->first,
                    healers,
                    healing
                  };
                  l.push_front(uhs);
		}
		if (healing > 0)
			i->second.heal(healing);
		else if (healing < 0)
			i->second.take_hit(-healing);
		resources::screen->invalidate_unit();
	}

        /* display healing with nearest first algorithm */
        if (!l.empty()) {

          /* the first unit to be healed is chosen arbitrarily */
          struct unit_healing_struct uhs = l.front();
          l.pop_front();

          unit_display::unit_healing(*uhs.healed, *uhs.healed_loc,
                                     uhs.healers, uhs.healing);

          /* next unit to be healed is nearest from uhs left in list l */
          while (!l.empty()) {

            std::list<struct unit_healing_struct>::iterator nearest;
            int min_d = INT_MAX;

            /* for each unit in l, remember nearest */
            for (std::list<struct unit_healing_struct>::iterator i = l.begin();
                 i != l.end() ; ++i) {
              //int d = uhs.square_dist(*i);
              int d = distance_between(*uhs.healed_loc, *i->healed_loc);
              if (d < min_d) {
                min_d = d;
                nearest = i;
              }
            }

            uhs = *nearest;
            l.erase(nearest);

            unit_display::unit_healing(*uhs.healed, *uhs.healed_loc,
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
	new_unit.get_experience(-new_unit.max_experience());
	new_unit.advance_to(new_type);
	return new_unit;
}

void advance_unit(map_location loc, const std::string &advance_to)
{
	unit_map::unit_iterator u = resources::units->find(loc);
	if(!u.valid()) {
		return;
	}
	// original_type is not a reference, since the unit may disappear at any moment.
	std::string original_type = u->second.type_id();
	LOG_NG << "firing advance event at " << loc <<"\n";

	game_events::fire("advance",loc);

	if(!u.valid() || u->second.experience() < u->second.max_experience()
	|| u->second.type_id() != original_type)
	{
		LOG_NG << "WML has invalidated the advancing unit, abort\n";
		return;
	}

	loc = u->first;
	unit new_unit = get_advanced_unit(u->second, advance_to);
	statistics::advance_unit(new_unit);

	preferences::encountered_units().insert(new_unit.type_id());
	LOG_CF << "Added '" << new_unit.type_id() << "' to encountered units\n";

	resources::units->replace(loc, new_unit);
	LOG_NG << "firing post_advance event at " << loc << "\n";
	game_events::fire("post_advance",loc);
}

int combat_modifier(const unit_map &units, const map_location &loc,
	unit_type::ALIGNMENT alignment, bool is_fearless)
{
	const time_of_day &tod = resources::tod_manager->time_of_day_at
		(units, loc, *resources::game_map);

	int bonus = tod.lawful_bonus;

	if(alignment == unit_type::NEUTRAL)
		bonus = 0;
	else if(alignment == unit_type::CHAOTIC)
		bonus = -bonus;
	if(is_fearless)
		bonus = std::max<int>(bonus, 0);

	return bonus;
}

namespace {

	bool clear_shroud_loc(team &tm,
			const map_location& loc,
			std::vector<map_location>* cleared)
	{
		gamemap &map = *resources::game_map;
		bool result = false;
		map_location adj[7];
		get_adjacent_tiles(loc,adj);
		adj[6] = loc;
		bool on_board_loc = map.on_board(loc);
		for(int i = 0; i != 7; ++i) {

			// We clear one past the edge of the board, so that the half-hexes
			// at the edge can also be cleared of fog/shroud.
			if (on_board_loc || map.on_board_with_border(adj[i])) {
				// Both functions should be executed so don't use || which
				// uses short-cut evaluation.
				const bool res = tm.clear_shroud(adj[i]) | tm.clear_fog(adj[i]);

				if(res) {
					if(res) {
						result = true;
						// If we're near the corner it might be the corner also needs to be cleared
						// this always happens at the lower left corner and depending on the with
						// at the upper or lower right corner.
						if(adj[i].x == 0 && adj[i].y == map.h() - 1) { // Lower left corner
							const map_location corner(-1 , map.h());
							tm.clear_shroud(corner);
							tm.clear_fog(corner);
						} else if(map.w() % 2 && adj[i].x == map.w() - 1 && adj[i].y == map.h() - 1) { // Lower right corner
							const map_location corner(map.w() , map.h());
							tm.clear_shroud(corner);
							tm.clear_fog(corner);
						} else if(!(map.w() % 2) && adj[i].x == map.w() - 1 && adj[i].y == 0) { // Upper right corner
							const map_location corner(map.w() , -1);
							tm.clear_shroud(corner);
							tm.clear_fog(corner);
						}
						if(cleared) {
							cleared->push_back(adj[i]);
						}
					}
				}
			}
		}

		return result;
	}

	/**
	 * Returns true if some shroud is cleared.
	 * seen_units will return new units that have been seen by this unit.
	 * If known_units is NULL, seen_units can be NULL and will not be changed.
	 */
	bool clear_shroud_unit(const map_location &loc, int side,
			const std::set<map_location>* known_units = NULL,
			std::set<map_location>* seen_units = NULL,
			std::set<map_location>* petrified_units = NULL)
	{
		std::vector<map_location> cleared_locations;
		team &tm = (*resources::teams)[side - 1];

		const unit_map::const_iterator u = resources::units->find(loc);
		if (!u.valid()) {
			return false;
		}

		pathfind::paths p(*resources::game_map, *resources::units, loc, *resources::teams, true, false, tm, 0, false, true);
		BOOST_FOREACH (const pathfind::paths::step &dest, p.destinations) {
			clear_shroud_loc(tm, dest.curr, &cleared_locations);
		}

		// clear_shroud_loc is supposed not introduce repetition in cleared_locations
		for(std::vector<map_location>::const_iterator it =
				cleared_locations.begin(); it != cleared_locations.end(); ++it) {

			const unit_map::const_iterator sighted = resources::units->find(*it);
			if (sighted.valid() &&
			    (!sighted->second.invisible(*it, *resources::units, *resources::teams)
			     || !tm.is_enemy(sighted->second.side())))
			{
				//check if we know this unit, but we always know ourself
				//just in case we managed to move on a fogged hex (teleport)
				if(seen_units != NULL && known_units != NULL
						&& known_units->count(*it) == 0 && *it != loc) {
					if (!(sighted->second.get_state(unit::STATE_PETRIFIED)))
					{
						seen_units->insert(*it);
					}
					else if (petrified_units != NULL)
					{
						petrified_units->insert(*it);
					}
				}
			}
		}

		return cleared_locations.empty() == false;
	}

}

void recalculate_fog(int side)
{
	team &tm = (*resources::teams)[side - 1];

	if (!tm.uses_fog())
		return;

	tm.refog();

	for (unit_map::iterator i = resources::units->begin(); i != resources::units->end(); ++i)
	{
		if (i->second.side() == side) {
			const unit_movement_resetter move_resetter(i->second);

			clear_shroud_unit(i->first, side);
		}
	}

	//FIXME: This pump don't catch any sighted events (they are not fired by
	// clear_shroud_unit) and if it caches another old event, maybe the caller
	// don't want to pump it here
	game_events::pump();
}

bool clear_shroud(int side)
{
	team &tm = (*resources::teams)[side - 1];
	if (!tm.uses_shroud() && !tm.uses_fog())
		return false;

	bool result = false;

	unit_map::iterator i;
	for (i = resources::units->begin(); i != resources::units->end(); ++i) {
		if (i->second.side() == side) {
			const unit_movement_resetter move_resetter(i->second);

			result |= clear_shroud_unit(i->first, side);
		}
	}

	//FIXME: This pump don't catch any sighted events (they are not fired by
	// clear_shroud_unit) and if it caches another old event, maybe the caller
	// don't want to pump it here
	game_events::pump();

	if (tm.uses_fog()) {
		recalculate_fog(side);
	}

	resources::screen->labels().recalculate_labels();
	resources::screen->labels().recalculate_shroud();

	return result;
}

size_t move_unit(move_unit_spectator *move_spectator,
		 const std::vector<map_location> &route,
		 replay* move_recorder, undo_list* undo_stack,
		 bool show_move,
		 map_location *next_unit, bool continue_move,
		 bool should_clear_shroud, bool is_replay)
{
	assert(route.empty() == false);

	if (route.size() <= 2 && route.front() == route.back()) {
		DBG_NG << "Ignore an unit trying to jump on its hex at " << route.front() << "\n";
	}

	// Stop the user from issuing any commands while the unit is moving
	const events::command_disabler disable_commands;

	gamemap &map = *resources::game_map;
	unit_map &units = *resources::units;
	std::vector<team> &teams = *resources::teams;
	game_display &disp = *resources::screen;

	unit_map::iterator ui = units.find(route.front());

	assert(ui != units.end());

	//don't modify goto if we're have a spectator
	//if it is present, then the caller code is responsible for modifying gotos
	if (move_spectator==NULL) {
		ui->second.set_goto(map_location());
	}

	size_t team_num = ui->second.side()-1;
	team *tm = &teams[team_num];

	const bool check_shroud = should_clear_shroud && tm->auto_shroud_updates() &&
		(tm->uses_shroud() || tm->uses_fog());

	std::set<map_location> known_units;
	if(check_shroud) {
		for(unit_map::const_iterator u = units.begin(); u != units.end(); ++u) {
			if (!tm->fogged(u->first)) {
				known_units.insert(u->first);
				tm->see(u->second.side() - 1);
			}
		}
	}

	// See how far along the given path we can move.
	const int starting_moves = ui->second.movement_left();
	int moves_left = starting_moves;
	std::vector<map_location> starting_waypoints = ui->second.waypoints();
	std::set<map_location> seen_units;
	std::set<map_location> petrified_units;
	bool discovered_unit = false;
	bool teleport_failed = false;
	bool should_clear_stack = false;
	std::vector<map_location>::const_iterator step;
	std::string ambushed_string;

	for(step = route.begin()+1; step != route.end(); ++step) {
		const bool skirmisher = ui->second.get_ability_bool("skirmisher",*step);
		const t_translation::t_terrain terrain = map[*step];

		const int cost = ui->second.movement_cost(terrain);

		//check whether a unit was sighted and whether it should interrupt move
		bool sighted_interrupts = false;
		if (continue_move == false && preferences::interrupt_when_ally_sighted() == false) {
			//check whether any sighted unit is an enemy
			for (std::set<map_location>::iterator it = seen_units.begin(); it != seen_units.end(); ++it)
			{
				const unit_map::const_iterator u = units.find(*it);

				// Unit may have been removed by an event.
				if(u == units.end()) {
					DBG_NG << "was removed\n";
					continue;
				}

				if (tm->is_enemy(u->second.side())) {
					sighted_interrupts = true;
					break;
				}
			}
		}
		else
			sighted_interrupts = seen_units.empty() == false; //interrupt if any unit was sighted

		if(cost >moves_left || discovered_unit || (continue_move == false && sighted_interrupts)) {
			if ((!is_replay) || (!skirmisher))
				break; // not enough MP or spotted new enemies
		}

		const unit_map::const_iterator enemy_unit = units.find(*step);
		if (enemy_unit != units.end()) {
			if (tm->is_enemy(enemy_unit->second.side())) {
				if (move_spectator!=NULL) {
					move_spectator->set_ambusher(enemy_unit);
				}
				// can't traverse enemy (bug in fog or pathfinding?)
				should_clear_stack = true; // assuming that this enemy was hidden somehow
				break;
			} else if (!tiles_adjacent(*(step-1),*step)) {
				// can't teleport on ally (on fogged village, with no-leader and view not-shared)
				if (move_spectator!=NULL) {
					move_spectator->set_failed_teleport(enemy_unit);
				}
				teleport_failed = true;
				should_clear_stack = true; // we have info not supposed to be shared
				break;
			}
		}

		moves_left -= cost;

		// remove passed waypoints
		std::vector<map_location>& waypoints = ui->second.waypoints();
		if(!waypoints.empty() && waypoints.front() == *step) {
			waypoints.erase(waypoints.begin());
		}

		// If we use fog or shroud, see if we have sighted an enemy unit,
		// in which case we should stop immediately.
		// Cannot use check shroud, because also need to check if delay shroud is on.
		if (should_clear_shroud && (tm->uses_shroud() || tm->uses_fog())) {
			//we don't want to interrupt our move when we are on an other unit
			//or a uncaptured village (except if it was our plan to end there)
			if (units.count(*step) == 0 &&
			    (!map.is_village(*step) || tm->owns_village(*step) || step + 1 == route.end()))
			{
				DBG_NG << "checking for units from " << (step->x+1) << "," << (step->y+1) << "\n";

				// Temporarily reset the unit's moves to full
				const unit_movement_resetter move_resetter(ui->second);

				// We have to swap out any unit that is already in the hex,
				// so we can put our unit there, then we'll swap back at the end.
				unit temp_unit(ui->second);
				const temporary_unit_placer unit_placer(units,*step,temp_unit);
				if (tm->auto_shroud_updates()) {
					should_clear_stack |= clear_shroud_unit(*step,
						ui->second.side(), &known_units, &seen_units, &petrified_units);
				} else {
					clear_shroud_unit(*step, ui->second.side(),
						&known_units, &seen_units, &petrified_units);
				}
				if(should_clear_stack) {
					disp.invalidate_all();
				}
			}
		}
		// we also refreh its side, just in case if an event change it
		team_num = ui->second.side()-1;
		tm = &teams[team_num];

		if (!skirmisher && pathfind::enemy_zoc(units, teams, *step, *tm, ui->second.side())) {
			moves_left = 0;
		}

		// Check if we have discovered an invisible enemy unit
		map_location adjacent[6];
		get_adjacent_tiles(*step,adjacent);

		for(int i = 0; i != 6; ++i) {
			// Check if we are checking ourselves
			if(adjacent[i] == ui->first)
				continue;

			const unit_map::const_iterator it = units.find(adjacent[i]);
			if (it != units.end() && tm->is_enemy(it->second.side()) &&
			    it->second.invisible(it->first, units, teams))
			{
				discovered_unit = true;
				should_clear_stack = true;
				moves_left = 0;
				if (move_spectator!=NULL) {
					move_spectator->set_ambusher(it);
				}

				unit_ability_list hides = it->second.get_abilities("hides");

				std::vector<std::pair<const config *, map_location> >::const_iterator hide_it = hides.cfgs.begin();
				// we only use the first valid alert message
				for(;hide_it != hides.cfgs.end() && ambushed_string.empty(); ++hide_it) {
					ambushed_string = (*hide_it->first)["alert"];
				}
			}
		}
	}

	// Make sure we don't tread on another unit.
	std::vector<map_location>::const_iterator begin = route.begin();

	std::vector<map_location> steps(begin,step);
	while (!steps.empty()) {
		map_location const &loc = steps.back();
		if (units.count(loc) == 0)
			break;
		moves_left += ui->second.movement_cost(map[loc]);
		steps.pop_back();
	}

	assert(steps.size() <= route.size());

	// If we can't get all the way there and have to set a go-to.
	if(steps.size() != route.size() && discovered_unit == false) {
		if(seen_units.empty() == false) {
			ui->second.set_interrupted_move(route.back());
		} else {

			//don't modify goto if we're have a spectator
			//if it is present, then the caller code is responsible for modifying gotos
			if (move_spectator==NULL) {
				ui->second.set_goto(route.back());
			}
		}
	} else {
		ui->second.set_interrupted_move(map_location());
	}

	if(steps.size() < 2) {
		return 0;
	}

	if(next_unit != NULL)
		*next_unit = steps.back();

	map_location::DIRECTION orig_dir = ui->second.facing();
	unit_display::move_unit(steps, ui->second, teams, show_move);

	// before moving the real unit, check if it may uncover invisible units
	// and, if any, invalidate their hexes to show their visibility's change
	map_location adjacent[6];
	get_adjacent_tiles(steps.back(), adjacent);
	for(int i = 0; i != 6; ++i) {
		const unit_map::const_iterator it = units.find(adjacent[i]);
		if (it != units.end() && tm->is_enemy(it->second.side()) &&
				it->second.invisible(it->first, units, teams))
			disp.invalidate(adjacent[i]);
	}

	// move the real unit
	units.move(ui->first, steps.back());
	unit::clear_status_caches();

	ui = units.find(steps.back());
	ui->second.set_movement(moves_left);
	ui->second.set_standing();

	disp.invalidate_unit_after_move(steps.front(), steps.back());

	if(move_recorder != NULL) {
		move_recorder->add_movement(steps);
	}

	bool event_mutated = false;

	int orig_village_owner = -1;
	int action_time_bonus = 0;

	if(map.is_village(steps.back())) {
		orig_village_owner = village_owner(steps.back(),teams);

		if(size_t(orig_village_owner) != team_num) {
			ui->second.set_movement(0);
			event_mutated = get_village(steps.back(), team_num + 1, &action_time_bonus);
		}
	}


	// Show the final move animation step
	disp.draw();

	if ( teams[ui->second.side()-1].uses_shroud() || teams[ui->second.side()-1].uses_fog())
	{
		std::set<map_location>::iterator sight_it;
		const std::string sighted_str("sighted");
		// Fire sighted event here
		for (sight_it = seen_units.begin();
				sight_it != seen_units.end(); ++sight_it)
		{
			game_events::raise(sighted_str,*sight_it,steps.back());
		}

		for (sight_it = petrified_units.begin();
				sight_it != petrified_units.end(); ++sight_it)
		{
			game_events::raise(sighted_str,*sight_it,steps.back());
		}
	}

	game_events::raise("moveto",steps.back(),steps.front());

	event_mutated |= game_events::pump();

	//NOTE: an wml event may have removed the unit pointed by ui
	unit_map::iterator maybe_ui = units.find(steps.back());
	// and always disable the previous iterator
	ui = units.end();

	if (move_spectator!=NULL) {
		move_spectator->set_unit(maybe_ui);
	}

	if(undo_stack != NULL) {
		if(event_mutated || should_clear_stack || maybe_ui == units.end()) {
			apply_shroud_changes(*undo_stack, team_num + 1);
			undo_stack->clear();
		} else {
			// MP_COUNTDOWN: added param
			undo_stack->push_back(
				undo_action(maybe_ui->second, steps,
						starting_waypoints, starting_moves,
						action_time_bonus, orig_village_owner, orig_dir));
		}
	}

	bool redraw = false;

	// Show messages on the screen here
	if (discovered_unit && !disp.fogged(steps.back())) {
		if (ambushed_string.empty())
			ambushed_string = _("Ambushed!");
		// We've been ambushed, display an appropriate message
		disp.announce(ambushed_string, font::BAD_COLOUR);
		redraw = true;
	}

	if(teleport_failed) {
		std::string teleport_string = _ ("Failed teleport! Exit not empty");
		disp.announce(teleport_string, font::BAD_COLOUR);
		redraw = true;
	}

	if(continue_move == false && seen_units.empty() == false) {
		// The message depends on how many units have been sighted,
		// and whether they are allies or enemies, so calculate that out here
		int nfriends = 0, nenemies = 0;
		for(std::set<map_location>::const_iterator i = seen_units.begin(); i != seen_units.end(); ++i) {
			DBG_NG << "processing unit at " << (i->x+1) << "," << (i->y+1) << "\n";
			const unit_map::const_iterator u = units.find(*i);

			// Unit may have been removed by an event.
			if(u == units.end()) {
				DBG_NG << "was removed\n";
				continue;
			}

			if (tm->is_enemy(u->second.side())) {
				++nenemies;
				if (move_spectator!=NULL) {
					move_spectator->add_seen_enemy(u);
				}
			} else {
				++nfriends;
				if (move_spectator!=NULL) {
					move_spectator->add_seen_friend(u);
				}
			}

			DBG_NG << "processed...\n";
			tm->see(u->second.side() - 1);
		}

		// The message we display is different depending on
		// whether the units sighted were enemies or friends,
		// and their respective number.
		utils::string_map symbols;
		symbols["friends"] = lexical_cast<std::string>(nfriends);
		symbols["enemies"] = lexical_cast<std::string>(nenemies);
		std::string message;
		SDL_Color msg_colour;
		if(nfriends == 0 || nenemies == 0) {
			if(nfriends > 0) {
				message = vngettext("Friendly unit sighted", "$friends friendly units sighted", nfriends, symbols);
				msg_colour = font::GOOD_COLOUR;
			} else if(nenemies > 0) {
				message = vngettext("Enemy unit sighted!", "$enemies enemy units sighted!", nenemies, symbols);
				msg_colour = font::BAD_COLOUR;
			}
		}
		else {
			symbols["friendphrase"] = vngettext("Part of 'Units sighted! (...)' sentence^1 friendly", "$friends friendly", nfriends, symbols);
			symbols["enemyphrase"] = vngettext("Part of 'Units sighted! (...)' sentence^1 enemy", "$enemies enemy", nenemies, symbols);
			message = vgettext("Units sighted! ($friendphrase, $enemyphrase)", symbols);
			msg_colour = font::NORMAL_COLOUR;
		}

		if(steps.size() < route.size()) {
			// See if the "Continue Move" action has an associated hotkey
			const hotkey::hotkey_item& hk = hotkey::get_hotkey(hotkey::HOTKEY_CONTINUE_MOVE);
			if(!hk.null()) {
				symbols["hotkey"] = hk.get_name();
				message += "\n" + vgettext("(press $hotkey to keep moving)", symbols);
			}
		}

		disp.announce(message, msg_colour);
		redraw = true;
	}

	if (redraw) {
		disp.draw();
	}
	disp.recalculate_minimap();

	assert(steps.size() <= route.size());

	return steps.size();
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
			if (i.valid()) {
				if(!i->second.incapacitated() && current_team.is_enemy(i->second.side())) {
					return true;
				}
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

	std::set<map_location> known_units;
	for(unit_map::const_iterator u = units.begin(); u != units.end(); ++u) {
		if (!tm.fogged(u->first)) {
			known_units.insert(u->first);
		}
	}

	bool cleared_shroud = false;  // for further optimization
	bool sighted_event = false;

	for(undo_list::iterator un = undos.begin(); un != undos.end(); ++un) {
		LOG_NG << "Turning an undo...\n";
		//NOTE: for the moment shroud cleared during recall seems never delayed
		if(un->is_recall() || un->is_recruit()) continue;

		// Make a temporary unit move in map and hide the original
		unit_map::iterator unit_itor = units.find(un->affected_unit.underlying_id());
		// check if the unit is still existing (maybe killed by an event)
		// FIXME: A wml-killed unit will not update the shroud explored before its death
		if(unit_itor == units.end())
			continue;

		unit temporary_unit(unit_itor->second);
		// We're not really going to mutate the unit, just temporarily
		// set its moves to maximum, but then switch them back.
		const unit_movement_resetter move_resetter(temporary_unit);

		std::vector<map_location>::const_iterator step;
		for(step = un->route.begin(); step != un->route.end(); ++step) {
			// we skip places where

			if (*step != unit_itor->first
				&& units.find(*step) != units.end())
				continue;
			// We have to swap out any unit that is already in the hex,
			// so we can put our unit there, then we'll swap back at the end.
			boost::scoped_ptr<temporary_unit_placer> unit_placer;
			if (*step != unit_itor->first)
			{
				unit_placer.reset(new temporary_unit_placer(units,*step, temporary_unit));
			}

			// In theory we don't know this clone, but
			// - he can't be in newly cleared locations
			// - clear_shroud_unit skip "self-detection"
			// so we normaly don't need to flood known_units with temporary stuff
			// known_units.insert(*step);

			// Clear the shroud, and collect new seen_units
			std::set<map_location> seen_units;
			std::set<map_location> petrified_units;
			cleared_shroud |= clear_shroud_unit(*step, side,
				&known_units,&seen_units,&petrified_units);

			// Fire sighted events
			// Try to keep same order (petrified units after normal units)
			// as with move_unit for replay
			for (std::set<map_location>::iterator sight_it = seen_units.begin();
				sight_it != seen_units.end(); ++sight_it)
			{
				unit_map::const_iterator new_unit = units.find(*sight_it);
				assert(new_unit != units.end());
				tm.see(new_unit->second.side() - 1);

				game_events::raise("sighted",*sight_it,unit_itor->first);
				sighted_event = true;
			}
			for (std::set<map_location>::iterator sight_it = petrified_units.begin();
				sight_it != petrified_units.end(); ++sight_it)
			{
				unit_map::const_iterator new_unit = units.find(*sight_it);
				assert(new_unit != units.end());
				tm.see(new_unit->second.side() - 1);

				game_events::raise("sighted",*sight_it,unit_itor->first);
				sighted_event = true;
			}
		}
	}

	// TODO: optimization: nothing cleared, so no sighted, and we can skip redraw
	// if (!cleared_shroud) return;

	// render shroud/fog cleared before pumping events
	// we don't refog yet to avoid hiding sighted stuff
	if (sighted_event) {
		disp.invalidate_unit();
		disp.invalidate_all();
		disp.recalculate_minimap();
		disp.draw();
	}

	game_events::pump();

	// refog and invalidate stuff
	disp.invalidate_unit();
	disp.invalidate_game_status();
	clear_shroud(side);
	disp.recalculate_minimap();
	disp.invalidate_all();
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
	if(opp->second.incapacitated()) return false;
	if(size_t(defender->second.side()-1) >= teams.size() ||
			size_t(opp->second.side()-1) >= teams.size())
		return true; // If sides aren't valid teams, then they are enemies
	if(teams[defender->second.side()-1].is_enemy(opp->second.side()))
		return true; // Defender and opposite are enemies
	return false; // Defender and opposite are friends
}
