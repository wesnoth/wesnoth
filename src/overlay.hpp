/*
	Copyright (C) 2003 - 2024
	by Fabian Mueller <fabianmueller5@gmx.de>
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

#include "halo.hpp"

struct overlay
{

	overlay(const std::string& img,
			const std::string& halo_img,
			const std::string& overlay_team_name,
			const std::string& item_id,
			const bool fogged,
			float submerge,
			float item_z_order = 0)
		: image(img)
		, halo(halo_img)
		, team_name(overlay_team_name)
		, name()
		, id(item_id)
		, halo_handle()
		, visible_in_fog(fogged)
		, submerge(submerge)
		, z_order(item_z_order)
	{}


	overlay(const config& cfg)
		: image(cfg["image"])
		, halo(cfg["halo"])
		, team_name(cfg["team_name"])
		, name(cfg["name"].t_str())
		, id(cfg["id"])
		, halo_handle()
		, visible_in_fog(cfg["visible_in_fog"].to_bool())
		, submerge(cfg["submerge"].to_double(0))
		, z_order(cfg["z_order"].to_double(0))
	{
	}

	std::string image;
	std::string halo;
	std::string team_name;
	t_string name;
	std::string id;

	halo::handle halo_handle;
	bool visible_in_fog;
	float submerge;
	float z_order;

};
