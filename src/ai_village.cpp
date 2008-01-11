/* $Id$ */
/*
   Copyright (C) 2008 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file ai_village.cpp
//! The village capturing part of the AI.
//! ai::get_villages and ai::find_villages are based on ai::get_villages is ai.cpp

#include "global.hpp"

#include "ai.hpp"
#include "log.hpp"

#include <cassert>
#include <numeric>

#define DBG_AI LOG_STREAM(debug, ai) 
#define LOG_AI LOG_STREAM(info, ai) 
#define WRN_AI LOG_STREAM(warn, ai)

// Basic strategy
// 1. Store all our units that can move.
//
// 2. Test all reachable locations to be a proper village.
//
// 3. How many units can reach the village?
//    - If 0 ignore village.
//    - If 1 capture and remove unit from available list.
//    - Else store.
// 
// 4. Remove units who can't reach a village.
//
// 5. Dispatch single solutions:
//    - Dispatch units that can capture 1 village.
//    - After this dispatch some units might not be able to reach
//      anything anymore, remove them.
//    - How many units left?
//      - If 1 dispatch to village.
//    - For villages that can be captured by 1 unit, dispatch that unit.
//    - how many villages left?
//      - If 1 dispatch a unit.
// 
// 6. If no units we finish.
//
// At this point we have X villages with Y units left where 
// X > 1 && Y > 1 and every unit can reach at least 2 villages
// and every village can be visited by at least 2 units.
//
// 7. Can every unit visit every village?
//    - Yes dispatch them first unit first village
//      second unit second village etc.
//
//  In the following example:
//
//	village x1,y1   x2,y2   x3,y3   x4,y4
//	unit  
//	1       X       X       X       X
//	2       -       X       X       -
//	3       X       -       -       X
//
//	We want to find squares of 2 units who both can reach the same village. 
//
//	village x1,y1   x2,y2   x3,y3   x4,y4
//	unit  
//	               ___________
//	1       X|     |X       X|     |X
//	        _|     |         |     |_
//	               |         |
//	2       -      |X       X|      -
//	               |_________|
//	        __                     __
//	3       X|      -       -      |X
//   
//  8. - Find a square where at least 1 unit can visit only 2 villages.
//     - Dispatch the units.
//     - Could the second unit also visit 2 villages and where the 2 villages
//       only visitable by two units?
//       - Yes found a perfect solution for them 
//         - Reduce the village count.
//         - Restart ourselves recursively.
//         - Finsished.
//
//       - No. 
//         - Remove the taken villages from the list.
//         - Go to step 5.
//
//  9. Didn't find a solution, test all permutations.

namespace {
	//! Location of the keep the closest to our leader.
	gamemap::location keep_loc = gamemap::location::null_location;
	//! Locaton of our leader.
	gamemap::location leader_loc = gamemap::location::null_location;
	//! The best possible location for our leader if it can't
	//! reach a village.
	gamemap::location best_leader_loc = gamemap::location::null_location;

	//! debug log level for AI enabled?
	bool debug = false;

	typedef std::map<gamemap::location /* unit location */, 
		std::vector<gamemap::location /* villages we can reach */> > treachmap;

	typedef std::vector<std::pair<gamemap::location /* destination */, 
		gamemap::location /* start */ > > tmoves;
}

//! Dispatches all units to their best location.
static void dispatch(treachmap& reachmap, tmoves& moves);

//! Dispatches all units who can reach one village.
//! Returns true if it modified reachmap isn't empty
static bool dispatch_unit_simple(treachmap& reachmap, tmoves& moves);

//! Dispatches units to villages which can only be reached by one unit.
//! Returns true if modified reachmap and reachmap isn't empty
static bool dispatch_village_simple(
	treachmap& reachmap, tmoves& moves, size_t& village_count);

//! Removes a village for all units, returns true if anything is deleted.
static bool remove_village(
	treachmap& reachmap, tmoves& moves, const gamemap::location& village);

//! Removes a unit which can't reach any village anymore.
static treachmap::iterator remove_unit(
	treachmap& reachmap, tmoves& moves, treachmap::iterator unit);

//! Dispatches the units to a village after the simple dispatching failed.
static void dispatch_complex(
	treachmap& reachmap, tmoves& moves, const size_t village_count);

