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

#include "server/common/resultsets/tournaments.hpp"

void tournaments::read(mariadb::result_set_ref rslt)
{
    while(rslt->next())
    {
        rows.push_back(data{ rslt->get_string("TITLE"), rslt->get_string("STATUS"), rslt->get_string("URL") });
    }
}

std::string tournaments::str()
{
    std::string text;
    for(const auto& row : rows)
    {
        text += "\nThe tournament "+row.title+" is "+row.status+". More information can be found at "+row.url;
    }
    return text;
}

#endif //HAVE_MYSQLPP
