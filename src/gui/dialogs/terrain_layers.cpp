/*
   Copyright (C) 2016 - 2018 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/terrain_layers.hpp"

#include "display.hpp"
#include "formatter.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/label.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "image.hpp"

namespace gui2
{
namespace dialogs
{

REGISTER_DIALOG(terrain_layers)

terrain_layers::terrain_layers(display_t& disp, const map_location& loc)
	: tile_(nullptr)
	, tile_logs_()
{
	terrain_builder& builder = disp.get_builder();
	tile_ = builder.get_tile(loc);

	assert(tile_);

	const std::string& tod_id = disp.get_time_of_day(loc).id;
	tile_->rebuild_cache(tod_id, &tile_logs_);
}

void terrain_layers::pre_show(window& window)
{
    //
	// List terrain flags
	//
	std::vector<std::string> flags(tile_->flags.begin(), tile_->flags.end());

	for(auto& flag : flags) {
		flag = (formatter() << font::unicode_bullet << " " << flag).str();
	}

	find_widget<label>(&window, "flags", false).set_label(utils::join(flags, "\n"));

	//
	// Generate terrain list
	//
	listbox& list = find_widget<listbox>(&window, "layer_list", false);

	int order = 1;
	for(const terrain_builder::tile::log_details det : tile_logs_) {
		const terrain_builder::tile::rule_image_rand& ri   = *det.first;
		const terrain_builder::rule_image_variant& variant = *det.second;

		// TODO: also use random image variations (not just take 1st)
		const image::locator& img = variant.images.front().get_first_frame();
		const std::string& name = img.get_filename();
		// TODO: deal with (rarely used) ~modifications
		//const std::string& modif = img.get_modifications();
		const map_location& loc_cut = img.get_loc();

		std::map<std::string, string_map> data;
		string_map item;

		item["label"] = (formatter() << (ri->is_background() ? "B ": "F ") << order).str();
		data.emplace("index", item);

		std::ostringstream image_steam;

		const int tz = game_config::tile_size;
		SDL_Rect r {0,0,tz,tz};

		surface surf = image::get_image(img.get_filename());

		// calculate which part of the image the terrain engine uses
		if(loc_cut.valid()) {
			// copied from image.cpp : load_image_sub_file()
			r = {
				((tz * 3) / 4) * loc_cut.x
				, tz           * loc_cut.y + (tz / 2) * (loc_cut.x % 2)
				, tz
				, tz
			};

			if(img.get_center_x() >= 0 && img.get_center_y() >= 0) {
				r.x += surf->w / 2 - img.get_center_x();
				r.y += surf->h / 2 - img.get_center_y();
			}
		}

		image_steam << "terrain/foreground.png";

		// Cut and mask the image
		// ~CROP and ~BLIT have limitations, we do some math to avoid them
		// TODO: ^ eh? what limitations?
		SDL_Rect r2 = sdl::intersect_rects(r, {0,0,surf->w,surf->h});
		if(r2.w > 0 && r2.h > 0) {
			image_steam
				<< "~BLIT(" << name
					<< "~CROP("
						<< r2.x << "," << r2.y << ","
						<< r2.w << "," << r2.h << ")"
					<< "," << r2.x - r.x << "," << r2.y - r.y
				<< ")"
				<< "~MASK(" << "terrain/alphamask.png" << ")";
		}

		item["label"] = image_steam.str();
		data.emplace("image_used", item);

		item["label"] = name + "~SCALE(72,72)";
		data.emplace("image_full", item);

		item["label"] = name;
		data.emplace("name", item);

		item["label"] = (formatter() << img.get_loc()).str();
		data.emplace("loc", item);

		item["label"] = std::to_string(ri->layer);
		data.emplace("layer", item);

		item["label"] = std::to_string(ri->basex);
		data.emplace("base_x", item);

		item["label"] = std::to_string(ri->basey);
		data.emplace("base_y", item);

		item["label"] = (formatter() << ri->center_x << ", " << ri->center_y).str();
		data.emplace("center", item);

		++order;

		list.add_row(data);
	}
}

} // namespace dialogs
} // namespace gui2
