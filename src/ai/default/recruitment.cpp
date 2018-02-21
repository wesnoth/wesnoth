/*
   Copyright (C) 2013 - 2018 by Felix Bauer
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
 * Recruitment Engine by flix
 * See http://wiki.wesnoth.org/AI_Recruitment
 */

#include "ai/default/recruitment.hpp"

#include "ai/actions.hpp"
#include "ai/composite/rca.hpp"
#include "ai/manager.hpp"
#include "actions/attack.hpp"
#include "attack_prediction.hpp"
#include "display.hpp"
#include "filter_context.hpp"
#include "game_board.hpp"
#include "log.hpp"
#include "map/map.hpp"
#include "map/label.hpp"
#include "pathfind/pathfind.hpp"
#include "pathutils.hpp"
#include "random.hpp"
#include "resources.hpp"
#include "team.hpp"
#include "tod_manager.hpp"
#include "units/filter.hpp"
#include "units/map.hpp"
#include "units/types.hpp"
#include "units/unit.hpp"
#include "variable.hpp"
#include "wml_exception.hpp"

#include <cmath>

static lg::log_domain log_ai_recruitment("ai/recruitment");
#define LOG_AI_RECRUITMENT LOG_STREAM(info, log_ai_recruitment)
#define ERR_AI_RECRUITMENT LOG_STREAM(err, log_ai_recruitment)

#ifdef _MSC_VER
#pragma warning(push)
// silence "inherits via dominance" warnings
#pragma warning(disable:4250)
#endif

