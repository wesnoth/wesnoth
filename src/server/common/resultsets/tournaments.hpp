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

#include <vector>

class tournaments : public rs_base
{
    struct data
    {
        std::string title;
        std::string status;
        std::string url;
    };

    public:
        void read(mariadb::result_set_ref rslt);
        std::string str();

    private:
        std::vector<data> rows;
};