//! Dispatches all units to a village, every unit can reach every village.
static void full_dispatch(treachmap& reachmap, tmoves& moves);

//! Shows which villages every unit can reach (debug function).
static void dump_reachmap(treachmap& reachmap);

bool ai::get_villages(std::map<gamemap::location,paths>& possible_moves,
		const move_map& dstsrc, const move_map& enemy_dstsrc, 
		unit_map::iterator &leader)
{
	DBG_AI << "deciding which villages we want...\n";
	const int ticks = SDL_GetTicks();
	best_leader_loc = gamemap::location::null_location;
	if(leader != units_.end()) {
		keep_loc = nearest_keep(leader->first);
		leader_loc = leader->first;
	} else {
		keep_loc = gamemap::location::null_location;
		leader_loc = gamemap::location::null_location;
	}

	debug = (!lg::debug.dont_log(lg::ai));

	// Find our units who can move.
	treachmap reachmap;	
	for(unit_map::const_iterator u_itor = units_.begin();
			u_itor != units_.end(); ++u_itor) {

		if(u_itor->second.side() == team_num_ 
				&& u_itor->second.movement_left()) {

			reachmap.insert(std::make_pair(u_itor->first,	std::vector<gamemap::location>()));
		}
	}

	// The list of moves we want to make
	tmoves moves;

	DBG_AI << reachmap.size() << " units found who can try to capture a village.\n";

	find_villages(reachmap, moves, dstsrc, possible_moves, enemy_dstsrc);

	treachmap::iterator itor = reachmap.begin();
	while(itor != reachmap.end()) {
		if(itor->second.size() == 0) {
			itor = remove_unit(reachmap, moves, itor);
		} else {
			++itor;
		}
	}

	if(reachmap.size()) {
		DBG_AI << reachmap.size() << " units left after removing the ones who "
			"can't reach a village, send the to the dispatcher.\n";

		dump_reachmap(reachmap);

		dispatch(reachmap, moves);
	} else {
		DBG_AI << "No more units left after removing the ones who can't reach a village.\n";
	}

	LOG_AI << "Village assignment done: " << (SDL_GetTicks() - ticks)
		<< " ms, resulted in " << moves.size() << " units being dispatched.\n";

	// Move all the units to get villages, however move the leader last,
	// so that the castle will be cleared if it wants to stop to recruit along the way.
	std::pair<location,location> leader_move;

	int moves_made = 0;
	for(tmoves::const_iterator i = moves.begin(); i != moves.end(); ++i) {

		if(leader != units_.end() && leader->first == i->second) {
			leader_move = *i;
		} else {
			if(units_.count(i->first) == 0) {
				const location loc = move_unit(i->second,i->first,possible_moves);
				++moves_made;
				leader = find_leader(units_, team_num_);

				// If we didn't make it to the destination, it means we were ambushed.
				if(loc != i->first) {
					return true;
				}

				const unit_map::const_iterator new_unit = units_.find(loc);

				if(new_unit != units_.end() &&
						power_projection(i->first,enemy_dstsrc) >= new_unit->second.hitpoints()/4) {
					add_target(target(new_unit->first,1.0,target::SUPPORT));
				}
			}
		}
	}

	if(leader_move.second.valid()) {
		if(units_.count(leader_move.first) == 0) {
			gamemap::location loc = move_unit(leader_move.second,leader_move.first,possible_moves);
			++moves_made;
			// Update leader iterator, since we moved it.
			leader = units_.find(loc);
		}
	}

	return false;
}

void ai::find_villages(
	treachmap& reachmap,
	tmoves& moves,
	const std::multimap<gamemap::location,gamemap::location>& dstsrc,
	const std::map<gamemap::location,paths>& possible_moves,
	const std::multimap<gamemap::location,gamemap::location>& enemy_dstsrc) const

