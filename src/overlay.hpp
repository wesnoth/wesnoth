/*
	Copyright (C) 2003 - 2025
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
#include "display.hpp"

struct overlay
{

	overlay(const std::string& img,
			const std::string& halo_img,
			const std::string& overlay_team_name,
			const std::string& item_id,
			const std::string& drawing_layer,
			const bool fogged,
			const bool multihex,
			float submerge,
			float parallax_mult,
			float item_z_order = 0,
			std::chrono::milliseconds duration = std::chrono::milliseconds(0))
		: image(img)
		, halo(halo_img)
		, team_name(overlay_team_name)
		, name() // The relation between id and name is strange, they are often assumed to be the same. Can cause issues for removal.
		, id(item_id)
		, drawing_layer(drawing_layer)
		, halo_handle()
		, visible_in_fog(fogged)
		, multihex(multihex)
		, submerge(submerge)
		, parallax_mult(parallax_mult)
		, z_order(item_z_order)
		, duration(duration)
	{}


	overlay(const config& cfg)
		: image(cfg["image"])
		, halo(cfg["halo"])
		, team_name(cfg["team_name"])
		, name(cfg["name"].t_str())
		, id(cfg["id"])
		, drawing_layer(cfg["drawing_layer"])
		, halo_handle()
		, visible_in_fog(cfg["visible_in_fog"].to_bool())
		, multihex(cfg["multihex"].to_bool())
		, submerge(cfg["submerge"].to_double(0))
		, parallax_mult(cfg["parallax_mult"].to_double(1.0))
		, z_order(cfg["z_order"].to_double(0))
		, duration(std::chrono::milliseconds(cfg["duration"].to_int(0)))
	{
	}

	std::string image;
	std::string halo;
	std::string team_name;
	t_string name;
	std::string id;
	std::string drawing_layer; // List of input based on drawing_layer.hpp

	halo::handle halo_handle;
	bool visible_in_fog;
	bool multihex;
	float submerge;
	float parallax_mult;
	float z_order;
	std::chrono::milliseconds duration;

	// Other support
	bool is_animated = false;
	bool is_child = false;         // A child overlay part of an larger multihex image (center part is parent)
	std::vector<std::pair<std::string, map_location>> child_hexes; // Locations and ids of child hexes if multihex (stored in parents)
	animated<image::locator> anim; // Manages the sequence of frames and timing for animated overlays.
};