namespace ai {

namespace default_recruitment {

namespace {
/**
 * CONSTANTS
 */

// This is used for a income estimation. We'll calculate the estimated income of this much
// future turns and decide if we'd gain gold if we start to recruit no units anymore.
const static int SAVE_GOLD_FORECAST_TURNS = 5;

// When a team has less then this much units, consider recruit-list too.
const static unsigned int UNIT_THRESHOLD = 5;

// Defines the shape of the border-zone between enemies.
// Higher values mean more important hexes.
const static double MAP_BORDER_THICKNESS = 2.0;
const static double MAP_BORDER_WIDTH = 0.2;

// This parameter can be used to shift all important hexes in one directon.
// For example if our AI should act rather defensivly we may want to set
// this value to a negative number. Then the AI will more care about hexes
// nearer to the own units.
const static int MAP_OFFENSIVE_SHIFT = 0;

// When villages are this near to imprtant hexes they count as important.
const static int MAP_VILLAGE_NEARNESS_THRESHOLD = 3;

// Radius of area around important villages.
const static int MAP_VILLAGE_SURROUNDING = 1;

// Determines the power of a raw unit comparison
// A higher power means that *very good* units will be
// stronger favored compared to just *good* units.
const static double COMBAT_SCORE_POWER = 1.;

// A cache is used to store the simulation results.
// This value determines how much the average defenses of the important hexes can differ
// until the simulation will run again.
const static double COMBAT_CACHE_TOLERANCY = 0.5;

// The old recruitment CA usually recruited too many scouts.
// To prevent this we multiply the aspect village_per_scout with this constant.
const static double VILLAGE_PER_SCOUT_MULTIPLICATOR = 2.;
}

std::string data::to_string() const {
	std::stringstream s;
	s << "---------------Content of leader data---------------\n";
	s << "For leader: " << leader->name() << "\n";
	s << "ratio_score: " << ratio_score << "\n";
	s << "recruit_count: " << recruit_count << "\n\n";
	for (const score_map::value_type& entry : scores) {
		s << std::setw(20) << entry.first <<
				" score: " << std::setw(7) << entry.second << "\n";
	}
	s << "----------------------------------------------------\n";
	return s.str();
}

recruitment::recruitment(rca_context& context, const config& cfg)
		: candidate_action(context, cfg),
		  important_hexes_(),
		  important_terrain_(),
		  own_units_in_combat_counter_(0),
		  average_local_cost_(),
		  cheapest_unit_costs_(),
		  combat_cache_(),
		  recruit_situation_change_observer_(),
		  average_lawful_bonus_(0.0),
		  recruitment_instructions_(),
		  recruitment_instructions_turn_(-1),
		  own_units_count_(),
		  total_own_units_(0),
		  scouts_wanted_(0)
{
	if (cfg["state"] == "save_gold") {
		state_ = SAVE_GOLD;
	} else if (cfg["state"] == "spend_all_gold") {
		state_ = SPEND_ALL_GOLD;
	} else {
		state_ = NORMAL;
	}
}

config recruitment::to_config() const {
	config cfg = candidate_action::to_config();
	if (state_ == SAVE_GOLD) {
		cfg["state"] = "save_gold";
	} else if (state_ == SPEND_ALL_GOLD) {
		cfg["state"] = "spend_all_gold";
	} else {
		cfg["state"] = "normal";
	}
	return cfg;
}

double recruitment::evaluate() {
	// Check if the recruitment list has changed.
	// Then cheapest_unit_costs_ is not valid anymore.
	if (recruit_situation_change_observer_.recruit_list_changed()) {
		cheapest_unit_costs_.clear();
		recruit_situation_change_observer_.set_recruit_list_changed(false);
	}

	// When evaluate() is called the first time this turn,
	// we'll retrieve the recruitment-instruction aspect.
	if (resources::tod_manager->turn() != recruitment_instructions_turn_) {
		recruitment_instructions_ = get_recruitment_instructions();
		integrate_recruitment_pattern_in_recruitment_instructions();
		recruitment_instructions_turn_ = resources::tod_manager->turn();
		LOG_AI_RECRUITMENT << "Recruitment-instructions updated:\n";
		LOG_AI_RECRUITMENT << recruitment_instructions_ << "\n";
	}

	// Check if we have something to do.
	const config* job = get_most_important_job();
	if (!job) {
		return BAD_SCORE;
	}

	const unit_map& units = resources::gameboard->units();
	const std::vector<unit_map::const_iterator> leaders = units.find_leaders(get_side());

	for (const unit_map::const_iterator& leader : leaders) {
		if (leader == resources::gameboard->units().end()) {
			return BAD_SCORE;
		}
		// Check Gold. But proceed if there is a unit with cost <= 0 (WML can do that)
		int cheapest_unit_cost = get_cheapest_unit_cost_for_leader(leader);
		if (current_team().gold() < cheapest_unit_cost && cheapest_unit_cost > 0) {
			continue;
		}

		const map_location& loc = leader->get_location();
		if (resources::gameboard->map().is_keep(loc) &&
				pathfind::find_vacant_castle(*leader) != map_location::null_location()) {
			return get_score();
		}
	}

	return BAD_SCORE;
}

void recruitment::execute() {
	LOG_AI_RECRUITMENT << "\n\n\n------------AI RECRUITMENT BEGIN---------------\n\n";
	LOG_AI_RECRUITMENT << "TURN: " << resources::tod_manager->turn() <<
			" SIDE: " << current_team().side() << "\n";

	/*
	 * Check which leaders can recruit and collect them in leader_data.
	 */

	const unit_map& units = resources::gameboard->units();
	const gamemap& map = resources::gameboard->map();
	const std::vector<unit_map::const_iterator> leaders = units.find_leaders(get_side());

	// This is the central datastructure with all score_tables in it.
	std::vector<data> leader_data;

	std::set<std::string> global_recruits;

	for (const unit_map::const_iterator& leader : leaders) {
		const map_location& keep = leader->get_location();
		if (!resources::gameboard->map().is_keep(keep)) {
			LOG_AI_RECRUITMENT << "Leader " << leader->name() << " is not on keep. \n";
			continue;
		}
		if (pathfind::find_vacant_castle(*leader) == map_location::null_location()) {
			LOG_AI_RECRUITMENT << "Leader " << leader->name() << " has no free hexes \n";
			continue;
		}
		int cheapest_unit_cost = get_cheapest_unit_cost_for_leader(leader);
		if (current_team().gold() < cheapest_unit_cost && cheapest_unit_cost > 0) {
			LOG_AI_RECRUITMENT << "Leader " << leader->name() << " recruits are too expensive. \n";
			continue;
		}

		// Leader can recruit.

		data data(leader);

		// Add team recruits.
		for (const std::string& recruit : current_team().recruits()) {
			if (!unit_types.find(recruit)) {
				lg::wml_error() << "Unit-type \"" << recruit << "\" doesn't exist.\n";
			}
			data.recruits.insert(recruit);
			data.scores[recruit] = 0.0;
			global_recruits.insert(recruit);
		}

		// Add extra recruits.
		for (const std::string& recruit : leader->recruits()) {
			if (!unit_types.find(recruit)) {
				lg::wml_error() << "Unit-type \"" << recruit << "\" doesn't exist.\n";
			}
			data.recruits.insert(recruit);
			data.scores[recruit] = 0.0;
			global_recruits.insert(recruit);
		}

		// Add recalls.
		// Recalls are treated as recruits. While recruiting
		// we'll check if we can do a recall instead of a recruitment.
		for (const unit_const_ptr & recall : current_team().recall_list()) {
			// Check if this leader is allowed to recall this unit.
			const unit_filter ufilt( vconfig(leader->recall_filter()));
			if (!ufilt(*recall, map_location::null_location())) {
				continue;
			}
			data.recruits.insert(recall->type_id());
			data.scores[recall->type_id()] = 0.0;
			global_recruits.insert(recall->type_id());
		}

		// Check if leader is in danger. (If a enemies unit can attack the leader)
		data.in_danger = power_projection(leader->get_location(), get_enemy_dstsrc()) > 0;

		// If yes, set ratio_score very high, so this leader will get priority while recruiting.
		if (data.in_danger) {
			data.ratio_score = 50;
			state_ = LEADER_IN_DANGER;
			LOG_AI_RECRUITMENT << "Leader " << leader->name() << " is in danger.\n";
		}

		leader_data.push_back(data);
	}

	if (leader_data.empty()) {
		LOG_AI_RECRUITMENT << "No leader available for recruiting. \n";
		return;  // This CA is going to be blacklisted for this turn.
	}

	if (global_recruits.empty()) {
		LOG_AI_RECRUITMENT << "All leaders have empty recruitment lists. \n";
		return;  // This CA is going to be blacklisted for this turn.
	}

	/**
	 * Find important hexes and calculate other static things.
	 */

	update_important_hexes();
	// Show "x" on important hexes if debug mode is activated AND
	// the log domain "ai/recruitment" is used.
	if (game_config::debug && !lg::info().dont_log(log_ai_recruitment)) {
		show_important_hexes();
	}

	for (const map_location& hex : important_hexes_) {
		++important_terrain_[map[hex]];
	}

	update_average_lawful_bonus();
	update_own_units_count();
	update_scouts_wanted();

	/**
	 * Fill scores.
	 */

	do_combat_analysis(&leader_data);

	LOG_AI_RECRUITMENT << "Scores before extra treatments:\n";
	for (const data& data : leader_data) {
		LOG_AI_RECRUITMENT << "\n" << data.to_string();
	}

	do_similarity_penalty(&leader_data);
	do_randomness(&leader_data);
	handle_recruitment_more(&leader_data);

	LOG_AI_RECRUITMENT << "Scores after extra treatments:\n";
	for (const data& data : leader_data) {
		LOG_AI_RECRUITMENT << "\n" << data.to_string();
	}

	/**
	 * Do recruitment according to [recruit]-tags and scores.
	 * Note that the scores don't indicate the preferred mix to recruit but rather
	 * the preferred mix of all units. So already existing units are considered.
	 */

	action_result_ptr action_result;
	config* job = nullptr;
	do {  // Recruitment loop
		recruit_situation_change_observer_.reset_gamestate_changed();

		// Check if we may want to save gold by not recruiting.
		update_state();
		int save_gold_turn = get_recruitment_save_gold()["active"].to_int(2);  // From aspect.
		int current_turn = resources::tod_manager->turn();
		bool save_gold_active = save_gold_turn > 0 && save_gold_turn <= current_turn;
		if (state_ == SAVE_GOLD && save_gold_active) {
			break;
		}

		job = get_most_important_job();
		if (!job) {
			LOG_AI_RECRUITMENT << "All recruitment jobs (recruitment_instructions) done.\n";
			break;
		}
		LOG_AI_RECRUITMENT << "Executing this job:\n" << *job << "\n";

		data* best_leader_data = get_best_leader_from_ratio_scores(leader_data, job);
		if (!best_leader_data) {
			LOG_AI_RECRUITMENT << "Leader with job (recruitment_instruction) is not on keep.\n";
			if (remove_job_if_no_blocker(job)) {
				continue;
			} else {
				break;
			}
		}
		LOG_AI_RECRUITMENT << "We want to have " << scouts_wanted_ << " more scouts.\n";

		const std::string best_recruit = get_best_recruit_from_scores(*best_leader_data, job);
		if (best_recruit.empty()) {
			LOG_AI_RECRUITMENT << "Cannot fulfill recruitment-instruction.\n";
			if (remove_job_if_no_blocker(job)) {
				continue;
			} else {
				break;
			}
		}

		LOG_AI_RECRUITMENT << "Best recruit is: " << best_recruit << "\n";
		const std::string* recall_id = get_appropriate_recall(best_recruit, *best_leader_data);
		if (recall_id) {
			LOG_AI_RECRUITMENT << "Found appropriate recall with id: " << *recall_id << "\n";
			action_result = execute_recall(*recall_id, *best_leader_data);
		} else {
			action_result = execute_recruit(best_recruit, *best_leader_data);
		}

		if (action_result->is_ok()) {
			++own_units_count_[best_recruit];
			++total_own_units_;
			if (recruit_matches_type(best_recruit, "scout")) {
				--scouts_wanted_;
			}

			// Update the current job.
			if (!job->operator[]("total").to_bool(false)) {
				job->operator[]("number") = job->operator[]("number").to_int(99999) - 1;
			}

			// Check if something changed in the recruitment list (WML can do that).
			// If yes, just return/break. evaluate() and execute() will be called again.
			if (recruit_situation_change_observer_.recruit_list_changed()) {
				break;
			}
			// Check if the gamestate changed more than once.
			// (Recruitment will trigger one gamestate change, WML could trigger more changes.)
			// If yes, just return/break. evaluate() and execute() will be called again.
			if (recruit_situation_change_observer_.gamestate_changed() > 1) {
				break;
			}

		} else {
			LOG_AI_RECRUITMENT << "Recruit result not ok.\n";
			// We'll end up here if
			// 1. We haven't enough gold,
			// 2. There aren't any free hexes around leaders,
			// 3. This leader can not recruit this type (this can happen after a recall)
		}
	} while((action_result && action_result->is_ok()) || !action_result);
	// A action_result may be uninitialized if a job was removed. Continue then anyway.

	// Recruiting is done now.
	// Update state_ for next execution().

	if (state_ == LEADER_IN_DANGER) {
		state_ = NORMAL;
	}

	int status = (action_result) ? action_result->get_status() : -1;
	bool no_gold = (status == recruit_result::E_NO_GOLD || status == recall_result::E_NO_GOLD);
	if (state_ == SPEND_ALL_GOLD && no_gold) {
		state_ = SAVE_GOLD;
	}
	if (job && no_gold) {
		remove_job_if_no_blocker(job);
	}
}

/**
 * A helper function for execute().
 */
action_result_ptr recruitment::execute_recall(const std::string& id, data& leader_data) {
	recall_result_ptr recall_result;
	recall_result = check_recall_action(id, map_location::null_location(),
			leader_data.leader->get_location());
	if (recall_result->is_ok()) {
		recall_result->execute();
		++leader_data.recruit_count;
	}
	return recall_result;
}

/**
 * A helper function for execute().
 */
action_result_ptr recruitment::execute_recruit(const std::string& type, data& leader_data) {
	recruit_result_ptr recruit_result;
	recruit_result = check_recruit_action(type, map_location::null_location(),
			leader_data.leader->get_location());

	if (recruit_result->is_ok()) {
		recruit_result->execute();
		LOG_AI_RECRUITMENT << "Recruited " << type << "\n";
		++leader_data.recruit_count;
	}
	return recruit_result;
}

/**
 * A helper function for execute().
 * Checks if this unit type can be recalled.
 * If yes, we calculate a estimated value in gold of the recall unit.
 * If this value is less then the recall cost, we dismiss the unit.
 * The unit with the highest value will be returned.
 */
const std::string* recruitment::get_appropriate_recall(const std::string& type,
		const data& leader_data) const {
	const std::string* best_recall_id = nullptr;
	double best_recall_value = -1;
	for (const unit_const_ptr & recall_unit : current_team().recall_list()) {
		if (type != recall_unit->type_id()) {
			continue;
		}
		// Check if this leader is allowed to recall this unit.
		const unit_filter ufilt(vconfig(leader_data.leader->recall_filter()));
		if (!ufilt(*recall_unit, map_location::null_location())) {
			LOG_AI_RECRUITMENT << "Refused recall because of filter: " << recall_unit->id() << "\n";
			continue;
		}
		double average_cost_of_advanced_unit = 0;
		int counter = 0;
		for (const std::string& advancement : recall_unit->advances_to()) {
			const unit_type* advancement_type = unit_types.find(advancement);
			if (!advancement_type) {
				continue;
			}
			average_cost_of_advanced_unit += advancement_type->cost();
			++counter;
		}
		if (counter > 0) {
			average_cost_of_advanced_unit /= counter;
		} else {
			// Unit don't have advancements. Use cost of unit itself.
			average_cost_of_advanced_unit = recall_unit->cost();
		}
		double xp_quantity = static_cast<double>(recall_unit->experience()) /
				recall_unit->max_experience();
		double recall_value = recall_unit->cost() + xp_quantity * average_cost_of_advanced_unit;
		if (recall_value < current_team().recall_cost()) {
			continue;  // Unit is not worth to get recalled.
		}
		if (recall_value > best_recall_value) {
			best_recall_id = &recall_unit->id();
			best_recall_value = recall_value;
		}
	}
	return best_recall_id;
}

/**
 * A helper function for execute().
 * Decides according to the leaders ratio scores which leader should recruit.
 */
data* recruitment::get_best_leader_from_ratio_scores(std::vector<data>& leader_data,
		const config* job) const {
	assert(job);
	// Find things for normalization.
	int total_recruit_count = 0;
	double ratio_score_sum = 0.0;
	for (const data& data : leader_data) {
		ratio_score_sum += data.ratio_score;
		total_recruit_count += data.recruit_count;
	}
	assert(ratio_score_sum > 0.0);

	// Shuffle leader_data to break ties randomly.
	std::shuffle(leader_data.begin(), leader_data.end(), randomness::rng::default_instance());

	// Find which leader should recruit according to ratio_scores.
	data* best_leader_data = nullptr;
	double biggest_difference = -99999.;
	for (data& data : leader_data) {
		if (!leader_matches_job(data, job)) {
			continue;
		}
		double desired_ammount = data.ratio_score / ratio_score_sum * (total_recruit_count + 1);
		double current_ammount = data.recruit_count;
		double difference = desired_ammount - current_ammount;
		if (difference > biggest_difference) {
			biggest_difference = difference;
			best_leader_data = &data;
		}
	}
	return best_leader_data;
}

/**
 * A helper function for execute().
 * Counts own units and then decides what unit should be recruited so that the
 * unit distribution approaches the given scores.
 */
const std::string recruitment::get_best_recruit_from_scores(const data& leader_data,
		const config* job) {
	assert(job);
	std::string pattern_type = get_random_pattern_type_if_exists(leader_data, job);
	if (!pattern_type.empty()) {
		LOG_AI_RECRUITMENT << "Randomly chosen pattern_type: " << pattern_type << "\n";
	}
	std::string best_recruit = "";
	double biggest_difference = -99999.;
	for (const score_map::value_type& i : leader_data.get_normalized_scores()) {
		const std::string& unit = i.first;
		const double score = i.second;

		if (!limit_ok(unit)) {
			continue;
		}
		if (!pattern_type.empty()) {
			if (!recruit_matches_type(unit, pattern_type)) {
				continue;
			}
		} else {
			if (!recruit_matches_job(unit, job)) {
				continue;
			}
		}

		double desired_ammount = score * (total_own_units_ + 1);
		double current_ammount = own_units_count_[unit];
		double difference = desired_ammount - current_ammount;
		if (scouts_wanted_ > 0 && recruit_matches_type(unit, "scout")) {
			difference += 1000.;
		}
		if (difference > biggest_difference) {
			biggest_difference = difference;
			best_recruit = unit;
		}
	}
	return best_recruit;
}

/**
 * For Map Analysis
 * Computes from our cost map and the combined cost map of all enemies the important hexes.
 */
void recruitment::compare_cost_maps_and_update_important_hexes(
		const pathfind::full_cost_map& my_cost_map,
		const pathfind::full_cost_map& enemy_cost_map) {

	const gamemap& map = resources::gameboard->map();

	// First collect all hexes where the average costs are similar in important_hexes_candidates
	// Then chose only those hexes where the average costs are relatively low.
	// This is done to remove hexes to where the teams need a similar amount of moves but
	// which are relatively far away comparing to other important hexes.
	typedef std::map<map_location, double> border_cost_map;
	border_cost_map important_hexes_candidates;
	double smallest_border_movecost = 999999;
	double biggest_border_movecost = 0;
	for(int x = 0; x < map.w(); ++x) {
		for (int y = 0; y < map.h(); ++y) {
			double my_cost_average = my_cost_map.get_average_cost_at(x, y);
			double enemy_cost_average = enemy_cost_map.get_average_cost_at(x, y);
			if (my_cost_average == -1 || enemy_cost_average == -1) {
				continue;
			}
			// We multiply the threshold MAP_BORDER_THICKNESS by the average_local_cost
			// to favor high cost hexes (a bit).
			if (std::abs(my_cost_average - MAP_OFFENSIVE_SHIFT - enemy_cost_average) <
					MAP_BORDER_THICKNESS * average_local_cost_[map_location(x, y)]) {
				double border_movecost = (my_cost_average + enemy_cost_average) / 2;

				important_hexes_candidates[map_location(x, y)] = border_movecost;

				if (border_movecost < smallest_border_movecost) {
					smallest_border_movecost = border_movecost;
				}
				if (border_movecost > biggest_border_movecost) {
					biggest_border_movecost = border_movecost;
				}
			}
		}  // for
	}  // for
	double threshold = (biggest_border_movecost - smallest_border_movecost) *
			MAP_BORDER_WIDTH + smallest_border_movecost;
	for (const border_cost_map::value_type& candidate : important_hexes_candidates) {
		if (candidate.second < threshold) {
			important_hexes_.insert(candidate.first);
		}
	}
}

/**
 * For Map Analysis.
 * Calculates for a given unit the average defense on the map.
 * (According to important_hexes_ / important_terrain_)
 */
double recruitment::get_average_defense(const std::string& u_type) const {
	const unit_type* const u_info = unit_types.find(u_type);
	if (!u_info) {
		return 0.0;
	}
	long summed_defense = 0;
	int total_terrains = 0;
	for (const terrain_count_map::value_type& entry : important_terrain_) {
		const t_translation::terrain_code& terrain = entry.first;
		int count = entry.second;
		int defense = 100 - u_info->movement_type().defense_modifier(terrain);
		summed_defense += defense * count;
		total_terrains += count;
	}
	double average_defense = (total_terrains == 0) ? 0.0 :
			static_cast<double>(summed_defense) / total_terrains;
	return average_defense;
}

/**
 * For Map Analysis.
 * Creates cost maps for a side. Each hex is map to
 * a) the summed movecost and
 * b) how many units can reach this hex
 * for all units of side.
 */
const  pathfind::full_cost_map recruitment::get_cost_map_of_side(int side) const {
	const unit_map& units = resources::gameboard->units();
	const team& team = resources::gameboard->get_team(side);

	pathfind::full_cost_map cost_map(true, true, team, true, true);

	// First add all existing units to cost_map.
	unsigned int unit_count = 0;
	for (const unit& unit : units) {
		if (unit.side() != side || unit.can_recruit() ||
				unit.incapacitated() || unit.total_movement() <= 0) {
			continue;
		}
		++unit_count;
		cost_map.add_unit(unit);
	}

	// If this side has not so many units yet, add unit_types with the leaders position as origin.
	if (unit_count < UNIT_THRESHOLD) {
		std::vector<unit_map::const_iterator> leaders = units.find_leaders(side);
		for (const unit_map::const_iterator& leader : leaders) {
			// First add team-recruits (it's fine when (team-)recruits are added multiple times).
			for (const std::string& recruit : team.recruits()) {
				cost_map.add_unit(leader->get_location(), unit_types.find(recruit), side);
			}

			// Next add extra-recruits.
			for (const std::string& recruit : leader->recruits()) {
				cost_map.add_unit(leader->get_location(), unit_types.find(recruit), side);
			}
		}
	}
	return cost_map;
}

/**
 * For Map Analysis.
 * Shows the important hexes for debugging purposes on the map. Only if debug is activated.
 */
void recruitment::show_important_hexes() const {
	if (!game_config::debug) {
		return;
	}
	display::get_singleton()->labels().clear_all();
	for (const map_location& loc : important_hexes_) {
		// Little hack: use map_location north from loc and make 2 linebreaks to center the "X".
		display::get_singleton()->labels().set_label(loc.get_direction(map_location::NORTH), "\n\nX");
	}
}

/**
 * Calculates a average lawful bonus, so Combat Analysis will work
 * better in caves and custom time of day cycles.
 */
void recruitment::update_average_lawful_bonus() {
	int sum = 0;
	int counter = 0;
	for (const time_of_day& time : resources::tod_manager->times()) {
		sum += time.lawful_bonus;
		++counter;
	}
	if (counter > 0) {
		average_lawful_bonus_ = round_double(static_cast<double>(sum) / counter);
	}
}

/**
 * For Map Analysis.
 * Creates a map where each hex is mapped to the average cost of the terrain for our units.
 */
void recruitment::update_average_local_cost() {
	average_local_cost_.clear();
	const gamemap& map = resources::gameboard->map();
	const team& team = resources::gameboard->get_team(get_side());

	for(int x = 0; x < map.w(); ++x) {
		for (int y = 0; y < map.h(); ++y) {
			map_location loc(x, y);
			int summed_cost = 0;
			int count = 0;
			for (const std::string& recruit : team.recruits()) {
				const unit_type* const unit_type = unit_types.find(recruit);
				if (!unit_type) {
					continue;
				}
				int cost = unit_type->movement_type().get_movement().cost(map[loc]);
				if (cost < 99) {
					summed_cost += cost;
					++count;
				}
			}
			average_local_cost_[loc] = (count == 0) ? 0 : static_cast<double>(summed_cost) / count;
		}
	}
}

/**
 * For Map Analysis.
 * Creates a std::set of hexes where a fight will occur with high probability.
 */
void recruitment::update_important_hexes() {
	important_hexes_.clear();
	important_terrain_.clear();
	own_units_in_combat_counter_ = 0;

	update_average_local_cost();
	const gamemap& map = resources::gameboard->map();
	const unit_map& units = resources::gameboard->units();

	// Mark battle areas as important
	// This are locations where one of my units is adjacent
	// to a enemies unit.
	for (const unit& unit : units) {
		if (unit.side() != get_side()) {
			continue;
		}
		if (is_enemy_in_radius(unit.get_location(), 1)) {
			// We found a enemy next to us. Mark our unit and all adjacent
			// hexes as important.
			std::vector<map_location> surrounding;
			get_tiles_in_radius(unit.get_location(), 1, surrounding);
			important_hexes_.insert(unit.get_location());
			std::copy(surrounding.begin(), surrounding.end(),
					std::inserter(important_hexes_, important_hexes_.begin()));
			++own_units_in_combat_counter_;
		}
	}

	// Mark area between me and enemies as important
	// This is done by creating a cost_map for each team.
	// A cost_map maps to each hex the average costs to reach this hex
	// for all units of the team.
	// The important hexes are those where my value on the cost map is
	// similar to a enemies one.
	const pathfind::full_cost_map my_cost_map = get_cost_map_of_side(get_side());
	for (const team& team : resources::gameboard->teams()) {
		if (current_team().is_enemy(team.side())) {
			const pathfind::full_cost_map enemy_cost_map = get_cost_map_of_side(team.side());

			compare_cost_maps_and_update_important_hexes(my_cost_map, enemy_cost_map);
		}
	}

	// Mark 'near' villages and area around them as important
	// To prevent a 'feedback' of important locations collect all
	// important villages first and add them and their surroundings
	// to important_hexes_ in a second step.
	std::vector<map_location> important_villages;
	for (const map_location& village : map.villages()) {
		std::vector<map_location> surrounding;
		get_tiles_in_radius(village, MAP_VILLAGE_NEARNESS_THRESHOLD, surrounding);
		for (const map_location& hex : surrounding) {
			if (important_hexes_.find(hex) != important_hexes_.end()) {
				important_villages.push_back(village);
				break;
			}
		}
	}
	for (const map_location& village : important_villages) {
		important_hexes_.insert(village);
		std::vector<map_location> surrounding;
		get_tiles_in_radius(village, MAP_VILLAGE_SURROUNDING, surrounding);
		for (const map_location& hex : surrounding) {
			// only add hex if one of our units can reach the hex
			if (map.on_board(hex) && my_cost_map.get_cost_at(hex.x, hex.y) != -1) {
				important_hexes_.insert(hex);
			}
		}
	}
}

/**
 * For Combat Analysis.
 * Calculates how good unit-type a is against unit type b.
 * If the value is bigger then 0, a is better then b.
 * If the value is 2.0 then unit-type a is twice as good as unit-type b.
 * Since this function is called very often it uses a cache.
 */
double recruitment::compare_unit_types(const std::string& a, const std::string& b) {
	const unit_type* const type_a = unit_types.find(a);
	const unit_type* const type_b = unit_types.find(b);
	if (!type_a || !type_b) {
		ERR_AI_RECRUITMENT << "Couldn't find unit type: " << ((type_a) ? b : a) << "." << std::endl;
		return 0.0;
	}
	double defense_a = get_average_defense(a);
	double defense_b = get_average_defense(b);

	const double* cache_value = get_cached_combat_value(a, b, defense_a, defense_b);
	if (cache_value) {
		return *cache_value;
	}

	double damage_to_a = 0.0;
	double damage_to_b = 0.0;

	// a attacks b
	simulate_attack(type_a, type_b, defense_a, defense_b, &damage_to_a, &damage_to_b);
	// b attacks a
	simulate_attack(type_b, type_a, defense_b, defense_a, &damage_to_b, &damage_to_a);

	int a_cost = (type_a->cost() > 0) ? type_a->cost() : 1;
	int b_cost = (type_b->cost() > 0) ? type_b->cost() : 1;
	int a_max_hp = (type_a->hitpoints() > 0) ? type_a->hitpoints() : 1;
	int b_max_hp = (type_b->hitpoints() > 0) ? type_b->hitpoints() : 1;

	double retval = 1.;
	// There are rare cases where a unit deals 0 damage (eg. Elvish Lady).
	// Then we just set the value to something reasonable.
	if (damage_to_a <= 0 && damage_to_b <= 0) {
		retval = 0.;
	} else if (damage_to_a <= 0) {
		retval = 2.;
	} else if (damage_to_b <= 0) {
		retval = -2.;
	} else {
		// Normal case
		double value_of_a = damage_to_b / (b_max_hp * a_cost);
		double value_of_b = damage_to_a / (a_max_hp * b_cost);

		if (value_of_a > value_of_b) {
			return value_of_a / value_of_b;
		} else if (value_of_a < value_of_b) {
			return -value_of_b / value_of_a;
		} else {
			return 0.;
		}
	}

	// Insert in cache.
	const cached_combat_value entry(defense_a, defense_b, retval);
	std::set<cached_combat_value>& cache = combat_cache_[a][b];
	cache.insert(entry);

	return retval;
}

/**
 * Combat Analysis.
 * Main function.
 * Compares all enemy units with all of our possible recruits and fills
 * the scores.
 */
void recruitment::do_combat_analysis(std::vector<data>* leader_data) {
	const unit_map& units = resources::gameboard->units();

	// Collect all enemy units (and their hp) we want to take into account in enemy_units.
	typedef std::vector<std::pair<std::string, int>> unit_hp_vector;
	unit_hp_vector enemy_units;
	for (const unit& unit : units) {
		if (!current_team().is_enemy(unit.side()) || unit.incapacitated()) {
			continue;
		}
		enemy_units.emplace_back(unit.type_id(), unit.hitpoints());
	}
	if (enemy_units.size() < UNIT_THRESHOLD) {
		// Use also enemies recruitment lists and insert units into enemy_units.
		for (const team& team : resources::gameboard->teams()) {
			if (!current_team().is_enemy(team.side())) {
				continue;
			}
			std::set<std::string> possible_recruits;
			// Add team recruits.
			possible_recruits.insert(team.recruits().begin(), team.recruits().end());
			// Add extra recruits.
			const std::vector<unit_map::const_iterator> leaders = units.find_leaders(team.side());
			for (unit_map::const_iterator leader : leaders) {
				possible_recruits.insert(leader->recruits().begin(), leader->recruits().end());
			}
			// Insert set in enemy_units.
			for (const std::string& possible_recruit : possible_recruits) {
				const unit_type* recruit_type = unit_types.find(possible_recruit);
				if (recruit_type) {
					int hp = recruit_type->hitpoints();
					enemy_units.emplace_back(possible_recruit, hp);
				}
			}
		}
	}

	for (data& leader : *leader_data) {
		if (leader.recruits.empty()) {
			continue;
		}
		typedef std::map<std::string, double> simple_score_map;
		simple_score_map temp_scores;

		for (const unit_hp_vector::value_type& entry : enemy_units) {
			const std::string& enemy_unit = entry.first;
			int enemy_unit_hp = entry.second;
			for (const std::string& recruit : leader.recruits) {
				double score = compare_unit_types(recruit, enemy_unit);
				score *= enemy_unit_hp;
				score = pow(score, COMBAT_SCORE_POWER);
				temp_scores[recruit] += score;
			}
		}

		if (temp_scores.empty()) {
			return;
		}
		// Find things for normalization.
		double max = -99999.;
		double sum = 0;
		for (const simple_score_map::value_type& entry : temp_scores) {
			double score = entry.second;
			if (score > max) {
				max = score;
			}
			sum += score;
		}
		double average = sum / temp_scores.size();

		// What we do now is a linear transformation.
		// We want to map the scores in temp_scores to something between 0 and 100.
		// The max score shall always be 100.
		// The min score depends on the aspect "recruitment_diversity".
		double new_100 = max;
		double score_threshold = get_recruitment_diversity();
		if (score_threshold <= 0) {
			score_threshold = 0.0001;
		}
		double new_0 = max - (score_threshold * (max - average));
		if (new_100 == new_0) {
			// This can happen if max == average. (E.g. only one possible recruit)
			new_0 -= 0.000001;
		}

		for (const simple_score_map::value_type& entry : temp_scores) {
			const std::string& recruit = entry.first;
			double score = entry.second;

			// Here we transform.
			// (If score <= new_0 then normalized_score will be 0)
			// (If score = new_100 then normalized_score will be 100)
			double normalized_score = 100 * ((score - new_0) / (new_100 - new_0));
			if (normalized_score < 0) {
				normalized_score = 0;
			}
			leader.scores[recruit] += normalized_score;
		}
	}  // for all leaders
}

/**
 * For Combat Analysis.
 * Returns the cached combat value for two unit types
 * or nullptr if there is none or terrain defenses are not within range.
 */
const double* recruitment::get_cached_combat_value(const std::string& a, const std::string& b,
		double a_defense, double b_defense) {
	double best_distance = 999;
	const double* best_value = nullptr;
	const std::set<cached_combat_value>& cache = combat_cache_[a][b];
	for (const cached_combat_value& entry : cache) {
		double distance_a = std::abs(entry.a_defense - a_defense);
		double distance_b = std::abs(entry.b_defense - b_defense);
		if (distance_a <= COMBAT_CACHE_TOLERANCY && distance_b <= COMBAT_CACHE_TOLERANCY) {
			if(distance_a + distance_b <= best_distance) {
				best_distance = distance_a + distance_b;
				best_value = &entry.value;
			}
		}
	}
	return best_value;
}

/**
 * For Combat Analysis.
 * This struct encapsulates all information for one attack simulation.
 * One attack simulation is defined by the unit-types, the weapons and the units defenses.
 */
struct attack_simulation {
	const unit_type* attacker_type;
	const unit_type* defender_type;
	const battle_context_unit_stats attacker_stats;
	const battle_context_unit_stats defender_stats;
	combatant attacker_combatant;
	combatant defender_combatant;

