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

#include "about.hpp"

#include "config.hpp"
#include "serialization/string_utils.hpp"

#include <map>

/**
 * @namespace about
 * Display credits %about all contributors.
 */
namespace about
{

static config about_list;
static std::map<std::string, std::vector<std::string>> images;
static std::vector<std::string> images_default;

const config& get_about_config()
{
	return about_list;
}

std::vector<std::string> get_background_images(const std::string& campaign)
{
	if(!campaign.empty() && !images[campaign].empty()){
		return images[campaign];
	}

	return images_default;
}

static void gather_images(const config& from, std::vector<std::string>& to)
{
	const auto& im = utils::parenthetical_split(from["images"], ',');
	if(!im.empty()) {
		to.insert(to.end(), im.begin(), im.end());
	}
}

void set_about(const config &cfg)
{
	about_list.clear();

	images.clear();
	images_default.clear();

	for(const config& group : cfg.child_range("credits_group")) {
		if(!group.has_child("about")) {
			continue;
		}
		about_list.add_child("credits_group", group);
		gather_images(group, images_default);
		for(const config& about : group.child_range("about")) {
			gather_images(about, images_default);
		}
	}

	config misc;
	for(const config& about : cfg.child_range("about")) {
		misc.add_child("about", about);
		gather_images(about, images_default);
	}
	if(!misc.empty()) {
		about_list.add_child("credits_group", std::move(misc));
	}

	for(const config& campaign : cfg.child_range("campaign")) {
		if(!campaign.has_child("about")) {
			continue;
		}

		const std::string& id = campaign["id"];

		config temp;
		temp["title"] = campaign["name"];
		temp["id"] = id;

		for(const config& about : campaign.child_range("about")) {
			temp.add_child("about", about);
			gather_images(about, images[id]);
		}

		about_list.add_child("credits_group", std::move(temp));
	}
}

} // end namespace about
