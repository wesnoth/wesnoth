/*
	Copyright (C) 2020 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#ifdef HAVE_MYSQLPP

#include "server/common/resultsets/game_history.hpp"
#include "serialization/string_utils.hpp"
#include "log.hpp"

static lg::log_domain log_sql_handler("sql_executor");
#define ERR_SQL LOG_STREAM(err, log_sql_handler)

void game_history::read(mariadb::result_set_ref rslt)
{
    while(rslt->next())
    {
        result r;
        r.game_name = rslt->get_string("GAME_NAME");
        r.game_start = rslt->get_date_time("START_TIME").str();
        r.scenario_name = rslt->get_string("SCENARIO_NAME");
        r.era_name = rslt->get_string("ERA_NAME");
        for(const auto& player_info : utils::split(rslt->get_string("PLAYERS")))
        {
            std::vector<std::string> info = utils::split(player_info, ':');
            if(info.size() == 2)
            {
                r.players.emplace_back(player{ info[0], info[1] });
            }
            else
            {
                ERR_SQL << "Expected player information to split into two fields, instead found the value `" << player_info << "`.";
            }
        }
        r.modification_names = utils::split(rslt->get_string("MODIFICATION_NAMES"));
        r.replay_url = rslt->get_string("REPLAY_URL");
        r.version = rslt->get_string("VERSION");
        results.push_back(std::move(r));
    }
}

std::unique_ptr<simple_wml::document> game_history::to_doc()
{
    auto doc = std::make_unique<simple_wml::document>();

    simple_wml::node& results_wml = doc->root().add_child("game_history_results");

    for(const auto& result : results)
    {
        simple_wml::node& ghr = results_wml.add_child("game_history_result");
        ghr.set_attr_dup("game_name", result.game_name.c_str());
        ghr.set_attr_dup("game_start", result.game_start.c_str());
        ghr.set_attr_dup("scenario_name", result.scenario_name.c_str());
        ghr.set_attr_dup("era_name", result.era_name.c_str());
        ghr.set_attr_dup("replay_url", result.replay_url.c_str());
        ghr.set_attr_dup("version", result.version.c_str());
        for(const auto& player : result.players)
        {
            simple_wml::node& p = ghr.add_child("player");
            p.set_attr_dup("name", player.name.c_str());
            p.set_attr_dup("faction", player.faction.c_str());
        }
        for(const auto& mod : result.modification_names)
        {
            simple_wml::node& m = ghr.add_child("modification");
            m.set_attr_dup("name", mod.c_str());
        }
    }
    return doc;
}

#endif //HAVE_MYSQLPP