{
	std::map<location, double> vulnerability;

	const bool passive_leader = 
		utils::string_bool(current_team().ai_parameters()["passive_leader"]);

	size_t min_distance = 100000;
	
	// When a unit is dispatched we need to make sure we don't
	// dispatch this unit a second time, so store them here.
	std::vector<gamemap::location> dispatched_units;
	for(std::multimap<gamemap::location, gamemap::location>::const_iterator 
			j = dstsrc.begin();
			j != dstsrc.end(); ++j) {

		const gamemap::location& current_loc = j->first;

		if(j->second == leader_loc) {
			if(passive_leader) {
				continue;
			}

			const size_t distance = distance_between(keep_loc, current_loc);
			if(distance < min_distance) {
				min_distance = distance;
				best_leader_loc = current_loc;
			}
		}

		if(std::find(dispatched_units.begin(), dispatched_units.end(),
				j->second) != dispatched_units.end()) {
			continue;
		}

		if(map_.is_village(current_loc) == false) {
			continue;
		}

		bool want_village = true, owned = false;
		for(size_t n = 0; n != teams_.size(); ++n) {
			owned = teams_[n].owns_village(current_loc);
			if(owned && !current_team().is_enemy(n+1)) {
				want_village = false;
			}

			if(owned) {
				break;
			}
		}

		if(want_village == false) {
			continue;
		}

		// If it is a neutral village, and we have no leader,
		// then the village is of no use to us, and we don't want it.
		if(!owned && leader_loc == gamemap::location::null_location) {
			continue;
		}

		// If we have a decent amount of gold, and the leader can't access
		// the keep this turn if they get this village,
		// then don't get this village with them.
		if(want_village &&
				current_team().gold() > 20 &&
				leader_loc == current_loc &&
				leader_loc != keep_loc &&
				multistep_move_possible(j->second, current_loc, keep_loc, possible_moves) == false) {
			continue;
		}

		double threat = 0.0;
		const std::map<location,double>::const_iterator vuln = vulnerability.find(current_loc);
		if(vuln != vulnerability.end()) {
			threat = vuln->second;
		} else {
			threat = power_projection(current_loc,enemy_dstsrc);
			vulnerability.insert(std::pair<location,double>(current_loc,threat));
		}

		const unit_map::const_iterator u = units_.find(j->second);
		if(u == units_.end() || utils::string_bool(u->second.get_state("guardian"))) {
			continue;
		}

		const unit& un = u->second;
		if(un.hitpoints() < (threat*2*un.defense_modifier(map_.get_terrain(current_loc)))/100) {
			continue;
		}

		// If the next and previous destination differs from our current destination, 
		// we're the only one who can reach the village -> dispatch.
		std::multimap<gamemap::location, gamemap::location>::const_iterator next = j;
		++next; // j + 1 fails
		const bool at_begin = (j == dstsrc.begin());
		std::multimap<gamemap::location, gamemap::location>::const_iterator prev = j; //FIXME seems not to work
		if(!at_begin) {
			--prev;
		}
#if 1		
		if((next == dstsrc.end() || next->first != current_loc) 
				&& (at_begin || prev->first != current_loc)) {

			DBG_AI << "Dispatched unit at " << j->second << " to village " << j->first << '\n';

			moves.push_back(std::make_pair(j->first, j->second));
			reachmap.erase(j->second);
			dispatched_units.push_back(j->second);
			continue;
		}
#endif		
		reachmap[j->second].push_back(current_loc);
	}

	DBG_AI << moves.size() << " units already dispatched, " 
		<< reachmap.size() << " left to evaluate.\n";
}

static void dispatch(treachmap& reachmap, tmoves& moves)
{
	DBG_AI << "Starting simple dispatch.\n";

	// we now have a list with units with the villages they can reach.
	// keep trying the following steps as long as one of them changes
	// the state, the first time both are ran.
	// 1. Dispatch units who can reach 1 village (if more units can reach that
	//    village only one can capture it, so use the first in the list.)
	// 2. Villages which can only be reached by one unit get that unit dispatched
	//    to them.
	bool first_run = true;
	size_t village_count = 0;
	while(true) {

		if(!dispatch_unit_simple(reachmap, moves)) {
			if(reachmap.empty()) {
				DBG_AI << "dispatch_unit_simple() found a final solution.\n";
			} else {
				DBG_AI << "dispatch_unit_simple() couldn't dispatch more units.\n";
			}
			if(!first_run) break; 
		}

		if(!dispatch_village_simple(reachmap, moves, village_count)) {
			if(reachmap.empty()) {
				DBG_AI << "dispatch_village_simple() found a final solution.\n";
			} else {
				DBG_AI << "dispatch_village_simple() couldn't dispatch more units.\n";
			}
			break;
		}
		
		if(reachmap.size() != 0) {
			DBG_AI << reachmap.size() << " unit(s) left restarting simple dispatching.\n";

			dump_reachmap(reachmap);
		}

		first_run = false;
	}

	if(reachmap.size() == 0) {
		DBG_AI << "No units left after simple dispatcher.\n";
		return;
	}

	DBG_AI << reachmap.size() << " units left for complex dispatch with " 
		<< village_count << " villages left.\n";

	dump_reachmap(reachmap);

	dispatch_complex(reachmap, moves, village_count);
}

