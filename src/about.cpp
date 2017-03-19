/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
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
static std::map<std::string, std::string> images;
static std::string images_default;

const config& get_about_config()
{
	return about_list;
}

std::vector<std::string> get_background_images(const std::string& campaign)
{
	std::vector<std::string> image_list;

	if(!campaign.empty() && !images[campaign].empty()){
		image_list = utils::parenthetical_split(images[campaign], ',');
	} else{
		image_list = utils::parenthetical_split(images_default, ',');
	}

	return image_list;
}

void set_about(const config &cfg)
{
	about_list.clear();

	images.clear();
	images_default.clear();

	for(const config& about : cfg.child_range("about")) {
		about_list.add_child("about", about);

		const std::string& im = about["images"];
		if(!im.empty()) {
			if(images_default.empty()) {
				images_default = im;
			} else {
				images_default += ',' + im;
			}
		}
	}

	for(const config& campaign : cfg.child_range("campaign")) {
		if(!campaign.has_child("about")) {
			continue;
		}

		const std::string& id = campaign["id"];

		config temp;
		temp["title"] = campaign["name"];
		temp["id"] = id;

		std::string campaign_images;

		for(const config& about : campaign.child_range("about")) {
			temp.add_child("about", about);

			const std::string& im = about["images"];
			if(!im.empty())	{
				if(campaign_images.empty()) {
					campaign_images = im;
				} else {
					campaign_images += ',' + im;
				}
			}
		}

		images[id] = campaign_images;

		about_list.add_child("credits_group", temp);
	}
}

} // end namespace about