	attack_simulation(const unit_type* attacker, const unit_type* defender,
			double attacker_defense, double defender_defense,
			const_attack_ptr att_weapon, const_attack_ptr def_weapon,
			int average_lawful_bonus) :
			attacker_type(attacker),
			defender_type(defender),
			attacker_stats(attacker, att_weapon, true, defender, def_weapon,
					round_double(defender_defense), average_lawful_bonus),
			defender_stats(defender, def_weapon, false, attacker, att_weapon,
					round_double(attacker_defense), average_lawful_bonus),
			attacker_combatant(attacker_stats),
			defender_combatant(defender_stats)
	{
		attacker_combatant.fight(defender_combatant);
	}

	bool better_result(const attack_simulation* other, bool for_defender) {
		assert(other);
		if (for_defender) {
			return battle_context::better_combat(
					defender_combatant, attacker_combatant,
					other->defender_combatant, other->attacker_combatant, 0);
		} else {
			return battle_context::better_combat(
					attacker_combatant, defender_combatant,
					other->attacker_combatant, other->defender_combatant, 0);
		}
	}

	double get_avg_hp_of_defender() const {
		return get_avg_hp_of_combatant(false);
	}

	double get_avg_hp_of_attacker() const {
		return get_avg_hp_of_combatant(true);
	}
	double get_avg_hp_of_combatant(bool attacker) const {
		const combatant& combatant = (attacker) ? attacker_combatant : defender_combatant;
		const unit_type* unit_type = (attacker) ? attacker_type : defender_type;
		double avg_hp = combatant.average_hp(0);

		// handle poisson
		avg_hp -= combatant.poisoned * game_config::poison_amount;

		avg_hp = std::max(0., avg_hp);
		avg_hp = std::min(static_cast<double>(unit_type->hitpoints()), avg_hp);
		return avg_hp;
	}
};

/**
 * For Combat Analysis.
 * Simulates a attack with a attacker and a defender.
 * The function will use battle_context::better_combat() to decide which weapon to use.
 */
void recruitment::simulate_attack(
			const unit_type* const attacker, const unit_type* const defender,
			double attacker_defense, double defender_defense,
			double* damage_to_attacker, double* damage_to_defender) const {
	if(!attacker || !defender || !damage_to_attacker || !damage_to_defender) {
		ERR_AI_RECRUITMENT << "nullptr pointer in simulate_attack()" << std::endl;
		return;
	}
	const_attack_itors attacker_weapons = attacker->attacks();
	const_attack_itors defender_weapons = defender->attacks();

	std::shared_ptr<attack_simulation> best_att_attack;

	// Let attacker choose weapon
	for (const attack_type& att_weapon : attacker_weapons) {
		std::shared_ptr<attack_simulation> best_def_response;
		// Let defender choose weapon
		for (const attack_type& def_weapon : defender_weapons) {
			if (att_weapon.range() != def_weapon.range()) {
				continue;
			}
			std::shared_ptr<attack_simulation> simulation(new attack_simulation(
					attacker, defender,
					attacker_defense, defender_defense,
					att_weapon.shared_from_this(), def_weapon.shared_from_this(), average_lawful_bonus_));
			if (!best_def_response || simulation->better_result(best_def_response.get(), true)) {
				best_def_response = simulation;
			}
		}  // for defender weapons

		if (!best_def_response) {
			// Defender can not fight back. Simulate this as well.
			best_def_response.reset(new attack_simulation(
					attacker, defender,
					attacker_defense, defender_defense,
					att_weapon.shared_from_this(), nullptr, average_lawful_bonus_));
		}
		if (!best_att_attack || best_def_response->better_result(best_att_attack.get(), false)) {
			best_att_attack = best_def_response;
		}
	}  // for attacker weapons

	if (!best_att_attack) {
		return;
	}

	*damage_to_defender += (defender->hitpoints() - best_att_attack->get_avg_hp_of_defender());
	*damage_to_attacker += (attacker->hitpoints() - best_att_attack->get_avg_hp_of_attacker());
}

/**
 * For Configuration / Aspect "recruitment-instructions"
 * We call a [recruit] tag a "job".
 */
config* recruitment::get_most_important_job() {
	config* most_important_job = nullptr;
	int most_important_importance = -1;
	int biggest_number = -1;
	for (config& job : recruitment_instructions_.child_range("recruit")) {
		if (job.empty()) {
			continue;
		}
		int importance = job["importance"].to_int(1);
		int number = job["number"].to_int(99999);
		bool total = job["total"].to_bool(false);
		if (total) {
			// If the total flag is set we have to subtract
			// all existing units which matches the type.
			update_own_units_count();
			for (const count_map::value_type& entry : own_units_count_) {
				const std::string& unit_type = entry.first;
				const int count = entry.second;
				if (recruit_matches_job(unit_type, &job)) {
					number = number - count;
				}
			}
		}
		if (number <= 0) {
			continue;
		}
		if (importance > most_important_importance ||
				(importance == most_important_importance && biggest_number > number)) {
			most_important_job = &job;
			most_important_importance = importance;
			biggest_number = number;
		}
	}
	return most_important_job;
}

/**
 * For Configuration / Aspect "recruitment-instructions"
 * If the flag pattern is set, this method returns a random element of the
 * type-attribute.
 */
const std::string recruitment::get_random_pattern_type_if_exists(const data& leader_data,
		const config* job) const {
	std::string choosen_type;
	if (job->operator[]("pattern").to_bool(false)) {
		std::vector<std::string> job_types = utils::split(job->operator[]("type"));

		if (job_types.empty()) {
			// Empty type attribute means random recruiting.
			// Fill job_types with recruitment list.
			std::copy(leader_data.recruits.begin(), leader_data.recruits.end(),
					std::back_inserter(job_types));
		}

		// Before we choose a random pattern type, we make sure that at least one recruit
		// matches the types and doesn't exceed the [limit].
		// We do this by erasing elements of job_types.
		std::vector<std::string>::iterator job_types_it = job_types.begin();

		// Iteration through all elements.
		while (job_types_it != job_types.end()) {
			bool type_ok = false;
			for (const std::string& recruit : leader_data.recruits) {
				if (recruit_matches_type(recruit, *job_types_it) && limit_ok(recruit)) {
					type_ok = true;
					break;
				}
			}
			if (type_ok) {
				++job_types_it;
			} else {
				// Erase Element. erase() will return iterator of next element.
				LOG_AI_RECRUITMENT << "Erase type " << *job_types_it << " from pattern.\n";
				job_types_it = job_types.erase(job_types_it);
			}
		}

		if (!job_types.empty()) {
			// Choose a random job_type.
			choosen_type = job_types[randomness::generator->get_random_int(0, job_types.size()-1)];
		}
	}
	return choosen_type;
}

/**
 * For Configuration / Aspect "recruitment_pattern"
 * Converts the (old) recruitment_pattern into a recruitment_instruction (job).
 */
void recruitment::integrate_recruitment_pattern_in_recruitment_instructions() {
	const std::vector<std::string> recruitment_pattern = get_recruitment_pattern();
	if (recruitment_pattern.empty()) {
		return;
	}
	// Create a job (recruitment_instruction).
	config job;
	std::stringstream s;
	for (std::vector<std::string>::const_iterator type = recruitment_pattern.begin();
			type != recruitment_pattern.end(); ++type) {
		s << *type;
		if (type != recruitment_pattern.end() - 1) {  // Avoid trailing comma.
			s  << ", ";
		}
	}
	job["type"] = s.str();
	job["number"] = 99999;
	job["pattern"] = true;
	job["blocker"] = true;
	job["total"] = false;
	job["importance"] = 1;
	recruitment_instructions_.add_child("recruit", job);
}

/**
 * For Configuration / Aspect "recruitment-instructions"
 * Checks if a given leader is specified in the "leader_id" attribute.
 */
bool recruitment::leader_matches_job(const data& leader_data, const config* job) const {
	assert(job);
	// First we make sure that this leader can recruit
	// at least one unit-type specified in the job.
	bool is_ok = false;
	for (const std::string& recruit : leader_data.recruits) {
		if (recruit_matches_job(recruit, job) && limit_ok(recruit)) {
			is_ok = true;
			break;
		}
	}
	if (!is_ok) {
		return false;
	}

	std::vector<std::string> ids = utils::split(job->operator[]("leader_id"));
	if (ids.empty()) {
		// If no leader is specified, all leaders are okay.
		return true;
	}
	return (std::find(ids.begin(), ids.end(), leader_data.leader->id()) != ids.end());
}

/**
 * For Configuration / Aspect "recruitment-instructions"
 * Checks if a recruit-type can be recruited according to the [limit]-tag.
 */
bool recruitment::limit_ok(const std::string& recruit) const {
	// We don't use the member recruitment_instruction_ but instead
	// retrieve the aspect again. So the [limit]s can be altered during a turn.
	const config aspect = get_recruitment_instructions();

	for (const config& limit : aspect.child_range("limit")) {
		std::vector<std::string> types = utils::split(limit["type"]);
		// First check if the recruit matches one of the types.
		if (recruit_matches_types(recruit, types)) {
			// Count all own existing units which matches the type.
			int count = 0;
			for (const count_map::value_type& entry : own_units_count_) {
				const std::string& unit = entry.first;
				int number = entry.second;
				if (recruit_matches_types(unit, types)) {
					count += number;
				}
			}
			// Check if we reached the limit.
			if (count >= limit["max"].to_int(0)) {
				return false;
			}
		}
	}
	return true;
}

/**
 * For Configuration / Aspect "recruitment-instructions"
 * Checks if a given recruit-type is specified in the "type" attribute.
 */
bool recruitment::recruit_matches_job(const std::string& recruit, const config* job) const {
	assert(job);
	std::vector<std::string> job_types = utils::split(job->operator[]("type"));
	return recruit_matches_types(recruit, job_types);
}

/**
 * For Configuration / Aspect "recruitment-instructions"
 * Checks if a given recruit-type matches one atomic "type" attribute.
 */
bool recruitment::recruit_matches_type(const std::string& recruit, const std::string& type) const {
	const unit_type* recruit_type = unit_types.find(recruit);
	if (!recruit_type) {
		return false;
	}
	// Consider type-name.
	if (recruit_type->id() == type) {
		return true;
	}
	// Consider usage.
	if (recruit_type->usage() == type) {
		return true;
	}
	// Consider level.
	std::stringstream s;
	s << recruit_type->level();
	if (s.str() == type) {
		return true;
	}
	return false;
}

/**
 * For Configuration / Aspect "recruitment-instructions"
 * Checks if a given recruit-type matches one of the given types.
 */
bool recruitment::recruit_matches_types(const std::string& recruit,
		const std::vector<std::string>& types) const {
	// If no type is specified, all recruits are okay.
	if (types.empty()) {
		return true;
	}
	for (const std::string& type : types) {
		if (recruit_matches_type(recruit, type)) {
			return true;
		}
	}
	return false;
}

/**
 * For Configuration / Aspect "recruitment-instructions"
 */
bool recruitment::remove_job_if_no_blocker(config* job) {
	assert(job);
	if ((*job)["blocker"].to_bool(true)) {
		LOG_AI_RECRUITMENT << "Canceling job.\n";
		job->clear();
		return true;
	} else {
		LOG_AI_RECRUITMENT << "Aborting recruitment.\n";
		return false;
	}
}

/**
 * For Aspect "recruitment_save_gold".
 * Guess the income over the next turns.
 * This doesn't need to be exact. In the end we are just interested if this value is
 * positive or negative.
 */
double recruitment::get_estimated_income(int turns) const {
	const team& team = resources::gameboard->get_team(get_side());
	const size_t own_villages = team.villages().size();
	const double village_gain = get_estimated_village_gain();
	const double unit_gain = get_estimated_unit_gain();

	double total_income = 0;
	for (int i = 1; i <= turns; ++i) {
		double income = (own_villages + village_gain * i) * game_config::village_income;
		double upkeep = resources::gameboard->side_upkeep(get_side()) + unit_gain * i -
				(own_villages + village_gain * i) * game_config::village_support;
		double resulting_income = team.base_income() + income - std::max(0., upkeep);
		total_income += resulting_income;
	}
	return total_income;
}

/**
 * For Aspect "recruitment_save_gold".
 * Guess how many units we will gain / loose over the next turns per turn.
 */
double recruitment::get_estimated_unit_gain() const {
	return - own_units_in_combat_counter_ / 3.;
}

/**
 * For Aspect "recruitment_save_gold".
 * Guess how many villages we will gain over the next turns per turn.
 */
double recruitment::get_estimated_village_gain() const {
	const gamemap& map = resources::gameboard->map();
	int neutral_villages = 0;
	for (const map_location& village : map.villages()) {
		if (resources::gameboard->village_owner(village) == -1) {
			++neutral_villages;
		}
	}
	return (neutral_villages / resources::gameboard->teams().size()) / 4.;
}

/**
 * For Aspect "recruitment_save_gold".
 * Returns our_total_unit_costs / enemy_total_unit_costs.
 */
double recruitment::get_unit_ratio() const {
	const unit_map& units = resources::gameboard->units();
	double own_total_value = 0.;
	double team_total_value = 0.;
	double enemy_total_value = 0.;
	for (const unit& unit : units) {
		if (unit.incapacitated() || unit.total_movement() <= 0 || unit.can_recruit()) {
			continue;
		}
		double value = unit.cost() *
			static_cast<double>(unit.hitpoints()) / static_cast<double>(unit.max_hitpoints());
		if (current_team().is_enemy(unit.side())) {
			enemy_total_value += value;
		} else {
			team_total_value += value;
			if (unit.side() == current_team().side()) {
				own_total_value += value;
			}
		}
	}
	int allies_count = 0;
	for (const team& team : resources::gameboard->teams()) {
		if (!current_team().is_enemy(team.side())) {
			++allies_count;
		}
	}
	// If only the leader is left, the values could be 0.
	// Catch those cases and return something reasonable.
	if ((own_total_value == 0. || team_total_value == 0) && enemy_total_value == 0.) {
		return 0.;  // do recruit
	} else if (enemy_total_value == 0.) {
		return 999.;  // save money
	}

	// We calculate two ratios: One for the team and one for just our self.
	// Then we return the minimum.
	// This prevents cases where side1 will recruit until the save_gold begin threshold
	// is reached, and side2 won't recruit anything. (assuming side1 and side2 are allied)
	double own_ratio = (own_total_value / enemy_total_value) * allies_count;
	double team_ratio = team_total_value / enemy_total_value;
	return std::min<double>(own_ratio, team_ratio);
}

/**
 * For Aspect "recruitment_save_gold".
 * Main method.
 */
void recruitment::update_state() {
	if (state_ == LEADER_IN_DANGER || state_ == SPEND_ALL_GOLD) {
		return;
	}
	// Retrieve from aspect.
	int spend_all_gold = get_recruitment_save_gold()["spend_all_gold"].to_int(-1);
	if (spend_all_gold > 0 && current_team().gold() >= spend_all_gold) {
		state_ = SPEND_ALL_GOLD;
		LOG_AI_RECRUITMENT << "Changed state_ to SPEND_ALL_GOLD. \n";
		return;
	}
	double ratio = get_unit_ratio();
	double income_estimation = 1.;
	if (!get_recruitment_save_gold()["save_on_negative_income"].to_bool(false)) {
		income_estimation = get_estimated_income(SAVE_GOLD_FORECAST_TURNS);
	}
	LOG_AI_RECRUITMENT << "Ratio is " << ratio << "\n";
	LOG_AI_RECRUITMENT << "Estimated income is " << income_estimation << "\n";

	// Retrieve from aspect.
	double save_gold_begin = get_recruitment_save_gold()["begin"].to_double(1.5);
	double save_gold_end = get_recruitment_save_gold()["end"].to_double(1.1);

	if (state_ == NORMAL && ratio > save_gold_begin && income_estimation > 0) {
		state_ = SAVE_GOLD;
		LOG_AI_RECRUITMENT << "Changed state to SAVE_GOLD.\n";
	} else if (state_ == SAVE_GOLD && ratio < save_gold_end) {
		state_ = NORMAL;
		LOG_AI_RECRUITMENT << "Changed state to NORMAL.\n";
	}
}

/**
 * Will add a random value between 0 and "recruitment_randomness"
 * to all recruits
 */
void recruitment::do_randomness(std::vector<data>* leader_data) const {
	if (!leader_data) {
		return;
	}
	for (data& data : *leader_data) {
		for (score_map::value_type& entry : data.scores) {
			double& score = entry.second;
			score += randomness::generator->get_random_double() * get_recruitment_randomness();
		}
	}
}

/**
 * Will give a penalty to similar units. Similar units are units in one advancement tree.
 * Example (Archer can advance to Ranger):
 *                 before    after
 * Elvish Fighter:   50        50
 * Elvish Archer:    50        25
 * Elvish Ranger:    50        25
 */
void recruitment::do_similarity_penalty(std::vector<data>* leader_data) const {
	if (!leader_data) {
		return;
	}
	for (data& data : *leader_data) {
		// First we count how many similarities each recruit have to other ones (in a map).
		// Some examples:
		// If unit A and unit B have nothing to do with each other, they have similarity = 0.
		// If A advances to B both have similarity = 1.
		// If A advances to B and B to C, A, B and C have similarity = 2.
		// If A advances to B or C, A have similarity = 2. B and C have similarity = 1.
		typedef std::map<std::string, int> similarity_map;
		similarity_map similarities;
		for (const score_map::value_type& entry : data.scores) {
			const std::string& recruit = entry.first;
			const unit_type* recruit_type = unit_types.find(recruit);
			if (!recruit_type) {
				continue;
			}
			for (const std::string& advanced_type : recruit_type->advancement_tree()) {
				if (data.scores.count(advanced_type) != 0) {
					++similarities[recruit];
					++similarities[advanced_type];
				}
			}
		}
		// Now we divide each score by similarity + 1.
		for (score_map::value_type& entry : data.scores) {
			const std::string& recruit = entry.first;
			double& score = entry.second;
			score /= (similarities[recruit] + 1);
		}
	}
}

/**
 * Called at the beginning and whenever the recruitment list changes.
 */
int recruitment::get_cheapest_unit_cost_for_leader(const unit_map::const_iterator& leader) {
	std::map<size_t, int>::const_iterator it = cheapest_unit_costs_.find(leader->underlying_id());
	if (it != cheapest_unit_costs_.end()) {
		return it->second;
	}

	int cheapest_cost = 999999;

	// team recruits
	for (const std::string& recruit : current_team().recruits()) {
		const unit_type* const info = unit_types.find(recruit);
		if (!info) {
			continue;
		}
		if (info->cost() < cheapest_cost) {
			cheapest_cost = info->cost();
		}
	}
	// extra recruits
	for (const std::string& recruit : leader->recruits()) {
		const unit_type* const info = unit_types.find(recruit);
		if (!info) {
			continue;
		}
		if (info->cost() < cheapest_cost) {
			cheapest_cost = info->cost();
		}
	}
	// Consider recall costs.
	if (!current_team().recall_list().empty() && current_team().recall_cost() < cheapest_cost) {
		cheapest_cost = current_team().recall_cost();
	}
	LOG_AI_RECRUITMENT << "Cheapest unit cost updated to " << cheapest_cost << ".\n";
	cheapest_unit_costs_[leader->underlying_id()] = cheapest_cost;
	return cheapest_cost;
}

/**
 * For Aspect "recruitment_more"
 */
void recruitment::handle_recruitment_more(std::vector<data>* leader_data) const {
	if (!leader_data) {
		return;
	}
	const std::vector<std::string> aspect = get_recruitment_more();
	for (const std::string& type : aspect) {
		for (data& data : *leader_data) {
			for (score_map::value_type& entry : data.scores) {
				const std::string& recruit = entry.first;
				double& score = entry.second;
				if (recruit_matches_type(recruit, type)) {
					score += 25.;
				}
			}
		}
	}
}

/**
 * Helper function.
 * Returns true if there is a enemy within the radius.
 */
bool recruitment::is_enemy_in_radius(const map_location& loc, int radius) const {
	const unit_map& units = resources::gameboard->units();
	std::vector<map_location> surrounding;
	get_tiles_in_radius(loc, radius, surrounding);
	if (surrounding.empty()) {
		return false;
	}
	for (const map_location& l : surrounding) {
		const unit_map::const_iterator& enemy_it = units.find(l);
		if(enemy_it == units.end()) {
			continue;
		}
		if (!current_team().is_enemy(enemy_it->side()) || enemy_it->incapacitated()) {
			continue;
		}
		return true;
	}
	return false;
}

/*
 * Helper Function.
 * Counts own units on the map and saves the result
 * in member own_units_count_
 */
void recruitment::update_own_units_count() {
	own_units_count_.clear();
	total_own_units_ = 0;
	const unit_map& units = resources::gameboard->units();
	for (const unit& unit : units) {
		if (unit.side() != get_side() || unit.can_recruit() ||
				unit.incapacitated() || unit.total_movement() <= 0) {
			continue;
		}
		++own_units_count_[unit.type_id()];
		++total_own_units_;
	}
}

/**
 * This function will use the aspect villages_per_scout to decide how many
 * scouts we want to recruit.
 */
void recruitment::update_scouts_wanted() {
	scouts_wanted_ = 0;
	if (get_villages_per_scout() == 0) {
		return;
	}
	int neutral_villages = 0;
	// We recruit the initial allocation of scouts
	// based on how many neutral villages there are.
	for (const map_location& village : resources::gameboard->map().villages()) {
		if (resources::gameboard->village_owner(village) == -1) {
			++neutral_villages;
		}
	}
	double our_share = static_cast<double>(neutral_villages) / resources::gameboard->teams().size();

	// The villages per scout is for a two-side battle,
	// accounting for all neutral villages on the map.
	// We only look at our share of villages, so we halve it,
	// making us get twice as many scouts.
	double villages_per_scout = (VILLAGE_PER_SCOUT_MULTIPLICATOR * get_villages_per_scout()) / 2;

	scouts_wanted_ = (villages_per_scout > 0) ? round_double(our_share / villages_per_scout) : 0;

	if (scouts_wanted_ == 0) {
		return;
	}

	// Subtract already recruited scouts.
	for (const count_map::value_type& entry : own_units_count_) {
		const std::string& unit_type = entry.first;
		const int count = entry.second;
		if (recruit_matches_type(unit_type, "scout")) {
			scouts_wanted_ -= count;
		}
	}
}

/**
 * Observer Code
 */
recruitment::recruit_situation_change_observer::recruit_situation_change_observer()
	: recruit_list_changed_(false), gamestate_changed_(0) {
	manager::get_singleton().add_recruit_list_changed_observer(this);
	manager::get_singleton().add_gamestate_observer(this);
}

void recruitment::recruit_situation_change_observer::handle_generic_event(
		const std::string& event) {
	if (event == "ai_recruit_list_changed") {
		LOG_AI_RECRUITMENT << "Recruitment List is not valid anymore.\n";
		set_recruit_list_changed(true);
	} else {
		++gamestate_changed_;
	}
}

recruitment::recruit_situation_change_observer::~recruit_situation_change_observer() {
	manager::get_singleton().remove_recruit_list_changed_observer(this);
	manager::get_singleton().remove_gamestate_observer(this);
}

bool recruitment::recruit_situation_change_observer::recruit_list_changed() {
	return recruit_list_changed_;
}

void recruitment::recruit_situation_change_observer::set_recruit_list_changed(bool changed) {
	recruit_list_changed_ = changed;
}

int recruitment::recruit_situation_change_observer::gamestate_changed() {
	return gamestate_changed_;
}

void recruitment::recruit_situation_change_observer::reset_gamestate_changed() {
	gamestate_changed_ = 0;
}

recruitment_aspect::recruitment_aspect(readonly_context &context, const config &cfg, const std::string &id)
	: standard_aspect<config>(context, cfg, id)
{
	config parsed_cfg(cfg.has_child("value") ? cfg.child("value") : cfg);
	// First, transform simplified tags into [recruit] tags.
	for (config pattern : parsed_cfg.child_range("pattern")) {
		parsed_cfg["pattern"] = true;
		parsed_cfg.add_child("recruit", std::move(pattern));
	}
	for (config total : parsed_cfg.child_range("total")) {
		parsed_cfg["total"] = true;
		parsed_cfg.add_child("recruit", std::move(total));
	}
	parsed_cfg.clear_children("pattern", "total");
	// Then, if there's no [recruit], add one.
	if (!parsed_cfg.has_child("recruit")) {
		parsed_cfg.add_child("recruit", config {"importance", 0});
	}
	// Finally, populate our lists
	for (config job : parsed_cfg.child_range("recruit")) {
		create_job(jobs_, job);
	}
	for (config lim : parsed_cfg.child_range("limit")) {
		create_limit(limits_, lim);
	}
	std::function<void(std::vector<std::shared_ptr<recruit_job>>&, const config&)> factory_jobs =
		std::bind(&recruitment_aspect::create_job,*this,_1,_2);
	std::function<void(std::vector<std::shared_ptr<recruit_limit>>&, const config&)> factory_limits =
		std::bind(&recruitment_aspect::create_limit,*this,_1,_2);
	register_vector_property(property_handlers(), "recruit", jobs_, factory_jobs);
	register_vector_property(property_handlers(), "limit", limits_, factory_limits);
}

void recruitment_aspect::recalculate() const {
	config cfg;
	for (const std::shared_ptr<recruit_job>& job : jobs_) {
		cfg.add_child("recruit", job->to_config());
	}
	for (const std::shared_ptr<recruit_limit>& lim : limits_) {
		cfg.add_child("limit", lim->to_config());
	}
	*this->value_ = cfg;
	this->valid_ = true;
}

void recruitment_aspect::create_job(std::vector<std::shared_ptr<recruit_job>> &jobs, const config &job) {
	std::shared_ptr<recruit_job> job_ptr(new recruit_job(
		utils::split(job["type"]),
		job["leader_id"], job["id"],
		job["number"].to_int(-1), job["importance"].to_int(1),
		job["total"].to_bool(false),
		job["blocker"].to_bool(true),
		job["pattern"].to_bool(true)
	));
	jobs.push_back(job_ptr);
}

void recruitment_aspect::create_limit(std::vector<std::shared_ptr<recruit_limit>> &limits, const config &lim) {
	std::shared_ptr<recruit_limit> lim_ptr(new recruit_limit(
		utils::split(lim["type"]),
		lim["id"],
		lim["max"].to_int(0)
	));
	limits.push_back(lim_ptr);
}

}  // namespace default_recruitment
}  // namespace ai