// Returns		need further processing
// false		Nothing has been modified or no units left
static bool dispatch_unit_simple(treachmap& reachmap, tmoves& moves)
{
	bool result = false;
	
	treachmap::iterator itor = reachmap.begin();
	while(itor != reachmap.end()) {
		if(itor->second.size() == 1) {
			const gamemap::location village = itor->second[0];
			result = true;

			DBG_AI << "Dispatched unit at " << itor->first << " to village " << village << '\n';
			moves.push_back(std::make_pair(village, itor->first));
			reachmap.erase(itor++);

			if(remove_village(reachmap, moves, village)) {
				itor = reachmap.begin();
			}

		} else {
			++itor;
		}
	}

	// Test special cases.
	if(reachmap.empty()) {
		// We're done.
		return false;
	}

	if(reachmap.size() == 1) {
		// One unit left.
		DBG_AI << "Dispatched _last_ unit at " << reachmap.begin()->first 
			<< " to village " << reachmap.begin()->second[0] << '\n';

		moves.push_back(std::make_pair(
			reachmap.begin()->second[0], reachmap.begin()->first));

		reachmap.clear();
		// We're done.
		return false;
	}

	return result;
}

static bool dispatch_village_simple(
	treachmap& reachmap, tmoves& moves, size_t& village_count)
{

	bool result = false;
	bool dispatched = true;
	while(dispatched) {
		dispatched = false;

		// build the reverse map
		std::map<gamemap::location /*village location*/, 
			std::vector<gamemap::location /* units that can reach it*/> >reversemap;

		treachmap::const_iterator itor = reachmap.begin();
		for(;itor != reachmap.end(); ++itor) {

			for(std::vector<gamemap::location>::const_iterator 
					v_itor = itor->second.begin();
			 		v_itor != itor->second.end(); ++v_itor) {

				reversemap[*v_itor].push_back(itor->first);

			}
		}

		village_count = reversemap.size();

		itor = reversemap.begin();
		while(itor != reversemap.end()) {
			if(itor->second.size() == 1) {
				// One unit can reach this village.
				const gamemap::location village = itor->first;
				dispatched = true;
				result = true;

				DBG_AI << "Dispatched unit at " << itor->second[0] << " to village " << itor->first << '\n';
				moves.push_back(std::make_pair(itor->first, itor->second[0]));

				reachmap.erase(itor->second[0]);
				remove_village(reachmap, moves, village);
				// Get can go to some trouble to remove the unit from the other villages
				// instead we abort this loop end do a full rebuild on the map.
				break;
			} else {
				++itor;
			}
		}
	}

	return result;
}

static bool remove_village(
	treachmap& reachmap, tmoves& moves, const gamemap::location& village)
{
	bool result = false;
	treachmap::iterator itor = reachmap.begin();
	while(itor != reachmap.end()) {
		itor->second.erase(std::remove(itor->second.begin(), itor->second.end(), village), itor->second.end());
		if(itor->second.empty()) {
			result = true;
			itor = remove_unit(reachmap, moves, itor);
		} else {
			++itor;
		}
	}
	return result;
}

static treachmap::iterator remove_unit(
	treachmap& reachmap, tmoves& moves, treachmap::iterator unit)
{
	assert(unit->second.empty());

	if(unit->first == leader_loc && best_leader_loc != gamemap::location::null_location) {
		DBG_AI << "Dispatch leader at " << leader_loc << " closer to the keep at " 
			<< best_leader_loc << '\n';

		moves.push_back(std::make_pair(best_leader_loc, leader_loc));
	}

	reachmap.erase(unit++);
	return unit;
}

