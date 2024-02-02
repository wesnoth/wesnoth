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

#pragma once

#include <vector>

#include "mariadb++/result_set.hpp"

#include "server/common/resultsets/rs_base.hpp"
#include "server/common/simple_wml.hpp"

class game_history : public rs_base
{
    struct player
    {
        std::string name;
        std::string faction;
    };

    struct result
    {
        std::string game_name;
        std::string game_start;
        std::string scenario_name;
        std::string era_name;
        std::vector<player> players;
        std::vector<std::string> modification_names;
        std::string replay_url;
        std::string version;
    };

    public:
        void read(mariadb::result_set_ref rslt);
        std::unique_ptr<simple_wml::document> to_doc();

    private:
        std::vector<result> results;
};
