/* $Id$ */
/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef PUBLISH_CAMPAIGN_HPP_INCLUDED
#define PUBLISH_CAMPAIGN_HPP_INCLUDED

class config;

#include <string>
#include <vector>

void get_campaign_info(const std::string& campaign_name, class config& cfg);
void set_campaign_info(const std::string& campaign_name, const class config& cfg);

std::vector<std::string> available_campaigns();
std::vector<std::string> installed_campaigns();
void archive_campaign(const std::string& campaign_name, class config& cfg);
void unarchive_campaign(const class config& cfg);

bool campaign_name_legal(const std::string& name);
bool check_names_legal(const config& dir);
std::vector<config *> find_scripts(const config &cfg, std::string extension);

#endif