static void dispatch_complex(
	treachmap& reachmap, tmoves& moves, const size_t village_count)
{
	// ***** ***** Init and dispatch if every unit can reach every village.

	const size_t unit_count = reachmap.size();
	// The maximum number of villages we can capture with the available units.
	const size_t max_result = unit_count < village_count ? unit_count : village_count;

	assert(unit_count >= 2 && village_count >= 2);

	// Every unit can reach every village.
	if(unit_count == 2 && village_count == 2) {
		DBG_AI << "Every unit can reach every village for 2 units, dispatch them.\n";
		full_dispatch(reachmap, moves);
		return;
	}

	std::vector<gamemap::location> units(unit_count);
	std::vector<size_t> villages_per_unit(unit_count);
	std::vector<gamemap::location> villages;
	std::vector<size_t> units_per_village(village_count);

	// We want to test the units, the ones who can reach the least
	// villages first so this is our lookup map.
	std::multimap<size_t /* villages_per_unit value*/, 
		size_t /*villages_per_unit index*/> unit_lookup;

	std::vector</*unit*/std::vector</*village*/bool> > 
		matrix(reachmap.size(), std::vector<bool>(village_count, false));

	treachmap::const_iterator itor = reachmap.begin();
	for(size_t u = 0; u < unit_count; ++u, ++itor) {
		units[u] = itor->first;
		villages_per_unit[u] = itor->second.size();
		unit_lookup.insert(std::make_pair(villages_per_unit[u], u));

		assert(itor->second.size() >= 2);

		for(size_t v = 0; v < itor->second.size(); ++v) {

			size_t v_index;
			// find the index of the v in the villages
			std::vector<gamemap::location>::const_iterator v_itor = 
				std::find(villages.begin(), villages.end(), itor->second[v]);
			if(v_itor == villages.end()) {
				v_index = villages.size(); // will be the last element after push_back.
				villages.push_back(itor->second[v]);
			} else {
				v_index = v_itor - villages.begin();
			}

			units_per_village[v_index]++;
			
			matrix[u][v_index] = true;
		}
	}
	for(std::vector<size_t>::const_iterator upv_it = units_per_village.begin();
			upv_it != units_per_village.end(); ++upv_it) {

		assert(*upv_it >=2);
	}

	if(debug) {
		// Print header
		std::cerr << "Reach matrix:\n\nvillage";
		size_t u, v;
		for(v = 0; v < village_count; ++v) {
			std::cerr << '\t' << villages[v];
		}
		std::cerr << "\ttotal\nunit\n";

		// Print data
		for(u = 0; u < unit_count; ++u) {
			std::cerr << units[u];

			for(size_t v = 0; v < village_count; ++v) {
				std::cerr << '\t' << matrix[u][v];
			}
			std::cerr << "\t" << villages_per_unit[u] << '\n';
		}

		// Print footer
		std::cerr << "total";
		for(v = 0; v < village_count; ++v) {
			std::cerr << '\t' << units_per_village[v];
		}
		std::cerr << '\n';
	}

	// Test the special case, everybody can reach all villages
	const bool reach_all = ((village_count == unit_count) 
		&& (std::accumulate(villages_per_unit.begin(), villages_per_unit.end(), size_t()) 
		== (village_count * unit_count)));

	if(reach_all) {
		DBG_AI << "Every unit can reach every village, dispatch them\n";
		full_dispatch(reachmap, moves);
		reachmap.clear();
		return;
	}

	// ***** ***** Find a square
	std::multimap<size_t /* villages_per_unit value*/, size_t /*villages_per_unit index*/>
		::const_iterator src_itor =  unit_lookup.begin();

	while(src_itor != unit_lookup.end() && src_itor->first == 2) {
		
		for(std::multimap<size_t, size_t>::const_iterator 
				dst_itor = unit_lookup.begin();
				dst_itor != unit_lookup.end(); ++ dst_itor) {

			// avoid comparing us with ourselves.
			if(src_itor == dst_itor) {
				continue;
			}

			std::vector<bool> result;
			std::transform(matrix[src_itor->second].begin(), matrix[src_itor->second].end(),
				matrix[dst_itor->second].begin(),
				std::back_inserter(result),
				std::logical_and<bool>()
				);

			size_t matched = std::count(result.begin(), result.end(), true);
			
			// we found a  solution, dispatch
			if(matched == 2) {
				// Collect data
				std::vector<bool>::iterator first = std::find(result.begin(), result.end(), true);
				std::vector<bool>::iterator second = std::find(first + 1, result.end(), true);

				const gamemap::location village1 = villages[first - result.begin()];
				const gamemap::location village2 = villages[second - result.begin()];

				const bool perfect = (src_itor->first == 2 && 
					dst_itor->first == 2 && 
					units_per_village[first - result.begin()] == 2 &&
					units_per_village[second - result.begin()] == 2);

				// Dispatch
				DBG_AI << "Found a square.\nDispatched unit at " << units[src_itor->second]
						<< " to village " << village1 << '\n';
				moves.push_back(std::make_pair(village1, units[src_itor->second]));

				DBG_AI << "Dispatched unit at " << units[dst_itor->second]
						<< " to village " << village2 << '\n';
				moves.push_back(std::make_pair(village2, units[dst_itor->second]));

				// Remove the units
				reachmap.erase(units[src_itor->second]);
				reachmap.erase(units[dst_itor->second]);
				
				// Evaluate and start correct function.
				if(perfect) {
					// We did a perfect dispatch 2 units who could visit 2 villages.
					// This means we didn't change the assertion for this funtions
					// so call ourselves recursively, and finish afterwards.
					DBG_AI << "Perfect dispatch, do complex again.\n";
					dispatch_complex(reachmap, moves, village_count - 2);
					return;
				} else {
					// We did a not perfect dispatch but we did modify things
					// so restart dispatching.
					DBG_AI << "NON Perfect dispatch, do dispatch again.\n";
					remove_village(reachmap, moves, village1);
					remove_village(reachmap, moves, village2);
					dispatch(reachmap, moves);
					return;
				}
			}
		}

		++src_itor;
	}

	// ***** ***** Do all permutations.
	// Now walk through all possible permutations
	// - test whether the suggestion is possible
	// - does it result in max_villages 
	//   - dispatch and ready
	// - is it's result better as the last best
	//   - store
	std::vector<std::pair<gamemap::location, gamemap::location> > best_result;

	// Bruteforcing all possible permutations can result in a slow game.
	// So there needs to be a balance between the best possible result and
	// not too slow. From the test (at the end of the file) a good number is
	// picked. In general we shouldn't reach this point too often if we do
	// there are a lot of villages which are unclaimed and a lot of units
	// to claim them.
	const size_t max_options = 8;
	if(unit_count >= max_options && village_count >= max_options) {

		DBG_AI << "Too many units " << unit_count << " and villages " 
			<< village_count<<" found, evaluate only the first " 
			<< max_options << " options;\n";

		std::vector<size_t> perm (max_options, 0);
		for(size_t i =0; i < max_options; ++i) {
			perm[i] = i;
		}
		while(std::next_permutation(perm.begin(), perm.end())) {

			// Get result for current permutation.
			std::vector<std::pair<gamemap::location,gamemap::location> > result;
			for(size_t u = 0; u < max_options; ++u) {
				if(matrix[u][perm[u]]) {
					result.push_back(std::make_pair(villages[perm[u]], units[u]));

				}
			}
			if(result.size() == max_result) {
				best_result.swap(result);
				break;
			}

			if(result.size() > best_result.size()) {
				best_result.swap(result);
			}
		}
		// End of loop no optimal found, assign the best
		std::copy(best_result.begin(), best_result.end(), std::back_inserter(moves));

		// Clean up the reachmap for dispatched units.
		for(std::vector<std::pair<gamemap::location, gamemap::location> >::const_iterator 
				itor = best_result.begin(); itor != best_result.end(); ++itor) {
			reachmap.erase(itor->second);
		}

		// Try to dispatch whatever is left
		dispatch(reachmap, moves);
		return;

	} else if(unit_count <= village_count) {

		DBG_AI << "Unit major\n";

		std::vector<size_t> perm (unit_count, 0);
		for(size_t i =0; i < unit_count; ++i) {
			perm[i] = i;
		}
		while(std::next_permutation(perm.begin(), perm.end())) {
			// Get result for current permutation.
			std::vector<std::pair<gamemap::location,gamemap::location> > result;
			for(size_t u = 0; u < unit_count; ++u) {
				if(matrix[u][perm[u]]) {
					result.push_back(std::make_pair(villages[perm[u]], units[u]));

				}
			}
			if(result.size() == max_result) {
				std::copy(result.begin(), result.end(), std::back_inserter(moves));
				reachmap.clear();
				return;
			}

			if(result.size() > best_result.size()) {
				best_result.swap(result);
			}
		}
		// End of loop no optimal found, assign the best
		std::copy(best_result.begin(), best_result.end(), std::back_inserter(moves));

		// clean up the reachmap we need to test whether the leader is still there
		// and if so remove him manually to get him dispatched.
		for(std::vector<std::pair<gamemap::location, gamemap::location> >::const_iterator 
				itor = best_result.begin(); itor != best_result.end(); ++itor) {
			reachmap.erase(itor->second);
		}
		treachmap::iterator unit = reachmap.find(leader_loc);
		if(unit != reachmap.end()) {
			unit->second.clear();
			remove_unit(reachmap, moves, unit);
		}
		reachmap.clear();

	} else {

		DBG_AI << "Village major\n";

		std::vector<size_t> perm (village_count, 0);
		for(size_t i =0; i < village_count; ++i) {
			perm[i] = i;
		}
		while(std::next_permutation(perm.begin(), perm.end())) {
			// Get result for current permutation.
			std::vector<std::pair<gamemap::location,gamemap::location> > result;
			for(size_t v = 0; v < village_count; ++v) {
				if(matrix[perm[v]][v]) {
					result.push_back(std::make_pair(villages[v], units[perm[v]]));

				}
			}
			if(result.size() == max_result) {
				std::copy(result.begin(), result.end(), std::back_inserter(moves));
				reachmap.clear();
				return;
			}

			if(result.size() > best_result.size()) {
				best_result.swap(result);
			}
		}
		// End of loop no optimal found, assigne the best
		std::copy(best_result.begin(), best_result.end(), std::back_inserter(moves));

		// clean up the reachmap we need to test whether the leader is still there
		// and if so remove him manually to get him dispatched.
		for(std::vector<std::pair<gamemap::location, gamemap::location> >::const_iterator 
				itor = best_result.begin(); itor != best_result.end(); ++itor) {
			reachmap.erase(itor->second);
		}
		treachmap::iterator unit = reachmap.find(leader_loc);
		if(unit != reachmap.end()) {
			unit->second.clear();
			remove_unit(reachmap, moves, unit);
		}
		reachmap.clear();
	}
}

