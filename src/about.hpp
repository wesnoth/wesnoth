/*
	Copyright (C) 2003 - 2024
	by David White <dave@whitevine.net>
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

#include "tstring.hpp"

#include <optional>
#include <string>
#include <utility>
#include <vector>

class config;
class game_config_view;

namespace about
{
struct credits_group
{
	struct about_group
	{
		explicit about_group(const config& cfg);

		/** Contributor names. */
		std::vector<std::pair<std::string, std::string>> names;

		/** The section title. */
		t_string title;

		bool operator<(const about_group& o) const;
	};

	credits_group(const config& cfg, bool is_campaign_credits);

	/** The group's sub-groups. Corresponds to each [about] tag .*/
	std::vector<about_group> sections;

	/** Optional group ID. Currently only used for identifying campaigns. */
	std::string id;

	/** Optional group title. Currently only used for identifying campaigns. */
	t_string header;
};

using credits_data = std::vector<credits_group>;

/** Gets all credits data. */
const credits_data& get_credits_data();

/** Gets credits for a given campaign. Returns a null optional if that campaign has no credits. */
std::optional<credits_data::const_iterator> get_campaign_credits(const std::string& campaign);

/** Gets credit background images for a given campaign. */
std::vector<std::string> get_background_images(const std::string& campaign);

/** Regenerates the credits data. */
void set_about(const game_config_view& cfg);

} // namespace about
