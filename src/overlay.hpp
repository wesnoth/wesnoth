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
			const bool fogged,
			const bool multihex,
			float submerge,
			float parallax_r,
			int layer = 0,
			float item_z_order = 0,
			std::chrono::milliseconds duration = std::chrono::milliseconds(0),
			int pixel_offset_x = 0,
			int pixel_offset_y = 0)
		: image(img)
		, halo(halo_img)
		, team_name(overlay_team_name)
		, name() // The relation between id and name is strange, they are often assumed to be the same. Can cause issues for removal.
		, id(item_id)
		, halo_handle()
		, visible_in_fog(fogged)
		, multihex(multihex)
		, submerge(submerge)
		, parallax_r(parallax_r)
		, layer(layer)
		, z_order(item_z_order)
		, duration(duration)
		, pixel_offset_x(pixel_offset_x)
		, pixel_offset_y(pixel_offset_y)
	{
		if(this->layer != 0) { // Offset layer to match unit layer. So that they behave the same way. (Gap between "terrain_bg" and "unit_first")
			constexpr int layer_offset = static_cast<int>(drawing_layer::unit_first) - static_cast<int>(drawing_layer::terrain_bg);
			this->layer += layer_offset;
		}
	}


	overlay(const config& cfg)
		: image(cfg["image"])
		, halo(cfg["halo"])
		, team_name(cfg["team_name"])
		, name(cfg["name"].t_str())
		, id(cfg["id"])
		, halo_handle()
		, visible_in_fog(cfg["visible_in_fog"].to_bool())
		, multihex(cfg["multihex"].to_bool())
		, submerge(cfg["submerge"].to_double(0))
		, parallax_r(cfg["parallax_r"].to_double(1.0))
		, layer(cfg["layer"].to_int(0))
		, z_order(cfg["z_order"].to_double(0))
		, duration(std::chrono::milliseconds(cfg["duration"].to_int(0)))
		, pixel_offset_x(cfg["pixel_offset_x"].to_int(0))
		, pixel_offset_y(cfg["pixel_offset_y"].to_int(0))
	{
	}

	std::string image;
	std::string halo;
	std::string team_name;
	t_string name;
	std::string id;

	halo::handle halo_handle;
	bool visible_in_fog;
	bool multihex; // True if this overlay is part of a multihex overlay.
	float submerge; // How deep the overlay is submerged on water hexes (0.0 = not submerged).
	float parallax_r; // Radial parallax factor for the overlay, (How fast it moves during scrolling, 1.0 = normal, <1.0 = slower, >1.0 = faster).
	int layer; // Layer offset for the overlay, higher values are drawn on top of lower values. Halos are always on top.
	float z_order; // Layer offset for the overlay within a layer. Images are sorted by layer and then z_order within each layer.
	std::chrono::milliseconds duration; // Duration before the overlay is removed in ms, 0 means infinite.
	int pixel_offset_x;
	int pixel_offset_y;

	// Other support
	bool is_animated = false;
	bool is_child = false;         // A child overlay part of an larger multihex image (center part is parent)
	map_location parent_location;  // Location of parent hex for multihex children (used for submerge calculation)
	int child_height = 0;        // Height of child hex (used for submerge calculation)
	std::vector<std::pair<std::string, map_location>> child_hexes; // Locations and ids of child hexes if multihex (stored in parents)
	animated<image::locator> animation; // Manages the sequence of frames and timing for animated overlays.
	image::locator image_static; // Locator for static (non-animated) overlays.
};
