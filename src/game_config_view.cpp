/*
	Copyright (C) 2013 - 2025
	by Andrius Silinskas <silinskas.andrius@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "game_config_view.hpp"
#include "config.hpp"
#include "log.hpp"

static lg::log_domain log_config("config");
#define ERR_CONFIG LOG_STREAM(err, log_config)
#define WRN_CONFIG LOG_STREAM(warn, log_config)
#define LOG_CONFIG LOG_STREAM(info, log_config)


config_array_view game_config_view::child_range(std::string_view key) const
{
	config_array_view res;
	if(cfgs_.size() <= 1 || key != "terrain_graphics") {
		for(const config& cfg : cfgs_) {
			for (const config& child : cfg.child_range(key)) {
				res.push_back(child);
			}
		}
	}
	else {
		//use mainline [terrain_graphics] last. cfgs_.front() is the main game configs while the later ones are add-ons.
		for(const config& cfg : boost::make_iterator_range(cfgs_.begin() + 1, cfgs_.end())) {
			for (const config& child : cfg.child_range(key)) {
				res.push_back(child);
			}
		}
		for (const config& child : cfgs_.front().get().child_range(key)) {
			res.push_back(child);
		}
	}
	return res;
}

optional_const_config game_config_view::find_child(std::string_view key, const std::string &name, const std::string &value) const
{
	for(const config& cfg : cfgs_) {
		if(optional_const_config res = cfg.find_child(key, name, value)) {
			return res;
		}
	}
	LOG_CONFIG << "gcv : cannot find [" << key <<  "] with " << name  << "=" << value << ", count = " << cfgs_.size();
	return optional_const_config();
}

const config& game_config_view::find_mandatory_child(std::string_view key, const std::string &name, const std::string &value) const
{
	auto res = find_child(key, name, value);
	if(res) {
		return *res;
	}
	throw config::error("Cannot find child [" + std::string(key) + "] with " + name + "=" + value);
}

const config& game_config_view::mandatory_child(std::string_view key) const
{
	for(const config& cfg : cfgs_) {
		if(const auto res = cfg.optional_child(key)) {
			return res.value();
		}
	}
	throw config::error("missing WML tag [" + std::string(key) + "]");
}

optional_const_config game_config_view::optional_child(std::string_view key) const
{
	for(const config& cfg : cfgs_) {
		if(const auto res = cfg.optional_child(key)) {
			return res.value();
		}
	}
	return optional_const_config();
}


const config& game_config_view::child_or_empty(std::string_view key) const
{
	for(const config& cfg : cfgs_) {
		if(const auto res = cfg.optional_child(key)) {
			return res.value();
		}
	}
	static const config cfg;
	return cfg;
}

game_config_view game_config_view::merged_children_view(std::string_view key) const
{
	game_config_view res;
	for(const config& cfg : cfgs_) {

		for(const config& child : cfg.child_range(key)) {
			res.cfgs_.push_back(child);
		}
	}
	return res;
}
