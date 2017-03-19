/*
   Copyright (C) 2003 - 2017 by Fabian Mueller <fabianmueller5@gmx.de>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef OVERLAY_INCLUDED
#define OVERLAY_INCLUDED

#include "halo.hpp"

struct overlay
{

	overlay(const std::string& img, const std::string& halo_img,
			halo::handle handle, const std::string& overlay_team_name, const std::string& item_id, const bool fogged) : image(img), halo(halo_img),
					team_name(overlay_team_name), id(item_id), halo_handle(handle) , visible_in_fog(fogged)
	{}


	overlay(const config& cfg) :
		image(cfg["image"]), halo(cfg["halo"]), team_name(cfg["team_name"]),
		name(cfg["name"].t_str()), id(cfg["id"]),
		halo_handle(), visible_in_fog(cfg["visible_in_fog"].to_bool())
	{
	}

	std::string image;
	std::string halo;
	std::string team_name;
	t_string name;
	std::string id;

	halo::handle halo_handle;
	bool visible_in_fog;

};


#endif