static void full_dispatch(treachmap& reachmap, tmoves& moves)
{
	treachmap::const_iterator itor = reachmap.begin();
	for(size_t i = 0; i < reachmap.size(); ++i, ++itor) { 
		DBG_AI << "Dispatched unit at " << itor->first
				<< " to village " << itor->second[i] << '\n';
		moves.push_back(std::make_pair(itor->second[i], itor->first));
	}
}

static void dump_reachmap(treachmap& reachmap)
{
	if(!debug) {
		return;
	}

	for(treachmap::const_iterator itor = 
			reachmap.begin(); itor != reachmap.end(); ++itor) {
			
		std::cerr << "Reachlist for unit at " << itor->first;

		if(itor->second.empty()) {
			std::cerr << "\tNone";
		}

		for(std::vector<gamemap::location>::const_iterator 
				v_itor = itor->second.begin();
				v_itor != itor->second.end(); ++v_itor) {

			std::cerr << '\t' << *v_itor;
		}
		std::cerr << '\n';

	}
}

#if 0
// small helper rule to test the matching rules
// building rule
//make ai_village.o &&  g++-3.3 -o ai_village about.o actions.o ai.o ai_dfool.o ai_attack.o ai_move.o ai_python.o ai_village.o animated_game.o attack_prediction.o config_adapter.o dialogs.o floating_textbox.o game_display.o game_events.o game_preferences.o game_preferences_display.o gamestatus.o generate_report.o generic_event.o halo.o help.o intro.o leader_list.o menu_events.o mouse_events.o multiplayer.o multiplayer_ui.o multiplayer_wait.o multiplayer_connect.o multiplayer_create.o multiplayer_lobby.o network.o network_worker.o pathfind.o playcampaign.o play_controller.o playmp_controller.o playsingle_controller.o playturn.o publish_campaign.o replay.o replay_controller.o sha1.o settings.o statistics.o team.o terrain_filter.o titlescreen.o tooltips.o unit.o unit_abilities.o unit_animation.o unit_display.o unit_frame.o unit_map.o unit_types.o upload_log.o variable.o widgets/combo.o widgets/scrollpane.o -L. -lwesnoth-core -lSDL_image -lSDL_mixer -lSDL_net  -L/usr/lib -lSDL -L/usr/lib -lpython2.4  -lfreetype -lz  -L/usr/lib -lfribidi libwesnoth.a -lboost_iostreams  -lX11 -L/usr/lib -R/usr/lib
/* 
// gcc-3.3 -O0
Option count : 1 duration 0 ms
Option count : 2 duration 0 ms
Option count : 3 duration 0 ms
Option count : 4 duration 1 ms
Option count : 5 duration 1 ms
Option count : 6 duration 3 ms
Option count : 7 duration 16 ms
Option count : 8 duration 208 ms
Option count : 9 duration 1915 ms
Option count : 10 duration 18424 ms

// gcc-3.3 -O2
Option count : 1 duration 0 ms
Option count : 2 duration 0 ms
Option count : 3 duration 0 ms
Option count : 4 duration 0 ms
Option count : 5 duration 1 ms
Option count : 6 duration 0 ms
Option count : 7 duration 3 ms
Option count : 8 duration 41 ms
Option count : 9 duration 374 ms
Option count : 10 duration 3496 ms
Option count : 11 duration 38862 ms

// gcc-4.1 -O2
Option count : 1 duration 0 ms
Option count : 2 duration 0 ms
Option count : 3 duration 0 ms
Option count : 4 duration 0 ms
Option count : 5 duration 0 ms
Option count : 6 duration 1 ms
Option count : 7 duration 2 ms
Option count : 8 duration 26 ms
Option count : 9 duration 261 ms
Option count : 10 duration 2336 ms
Option count : 11 duration 26684 ms

*/ 
int main()
{
	const size_t max_matrix = 100;
	std::vector</*unit*/std::vector</*village*/bool> > matrix(max_matrix, std::vector<bool>(max_matrix, false));
	std::vector<gamemap::location> villages(max_matrix);
	std::vector<gamemap::location> units(max_matrix);

	srand(10);
	for(size_t i = 0; i < max_matrix; ++i) {
		for(size_t j = 0; j < max_matrix; ++j) {
			matrix[i][j] = ((rand() % 3) == 0);
			villages[i] = gamemap::location(rand() % 100, rand() % 100);
			units[i] = gamemap::location(rand() % 100, rand() % 100);
		}
	}

	// Permutations for 0 are quite senseless.
	std::vector<std::pair<gamemap::location,gamemap::location> > best_result;
	for(size_t option = 1; option < max_matrix; ++option) {
		// Set up the permuation 
		std::vector<size_t> perm (option, 0);
		for(size_t i = 0; i < option; ++i) {
			perm[i] = i;
		}

		const int start = SDL_GetTicks();
		while(std::next_permutation(perm.begin(), perm.end())) {

			// Get result for current permutation.
			std::vector<std::pair<gamemap::location,gamemap::location> > result;
			for(size_t u = 0; u < option; ++u) {
				if(matrix[u][perm[u]]) {
					result.push_back(std::make_pair(villages[perm[u]], units[u]));

				}
			}
			if(result.size() == option) {
				best_result.swap(result);
				break;
			}

			if(result.size() > best_result.size()) {
				best_result.swap(result);
			}
		}

		// Don't use buffered output we _expect_ the user to kill the program.
		std::cerr << "Option count : " << option << " duration " << (SDL_GetTicks() - start) << " ms\n";

	}
}

#endif

