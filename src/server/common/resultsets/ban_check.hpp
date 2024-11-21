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

#include "mariadb++/result_set.hpp"

#include "server/common/resultsets/rs_base.hpp"

#include <chrono>

class ban_check : public rs_base
{
public:
	ban_check();
	explicit ban_check(const mariadb::result_set& rslt);
	void read(mariadb::result_set_ref rslt) override;
	long get_ban_type();
	std::chrono::seconds get_ban_duration();
	int get_user_id();
	std::string get_email();

private:
	long ban_type;
	std::chrono::seconds ban_duration;
	int user_id;
	std::string email;
};
