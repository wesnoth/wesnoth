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

#pragma once

#include "tstring.hpp"

#include <vector>
#include <string>

class config;

namespace about
{
struct credits_group
{
	struct about_group
	{
		explicit about_group(const config& cfg);

		/** Contributor names. */
		std::vector<std::string> names;

		/** The section title. */
		t_string title;

        bool operator<(const about_group& o);
	};

	credits_group(const config& cfg, bool is_campaign_credits);

	/** The group's sub-groups. Corresponds to each [about] tag .*/
	std::vector<about_group> sections;

	/** Optional group ID. Currently only used for identifying campaigns. */
	std::string id;

	/** Optional group tite. Currently only used for identifying campaigns. */
	t_string header;
};

using credits_data = std::vector<credits_group>;

/**
 * General getter methods for the credits config and image lists by campaign id
 */
const credits_data& get_credits_data();

std::vector<std::string> get_background_images(const std::string& campaign);

/**
 * Regenerates the credits config
 */
void set_about(const config& cfg);

}
