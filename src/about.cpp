/*
	Copyright (C) 2003 - 2025
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

#include "about.hpp"

#include "config.hpp"
#include "font/pango/escape.hpp"
#include "game_config_view.hpp"
#include "gettext.hpp"
#include "serialization/string_utils.hpp"
#include "utils/general.hpp"

#include <map>

/**
 * @namespace about
 * Display credits %about all contributors.
 */
namespace about
{
namespace
{

credits_data parsed_credits_data;
std::map<std::string, std::vector<std::string>> images_campaigns;
std::vector<std::string> images_general;

void gather_images(const config& from, std::vector<std::string>& to)
{
	const auto& im = utils::parenthetical_split(from["images"].str(), ',');
	to.insert(to.end(), im.begin(), im.end());
}

} // namespace

credits_group::credits_group(const config& cfg, bool is_campaign_credits)
	: sections()
	, id()
	, header()
{
	if(is_campaign_credits) {
		id = cfg["id"].str();
		header = cfg["name"].t_str();
	}

	sections.reserve(cfg.child_count("about"));

	for(const config& about : cfg.child_range("about")) {
		if(!about.has_child("entry")) {
			continue;
		}

		sections.emplace_back(about);

		if(is_campaign_credits) {
			gather_images(about, images_campaigns[id]);
		} else {
			gather_images(about, images_general);
		}
	}

	if(cfg["sort"].to_bool(false)) {
		std::sort(sections.begin(), sections.end());
	}
}

credits_group::about_group::about_group(const config& cfg)
	: names()
	, title(cfg["title"].t_str())
{
	names.reserve(cfg.child_count("entry"));

	for(const config& entry : cfg.child_range("entry")) {
		names.emplace_back(font::escape_text(entry["name"].str()), font::escape_text(entry["comment"].str()));
	}
}

bool credits_group::about_group::operator<(const about_group& o) const
{
	return translation::compare(title.str(), o.title.str()) < 0;
}

const credits_data& get_credits_data()
{
	return parsed_credits_data;
}

utils::optional<credits_data::const_iterator> get_campaign_credits(const std::string& campaign)
{
	const auto res = utils::ranges::find(get_credits_data(), campaign, &credits_group::id);
	return res != parsed_credits_data.end() ? utils::make_optional(res) : utils::nullopt;
}

std::vector<std::string> get_background_images(const std::string& campaign)
{
	if(campaign.empty()) {
		return images_general;
	}

	if(const auto it = images_campaigns.find(campaign); it != images_campaigns.cend()) {
		return it->second;
	}

	return images_general;
}

void set_about(const game_config_view& cfg)
{
	parsed_credits_data.clear();

	// TODO: should we reserve space in parsed_credits_data here?

	images_campaigns.clear();
	images_general.clear();

	//
	// Parse all [credits_group] tags
	//
	for(const config& group : cfg.child_range("credits_group")) {
		if(group.has_child("about")) {
			parsed_credits_data.emplace_back(group, false);

			// Not in the credits_group since we don't want to inadvertently
			// pick up images from campaigns.
			gather_images(group, images_general);
		}
	}

	//
	// Parse all toplevel [about] tags.
	//
	config misc;
	for(const config& about : cfg.child_range("about")) {
		misc.add_child("about", about);
	}

	if(!misc.empty()) {
		parsed_credits_data.emplace_back(misc, false);
	}

	//
	// Parse all campaign [about] tags.
	//
	for(const config& campaign : cfg.child_range("campaign")) {
		if(campaign.has_child("about")) {
			parsed_credits_data.emplace_back(campaign, true);
		}
	}
}

} // end namespace about
