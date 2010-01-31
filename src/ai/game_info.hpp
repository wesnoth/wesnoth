/* $Id$ */
/*
   Copyright (C) 2003 - 2010 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file ai/game_info.hpp
 * Game information for the AI
 */

#ifndef AI_GAME_INFO_HPP_INCLUDED
#define AI_GAME_INFO_HPP_INCLUDED

#include <boost/shared_ptr.hpp>
#include "../map_location.hpp"
namespace pathfind {
    struct paths;
}

#include <set>
#include <vector>
#include <map>

class game_display;
class game_state;
class gamemap;
class unit_map;
class team;
class tod_manager;

/**
 * info is structure which holds references to all the important objects
 * that an AI might need access to, in order to make and implement its
 * decisions.
 */
namespace ai {

typedef int side_number;

/** The standard way in which a map of possible moves is recorded. */
typedef std::multimap<map_location,map_location> move_map;

/** The standard way in which a map of possible movement routes to location is recorded*/
typedef std::map<map_location,pathfind::paths> moves_map;

class interface;

typedef boost::shared_ptr< interface > ai_ptr;

class attack_analysis;
typedef std::vector<attack_analysis> attacks_vector;

class readonly_context;
class readwrite_context;
class default_ai_context;
class ai_context;

class aspect;
class candidate_action;
class engine;
class goal;
class known_aspect;
class ministage;
class stage;

template<typename T>
class typesafe_aspect;

template<typename T>
struct aspect_type {
	typedef boost::shared_ptr< typesafe_aspect<T> > typesafe_ptr;
	typedef std::vector< boost::shared_ptr< typesafe_aspect<T> > > typesafe_ptr_vector;
};

template<typename T>
class typesafe_known_aspect;

template<typename T>
struct known_aspect_type {
	typedef boost::shared_ptr< typesafe_known_aspect<T> > typesafe_ptr;
	typedef std::vector< boost::shared_ptr< typesafe_known_aspect<T> > > typesafe_ptr_vector;
};

class attack_result;
class recall_result;
class recruit_result;
class move_result;
class move_and_attack_result;
class stopunit_result;

typedef boost::shared_ptr<attack_result> attack_result_ptr;
typedef boost::shared_ptr<recall_result> recall_result_ptr;
typedef boost::shared_ptr<recruit_result> recruit_result_ptr;
typedef boost::shared_ptr<move_result> move_result_ptr;
typedef boost::shared_ptr<move_and_attack_result> move_and_attack_result_ptr;
typedef boost::shared_ptr<stopunit_result> stopunit_result_ptr;

typedef boost::shared_ptr< aspect > aspect_ptr;
typedef boost::shared_ptr< candidate_action > candidate_action_ptr;
typedef boost::shared_ptr< engine > engine_ptr;
typedef boost::shared_ptr< goal > goal_ptr;
typedef boost::shared_ptr< known_aspect > known_aspect_ptr;
typedef boost::shared_ptr< ministage > ministage_ptr;
typedef boost::shared_ptr< stage > stage_ptr;

typedef std::map<std::string, aspect_ptr > aspect_map;
typedef std::map<std::string, known_aspect_ptr > known_aspect_map;

class game_info {
public:

		game_info(game_display& disp, gamemap& map, unit_map& units,
			std::vector<team>& teams, tod_manager& tod_mng, class game_state& game_state)
			: disp(disp)
			, map(map)
			, units(units)
			, teams(teams)
			, tod_manager_(tod_mng)
			, game_state_(game_state)
			, recent_attacks()
		{}

		/** The display object, used to draw the moves the AI makes. */
		game_display& disp;

		/** The map of the game -- use this object to find the terrain at any location. */
		gamemap& map;

		/** The map of units. It maps locations -> units. */
		unit_map& units;

		/** A list of the teams in the game. */
		std::vector<team>& teams;

		/** Information about what turn it is, and what time of day. */
		tod_manager& tod_manager_;

		/** The global game state, because we may set the completion field. */
		class game_state& game_state_;

		/** hack. @todo 1.9 rework that via extended event system, or at least ensure it hurts no one */
		std::set<map_location> recent_attacks;
};

} //of namespace ai

#endif
