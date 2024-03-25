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

#include <ctime>

#include "server/common/resultsets/ban_check.hpp"
#include "server/common/user_handler.hpp"

ban_check::ban_check()
{
    ban_type = user_handler::BAN_TYPE::BAN_NONE;
    ban_duration = 0;
    user_id = 0;
    email = "";
}

void ban_check::read(mariadb::result_set_ref rslt)
{
    if(rslt->next())
    {
        ban_type = rslt->get_signed32("ban_type");
        ban_duration = rslt->get_signed32("ban_end") != 0 ? rslt->get_signed32("ban_end") - std::time(nullptr) : 0;
        user_id = rslt->get_signed32("ban_userid");
        email = rslt->get_string("ban_email");
    }
}

long ban_check::get_ban_type()
{
    return ban_type;
}

int ban_check::get_ban_duration()
{
    return ban_duration;
}

int ban_check::get_user_id()
{
    return user_id;
}

std::string ban_check::get_email()
{
    return email;
}

#endif //HAVE_MYSQLPP
