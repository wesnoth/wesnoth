/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
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
 * Game information for the AI
 */

#pragma once

#include "map/location.hpp"
namespace pathfind {
    struct paths;
}
#include <map>
#include <memory>

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

class ai_composite;

typedef std::shared_ptr<ai_composite> ai_ptr;

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
class stage;

template<typename T>
class typesafe_aspect;

template<typename T>
struct aspect_type {
	typedef std::shared_ptr< typesafe_aspect<T>> typesafe_ptr;
	typedef std::vector< std::shared_ptr< typesafe_aspect<T>> > typesafe_ptr_vector;
};

template<typename T>
class typesafe_known_aspect;

template<typename T>
struct known_aspect_type {
	typedef std::shared_ptr< typesafe_known_aspect<T>> typesafe_ptr;
	typedef std::vector< std::shared_ptr< typesafe_known_aspect<T>> > typesafe_ptr_vector;
};

class action_result;
class attack_result;
class recall_result;
class recruit_result;
class move_result;
class move_and_attack_result;
class stopunit_result;
class synced_command_result;

typedef std::shared_ptr<action_result> action_result_ptr;
typedef std::shared_ptr<attack_result> attack_result_ptr;
typedef std::shared_ptr<recall_result> recall_result_ptr;
typedef std::shared_ptr<recruit_result> recruit_result_ptr;
typedef std::shared_ptr<move_result> move_result_ptr;
typedef std::shared_ptr<move_and_attack_result> move_and_attack_result_ptr;
typedef std::shared_ptr<stopunit_result> stopunit_result_ptr;
typedef std::shared_ptr<synced_command_result> synced_command_result_ptr;

typedef std::shared_ptr< aspect > aspect_ptr;
typedef std::shared_ptr< candidate_action > candidate_action_ptr;
typedef std::shared_ptr< engine > engine_ptr;
typedef std::shared_ptr< goal > goal_ptr;
typedef std::shared_ptr< known_aspect > known_aspect_ptr;
typedef std::shared_ptr< stage > stage_ptr;

typedef std::map<std::string, aspect_ptr > aspect_map;
typedef std::map<std::string, known_aspect_ptr > known_aspect_map;

class game_info {
public:

		game_info()
			: recent_attacks()
		{}

		/** hack. @todo 1.9 rework that via extended event system, or at least ensure it hurts no one */
		std::set<map_location> recent_attacks;
};

} //of namespace ai
